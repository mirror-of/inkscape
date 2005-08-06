/*
 * Inkscape::XML::SimpleSession - simple session/logging implementation
 *
 * Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#include <glib/gmessages.h>
#include "xml/simple-session.h"
#include "xml/event.h"
#include "xml/event-fns.h"

namespace Inkscape {

namespace XML {

void SimpleSession::beginTransaction() {
    g_assert(!_in_transaction);
    _in_transaction = true;
}

void SimpleSession::rollback() {
    g_assert(_in_transaction);
    _in_transaction = false;
    Event *log = _log_builder.detach();
    sp_repr_undo_log(log);
    sp_repr_free_log(log);
}

void SimpleSession::commit() {
    g_assert(_in_transaction);
    _in_transaction = false;
    _log_builder.discard();
}

Inkscape::XML::Event *SimpleSession::commitUndoable() {
    g_assert(_in_transaction);
    _in_transaction = false;
    return _log_builder.detach();
}

void SimpleSession::notifyChildAdded(Inkscape::XML::Node &parent,
                                     Inkscape::XML::Node &child,
                                     Inkscape::XML::Node *prev)
{
    if (_in_transaction) {
        _log_builder.notifyChildAdded(parent, child, prev);
    }
}

void SimpleSession::notifyChildRemoved(Inkscape::XML::Node &parent,
                                       Inkscape::XML::Node &child,
                                       Inkscape::XML::Node *prev)
{
    if (_in_transaction) {
        _log_builder.notifyChildRemoved(parent, child, prev);
    }
}

void SimpleSession::notifyChildOrderChanged(Inkscape::XML::Node &parent,
                                            Inkscape::XML::Node &child,
                                            Inkscape::XML::Node *old_prev,
                                            Inkscape::XML::Node *new_prev)
{
    if (_in_transaction) {
        _log_builder.notifyChildOrderChanged(parent, child, old_prev, new_prev);
    }
}

void SimpleSession::notifyContentChanged(Inkscape::XML::Node &node,
                                         Util::SharedCStringPtr old_content,
                                         Util::SharedCStringPtr new_content)
{
    if (_in_transaction) {
        _log_builder.notifyContentChanged(node, old_content, new_content);
    }
}

void SimpleSession::notifyAttributeChanged(Inkscape::XML::Node &node,
                                           GQuark name,
                                           Util::SharedCStringPtr old_value,
                                           Util::SharedCStringPtr new_value)
{
    if (_in_transaction) {
        _log_builder.notifyAttributeChanged(node, name, old_value, new_value);
    }
}

}

}

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
