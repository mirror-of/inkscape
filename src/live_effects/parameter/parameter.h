#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/ustring.h>
#include <2geom/forward.h>
#include <2geom/pathvector.h>
#include "ui/widget/registered-widget.h"

class KnotHolder;
class SPLPEItem;
class SPDesktop;
class SPItem;

namespace Gtk {
    class Widget;
}

namespace Inkscape {

namespace NodePath {
    class Path ;
}

namespace UI {
namespace Widget {
    class Registry;
}
}

namespace LivePathEffect {

class Effect;

class Parameter {
public:
    Parameter(  const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect);
    virtual ~Parameter() {};


    void writeToSVG();
    virtual Gtk::Widget *      param_newWidget() = 0;
    // Returns true if new value is valid / accepted.
    virtual bool               param_readSVGValue(const gchar * strvalue) = 0;   
    virtual gchar *            param_getSVGValue() const = 0;
    virtual void               param_valueFromDefault(bool write = false) = 0;
    virtual void               param_updateDefault(bool const defaultvalue) = 0;
    virtual void               param_updateDefault(const gchar * default_value) = 0;
    // This creates a new widget (newed with Gtk::manage(new ...);)
    virtual Glib::ustring *    param_getTooltip() { return &param_tooltip; };
    virtual void               param_transformMultiply(Geom::Affine const& /*postmul*/, bool /*set*/) {};

    // Overload these for your particular parameter to make it provide knotholder handles or canvas helperpaths
    virtual bool providesKnotHolderEntities() const { return false; }
    virtual void addKnotHolderEntities(KnotHolder */*knotholder*/, SPItem */*item*/) {};
    virtual void addCanvasIndicators(SPLPEItem const*/*lpeitem*/, std::vector<Geom::PathVector> &/*hp_vec*/) {};
    virtual void param_editOnCanvas(SPItem * /*item*/, SPDesktop * /*dt*/) {};
    virtual void param_setupNodepath(Inkscape::NodePath::Path */*np*/) {};

    Glib::ustring param_key;
    Inkscape::UI::Widget::Registry * param_wr;
    Glib::ustring param_label;
    bool oncanvas_editable;
    bool widget_is_visible;

protected:
    Glib::ustring param_tooltip;
    Effect* param_effect;
    void param_writeToRepr(const char * svgd);

private:
    Parameter(const Parameter&);
    Parameter& operator=(const Parameter&);
};

} //namespace LivePathEffect

} //namespace Inkscape

#endif

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
