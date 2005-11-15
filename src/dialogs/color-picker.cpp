#define __COLOR_PICKER_C__

/**
 * \brief  Color picker button & window
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) Lauris Kaplinski 2000-2002
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "application/application.h"
#include "application/editor.h"
#include "macros.h"
#include "inkscape.h"
#include "desktop-handles.h"
#include "document.h"
#include "sp-object.h"
#include <glibmm/i18n.h>
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "helper/window.h"
#include "widgets/sp-color-selector.h"
#include "widgets/sp-color-notebook.h"
#include "widgets/sp-color-preview.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "dialog-events.h"

#include "color-picker.h"

static sigc::connection _dialogs_hidden_connection;
static sigc::connection _dialogs_unhidden_connection;
static sigc::connection _selector_changed_connection;

static void sp_color_picker_clicked(GObject *cp, void *data);

void
sp_color_picker_button(Inkscape::XML::Node *repr, bool undo, GtkWidget *dlg, GtkWidget *t,
                       gchar const *label, gchar *key,
                       gchar *color_dialog_label,
                       gchar *tip,
                       gchar *opacity_key,
                       int row)
{
    GtkWidget *l = gtk_label_new(label);
    gtk_misc_set_alignment(GTK_MISC(l), 1.0, 0.5);
    gtk_widget_show(l);
    gtk_table_attach(GTK_TABLE(t), l, 0, 1, row, row+1,
                     (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                     (GtkAttachOptions)0, 0, 0);

    GtkWidget *cp = sp_color_picker_new(repr, undo, dlg, key, opacity_key, color_dialog_label, tip, 0);
    gtk_widget_show(cp);
    gtk_table_attach(GTK_TABLE(t), cp, 1, 2, row, row+1,
                     (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                     (GtkAttachOptions)0, 0, 0);

    g_object_set_data(G_OBJECT(dlg), key, cp);
}



static void
sp_color_picker_destroy(GtkObject *cp, gpointer data)
{
    _selector_changed_connection.disconnect();
    GtkObject *w = (GtkObject *) g_object_get_data(G_OBJECT(cp), "window");

    if (w) {
        g_assert ( G_IS_OBJECT (w) );
        gtk_object_destroy(w);
    }

}



/**
 * \brief  Creates a new color picker 
 *
 */
GtkWidget *
sp_color_picker_new(Inkscape::XML::Node *repr, bool undo, GtkWidget *dlg, gchar *colorkey, gchar *alphakey,
                    gchar *title, gchar *tip, guint32 rgba)
{
    GtkWidget *b = gtk_button_new();
    gtk_button_set_relief (GTK_BUTTON(b), GTK_RELIEF_NONE);

    g_object_set_data(G_OBJECT(b), "title", title);

    GtkWidget *cpv = sp_color_preview_new(rgba);

    gtk_widget_show(cpv);
    gtk_container_add(GTK_CONTAINER(b), cpv);

    GtkTooltips *tt = gtk_tooltips_new();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, tip, NULL);

    g_object_set_data(G_OBJECT(b), "preview", cpv);

    g_object_set_data(G_OBJECT(b), "colorkey", colorkey);
    g_object_set_data(G_OBJECT(b), "alphakey", alphakey);
    g_object_set_data(G_OBJECT(b), "repr", repr);
    g_object_set_data(G_OBJECT(b), "undo", GINT_TO_POINTER(undo));

    g_signal_connect(G_OBJECT(b), "destroy",
                     G_CALLBACK(sp_color_picker_destroy), dlg);
    g_signal_connect(G_OBJECT(b), "clicked",
                     G_CALLBACK(sp_color_picker_clicked), dlg);

    return b;
}



void
sp_color_picker_set_rgba32(GtkWidget *cp, guint32 rgba)
{
    g_assert ( G_IS_OBJECT (cp) );

    SPColorPreview *cpv = (SPColorPreview *)g_object_get_data(G_OBJECT(cp), "preview");
    g_assert ( G_IS_OBJECT (cpv) );
    sp_color_preview_set_rgba32(cpv, rgba);

    SPColorSelector *csel = (SPColorSelector *)g_object_get_data(G_OBJECT(cp), "selector");

    if (csel) {
        g_assert ( G_IS_OBJECT (csel) );

        SPColor color;
        sp_color_set_rgb_rgba32(&color, rgba);
        csel->base->setColorAlpha(color, SP_RGBA32_A_F(rgba));
    }

    g_object_set_data(G_OBJECT(cp), "color", GUINT_TO_POINTER(rgba));

}



static void
sp_color_picker_window_destroy(GtkObject *object, GObject *cp)
{
    /* remove window object */
    GtkWidget *w = (GtkWidget*) g_object_get_data(G_OBJECT(cp), "window");
    if (w) {
        if (Inkscape::NSApplication::Application::getNewGui())
        {
            _dialogs_hidden_connection.disconnect();
            _dialogs_unhidden_connection.disconnect();
        } else {
            sp_signal_disconnect_by_data(INKSCAPE, w);
        }
        gtk_widget_destroy(GTK_WIDGET(w));
    }

    g_object_set_data(G_OBJECT(cp), "window", NULL);
    g_object_set_data(G_OBJECT(cp), "selector", NULL);

}



