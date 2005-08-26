#ifndef INKSCAPE_UI_VIEW_VIEW_H
#define INKSCAPE_UI_VIEW_VIEW_H

/** \file
 * Abstract base class for all SVG document views
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-point.h>
#include <sigc++/connection.h>
#include <gtk/gtkeventbox.h>
#include "message.h"

#define SP_TYPE_VIEW (sp_view_get_type ())
#define SP_VIEW(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_VIEW, Inkscape::UI::View::View))
#define SP_IS_VIEW(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_VIEW))

namespace Inkscape {
class MessageContext;
class MessageStack;
}
class SPView;
class SPDocument;


GType    sp_view_get_type (void);
void     sp_view_set_document (SPView *view, SPDocument *doc);
void     sp_view_emit_resized (SPView *view, gdouble width, gdouble height);
void     sp_view_set_position (SPView *view, gdouble x, gdouble y);
gboolean sp_view_shutdown (SPView *view);
void     sp_view_request_redraw (SPView *view);

/**
 * Calls sp_view_set_position() with the point's coordinates.
 */
inline void sp_view_set_position(SPView *view, NR::Point const &p)
{
    sp_view_set_position(view, p[NR::X], p[NR::Y]);
}

/**
 * SPView is an abstract base class of all UI document views.  This
 * includes both the editing window and the SVG preview, but does not
 * include the non-UI RGBA buffer-based NRArenas nor the XML editor or
 * similar views.  The SPView base class has very little functionality of
 * its own.
 */
class SPView : public GObject {
 public:
    GObject object;  /// \todo TODO:  Remove this

    SPDocument *doc;

    static void init(SPView *view);
    static void dispose(GObject *obj);

    /// Returns a pointer to the view's message stack.
    Inkscape::MessageStack *messageStack() {
	return _message_stack;
    }

    /// Returns a pointer to the view's tipsMessageContext.
    Inkscape::MessageContext *tipsMessageContext() {
	return _tips_message_context;
    }

    // Wrappers for C versions of routines
    void setDocument(SPDocument *doc);
    void emitResized(gdouble width, gdouble height);
    void setPosition(gdouble x, gdouble y);
    void setPosition(NR::Point const &p);
    gboolean shutdown();
    void requestRedraw();

private:
    static void _set_status_message(Inkscape::MessageType type, gchar const *message, SPView *view);

    Inkscape::MessageStack *_message_stack;
    Inkscape::MessageContext *_tips_message_context;

    sigc::connection _message_changed_connection;
 public: // for now...
    sigc::connection _document_uri_set_connection;
    sigc::connection _document_resized_connection;
};

namespace Inkscape {
namespace UI {
namespace View {
  typedef SPView View;
}; // namespace View
}; // namespace UI
}; // namespace Inkscape



/**
 * The Glib-style vtable for the SPView class.
 */
class SPViewClass {
 public:
	GObjectClass parent_class;

	/// Request shutdown
	gboolean (* shutdown) (SPView *view);
	/// Request redraw of visible area
	void (* request_redraw) (SPView *view);
	/// Virtual method to set/change/remove document link
	void (* set_document) (SPView *view, SPDocument *doc);
	/// Virtual method about document size change
	void (* document_resized) (SPView *view, SPDocument *doc, gdouble width, gdouble height);

	/// Signal of view uri change
	void (* uri_set) (SPView *view, const guchar *uri);
	/// Signal of view size change
	void (* resized) (SPView *view, gdouble width, gdouble height);
	/// Cursor position
	void (* position_set) (SPView *view, gdouble x, gdouble y);
	/// Status
	void (* set_status_message) (SPView *view, Inkscape::MessageType type, gchar const *message);
};

#define SP_VIEW_DOCUMENT(v) (SP_VIEW (v)->doc)

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
