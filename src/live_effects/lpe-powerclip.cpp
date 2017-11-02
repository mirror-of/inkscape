/*
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "live_effects/lpe-powerclip.h"
#include <2geom/path-intersection.h>
#include <2geom/intersection-graph.h>
#include "display/curve.h"
#include "helper/geom.h"
#include "sp-clippath.h"
#include "sp-path.h"
#include "sp-shape.h"
#include "sp-item-group.h"
#include "ui/tools-switch.h"
#include "path-chemistry.h"
#include "uri.h"
#include "extract-uri.h"
#include <bad-uri-exception.h>

// TODO due to internal breakage in glibmm headers, this must be last:
#include <glibmm/i18n.h>

namespace Inkscape {
namespace LivePathEffect {

LPEPowerClip::LPEPowerClip(LivePathEffectObject *lpeobject)
    : Effect(lpeobject),
    hide_clip(_("Hide clip"), _("Hide clip"), "hide_clip", &wr, this, false),
    is_inverse("Store the last inverse apply", "", "is_inverse", &wr, this, "false", false),
    uri("Store the uri of clip", "", "uri", &wr, this, "false", false),
    inverse(_("Inverse clip"), _("Inverse clip"), "inverse", &wr, this, false),
    flatten(_("Flatten clip"), _("Flatten clip, see fill rule once convert to paths"), "flatten", &wr, this, false)
{
    registerParameter(&uri);
    registerParameter(&inverse);
    registerParameter(&flatten);
    registerParameter(&hide_clip);
    registerParameter(&is_inverse);
    apply_to_clippath_and_mask = true;
}

LPEPowerClip::~LPEPowerClip() {}

void
LPEPowerClip::doBeforeEffect (SPLPEItem const* lpeitem){
    SPObject * clip_path = SP_ITEM(sp_lpe_item)->clip_ref->getObject();
    if(hide_clip && clip_path) {
        SP_ITEM(sp_lpe_item)->clip_ref->detach();
    } else if (!hide_clip && !clip_path && uri.param_getSVGValue()) {
        try {
            SP_ITEM(sp_lpe_item)->clip_ref->attach(Inkscape::URI(uri.param_getSVGValue()));
        } catch (Inkscape::BadURIException &e) {
            g_warning("%s", e.what());
            SP_ITEM(sp_lpe_item)->clip_ref->detach();
        }
    }
    clip_path = SP_ITEM(sp_lpe_item)->clip_ref->getObject();
    if (clip_path) {
        uri.param_setValue(Glib::ustring(extract_uri(sp_lpe_item->getRepr()->attribute("clip-path"))), true);
        SP_ITEM(sp_lpe_item)->clip_ref->detach();
        Geom::OptRect bbox = sp_lpe_item->visualBounds();
        if(!bbox) {
            return;
        }
        if (uri.param_getSVGValue()) {
            try {
                SP_ITEM(sp_lpe_item)->clip_ref->attach(Inkscape::URI(uri.param_getSVGValue()));
            } catch (Inkscape::BadURIException &e) {
                g_warning("%s", e.what());
                SP_ITEM(sp_lpe_item)->clip_ref->detach();
            }
        } else {
            SP_ITEM(sp_lpe_item)->clip_ref->detach();
        }
        Geom::Rect bboxrect = (*bbox);
        bboxrect.expandBy(1);
        Geom::Point topleft      = bboxrect.corner(0);
        Geom::Point topright     = bboxrect.corner(1);
        Geom::Point bottomright  = bboxrect.corner(2);
        Geom::Point bottomleft   = bboxrect.corner(3);
        clip_box.clear();
        clip_box.start(topleft);
        clip_box.appendNew<Geom::LineSegment>(topright);
        clip_box.appendNew<Geom::LineSegment>(bottomright);
        clip_box.appendNew<Geom::LineSegment>(bottomleft);
        clip_box.close();
        std::vector<SPObject*> clip_path_list = clip_path->childList(true);
        for ( std::vector<SPObject*>::const_iterator iter=clip_path_list.begin();iter!=clip_path_list.end();++iter) {
            SPObject * clip_data = *iter;
            if( is_inverse.param_getSVGValue() == (Glib::ustring)"false" && inverse && isVisible()) {
                addInverse(SP_ITEM(clip_data));
            } else if(is_inverse.param_getSVGValue() == (Glib::ustring)"true" && !inverse && isVisible()) {
                removeInverse(SP_ITEM(clip_data));
            } else if (inverse && !is_visible && is_inverse.param_getSVGValue() == (Glib::ustring)"true"){
                removeInverse(SP_ITEM(clip_data));
            }
        }
    }
}

void
LPEPowerClip::addInverse (SPItem * clip_data){
    if(is_inverse.param_getSVGValue() == (Glib::ustring)"false") {
        SPPath *path   = dynamic_cast<SPPath  *>(clip_data);
        SPShape *shape = dynamic_cast<SPShape *>(clip_data);
        SPGroup *group = dynamic_cast<SPGroup *>(clip_data);
        if (group) {
            std::vector<SPItem*> item_list = sp_item_group_item_list(group);
            for ( std::vector<SPItem*>::const_iterator iter=item_list.begin();iter!=item_list.end();++iter) {
                SPItem *subitem = *iter;
                addInverse(subitem);
            }
        } else if (shape) {
            SPCurve * c = NULL;
            c = shape->getCurve();
            if (c) {
                Geom::PathVector c_pv = c->get_pathvector();
                //TODO: this can be not correct but no better way
                bool dir_a = Geom::path_direction(c_pv[0]);
                bool dir_b = Geom::path_direction(clip_box);
                if (dir_a == dir_b) {
                   clip_box = clip_box.reversed();
                }
                c_pv.push_back(clip_box);
                c->set_pathvector(c_pv);
                if (!path) {
                    shape->setCurveInsync( shape->getCurveBeforeLPE(), TRUE);
                }
                shape->setCurve(c, TRUE);
                if (!path) {
                    shape->setCurveInsync( c, TRUE);
                }
                c->unref();
                is_inverse.param_setValue((Glib::ustring)"true", true);
                SPDesktop *desktop = SP_ACTIVE_DESKTOP;
                if (desktop) {
                    if (tools_isactive(desktop, TOOLS_NODES)) {
                        Inkscape::Selection * sel = SP_ACTIVE_DESKTOP->getSelection();
                        SPItem * item = sel->singleItem();
                        if (item != NULL) {
                            sel->remove(item);
                            sel->add(item);
                        }
                    }
                }
            }
        }
    }
}

void
LPEPowerClip::removeInverse (SPItem * clip_data){
    if(is_inverse.param_getSVGValue() == (Glib::ustring)"true") {
        SPPath *path   = dynamic_cast<SPPath  *>(clip_data);
        SPShape *shape = dynamic_cast<SPShape *>(clip_data);
        SPGroup *group = dynamic_cast<SPGroup *>(clip_data);
        if (group) {
             std::vector<SPItem*> item_list = sp_item_group_item_list(group);
             for ( std::vector<SPItem*>::const_iterator iter=item_list.begin();iter!=item_list.end();++iter) {
                 SPItem *subitem = *iter;
                 removeInverse(subitem);
             }
        } else if (shape) {
            SPCurve * c = NULL;
            c = shape->getCurve();
            if (c) {
                Geom::PathVector c_pv = c->get_pathvector();
                if(c_pv.size() > 1) {
                    c_pv.pop_back();
                }
                c->set_pathvector(c_pv);
                if (!path) {
                    shape->setCurveInsync( shape->getCurveBeforeLPE(), TRUE);
                }
                shape->setCurve(c, TRUE);
                if (!path) {
                    shape->setCurveInsync( c, TRUE);
                }
                c->unref();
                is_inverse.param_setValue((Glib::ustring)"false", true);
                SPDesktop *desktop = SP_ACTIVE_DESKTOP;
                if (desktop) {
                    if (tools_isactive(desktop, TOOLS_NODES)) {
                        Inkscape::Selection * sel = SP_ACTIVE_DESKTOP->getSelection();
                        SPItem * item = sel->singleItem();
                        if (item != NULL) {
                            sel->remove(item);
                            sel->add(item);
                        }
                    }
                }
            }
        }
    }
}

void 
LPEPowerClip::doOnRemove (SPLPEItem const* /*lpeitem*/)
{
    SPClipPath *clip_path = SP_ITEM(sp_lpe_item)->clip_ref->getObject();
    if(!keep_paths) {
        if(clip_path) {
            std::vector<SPObject*> clip_path_list = clip_path->childList(true);
            for ( std::vector<SPObject*>::const_iterator iter=clip_path_list.begin();iter!=clip_path_list.end();++iter) {
                SPObject * clip_data = *iter;
                if(is_inverse.param_getSVGValue() == (Glib::ustring)"true") {
                    removeInverse(SP_ITEM(clip_data));
                }
            }
        }
    } else {
        if (flatten && clip_path) {
            clip_path->deleteObject();
        }
    }
}

