#define __SP_QUICK_ALIGN_C__

/**
 * \brief  Object align dialog
 *
 * Authors:
 *   Aubanel MONNIER <aubi@libertysurf.fr>
 *   Frank Felfe <innerspace@iname.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <config.h>
#include "helper/sp-intl.h"

#include "dialogs/align.h"
#include "dialogs/docker.h"
#include "dialogs/dockable.h"

#include <gtkmm/buttonbox.h>
#include <gtkmm/button.h>
#include <gtkmm/table.h>
#include <gtkmm/image.h>
#include <gtkmm/frame.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/tooltips.h>

#include <sigc++/sigc++.h>

#include <list>
#include <map>
#include <vector>

#include "libnr/nr-macros.h"
#include "libnr/nr-point-fns.h"
#include "libnr/nr-rect.h"

#include "widgets/icon.h"
#include "sp-item.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "sp-item-transform.h"

#include "node-context.h" //For node align/distribute function
#include "nodepath.h" //For node align/distribute function

#include "tools-switch.h"

/*
 * handler functions for quick align dialog
 *
 * todo: dialog with more aligns
 * - more aligns (30 % left from center ...)
 * - aligns for nodes
 *
 */ 
 
class Action {
public :
    Action(const Glib::ustring &id,
           const Glib::ustring &tiptext,
           guint row, guint column,
           Gtk::Table &parent, 
           Gtk::Tooltips &tooltips):
        _id(id),         
        _parent(parent)
    {
        Gtk::Image*  pImage = Gtk::manage( new Gtk::Image(PixBufFactory::get().getFromSVG(_id)));
	Gtk::Button * pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
	pImage->show();
	pButton->add(*pImage);
	pButton->show();

        pButton->signal_clicked()
            .connect(sigc::mem_fun(*this, &Action::on_button_click));
        tooltips.set_tip(*pButton, tiptext);
	parent.attach(*pButton, column, column+1, row, row+1, Gtk::FILL, Gtk::FILL);
    }
    virtual ~Action(){}
private :    

    virtual void on_button_click(){}

    Glib::ustring _id;    
    Gtk::Table &_parent;
};



class ActionAlign : public Action {
public :
    struct Coeffs {
       double mx0, mx1, my0, my1;
	double sx0, sx1, sy0, sy1; 
    };
    ActionAlign(const Glib::ustring &id,
                const Glib::ustring &tiptext,
                guint row, guint column,
                DialogAlign &dialog, 
                guint coeffIndex):
        Action(id, tiptext, row, column, 
               dialog.align_table(), dialog.tooltips()), 
        _index(coeffIndex), 
        _dialog(dialog)
    {}
    
private :    

    virtual void on_button_click() {
        //Retreive selected objects        
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        if (!desktop) return;

        SPSelection *selection = SP_DT_SELECTION(desktop);
        if (!selection) return;

        std::list<SPItem *> selected;
        selection->list(selected);
        if (selected.empty()) return;

        NR::Point mp; //Anchor point
        DialogAlign::AlignTarget target = _dialog.getAlignTarget();
        const Coeffs &a= _allCoeffs[_index];
        switch (target) 
        {            
        case DialogAlign::LAST:
        case DialogAlign::FIRST:
        case DialogAlign::BIGGEST:        
        case DialogAlign::SMALLEST:
        {
            //Check 2 or more selected objects
            std::list<SPItem *>::iterator second(selected.begin());
            ++second;
            if (second == selected.end())
                return;            
            //Find the master (anchor on which the other objects are aligned)
            std::list<SPItem *>::iterator master(
                _dialog.find_master ( 
                    selected, 
                    (a.mx0 != 0.0) || 
                    (a.mx1 != 0.0) )
                );
            //remove the master from the selection
            selected.erase(master);
            //Compute the anchor point
            NR::Rect b = sp_item_bbox_desktop (*master);
            mp = NR::Point(a.mx0 * b.min()[NR::X] + a.mx1 * b.max()[NR::X],
                           a.my0 * b.min()[NR::Y] + a.my1 * b.max()[NR::Y]);
            break;
        }
            
        case DialogAlign::PAGE:
            mp = NR::Point(a.mx1 * sp_document_width(SP_DT_DOCUMENT(desktop)),
                           a.my1 * sp_document_height(SP_DT_DOCUMENT(desktop)));
            break;
        
        case DialogAlign::DRAWING:
        {
            NR::Rect b = sp_item_bbox_desktop 
                ( (SPItem *) sp_document_root (SP_DT_DOCUMENT (desktop)) );
            mp = NR::Point(a.mx0 * b.min()[NR::X] + a.mx1 * b.max()[NR::X],
                           a.my0 * b.min()[NR::Y] + a.my1 * b.max()[NR::Y]);
            break;
        }

        case DialogAlign::SELECTION:
        {
            NR::Rect b =  selection->bounds();
            mp = NR::Point(a.mx0 * b.min()[NR::X] + a.mx1 * b.max()[NR::X],
                           a.my0 * b.min()[NR::Y] + a.my1 * b.max()[NR::Y]);
            break;
        }

        default:
            g_assert_not_reached ();
            break;
        };  // end of switch

        bool changed = false;
        //Move each item in the selected list
        for (std::list<SPItem *>::iterator it(selected.begin());
             it != selected.end(); 
             it++) 
        {
            NR::Rect b = sp_item_bbox_desktop (*it);
            NR::Point const sp(a.sx0 * b.min()[NR::X] + a.sx1 * b.max()[NR::X],
                               a.sy0 * b.min()[NR::Y] + a.sy1 * b.max()[NR::Y]);
            NR::Point const mp_rel( mp - sp );
            if (LInfty(mp_rel) > 1e-9) {
                sp_item_move_rel(*it, NR::translate(mp_rel));
                changed = true;
            }
        }


        if (changed) {
            sp_document_done ( SP_DT_DOCUMENT (desktop) );
        }
   

    }
    guint _index;
    DialogAlign &_dialog;
    
