#ifndef __COLOR_PICKER_H__
#define __COLOR_PICKER_H__

/**
 * \file  Color picker button \& window.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) Lauris Kaplinski 2000-2002
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>
#include <gtk/gtkwidget.h>

namespace Inkscape {
namespace XML {
class Node;
}
}

void
sp_color_picker_button(Inkscape::XML::Node *repr, bool undo, GtkWidget *dlg, GtkWidget *t,
                       gchar const *label, gchar *key,
                       gchar *color_dialog_label,
                       gchar *tip,
                       gchar *opacity_key,
                       int row);

GtkWidget *
sp_color_picker_new(Inkscape::XML::Node *repr, bool undo, GtkWidget *dlg, gchar *colorkey, gchar *alphakey,
                    gchar *title, gchar *tip, guint32 rgba);

void sp_color_picker_set_rgba32(GtkWidget *cp, guint32 rgba);


#endif /* !__COLOR_PICKER_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
