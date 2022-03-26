// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <2geom/point.h>
#include <memory>
#include "document.h"
#include "view.h"
#include "message-stack.h"
#include "message-context.h"
#include "inkscape.h"

namespace Inkscape {
namespace UI {
namespace View {

View::View()
:  _doc(nullptr)
{
    _message_stack = std::make_shared<Inkscape::MessageStack>();
    _tips_message_context = std::make_unique<Inkscape::MessageContext>(_message_stack);

    _resized_connection = _resized_signal.connect([this] (double x, double y) {
        onResized(x, y);
    });
    
    _message_changed_connection = _message_stack->connectChanged([this] (Inkscape::MessageType type, const gchar *message) {
        onStatusMessage(type, message);
    });
}

View::~View()
{
    _close();
}

void View::_close() {
    _message_changed_connection.disconnect();

    _tips_message_context = nullptr;

    _message_stack = nullptr;

    if (_doc) {
        _document_uri_set_connection.disconnect();
        INKSCAPE.remove_document(_doc);
        _doc = nullptr;
    }
}

void View::emitResized (double width, double height)
{
    _resized_signal.emit (width, height);
}

void View::setDocument(SPDocument *doc) {
    if (!doc) return;

    if (_doc) {
        _document_uri_set_connection.disconnect();
        INKSCAPE.remove_document(_doc);
    }

    INKSCAPE.add_document(doc);

    _doc = doc;
    _document_uri_set_connection = _doc->connectFilenameSet([this] (const gchar *filename) {
        onDocumentFilenameSet(filename);
    });
    _document_filename_set_signal.emit( _doc->getDocumentFilename() );
}

} // namespace View
} // namespace UI
} // namespace Inkscape

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
