/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <helper/action.h>

#include "implementation/implementation.h"
#include "effect.h"

/* Inkscape::Extension::Effect */

namespace Inkscape {
namespace Extension {

Effect * Effect::_last_effect = NULL;

Effect::Effect (SPRepr * in_repr, Implementation::Implementation * in_imp)
    : Extension(in_repr, in_imp), _verb(get_id(), get_name(), NULL, NULL, this)
{
    return;
}

Effect::~Effect (void)
{
    if (_last_effect == this)
        _last_effect = NULL;
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
    _last_effect = this;
    return imp->prefs_effect(this);
}

void
Effect::effect (SPDocument * doc)
{
    _last_effect = this;
    return imp->effect(this, doc);
}


/** \brief  Create an action for a \c EffectVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
Effect::EffectVerb::make_action (SPView * view)
{
    return make_action_helper(view, &vector, static_cast<void *>(_effect));
}

/** \brief  Decode the verb code and take appropriate action */
void
Effect::EffectVerb::perform (SPAction *action, void * data, void *pdata)
{
    SPView * current_view = sp_action_get_view(action);
    SPDocument * current_document = SP_VIEW_DOCUMENT(current_view);
    Effect * effect = reinterpret_cast<Effect *>(data);

    if (effect == NULL) return;
    if (current_document == NULL) return;

    std::cout << "Executing: " << effect->get_name() << std::endl;

    return;
}

/**
 * Action vector to define functions called if a staticly defined file verb
 * is called.
 */
SPActionEventVector Effect::EffectVerb::vector =
            {{NULL},Effect::EffectVerb::perform, NULL, NULL, NULL};


} }  /* namespace Inkscape, Extension */

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
