// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape pages implementation
 *
 * Authors:
 *   Martin Owens <doctormo@geek-2.com>
 *
 * Copyright (C) 2021 Martin Owens
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <glibmm/i18n.h>

#include "sp-page.h"

#include "attributes.h"
#include "desktop.h"
#include "display/control/canvas-page.h"
#include "inkscape.h"
#include "object/object-set.h"
#include "sp-namedview.h"
#include "sp-root.h"

using Inkscape::DocumentUndo;

SPPage::SPPage()
    : SPObject()
{
    _canvas_item = new Inkscape::CanvasPage();
}

SPPage::~SPPage()
{
    delete _canvas_item;
    _canvas_item = nullptr;
}

void SPPage::build(SPDocument *document, Inkscape::XML::Node *repr)
{
    SPObject::build(document, repr);

    this->readAttr(SPAttr::INKSCAPE_LABEL);
    this->readAttr(SPAttr::X);
    this->readAttr(SPAttr::Y);
    this->readAttr(SPAttr::WIDTH);
    this->readAttr(SPAttr::HEIGHT);

    /* Register */
    document->addResource("page", this);
}

void SPPage::release()
{
    if (this->document) {
        // Unregister ourselves
        this->document->removeResource("page", this);
    }

    SPObject::release();
}

