// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * <sodipodi:namedview> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 1999-2013 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "sp-namedview.h"

#include <cstring>
#include <string>

#include <2geom/transforms.h>

#include <gtkmm/window.h>

#include "attributes.h"
#include "conn-avoid-ref.h" // for defaultConnSpacing.
#include "desktop-events.h"
#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "enums.h"
#include "event-log.h"
#include "layer-manager.h"
#include "page-manager.h"
#include "preferences.h"
#include "sp-guide.h"
#include "sp-page.h"
#include "sp-item-group.h"
#include "sp-root.h"

#include "actions/actions-canvas-snapping.h"
#include "display/control/canvas-grid.h"
#include "display/control/canvas-page.h"
#include "svg/svg-color.h"
#include "ui/monitor.h"
#include "ui/widget/canvas.h"
#include "util/units.h"
#include "xml/repr.h"

using Inkscape::DocumentUndo;
using Inkscape::Util::unit_table;

#define DEFAULTGRIDCOLOR 0x3f3fff25
#define DEFAULTGRIDEMPCOLOR 0x3f3fff60
#define DEFAULTGRIDEMPSPACING 5
#define DEFAULTGUIDECOLOR   0x0086e599
#define DEFAULTGUIDEHICOLOR 0xff00007f
#define DEFAULTDESKCOLOR 0xd1d1d1ff

static void sp_namedview_setup_guides(SPNamedView * nv);
static void sp_namedview_lock_guides(SPNamedView * nv);
static void sp_namedview_show_single_guide(SPGuide* guide, bool show);
static void sp_namedview_lock_single_guide(SPGuide* guide, bool show);

SPNamedView::SPNamedView()
    : SPObjectGroup()
    , snap_manager(this, get_snapping_preferences())
    , showguides(true)
    , lockguides(false)
    , grids_visible(false)
    , desk_checkerboard(false)
{
    this->zoom = 0;
    this->guidecolor = 0;
    this->guidehicolor = 0;
    this->views.clear();
    // this->page_size_units = nullptr;
    this->window_x = 0;
    this->cy = 0;
    this->window_y = 0;
    this->display_units = nullptr;
    // this->page_size_units = nullptr;
    this->cx = 0;
    this->rotation = 0;
    this->window_width = 0;
    this->window_height = 0;
    this->window_maximized = 0;
    this->desk_color = DEFAULTDESKCOLOR;

    this->editable = TRUE;
    this->guides.clear();
    this->viewcount = 0;
    this->grids.clear();

    this->default_layer_id = 0;

    this->connector_spacing = defaultConnSpacing;

    this->_viewport = new Inkscape::CanvasPage();
    this->_viewport->hide();
}

SPNamedView::~SPNamedView()
{
    delete _viewport;
}

static void sp_namedview_generate_old_grid(SPNamedView * /*nv*/, SPDocument *document, Inkscape::XML::Node *repr) {
    bool old_grid_settings_present = false;

    // set old settings
    const char* gridspacingx    = "1px";
    const char* gridspacingy    = "1px";
    const char* gridoriginy     = "0px";
    const char* gridoriginx     = "0px";
    const char* gridempspacing  = "5";
    const char* gridcolor       = "#3f3fff";
    const char* gridempcolor    = "#3f3fff";
    const char* gridopacity     = "0.15";
    const char* gridempopacity  = "0.38";

    const char* value = nullptr;
    if ((value = repr->attribute("gridoriginx"))) {
        gridoriginx = value;
        old_grid_settings_present = true;
    }
    if ((value = repr->attribute("gridoriginy"))) {
        gridoriginy = value;
        old_grid_settings_present = true;
    }
    if ((value = repr->attribute("gridspacingx"))) {
        gridspacingx = value;
        old_grid_settings_present = true;
    }
    if ((value = repr->attribute("gridspacingy"))) {
        gridspacingy = value;
        old_grid_settings_present = true;
    }
    if ((value = repr->attribute("gridcolor"))) {
        gridcolor = value;
        old_grid_settings_present = true;
    }
    if ((value = repr->attribute("gridempcolor"))) {
        gridempcolor = value;
        old_grid_settings_present = true;
    }
    if ((value = repr->attribute("gridempspacing"))) {
        gridempspacing = value;
        old_grid_settings_present = true;
    }
    if ((value = repr->attribute("gridopacity"))) {
        gridopacity = value;
        old_grid_settings_present = true;
    }
    if ((value = repr->attribute("gridempopacity"))) {
        gridempopacity = value;
        old_grid_settings_present = true;
    }

    if (old_grid_settings_present) {
        // generate new xy grid with the correct settings
        // first create the child xml node, then hook it to repr. This order is important, to not set off listeners to repr before the new node is complete.

        Inkscape::XML::Document *xml_doc = document->getReprDoc();
        Inkscape::XML::Node *newnode = xml_doc->createElement("inkscape:grid");
        newnode->setAttribute("id", "GridFromPre046Settings");
        newnode->setAttribute("type", Inkscape::CanvasGrid::getSVGName(Inkscape::GRID_RECTANGULAR));
        newnode->setAttribute("originx", gridoriginx);
        newnode->setAttribute("originy", gridoriginy);
        newnode->setAttribute("spacingx", gridspacingx);
        newnode->setAttribute("spacingy", gridspacingy);
        newnode->setAttribute("color", gridcolor);
        newnode->setAttribute("empcolor", gridempcolor);
        newnode->setAttribute("opacity", gridopacity);
        newnode->setAttribute("empopacity", gridempopacity);
        newnode->setAttribute("empspacing", gridempspacing);

        repr->appendChild(newnode);
        Inkscape::GC::release(newnode);

        // remove all old settings
        repr->removeAttribute("gridoriginx");
        repr->removeAttribute("gridoriginy");
        repr->removeAttribute("gridspacingx");
        repr->removeAttribute("gridspacingy");
        repr->removeAttribute("gridcolor");
        repr->removeAttribute("gridempcolor");
        repr->removeAttribute("gridopacity");
        repr->removeAttribute("gridempopacity");
        repr->removeAttribute("gridempspacing");

//        SPDocumentUndo::done(doc, _("Create new grid from pre0.46 grid settings"), "");
    }
}