    static const Coeffs _allCoeffs[10];
    
};
ActionAlign::Coeffs const ActionAlign::_allCoeffs[10] = {
    {1., 0., 0., 0., 0., 1., 0., 0.}, 
    {1., 0., 0., 0., 1., 0., 0., 0.},
    {.5, .5, 0., 0., .5, .5, 0., 0.},
    {0., 1., 0., 0., 0., 1., 0., 0.},
    {0., 1., 0., 0., 1., 0., 0., 0.},
    {0., 0., 0., 1., 0., 0., 1., 0.},
    {0., 0., 0., 1., 0., 0., 0., 1.},
    {0., 0., .5, .5, 0., 0., .5, .5},
    {0., 0., 1., 0., 0., 0., 1., 0.},
    {0., 0., 1., 0., 0., 0., 0., 1.}



};


struct BBoxSort 
{    
    SPItem *item;
    float anchor;
    NR::Rect bbox;
    BBoxSort(SPItem *pItem, NR::Dim2 orientation, double kBegin, double kEnd) : 
        item(pItem), 
        bbox (sp_item_bbox_desktop (pItem))
    {
        anchor = kBegin * bbox.min()[orientation] + kEnd * bbox.max()[orientation];
    }
    BBoxSort(const BBoxSort &rhs):
        //NOTE :  this copy ctor is called O(sort) when sorting the vector
        //this is bad. The vector should be a vector of pointers.
        //But I'll wait the bohem GC before doing that
        item(rhs.item), anchor(rhs.anchor), bbox(rhs.bbox) {
    }
};
bool operator< (const BBoxSort &a, const BBoxSort &b)
{
    return (a.anchor < b.anchor);
}

class ActionDistribute : public Action {
public :
    ActionDistribute(const Glib::ustring &id,
                     const Glib::ustring &tiptext,
                     guint row, guint column,
                     DialogAlign &dialog, 
                     bool onInterSpace,
                     NR::Dim2 orientation,
                     double kBegin, double kEnd
        ):
        Action(id, tiptext, row, column, 
               dialog.ditribute_table(), dialog.tooltips()), 
        _dialog(dialog), 
        _onInterSpace(onInterSpace),
        _orientation(orientation),        
        _kBegin(kBegin), 
        _kEnd( kEnd)
    {}

private :    
    virtual void on_button_click() {
        //Retreive selected objects        
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        if (!desktop) return;

        SPSelection *selection = SP_DT_SELECTION(desktop);
        if (!selection) return;

        std::list<SPItem *> selected;
        selection->list(selected);
        if (selected.empty()) return;

        //Check 2 or more selected objects
        std::list<SPItem *>::iterator second(selected.begin());
        ++second;
        if (second == selected.end()) return;   


        std::vector< BBoxSort  > sorted;
        for (std::list<SPItem *>::iterator it(selected.begin());
            it != selected.end();
            ++it)
        {
            BBoxSort b (*it, _orientation, _kBegin, _kEnd);
            sorted.push_back(b);
        }
        //sort bbox by anchors
        std::sort(sorted.begin(), sorted.end());

        unsigned int len = sorted.size();
        if (_onInterSpace)
        {
            //overall bboxes span
            float dist = (sorted.back().bbox.max()[_orientation] -
                          sorted.front().bbox.min()[_orientation]);
            //space eaten by bboxes
            float span = 0;
            for (unsigned int i = 0; i < len; i++) 
            {
                span += sorted[i].bbox.extent(_orientation);
            }
            //new distance between each bbox
            float step = (dist - span) / (len - 1);
            float pos = sorted.front().bbox.min()[_orientation];
            for ( std::vector<BBoxSort> ::iterator it (sorted.begin());
                  it < sorted.end();
                  it ++ )
            {
                if (!NR_DF_TEST_CLOSE (pos, it->bbox.min()[_orientation], 1e-6)) {
                    NR::Point t(0.0, 0.0);
                    t[_orientation] = pos - it->bbox.min()[_orientation];
                    sp_item_move_rel(it->item, NR::translate(t));
                }
                pos += it->bbox.extent(_orientation);
                pos += step;
            }
        }
        else
        {
            //overall anchor span
            float dist = sorted.back().anchor - sorted.front().anchor;
            //distance between anchors
            float step = dist / (len - 1);

            for ( unsigned int i = 0; i < len ; i ++ )
            {
                BBoxSort & it(sorted[i]);
                //new anchor position
                float pos = sorted.front().anchor + i * step;
                //Don't move if we are really close
                if (!NR_DF_TEST_CLOSE (pos, it.anchor, 1e-6)) {
                    //Compute translation
                    NR::Point t(0.0, 0.0);
                    t[_orientation] = pos - it.anchor;
                    //translate
                    sp_item_move_rel(it.item, NR::translate(t));
                }
            }
        }
    }
    guint _index;
    DialogAlign &_dialog;
    bool _onInterSpace;
    NR::Dim2 _orientation;

    double _kBegin;
    double _kEnd;

};