void SPPage::set(SPAttr key, const gchar *value)
{
    switch (key) {
        case SPAttr::X:
            this->x.readOrUnset(value);
            break;
        case SPAttr::Y:
            this->y.readOrUnset(value);
            break;
        case SPAttr::WIDTH:
            this->width.readOrUnset(value);
            break;
        case SPAttr::HEIGHT:
            this->height.readOrUnset(value);
            break;
        default:
            SPObject::set(key, value);
            break;
    }
    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Gets the rectangle in document units
 */
Geom::Rect SPPage::getRect() const
{
    return Geom::Rect(this->x.computed, this->y.computed, this->x.computed + this->width.computed,
                      this->y.computed + this->height.computed);
}

/**
 * Get the rectangle of the page, in desktop units
 */
Geom::Rect SPPage::getDesktopRect() const
{
    auto rect = getDocumentRect();
    rect *= document->dt2doc();
    return rect;
}

/**
 * Get the rectangle of the page, scaled to the document.
 */
Geom::Rect SPPage::getDocumentRect() const
{
    return getRect() * document->getDocumentScale();
}

/**
 * Like getDesktopRect but returns a slightly shrunken rectangle
 * so interactions don't confuse the border with the object.
 */
Geom::Rect SPPage::getSensitiveRect() const
{
    auto rect = getDesktopRect();
    rect.expandBy(-0.1);
    return rect;
}

/**
 * Set the page rectangle in it's native units.
 */
void SPPage::setRect(Geom::Rect rect)
{
    this->x = rect.left();
    this->y = rect.top();
    this->width = rect.width();
    this->height = rect.height();

    // This is needed to update the xml
    this->updateRepr();

    // This eventually calls the ::update below while idle
    this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Set the page rectangle is desktop coordinates.
 */
void SPPage::setDesktopRect(Geom::Rect rect)
{
    rect *= document->doc2dt();
    setRect(rect * document->getDocumentScale().inverse());
}

/**
 * Set just the height and width from a predefined size.
 */
void SPPage::setDesktopSize(double width, double height)
{
    auto rect = getDesktopRect();
    rect.setMax(rect.corner(0) + Geom::Point(width, height));
    setDesktopRect(rect);
}

/**
 * Get the items which are ONLY on this page and don't overlap.
 *
 * This ignores layers so items in the same layer which are shared
 * between pages are not moved around or exported into pages they
 * shouldn't be.
 *
 * @param hidden - Return hidden items (default: true)
 */
std::vector<SPItem *> SPPage::getExclusiveItems(bool hidden) const
{
    return document->getItemsInBox(0, getDocumentRect(), hidden, true, true, false);
}

/**
 * Like ExcludiveItems above but get all the items which are inside or overlapping.
 *
 * @param hidden - Return hidden items (default: true)
 */
std::vector<SPItem *> SPPage::getOverlappingItems(bool hidden) const
{
    return document->getItemsPartiallyInBox(0, getDocumentRect(), hidden, true, true, false);
}

/**
 * Return true if this item is contained within the page boundary.
 */
bool SPPage::itemOnPage(SPItem *item, bool contains) const
{
    if (auto box = item->desktopGeometricBounds()) {
        if (contains) {
            return getDesktopRect().contains(*box);
        }
        return getDesktopRect().intersects(*box);
    }
    return false;
}

/**
 * Returns true if this page is the same as the viewport.
 */
bool SPPage::isViewportPage() const
{
    auto rect = document->preferredBounds();
    return getDesktopRect().corner(0) == rect->corner(0);
}

/**
 * Shows the page in the given canvas item group.
 */
void SPPage::showPage(Inkscape::CanvasItemGroup *fg, Inkscape::CanvasItemGroup *bg)
{
    _canvas_item->add(getDesktopRect(), fg, bg);
    // The final steps are completed in an update cycle
    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Sets the default attributes from the namedview.
 */
bool SPPage::setDefaultAttributes()
{
    if (document->getPageManager().setDefaultAttributes(_canvas_item)) {
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        return true;
    }
    return false;
}

/**
 * Set the selected high-light for this page.
 */
void SPPage::setSelected(bool sel)
{
    this->_canvas_item->is_selected = sel;
    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Returns the page number (order of pages) starting at 1
 */
int SPPage::getPageIndex() const
{
    return document->getPageManager().getPageIndex(this);
}

/**
 * Set this page to a new order in the page stack.
 *
 * @param index - Placement of page in the stack, starting at '0'
 * @param swap_page - Swap the rectangle position
 *
 * @returns true if page has been moved.
 */
bool SPPage::setPageIndex(int index, bool swap_page)
{
    int current = getPageIndex();

    if (current != index) {
        auto &page_manager = document->getPageManager();

        // The page we're going to be shifting to
        auto sibling = page_manager.getPage(index);

        // Insertions are done to the right of the sibling
        if (index < current) {
            index -= 1;
        }
        auto insert_after = page_manager.getPage(index);

        // We may have selected an index off the end, so attach it after the last page.
        if (!insert_after && index > 0) {
            insert_after = page_manager.getLastPage();
            sibling = nullptr; // disable swap
        }

        if (insert_after) {
            if (this == insert_after) {
                g_warning("Page is already at this index. Not moving.");
                return false;
            }
            // Attach after the given page
            getRepr()->parent()->changeOrder(getRepr(), insert_after->getRepr());
        } else {
            // Attach to before any existing page
            sibling = page_manager.getFirstPage();
            getRepr()->parent()->changeOrder(getRepr(), nullptr);
        }
        if (sibling && swap_page) {
            swapPage(sibling, true);
        }
        return true;
    }
    return false;
}

/**
 * Returns the sibling page next to this one in the stack order.
 */
SPPage *SPPage::getNextPage()
{
    SPObject *item = this;
    while ((item = item->getNext())) {
        if (auto next = dynamic_cast<SPPage *>(item)) {
            return next;
        }
    }
    return nullptr;
}

/**
 * Returns the sibling page previous to this one in the stack order.
 */
SPPage *SPPage::getPreviousPage()
{
    SPObject *item = this;
    while ((item = item->getPrev())) {
        if (auto prev = dynamic_cast<SPPage *>(item)) {
            return prev;
        }
    }
    return nullptr;
}

/**
 * Move the page by the given affine, in desktop units.
 *
 * @param translate - The positional translation to apply.
 * @param with_objects - Flag to request that connected objects also move.
 */
void SPPage::movePage(Geom::Affine translate, bool with_objects)
{
    if (translate.isTranslation()) {
        if (with_objects) {
            // Move each item that is overlapping this page too
            moveItems(translate * document->dt2doc(), getOverlappingItems());
        }
        setDesktopRect(getDesktopRect() * translate);
    }
}

/**
 * Move the given items by the given translation in document units.
 *
 * @param translate - The movement to be applied
 * @param objects - a vector of SPItems to move
 */
void SPPage::moveItems(Geom::Affine translate, std::vector<SPItem *> const &objects)
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
 * Swap the locations of this page with another page (see movePage)
 *
 * @param other - The other page to swap with
 * @param with_objects - Should the page objects move too.
 */
void SPPage::swapPage(SPPage *other, bool with_objects)
{
    // Swapping with the viewport page must be handled gracefully.
    if (this->isViewportPage()) {
        auto other_rect = other->getDesktopRect();
        auto new_rect = Geom::Rect(Geom::Point(0, 0),
            Geom::Point(other_rect.width(), other_rect.height()));
        this->document->fitToRect(new_rect, false);
    } else if (other->isViewportPage()) {
        other->swapPage(this, with_objects);
        return;
    }

    auto this_affine = Geom::Translate(getDesktopRect().corner(0));
    auto other_affine = Geom::Translate(other->getDesktopRect().corner(0));
    movePage(this_affine.inverse() * other_affine, with_objects);
    other->movePage(other_affine.inverse() * this_affine, with_objects);
}

void SPPage::update(SPCtx * /*ctx*/, unsigned int /*flags*/)
{
    // This is manual because this is not an SPItem, but it's own visual identity.
    _canvas_item->update(getDesktopRect(), this->label());
}

/**
 * Write out the page's data into it's xml structure.
 */
Inkscape::XML::Node *SPPage::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("inkscape:page");
    }

    repr->setAttributeSvgDouble("x", this->x.computed);
    repr->setAttributeSvgDouble("y", this->y.computed);
    repr->setAttributeSvgDouble("width", this->width.computed);
    repr->setAttributeSvgDouble("height", this->height.computed);

    return SPObject::write(xml_doc, repr, flags);
}

std::string SPPage::getDefaultLabel() const
{
    gchar *format = g_strdup_printf(_("Page %d"), getPagePosition());
    auto ret = std::string(format);
    g_free(format);
    return ret;
}

std::string SPPage::getLabel() const
{
    auto ret = label();
    if (!ret) {
        return getDefaultLabel();
    }
    return std::string(ret);
}

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
