#define __SP_FILE_C__

/*
 * File/Print operations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Chema Celorio <chema@celorio.com>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/**
 * Note: This file needs to be cleaned up extensively.
 * What it probably needs is to have one .h file for
 * the API, and two or more .cpp files for the implementations.
 */

#include <config.h>

#include <string.h>
#include <time.h>
#include <libnr/nr-pixops.h>
#include <glib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkfilesel.h>

#include "macros.h"
#include "xml/repr-private.h"
#include "document.h"
#include "view.h"
#include "dir-util.h"
#include "helper/png-write.h"
#include "dialogs/export.h"
#include "helper/sp-intl.h"
#include "inkscape.h"
#include "desktop.h"
#include "sp-image.h"
#include "interface.h"
#include "print.h"
#include "file.h"
#include "dialogs/dialog-events.h"

#include "dialogs/filedialog.h"

#include "sp-namedview.h"

#include "module.h"
#include "modules/menu.h"
#include "modules/system.h"

#ifdef WIN32
#include "modules/win32.h"
#else
static void sp_file_save_ok (GtkWidget *save_dialog, SPDocument *doc, const gchar *key);
#endif

/**
 * 'Current' paths.  Used to remember which directory
 * had the last file accessed.
 * Static globals are evil.  This will be gone soon
 * as C++ification continues
 */
gchar *open_path   = NULL;
gchar *save_path   = NULL;
gchar *import_path = NULL;
gchar *export_path = NULL;



/*######################
## N E W
######################*/

/**
 * Create a blank document and add it to the desktop
 */
void
sp_file_new (void)
{

    SPDocument *doc = sp_document_new (NULL, TRUE, TRUE);
    g_return_if_fail (doc != NULL);

    SPViewWidget *dtw = sp_desktop_widget_new (sp_document_namedview (doc, NULL));
    sp_document_unref (doc);
    g_return_if_fail (dtw != NULL);

    sp_create_window (dtw, TRUE);
    sp_namedview_window_from_document (SP_DESKTOP(dtw->view));

}


/*######################
## D E L E T E
######################*/

/**
 *  Perform document closures preceding an exit()
 */
void
sp_file_exit (void)
{
    sp_ui_close_all ();
    // no need to call inkscape_exit here; last document being closed will take care of that
}


/*######################
## O P E N
######################*/

/**
 *  Open a file, add the document to the desktop
 */
void
sp_file_open (const gchar *uri, const gchar *key)
{

    if (!key)
        key = SP_MODULE_KEY_INPUT_DEFAULT;

    SPDocument *doc = sp_module_system_open (key, uri);
    if (doc) {
        SPViewWidget *dtw = sp_desktop_widget_new (sp_document_namedview (doc, NULL));
        sp_document_unref (doc);
        sp_create_window (dtw, TRUE);
        sp_namedview_window_from_document (SP_DESKTOP(dtw->view));
    } else {
        gchar *text = g_strdup_printf(_("Failed to load the requested file %s"), uri);
        sp_ui_error_dialog (text);
        g_free (text);
    }
}

/**
 * OK callback for GTK dialog
 */
static void
file_open_dialog_ok (GtkWidget *widget, GtkFileSelection *fs)
{
    gchar *filename = g_strdup (gtk_file_selection_get_filename (fs));

    if (filename && g_file_test (filename, G_FILE_TEST_IS_DIR)) {
        g_free (open_path);
        if (filename[strlen(filename) - 1] != G_DIR_SEPARATOR) {
            open_path = g_strconcat (filename, G_DIR_SEPARATOR_S, NULL);
            g_free (filename);
        } else {
            open_path = filename;
        }
        gtk_file_selection_set_filename (fs, open_path);
        return;
    }

    if (filename != NULL) {
        g_free (open_path);
        open_path = g_dirname (filename);
        if (open_path) open_path = g_strconcat (open_path, G_DIR_SEPARATOR_S, NULL);
        gchar *key = (gchar*)g_object_get_data (G_OBJECT (fs), "type-key");
        sp_file_open (filename, key);
        g_free (filename);
    }

    gtk_widget_destroy (GTK_WIDGET (fs));
}

/**
 * Cancel callback for GTK dialog
 */