void SPNamedView::build(SPDocument *document, Inkscape::XML::Node *repr) {
    SPObjectGroup::build(document, repr);

    this->readAttr(SPAttr::INKSCAPE_DOCUMENT_UNITS);
    this->readAttr(SPAttr::UNITS);
    this->readAttr(SPAttr::VIEWONLY);
    this->readAttr(SPAttr::SHOWGUIDES);
    this->readAttr(SPAttr::SHOWGRIDS);
    this->readAttr(SPAttr::GRIDTOLERANCE);
    this->readAttr(SPAttr::GUIDETOLERANCE);
    this->readAttr(SPAttr::OBJECTTOLERANCE);
    this->readAttr(SPAttr::ALIGNMENTTOLERANCE);
    this->readAttr(SPAttr::DISTRIBUTIONTOLERANCE);
    this->readAttr(SPAttr::GUIDECOLOR);
    this->readAttr(SPAttr::GUIDEOPACITY);
    this->readAttr(SPAttr::GUIDEHICOLOR);
    this->readAttr(SPAttr::GUIDEHIOPACITY);
    this->readAttr(SPAttr::SHOWBORDER);
    this->readAttr(SPAttr::SHOWPAGESHADOW);
    this->readAttr(SPAttr::BORDERLAYER);
    this->readAttr(SPAttr::BORDERCOLOR);
    this->readAttr(SPAttr::BORDEROPACITY);
    this->readAttr(SPAttr::PAGECOLOR);
    this->readAttr(SPAttr::INKSCAPE_DESK_COLOR);
    this->readAttr(SPAttr::INKSCAPE_DESK_CHECKERBOARD);
    this->readAttr(SPAttr::INKSCAPE_PAGESHADOW);
    this->readAttr(SPAttr::INKSCAPE_ZOOM);
    this->readAttr(SPAttr::INKSCAPE_ROTATION);
    this->readAttr(SPAttr::INKSCAPE_CX);
    this->readAttr(SPAttr::INKSCAPE_CY);
    this->readAttr(SPAttr::INKSCAPE_WINDOW_WIDTH);
    this->readAttr(SPAttr::INKSCAPE_WINDOW_HEIGHT);
    this->readAttr(SPAttr::INKSCAPE_WINDOW_X);
    this->readAttr(SPAttr::INKSCAPE_WINDOW_Y);
    this->readAttr(SPAttr::INKSCAPE_WINDOW_MAXIMIZED);
    this->readAttr(SPAttr::INKSCAPE_CURRENT_LAYER);
    this->readAttr(SPAttr::INKSCAPE_CONNECTOR_SPACING);
    this->readAttr(SPAttr::INKSCAPE_LOCKGUIDES);

    /* Construct guideline and pages list */
    for (auto &child : children) {
        if (auto guide = dynamic_cast<SPGuide *>(&child)) {
            this->guides.push_back(guide);
            //g_object_set(G_OBJECT(g), "color", nv->guidecolor, "hicolor", nv->guidehicolor, NULL);
            guide->setColor(this->guidecolor);
            guide->setHiColor(this->guidehicolor);
            guide->readAttr(SPAttr::INKSCAPE_COLOR);
        }
        if (auto page = dynamic_cast<SPPage *>(&child)) {
            document->getPageManager().addPage(page);
        }
    }

    // backwards compatibility with grid settings (pre 0.46)
    sp_namedview_generate_old_grid(this, document, repr);
}

void SPNamedView::release() {
    this->guides.clear();

    // delete grids:
    for(auto grid : this->grids)
        delete grid;
    this->grids.clear();
    SPObjectGroup::release();
}

void SPNamedView::set_desk_color(SPDesktop* desktop) {
    if (desktop) {
        if (desk_checkerboard) {
            desktop->getCanvas()->set_background_checkerboard(desk_color);
        } else {
            desktop->getCanvas()->set_background_color(desk_color);
        }
    }
}

