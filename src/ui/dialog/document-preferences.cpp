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
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "dialogs/rdf.h"
#include "application/editor.h"

#include "inkscape.h"
#include "verbs.h"
#include "document.h"
#include "desktop-handles.h"
#include "sp-namedview.h"

#include "document-preferences.h"

using std::pair;

namespace Inkscape {
namespace UI {
namespace Dialog {

//===================================================

//---------------------------------------------------

DocumentPreferences *_instance = 0;

DocumentPreferences*
DocumentPreferences::create()
{
    if (_instance) return _instance;
    _instance = new DocumentPreferences;
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
      _page_page(1, 1), _page_grid(1, 1), 
      _page_guides(1, 1), _page_metadata1(1, 1), _page_metadata2(1, 1),
      _prefs_path("dialogs.documentoptions")
{
    set_resizable (false);
    _tt.enable();
    get_vbox()->set_spacing (4);
    get_vbox()->pack_start (_notebook, true, true);

    _notebook.append_page(_page_page,      _("Page"));
    _notebook.append_page(_page_grid,      _("Grid"));
    _notebook.append_page(_page_guides,    _("Guides"));
    _notebook.append_page(_page_metadata1, _("Metadata 1"));
    _notebook.append_page(_page_metadata2, _("Metadata 2"));

    build_page();
    build_grid();
    build_guides();
    build_metadata();

//sp_dtw_update(dlg, SP_ACTIVE_DESKTOP);

//sp_repr_add_listener(SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP)), &docoptions_repr_events, dlg);

    show_all_children();
    present();
}

DocumentPreferences::~DocumentPreferences() 
{
//    sp_repr_remove_listener_by_data (SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP)), dlg);

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
    _rcb_canb.init (_("Show page border"), "", "showborder", _wr);
    _rcb_bord.init (_("Border on top of drawing"), "", "borderlayer", _wr);
    _rcp_bord.init (_("Border color:"), _("Page border color"),
                    _("Color of the page border"),
                    "bordercolor", "borderopacity", _wr);
    _rcb_shad.init (_("Show page shadow"), "", "inkscape:showpageshadow", _wr);
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
    
    _rcbgrid.init (_("Show grid"), _("Show or hide grid"), "showgrid", _wr);
    _rcbsnbb.init (_("Snap bounding boxes to grid"), 
                _("Snap the edges of the object bounding boxes"), 
                "inkscape:grid-bbox", _wr);
    _rcbsnnod.init (_("Snap nodes to grid"), 
                _("Snap path nodes, text baselines, ellipse centers, etc."), 
                "inkscape:grid-points", _wr);
    _rumg.init (_("Grid units:"), "grid_units", _wr);
    _rsu_ox.init (_("Origin X:"), _("X coordinate of grid origin"), 
                  "gridoriginx", _rumg, _wr);
    _rsu_oy.init (_("Origin Y:"), _("Y coordinate of grid origin"), 
                  "gridoriginy", _rumg, _wr);
    _rsu_sx.init (_("Spacing X:"), _("Distance of vertical grid lines"), 
                  "gridspacingx", _rumg, _wr);
    _rsu_sy.init (_("Spacing Y:"), _("Distance of horizontal grid lines"), 
                  "gridspacingy", _rumg, _wr);
    _rums.init (_("Snap units:"), "grid_snap_units", _wr);
    _rsu_sn.init (_("Snap distance:"), 
                  _("Max. snapping distance from grid"),
                  "gridtolerance", _rums, _wr);
    _rcp_gcol.init (_("Grid line color:"), _("Grid line color"), 
                    _("Color of grid lines"), "gridcolor", "gridopacity", _wr);
    _rcp_gmcol.init (_("Major grid line color:"), _("Major grid line color"), 
                     _("Color of the major (highlighted) grid lines"), 
                     "gridempcolor", "gridempopacity", _wr);
    _rsi.init (_("Major grid line every:"), _("lines"), "gridempspacing", _wr);

    const Gtk::Widget* widget_array[] = 
    {
        0,                  &_rcbgrid.getHBox(),
        0,                  &_rcbsnbb.getHBox(),
        0,                  &_rcbsnnod.getHBox(),
        _rumg._label,       _rumg._sel,
        0,                  _rsu_ox.getSU(),
        0,                  _rsu_oy.getSU(),
        0,                  _rsu_sx.getSU(),
        0,                  _rsu_sy.getSU(),
        _rums._label,       _rums._sel,
        0,                  _rsu_sn.getSU(),
        _rcp_gcol._label,   _rcp_gcol._cp, 
        _rcp_gmcol._label,  _rcp_gmcol._cp,
        _rsi._label,        &_rsi._hbox,
    };

    attach_all (_page_grid.table(), widget_array, sizeof(widget_array));
}

void
DocumentPreferences::build_guides()
{
    _page_guides.show();

    /// \todo FIXME: gray out snapping when guides are off
    /// Dissenting view: you want snapping without guides.

    _rcb_sgui.init (_("Show guides"), _("Show or hide guides"), "showguides", _wr);
    _rcb_snpgui.init (_("Snap bounding boxes to guides"),  
                     _("Snap the edges of the object bounding boxes"), 
                     "inkscape:guide-bbox", _wr);
    _rcb_snbgui.init (_("Snap points to guides"), 
                _("Snap path nodes, text baselines, ellipse centers, etc."), 
                "inkscape:guide-points", _wr);
    _rum_gusn.init (_("Snap units:"), "guide_snap_units", _wr);
    _rsu_gusn.init (_("Snap distance:"), "", "guidetolerance", _rum_gusn, _wr);
    _rcp_gui.init (_("Guide color:"), _("Guideline color"), 
                   _("Color of guidelines"), "guidecolor", "guideopacity", _wr);
    _rcp_hgui.init (_("Highlight color:"), _("Highlighted guideline color"), 
                    _("Color of a guideline when it is under mouse"),
                    "guidehicolor", "guidehiopacity", _wr);

    const Gtk::Widget* widget_array[] = 
    {
        0,                &_rcb_sgui.getHBox(),
        0,                &_rcb_snpgui.getHBox(),
        0,                &_rcb_snbgui.getHBox(),
        _rum_gusn._label, _rum_gusn._sel,
        0,                _rsu_gusn.getSU(),
        _rcp_gui._label, _rcp_gui._cp,
        _rcp_hgui._label, _rcp_hgui._cp,
    };

    attach_all (_page_guides.table(), widget_array, sizeof(widget_array));
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
