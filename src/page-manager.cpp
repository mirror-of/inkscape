// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape::PageManager - Multi-Page management.
 *
 * Copyright 2021 Martin Owens <doctormo@geek-2.com>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "page-manager.h"

#include "attributes.h"
#include "desktop.h"
#include "display/control/canvas-page.h"
#include "document.h"
#include "object/object-set.h"
#include "object/sp-item.h"
#include "object/sp-namedview.h"
#include "object/sp-page.h"
#include "object/sp-root.h"
#include "selection-chemistry.h"
#include "svg/svg-color.h"

namespace Inkscape {

bool PageManager::move_objects()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    return prefs->getBool("/tools/pages/move_objects", true);
}

PageManager::PageManager(SPDocument *document)
    : border_show(true)
    , border_on_top(true)
    , shadow_show(true)
    , _checkerboard(false)
{
    _document = document;
}

PageManager::~PageManager()
{
    pages.clear();
    _selected_page = nullptr;
    _document = nullptr;
}

/**
 * Add a page to this manager, called from namedview parent.
 */
void PageManager::addPage(SPPage *page)
{
    g_assert(page->document == _document);
    if (std::find(pages.begin(), pages.end(), page) != pages.end()) {
        // Refuse to double add pages to list.
        return;
    }
    if (auto next = page->getNextPage()) {
        // Inserted in the middle, probably an undo.
        auto it = std::find(pages.begin(), pages.end(), next);
        if (it != pages.end()) {
            pages.insert(it, page);
        } else {
            // This means the next page is not yet added either
            pages.push_back(page);
        }
    } else {
        pages.push_back(page);
    }
    pagesChanged();
}

/**
 * Remove a page from this manager, called from namedview parent.
 */
void PageManager::removePage(Inkscape::XML::Node *child)
{
    for (auto it = pages.begin(); it != pages.end(); ++it) {
        SPPage *page = *it;
        if (page->getRepr() == child) {
            pages.erase(it);

            // Reselect because this page is gone.
            if (_selected_page == page) {
                if (auto next = page->getNextPage()) {
                    selectPage(next);
                } else if (auto prev = page->getPreviousPage()) {
                    selectPage(prev);
                } else {
                    selectPage(nullptr);
                }
            }

            pagesChanged();
            break;
        }
    }
}

/**
 * Reorder page within the internal list to keep it up to date.
 */
void PageManager::reorderPage(Inkscape::XML::Node *child)
{
    auto nv = _document->getNamedView();
    pages.clear();
    for (auto &child : nv->children) {
        if (auto page = dynamic_cast<SPPage *>(&child)) {
            pages.push_back(page);
        }
    }
    pagesChanged();
}

/**
 * Enables multi page support by turning the document viewBox into
 * the first page.
 */
void PageManager::enablePages()
{
    if (!hasPages()) {
        _selected_page = newDesktopPage(*_document->preferredBounds(), true);
    }
}

/**
 * Add a new page of the default size, this will be either
 * the size of the viewBox if no pages exist, or the size
 * of the selected page.
 */
SPPage *PageManager::newPage()
{
    enablePages();
    auto rect = _selected_page->getRect();
    return newPage(rect.width(), rect.height());
}

/**
 * Add a new page of the given width and height.
 */
SPPage *PageManager::newPage(double width, double height)
{
    auto loc = nextPageLocation();
    return newPage(Geom::Rect::from_xywh(loc, Geom::Point(width, height)));
}

/**
 * Return the location of the next created page.
 */
Geom::Point PageManager::nextPageLocation() const
{
    // Get a new location for the page.
    double top = 0.0;
    double left = 0.0;
    for (auto &page : pages) {
        auto rect = page->getRect();
        if (rect.right() > left) {
            left = rect.right() + 10;
        }
    }
    return Geom::Point(left, top);
}

/**
 * Add a new page with the given rectangle.
 */
SPPage *PageManager::newPage(Geom::Rect rect, bool first_page)
{
    // This turns on pages support, which will make two pages if none exist yet.
    // The first is the ViewBox page, and the second is made below as the "second"
    if (!hasPages() && !first_page) {
        enablePages();
    }

    auto xml_doc = _document->getReprDoc();
    auto repr = xml_doc->createElement("inkscape:page");
    repr->setAttributeSvgDouble("x", rect.left());
    repr->setAttributeSvgDouble("y", rect.top());
    repr->setAttributeSvgDouble("width", rect.width());
    repr->setAttributeSvgDouble("height", rect.height());
    if (auto nv = _document->getNamedView()) {
        if (auto page = dynamic_cast<SPPage *>(nv->appendChildRepr(repr))) {
            Inkscape::GC::release(repr);
            return page;
        }
    }
    return nullptr;
}