class ActionNode : public Action {
public :
    ActionNode(const Glib::ustring &id,
               const Glib::ustring &tiptext,
               guint column, 
               DialogAlign &dialog, 
               NR::Dim2 orientation, bool distribute):
        Action(id, tiptext, 0, column, 
               dialog.nodes_table(), dialog.tooltips()), 
        _orientation(orientation),
        _distribute(distribute)
    {}
    
private :    
    NR::Dim2 _orientation;
    bool _distribute;
    virtual void on_button_click() 
    {

        if (!SP_ACTIVE_DESKTOP) return;
	SPEventContext *event_context = (SP_ACTIVE_DESKTOP)->event_context;
	if (!SP_IS_NODE_CONTEXT (event_context)) return ;

        Path::Path *nodepath = SP_NODE_CONTEXT (event_context)->nodepath;
        if (!nodepath) return;
        if (_distribute)
            sp_nodepath_selected_distribute(nodepath, _orientation);
        else
            sp_nodepath_selected_align(nodepath, _orientation);

    }
};


void DialogAlign::on_ref_change(){
//Make blink the master
}

void DialogAlign::on_tool_changed( sp_verb_t verb)
{
    setMode(verb == TOOLS_NODES) ;       
}

void DialogAlign::setMode(bool nodeEdit) 
{
    //Act on widgets used in node mode
    void ( Gtk::Widget::*mNode) ()  = nodeEdit ? 
        &Gtk::Widget::show_all : &Gtk::Widget::hide_all;
    
    //Act on widgets used in selection mode
  void ( Gtk::Widget::*mSel) ()  = nodeEdit ? 
      &Gtk::Widget::hide_all : &Gtk::Widget::show_all;
        
    
    ((_alignFrame).*(mSel))();
    ((_distributeFrame).*(mSel))();
    ((_nodesFrame).*(mNode))();
    
}
void DialogAlign::addAlignButton(const Glib::ustring &id, const Glib::ustring tiptext, 
                                 guint row, guint col)
{
    _actionList.push_back(
        new ActionAlign(
            id, tiptext, row, col, 
            *this , col + row * 5));
}
void DialogAlign::addDistributeButton(const Glib::ustring &id, const Glib::ustring tiptext, 
                                      guint row, guint col, bool onInterSpace, 
                                      NR::Dim2 orientation, float kBegin, float kEnd)
{
    _actionList.push_back(
        new ActionDistribute(
            id, tiptext, row, col, *this ,  
            onInterSpace, orientation, 
            kBegin, kEnd
            )
        );
}
void DialogAlign::addNodeButton(const Glib::ustring &id, const Glib::ustring tiptext, 
                   guint col, NR::Dim2 orientation, bool distribute)
{
    _actionList.push_back(
        new ActionNode(
            id, tiptext, col, 
            *this, orientation, distribute));
}

DialogAlign::DialogAlign():
    Dockable(_("Layout"), "dialogs.align"),
    _alignFrame(_("Align")), 
    _distributeFrame(_("Distribute")),
    _nodesFrame(_("Align Nodes")),
    _alignTable(2,5, true), 
    _distributeTable(2,4, true), 
    _nodesTable(1, 4, true), 
    _anchorLabel(_("Relative to: "))
{
    //Instanciate the align buttons
    addAlignButton("al_left_out", 
                   _("Right side of aligned objects to left side of anchor"), 
                   0, 0); 
    addAlignButton("al_left_in",
                   _("Left side of aligned objects to left side of anchor"), 
                   0, 1); 
    addAlignButton("al_center_hor",
                   _("Center horizontally"), 
                   0, 2); 
    addAlignButton("al_right_in",
                   _("Right side of aligned objects to right side of anchor"), 
                   0, 3); 
    addAlignButton("al_right_out", 
                   _("Left side of aligned objects to right side of anchor"), 
                   0, 4); 
    addAlignButton("al_top_out", 
                   _("Bottom of aligned objects to top of anchor"), 
                   1, 0); 
    addAlignButton("al_top_in", 
                   _("Top of aligned objects to top of anchor"), 
                   1, 1); 
    addAlignButton("al_center_ver", 
                   _("Center vertically"), 
                   1, 2); 
    addAlignButton("al_bottom_in",
                   _("Bottom of aligned objects to bottom of anchor"), 
                   1, 3); 
    addAlignButton("al_bottom_out",
                   _("Top of aligned objects to bottom of anchor"), 
                   1, 4); 
        
    
        
    //The distribute buttons
    addDistributeButton("distribute_left",
                        _("Distribute left sides of objects "
                          "at even distances"), 
                        0, 0, false, NR::X, 1., 0.); 
    addDistributeButton("distribute_hcentre",
                        _("Distribute centers of objects of objects "
                          "at even distances horizontally"),
                        0, 1, false, NR::X, .5, .5); 
    addDistributeButton("distribute_right",
                        _("Distribute right sides of objects at even " 
                          "distances"), 
                        0, 2, false, NR::X, 0., 1.); 
    addDistributeButton("distribute_hdist", 
                        _("Distribute horizontal distance between " 
                          "objects equally"),
                        0, 3, true, NR::X, .5, .5); 
    addDistributeButton("distribute_bottom",
                        _("Distribute bottom sides of objects at even " 
                          "distances"), 
                        1, 0, false, NR::Y, 1., 0.); 
    addDistributeButton("distribute_vcentre",
                        _("Distribute centers of objects at even " 
                          "distances vertically"),
                        1, 1, false, NR::Y, .5, .5); 
    addDistributeButton("distribute_top",
                        _("Distribute top sides of objects at even " 
                          "distances"), 
                        1, 2, false, NR::Y, 0, 1); 
    addDistributeButton("distribute_vdist",
                        _("Distribute vertical distance between objects "
                          "equally"), 
                        1, 3, true, NR::Y, .5, .5); 

    //Node Mode buttons
    addNodeButton("node_halign", 
                  _("Align selected nodes horizontally"), 
                  0, NR::X, false);
    addNodeButton("node_valign", 
                  _("Align selected nodes vertically"), 
                  1, NR::Y, false);
    addNodeButton("node_hdistribute", 
                  _("Distribute selected nodes horizontally"), 
                  2, NR::X, true);
    addNodeButton("node_vdistribute", 
                  _("Distribute selected nodes vertically"), 
                  3, NR::Y, true);
        
    //Rest of the widgetry
        

    _combo.append_text("Last selected");
    _combo.append_text("First selected");
    _combo.append_text("Biggest item");
    _combo.append_text("Smallest item");
    _combo.append_text("Page");
    _combo.append_text("Drawing");
    _combo.append_text("Selection");
        
    _combo.set_active(6);
    _combo.signal_changed().connect(sigc::mem_fun(*this, &DialogAlign::on_ref_change));

    _anchorBox.pack_start(_anchorLabel);
    _anchorBox.pack_start(_combo);

    _alignBox.pack_start(_anchorBox);
    _alignBox.pack_start(_alignTable);

    _alignFrame.add(_alignBox);
    _distributeFrame.add(_distributeTable);
    _nodesFrame.add(_nodesTable);


    _widget.pack_start(_alignFrame, false, false, 0);
    _widget.pack_start(_distributeFrame, false, false, 0);
    _widget.pack_start(_nodesFrame, false, false, 0);

    _widget.show();

    bool in_node_mode = false;
    //Connect to desktop signals
    SPDesktop *pD = SP_ACTIVE_DESKTOP;
    if ( pD )
    {
        pD->_tool_changed.connect(sigc::mem_fun(*this, &DialogAlign::on_tool_changed));
        in_node_mode = tools_isactive (pD, TOOLS_NODES);
    }
    setMode(in_node_mode);
}
    
