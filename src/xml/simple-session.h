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

#ifndef SEEN_INKSCAPE_XML_SIMPLE_SESSION_H
#define SEEN_INKSCAPE_XML_SIMPLE_SESSION_H

#include "xml/session.h"
#include "xml/transaction-logger.h"

namespace Inkscape {

namespace XML {

class SimpleSession : public Session, public TransactionLogger {
public:
    SimpleSession() : _in_transaction(false), _log(NULL) {}

    bool inTransaction() { return _in_transaction; }

    void beginTransaction();
    void rollback();
    void commit();
    SPReprAction *commitUndoable();

    Session &session() { return *this; }

    void notifyChildAdded(SPRepr &parent, SPRepr &child, SPRepr *prev);

    void notifyChildRemoved(SPRepr &parent, SPRepr &child, SPRepr *prev);

    void notifyChildOrderChanged(SPRepr &parent, SPRepr &child,
                                 SPRepr *old_prev, SPRepr *new_prev);

    void notifyContentChanged(SPRepr &node,
                              Util::SharedCStringPtr old_content,
                              Util::SharedCStringPtr new_content);

    void notifyAttributeChanged(SPRepr &node, GQuark name,
                                Util::SharedCStringPtr old_value,
                                Util::SharedCStringPtr new_value);

private:
    SimpleSession(SimpleSession const &); // no copy
    void operator=(SimpleSession const &); // no assign

    bool _in_transaction;
    SPReprAction *_log;
};

}

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