static void
file_open_dialog_cancel (GtkButton *b, GtkFileSelection *fs)
{
    gtk_widget_destroy (GTK_WIDGET (fs));
}

/**
 * Type pulldown menu selection callback for GTK dialog
 */
static void
file_open_dialog_type_selected (SPMenu *menu, gpointer itemdata, GObject *fsel)
{
    g_object_set_data (fsel, "type-key", itemdata);
}

/**
 *  Display an file Open selector.  Open a document if OK is pressed.
 */
void
sp_file_open_dialog (gpointer object, gpointer data)
{
#ifdef WIN32
    Inkscape::UI::Dialogs::FileOpenDialog *dlg =
        new Inkscape::UI::Dialogs::FileOpenDialog(
                 (const char *)open_path,
                 Inkscape::UI::Dialogs::SVG_TYPES,
                 (const char *)_("Select file to open"));
    char *fileName = dlg->show();
    delete dlg;
    if (fileName) {
        g_free (open_path);
        open_path = g_dirname (fileName);
        if (open_path) open_path = g_strconcat (open_path, G_DIR_SEPARATOR_S, NULL);
        sp_file_open (fileName, NULL);
        g_free (fileName);
    }

#else

    GtkFileSelection *fsel = (GtkFileSelection *) gtk_file_selection_new (_("Select file to open"));
    gtk_file_selection_hide_fileop_buttons (fsel);

    g_signal_connect (G_OBJECT (fsel->ok_button), "clicked", G_CALLBACK (file_open_dialog_ok), fsel);
    g_signal_connect (G_OBJECT (fsel->cancel_button), "clicked", G_CALLBACK (file_open_dialog_cancel), fsel);

    if (open_path)
        gtk_file_selection_set_filename (fsel, open_path);

    /* Create file type box */
    GtkWidget *hb = gtk_hbox_new (FALSE, 4);
    gtk_box_pack_start (GTK_BOX (fsel->main_vbox), hb, FALSE, FALSE, 0);
    GtkWidget *om = gtk_option_menu_new ();
    gtk_box_pack_end (GTK_BOX (hb), om, FALSE, FALSE, 0);

    GtkWidget *m = GTK_WIDGET(sp_module_menu_open ());
    g_object_set_data (G_OBJECT (fsel), "type-key", ((SPMenu *) m)->activedata);
    g_signal_connect (G_OBJECT (m), "selected", G_CALLBACK (file_open_dialog_type_selected), fsel);
    gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);
    GtkWidget *l = gtk_label_new (_("File type:"));
    gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    gtk_widget_show_all (hb);

    gtk_window_set_modal (GTK_WINDOW (fsel), TRUE);
    sp_transientize ((GtkWidget *) fsel);

    gtk_widget_show ((GtkWidget *) fsel);

#endif

}




/*######################
## S A V E
######################*/

/**
 * This 'save' function called by the others below
 */
static void
file_save (SPDocument *doc, const gchar *uri, const gchar *key)
{
    if (!doc || !uri) //Safety check
        return;
    if (!key)
        key = SP_MODULE_KEY_OUTPUT_DEFAULT;
    return sp_module_system_save (key, doc, uri);
}

/**
 *  Display a SaveAs dialog.  Save the document if OK pressed.
 */