/**
 * Create a new page, resizing the rectangle from desktop coordinates.
 */
SPPage *PageManager::newDesktopPage(Geom::Rect rect, bool first_page)
{
    return newPage(rect * _document->getDocumentScale().inverse(), first_page);
}

/**
 * Create a new page from another page. This can be in it's own document
 * or the same document (cloning) all items are also cloned.
 */
SPPage *PageManager::newPage(SPPage *page)
{
    auto xml_root = _document->getReprDoc();
    auto sp_root = _document->getRoot();

    // Record the new location of the new page.
    enablePages();
    auto new_loc = nextPageLocation();
    auto new_page = newDesktopPage(page->getDesktopRect(), false);
    Geom::Affine page_move = Geom::Translate((new_loc * _document->getDocumentScale()) - new_page->getDesktopRect().min());
    Geom::Affine item_move = Geom::Translate(new_loc - new_page->getRect().min());

    for (auto &item : page->getOverlappingItems()) {
        auto new_repr = item->getRepr()->duplicate(xml_root);
        if (auto new_item = dynamic_cast<SPItem *>(sp_root->appendChildRepr(new_repr))) {
            Geom::Affine affine = Geom::Affine();

            // a. Add the object's original transform back in.
            affine *= item->transform;

            // b. apply parent transform (for layers that have been ignored by getOverlappingItems)
            if (auto parent = dynamic_cast<SPItem *>(item->parent)) {
                affine *= parent->i2doc_affine();
            }

            // c. unit conversion, add in _document->getDocumentScale()
            affine *= _document->getDocumentScale().inverse();

            // d. apply item_move to offset it.
            affine *= item_move;

            new_item->doWriteTransform(affine, &affine, false);
        }
    }
    new_page->movePage(page_move, false);
    return new_page;
}

/**
 * Delete the given page.
 *
 * @param page - The page to be deleted.
 * @param content - Also remove the svg objects that are inside the page.
 */
void PageManager::deletePage(SPPage *page, bool content)
{
    if (page) {
        if (content) {
            for (auto &item : page->getExclusiveItems()) {
                item->deleteObject();
            }
            for (auto &item : page->getOverlappingItems()) {
                // Only delete objects when they rest on one page.
                if (getPagesFor(item, false).size() == 1) {
                    item->deleteObject();
                }
            }
        }
        // Only adjust if there will be a page after viewport page is deleted
        bool fit_viewport = page->isViewportPage() && getPageCount() > 2;

        // Removal from pages is done automatically via signals.
        page->deleteObject();

        if (fit_viewport) {
            _document->fitToRect(getFirstPage()->getDesktopRect(), false);
        }
    }

    // As above with the viewbox shadowing, we need go back to a single page
    // (which is zero pages) when needed.
    if (getPageCount() == 1) {
        if (auto page = getFirstPage()) {
            auto rect = page->getDesktopRect();
            deletePage(page, false);
            _document->fitToRect(rect, false);
         }
    }
}

/**
 * Delete the selected page.
 *
 * @param content - Also remove the svg objects that are inside the page.
 */
void PageManager::deletePage(bool content)
{
    deletePage(_selected_page, content);
}

/**
 * Disables multi page supply by removing all the page objects.
 */
void PageManager::disablePages()
{
    while (hasPages()) {
        deletePage(getLastPage(), false);
    }
}


/**
 * Get page index, returns -1 if the page is not found in this document.
 */
int PageManager::getPageIndex(const SPPage *page) const
{
    if (page) {
        auto it = std::find(pages.begin(), pages.end(), page);
        if (it != pages.end()) {
            return it - pages.begin();
        }
        g_warning("Can't get page index for %s", page->getId());
    }
    return -1;
}

/**
 * Return the index of the page in the index
 */
int PageManager::getSelectedPageIndex() const
{
    return getPageIndex(_selected_page);
}

/**
 * Returns the selected page rect, OR the viewbox rect.
 */
Geom::Rect PageManager::getSelectedPageRect() const
{
    return _selected_page ? _selected_page->getDesktopRect() : *(_document->preferredBounds());
}

/**
 * Called when the pages vector is updated, either page
 * deleted or page created (but not if the page is modified)
 */
void PageManager::pagesChanged()
{
    if (pages.empty() || getSelectedPageIndex() == -1) {
        selectPage(nullptr);
    }

    _pages_changed_signal.emit();

    if (!_selected_page) {
        for (auto &page : pages) {
            selectPage(page);
            break;
        }
    }
}

