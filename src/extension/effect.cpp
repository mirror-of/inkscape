/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <helper/action.h>

#include <gtkmm/dialog.h>
#include <gtkmm/socket.h>
#include <gtkmm/stock.h>

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

bool
Effect::prefs (SPView * doc)
{
    if (!loaded())
        set_state(Extension::STATE_LOADED);
    if (!loaded()) return false;

    Gdk::NativeWindow plug;
    plug = imp->prefs_effect(this, doc);
    if (plug == 0) {
        std::cout << "No preferences for Effect" << std::endl;
        return true;
    }

    /* Note: these are pointers with new... the reason for this is
     * because I can do a delete, which deletes them in teh proper order,
     * while the auto-delete in the function does not. */
    Gtk::Dialog * dialog = new Gtk::Dialog("Effect Preferences", true, true);
    Gtk::Socket * socket = new Gtk::Socket();
    dialog->get_vbox()->pack_start(*socket, true, true, 5);
    socket->add_id(plug);
    socket->show();
    dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog->add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    int response = dialog->run();
    dialog->hide();

    delete dialog;

    if (response == Gtk::RESPONSE_OK) return true;

    return false;
}

void
Effect::effect (SPView * doc)
{
    if (!loaded())
        set_state(Extension::STATE_LOADED);
    if (!loaded()) return;

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
//  SPDocument * current_document = SP_VIEW_DOCUMENT(current_view);
    Effect * effect = reinterpret_cast<Effect *>(data);

    if (effect == NULL) return;
    if (current_view == NULL) return;

    std::cout << "Executing: " << effect->get_name() << std::endl;
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