DialogAlign::~DialogAlign(){
    for (std::list<Action *>::iterator it = _actionList.begin();
         it != _actionList.end();
         it ++)
        delete *it;
};

 
DialogAlign & DialogAlign::get()
{
    static DialogAlign &da = *(new DialogAlign());    
    return da;
}


std::list<SPItem *>::iterator DialogAlign::find_master( std::list<SPItem *> &list, bool horizontal){
    std::list<SPItem *>::iterator master = list.end();
    switch (getAlignTarget()) {
    case LAST:
        return list.begin();
        break;
        
    case FIRST: 
        return --(list.end());
        break;
        
    case BIGGEST:
    {
        gdouble max = -1e18;
        for (std::list<SPItem *>::iterator it = list.begin(); it != list.end(); it++) {
            NR::Rect b = sp_item_bbox_desktop (*it);
            gdouble dim = b.extent(horizontal ? NR::X : NR::Y);
            if (dim > max) {
                max = dim;
                master = it;
            }
        }
        return master;
        break;
    }
        
    case SMALLEST:
    {
        gdouble max = 1e18;
        for (std::list<SPItem *>::iterator it = list.begin(); it != list.end(); it++) {            
            NR::Rect b = sp_item_bbox_desktop (*it);
            gdouble dim = b.extent(horizontal ? NR::X : NR::Y);
            if (dim < max) {
                max = dim;
                master = it;
            }
        }
        return master;
        break;
    }
        
    default:
        g_assert_not_reached ();
        break;
            
    } // end of switch statement
    return NULL;
}

DialogAlign::AlignTarget DialogAlign::getAlignTarget(){
    return AlignTarget(_combo.get_active_row_number());
}
void sp_quick_align_dialog (void)
{
    DialogAlign::get().present();
}
 




#if 0
#include <config.h>

#include <glib.h>
#include <math.h>
#include <stdlib.h>
#include <libnr/nr-macros.h>
#include <gtk/gtksignal.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtklabel.h>

#include "helper/sp-intl.h"
#include "helper/window.h"
#include "widgets/button.h"
#include "widgets/sp-widget.h"
#include "inkscape.h"
#include "document.h"
#include "desktop-handles.h"
#include <sp-item.h>
#include "sp-item-transform.h"
#include "selection.h"
#include "dialog-events.h"
#include "macros.h"
#include <libnr/nr-point-fns.h>

#include "dialog-events.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"

#include "align.h"

/*
 * handler functions for quick align dialog
 *
 * todo: dialog with more aligns
 * - more aligns (30 % left from center ...)
 * - aligns for nodes
 *
 */ 

enum {
    SP_ALIGN_LAST,
    SP_ALIGN_FIRST,
    SP_ALIGN_BIGGEST,
    SP_ALIGN_SMALLEST,
    SP_ALIGN_PAGE,
    SP_ALIGN_DRAWING,
    SP_ALIGN_SELECTION
};

enum align_ixT {
    SP_ALIGN_TOP_IN,
    SP_ALIGN_TOP_OUT,
    SP_ALIGN_RIGHT_IN,
    SP_ALIGN_RIGHT_OUT,
    SP_ALIGN_BOTTOM_IN,
    SP_ALIGN_BOTTOM_OUT,
    SP_ALIGN_LEFT_IN,
    SP_ALIGN_LEFT_OUT,
    SP_ALIGN_CENTER_HOR,
    SP_ALIGN_CENTER_VER
};

