#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_POINT_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_POINT_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <2geom/point.h>
#include "ui/widget/registered-widget.h"
#include "live_effects/parameter/parameter.h"

#include "knot-holder-entity.h"

namespace Inkscape {

namespace LivePathEffect {

class PointParamKnotHolderEntity;

class PointParam : public Geom::Point, public Parameter {
public:
    PointParam( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                const gchar *handle_tip = NULL,// tip for automatically associated on-canvas handle
                Geom::Point defaultvalue = Geom::Point(0,0), 
                bool live_update = true );
    virtual ~PointParam();

    virtual Gtk::Widget * param_newWidget();

    bool param_readSVGValue(const gchar * strvalue);
    gchar * param_getSVGValue() const;
    inline const gchar *handleTip() const { return handle_tip ? handle_tip : param_tooltip.c_str(); }
    void param_setValue(Geom::Point newpoint, bool write = false);
    void param_setDefault();
    Geom::Point param_getDefault() const;
    void param_setLiveupdate(bool live_update);
    void param_updateDefault(Geom::Point default_point);

    virtual void param_updateDefault(const gchar * default_point);
    virtual void param_transformMultiply(Geom::Affine const& /*postmul*/, bool /*set*/);

    void set_onCanvasLooks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color);

    virtual bool providesKnotHolderEntities() const { return true; }
    virtual void addKnotHolderEntities(KnotHolder *knotholder, SPItem *item);
    friend class PointParamKnotHolderEntity;
private:
    PointParam(const PointParam&);
    PointParam& operator=(const PointParam&);
    Geom::Point defvalue;
    bool liveupdate;
    KnotHolder *knoth;
    SPKnotShapeType knot_shape;
    SPKnotModeType knot_mode;
    guint32 knot_color;
    gchar *handle_tip;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif
