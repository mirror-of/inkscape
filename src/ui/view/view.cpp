#define __SP_VIEW_C__

/** \file
 * View implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "macros.h"
#include "libnr/nr-point.h"
#include "document.h"
#include "view.h"
#include "message-stack.h"
#include "message-context.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace View {

//static bool 
//_onShutdown (View* v)
//{
//    return v->onShutdown();
///}

static void 
_onPositionSet (double x, double y, View* v)
{
    v->onPositionSet (x,y);
}

static void 
_onResized (double x, double y, View* v)
{
    v->onResized (x,y);
}

static void 
_onRedrawRequested (View* v)
{
    v->onRedrawRequested();
}

static void 
_onStatusMessage (Inkscape::MessageType type, gchar const *message, View* v)
{
//fprintf(stderr,"type=%d msg=%s v=%X\n",(int)type, message, v);fflush(stderr);
    v->onStatusMessage (type, message);
}

static void 
_onDocumentURISet (gchar const* uri, View* v)
{
    v->onDocumentURISet (uri);
}

static void 
_onDocumentResized (double x, double y, View* v)
{
    v->onDocumentResized (x,y);
}

//--------------------------------------------------------------------
View::View()
:  _doc(0)
{
    _message_stack = new Inkscape::MessageStack();
    _tips_message_context = new Inkscape::MessageContext(_message_stack);

    _position_set_connection = _position_set_signal.connect (sigc::bind (sigc::ptr_fun (&_onPositionSet), this));
    _resized_connection = _resized_signal.connect (sigc::bind (sigc::ptr_fun (&_onResized), this));
    _redraw_requested_connection = _redraw_requested_signal.connect (sigc::bind (sigc::ptr_fun (&_onRedrawRequested), this));
    
    _message_changed_connection = _message_stack->connectChanged (sigc::bind (sigc::ptr_fun (&_onStatusMessage), this));
}

/**
 * Deletes and nulls all View message stacks and disconnects it from signals.
 */
View::~View()
{
    _message_changed_connection.disconnect();

    delete _tips_message_context;
    _tips_message_context = 0;

    Inkscape::GC::release(_message_stack);
    _message_stack = 0;

    if (_doc) {
        _document_uri_set_connection.disconnect();
        _document_resized_connection.disconnect();
        sp_document_unref(_doc);
        _doc = 0;
    }
    
    _position_set_connection.~connection();
    _resized_connection.~connection();
    _redraw_requested_connection.~connection();
    _message_changed_connection.~connection();
    _document_uri_set_connection.~connection();
    _document_resized_connection.~connection();

    _position_set_signal.~signal();
    _resized_signal.~signal();
    _redraw_requested_signal.~signal();

   Inkscape::Verb::delete_all_view (this);
}

void View::setPosition (double x, double y)
{
    _position_set_signal.emit (x,y);
}

void View::setPosition(NR::Point const &p) 
{ 
    setPosition (double(p[NR::X]), double(p[NR::Y])); 
}

void View::emitResized (double width, double height)
{
    _resized_signal.emit (width, height);
}

void View::requestRedraw() 
{
    _redraw_requested_signal.emit();
}

/**
 * Calls the view's virtual function setDocument(), disconnects the view 
 * from the document signals, connects the view to a new one, and emits the
 * _document_set_signal on the view.
 * 
 * \param doc The new document to connect the view to.
 */
void View::setDocument(SPDocument *doc) {
    g_return_if_fail(doc != NULL);

    this->setDoc (doc);
    if (_doc) {
        _document_uri_set_connection.disconnect();
        _document_resized_connection.disconnect();
        sp_document_unref (_doc);
    }

    _doc = sp_document_ref (doc);
    _document_uri_set_connection = 
        _doc->connectURISet(sigc::bind(sigc::ptr_fun(&_onDocumentURISet), this));
    _document_resized_connection = 
        _doc->connectResized(sigc::bind(sigc::ptr_fun(&_onDocumentResized), this));
    _document_uri_set_signal.emit (SP_DOCUMENT_URI(_doc));
}

}}}

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