enum {
    SP_DISTRIBUTE_LEFT,
    SP_DISTRIBUTE_HCENTRE,
    SP_DISTRIBUTE_RIGHT,
    SP_DISTRIBUTE_HDIST
};

enum {
    SP_DISTRIBUTE_TOP,
    SP_DISTRIBUTE_VCENTRE,
    SP_DISTRIBUTE_BOTTOM,
    SP_DISTRIBUTE_VDIST
};

static struct AlignCoeffs {
    double mx0;
    double mx1;
    double my0;
    double my1;
    double sx0;
    double sx1;
    double sy0;
    double sy1;
} const aligns[10] = {
    {0., 0., 0., 1., 0., 0., 0., 1.},
    {0., 0., 0., 1., 0., 0., 1., 0.},
    {0., 1., 0., 0., 0., 1., 0., 0.},
    {0., 1., 0., 0., 1., 0., 0., 0.},
    {0., 0., 1., 0., 0., 0., 1., 0.},
    {0., 0., 1., 0., 0., 0., 0., 1.},
    {1., 0., 0., 0., 1., 0., 0., 0.},
    {1., 0., 0., 0., 0., 1., 0., 0.},
    {.5, .5, 0., 0., .5, .5, 0., 0.},
    {0., 0., .5, .5, 0., 0., .5, .5}
};

static const gchar hdist[4][3] = {
    {2, 0, 0},
    {1, 1, 0},
    {0, 2, 0},
    {1, 1, 1}
};

static const gchar vdist[4][3] = {
    {0, 2, 0},
    {1, 1, 0},
    {2, 0, 0},
    {1, 1, 1}
};

static void sp_align_arrange_clicked 
(GtkWidget *widget, gconstpointer data);

static void sp_align_distribute_h_clicked 
(GtkWidget *widget, const gchar *layout);

static void sp_align_distribute_v_clicked 
(GtkWidget *widget, const gchar *layout);

static GtkWidget *sp_align_dialog_create_base_menu (void);
static void set_base (GtkMenuItem * menuitem, gpointer data);

static SPItem * sp_quick_align_find_master 
(const GSList * slist, gboolean horizontal);

static GtkWidget *dlg = NULL;
static win_data wd;

/* impossible original values to make sure they are read from prefs */
static gint x = -1000, y = -1000, w = 0, h = 0;
static gchar *prefs_path = "dialogs.align";

static unsigned int base = SP_ALIGN_LAST;



static void
sp_quick_align_dialog_destroy (void)
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);

    wd.win = dlg = NULL;
    wd.stop = 0;
    
} // end of sp_quick_align_dialog_destroy()



static gboolean sp_align_dialog_delete(GtkObject *, GdkEvent *, gpointer data)
{
    gtk_window_get_position(GTK_WINDOW (dlg), &x, &y);
    gtk_window_get_size(GTK_WINDOW (dlg), &w, &h);

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it

} // end of sp_align_dialog_delete()