static gboolean
sp_file_save_dialog (SPDocument *doc)
{
#ifdef WIN32
    Inkscape::UI::Dialogs::FileSaveDialog *dlg =
        new Inkscape::UI::Dialogs::FileSaveDialog(
                 (const char *)save_path,
                 Inkscape::UI::Dialogs::SVG_TYPES,
                 (const char *)_("Select file to save"));
    char *fileName = dlg->show();
    gint selectionType = dlg->getSelectionType();
    //Convert to old types
    gchar *oldSelectionType = SP_MODULE_KEY_OUTPUT_SVG;
    if (selectionType == Inkscape::UI::Dialogs::SVG_NAMESPACE_WITH_EXTENSIONS)
        oldSelectionType = SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE;
    delete dlg;
    if (fileName && *fileName) {
        gchar *ext = (gchar *)sp_extension_from_path ((const gchar *)fileName);
        if (!ext || (strcmp(ext, "svg") && strcmp(ext, "xml"))) {
            gchar *oldFileName = fileName;
            fileName = g_strconcat (oldFileName, ".svg", NULL);
            g_free (oldFileName);
        }
        file_save (doc, fileName, oldSelectionType);
        g_free (save_path);
        save_path = g_dirname (fileName);
        save_path = g_strdup (save_path);
        g_free (fileName);
        return TRUE;
    } else {
        return FALSE;
    }
#else

    GtkWidget *dlg = gtk_file_selection_new (_("Save file"));
    g_object_set_data (G_OBJECT (dlg), "document", doc);
    gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);

    GtkFileSelection *fs = GTK_FILE_SELECTION (dlg);

    GtkWidget *hb = gtk_hbox_new (FALSE, 4);
    gtk_box_pack_start (GTK_BOX (fs->main_vbox), hb, FALSE, FALSE, 0);
    GtkWidget *om = gtk_option_menu_new ();
    gtk_box_pack_end (GTK_BOX (hb), om, FALSE, FALSE, 0);

    GtkWidget *menu = GTK_WIDGET(sp_module_menu_save ());
    gtk_option_menu_set_menu (GTK_OPTION_MENU (om), menu);
    GtkWidget *l = gtk_label_new (_("File type:"));
    gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    gtk_widget_show_all (hb);

    gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
    sp_transientize (dlg);
    int b = gtk_dialog_run (GTK_DIALOG (dlg));

    if (b == GTK_RESPONSE_OK) {
        sp_file_save_ok (dlg, doc, (gchar const *)((SPMenu *) menu)->activedata);
        gtk_widget_destroy (dlg);
        return TRUE;
    } else {
        gtk_widget_destroy (dlg);
        return FALSE;
    }
#endif
}

#ifndef WIN32
/**
 * Callback for the OK button of the GTK save file selector
 */
static void
sp_file_save_ok (GtkWidget *save_dialog, SPDocument *doc, gchar const *key)
{

    GtkFileSelection *fs = GTK_FILE_SELECTION (save_dialog);

    const gchar *filename = gtk_file_selection_get_filename (fs);
    gchar *ext = (gchar *)sp_extension_from_path ((const gchar *)filename);
    if (!ext || (strcmp(ext, "svg") && strcmp(ext, "xml"))) {
        gchar *oldFileName = filename;
        filename = g_strconcat (oldFileName, ".svg", NULL);
        g_free (oldFileName);
    }
    const gchar *raw_filename = gtk_entry_get_text (GTK_ENTRY (fs->selection_entry));

    g_assert (filename && raw_filename);

    if (g_file_test (filename, G_FILE_TEST_EXISTS)) {
        if (g_file_test (filename, G_FILE_TEST_IS_DIR)) {
            if (filename[strlen (filename) - 1] != G_DIR_SEPARATOR) {
                gchar *s = g_strconcat (filename, G_DIR_SEPARATOR_S, NULL);
                gtk_file_selection_set_filename (fs, s);
                g_free (s);
            } else {
                gtk_file_selection_set_filename (fs, filename);
            }
        } else {
            /* TODO: Handle overwriting files differently - TJG */
            gtk_widget_set_sensitive (GTK_WIDGET (fs), FALSE);
            file_save (doc, filename, key);
            gtk_widget_set_sensitive (GTK_WIDGET (fs), TRUE);
        }
    } else {
        gtk_widget_set_sensitive (GTK_WIDGET (fs), FALSE);
        file_save (doc, filename, key);
        gtk_widget_set_sensitive (GTK_WIDGET (fs), TRUE);
    }
}
#endif

/**
 * Save a document, displaying a SaveAs dialog if necessary.
 */