static void
sp_color_picker_color_mod(SPColorSelector *csel, GObject *cp)
{
    GtkWidget *dlg = (GtkWidget *) g_object_get_data(G_OBJECT(cp), "dialog");

    if (!dlg || g_object_get_data(G_OBJECT(dlg), "update")) {
        return;
    }

    if (!SP_ACTIVE_DESKTOP) {
        return;
    }

    gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(TRUE));

    SPColor color;
    float alpha;
    csel->base->getColorAlpha(color, &alpha);
    guint32 rgba = sp_color_get_rgba32_falpha(&color, alpha);

    g_object_set_data(G_OBJECT(cp), "color", GUINT_TO_POINTER(rgba));

    SPColorPreview *cpv = (SPColorPreview *)g_object_get_data(G_OBJECT(cp), "preview");
    gchar *colorkey = (gchar *)g_object_get_data(G_OBJECT(cp), "colorkey");
    gchar *alphakey = (gchar *)g_object_get_data(G_OBJECT(cp), "alphakey");

    sp_color_preview_set_rgba32(cpv, rgba);

    Inkscape::XML::Node *repr = (Inkscape::XML::Node *)g_object_get_data(G_OBJECT(cp), "repr");
    if (!repr)
         repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP));

    gchar c[32];
    sp_svg_write_color(c, 32, rgba);
    if (repr)
        sp_repr_set_attr(repr, colorkey, c);

    if (alphakey && repr) {
        sp_repr_set_css_double(repr, alphakey, (rgba & 0xff) / 255.0);
    }

    gtk_object_set_data(GTK_OBJECT(dlg), "update", GINT_TO_POINTER(FALSE));

    bool undo = (bool) g_object_get_data(G_OBJECT(cp), "undo");
    if (undo)
        sp_document_done(SP_DT_DOCUMENT(SP_ACTIVE_DESKTOP));
}


static void
sp_color_picker_window_close(GtkButton *button, GtkWidget *w)
{
    gtk_widget_destroy(w);
}


static void
sp_color_picker_clicked(GObject *cp, void *data)
{
    GtkWidget *dlg = (GtkWidget *) data;

    GtkWidget *w = (GtkWidget *) g_object_get_data(cp, "window");

    if (!w) {
        w = sp_window_new(NULL, TRUE);
        gtk_window_set_title( GTK_WINDOW(w), (gchar *)g_object_get_data(cp, "title"));
        gtk_container_set_border_width(GTK_CONTAINER(w), 4);
        g_object_set_data(cp, "window", w);

        gtk_window_set_position(GTK_WINDOW(w), GTK_WIN_POS_CENTER);
        sp_transientize(w);

        gtk_signal_connect(GTK_OBJECT(w), "event", GTK_SIGNAL_FUNC(sp_dialog_event_handler), w);

        if (Inkscape::NSApplication::Application::getNewGui())
        {
            _dialogs_hidden_connection = Inkscape::NSApplication::Editor::connectDialogsHidden (sigc::bind (&on_dialog_hide, w));
            _dialogs_unhidden_connection = Inkscape::NSApplication::Editor::connectDialogsUnhidden (sigc::bind (&on_dialog_unhide, w));
        } else {
            g_signal_connect(G_OBJECT(INKSCAPE), "dialogs_hide", G_CALLBACK(sp_dialog_hide), w);
            g_signal_connect(G_OBJECT(INKSCAPE), "dialogs_unhide", G_CALLBACK(sp_dialog_unhide), w);
        }

        g_signal_connect(G_OBJECT(w), "destroy", G_CALLBACK(sp_color_picker_window_destroy), cp);

        GtkWidget *vb = gtk_vbox_new(FALSE, 4);
        gtk_container_add(GTK_CONTAINER(w), vb);

        GtkWidget *csel = sp_color_selector_new(SP_TYPE_COLOR_NOTEBOOK,
                                                SP_COLORSPACE_TYPE_UNKNOWN);
        gtk_box_pack_start(GTK_BOX(vb), csel, TRUE, TRUE, 0);
        guint32 rgba = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(cp), "color"));

        SPColor color;
        sp_color_set_rgb_rgba32(&color, rgba);
        SP_COLOR_SELECTOR(csel)->base->setColorAlpha(color,
                                                     SP_RGBA32_A_F(rgba));
        g_signal_connect(G_OBJECT(csel), "dragged",
                         G_CALLBACK(sp_color_picker_color_mod), cp);
        _selector_changed_connection = ((SPColorSelector*)csel)->base->connectChanged (sigc::bind (sigc::ptr_fun (&sp_color_picker_color_mod), cp));
//        g_signal_connect(G_OBJECT(csel), "changed",
//                         G_CALLBACK(sp_color_picker_color_mod), cp);

        g_object_set_data(cp, "selector", csel);
        g_object_set_data(cp, "dialog", dlg);

        GtkWidget *hs = gtk_hseparator_new();
        gtk_box_pack_start(GTK_BOX(vb), hs, FALSE, FALSE, 0);

        GtkWidget *hb = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, FALSE, 0);

        GtkWidget *b = gtk_button_new_with_label(_("Close"));
        gtk_box_pack_end(GTK_BOX(hb), b, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(b), "clicked",
                         G_CALLBACK(sp_color_picker_window_close), w);

        gtk_widget_show_all(w);

    } else {
        gtk_window_present(GTK_WINDOW(w));
    }

}


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