static void
sp_align_add_button ( GtkWidget *t, int col, int row, 
                      GCallback handler, 
                      gconstpointer data, 
                      const gchar *px, const gchar *tip,
                      GtkTooltips * tt )
{
    GtkWidget *b;
    b = sp_button_new_from_data ( 24, SP_BUTTON_TYPE_NORMAL, NULL, 
                                  px, tip, tt );
    gtk_widget_show (b);
    if (handler) g_signal_connect ( G_OBJECT (b), "clicked", 
                                    handler, (gpointer) data );
    gtk_table_attach ( GTK_TABLE (t), b, col, col + 1, row, row + 1, 
                       (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0 );

} // end of sp_align_add_button()



void
sp_quick_align_dialog (void)
{
    if (!dlg) {
        GtkTooltips * tt = gtk_tooltips_new ();

        gchar title[500];
        sp_ui_dialog_title_string (SP_VERB_DIALOG_ALIGN_DISTRIBUTE, title);

        dlg = sp_window_new (title, TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs_get_int_attribute (prefs_path, "x", 0);
            y = prefs_get_int_attribute (prefs_path, "y", 0);
        }
        
        if (w == 0 || h == 0) {
            w = prefs_get_int_attribute (prefs_path, "w", 0);
            h = prefs_get_int_attribute (prefs_path, "h", 0);
        }
        
        if (x != 0 || y != 0) {
            gtk_window_move ((GtkWindow *) dlg, x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }
        
        if (w && h) {
            gtk_window_resize (GTK_WINDOW (dlg), w, h);
        }
        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;
        
        g_signal_connect (   G_OBJECT (INKSCAPE), "activate_desktop", 
                             G_CALLBACK (sp_transientize_callback), &wd );
        
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", 
                             GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg );
        
        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", 
                             G_CALLBACK (sp_quick_align_dialog_destroy), dlg );
        
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", 
                             G_CALLBACK (sp_align_dialog_delete), dlg );
        
        g_signal_connect (   G_OBJECT (INKSCAPE), "shut_down", 
                             G_CALLBACK (sp_align_dialog_delete), dlg );
        
        g_signal_connect (   G_OBJECT (INKSCAPE), "dialogs_hide", 
                             G_CALLBACK (sp_dialog_hide), dlg);
                             
        g_signal_connect (   G_OBJECT (INKSCAPE), "dialogs_unhide", 
                             G_CALLBACK (sp_dialog_unhide), dlg);

        GtkWidget *nb = gtk_notebook_new ();
        gtk_container_add (GTK_CONTAINER (dlg), nb);

        /* Align */

        GtkWidget *vb = gtk_vbox_new (FALSE, 4);
        gtk_container_set_border_width (GTK_CONTAINER (vb), 4);

        GtkWidget *om = gtk_option_menu_new ();
        gtk_box_pack_start (GTK_BOX (vb), om, FALSE, FALSE, 0);
        gtk_option_menu_set_menu ( GTK_OPTION_MENU (om), 
                                   sp_align_dialog_create_base_menu () );

        GtkWidget *t = gtk_table_new (2, 5, TRUE);
        gtk_box_pack_start (GTK_BOX (vb), t, FALSE, FALSE, 0);

        struct {
        
            int col;
            int row;
            align_ixT ix;
            gchar const *px;
            gchar const *tip;
            
        } const align_buttons[] = {
            {0, 0, SP_ALIGN_LEFT_OUT, "al_left_out", 
             _("Right side of aligned objects to left side of anchor")},
            {1, 0, SP_ALIGN_LEFT_IN, "al_left_in", 
             _("Left side of aligned objects to left side of anchor")},
            {2, 0, SP_ALIGN_CENTER_HOR, "al_center_hor", 
             _("Center horizontally")},
            {3, 0, SP_ALIGN_RIGHT_IN, "al_right_in", 
             _("Right side of aligned objects to right side of anchor")},
            {4, 0, SP_ALIGN_RIGHT_OUT, "al_right_out", 
             _("Left side of aligned objects to right side of anchor")},

            {0, 1, SP_ALIGN_TOP_OUT, "al_top_out", 
             _("Bottom of aligned objects to top of anchor")},
            {1, 1, SP_ALIGN_TOP_IN, "al_top_in", 
             _("Top of aligned objects to top of anchor")},
            {2, 1, SP_ALIGN_CENTER_VER, "al_center_ver", 
             _("Center vertically")},
            {3, 1, SP_ALIGN_BOTTOM_IN, "al_bottom_in", 
             _("Bottom of aligned objects to bottom of anchor")},
            {4, 1, SP_ALIGN_BOTTOM_OUT, "al_bottom_out", 
             _("Top of aligned objects to bottom of anchor")},
                
        };
        
        for (unsigned int i = 0 ; i < G_N_ELEMENTS(align_buttons) ; ++i) {
        
            sp_align_add_button ( t, align_buttons[i].col,
                                  align_buttons[i].row,
                                  G_CALLBACK( sp_align_arrange_clicked ),
                                  &aligns[align_buttons[i].ix],
                                  align_buttons[i].px,
                                  align_buttons[i].tip,
                                  tt );
        }

        GtkWidget *l = gtk_label_new (_("Align"));
        gtk_widget_show (l);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

        /* Distribute */

        vb = gtk_vbox_new (FALSE, 4);
        gtk_container_set_border_width ( GTK_CONTAINER (vb), 4 );

        om = gtk_option_menu_new ();
        gtk_box_pack_start ( GTK_BOX (vb), om, FALSE, FALSE, 0 );
        gtk_option_menu_set_menu ( GTK_OPTION_MENU (om), 
                                   sp_align_dialog_create_base_menu () );
        gtk_widget_set_sensitive ( om, FALSE );

        t = gtk_table_new (2, 4, TRUE);
        gtk_box_pack_start (GTK_BOX (vb), t, FALSE, FALSE, 0);

        sp_align_add_button ( t, 0, 0, 
                              G_CALLBACK (sp_align_distribute_h_clicked), 
                              hdist[SP_DISTRIBUTE_LEFT], "distribute_left",
                              _("Distribute left sides of objects at even " 
                                "distances"),
                              tt );
                              
        sp_align_add_button ( t, 1, 0, 
                              G_CALLBACK (sp_align_distribute_h_clicked), 
                              hdist[SP_DISTRIBUTE_HCENTRE], 
                              "distribute_hcentre",
                              _("Distribute centers of objects at even " 
                                "distances horizontally"),
                              tt );
                              
        sp_align_add_button ( t, 2, 0, 
                              G_CALLBACK (sp_align_distribute_h_clicked), 
                              hdist[SP_DISTRIBUTE_RIGHT], "distribute_right",
                              _("Distribute right sides of objects at even " 
                                "distances"),
                              tt );
                              
        sp_align_add_button ( t, 3, 0, 
                              G_CALLBACK (sp_align_distribute_h_clicked), 
                              hdist[SP_DISTRIBUTE_HDIST], "distribute_hdist",
                              _("Distribute horizontal distance between " 
                                "objects equally"),
                              tt );

        sp_align_add_button ( t, 0, 1, 
                              G_CALLBACK (sp_align_distribute_v_clicked), 
                              vdist[SP_DISTRIBUTE_TOP], "distribute_top",
                              _("Distribute top sides of objects at even " 
                                "distances"),
                              tt );
                              
        sp_align_add_button ( t, 1, 1, 
                              G_CALLBACK (sp_align_distribute_v_clicked), 
                              vdist[SP_DISTRIBUTE_VCENTRE], 
                              "distribute_vcentre",
                              _("Distribute centers of objects at even " 
                                "distances vertically"),
                              tt );
                              
        sp_align_add_button ( t, 2, 1, 
                              G_CALLBACK (sp_align_distribute_v_clicked), 
                              vdist[SP_DISTRIBUTE_BOTTOM], "distribute_bottom",
                              _("Distribute bottom sides of objects at even " 
                                "distances"),
                              tt );
                              
        sp_align_add_button ( t, 3, 1, 
                              G_CALLBACK (sp_align_distribute_v_clicked), 
                              vdist[SP_DISTRIBUTE_VDIST], "distribute_vdist",
                              _("Distribute vertical distance between objects "
                                "equally"),
                              tt );

        l = gtk_label_new (_("Distribute"));
        gtk_widget_show (l);
        gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

        gtk_widget_show_all (nb);
    
    } // end of if (!dlg)

    gtk_window_present ((GtkWindow *) dlg);
    
} // end of sp_quick_align_dialog()