void SPNamedView::modified(unsigned int flags)
{
    // Copy the page style for the default viewport attributes
    auto &page_manager = document->getPageManager();
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        page_manager.setDefaultAttributes(_viewport);
        updateViewPort();
        // Pass modifications to the page manager to update the page items.
        for (auto &page : page_manager.getPages()) {
            page->setDefaultAttributes();
        }
    }
    // Add desk color, and chckerboard pattern to desk view
    for (auto desktop : views) {
        set_desk_color(desktop);
    }

    for (auto child : this->childList(false)) {
        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags & SP_OBJECT_MODIFIED_CASCADE);
        }
    }
}

/**
 * Propergate the update to the child nodes so they can be updated correctly.
 */
void SPNamedView::update(SPCtx *ctx, guint flags)
{
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    for (auto child : this->childList(false)) {
        if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->updateDisplay(ctx, flags);
        }
    }
}

void SPNamedView::set(SPAttr key, const gchar* value) {
    // Send page attributes to the page manager.
    if (document->getPageManager().subset(key, value)) {
        this->requestModified(SP_OBJECT_MODIFIED_FLAG);
        return;
    }

    switch (key) {
    case SPAttr::VIEWONLY:
        this->editable = (!value);
        break;
    case SPAttr::SHOWGUIDES:
        this->showguides.readOrUnset(value);
        sp_namedview_setup_guides(this);
        break;
    case SPAttr::INKSCAPE_LOCKGUIDES:
        this->lockguides.readOrUnset(value);
        this->lockGuides();
        break;
    case SPAttr::SHOWGRIDS:
        this->grids_visible.readOrUnset(value);
        break;
    case SPAttr::GRIDTOLERANCE:
        this->snap_manager.snapprefs.setGridTolerance(value ? g_ascii_strtod(value, nullptr) : 10000);
        break;
    case SPAttr::GUIDETOLERANCE:
        this->snap_manager.snapprefs.setGuideTolerance(value ? g_ascii_strtod(value, nullptr) : 20);
        break;
    case SPAttr::OBJECTTOLERANCE:
        this->snap_manager.snapprefs.setObjectTolerance(value ? g_ascii_strtod(value, nullptr) : 20);
        break;
    case SPAttr::ALIGNMENTTOLERANCE:
        this->snap_manager.snapprefs.setAlignementTolerance(value ? g_ascii_strtod(value, nullptr) : 5);
        break;
    case SPAttr::DISTRIBUTIONTOLERANCE:
        this->snap_manager.snapprefs.setDistributionTolerance(value ? g_ascii_strtod(value, nullptr) : 5);
        break;
    case SPAttr::GUIDECOLOR:
        this->guidecolor = (this->guidecolor & 0xff) | (DEFAULTGUIDECOLOR & 0xffffff00);
        if (value) {
            this->guidecolor = (this->guidecolor & 0xff) | sp_svg_read_color(value, this->guidecolor);
        }
        for(auto guide : this->guides) {
            guide->setColor(this->guidecolor);
            guide->readAttr(SPAttr::INKSCAPE_COLOR);
        }
        break;
    case SPAttr::GUIDEOPACITY:
        sp_ink_read_opacity(value, &this->guidecolor, DEFAULTGUIDECOLOR);
        for (auto guide : this->guides) {
            guide->setColor(this->guidecolor);
            guide->readAttr(SPAttr::INKSCAPE_COLOR);
        }
        break;
    case SPAttr::GUIDEHICOLOR:
        this->guidehicolor = (this->guidehicolor & 0xff) | (DEFAULTGUIDEHICOLOR & 0xffffff00);
        if (value) {
            this->guidehicolor = (this->guidehicolor & 0xff) | sp_svg_read_color(value, this->guidehicolor);
        }
        for(auto guide : this->guides) {
            guide->setHiColor(this->guidehicolor);
        }
        break;
    case SPAttr::GUIDEHIOPACITY:
        sp_ink_read_opacity(value, &this->guidehicolor, DEFAULTGUIDEHICOLOR);
        for (auto guide : this->guides) {
            guide->setHiColor(this->guidehicolor);
        }
        break;
    case SPAttr::PAGECOLOR:
        break;
    case SPAttr::INKSCAPE_DESK_COLOR:
        if (value) {
            desk_color = sp_svg_read_color(value, desk_color);
        }
        break;
    case SPAttr::INKSCAPE_DESK_CHECKERBOARD:
        this->desk_checkerboard.readOrUnset(value);
        break;
    case SPAttr::INKSCAPE_ZOOM:
        this->zoom = value ? g_ascii_strtod(value, nullptr) : 0; // zero means not set
        break;
    case SPAttr::INKSCAPE_ROTATION:
        this->rotation = value ? g_ascii_strtod(value, nullptr) : 0; // zero means not set
        break;
    case SPAttr::INKSCAPE_CX:
        this->cx = value ? g_ascii_strtod(value, nullptr) : HUGE_VAL; // HUGE_VAL means not set
        break;
    case SPAttr::INKSCAPE_CY:
        this->cy = value ? g_ascii_strtod(value, nullptr) : HUGE_VAL; // HUGE_VAL means not set
        break;
    case SPAttr::INKSCAPE_WINDOW_WIDTH:
        this->window_width = value? atoi(value) : -1; // -1 means not set
        break;
    case SPAttr::INKSCAPE_WINDOW_HEIGHT:
        this->window_height = value ? atoi(value) : -1; // -1 means not set
        break;
    case SPAttr::INKSCAPE_WINDOW_X:
        this->window_x = value ? atoi(value) : 0;
        break;
    case SPAttr::INKSCAPE_WINDOW_Y:
        this->window_y = value ? atoi(value) : 0;
        break;
    case SPAttr::INKSCAPE_WINDOW_MAXIMIZED:
        this->window_maximized = value ? atoi(value) : 0;
        break;
    case SPAttr::INKSCAPE_CURRENT_LAYER:
        this->default_layer_id = value ? g_quark_from_string(value) : 0;
        break;
    case SPAttr::INKSCAPE_CONNECTOR_SPACING:
        this->connector_spacing = value ? g_ascii_strtod(value, nullptr) : defaultConnSpacing;
        break;
    case SPAttr::INKSCAPE_DOCUMENT_UNITS: {
        /* The default display unit if the document doesn't override this: e.g. for files saved as
            * `plain SVG', or non-inkscape files, or files created by an inkscape 0.40 &
            * earlier.
            *
            * Note that these units are not the same as the units used for the values in SVG!
            *
            * We default to `px'.
            */
        static Inkscape::Util::Unit const *px = unit_table.getUnit("px");
        Inkscape::Util::Unit const *new_unit = px;

        if (value) {
            Inkscape::Util::Unit const *const req_unit = unit_table.getUnit(value);
            if ( !unit_table.hasUnit(value) ) {
                g_warning("Unrecognized unit `%s'", value);
                /* fixme: Document errors should be reported in the status bar or
                    * the like (e.g. as per
                    * http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing); g_log
                    * should be only for programmer errors. */
            } else if ( req_unit->isAbsolute() ) {
                new_unit = req_unit;
            } else {
                g_warning("Document units must be absolute like `mm', `pt' or `px', but found `%s'", value);
                /* fixme: Don't use g_log (see above). */
            }
        }
        this->display_units = new_unit;
        break;
    }
    /*
    case SPAttr::UNITS: {
        // Only used in "Custom size" section of Document Properties dialog
            Inkscape::Util::Unit const *new_unit = nullptr;

            if (value) {
                Inkscape::Util::Unit const *const req_unit = unit_table.getUnit(value);
                if ( !unit_table.hasUnit(value) ) {
                    g_warning("Unrecognized unit `%s'", value);
                    / * fixme: Document errors should be reported in the status bar or
                     * the like (e.g. as per
                     * http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing); g_log
                     * should be only for programmer errors. * /
                } else if ( req_unit->isAbsolute() ) {
                    new_unit = req_unit;
                } else {
                    g_warning("Document units must be absolute like `mm', `pt' or `px', but found `%s'",
                              value);
                    / * fixme: Don't use g_log (see above). * /
                }
            }
            this->page_size_units = new_unit;
            break;
    } */
    default:
        SPObjectGroup::set(key, value);
        return;
    }

    this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
* add a grid item from SVG-repr. Check if this namedview already has a gridobject for this one! If desktop=null, add grid-canvasitem to all desktops of this namedview,
* otherwise only add it to the specified desktop.
*/
static Inkscape::CanvasGrid*
sp_namedview_add_grid(SPNamedView *nv, Inkscape::XML::Node *repr, SPDesktop *desktop) {
    Inkscape::CanvasGrid* grid = nullptr;
    //check if namedview already has an object for this grid
    for(auto it : nv->grids) {
        if (repr == it->repr) {
            grid = it;
            break;
        }
    }

    if (!grid) {
        //create grid object
        Inkscape::GridType gridtype = Inkscape::CanvasGrid::getGridTypeFromSVGName(repr->attribute("type"));
        if (!nv->document) {
            g_warning("sp_namedview_add_grid - how come doc is null here?!");
            return nullptr;
        }
        grid = Inkscape::CanvasGrid::NewGrid(nv, repr, nv->document, gridtype);
        nv->grids.push_back(grid);
    }

    if (!desktop) {
        //add canvasitem to all desktops
        for(auto view : nv->views) {
            grid->createCanvasItem(view);
        }
    } else {
        //add canvasitem only for specified desktop
        grid->createCanvasItem(desktop);
    }

    return grid;
}

/**
 * Update the visibility of the viewport space. This can look like a page
 * if there's no multi-pages, or invisible if it shadows the first page.
 */
void SPNamedView::updateViewPort()
{
    auto box = document->preferredBounds();
    if (auto page = document->getPageManager().getPageAt(box->corner(0))) {
        // An existing page is set as the main page, so hide th viewport canvas item.
        _viewport->hide();
        page->setDesktopRect(*box);
    } else {
        // Otherwise we are showing the viewport item.
        _viewport->show();
        _viewport->update(*box, nullptr, document->getPageManager().hasPages());
    }
}

void SPNamedView::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
    SPObjectGroup::child_added(child, ref);

    SPObject *no = this->document->getObjectByRepr(child);
    if (!no)
        return;

    if (!strcmp(child->name(), "inkscape:grid")) {
        sp_namedview_add_grid(this, child, nullptr);
    } else if (!strcmp(child->name(), "inkscape:page")) {
        if (auto page = dynamic_cast<SPPage *>(no)) {
            document->getPageManager().addPage(page);
            for (auto view : this->views) {
                page->showPage(view->getCanvasPagesBg(), view->getCanvasPagesFg());
            }
        }
    } else {
        if (auto g = dynamic_cast<SPGuide *>(no)) {
            this->guides.push_back(g);

            //g_object_set(G_OBJECT(g), "color", this->guidecolor, "hicolor", this->guidehicolor, NULL);
            g->setColor(this->guidecolor);
            g->setHiColor(this->guidehicolor);
            g->readAttr(SPAttr::INKSCAPE_COLOR);

            if (this->editable) {
                for(auto view : this->views) {
                    g->SPGuide::showSPGuide(view->getCanvasGuides());

                    if (view->guides_active) {
                        g->sensitize(view->getCanvas(), TRUE);
                    }

                    sp_namedview_show_single_guide(g, this->showguides);
                }
            }
        }
    }
}

