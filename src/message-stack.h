/*
 * Inkscape::MessageStack - class for mangaging current status messages
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_MESSAGE_STACK_H
#define SEEN_INKSCAPE_MESSAGE_STACK_H

#include <sigc++/sigc++.h>
#include <glib/gtypes.h>
#include <stdarg.h>
#include "managed.h"
#include "message.h"

namespace Inkscape {

class MessageStack : public Managed {
public:
    MessageStack();
    ~MessageStack();

    MessageType currentMessageType() {
        return _messages ? _messages->type : NORMAL_MESSAGE;
    }
    gchar const *currentMessage() {
        return _messages ? _messages->message : NULL;
    }

    SigC::Connection connectChanged(SigC::Slot2<void, MessageType, gchar const *> slot)
    {
        return _changed_signal.connect(slot);
    }

    MessageId push(MessageType type, gchar const *message);
    MessageId pushF(MessageType type, gchar const *message, ...);
    MessageId pushVF(MessageType type, gchar const *message, va_list args);

    void cancel(MessageId id);

    void flash(MessageType type, gchar const *message);
    void flashF(MessageType type, gchar const *format, ...);
    void flashVF(MessageType type, gchar const *format, va_list args);

private:
    struct Message {
        Message *next;
        MessageStack *stack;
        MessageId id;
        MessageType type;
        gchar *message;
        guint timeout_id;
    };

    MessageStack(MessageStack const &); // no copy

    void operator=(MessageStack const &); // no assign

    MessageId _push(MessageType type, guint lifetime, gchar const *message);

    Message *_discard(Message *m);
    void _emitChanged();
    static gboolean _timeout(gpointer data);

    SigC::Signal2<void, MessageType, gchar const *> _changed_signal;
    Message *_messages;
    MessageId _next_id;
};

}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