gboolean
sp_file_save_document (SPDocument *doc)
{

    gboolean success = TRUE;

    SPRepr *repr = sp_document_repr_root (doc);

    const gchar *fn = sp_repr_attr (repr, "sodipodi:modified");
    if (fn != NULL) {
        if (doc->uri == NULL) {
            success = sp_file_save_dialog (doc);
        } else {
            /* TODO: This currently requires a recognizable extension to
                 be on the file name - odd stuff won't work */
            fn = g_strdup (doc->uri);
            file_save(doc, fn, SP_MODULE_KEY_AUTODETECT);
            success = TRUE;
            g_free ((void *) fn);
        }

        if (success)
            sp_view_set_statusf_flash (SP_VIEW(SP_ACTIVE_DESKTOP), "Document saved.");
        else
            sp_view_set_statusf_flash (SP_VIEW(SP_ACTIVE_DESKTOP), "Document not saved.");

    } else {
        sp_view_set_statusf_flash (SP_VIEW(SP_ACTIVE_DESKTOP), "No changes need to be saved.");
        success = TRUE;
    }
    
    return success;
}


/**
 * Save a document.
 */
void
sp_file_save (gpointer object, gpointer data)
{

    if (!SP_ACTIVE_DOCUMENT)
        return;
    sp_namedview_document_from_window (SP_ACTIVE_DESKTOP);
    sp_file_save_document (SP_ACTIVE_DOCUMENT);

}


/**
 *  Save a document, always displaying the SaveAs dialog.
 */
void
sp_file_save_as (gpointer object, gpointer data)
{

    if (!SP_ACTIVE_DOCUMENT)
        return;
    sp_namedview_document_from_window (SP_ACTIVE_DESKTOP);
    sp_file_save_dialog (SP_ACTIVE_DOCUMENT);
    //TODO: make this dependent on the success of the save operation
    sp_view_set_statusf_flash (SP_VIEW(SP_ACTIVE_DESKTOP), "Document saved.");

}




/*######################
## I M P O R T
######################*/

/**
 *  Import a resource.  Called by sp_file_import()
 */
static void
file_import (SPDocument *doc, const gchar *filename)
{
    if (filename && g_file_test (filename, G_FILE_TEST_IS_DIR)) {
        g_free (import_path);
        if (filename[strlen(filename) - 1] != G_DIR_SEPARATOR) {
            import_path = g_strconcat (filename, G_DIR_SEPARATOR_S, NULL);
        } else {
            import_path = g_strdup (filename);
        }
        return;
    }

    if (filename == NULL)
        return;

    import_path = g_dirname (filename);
    if (import_path) import_path = g_strconcat (import_path, G_DIR_SEPARATOR_S, NULL);

    SPRepr *rdoc = sp_document_repr_root (doc);

    const gchar *docbase = sp_repr_attr (rdoc, "sodipodi:docbase");
    const gchar *relname = sp_relative_path_from_path (filename, docbase);
    /* fixme: this should be implemented with mime types */
    const gchar *e = sp_extension_from_path (filename);

    if ((e == NULL) || (strcmp (e, "svg") == 0) || (strcmp (e, "xml") == 0)) {

        SPReprDoc *rnewdoc = sp_repr_read_file (filename, SP_SVG_NS_URI);
        if (rnewdoc == NULL)
            {
            /*
              We might need an error dialog here
              for failing to load an SVG document
            */
            return;
            }
        SPRepr *repr = sp_repr_document_root (rnewdoc);
        const gchar *style = sp_repr_attr (repr, "style");

        SPRepr *newgroup = sp_repr_new ("g");
        sp_repr_set_attr (newgroup, "style", style);

        for (SPRepr *child = repr->children; child != NULL; child = child->next) {
            SPRepr * newchild;
            newchild = sp_repr_duplicate (child);
            sp_repr_append_child (newgroup, newchild);
        }

        sp_repr_document_unref (rnewdoc);

        sp_document_add_repr (doc, newgroup);
        sp_repr_unref (newgroup);
        sp_document_done (doc);
        return;
    }

    if ((strcmp (e, "png" ) == 0) ||
        (strcmp (e, "jpg" ) == 0) ||
        (strcmp (e, "jpeg") == 0) ||
        (strcmp (e, "bmp" ) == 0) ||
        (strcmp (e, "gif" ) == 0) ||
        (strcmp (e, "tiff") == 0) ||
        (strcmp (e, "xpm" ) == 0)) {

        /* Try pixbuf */
        GError *err = NULL;
        GdkPixbuf *pb = gdk_pixbuf_new_from_file (filename, &err);
        if (pb) {
            /* We are readable */
            SPRepr *repr = sp_repr_new ("image");
            sp_repr_set_attr (repr, "xlink:href", relname);
            sp_repr_set_attr (repr, "sodipodi:absref", filename);
            sp_repr_set_double (repr, "width", gdk_pixbuf_get_width (pb));
            sp_repr_set_double (repr, "height", gdk_pixbuf_get_height (pb));
            sp_document_add_repr (doc, repr);
            sp_repr_unref (repr);
            sp_document_done (doc);
            gdk_pixbuf_unref (pb);
        } else {
            //error stuff here
            if (err) {
                gchar *text;
                text = g_strdup_printf(
                      _("Unable to import image '%s': %s"),
                      filename, err->message);
                sp_ui_error_dialog (text);
                g_free (text);
                g_error_free (err);
            }
        }
    }
}


