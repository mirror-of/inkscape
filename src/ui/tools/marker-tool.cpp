// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Marker edit mode - onCanvas marker editing of marker orientation, position, scale
 *//*
 * Authors:
 * see git history
 * Rachana Podaralla <rpodaralla3@gatech.edu>
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "display/curve.h"

#include "desktop.h"
#include "document.h"
#include "style.h"
#include "message-context.h"
#include "selection.h"

#include "object/sp-path.h"
#include "object/sp-shape.h"
#include "object/sp-marker.h"

#include "ui/shape-editor.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/multi-path-manipulator.h"
#include "ui/tool/path-manipulator.h"
#include "ui/tools/marker-tool.h"


namespace Inkscape {
namespace UI {
namespace Tools {

const std::string& MarkerTool::getPrefsPath() {
	return MarkerTool::prefsPath;
}

const std::string MarkerTool::prefsPath = "/tools/marker";

MarkerTool::MarkerTool()
    :   ToolBase("select.svg")
{
}

MarkerTool::~MarkerTool() {
    this->_shape_editors.clear();
    
    this->enableGrDrag(false);
    this->sel_changed_connection.disconnect();
}

void MarkerTool::finish() {
    ungrabCanvasEvents();

    this->message_context->clear();
    this->sel_changed_connection.disconnect();
    
    ToolBase::finish();
}

void MarkerTool::setup() {

    ToolBase::setup();
    Inkscape::Selection *selection = this->desktop->getSelection();

    this->sel_changed_connection.disconnect();
    this->sel_changed_connection = selection->connectChanged(
    	sigc::mem_fun(this, &MarkerTool::selection_changed)
    );
    this->selection_changed(selection);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/marker/selcue")) this->enableSelectionCue();
    if (prefs->getBool("/tools/marker/gradientdrag")) this->enableGrDrag();

}

/*
- cycles through all the selected items to see if any have a marker in the right location (based on enterMarkerMode)
- if a matching item is found, loads the corresponding marker on the shape into the shape-editor and exits the loop
- forces user to only edit one marker at a time
*/
void MarkerTool::selection_changed(Inkscape::Selection *selection) {
    using namespace Inkscape::UI;

    SPDesktop *desktop = this->desktop;;
    g_assert(desktop != nullptr);

    SPDocument *doc = desktop->getDocument();
    g_assert(doc != nullptr);

    auto selected_items = selection->items();
    this->_shape_editors.clear();

    for(auto i = selected_items.begin(); i != selected_items.end(); ++i){
        SPItem *item = *i;

        if(item) {
            SPShape* shape = dynamic_cast<SPShape*>(item);

            if(shape && shape->hasMarkers() && (editMarkerMode != -1)) {
                SPObject *obj = shape->_marker[editMarkerMode];

                if(obj) {

                    SPMarker *sp_marker = dynamic_cast<SPMarker *>(obj);
                    g_assert(sp_marker != nullptr);

                    sp_validate_marker(sp_marker, doc);

                    ShapeRecord sr;
                    switch(editMarkerMode) {
                        case SP_MARKER_LOC_START:
                            sr  = get_marker_transform(shape, item, sp_marker, SP_MARKER_LOC_START);
                            break;

                        case SP_MARKER_LOC_MID:
                            sr  = get_marker_transform(shape, item, sp_marker, SP_MARKER_LOC_MID);
                            break;

                        case SP_MARKER_LOC_END:
                            sr  = get_marker_transform(shape, item, sp_marker, SP_MARKER_LOC_END);
                            break;

                        default:
                            break;
                    }

                    auto si = std::make_unique<ShapeEditor>(this->desktop, sr.edit_transform, sr.edit_rotation, editMarkerMode);
                    si->set_item(dynamic_cast<SPItem *>(sr.object));

                    this->_shape_editors.insert({item, std::move(si)});
                    break;                     
                }
            }
        }
    }
}

// handles selection of new items
bool MarkerTool::root_handler(GdkEvent* event) {
    SPDesktop *desktop = this->desktop;
    g_assert(desktop != nullptr);

    Inkscape::Selection *selection = desktop->getSelection();
    gint ret = false;
    
    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {

                Geom::Point const button_w(event->button.x, event->button.y);  
                this->item_to_select = sp_event_context_find_item (desktop, button_w, event->button.state & GDK_MOD1_MASK, TRUE);

                grabCanvasEvents();
                ret = true;
            }
            break;
        case GDK_BUTTON_RELEASE:
            if (event->button.button == 1) {

                if (this->item_to_select) {
                    // unselect all items, except for newly selected item
                    selection->set(this->item_to_select);
                } else {
                    // clicked into empty space, deselect any selected items
                    selection->clear();
                }

                this->item_to_select = nullptr;
                ungrabCanvasEvents();
                ret = true;
            }
            break;
        default:
            break;
    }