/**
 * Set the given page as the selected page.
 *
 * @param page - The page to set as the selected page.
 */
bool PageManager::selectPage(SPPage *page)
{
    if (!page || getPageIndex(page) >= 0) {
        if (_selected_page != page) {
            _selected_page = page;
            _page_selected_signal.emit(_selected_page);
            return true;
        }
    }
    return false;
}

/**
 * Select the first page the given sp-item object is within.
 *
 * If the item is between two pages and one of them is already selected
 * then don't change the selection.
 */
bool PageManager::selectPage(SPItem *item, bool contains)
{
    if (_selected_page && _selected_page->itemOnPage(item, contains)) {
        return true;
    }
    for (auto &page : getPagesFor(item, contains)) {
        return selectPage(page);
    }
    return false;
}

/**
 * Get the page at the given position or return nullptr if out of range.
 *
 * @param index - The page index (from 0) of the page.
 */
SPPage *PageManager::getPage(int index) const
{
    if (index < 0 || index >= pages.size()) {
        return nullptr;
    }
    return pages[index];
}

/**
 * Return a list of pages this item is on.
 */
std::vector<SPPage *> PageManager::getPagesFor(SPItem *item, bool contains) const
{
    std::vector<SPPage *> ret;
    for (auto &page : pages) {
        if (page->itemOnPage(item, contains)) {
            ret.push_back(page);
        }
    }
    return ret;
}

/**
 * Get a page at a specific starting location.
 */
SPPage *PageManager::getPageAt(Geom::Point pos) const
{
    for (auto &page : pages) {
        if (page->getDesktopRect().corner(0) == pos) {
            return page;
        }
    }
    return nullptr;
}

/**
 * Returns the page attached to the viewport, or nullptr if no pages
 * or none of the pages are the viewport page.
 */
SPPage *PageManager::getViewportPage() const
{
    for (auto &page : pages) {
        if (page->isViewportPage()) {
            return page;
        }
    }
    return nullptr;
}

/**
 * Returns the total area of all the pages in desktop units.
 */
Geom::OptRect PageManager::getDesktopRect() const
{
    Geom::OptRect total_area;
    for (auto &page : pages) {
        if (total_area) {
            total_area->unionWith(page->getDesktopRect());
        } else {
            total_area = page->getDesktopRect();
        }
    }
    return total_area;
}

/**
 * Center/zoom on the given page.
 */
void PageManager::zoomToPage(SPDesktop *desktop, SPPage *page, bool width_only)
{
    Geom::Rect rect = page ? page->getDesktopRect() : *(_document->preferredBounds());
    if (rect.minExtent() < 1.0)
        return;
    if (width_only) {
        desktop->set_display_width(rect, 10);
    } else {
        desktop->set_display_area(rect, 10);
    }
}

/**
 * Center without zooming on the given page
 */
void PageManager::centerToPage(SPDesktop *desktop, SPPage *page)
{
    Geom::Rect rect = page ? page->getDesktopRect() : *(_document->preferredBounds());
    desktop->set_display_center(rect);
}

void PageManager::resizePage(double width, double height)
{
    if (pages.empty() || _selected_page) {
        // Resizing the Viewport, means the page gets updated automatically
        if (pages.empty() || _selected_page->isViewportPage()) {
            auto rect = Geom::Rect(Geom::Point(0, 0), Geom::Point(width, height));
            _document->fitToRect(rect, false);
        } else {
            _selected_page->setDesktopSize(width, height);
        }
    }
}

/**
 * Change page orientation, landscape to portrait and back.
 */
void PageManager::changeOrientation()
{
    auto rect = getSelectedPageRect();
    resizePage(rect.height(), rect.width());
}

/**
 * Resize the page to the given selection. If nothing is selected,
 * Resize to all the items on the selected page.
 */