static void
sp_align_add_menuitem ( GtkWidget *menu, const gchar *label, 
                        GCallback handler, int value )
{
    GtkWidget *menuitem = gtk_menu_item_new_with_label (label);
    gtk_widget_show (menuitem);
    
    if (handler) {
        g_signal_connect ( G_OBJECT (menuitem), "activate", 
                           handler, GINT_TO_POINTER (value) );
    }
    
    gtk_menu_append (GTK_MENU (menu), menuitem);

} // end of sp_align_add_menuitem()



static GtkWidget *
sp_align_dialog_create_base_menu (void)
{
    GtkWidget *menu = gtk_menu_new ();

    sp_align_add_menuitem ( menu, _("Last selected"), 
                            G_CALLBACK (set_base), SP_ALIGN_LAST);
                            
    sp_align_add_menuitem ( menu, _("First selected"), 
                            G_CALLBACK (set_base), SP_ALIGN_FIRST);
                            
    sp_align_add_menuitem ( menu, _("Biggest item"), 
                            G_CALLBACK (set_base), SP_ALIGN_BIGGEST);
                            
    sp_align_add_menuitem ( menu, _("Smallest item"), 
                            G_CALLBACK (set_base), SP_ALIGN_SMALLEST);
                            
    sp_align_add_menuitem ( menu, _("Page"), 
                            G_CALLBACK (set_base), SP_ALIGN_PAGE);
                            
    sp_align_add_menuitem ( menu, _("Drawing"), 
                            G_CALLBACK (set_base), SP_ALIGN_DRAWING);
                            
    sp_align_add_menuitem ( menu, _("Selection"), 
                            G_CALLBACK (set_base), SP_ALIGN_SELECTION);

    gtk_widget_show (menu);

    return menu;
    
} // end of sp_align_dialog_create_base_menu()



static void set_base(GtkMenuItem *, gpointer data)
{
    base = GPOINTER_TO_UINT (data);
}



static void sp_align_arrange_clicked(GtkWidget *, gconstpointer data)
{
    AlignCoeffs const &a = *static_cast<AlignCoeffs const *>(data);

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) {
        return;
    }

    SPSelection *selection = SP_DT_SELECTION(desktop);
    GSList *slist = (GSList *) selection->itemList();
    
    if (!slist) {
        return;
    }

    NR::Point mp;
    switch (base) {
    
    case SP_ALIGN_LAST:
    case SP_ALIGN_FIRST:
    case SP_ALIGN_BIGGEST:
        
    case SP_ALIGN_SMALLEST:
    {
        if (!slist->next)
            return;

        slist = g_slist_copy (slist);
        SPItem *master =
            sp_quick_align_find_master ( slist, 
                                         (a.mx0 != 0.0) || (a.mx1 != 0.0) );
        slist = g_slist_remove (slist, master);
        NR::Rect b = sp_item_bbox_desktop (master);
        mp = NR::Point(a.mx0 * b.min()[NR::X] + a.mx1 * b.max()[NR::X],
                       a.my0 * b.min()[NR::Y] + a.my1 * b.max()[NR::Y]);
        break;
    }
            
    case SP_ALIGN_PAGE:
        slist = g_slist_copy (slist);
        mp = NR::Point(a.mx1 * sp_document_width(SP_DT_DOCUMENT(desktop)),
                       a.my1 * sp_document_height(SP_DT_DOCUMENT(desktop)));
        break;
        
    case SP_ALIGN_DRAWING:
    {
        slist = g_slist_copy (slist);
        NR::Rect b = sp_item_bbox_desktop 
            ( (SPItem *) sp_document_root (SP_DT_DOCUMENT (desktop)) );
        mp = NR::Point(a.mx0 * b.min()[NR::X] + a.mx1 * b.max()[NR::X],
                       a.my0 * b.min()[NR::Y] + a.my1 * b.max()[NR::Y]);
        break;
    }

    case SP_ALIGN_SELECTION:
    {
        slist = g_slist_copy (slist);
        NR::Rect b =  selection->bounds();
        mp = NR::Point(a.mx0 * b.min()[NR::X] + a.mx1 * b.max()[NR::X],
                       a.my0 * b.min()[NR::Y] + a.my1 * b.max()[NR::Y]);
        break;
    }

    default:
        g_assert_not_reached ();
        break;
    };  // end of switch

    bool changed = false;
    for (GSList *l = slist; l != NULL; l = l->next) {
        SPItem *item = (SPItem *) l->data;
        NR::Rect b = sp_item_bbox_desktop (item);
        NR::Point const sp(a.sx0 * b.min()[NR::X] + a.sx1 * b.max()[NR::X],
                           a.sy0 * b.min()[NR::Y] + a.sy1 * b.max()[NR::Y]);
        NR::Point const mp_rel( mp - sp );
        if (LInfty(mp_rel) > 1e-9) {
            sp_item_move_rel(item, NR::translate(mp_rel));
            changed = true;
        }
    }

    g_slist_free (slist);

    if (changed) {
        sp_document_done ( SP_DT_DOCUMENT (desktop) );
    }
    
} // end of sp_align_arrange_clicked()