Geom::PathVector
LPEPowerClip::doEffect_path(Geom::PathVector const & path_in){
    Geom::PathVector path_out = pathv_to_linear_and_cubic_beziers(path_in);
    if (!hide_clip && flatten && isVisible()) {
        SPClipPath *clip_path = SP_ITEM(sp_lpe_item)->clip_ref->getObject();
        if(clip_path) {
            std::vector<SPObject*> clip_path_list = clip_path->childList(true);
            for ( std::vector<SPObject*>::const_iterator iter=clip_path_list.begin();iter!=clip_path_list.end();++iter) {
                SPObject * clip_data = *iter;
                flattenClip(SP_ITEM(clip_data), path_out);
            }
        }
        SP_ITEM(sp_lpe_item)->clip_ref->detach();
    }
    return path_out;
}

void 
LPEPowerClip::doOnVisibilityToggled(SPLPEItem const* lpeitem)
{
    doBeforeEffect(lpeitem);
}


void
LPEPowerClip::flattenClip(SPItem * clip_data, Geom::PathVector &path_in)
{
    SPShape *shape = dynamic_cast<SPShape *>(clip_data);
    SPGroup *group = dynamic_cast<SPGroup *>(clip_data);
    if (group) {
         std::vector<SPItem*> item_list = sp_item_group_item_list(group);
         for ( std::vector<SPItem*>::const_iterator iter=item_list.begin();iter!=item_list.end();++iter) {
             SPItem *subitem = *iter;
             flattenClip(subitem, path_in);
         }
    } else if (shape) {
        SPCurve * c = NULL;
        c = shape->getCurve();
        if (c) {
            Geom::PathVector c_pv = c->get_pathvector();
            Geom::PathIntersectionGraph *pig = new Geom::PathIntersectionGraph(c_pv, path_in);
            if (pig && !c_pv.empty() && !path_in.empty()) {
                path_in = pig->getIntersection();
            }
            c->unref();
        }
    }
}

}; //namespace LivePathEffect
}; /* namespace Inkscape */

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