void PageManager::fitToSelection(ObjectSet *selection)
{
    auto desktop = selection->desktop();
    bool move_items = false; // DISABLED: This feature was set as a bug, but it's design is a little odd

    if (!selection || selection->isEmpty()) {
        // This means there aren't any pages, so revert to the default assumption
        // that the viewport is resized around ALL objects.
        if (!_selected_page) {
            fitToRect(_document->getRoot()->documentVisualBounds(), _selected_page);
        } else {
            // This allows the pages to be resized around the items related to the page only.
            auto contents = ObjectSet();
            contents.setList(getOverlappingItems(desktop, _selected_page));
            // Do we have anything to do?
            if (contents.isEmpty())
                return;
            fitToSelection(&contents);
        }
    } else if (auto rect = selection->visualBounds()) {
        if (move_objects() && move_items) {
            auto prev_items = getOverlappingItems(desktop, _selected_page);
            auto selected = selection->items();
            auto origin = Geom::Point(0, 0);
            if (_selected_page) {
                origin = _selected_page->getDesktopRect().min();
            }

            fitToRect(rect, _selected_page);

            // Do not move the selected items, as the page has just been moved around them.
            std::vector<SPItem *> page_items;
            std::set_difference(prev_items.begin(), prev_items.end(), selected.begin(), selected.end(),
                                std::insert_iterator<std::vector<SPItem *> >(page_items, page_items.begin()));

            moveItems(Geom::Translate(rect->min() - origin), page_items);
        } else {
            fitToRect(rect, _selected_page);
        }
    }
}

/**
 * Fit the selected page to the given rectangle.
 */
void PageManager::fitToRect(Geom::OptRect rect, SPPage *page)
{
    if (!rect) return;
    bool viewport = true;
    if (page) {
        viewport = page->isViewportPage();
        page->setDesktopRect(*rect);
    }
    if (viewport) {
        _document->fitToRect(*rect);
        if (page && !page->isViewportPage()) {
            // The document's fitToRect has slightly mangled the page rect, fix it.
            page->setDesktopRect(Geom::Rect(Geom::Point(0, 0), rect->dimensions()));
        }
    }
}


/**
 * Return a list of objects touching this page, or viewbox (of single page document)
 */
std::vector<SPItem *> PageManager::getOverlappingItems(SPDesktop *desktop, SPPage *page)
{
    if (page) {
        return page->getOverlappingItems();
    }
    auto doc_rect = _document->preferredBounds();
    return _document->getItemsPartiallyInBox(desktop->dkey, *doc_rect, true, true, true, false);
}

/**
 * Move the given items by the given affine (surely this already exists somewhere?)
 */
void PageManager::moveItems(Geom::Affine translate, std::vector<SPItem *> const &objects)
{
    for (auto &item : objects) {
        if (item->isLocked()) {
            continue;
        }
        if (auto parent_item = dynamic_cast<SPItem *>(item->parent)) {
            auto move = item->i2dt_affine() * (translate * parent_item->i2doc_affine().inverse());
            item->doWriteTransform(move, &move, false);
        }
    }
}


/**
 * Manage the page subset of attributes from sp-namedview and store them.
 */
bool PageManager::subset(SPAttr key, const gchar *value)
{
    switch (key) {
        case SPAttr::SHOWBORDER:
            this->border_show.readOrUnset(value);
            break;
        case SPAttr::BORDERLAYER:
            this->border_on_top.readOrUnset(value);
            break;
        case SPAttr::BORDERCOLOR:
            this->border_color = this->border_color & 0xff;
            if (value) {
                this->border_color = this->border_color | sp_svg_read_color(value, this->border_color);
            }
            break;
        case SPAttr::BORDEROPACITY:
            sp_ink_read_opacity(value, &this->border_color, 0x000000ff);
            break;
        case SPAttr::PAGECOLOR:
            this->background_color = this->background_color & 0xff;
            if (value) {
                this->background_color = this->background_color | sp_svg_read_color(value, this->background_color);
                return false; // propagate further
            }
            break;
        case SPAttr::INKSCAPE_PAGEOPACITY:
            sp_ink_read_opacity(value, &this->background_color, 0xffffff00);
            break;
        case SPAttr::SHOWPAGESHADOW: // Deprecated
            this->shadow_show.readOrUnset(value);
            break;
        case SPAttr::INKSCAPE_DESK_CHECKERBOARD:
            _checkerboard.readOrUnset(value);
            return false; // propagate further
        default:
            return false;
    }
    return true;
}

/**
 * Update the canvas item with the default display attributes.
 */
bool PageManager::setDefaultAttributes(Inkscape::CanvasPage *item)
{
    const int shadow_size = 2; // fixed, not configurable; shadow changes size with zoom
    return item->setAttributes(border_on_top, border_show ? border_color : 0x0, background_color,
                               border_show && shadow_show ? shadow_size : 0, _checkerboard);
}

/**
 * Called when the viewbox is resized.
 */
void PageManager::movePages(Geom::Affine tr)
{
    // Adjust each page against the change in position of the viewbox.
    for (auto &page : pages) {
        page->movePage(tr, false);
    }
}

}; // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