void SPNamedView::remove_child(Inkscape::XML::Node *child) {
    if (!strcmp(child->name(), "inkscape:grid")) {
        for(std::vector<Inkscape::CanvasGrid *>::iterator it=this->grids.begin();it!=this->grids.end();++it ) {
            if ( (*it)->repr == child ) {
                delete (*it);
                this->grids.erase(it);
                break;
            }
        }
    } else if (!strcmp(child->name(), "inkscape:page")) {
        document->getPageManager().removePage(child);
    } else {
        for(std::vector<SPGuide *>::iterator it=this->guides.begin();it!=this->guides.end();++it ) {
            if ( (*it)->getRepr() == child ) {
                this->guides.erase(it);
                break;
            }
        }
    }

    SPObjectGroup::remove_child(child);
}

void SPNamedView::order_changed(Inkscape::XML::Node *child, Inkscape::XML::Node *old_repr,
                                Inkscape::XML::Node *new_repr)
{
    SPObjectGroup::order_changed(child, old_repr, new_repr);
    if (!strcmp(child->name(), "inkscape:page")) {
        document->getPageManager().reorderPage(child);
    }
}

Inkscape::XML::Node* SPNamedView::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ( ( flags & SP_OBJECT_WRITE_EXT ) &&
         repr != this->getRepr() )
    {
        if (repr) {
            repr->mergeFrom(this->getRepr(), "id");
        } else {
            repr = this->getRepr()->duplicate(xml_doc);
        }
    }

    return repr;
}

