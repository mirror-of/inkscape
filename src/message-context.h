/*
 * Inkscape::MessageContext - class for message tracking
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_MESSAGE_CONTEXT_H
#define SEEN_INKSCAPE_MESSAGE_CONTEXT_H

#include <stdarg.h>
#include <glib/gtypes.h>
#include "message.h"

namespace Inkscape {

class MessageStack;

class MessageContext {
public:
    MessageContext(MessageStack *stack);
    ~MessageContext();

    void set(MessageType type, gchar const *message);
    void setF(MessageType type, gchar const *format, ...);
    void setVF(MessageType type, gchar const *format, va_list args);

    void clear();

private:
    MessageStack *_stack;
    MessageId _message_id;
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
