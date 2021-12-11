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
#include "object/sp-item.h"
#include "object/sp-namedview.h"
#include "object/sp-page.h"
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
    if (auto next = page->getNextPage()) {
        // Inserted in the middle, probably an undo.
        auto it = std::find(pages.begin(), pages.end(), next);
        g_assert (it != pages.end());
        pages.insert(it, page);
    } else {
        pages.push_back(page);
    }
    page->setManager(this);
    pagesChanged();
}

/**
 * Remove a page from this manager, called from namedview parent.
 */
void PageManager::removePage(Inkscape::XML::Node *child)
{
    for (auto it = pages.begin(); it != pages.end(); ++it) {
        if ((*it)->getRepr() == child) {
            pages.erase(it);
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
    // Get a new location for the page.
    double top = 0.0;
    double left = 0.0;
    for (auto &page : pages) {
        auto rect = page->getRect();
        if (rect.right() > left) {
            left = rect.right() + 10;
        }
    }
    return newPage(Geom::Rect(left, top, left + width, top + height));
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
        if (_selected_page == page) {
            if (auto next = page->getNextPage()) {
                selectPage(next);
            } else if (auto prev = page->getPreviousPage()) {
                selectPage(prev);
            } else {
                selectPage(nullptr);
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
    if (hasPages()) {
        for (auto &page : pages) {
            page->deleteObject();
        }
    }
}

/**
 * Get page index, returns -1 if the page is not found in this document.
 */
int PageManager::getPageIndex(SPPage *page) const
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
 * Get the page at the given positon or return nullptr if out of range.
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
void PageManager::moveItems(Geom::Affine translate, std::vector<SPItem *> const objects)
{
    auto scale = _document->getDocumentScale();
    for (auto &item : objects) {
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
            }
            break;
        case SPAttr::INKSCAPE_PAGEOPACITY:
            sp_ink_read_opacity(value, &this->background_color, 0xffffff00);
            break;
        case SPAttr::INKSCAPE_PAGESHADOW:
            this->shadow_size = value ? atoi(value) : 2;
            break;
        case SPAttr::SHOWPAGESHADOW: // Depricated
            this->shadow_show.readOrUnset(value);
            break;
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
    return item->setAttributes(border_on_top, border_show ? border_color : 0x0, background_color,
                               border_show && shadow_show ? shadow_size : 0);
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