void SPNamedView::show(SPDesktop *desktop)
{

    for(auto guide : this->guides) {
        guide->showSPGuide( desktop->getCanvasGuides() );

        if (desktop->guides_active) {
            guide->sensitize(desktop->getCanvas(), TRUE);
        }
        sp_namedview_show_single_guide(guide, showguides);
    }

    auto box = document->preferredBounds();
    _viewport->add(*box, desktop->getCanvasPagesBg(), desktop->getCanvasPagesFg());
    document->getPageManager().setDefaultAttributes(_viewport);
    updateViewPort();

    for (auto page : document->getPageManager().getPages()) {
        page->showPage(desktop->getCanvasPagesBg(), desktop->getCanvasPagesFg());
    }

    views.push_back(desktop);

    // generate grids specified in SVG:
    Inkscape::XML::Node *repr = this->getRepr();
    if (repr) {
        for (Inkscape::XML::Node * child = repr->firstChild() ; child != nullptr; child = child->next() ) {
            if (!strcmp(child->name(), "inkscape:grid")) {
                sp_namedview_add_grid(this, child, desktop);
            }
        }
    }

    desktop->showGrids(grids_visible, false);
}

/*
 * Restores window geometry from the document settings or defaults in prefs
 */
void sp_namedview_window_from_document(SPDesktop *desktop)
{
    SPNamedView *nv = desktop->namedview;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int window_geometry = prefs->getInt("/options/savewindowgeometry/value", PREFS_WINDOW_GEOMETRY_NONE);
    int default_size = prefs->getInt("/options/defaultwindowsize/value", PREFS_WINDOW_SIZE_NATURAL);
    bool new_document = (nv->window_width <= 0) || (nv->window_height <= 0);

    // restore window size and position stored with the document
    Gtk::Window *win = desktop->getToplevel();
    g_assert(win);

    if (window_geometry == PREFS_WINDOW_GEOMETRY_LAST) {
        gint pw = prefs->getInt("/desktop/geometry/width", -1);
        gint ph = prefs->getInt("/desktop/geometry/height", -1);
        gint px = prefs->getInt("/desktop/geometry/x", -1);
        gint py = prefs->getInt("/desktop/geometry/y", -1);
        gint full = prefs->getBool("/desktop/geometry/fullscreen");
        gint maxed = prefs->getBool("/desktop/geometry/maximized");
        if (pw>0 && ph>0) {

            Gdk::Rectangle monitor_geometry = Inkscape::UI::get_monitor_geometry_at_point(px, py);
            pw = std::min(pw, monitor_geometry.get_width());
            ph = std::min(ph, monitor_geometry.get_height());
            desktop->setWindowSize(pw, ph);
            desktop->setWindowPosition(Geom::Point(px, py));
        }
        if (maxed) {
            win->maximize();
        }
        if (full) {
            win->fullscreen();
        }
    } else if ((window_geometry == PREFS_WINDOW_GEOMETRY_FILE && nv->window_maximized) ||
               (new_document && (default_size == PREFS_WINDOW_SIZE_MAXIMIZED))) {
        win->maximize();
    } else {
        const int MIN_WINDOW_SIZE = 600;

        int w = prefs->getInt("/template/base/inkscape:window-width", 0);
        int h = prefs->getInt("/template/base/inkscape:window-height", 0);
        bool move_to_screen = false;
        if (window_geometry == PREFS_WINDOW_GEOMETRY_FILE && !new_document) {
            Gdk::Rectangle monitor_geometry = Inkscape::UI::get_monitor_geometry_at_point(nv->window_x, nv->window_y);
            w = MIN(monitor_geometry.get_width(), nv->window_width);
            h = MIN(monitor_geometry.get_height(), nv->window_height);
            move_to_screen = true;
        } else if (default_size == PREFS_WINDOW_SIZE_LARGE) {
            Gdk::Rectangle monitor_geometry = Inkscape::UI::get_monitor_geometry_at_window(win->get_window());
            w = MAX(0.75 * monitor_geometry.get_width(), MIN_WINDOW_SIZE);
            h = MAX(0.75 * monitor_geometry.get_height(), MIN_WINDOW_SIZE);
        } else if (default_size == PREFS_WINDOW_SIZE_SMALL) {
            w = h = MIN_WINDOW_SIZE;
        } else if (default_size == PREFS_WINDOW_SIZE_NATURAL) {
            // don't set size (i.e. keep the gtk+ default, which will be the natural size)
            // unless gtk+ decided it would be a good idea to show a window that is larger than the screen
            Gdk::Rectangle monitor_geometry = Inkscape::UI::get_monitor_geometry_at_window(win->get_window());
            int monitor_width =  monitor_geometry.get_width();
            int monitor_height = monitor_geometry.get_height();
            int window_width, window_height;
            win->get_size(window_width, window_height);
            if (window_width > monitor_width || window_height > monitor_height) {
                w = std::min(monitor_width, window_width);
                h = std::min(monitor_height, window_height);
            }
        }
        if ((w > 0) && (h > 0)) {
            desktop->setWindowSize(w, h);
            if (move_to_screen) {
                desktop->setWindowPosition(Geom::Point(nv->window_x, nv->window_y));
            }
        }
    }

    // Cancel any history of transforms up to this point (must be before call to zoom).
    desktop->clear_transform_history();
}

