#ifndef INKSCAPE_UI_VIEW_VIEW_H
#define INKSCAPE_UI_VIEW_VIEW_H

/** \file
 * Abstract base class for all SVG document views
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

#include <gdk/gdktypes.h>
#include <sigc++/connection.h>
#include "message.h"

namespace Inkscape {
    class MessageContext;
    class MessageStack;
}
namespace NR {
    class Point;
}
class SPView;
class SPDocument;

/**
 * Iterates until true or returns false.
 * When used as signal accumulator, stops emission if one slot returns true.
 */
struct StopOnTrue {
  typedef bool result_type;

  template<typename T_iterator>
  result_type operator()(T_iterator first, T_iterator last) const{	
	for (; first != last; ++first)
		if (*first) return true;
	return false;      
  }
};

/**
 * SPView is an abstract base class of all UI document views.  This
 * includes both the editing window and the SVG preview, but does not
 * include the non-UI RGBA buffer-based NRArenas nor the XML editor or
 * similar views.  The SPView base class has very little functionality of
 * its own.
 */
class SPView {
public:

    SPView::SPView();
    virtual SPView::~SPView();

    /// Returns a pointer to the view's document.
    SPDocument *doc() const 
      { return _doc; }
    /// Returns a pointer to the view's message stack.
    Inkscape::MessageStack *messageStack() const 
      { return _message_stack; }
    /// Returns a pointer to the view's tipsMessageContext.
    Inkscape::MessageContext *tipsMessageContext() const 
      { return _tips_message_context; }

//    bool shutdown();
//    sigc::connection connectShutdown (const sigc::slot<bool>& slot)
//      { return _shutdown_signal.connect (slot); }
    void setPosition(gdouble x, gdouble y);
    void setPosition(NR::Point const &p); 
    void emitResized(gdouble width, gdouble height);
    void requestRedraw(); 
    void setDocument(SPDocument *doc);

    // view subclasses must give implementations of these methods
    
    virtual bool shutdown() = 0;
    virtual void mouseover() = 0;
    virtual void mouseout() = 0;

    virtual void onPositionSet (double, double) = 0;
    virtual void onResized (double, double) = 0;
    virtual void onRedrawRequested() = 0;
    virtual void onDocumentSet (SPDocument*) = 0;
    virtual void onStatusMessage (Inkscape::MessageType type, gchar const *message) = 0;
    virtual void onDocumentURISet (gchar const* uri) = 0;
    virtual void onDocumentResized (double, double) = 0;

protected:
    SPDocument *_doc;
    Inkscape::MessageStack *_message_stack;
    Inkscape::MessageContext *_tips_message_context;

//    sigc::signal<bool>::accumulated<StopOnTrue>      _shutdown_signal;
    sigc::signal<void,double,double>                 _position_set_signal;
    sigc::signal<void,double,double>                 _resized_signal;
    sigc::signal<void>                               _redraw_requested_signal;
    sigc::signal<void,SPDocument*>                   _document_set_signal;
//    sigc::connection _shutdown_connection;

private:
    sigc::connection _position_set_connection;
    sigc::connection _resized_connection;
    sigc::connection _redraw_requested_connection;
    sigc::connection _document_set_connection;
    sigc::connection _message_changed_connection;  // foreign
    sigc::connection _document_uri_set_connection; // foreign
    sigc::connection _document_resized_connection; // foreign
};

#endif  // INKSCAPE_UI_VIEW_VIEW_H

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
