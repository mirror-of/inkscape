#ifndef __COLOR_PICKER_H__
#define __COLOR_PICKER_H__

/**
 * \brief  Color picker button & window
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) Lauris Kaplinski 2000-2002
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

namespace Inkscape {
	namespace XML {
		class Node;
	}
}

void
sp_color_picker_button(Inkscape::XML::Node *repr, bool undo, GtkWidget *dlg, GtkWidget *t,
                       gchar const *label, gchar *key,
                       gchar *color_dialog_label,
                       gchar *opacity_key,
                       int row);

GtkWidget *
sp_color_picker_new(Inkscape::XML::Node *repr, bool undo, GtkWidget *dlg, gchar *colorkey, gchar *alphakey,
                    gchar *title, guint32 rgba);

void sp_color_picker_set_rgba32(GtkWidget *cp, guint32 rgba);

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