/**
 *  Display an Open dialog, import a resource of OK pressed.
 */
void
sp_file_import (GtkWidget * widget)
{

    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (!SP_IS_DOCUMENT(doc))
        return;

#ifdef WIN32
    Inkscape::UI::Dialogs::FileOpenDialog *dlg =
        new Inkscape::UI::Dialogs::FileOpenDialog(
                 (const char *)import_path,
                 Inkscape::UI::Dialogs::IMPORT_TYPES,
                 (const char *)_("Select file to import"));
    char *fileName = dlg->show();
    delete dlg;
    if (fileName) {
        file_import (doc, fileName);
        g_free (fileName);
    }
#else
    GtkWidget *w = gtk_file_selection_new (
              _("Select file to import"));
    gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (w));
    if (import_path) gtk_file_selection_set_filename (GTK_FILE_SELECTION (w), import_path);
    gtk_window_set_modal (GTK_WINDOW (w), TRUE);
    sp_transientize (w);
    int b = gtk_dialog_run (GTK_DIALOG (w));

    if (b == GTK_RESPONSE_OK) {
        const gchar *filename;
        filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (w));
        file_import (doc, filename);
    }

    gtk_widget_destroy (w);
#endif
}



/*######################
## E X P O R T
######################*/

/**
 *
 */
void
sp_file_export_dialog (void *widget)
{
    sp_export_dialog ();
}

#include <display/nr-arena-item.h>
#include <display/nr-arena.h>

struct SPEBP {
    int width, height, sheight;
    guchar r, g, b, a;
    NRArenaItem *root;
    guchar *px;
    unsigned int (*status) (float, void *);
    void *data;
};


/**
 *
 */
static int
sp_export_get_rows (const guchar **rows, int row, int num_rows, void *data)
{

    struct SPEBP *ebp = (struct SPEBP *) data;

    if (ebp->status) {
        if (!ebp->status ((float) row / ebp->height, ebp->data)) return 0;
    }

    num_rows = MIN (num_rows, ebp->sheight);
    num_rows = MIN (num_rows, ebp->height - row);

    /* Set area of interest */
    NRRectL bbox;
    bbox.x0 = 0;
    bbox.y0 = row;
    bbox.x1 = ebp->width;
    bbox.y1 = row + num_rows;
    /* Update to renderable state */
    NRGC gc;
    nr_matrix_set_identity (&gc.transform);
    nr_arena_item_invoke_update (ebp->root, &bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);

    NRPixBlock pb;
    nr_pixblock_setup_extern (&pb, NR_PIXBLOCK_MODE_R8G8B8A8N, bbox.x0, bbox.y0, bbox.x1, bbox.y1, ebp->px, 4 * ebp->width, FALSE, FALSE);

    for (int r = 0; r < num_rows; r++) {
        guchar *p = NR_PIXBLOCK_PX (&pb) + r * pb.rs;
        for (int c = 0; c < ebp->width; c++) {
            *p++ = ebp->r;
            *p++ = ebp->g;
            *p++ = ebp->b;
            *p++ = ebp->a;
        }
    }

    /* Render */
    nr_arena_item_invoke_render (ebp->root, &bbox, &pb, 0);

    for (int r = 0; r < num_rows; r++) {
        rows[r] = NR_PIXBLOCK_PX (&pb) + r * pb.rs;
    }

    nr_pixblock_release (&pb);

    return num_rows;
}



