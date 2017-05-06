/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "svg/svg.h"
#include "xml/repr.h"

#include "svg/stringstream.h"

#include "verbs.h"

#include <glibmm/i18n.h>

#define noLPEREALPARAM_DEBUG

namespace Inkscape {

namespace LivePathEffect {


Parameter::Parameter( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect )
    : param_key(key),
      param_wr(wr),
      param_label(label),
      oncanvas_editable(false),
      widget_is_visible(true),
      param_tooltip(tip),
      param_effect(effect)
{
}

void
Parameter::param_writeToRepr(const char * svgd)
{
    param_effect->upd_params = true;
    param_effect->getRepr()->setAttribute(param_key.c_str(), svgd);
}

void Parameter::writeToSVG(void)
{
    gchar * str = param_getSVGValue();
    param_writeToRepr(str);
    g_free(str);
}

} /* namespace LivePathEffect */
} /* namespace Inkscape */

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