/*
 * Restores zoom and view from the document settings
 */
void sp_namedview_zoom_and_view_from_document(SPDesktop *desktop)
{
    SPNamedView *nv = desktop->namedview;
    if (nv->zoom != 0 && nv->zoom != HUGE_VAL && !std::isnan(nv->zoom)
        && nv->cx != HUGE_VAL && !std::isnan(nv->cx)
        && nv->cy != HUGE_VAL && !std::isnan(nv->cy)) {
        desktop->zoom_absolute( Geom::Point(nv->cx, nv->cy), nv->zoom, false );
    } else if (auto document = desktop->getDocument()) {
        // document without saved zoom, zoom to its page
        document->getPageManager().zoomToSelectedPage(desktop);
    }
    if (nv->rotation != 0 && nv->rotation != HUGE_VAL && !std::isnan(nv->rotation)) {
        Geom::Point p;
        if (nv->cx != HUGE_VAL && !std::isnan(nv->cx) && nv->cy != HUGE_VAL && !std::isnan(nv->cy)) {
            p = Geom::Point(nv->cx, nv->cy);
        }else{
            p = desktop->current_center();
        }
        desktop->rotate_absolute_keep_point(p, nv->rotation * M_PI / 180.0);
    }
}

void SPNamedView::writeNewGrid(SPDocument *document,int gridtype)
{
    g_assert(this->getRepr() != nullptr);
    Inkscape::CanvasGrid::writeNewGridToRepr(this->getRepr(),document,static_cast<Inkscape::GridType>(gridtype));
}

void sp_namedview_update_layers_from_document (SPDesktop *desktop)
{
    SPObject *layer = nullptr;
    SPDocument *document = desktop->doc();
    SPNamedView *nv = desktop->namedview;
    if ( nv->default_layer_id != 0 ) {
        layer = document->getObjectById(g_quark_to_string(nv->default_layer_id));
    }
    // don't use that object if it's not at least group
    if ( !layer || !SP_IS_GROUP(layer) ) {
        layer = nullptr;
    }
    // if that didn't work out, look for the topmost layer
    if (!layer) {
        for (auto& iter: document->getRoot()->children) {
            if (desktop->layerManager().isLayer(&iter)) {
                layer = &iter;
            }
        }
    }
    if (layer) {
        desktop->layerManager().setCurrentLayer(layer);
    }

    // FIXME: find a better place to do this
    document->get_event_log()->updateUndoVerbs();
}