/**
 *  Render the SVG drawing onto a PNG raster image, then save to
 *  a file.
 */
void
sp_export_png_file (SPDocument *doc, const gchar *filename,
            double x0, double y0, double x1, double y1,
            unsigned int width, unsigned int height,
            unsigned long bgcolor,
            unsigned int (*status) (float, void*), void *data)
{

    g_return_if_fail (doc != NULL);
    g_return_if_fail (SP_IS_DOCUMENT (doc));
    g_return_if_fail (filename != NULL);
    g_return_if_fail (width >= 1);
    g_return_if_fail (height >= 1);

    sp_document_ensure_up_to_date (doc);

    /* Go to document coordinates */
    gdouble t = y0;
    y0 = sp_document_height (doc) - y1;
    y1 = sp_document_height (doc) - t;

    /*
     * 1) a[0] * x0 + a[2] * y1 + a[4] = 0.0
     * 2) a[1] * x0 + a[3] * y1 + a[5] = 0.0
     * 3) a[0] * x1 + a[2] * y1 + a[4] = width
     * 4) a[1] * x0 + a[3] * y0 + a[5] = height
     * 5) a[1] = 0.0;
     * 6) a[2] = 0.0;
     *
     * (1,3) a[0] * x1 - a[0] * x0 = width
     * a[0] = width / (x1 - x0)
     * (2,4) a[3] * y0 - a[3] * y1 = height
     * a[3] = height / (y0 - y1)
     * (1) a[4] = -a[0] * x0
     * (2) a[5] = -a[3] * y1
     */

    NRMatrix affine;
    affine.c[0] = width / ((x1 - x0) * 1.25);
    affine.c[1] = 0.0;
    affine.c[2] = 0.0;
    affine.c[3] = height / ((y1 - y0) * 1.25);
    affine.c[4] = -affine.c[0] * x0 * 1.25;
    affine.c[5] = -affine.c[3] * y0 * 1.25;

    //SP_PRINT_MATRIX ("SVG2PNG", &affine);

    struct SPEBP ebp;
    ebp.width  = width;
    ebp.height = height;
    ebp.r      = NR_RGBA32_R (bgcolor);
    ebp.g      = NR_RGBA32_G (bgcolor);
    ebp.b      = NR_RGBA32_B (bgcolor);
    ebp.a      = NR_RGBA32_A (bgcolor);

    /* Create new arena */
    NRArena *arena = (NRArena *) nr_object_new (NR_TYPE_ARENA);
    unsigned int dkey = sp_item_display_key_new (1);

    /* Create ArenaItem and set transform */
    ebp.root = sp_item_invoke_show (SP_ITEM (sp_document_root (doc)), arena, dkey, SP_ITEM_SHOW_PRINT);
    nr_arena_item_set_transform (ebp.root, &affine);

    ebp.status = status;
    ebp.data   = data;

    if ((width < 256) || ((width * height) < 32768)) {
        ebp.px = nr_pixelstore_64K_new (FALSE, 0);
        ebp.sheight = 65536 / (4 * width);
        sp_png_write_rgba_striped (filename, width, height, sp_export_get_rows, &ebp);
        nr_pixelstore_64K_free (ebp.px);
    } else {
        ebp.px = nr_new (guchar, 4 * 64 * width);
        ebp.sheight = 64;
        sp_png_write_rgba_striped (filename, width, height, sp_export_get_rows, &ebp);
        nr_free (ebp.px);
    }

    /* Free Arena and ArenaItem */
    sp_item_invoke_hide (SP_ITEM (sp_document_root (doc)), dkey);
    nr_arena_item_unref (ebp.root);
    nr_object_unref ((NRObject *) arena);
}


/*######################
## P R I N T
######################*/


/**
 *  Print the current document, if any.
 */
void
sp_file_print (void)
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_document (doc, FALSE);
}


/**
 *  Print the current document, if any.  Do not use
 *  the machine's print drivers.
 */
void
sp_file_print_direct (void)
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_document (doc, TRUE);
}


/**
 * Display what the drawing would look like, if
 * printed.
 */
void
sp_file_print_preview (gpointer object, gpointer data)
{

    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_preview_document (doc);

}

