// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Page editing tool
 *
 * Authors:
 *   Martin Owens <doctormo@geek-2.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "pages-tool.h"

#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>

#include "desktop.h"
#include "display/control/canvas-item-bpath.h"
#include "display/control/canvas-item-curve.h"
#include "display/control/canvas-item-group.h"
#include "display/control/canvas-item-rect.h"
#include "display/control/snap-indicator.h"
#include "document-undo.h"
#include "include/macros.h"
#include "object/sp-page.h"
#include "path/path-outline.h"
#include "pure-transform.h"
#include "rubberband.h"
#include "selection-chemistry.h"
#include "selection.h"
#include "snap-preferences.h"
#include "snap.h"
#include "ui/icon-names.h"
#include "ui/knot/knot.h"
#include "ui/widget/canvas.h"

namespace Inkscape {
namespace UI {
namespace Tools {

PagesTool::PagesTool(SPDesktop *desktop)
    : ToolBase(desktop, "/tools/pages", "select.svg")
{
    // Stash the regular object selection so we don't modify them in base-tools root handler.
    desktop->selection->setBackup();
    desktop->selection->clear();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    drag_tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    if (resize_knots.empty()) {
        for (int i = 0; i < 4; i++) {
            auto knot = new SPKnot(desktop, _("Resize page"), Inkscape::CANVAS_ITEM_CTRL_TYPE_SHAPER, "PageTool:Resize");
            knot->setShape(Inkscape::CANVAS_ITEM_CTRL_SHAPE_SQUARE);
            knot->setFill(0xffffff00, 0x0000ff00, 0x000000ff, 0x000000ff);
            knot->setSize(9);
            knot->setAnchor(SP_ANCHOR_CENTER);
            knot->updateCtrl();
            knot->hide();
            knot->moved_signal.connect(sigc::mem_fun(*this, &PagesTool::resizeKnotMoved));
            knot->ungrabbed_signal.connect(sigc::mem_fun(*this, &PagesTool::resizeKnotFinished));
            if (auto window = desktop->getCanvas()->get_window()) {
                knot->setCursor(SP_KNOT_STATE_DRAGGING, this->get_cursor(window, "page-resizing.svg"));
                knot->setCursor(SP_KNOT_STATE_MOUSEOVER, this->get_cursor(window, "page-resize.svg"));
            }
            resize_knots.push_back(knot);
        }
    }

    if (!visual_box) {
        visual_box = new Inkscape::CanvasItemRect(desktop->getCanvasControls());
        visual_box->set_stroke(0x0000ff7f);
        visual_box->hide();
    }
    if (!drag_group) {
        drag_group = new Inkscape::CanvasItemGroup(desktop->getCanvasTemp());
        drag_group->set_name("CanvasItemGroup:PagesDragShapes");
    }
    auto &page_manager = desktop->getDocument()->getPageManager();
    _selector_changed_connection =
            page_manager.connectPageSelected(sigc::mem_fun(*this, &PagesTool::selectionChanged));
    selectionChanged(page_manager.getSelected());

    _doc_modified_connection = desktop->getDocument()->connectModified([=](guint){
        // This readjusts the knot when in single page mode.
        if (!desktop->getDocument()->getPageManager().hasPages()) {
            selectionChanged(nullptr);
        }
    });

    _zoom_connection = desktop->signal_zoom_changed.connect([=](double) {
        // This readjusts the knot on zoom because the viewbox position
        // becomes detached on zoom, likely a precision problem.
        if (!desktop->getDocument()->getPageManager().hasPages()) {
            selectionChanged(nullptr);
        }
    });
}

PagesTool::~PagesTool()
{
    _selector_changed_connection.disconnect();
    selectionChanged(nullptr);

    ungrabCanvasEvents();

    _desktop->selection->restoreBackup();

    if (visual_box) {
        delete visual_box;
        visual_box = nullptr;
    }

    for (auto knot : resize_knots) {
        delete knot;
    }
    resize_knots.clear();

    if (drag_group) {
        delete drag_group;
        drag_group = nullptr;
        drag_shapes.clear(); // Already deleted by group
    }

    _doc_replaced_connection.disconnect();
    _doc_modified_connection.disconnect();
    _zoom_connection.disconnect();
}

void PagesTool::resizeKnotSet(Geom::Rect rect)
{
    for (int i = 0; i < 4; i++) {
        resize_knots[i]->moveto(rect.corner(i));
        resize_knots[i]->show();
    }
}

void PagesTool::resizeKnotMoved(SPKnot *knot, Geom::Point const &ppointer, guint state)
{
    Geom::Rect rect;

    auto page = _desktop->getDocument()->getPageManager().getSelected();
    if (page) {
        // Resizing a specific selected page
        rect = page->getDesktopRect();
    } else if (auto document = _desktop->getDocument()) {
        // Resizing the naked viewBox
        rect = *(document->preferredBounds());
    }

    int index;
    for (index = 0; index < 4; index++) {
        if (knot == resize_knots[index]) {
            break;
        }
    }
    Geom::Point start = rect.corner(index);
    Geom::Point point = getSnappedResizePoint(knot->position(), state, start, page);

    if (point != start) {
        if (index % 3 == 0)
            rect[Geom::X].setMin(point[Geom::X]);
        else
            rect[Geom::X].setMax(point[Geom::X]);

        if (index < 2)
            rect[Geom::Y].setMin(point[Geom::Y]);
        else
            rect[Geom::Y].setMax(point[Geom::Y]);

        visual_box->show();
        visual_box->set_rect(rect);
        on_screen_rect = Geom::Rect(rect);
        mouse_is_pressed = true;
    }
}

/**
 * Resize snapping allows knot and tool point snapping consistency.
 */
Geom::Point PagesTool::getSnappedResizePoint(Geom::Point point, guint state, Geom::Point origin, SPObject *target)
{
    if (!(state & GDK_SHIFT_MASK)) {
        SnapManager &snap_manager = _desktop->namedview->snap_manager;
        snap_manager.setup(_desktop, true, target);
        Inkscape::SnapCandidatePoint scp(point, Inkscape::SNAPSOURCE_PAGE_CORNER);
        scp.addOrigin(origin);
        Inkscape::SnappedPoint sp = snap_manager.freeSnap(scp);
        point = sp.getPoint();
        snap_manager.unSetup();
    }
    return point;
}

void PagesTool::resizeKnotFinished(SPKnot *knot, guint state)
{
    auto document = _desktop->getDocument();
    auto page = document->getPageManager().getSelected();
    if (on_screen_rect) {
        if (!page || page->isViewportPage()) {
            // Adjust viewport so it's scroll adjustment is correct
            *on_screen_rect *= document->dt2doc();
        }
        document->getPageManager().fitToRect(*on_screen_rect, page);
        Inkscape::DocumentUndo::done(document, "Resize page", INKSCAPE_ICON("tool-pages"));
        on_screen_rect = {};
    }
    visual_box->hide();
    mouse_is_pressed = false;
}

bool PagesTool::root_handler(GdkEvent *event)
{
    bool ret = false;
    auto &page_manager = _desktop->getDocument()->getPageManager();

    switch (event->type) {
        case GDK_BUTTON_PRESS: {
            if (event->button.button == 1) {
                mouse_is_pressed = true;
                drag_origin_w = Geom::Point(event->button.x, event->button.y);
                drag_origin_dt = _desktop->w2d(drag_origin_w);
                ret = true;
                if (auto page = pageUnder(drag_origin_dt, false)) {
                    // Select the clicked on page. Manager ignores the same-page.
                    _desktop->getDocument()->getPageManager().selectPage(page);
                    this->set_cursor("page-dragging.svg");
                } else if (viewboxUnder(drag_origin_dt)) {
                    dragging_viewbox = true;
                    this->set_cursor("page-dragging.svg");
                } else {
                    drag_origin_dt = getSnappedResizePoint(drag_origin_dt, event->button.state, Geom::Point(0, 0));
                }
            }
            break;
        }
        case GDK_MOTION_NOTIFY: {

            auto point_w = Geom::Point(event->motion.x, event->motion.y);
            auto point_dt = _desktop->w2d(point_w);
            bool snap = !(event->motion.state & GDK_SHIFT_MASK);

            if (event->motion.state & GDK_BUTTON1_MASK) {
                if (!mouse_is_pressed) {
                    // this sometimes happens if the mouse was off the edge when the event started
                    drag_origin_w = point_w;
                    drag_origin_dt = point_dt;
                    mouse_is_pressed = true;
                }

                if (dragging_item || dragging_viewbox) {
                    // Continue to drag item.
                    Geom::Affine tr = moveTo(point_dt, snap);
                    // XXX Moving the existing shapes would be much better, but it has
                    // a weird bug which stops it from working well.
                    // drag_group->update(tr * drag_group->get_parent()->get_affine());
                    addDragShapes(dragging_item, tr);
                } else if (on_screen_rect) {
                    // Continue to drag new box
                    point_dt = getSnappedResizePoint(point_dt, event->motion.state, drag_origin_dt);
                    on_screen_rect = Geom::Rect(drag_origin_dt, point_dt);
                } else if (Geom::distance(drag_origin_w, point_w) < drag_tolerance) {
                    // do not start dragging anything new if we're within tolerance from origin.
                    // pass
                } else if (auto page = pageUnder(drag_origin_dt)) {
                    // Starting to drag page around the screen, the pageUnder must
                    // be the drag_origin as small movements can kill the UX feel.
                    dragging_item = page;
                    page_manager.selectPage(page);
                    addDragShapes(page, Geom::Affine());
                    grabPage(page);
                } else if (viewboxUnder(drag_origin_dt)) {
                    // Special handling of viewbox dragging
                    dragging_viewbox = true;
                } else {
                    // Start making a new page.
                    dragging_item = nullptr;
                    on_screen_rect = Geom::Rect(drag_origin_dt, drag_origin_dt);
                    this->set_cursor("page-draw.svg");
                }
            } else {
                mouse_is_pressed = false;
                drag_origin_dt = point_dt;
            }
            break;
        }
        case GDK_BUTTON_RELEASE: {
            if (event->button.button != 1) {
                break;
            }
            auto point_w = Geom::Point(event->button.x, event->button.y);
            auto point_dt = _desktop->w2d(point_w);
            bool snap = !(event->button.state & GDK_SHIFT_MASK);
            auto document = _desktop->getDocument();

            if (dragging_viewbox || dragging_item) {
                if (dragging_viewbox || dragging_item->isViewportPage()) {
                    // Move the document's viewport first
                    auto page_items = page_manager.getOverlappingItems(_desktop, dragging_item);
                    auto rect = document->preferredBounds();
                    auto affine = moveTo(point_dt, snap) * document->dt2doc();
                    document->fitToRect(*rect * affine, false);
                    // Now move the page back to where we expect it.
                    if (dragging_item) {
                        dragging_item->movePage(affine, false);
                        dragging_item->setDesktopRect(*rect);
                    }
                    // We have a custom move object because item detection is fubar after fitToRect
                    if (page_manager.move_objects()) {
                        SPPage::moveItems(affine, page_items);
                    }
                } else {
                    // Move the page object on the canvas.
                    dragging_item->movePage(moveTo(point_dt, snap), page_manager.move_objects());
                }
                Inkscape::DocumentUndo::done(_desktop->getDocument(), "Move page position", INKSCAPE_ICON("tool-pages"));
            } else if (on_screen_rect) {
                // conclude box here (make new page)
                page_manager.selectPage(page_manager.newDesktopPage(*on_screen_rect));
                Inkscape::DocumentUndo::done(_desktop->getDocument(), "Create new drawn page", INKSCAPE_ICON("tool-pages"));
            }
            mouse_is_pressed = false;
            drag_origin_dt = point_dt;
            ret = true;

            // Clear snap indication on mouse up.
            _desktop->snapindicator->remove_snaptarget();
            break;
        }
        case GDK_KEY_PRESS: {
            if (event->key.keyval == GDK_KEY_Escape) {
                mouse_is_pressed = false;
                ret = true;
            }
            if (event->key.keyval == GDK_KEY_Delete) {
                page_manager.deletePage(page_manager.move_objects());

                Inkscape::DocumentUndo::done(_desktop->getDocument(), "Delete Page", INKSCAPE_ICON("tool-pages"));
                ret = true;
            }
        }
        default:
            break;
    }

    // Clean up any finished dragging, doesn't matter how it ends
    if (!mouse_is_pressed && (dragging_item || on_screen_rect || dragging_viewbox)) {
        dragging_viewbox = false;
        dragging_item = nullptr;
        on_screen_rect = {};
        clearDragShapes();
        visual_box->hide();
        ret = true;
    } else if (on_screen_rect) {
        visual_box->show();
        visual_box->set_rect(*on_screen_rect);
        ret = true;
    }
    if (!mouse_is_pressed) {
        if (pageUnder(drag_origin_dt) || viewboxUnder(drag_origin_dt)) {
            // This page under uses the current mouse position (unlike the above)
            this->set_cursor("page-mouseover.svg");
        } else {
            this->set_cursor("page-draw.svg");
        }
    }


    return ret ? true : ToolBase::root_handler(event);
}

void PagesTool::menu_popup(GdkEvent *event, SPObject *obj)
{
    auto &page_manager = _desktop->getDocument()->getPageManager();
    SPPage *page = page_manager.getSelected();
    if (event->type != GDK_KEY_PRESS) {
        drag_origin_w = Geom::Point(event->button.x, event->button.y);
        drag_origin_dt = _desktop->w2d(drag_origin_w);
        page = pageUnder(drag_origin_dt);
    }
    if (page) {
        ToolBase::menu_popup(event, page);
    }
}

/**
 * Creates the right snapping setup for dragging items around.
 */
void PagesTool::grabPage(SPPage *target)
{
    _bbox_points.clear();
    getBBoxPoints(target->getDesktopRect(), &_bbox_points, false, SNAPSOURCE_PAGE_CORNER, SNAPTARGET_UNDEFINED,
                  SNAPSOURCE_UNDEFINED, SNAPTARGET_UNDEFINED, SNAPSOURCE_PAGE_CENTER, SNAPTARGET_UNDEFINED);
}

/*
 * Generate the movement affine as the page is dragged around (including snapping)
 */
Geom::Affine PagesTool::moveTo(Geom::Point xy, bool snap)
{
    Geom::Point dxy = xy - drag_origin_dt;

    if (snap) {
        SnapManager &snap_manager = _desktop->namedview->snap_manager;
        snap_manager.setup(_desktop, true, dragging_item);
        snap_manager.snapprefs.clearTargetMask(0); // Disable all snapping targets
        snap_manager.snapprefs.setTargetMask(SNAPTARGET_ALIGNMENT_CATEGORY, -1);
        snap_manager.snapprefs.setTargetMask(SNAPTARGET_ALIGNMENT_PAGE_CORNER, -1);
        snap_manager.snapprefs.setTargetMask(SNAPTARGET_ALIGNMENT_PAGE_CENTER, -1);
        snap_manager.snapprefs.setTargetMask(SNAPTARGET_PAGE_CORNER, -1);
        snap_manager.snapprefs.setTargetMask(SNAPTARGET_PAGE_CENTER, -1);
        snap_manager.snapprefs.setTargetMask(SNAPTARGET_GRID_INTERSECTION, -1);
        snap_manager.snapprefs.setTargetMask(SNAPTARGET_GUIDE, -1);
        snap_manager.snapprefs.setTargetMask(SNAPTARGET_GUIDE_INTERSECTION, -1);

        Inkscape::PureTranslate *bb = new Inkscape::PureTranslate(dxy);
        snap_manager.snapTransformed(_bbox_points, drag_origin_dt, (*bb));

        if (bb->best_snapped_point.getSnapped()) {
            dxy = bb->getTranslationSnapped();
            _desktop->snapindicator->set_new_snaptarget(bb->best_snapped_point);
        }

        snap_manager.snapprefs.clearTargetMask(-1); // Reset preferences
        snap_manager.unSetup();
    }

    return Geom::Translate(dxy);
}

/**
 * Add all the shapes needed to see it being dragged.
 */
void PagesTool::addDragShapes(SPPage *page, Geom::Affine tr)
{
    clearDragShapes();
    auto doc = _desktop->getDocument();

    if (page) {
        addDragShape(Geom::PathVector(Geom::Path(page->getDesktopRect())), tr);
    } else {
        auto doc_rect = doc->preferredBounds();
        addDragShape(Geom::PathVector(Geom::Path(*doc_rect)), tr);
    }
    if (Inkscape::Preferences::get()->getBool("/tools/pages/move_objects", true)) {
        for (auto &item : doc->getPageManager().getOverlappingItems(_desktop, page)) {
            if (item && !item->isLocked()) {
                addDragShape(item, tr);
            }
        }
    }
}

/**
 * Add an SPItem to the things being dragged.
 */
void PagesTool::addDragShape(SPItem *item, Geom::Affine tr)
{
    if (auto shape = item_to_outline(item)) {
        addDragShape(*shape * item->i2dt_affine(), tr);
    }
}

/**
 * Add a shape to the set of dragging shapes, these are deleted when dragging stops.
 */
void PagesTool::addDragShape(Geom::PathVector &&pth, Geom::Affine tr)
{
    auto shape = new CanvasItemBpath(drag_group, pth * tr, false);
    shape->set_stroke(0x00ff007f);
    shape->set_fill(0x00000000, SP_WIND_RULE_EVENODD);
    drag_shapes.push_back(shape);
}

/**
 * Remove all drag shapes from the canvas.
 */
void PagesTool::clearDragShapes()
{
    for (auto &shape : drag_shapes) {
        delete shape;
    }
    drag_shapes.clear();
}

/**
 * Find a page under the cursor point.
 */
SPPage *PagesTool::pageUnder(Geom::Point pt, bool retain_selected)
{
    auto &pm = _desktop->getDocument()->getPageManager();

    // If the point is still on the selected, favour that one.
    if (auto selected = pm.getSelected()) {
        if (retain_selected && selected->getSensitiveRect().contains(pt)) {
            return selected;
        }
    }
    // This provides a simple way of selecting a page based on their layering
    // Pages which are entirely contained within another are selected before
    // their larger parents.
    SPPage* ret = nullptr;
    for (auto &page : pm.getPages()) {
        auto rect = page->getSensitiveRect();
        // If the point is inside the page boundry
        if (rect.contains(pt)) {
            // If we don't have a page yet, or the new page is inside the old one.
            if (!ret || ret->getSensitiveRect().contains(rect)) {
                ret = page;
            }
        }
    }
    return ret;
}

/**
 * Returns true if the document contains no pages AND the point
 * is within the document viewbox.
 */
bool PagesTool::viewboxUnder(Geom::Point pt)
{
    if (auto document = _desktop->getDocument()) {
        auto rect = document->preferredBounds();
        rect->expandBy(-0.1); // see sp-page getSensitiveRect
        return !document->getPageManager().hasPages() && rect.contains(pt);
    }
    return true;
}

void PagesTool::selectionChanged(SPPage *page)
{
    if (_page_modified_connection) {
        _page_modified_connection.disconnect();
        for (auto knot : resize_knots) {
            knot->hide();
        }
    }

    // Loop existing pages because highlight_item is unsafe.
    for (auto &possible : _desktop->getDocument()->getPageManager().getPages()) {
        if (highlight_item == possible) {
            highlight_item->setSelected(false);
        }
    }
    highlight_item = page;
    if (page) {
        _page_modified_connection = page->connectModified(sigc::mem_fun(*this, &PagesTool::pageModified));
        page->setSelected(true);
        pageModified(page, 0);
    } else {
        // This is for viewBox editng directly. A special extra feature
        if (auto document = _desktop->getDocument()) {
            resizeKnotSet(*(document->preferredBounds()));
        }
    }
}

void PagesTool::pageModified(SPObject *object, guint /*flags*/)
{
    if (auto page = dynamic_cast<SPPage *>(object)) {
        resizeKnotSet(page->getDesktopRect());
    }
}

} // namespace Tools
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