void sp_namedview_document_from_window(SPDesktop *desktop)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int window_geometry = prefs->getInt("/options/savewindowgeometry/value", PREFS_WINDOW_GEOMETRY_NONE);
    bool save_geometry_in_file = window_geometry == PREFS_WINDOW_GEOMETRY_FILE;
    bool save_viewport_in_file = prefs->getBool("/options/savedocviewport/value", true);
    Inkscape::XML::Node *view = desktop->namedview->getRepr();

    // saving window geometry is not undoable
    bool saved = DocumentUndo::getUndoSensitive(desktop->getDocument());
    DocumentUndo::setUndoSensitive(desktop->getDocument(), false);

    if (save_viewport_in_file) {
        view->setAttributeSvgDouble("inkscape:zoom", desktop->current_zoom());
        double rotation = ::round(desktop->current_rotation() * 180.0 / M_PI);
        view->setAttributeSvgNonDefaultDouble("inkscape:rotation", rotation, 0.0);
        Geom::Point center = desktop->current_center();
        view->setAttributeSvgDouble("inkscape:cx", center.x());
        view->setAttributeSvgDouble("inkscape:cy", center.y());
    }

    if (save_geometry_in_file) {
        gint w, h, x, y;
        desktop->getWindowGeometry(x, y, w, h);
        view->setAttributeInt("inkscape:window-width", w);
        view->setAttributeInt("inkscape:window-height", h);
        view->setAttributeInt("inkscape:window-x", x);
        view->setAttributeInt("inkscape:window-y", y);
        view->setAttributeInt("inkscape:window-maximized", desktop->is_maximized());
    }

    view->setAttribute("inkscape:current-layer", desktop->layerManager().currentLayer()->getId());

    // restore undoability
    DocumentUndo::setUndoSensitive(desktop->getDocument(), saved);
}

void SPNamedView::hide(SPDesktop const *desktop)
{
    g_assert(desktop != nullptr);
    g_assert(std::find(views.begin(),views.end(),desktop)!=views.end());
    for(auto & guide : this->guides) {
        guide->hideSPGuide(desktop->getCanvas());
    }
    _viewport->remove(desktop->getCanvas());
    for (auto &page : document->getPageManager().getPages()) {
        page->hidePage(desktop->getCanvas());
    }
    views.erase(std::remove(views.begin(),views.end(),desktop),views.end());
}

/**
 * Set an attribute in the named view to the value in this preference, or use the fallback.
 *
 * @param attribute - The svg namedview attribute to set.
 * @param preference - The preference to find the value from (optional)
 * @param fallback - The fallback to use if preference not set or not found. (optional)
 */
void SPNamedView::setDefaultAttribute(std::string attribute, std::string preference, std::string fallback)
{
    if (!getAttribute(attribute.c_str())) {
        std::string value = "";
        if (!preference.empty()) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            value = prefs->getString(preference);
        }
        if (value.empty() && !fallback.empty()) {
            value = fallback;
        }
        if (!value.empty()) {
            setAttribute(attribute, value);
        }
    }
}

void SPNamedView::activateGuides(void* desktop, bool active)
{
    g_assert(desktop != nullptr);
    g_assert(std::find(views.begin(),views.end(),desktop)!=views.end());

    SPDesktop *dt = static_cast<SPDesktop*>(desktop);
    for(auto & guide : this->guides) {
        guide->sensitize(dt->getCanvas(), active);
    }
}

static void sp_namedview_setup_guides(SPNamedView *nv)
{
    for(std::vector<SPGuide *>::iterator it=nv->guides.begin();it!=nv->guides.end();++it ) {
        sp_namedview_show_single_guide(*it, nv->showguides);
    }
}

static void sp_namedview_lock_guides(SPNamedView *nv)
{
    for(std::vector<SPGuide *>::iterator it=nv->guides.begin();it!=nv->guides.end();++it ) {
        sp_namedview_lock_single_guide(*it, nv->lockguides);
    }
}

static void sp_namedview_show_single_guide(SPGuide* guide, bool show)
{
    if (show) {
        guide->showSPGuide();
    } else {
        guide->hideSPGuide();
    }
}

static void sp_namedview_lock_single_guide(SPGuide* guide, bool locked)
{
    guide->set_locked(locked, true);
}

void sp_namedview_toggle_guides(SPDocument *doc, SPNamedView *namedview)
{
    bool saved = DocumentUndo::getUndoSensitive(doc);
    DocumentUndo::setUndoSensitive(doc, false);

    namedview->toggleGuides();

    DocumentUndo::setUndoSensitive(doc, saved);
    doc->setModifiedSinceSave();
}

void sp_namedview_guides_toggle_lock(SPDocument *doc, SPNamedView * namedview)
{
    Inkscape::XML::Node *repr = namedview->getRepr();
    bool v = repr->getAttributeBoolean("inkscape:lockguides", false);
    v = !v;

    bool saved = DocumentUndo::getUndoSensitive(doc);
    DocumentUndo::setUndoSensitive(doc, false);
    repr->setAttributeBoolean("inkscape:lockguides", v);
    sp_namedview_lock_guides(namedview);
    DocumentUndo::setUndoSensitive(doc, saved);
    doc->setModifiedSinceSave();
}

