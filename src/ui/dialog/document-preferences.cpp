/** \file
 *
 * Document preferences dialog, Gtkmm-style
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2000 - 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <utility>  // pair

#include <glibmm/i18n.h>

#include "ui/widget/color-picker.h"
#include "ui/widget/entity-entry.h"
#include "ui/widget/registry.h"
#include "ui/widget/scalar-unit.h"
#include "ui/widget/unit-menu.h"

#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "dialogs/rdf.h"
#include "helper/units.h"
#include "application/editor.h"

#include "inkscape.h"
#include "verbs.h"
#include "document.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "sp-namedview.h"

#include "document-preferences.h"

using std::pair;

namespace Inkscape {
namespace UI {
namespace Dialog {

//===================================================

//---------------------------------------------------

DocumentPreferences *_instance = 0;

static void on_repr_attr_changed (Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer);
static void on_doc_replaced (SPDesktop* dt, SPDocument* doc);
static void on_activate_desktop (Inkscape::Application *, SPDesktop* dt, void*);
static void on_deactivate_desktop (Inkscape::Application *, SPDesktop* dt, void*);

static Inkscape::XML::NodeEventVector const _repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    on_repr_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


DocumentPreferences*
DocumentPreferences::create()
{
    if (_instance) return _instance;
    _instance = new DocumentPreferences;
    _instance->init();
    return _instance;
}

void
DocumentPreferences::destroy()
{
    if (_instance)
    {
        delete _instance;
        _instance = 0;
    }
}

DocumentPreferences::DocumentPreferences() 
    : Dialog ("dialogs.documentoptions", SP_VERB_DIALOG_NAMEDVIEW),
      _page_page(1, 1), _page_grid(1, 1), _page_guides(1, 1),
      _page_snap(1, 1), _page_metadata1(1, 1), _page_metadata2(1, 1),
      _prefs_path("dialogs.documentoptions")
{
    set_resizable (false);
    _tt.enable();
    get_vbox()->set_spacing (4);
    get_vbox()->pack_start (_notebook, true, true);

    _notebook.append_page(_page_page,      _("Page"));
    _notebook.append_page(_page_grid,      _("Grid/Guides"));
    _notebook.append_page(_page_snap,      _("Snap"));
    _notebook.append_page(_page_metadata1, _("Metadata 1"));
    _notebook.append_page(_page_metadata2, _("Metadata 2"));

    build_page();
    build_grid();
    build_snap();
    build_metadata();
}

void
DocumentPreferences::init()
{
    update();

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP));
    repr->addListener (&_repr_events, this);

    _doc_replaced_connection = SP_ACTIVE_DESKTOP->connectDocumentReplaced (sigc::ptr_fun (on_doc_replaced));

    g_signal_connect(G_OBJECT(INKSCAPE), "activate_desktop",
                     G_CALLBACK(on_activate_desktop), 0);
    
    g_signal_connect(G_OBJECT(INKSCAPE), "deactivate_desktop",
                     G_CALLBACK(on_deactivate_desktop), 0);
    
    show_all_children();
    present();
}

DocumentPreferences::~DocumentPreferences() 
{
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP));
    repr->removeListenerByData (this);
    _doc_replaced_connection.disconnect();

    for (RDElist::iterator it = _rdflist.begin(); it != _rdflist.end(); it++)
        delete (*it);
}

//========================================================================

inline void
attach_all (Gtk::Table &table, const Gtk::Widget *arr[], unsigned size, int start = 0)
{
    for (unsigned i=0, r=start; i<size/sizeof(Gtk::Widget*); i+=2)
    {
        if (arr[i])
        {
            table.attach (const_cast<Gtk::Widget&>(*arr[i]),   0, 1, r, r+1, 
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            table.attach (const_cast<Gtk::Widget&>(*arr[i+1]), 1, 2, r, r+1, 
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        }
        else
            table.attach (const_cast<Gtk::Widget&>(*arr[i+1]), 0, 2, r, r+1, 
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        ++r;
    }
}

void
DocumentPreferences::build_page()
{
    _page_page.show();

    _rcp_bg.init (_("Background:"), _("Background color"), _("Color and transparency of the page background (also used for bitmap export)"),
                   "pagecolor", "inkscape:pageopacity", _wr);
    _rcb_canb.init (_("Show page border"), _("If set, rectangular page border is shown"), "showborder", _wr);
    _rcb_bord.init (_("Border on top of drawing"), _("If set, border is always on top of the drawing"), "borderlayer", _wr);
    _rcp_bord.init (_("Border color:"), _("Page border color"),
                    _("Color of the page border"),
                    "bordercolor", "borderopacity", _wr);
    _rcb_shad.init (_("Show page shadow"), "If set, page border shows a shadow on its right and lower side", "inkscape:showpageshadow", _wr);
    _rum_deflt.init (_("Default units:"), "inkscape:document-units", _wr);

    const Gtk::Widget* widget_array[] = 
    {
        _rcp_bg._label, _rcp_bg._cp,
        _rcb_canb._label, _rcb_canb._button,
        _rcb_bord._label, _rcb_bord._button,
        _rcp_bord._label, _rcp_bord._cp,
        _rcb_shad._label, _rcb_shad._button,
        _rum_deflt._label, _rum_deflt._sel,
    };
    
    attach_all (_page_page.table(), widget_array, sizeof(widget_array));

    _page_sizer.init (_wr);
    _page_page.add (_page_sizer);
}

void
DocumentPreferences::build_grid()
{
    _page_grid.show();

    /// \todo FIXME: gray out snapping when grid is off.
    /// Dissenting view: you want snapping without grid.
    
    Gtk::Frame* grid_frame = manage (new Gtk::Frame (_("Grid")));
    _page_grid.table().attach (*grid_frame, 0,2,0,1, Gtk::EXPAND|Gtk::FILL, (Gtk::AttachOptions)0,0,0);
    Gtk::Table* table_grid = manage (new Gtk::Table (9, 2, false));
    grid_frame->add (*table_grid);

    _rcbgrid.init (_("Show grid"), _("Show or hide grid"), "showgrid", _wr);
    _rumg.init (_("Grid units:"), "grid_units", _wr);
    _rsu_ox.init (_("Origin X:"), _("X coordinate of grid origin"), 
                  "gridoriginx", _rumg, _wr);
    _rsu_oy.init (_("Origin Y:"), _("Y coordinate of grid origin"), 
                  "gridoriginy", _rumg, _wr);
    _rsu_sx.init (_("Spacing X:"), _("Distance of vertical grid lines"), 
                  "gridspacingx", _rumg, _wr);
    _rsu_sy.init (_("Spacing Y:"), _("Distance of horizontal grid lines"), 
                  "gridspacingy", _rumg, _wr);
    _rcp_gcol.init (_("Grid line color:"), _("Grid line color"), 
                    _("Color of grid lines"), "gridcolor", "gridopacity", _wr);
    _rcp_gmcol.init (_("Major grid line color:"), _("Major grid line color"), 
                     _("Color of the major (highlighted) grid lines"), 
                     "gridempcolor", "gridempopacity", _wr);
    _rsi.init (_("Major grid line every:"), _("lines"), "gridempspacing", _wr);

    const Gtk::Widget* widget_array[] = 
    {
        0,                  &_rcbgrid.getHBox(),
        _rumg._label,       _rumg._sel,
        0,                  _rsu_ox.getSU(),
        0,                  _rsu_oy.getSU(),
        0,                  _rsu_sx.getSU(),
        0,                  _rsu_sy.getSU(),
        _rcp_gcol._label,   _rcp_gcol._cp, 
        _rcp_gmcol._label,  _rcp_gmcol._cp,
        _rsi._label,        &_rsi._hbox,
    };

    attach_all (*table_grid, widget_array, sizeof(widget_array));

    Gtk::Frame* guide_frame = manage (new Gtk::Frame (_("Guides")));
    _page_grid.table().attach (*guide_frame, 0,2,1,2, Gtk::EXPAND|Gtk::FILL, (Gtk::AttachOptions)0,0,0);
    Gtk::Table* table_guide = manage (new Gtk::Table (3, 2, false));
    guide_frame->add (*table_guide);

    _rcb_sgui.init (_("Show guides"), _("Show or hide guides"), "showguides", _wr);
    _rcp_gui.init (_("Guide color:"), _("Guideline color"), 
                   _("Color of guidelines"), "guidecolor", "guideopacity", _wr);
    _rcp_hgui.init (_("Highlight color:"), _("Highlighted guideline color"), 
                    _("Color of a guideline when it is under mouse"),
                    "guidehicolor", "guidehiopacity", _wr);

    const Gtk::Widget* array[] = 
    {
        0,                &_rcb_sgui.getHBox(),
        _rcp_gui._label, _rcp_gui._cp,
        _rcp_hgui._label, _rcp_hgui._cp,
    };

    attach_all (*table_guide, array, sizeof(array));
}

void
DocumentPreferences::build_snap()
{
    _page_snap.show();

    Gtk::Frame* obj_frame = manage (new Gtk::Frame (_("Object snapping")));
    _page_snap.table().attach (*obj_frame, 0,2,0,1, Gtk::EXPAND|Gtk::FILL, (Gtk::AttachOptions)0,0,0);
    Gtk::Table* table_obj = manage (new Gtk::Table (4, 2, false));
    obj_frame->add (*table_obj);

    _rcbsnbo.init (_("Snap bounding boxes to objects"), 
                _("Snap the edges of the object bounding boxes to other objects"), 
                "inkscape:object-bbox", _wr);
    _rcbsnnob.init (_("Snap nodes to objects"), 
                _("Snap the nodes of objects to other objects"), 
                "inkscape:object-points", _wr);
    _rcbsnop.init (_("Snap to object paths"), 
                _("Snap to other object paths"), 
                "inkscape:object-paths", _wr);
    _rcbsnon.init (_("Snap to object nodes"), 
                _("Snap to other object nodes"), 
                "inkscape:object-nodes", _wr);
    _rumso.init (_("Snap units:"), "object_snap_units", _wr);
    _rsu_sno.init (_("Snap distance:"), 
                  _("Max. snapping distance from object"),
                  "objecttolerance", _rumso, _wr);
    
    const Gtk::Widget* array2[] = 
    {
        0,                  &_rcbsnbo.getHBox(),
        0,                  &_rcbsnnob.getHBox(),
        0,                  &_rcbsnop.getHBox(),
        0,                  &_rcbsnon.getHBox(),
        _rumso._label,      _rumso._sel,
        0,                  _rsu_sno.getSU(),
    };

    attach_all (*table_obj, array2, sizeof(array2));
    
    Gtk::Frame* grid_frame = manage (new Gtk::Frame (_("Grid snapping")));
    _page_snap.table().attach (*grid_frame, 0,2,1,2, Gtk::EXPAND|Gtk::FILL, (Gtk::AttachOptions)0,0,0);
    Gtk::Table* table_grid = manage (new Gtk::Table (4, 2, false));
    grid_frame->add (*table_grid);

    _rcbsnbb.init (_("Snap bounding boxes to grid"), 
                _("Snap the edges of the object bounding boxes"), 
                "inkscape:grid-bbox", _wr);
    _rcbsnnod.init (_("Snap nodes to grid"), 
                _("Snap path nodes, text baselines, ellipse centers, etc."), 
                "inkscape:grid-points", _wr);
    _rums.init (_("Snap units:"), "grid_snap_units", _wr);
    _rsu_sn.init (_("Snap distance:"), 
                  _("Max. snapping distance from grid"),
                  "gridtolerance", _rums, _wr);
    
    const Gtk::Widget* array1[] = 
    {
        0,                  &_rcbsnbb.getHBox(),
        0,                  &_rcbsnnod.getHBox(),
        _rums._label,       _rums._sel,
        0,                  _rsu_sn.getSU(),
    };

    attach_all (*table_grid, array1, sizeof(array1));

   
    Gtk::Frame* gui_frame = manage (new Gtk::Frame (_("Guide snapping")));
    _page_snap.table().attach (*gui_frame, 0,2,2,3, Gtk::EXPAND|Gtk::FILL, (Gtk::AttachOptions)0,0,0);
    Gtk::Table* table_gui = manage (new Gtk::Table (4, 2, false));
    gui_frame->add (*table_gui);

    _rcb_snpgui.init (_("Snap bounding boxes to guides"),  
                     _("Snap the edges of the object bounding boxes"), 
                     "inkscape:guide-bbox", _wr);
    _rcb_snbgui.init (_("Snap points to guides"), 
                _("Snap path nodes, text baselines, ellipse centers, etc."), 
                "inkscape:guide-points", _wr);
    _rum_gusn.init (_("Snap units:"), "guide_snap_units", _wr);
    _rsu_gusn.init (_("Snap distance:"), "", "guidetolerance", _rum_gusn, _wr);

    const Gtk::Widget* widget_array[] = 
    {
        0,                &_rcb_snpgui.getHBox(),
        0,                &_rcb_snbgui.getHBox(),
        _rum_gusn._label, _rum_gusn._sel,
        0,                _rsu_gusn.getSU(),
    };

    attach_all (*table_gui, widget_array, sizeof(widget_array));

 }

void
DocumentPreferences::build_metadata()
{
    _page_metadata1.show();

    /* add generic metadata entry areas */
    struct rdf_work_entity_t * entity;
    int row = 0;
    for (entity = rdf_work_entities; entity && entity->name; entity++, row++) {
        if ( entity->editable == RDF_EDIT_GENERIC ) {
            EntityEntry *w = EntityEntry::create (entity, _tt, _wr);
            _rdflist.push_back (w);
            _page_metadata1.table().attach (w->_label, 0,1, row, row+1, Gtk::SHRINK, (Gtk::AttachOptions)0,0,0);
            _page_metadata1.table().attach (*w->_packable, 1,2, row, row+1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        }
        if (row>=12) break;
    }

    _page_metadata2.show();

    row = 0;
    for (entity++; entity && entity->name; entity++, row++) {
        if ( entity->editable == RDF_EDIT_GENERIC ) {
            EntityEntry *w = EntityEntry::create (entity, _tt, _wr);
            _rdflist.push_back (w);
            _page_metadata2.table().attach (w->_label, 0,1, row, row+1, Gtk::SHRINK, (Gtk::AttachOptions)0,0,0);
            _page_metadata2.table().attach (*w->_packable, 1,2, row, row+1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        }
    }

    /* add license selector pull-down and URI */
    _licensor.init (_tt, _wr);
    _page_metadata2.table().attach (_licensor._frame, 0,2, row, row+1, Gtk::EXPAND|Gtk::FILL, (Gtk::AttachOptions)0,0,0);
}

/**
 * Update dialog widgets from desktop.
 */
void
DocumentPreferences::update()
{
    if (_wr.isUpdating()) return;
    
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    SPNamedView *nv = SP_DT_NAMEDVIEW(dt);
    _wr.setUpdating (true);
    set_sensitive (true);

    //-----------------------------------------------------------page page
    _rcp_bg.setRgba32 (nv->pagecolor);
    _rcb_canb.setActive (nv->showborder);
    _rcb_bord.setActive (nv->borderlayer == SP_BORDER_LAYER_TOP);
    _rcp_bord.setRgba32 (nv->bordercolor);
    _rcb_shad.setActive (nv->showpageshadow);
    
    if (nv->doc_units) 
        _rum_deflt.setUnit (nv->doc_units);

    double const doc_w_px = sp_document_width(SP_DT_DOCUMENT(dt));
    double const doc_h_px = sp_document_height(SP_DT_DOCUMENT(dt));
    _page_sizer.setDim (doc_w_px, doc_h_px);

    //-----------------------------------------------------------grid page
    _rcbgrid.setActive (nv->showgrid);
    _rumg.setUnit (nv->gridunit);
    
    gdouble val;
    val = nv->gridorigin[NR::X];
    val = sp_pixels_get_units (val, *(nv->gridunit));
    _rsu_ox.setValue (val);
    val = nv->gridorigin[NR::Y];
    val = sp_pixels_get_units (val, *(nv->gridunit));
    _rsu_oy.setValue (val);
    val = nv->gridspacing[NR::X];
    val = sp_pixels_get_units (val, *(nv->gridunit));
    _rsu_sx.setValue (val);
    val = nv->gridspacing[NR::Y];
    val = sp_pixels_get_units (val, *(nv->gridunit));
    _rsu_sy.setValue (val);

    _rcp_gcol.setRgba32 (nv->gridcolor);
    _rcp_gmcol.setRgba32 (nv->gridempcolor);
    _rsi.setValue (nv->gridempspacing);

    //-----------------------------------------------------------guide
    _rcb_sgui.setActive (nv->showguides);
    _rcp_gui.setRgba32 (nv->guidecolor);
    _rcp_hgui.setRgba32 (nv->guidehicolor);

    //-----------------------------------------------------------snap
    _rcbsnbo.setActive (nv->object_snapper.getSnapTo(Inkscape::Snapper::BBOX_POINT));
    _rcbsnnob.setActive (nv->object_snapper.getSnapTo(Inkscape::Snapper::SNAP_POINT));
    _rcbsnop.setActive (nv->object_snapper.getSnapToPaths());
    _rcbsnop.setActive (nv->object_snapper.getSnapToNodes());
    _rumso.setUnit (nv->objecttoleranceunit);
    _rsu_sno.setValue (nv->objecttolerance);
     
    _rcbsnbb.setActive (nv->grid_snapper.getSnapTo(Inkscape::Snapper::BBOX_POINT));
    _rcbsnnod.setActive (nv->grid_snapper.getSnapTo(Inkscape::Snapper::SNAP_POINT));
    _rums.setUnit (nv->gridtoleranceunit);
    _rsu_sn.setValue (nv->gridtolerance);
    
     _rcb_snpgui.setActive (nv->guide_snapper.getSnapTo(Inkscape::Snapper::BBOX_POINT));
    _rcb_snbgui.setActive (nv->guide_snapper.getSnapTo(Inkscape::Snapper::SNAP_POINT));
    _rum_gusn.setUnit (nv->guidetoleranceunit);
    _rsu_gusn.setValue (nv->guidetolerance);

    //-----------------------------------------------------------meta pages
    /* load the RDF entities */
    for (RDElist::iterator it = _rdflist.begin(); it != _rdflist.end(); it++)
        (*it)->update (SP_ACTIVE_DOCUMENT);
        
    _licensor.update (SP_ACTIVE_DOCUMENT);

    _wr.setUpdating (false);
}

//--------------------------------------------------------------------

/**
 * Called when XML node attribute changed; updates dialog widgets.
 */
static void
on_repr_attr_changed (Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer)
{
    if (!_instance)
        return;

    _instance->update();
}

static void 
on_activate_desktop (Inkscape::Application *, SPDesktop* dt, void*)
{
    if (!_instance)
        return;

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP));
    repr->addListener (&_repr_events, _instance);
    _instance->_doc_replaced_connection = SP_ACTIVE_DESKTOP->connectDocumentReplaced (sigc::ptr_fun (on_doc_replaced));
    _instance->update();
}

static void 
on_deactivate_desktop (Inkscape::Application *, SPDesktop* dt, void*)
{
    if (!_instance)
        return;

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP));
    repr->removeListenerByData (_instance);
    _instance->_doc_replaced_connection.disconnect();
}

static void 
on_doc_replaced (SPDesktop* dt, SPDocument* doc)
{
    if (!_instance)
        return;

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(dt));
    repr->addListener (&_repr_events, _instance);
    _instance->update();
}


} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