static SPItem *
sp_quick_align_find_master (const GSList *slist, gboolean horizontal)
{
    switch (base) {
    case SP_ALIGN_LAST:
        return (SPItem *) slist->data;
        break;
        
    case SP_ALIGN_FIRST: 
        return (SPItem *) g_slist_last ((GSList *) slist)->data;
        break;
        
    case SP_ALIGN_BIGGEST:
    {
        gdouble max = -1e18;
        SPItem *master = NULL;
        for (const GSList *l = slist; l != NULL; l = l->next) {
            SPItem *item = (SPItem *) l->data;
            NR::Rect b = sp_item_bbox_desktop (item);
            gdouble dim = b.extent(horizontal ? NR::X : NR::Y);
            if (dim > max) {
                max = dim;
                master = item;
            }
        }
        return master;
        break;
    }
        
    case SP_ALIGN_SMALLEST:
    {
        gdouble max = 1e18;
        SPItem *master = NULL;
        for (const GSList *l = slist; l != NULL; l = l->next) {
            SPItem *item = (SPItem *) l->data;
            NR::Rect b = sp_item_bbox_desktop (item);
            gdouble dim = b.extent(horizontal ? NR::X : NR::Y);
            if (dim < max) {
                max = dim;
                master = item;
            }
        }
        return master;
        break;
    }
        
    default:
        g_assert_not_reached ();
        break;
            
    } // end of switch statement

    return NULL;
    
} //end of sp_quick_align_find_master()



struct SPBBoxSort {
    SPItem *item;
    NR::Rect bbox;
    float anchor;
};



static int
sp_align_bbox_sort ( const void *a, const void *b )
{
    const SPBBoxSort *bbsa = (SPBBoxSort *) a;
    const SPBBoxSort *bbsb = (SPBBoxSort *) b;
    
    if (bbsa->anchor < bbsb->anchor) 
        return -1;
    
    if (bbsa->anchor > bbsb->anchor)
        return 1;
    
    return 0;
} // end of sp_align_bbox_sort()


static void sp_align_distribute_h_or_v_clicked(GtkWidget *, gchar const *layout, NR::Dim2 dim)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    
    if (!desktop)
        return;

    const GSList* slist = SP_DT_SELECTION(desktop)->itemList();
    if (!slist)
        return;
    
    if (!slist->next)
        return;
    
//Sort bounding boxes by anchor (ie position on the canvas, horizontally or vertically)
    int len = g_slist_length ((GSList *) slist);
    SPBBoxSort *bbs = g_new (SPBBoxSort, len);
    
    {
        unsigned int pos = 0;
        
        for (const GSList *l = slist; l != NULL; l = l->next) {
            bbs[pos].item = SP_ITEM (l->data);
            bbs[pos].bbox = sp_item_bbox_desktop (bbs[pos].item);
            bbs[pos].anchor =
                0.5 * layout[0] * bbs[pos].bbox.min()[dim] +
                0.5 * layout[1] * bbs[pos].bbox.max()[dim];
            ++pos;
        }
    }
    

    qsort (bbs, len, sizeof (SPBBoxSort), sp_align_bbox_sort);

    bool changed = false;

    if (!layout[2]) {
        //overall anchor span
        float dist = bbs[len - 1].anchor - bbs[0].anchor;
        //distance between anchors
        float step = dist / (len - 1);
        for (int i = 0; i < len; i++) {
            //new anchor position
            float pos = bbs[0].anchor + i * step;
            //Don't move if we are really close
            if (!NR_DF_TEST_CLOSE (pos, bbs[i].anchor, 1e-6)) {
                //Compute translation
                NR::Point t(0.0, 0.0);
                t[dim] = pos - bbs[i].anchor;
                //translate
                sp_item_move_rel(bbs[i].item, NR::translate(t));
                //mark as changed
                changed = true;
            }
        }
    } else {
        /* Damn I am not sure, how to order them initially (Lauris) */
        //overall bbox span
        float dist = (bbs[len - 1].bbox.max()[dim] - bbs[0].bbox.min()[dim]);
        //space eaten by bboxes
        float span = 0;
        for (int i = 0; i < len; i++) {
            span += bbs[i].bbox.extent(dim);
        }
        //new distance between each bbox
        float step = (dist - span) / (len - 1);
        float pos = bbs[0].bbox.min()[dim];
        for (int i = 0; i < len; i++) {
            if (!NR_DF_TEST_CLOSE (pos, bbs[i].bbox.min()[dim], 1e-6)) {
                NR::Point t(0.0, 0.0);
                t[dim] = pos - bbs[i].bbox.min()[dim];
                sp_item_move_rel(bbs[i].item, NR::translate(t));
                changed = true;
            }
            pos += bbs[i].bbox.extent(dim);
            pos += step;
        }
        
    } // end of if (!layout[2])

    g_free (bbs);

    if ( changed ) {
        sp_document_done ( SP_DT_DOCUMENT (desktop) );
    }
    

}

static void sp_align_distribute_h_clicked(GtkWidget *w, gchar const *layout)
{
    sp_align_distribute_h_or_v_clicked(w, layout, NR::X);
}

static void sp_align_distribute_v_clicked(GtkWidget *w, gchar const *layout)
{
    sp_align_distribute_h_or_v_clicked(w, layout, NR::Y);
}


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
#endif