    return (!ret? ToolBase::root_handler(event): ret);
}

/* 
- this function uses similar logic that exists in sp_shape_update_marker_view
- however, the tangent angle needs to be saved here and parent_item->i2dt_affine() needs to also be accounted for in the right places
- calculate where the shape-editor knotholders need to go based on the reference shape
*/
ShapeRecord MarkerTool::get_marker_transform(SPShape* shape, SPItem *parent_item, SPMarker *sp_marker, SPMarkerLoc marker_type)
{

    // scale marker transform with parent stroke width
    SPStyle *style = shape->style;
    Geom::Scale scale = this->desktop->getDocument()->getDocumentScale(); 
    
    if(sp_marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
        scale = Geom::Scale(style->stroke_width.computed * this->desktop->getDocument()->getDocumentScale()[Geom::X]);
    }

    Geom::PathVector const &pathv = shape->curve()->get_pathvector();
    Geom::Affine ret = Geom::identity(); //edit_transform
    double angle = 0.0; // edit_rotation - tangent angle used for auto orientation
    
    if(marker_type == SP_MARKER_LOC_START) {

        Geom::Curve const &c = pathv.begin()->front();
        Geom::Point p = c.pointAt(0);
        ret = Geom::Translate(p * parent_item->i2dt_affine());

        if (!c.isDegenerate()) {
            Geom::Point tang = c.unitTangentAt(0);
            angle = Geom::atan2(tang);
            ret = Geom::Rotate(angle) * ret;
        }

    } else if(marker_type == SP_MARKER_LOC_MID) {
        /* 
        - a shape can have multiple mid markers - only one is needed
        - once a valid mid marker is found, save edit_transfom and edit_rotation and break out of loop
        */
        for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {

            // mid marker start position
            if (path_it != pathv.begin() && ! ((path_it == (pathv.end()-1)) && (path_it->size_default() == 0)))
            {
                Geom::Curve const &c = path_it->front();
                Geom::Point p = c.pointAt(0);
                ret = Geom::Translate(p * parent_item->i2dt_affine());

                if (!c.isDegenerate()) {
                    Geom::Point tang = c.unitTangentAt(0);
                    angle = Geom::atan2(tang);
                    ret = Geom::Rotate(angle) * ret;
                    break;
                }
            }

            // mid marker mid positions
            if ( path_it->size_default() > 1) {
                Geom::Path::const_iterator curve_it1 = path_it->begin();
                Geom::Path::const_iterator curve_it2 = ++(path_it->begin());
                while (curve_it2 != path_it->end_default())
                {
                    Geom::Curve const & c1 = *curve_it1;
                    Geom::Curve const & c2 = *curve_it2;

                    Geom::Point p = c1.pointAt(1);
                    
                    Geom::Curve * c1_reverse = c1.reverse();
                    Geom::Point tang1 = - c1_reverse->unitTangentAt(0);
                    delete c1_reverse;
                    Geom::Point tang2 = c2.unitTangentAt(0);

                    double const angle1 = Geom::atan2(tang1);
                    double const angle2 = Geom::atan2(tang2);

                    angle = .5 * (angle1 + angle2);

                    if ( fabs( angle2 - angle1 ) > M_PI ) {
                        angle += M_PI;
                    }

                    ret = Geom::Rotate(angle) * Geom::Translate(p * parent_item->i2dt_affine());

                    ++curve_it1;
                    ++curve_it2;
                    break;
                }
            }

            // mid marker end position
            if ( path_it != (pathv.end()-1) && !path_it->empty()) {
                Geom::Curve const &c = path_it->back_default();
                Geom::Point p = c.pointAt(1);
                ret = Geom::Translate(p * parent_item->i2dt_affine());

                if ( !c.isDegenerate() ) {
                    Geom::Curve * c_reverse = c.reverse();
                    Geom::Point tang = - c_reverse->unitTangentAt(0);
                    delete c_reverse;
                    angle = Geom::atan2(tang);
                    ret = Geom::Rotate(angle) * ret;
                    break;
                } 
            }
        }

    } else if (marker_type == SP_MARKER_LOC_END) {

        Geom::Path const &path_last = pathv.back();
        unsigned int index = path_last.size_default();
        if (index > 0) index--;

        Geom::Curve const &c = path_last[index];
        Geom::Point p = c.pointAt(1);
        ret = Geom::Translate(p * parent_item->i2dt_affine());

        if ( !c.isDegenerate() ) {
            Geom::Curve * c_reverse = c.reverse();
            Geom::Point tang = - c_reverse->unitTangentAt(0);
            delete c_reverse;
            angle = Geom::atan2(tang);
            ret = Geom::Rotate(angle) * ret;
        } 
    }

    /* scale by stroke width */
    ret = scale * ret;
    /* account for parent transform */
    ret = parent_item->transform.withoutTranslation() * ret;

    ShapeRecord sr;
    sr.object = sp_marker;
    sr.edit_transform = ret;
    sr.edit_rotation = angle * 180.0/M_PI;
    sr.role = SHAPE_ROLE_NORMAL;
    return sr;
}

}}}
