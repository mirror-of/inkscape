#include "implementation/implementation.h"
#include "effect.h"

/* Inkscape::Extension::Effect */

namespace Inkscape {
namespace Extension {

Effect::Effect (SPRepr * in_repr, Implementation::Implementation * in_imp) : Extension(in_repr, in_imp)
{
    return;
}

Effect::~Effect (void)
{
    return;
}

bool
Effect::check (void)
{
	return Extension::check();
}

GtkDialog *
Effect::prefs (void)
{
    return imp->prefs_effect(this);
}

void
Effect::effect (SPDocument * doc)
{
    return imp->effect(this, doc);
}

}; }; /* namespace Inkscape, Extension */

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