void sp_namedview_show_grids(SPNamedView * namedview, bool show, bool dirty_document)
{
    namedview->grids_visible = show;

    SPDocument *doc = namedview->document;
    Inkscape::XML::Node *repr = namedview->getRepr();

    bool saved = DocumentUndo::getUndoSensitive(doc);
    DocumentUndo::setUndoSensitive(doc, false);
    repr->setAttributeBoolean("showgrid", namedview->grids_visible);
    DocumentUndo::setUndoSensitive(doc, saved);

    /* we don't want the document to get dirty on startup; that's when
       we call this function with dirty_document = false */
    if (dirty_document) {
        doc->setModifiedSinceSave();
    }
}

gchar const *SPNamedView::getName() const
{
    return this->getAttribute("id");
}

std::vector<SPDesktop *> const SPNamedView::getViewList() const
{
    return views;
}

void SPNamedView::toggleGuides()
{
    bool v = this->getGuides();
    this->setGuides(!v);
}

void SPNamedView::setGuides(bool v)
{
    g_assert(this->getRepr() != nullptr);
    this->getRepr()->setAttributeBoolean("showguides", v);
}

bool SPNamedView::getGuides()
{
    g_assert(this->getRepr() != nullptr);
    // show guides if not specified, for backwards compatibility
    return this->getRepr()->getAttributeBoolean("showguides", true);
}

void SPNamedView::lockGuides()
{
    sp_namedview_lock_guides(this);
}

/**
 * Gets page fitting margin information from the namedview node in the XML.
 * \param nv_repr reference to this document's namedview
 * \param key
 * \param margin_units units for the margin
 * \param return_units units to return the result in
 * \param width width in px (for percentage margins)
 * \param height height in px (for percentage margins)
 * \param use_width true if the this key is left or right margins, false
 *        otherwise.  Used for percentage margins.
 * \return the margin size in px, else 0.0 if anything is invalid.
 */
double SPNamedView::getMarginLength(gchar const * const key,
                             Inkscape::Util::Unit const * const margin_units,
                             Inkscape::Util::Unit const * const return_units,
                             double const width,
                             double const height,
                             bool const use_width)
{
    double value;
    static Inkscape::Util::Unit const *percent = unit_table.getUnit("%");
    if(!this->storeAsDouble(key,&value)) {
        return 0.0;
    }
    if (*margin_units == *percent) {
        return (use_width)? width * value : height * value;
    }
    if (!margin_units->compatibleWith(return_units)) {
        return 0.0;
    }
    return value;
}

/**
 * Returns namedview's default unit.
 * If no default unit is set, "px" is returned
 */
Inkscape::Util::Unit const * SPNamedView::getDisplayUnit() const
{
    return display_units ? display_units : unit_table.getUnit("px");
}

/**
 * Set the display unit to the given value.
 */
void SPNamedView::setDisplayUnit(std::string unit)
{
    setDisplayUnit(unit_table.getUnit(unit));
}

void SPNamedView::setDisplayUnit(Inkscape::Util::Unit const *unit)
{
    // If this is unset, it will be returned as px by getDisplayUnit
    display_units = unit;
    getRepr()->setAttributeOrRemoveIfEmpty("inkscape:document-units",
                                           unit ? unit->abbr.c_str() : nullptr);
}

/**
 * Returns the first grid it could find that isEnabled(). Returns NULL, if none is enabled
 */
Inkscape::CanvasGrid * sp_namedview_get_first_enabled_grid(SPNamedView *namedview)
{
    for(auto grid : namedview->grids) {
        if (grid->isEnabled())
            return grid;
    }

    return nullptr;
}

void SPNamedView::translateGuides(Geom::Translate const &tr) {
    for(auto & it : this->guides) {
        SPGuide &guide = *it;
        Geom::Point point_on_line = guide.getPoint();
        point_on_line *= tr;
        guide.moveto(point_on_line, true);
    }
}

void SPNamedView::translateGrids(Geom::Translate const &tr) {
    for(auto & grid : this->grids) {
        grid->setOrigin(grid->origin * tr);
    }
}

void SPNamedView::scrollAllDesktops(double dx, double dy, bool is_scrolling) {
    for(auto & view : this->views) {
        view->scroll_relative_in_svg_coords(dx, dy, is_scrolling);
    }
}

void SPNamedView::change_color(unsigned int rgba, SPAttr color_key, SPAttr opacity_key /*= SPAttr::INVALID*/) {
    gchar buf[32];
    sp_svg_write_color(buf, sizeof(buf), rgba);
    getRepr()->setAttribute(sp_attribute_name(color_key), buf);

    if (opacity_key != SPAttr::INVALID) {
        getRepr()->setAttributeCssDouble(sp_attribute_name(opacity_key), (rgba & 0xff) / 255.0);
    }
}

void SPNamedView::change_bool_setting(SPAttr key, bool value) {
    const char* str_value = nullptr;
    if (key == SPAttr::SHAPE_RENDERING) {
        str_value = value ? "auto" : "crispEdges";
    }
    else {
        str_value = value ? "true" : "false";
    }
    getRepr()->setAttribute(sp_attribute_name(key), str_value);
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
