#include <glibmm/i18n.h>
#include "sp-item.h"
#include "desktop.h"
#include "message-context.h"
#include "message-stack.h"
#include "context-fns.h"

/* FIXME: could probably use a template here */

/**
 *  Check to see if the current layer is both unhidden and unlocked.  If not,
 *  set a message about it on the given context.
 *
 *  \param desktop Desktop.
 *  \param message Message context to put messages on.
 *  \return true if the current layer is both unhidden and unlocked, otherwise false.
 */

bool Inkscape::have_viable_layer(SPDesktop *desktop, MessageContext *message)
{
    SPItem const *layer = SP_ITEM(desktop->currentLayer());
    
    if ( !layer || desktop->itemIsHidden(layer) ) {
            message->flash(Inkscape::ERROR_MESSAGE,
                         _("<b>Current layer is hidden</b>. Unhide it to be able to draw on it."));
            return false;
    }
    
    if ( !layer || layer->isLocked() ) {
            message->flash(Inkscape::ERROR_MESSAGE,
                         _("<b>Current layer is locked</b>. Unlock it to be able to draw on it."));
            return false;
    }

    return true;
}


/**
 *  Check to see if the current layer is both unhidden and unlocked.  If not,
 *  set a message about it on the given context.
 *
 *  \param desktop Desktop.
 *  \param message Message context to put messages on.
 *  \return true if the current layer is both unhidden and unlocked, otherwise false.
 */

bool Inkscape::have_viable_layer(SPDesktop *desktop, MessageStack *message)
{
    SPItem const *layer = SP_ITEM(desktop->currentLayer());
    
    if ( !layer || desktop->itemIsHidden(layer) ) {
            message->flash(Inkscape::WARNING_MESSAGE,
                         _("<b>Current layer is hidden</b>. Unhide it to be able to draw on it."));
            return false;
    }
    
    if ( !layer || layer->isLocked() ) {
            message->flash(Inkscape::WARNING_MESSAGE,
                         _("<b>Current layer is locked</b>. Unlock it to be able to draw on it."));
            return false;
    }

    return true;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
