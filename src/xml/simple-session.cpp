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
#include "xml/sp-repr-action.h"
#include "xml/sp-repr-action-fns.h"

namespace Inkscape {

namespace XML {

void SimpleSession::beginTransaction() {
    g_assert(!_in_transaction);
    _in_transaction = true;
}

void SimpleSession::rollback() {
    g_assert(_in_transaction);
    _in_transaction = false;
    sp_repr_undo_log(_log);
    sp_repr_free_log(_log);
    _log = NULL;
}

void SimpleSession::commit() {
    g_assert(_in_transaction);
    _in_transaction = false;
    sp_repr_free_log(_log);
    _log = NULL;
}

SPReprAction *SimpleSession::commitUndoable() {
    g_assert(_in_transaction);
    _in_transaction = false;
    SPReprAction *log=_log;
    _log = NULL;
    return log;
}

void SimpleSession::notifyChildAdded(SPRepr &parent, SPRepr &child,
                                     SPRepr *prev)
{
    if (_in_transaction) {
        _log = new SPReprActionAdd(&parent, &child, prev, _log);
        _log = _log->optimizeOne();
    }
}

void SimpleSession::notifyChildRemoved(SPRepr &parent, SPRepr &child,
                                       SPRepr *prev)
{
    if (_in_transaction) {
        _log = new SPReprActionDel(&parent, &child, prev, _log);
        _log = _log->optimizeOne();
    }
}

void SimpleSession::notifyChildOrderChanged(SPRepr &parent, SPRepr &child,
                                            SPRepr *old_prev, SPRepr *new_prev)
{
    if (_in_transaction) {
        _log = new SPReprActionChgOrder(&parent, &child, old_prev, new_prev, _log);
        _log = _log->optimizeOne();
    }
}

void SimpleSession::notifyContentChanged(SPRepr &node,
                                         Util::SharedCStringPtr old_content,
                                         Util::SharedCStringPtr new_content)
{
    if (_in_transaction) {
        _log = new SPReprActionChgContent(&node, old_content, new_content, _log);
        _log = _log->optimizeOne();
    }
}

void SimpleSession::notifyAttributeChanged(SPRepr &node, GQuark name,
                                           Util::SharedCStringPtr old_value,
                                           Util::SharedCStringPtr new_value)
{
    if (_in_transaction) {
        _log = new SPReprActionChgAttr(&node, name, old_value, new_value, _log);
        _log = _log->optimizeOne();
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
