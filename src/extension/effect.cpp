/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "helper/action.h"
#include "document.h"
#include "prefdialog.h"
#include "implementation/implementation.h"
#include "effect.h"

/* Inkscape::Extension::Effect */

namespace Inkscape {
namespace Extension {

Effect * Effect::_last_effect = NULL;

Effect::Effect (Inkscape::XML::Node * in_repr, Implementation::Implementation * in_imp)
    : Extension(in_repr, in_imp), _verb(get_id(), get_name(), NULL, NULL, this)
{
    return;
}

Effect::~Effect (void)
{
    if (get_last_effect() == this)
        set_last_effect(NULL);
    return;
}

bool
Effect::check (void)
{
    return Extension::check();
}

bool
Effect::prefs (Inkscape::UI::View::View * doc)
{
    if (!loaded())
        set_state(Extension::STATE_LOADED);
    if (!loaded()) return false;

    Gtk::Widget * controls;
    controls = imp->prefs_effect(this, doc);
    if (controls == NULL) {
        // std::cout << "No preferences for Effect" << std::endl;
        return true;
    }

    PrefDialog * dialog = new PrefDialog(this->get_name(), controls);
    int response = dialog->run();
    dialog->hide();

    delete dialog;

    if (response == Gtk::RESPONSE_OK) return true;

    return false;
}

/**
    \brief  The function that 'does' the effect itself
    \param  doc  The Inkscape::UI::View::View to do the effect on

    This function first insures that the extension is loaded, and if not,
    loads it.  It then calls the implemention to do the actual work.  It
    also resets the last effect pointer to be this effect.  Finally, it
    executes a \c sp_document_done to commit the changes to the undo
    stack.
*/
void
Effect::effect (Inkscape::UI::View::View * doc)
{
    if (!loaded())
        set_state(Extension::STATE_LOADED);
    if (!loaded()) return;

    set_last_effect(this);
    imp->effect(this, doc);

    sp_document_done(doc->doc);

    return;
}

void
Effect::set_last_effect (Effect * in_effect)
{
    if (in_effect == NULL) {
        Inkscape::Verb::get(SP_VERB_EFFECT_LAST)->sensitive(NULL, false);
        Inkscape::Verb::get(SP_VERB_EFFECT_LAST_PREF)->sensitive(NULL, false);
    } else if (_last_effect == NULL) {
        Inkscape::Verb::get(SP_VERB_EFFECT_LAST)->sensitive(NULL, true);
        Inkscape::Verb::get(SP_VERB_EFFECT_LAST_PREF)->sensitive(NULL, true);
    }

    _last_effect = in_effect;
    return;
}


/** \brief  Create an action for a \c EffectVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
Effect::EffectVerb::make_action (Inkscape::UI::View::View * view)
{
    return make_action_helper(view, &vector, static_cast<void *>(_effect));
}

/** \brief  Decode the verb code and take appropriate action */
void
Effect::EffectVerb::perform (SPAction *action, void * data, void *pdata)
{
    Inkscape::UI::View::View * current_view = sp_action_get_view(action);
//  SPDocument * current_document = SP_VIEW_DOCUMENT(current_view);
    Effect * effect = reinterpret_cast<Effect *>(data);

    if (effect == NULL) return;
    if (current_view == NULL) return;

    // std::cout << "Executing: " << effect->get_name() << std::endl;
    if (effect->prefs(current_view))
        effect->effect(current_view);

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
