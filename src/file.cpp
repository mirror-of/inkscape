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
 * Copyright (C) 2004 David Turner
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

#include "macros.h"
#include "xml/repr-private.h"
#include "xml/repr-get-children.h"
#include "document.h"
#include "document-private.h"
#include "selection-chemistry.h"
#include "view.h"
#include "dir-util.h"
#include "helper/png-write.h"
#include "dialogs/export.h"
#include "helper/sp-intl.h"
#include "inkscape.h"
#include "desktop.h"
#include "selection.h"
#include "sp-image.h"
#include "interface.h"
#include "print.h"
#include "file.h"
#include "dialogs/dialog-events.h"
#include "message-stack.h"

#include "dialogs/filedialog.h"
#include "prefs-utils.h"
#include "path-prefix.h"

#include "sp-namedview.h"
#include "desktop-handles.h"

#include "extension/extension.h"
/* #include "extension/menu.h"  */
#include "extension/system.h"


/**
 * 'Current' paths.  Used to remember which directory
 * had the last file accessed.
 * Static globals are evil.  This will be gone soon
 * as C++ification continues
 */
static gchar *import_path = NULL;



/*######################
## N E W
######################*/

/**
 * Create a blank document and add it to the desktop
 */
void
sp_file_new(const gchar *templ)
{
    SPDocument *doc = sp_document_new(templ, TRUE, TRUE, true);
    g_return_if_fail(doc != NULL);

    SPViewWidget *dtw = sp_desktop_widget_new(sp_document_namedview(doc, NULL));
    sp_document_unref(doc);
    g_return_if_fail(dtw != NULL);

    sp_create_window(dtw, TRUE);
    sp_namedview_window_from_document(SP_DESKTOP(dtw->view));
}

void
sp_file_new_default ()
{
    char *default_template = g_build_filename(INKSCAPE_TEMPLATESDIR, "/default.svg", NULL);
    if (g_file_test (default_template, G_FILE_TEST_IS_REGULAR)) {
        sp_file_new (default_template);
    } else {
        sp_file_new (NULL);
    }
}


/*######################
## D E L E T E
######################*/

/**
 *  Perform document closures preceding an exit()
 */
void
sp_file_exit()
{
    sp_ui_close_all();
    // no need to call inkscape_exit here; last document being closed will take care of that
}


/*######################
## O P E N
######################*/

/**
 *  Open a file, add the document to the desktop
 */
bool
sp_file_open(gchar const *uri, Inkscape::Extension::Extension *key)
{
    SPDocument *doc;
    try {
        doc = Inkscape::Extension::open(key, uri);
    } catch (Inkscape::Extension::Input::no_extension_found &e) {
        doc = NULL;
    } catch (Inkscape::Extension::Input::open_failed &e) {
        doc = NULL;
    }

    if (doc) {
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        SPDocument *existing = desktop ? SP_DT_DOCUMENT(desktop) : NULL;
        if (existing && existing->virgin) {
            // If the current desktop is empty, open the document there
            sp_desktop_change_document(desktop, doc);
        }
        else {
            // create a whole new desktop and window
            SPViewWidget *dtw = sp_desktop_widget_new(sp_document_namedview(doc, NULL));
            sp_create_window(dtw, TRUE);
            desktop = SP_DESKTOP(dtw->view);
        }
        // everyone who cares now has a reference, get rid of ours
        sp_document_unref(doc); 
        // resize the window to match the document properties
        // (this may be redundant for new windows... if so, move to the "virgin"
        //  section above)
        sp_namedview_window_from_document(desktop);
        doc->virgin = FALSE;

        prefs_set_recent_file(SP_DOCUMENT_URI(doc), SP_DOCUMENT_NAME(doc));

        return TRUE;
    } else {
        gchar *text = g_strdup_printf(_("Failed to load the requested file %s"), uri);
        sp_ui_error_dialog(text);
        g_free(text);
        return FALSE;
    }
}

/**
 *  Handle prompting user for "do you want to revert"?  Revert on "OK"
 */
void
sp_file_revert_dialog()
{
    SPDesktop  *desktop = SP_ACTIVE_DESKTOP;
    g_assert(desktop != NULL);

    SPDocument *doc = SP_DT_DOCUMENT(desktop);
    g_assert(doc != NULL);

    SPRepr     *repr = sp_document_repr_root(doc);
    g_assert(repr != NULL);

    gchar const *uri = doc->uri;
    if (!uri) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved yet.  Cannot revert."));
        return;
    }

    if (sp_repr_attr(repr, "sodipodi:modified") != NULL) {
        bool reverted = FALSE;

        gchar *text = g_strdup_printf(_("Changes will be lost!  Are you sure you want to reload document %s?"), uri);

        GtkWidget *dialog = gtk_message_dialog_new(
                GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(desktop->owner))),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_YES_NO,
                text);
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_free(text);

        if (response == GTK_RESPONSE_YES) {
            // allow overwriting of current document
            doc->virgin=TRUE;
            reverted = sp_file_open(uri,NULL);
        }

        if (reverted) {
            desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Document reverted."));
        }
        else {
            desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not reverted."));
        }
    }
    else {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Document not modified.  No need to revert."));
    }
}


static Inkscape::UI::Dialogs::FileOpenDialog *openDialogInstance = NULL;

/**
 *  Display an file Open selector.  Open a document if OK is pressed.
 */
void
sp_file_open_dialog(gpointer object, gpointer data)
{
    gchar * open_path = NULL;
    gchar * open_path2 = NULL;

    open_path = g_strdup(prefs_get_string_attribute("dialogs.open", "path"));
    if (open_path != NULL && open_path[0] == '\0') {
        g_free(open_path);
        open_path = NULL;
    }
    if (open_path && !g_file_test(open_path, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
        g_free(open_path);
        open_path = NULL;
    }
    if (open_path == NULL)
        open_path = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, NULL);

    if (!openDialogInstance)
        {
        openDialogInstance = 
              Inkscape::UI::Dialogs::FileOpenDialog::create(
                 (char const *)open_path,
                 Inkscape::UI::Dialogs::SVG_TYPES,
                 (char const *)_("Select file to open"));
        }
    bool const success = openDialogInstance->show();
    gchar *fileName = ( success
                        ? g_strdup(openDialogInstance->getFilename())
                        : NULL );
    Inkscape::Extension::Extension *selection =
            openDialogInstance->getSelectionType();
    g_free(open_path);

    if (!success) return;
    if (fileName) {
        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError *error;
        gchar *newFileName = g_filename_to_utf8( fileName,
                                                 -1,
                                                 &bytesRead,
                                                 &bytesWritten,
                                                 &error);
        if ( newFileName != NULL )
        {
            g_free(fileName);
            fileName = newFileName;
        }
        else
        {
            // TODO: bulia, please look over
            g_warning( "ERROR CONVERTING OPEN FILENAME TO UTF-8" );
        }


        if ( !g_utf8_validate(fileName, -1, NULL) )
        {
            // TODO: bulia, please look over
            g_warning( "INPUT FILENAME IS NOT UTF-8" );
        }


        open_path = g_dirname(fileName);
        open_path2 = g_strconcat(open_path, G_DIR_SEPARATOR_S, NULL);
        prefs_set_string_attribute("dialogs.open", "path", open_path2);
        g_free(open_path);
        g_free(open_path2);

        sp_file_open(fileName, selection);
        g_free(fileName);
    }

    return;
}


/*######################
## V A C U U M
######################*/

/**
 * Remove unreferenced defs from the defs section of the document.
 */


void
sp_file_vacuum()
{

    SPDocument* doc = SP_ACTIVE_DOCUMENT;
    SPDefs *defs = SP_ROOT(SP_DOCUMENT_ROOT(doc))->defs;

    for ( SPObject* def = defs->object.firstChild () ;
          def ; def = SP_OBJECT_NEXT (def) )
    {

        /* fixme: some inkscape-internal nodes in the future might not be collectable */
        def->requestOrphanCollection ();
    }

    sp_document_done (doc);
}



/*######################
## S A V E
######################*/

/**
 * This 'save' function called by the others below
 */
static bool
file_save(SPDocument *doc, gchar const *uri, Inkscape::Extension::Extension *key, bool saveas)
{
    if (!doc || !uri) //Safety check
        return FALSE;

    try {
        Inkscape::Extension::save(key, doc, uri,
                                  saveas && prefs_get_int_attribute("dialogs.save_as", "append_extension", 1),
                                  saveas, TRUE); // save officially, with inkscape: attributes set
    } catch (Inkscape::Extension::Output::no_extension_found &e) {
        gchar *text = g_strdup_printf(_("No Inkscape extension found to save document (%s).  This may have been caused by an unknown filename extension."), uri);
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved."));
        sp_ui_error_dialog(text);
        g_free(text);
        return FALSE;
    } catch (Inkscape::Extension::Output::save_failed &e) {
        gchar *text = g_strdup_printf(_("File %s could not be saved."), uri);
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved."));
        sp_ui_error_dialog(text);
        g_free(text);
        return FALSE;
    } catch (Inkscape::Extension::Output::no_overwrite &e) {
        return sp_file_save_dialog(doc);
    }

    SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Document saved."));
    return TRUE;
}

static Inkscape::UI::Dialogs::FileSaveDialog *saveDialogInstance = NULL;

/**
 *  Display a SaveAs dialog.  Save the document if OK pressed.
 */
gboolean
sp_file_save_dialog(SPDocument *doc)
{
    SPRepr *repr = sp_document_repr_root(doc);
    gchar const *default_extension = NULL;
    gchar *save_loc;
    Inkscape::Extension::Output *extension;
    gchar *save_path = NULL;

    default_extension = sp_repr_attr(repr, "inkscape:output_extension");
    if (default_extension == NULL) {
        default_extension = prefs_get_string_attribute("dialogs.save_as", "default");
    }
    // printf("Extension: %s\n", default_extension);

    if (doc->uri == NULL) {
        int i = 1;
        char const *filename_extension;
        char *temp_filename;

        extension = dynamic_cast<Inkscape::Extension::Output *>(Inkscape::Extension::db.get(default_extension));
        if (extension == NULL) {
            filename_extension = ".svg";
        } else {
            filename_extension = extension->get_extension();
        }

        save_path = g_strdup(prefs_get_string_attribute("dialogs.save_as", "path"));
        if (save_path != NULL && save_path[0] == '\0') {
            g_free(save_path);
            save_path = NULL;
        }
        if (save_path && !g_file_test(save_path, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
            g_free(save_path);
            save_path = NULL;
        }
        if (save_path == NULL)
            save_path = g_strdup(g_get_home_dir());
        temp_filename = g_strdup_printf(_("drawing%s"), filename_extension);
        save_loc = g_build_filename(save_path, temp_filename, NULL);
        g_free(temp_filename);

        while (g_file_test(save_loc, G_FILE_TEST_EXISTS)) {
            g_free(save_loc);
            temp_filename = g_strdup_printf(_("drawing-%d%s"), i++, filename_extension);
            save_loc = g_build_filename(save_path, temp_filename, NULL);
            g_free(temp_filename);
        }
    } else {
        save_loc = g_strdup(doc->uri); /* \todo should use a getter */
    }

    { // convert save_lock from utf-8 to locale
        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError* error = NULL;
        gchar* save_loc_local = g_filename_from_utf8 ( save_loc, -1, &bytesRead, &bytesWritten, &error);

        if ( save_loc_local != NULL ) {
            g_free(save_loc);
            save_loc = save_loc_local;
        } else {
            g_warning( "Error converting save filename stored in the file to locale encoding.");
        }
    }

    if (!saveDialogInstance)
        {
        saveDialogInstance =
             Inkscape::UI::Dialogs::FileSaveDialog::create(
                 (char const *) save_loc,
                 Inkscape::UI::Dialogs::SVG_TYPES,
                 (char const *) _("Select file to save to"),
                 default_extension
            );
        }
    bool success = saveDialogInstance->show();
    char *fileName = ( success
                       ? g_strdup(saveDialogInstance->getFilename())
                       : NULL );
    Inkscape::Extension::Extension *selectionType =
        saveDialogInstance->getSelectionType();
    g_free(save_loc);
    g_free(save_path);
    if (!success) return success;

    if (fileName && *fileName) {
        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError *error;
        gchar *newFileName = g_filename_to_utf8( fileName,
                                                 -1,
                                                 &bytesRead,
                                                 &bytesWritten,
                                                 &error);
        if ( newFileName != NULL )
        {
            g_free(fileName);
            fileName = newFileName;
        }
        else
        {
            g_warning( "Error converting save filename to UTF-8." );
        }

        if ( !g_utf8_validate(fileName, -1, NULL) )
        {
            // TODO: bulia, please look over
            g_warning( "The filename is not UTF-8." );
        }

        success = file_save(doc, fileName, selectionType, TRUE);

        if (success)
            prefs_set_recent_file(SP_DOCUMENT_URI(doc), SP_DOCUMENT_NAME(doc));

        save_path = g_dirname(fileName);
        prefs_set_string_attribute("dialogs.save_as", "path", save_path);
        g_free(save_path);

        g_free(fileName);
        return success;
    } else {
        return FALSE;
    }
}


/**
 * Save a document, displaying a SaveAs dialog if necessary.
 */
gboolean
sp_file_save_document(SPDocument *doc)
{
    gboolean success = TRUE;

    SPRepr *repr = sp_document_repr_root(doc);

    gchar const *fn = sp_repr_attr(repr, "sodipodi:modified");
    if (fn != NULL) {
        if (doc->uri == NULL || 
            sp_repr_attr(repr, "inkscape:output_extension") == NULL) {
            return sp_file_save_dialog(doc);
        } else {
            fn = g_strdup(doc->uri);			
            gchar const *ext = sp_repr_attr(repr, "inkscape:output_extension");
            success = file_save(doc, fn, Inkscape::Extension::db.get(ext), FALSE);
            g_free((void *) fn);
        }
    } else {
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No changes need to be saved."));
        success = TRUE;
    }
    
    return success;
}


/**
 * Save a document.
 */
bool
sp_file_save(gpointer object, gpointer data)
{
    if (!SP_ACTIVE_DOCUMENT)
        return false;
    sp_namedview_document_from_window(SP_ACTIVE_DESKTOP);
    return sp_file_save_document(SP_ACTIVE_DOCUMENT);
}


/**
 *  Save a document, always displaying the SaveAs dialog.
 */
bool
sp_file_save_as(gpointer object, gpointer data)
{
    if (!SP_ACTIVE_DOCUMENT)
        return false;
    sp_namedview_document_from_window(SP_ACTIVE_DESKTOP);
    return sp_file_save_dialog(SP_ACTIVE_DOCUMENT);
}




/*######################
## I M P O R T
######################*/

/**
 *  Import a resource.  Called by sp_file_import()
 */
static void
file_import(SPDocument *in_doc, gchar const *uri, Inkscape::Extension::Extension *key)
{
    SPDocument *doc;	
    try {
        doc = Inkscape::Extension::open(key, uri);
    } catch (Inkscape::Extension::Input::no_extension_found &e) {
        doc = NULL;
    } catch (Inkscape::Extension::Input::open_failed &e) {
        doc = NULL;
    }

    if (doc != NULL) {
        // the import extension has passed us a document, now we need to embed it into our document

        // move imported defs to our document's defs
        SPObject *in_defs = SP_DOCUMENT_DEFS(in_doc);
        SPObject *defs = SP_DOCUMENT_DEFS(doc);
        SPRepr *last_def = sp_repr_last_child(SP_OBJECT_REPR(in_defs));
        for (SPObject *child = sp_object_first_child(defs); child != NULL; child = SP_OBJECT_NEXT(child) ) {
            // FIXME: in case of id conflict, newly added thing will be re-ided and thus likely break a reference to it from imported stuff
            sp_repr_add_child(SP_OBJECT_REPR(in_defs), sp_repr_duplicate(SP_OBJECT_REPR(child)), last_def);
        }

        SPRepr *repr = sp_document_repr_root(doc);
        guint items_count = 0;
        for (SPObject *child = sp_object_first_child(SP_DOCUMENT_ROOT(doc)); child != NULL; child = SP_OBJECT_NEXT(child) ) {
            if (SP_IS_ITEM(child))
                items_count ++;
        }
        gchar const *style = sp_repr_attr(repr, "style");

        SPObject *new_obj = NULL;

        if (style || items_count > 1) {
            // create group
            SPRepr *newgroup = sp_repr_new("g");
            sp_repr_set_attr(newgroup, "style", style);

            for (SPObject *child = sp_object_first_child(SP_DOCUMENT_ROOT(doc)); child != NULL; child = SP_OBJECT_NEXT(child) ) {
                if (SP_IS_ITEM(child)) {
                    sp_repr_append_child(newgroup, sp_repr_duplicate(SP_OBJECT_REPR(child)));
                }
            }

            new_obj = SP_DOCUMENT_ROOT(in_doc)->appendChildRepr(newgroup);
            sp_repr_unref(newgroup);
        } else {
            // just add one item
            for (SPObject *child = sp_object_first_child(SP_DOCUMENT_ROOT(doc)); child != NULL; child = SP_OBJECT_NEXT(child) ) {
                if (SP_IS_ITEM(child)) {
                    SPRepr *newitem = sp_repr_duplicate(SP_OBJECT_REPR(child));
                    new_obj = SP_DOCUMENT_ROOT(in_doc)->appendChildRepr(newitem);
                }
            }
        }

        // select and move the imported item
        if (new_obj && SP_IS_ITEM(new_obj)) {
            SPDesktop *desktop = SP_ACTIVE_DESKTOP;
            SPSelection *selection = SP_DT_SELECTION(desktop);
            selection->setItem(SP_ITEM(new_obj));
            sp_document_ensure_up_to_date(SP_DT_DOCUMENT(desktop));
            NR::Point m( sp_desktop_point(desktop) - selection->bounds().midpoint() );
            sp_selection_move_relative(selection, m[NR::X], m[NR::Y]);
        }

        sp_document_unref(doc);
        sp_document_done(in_doc);

    } else {
        gchar *text = g_strdup_printf(_("Failed to load the requested file %s"), uri);
        sp_ui_error_dialog(text);
        g_free(text);
    }

    return;
}


static Inkscape::UI::Dialogs::FileOpenDialog *importDialogInstance = NULL;

/**
 *  Display an Open dialog, import a resource if OK pressed.
 */
void
sp_file_import(GtkWidget *widget)
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (!SP_IS_DOCUMENT(doc))
        return;

    if (!importDialogInstance)
        {
        importDialogInstance =
             Inkscape::UI::Dialogs::FileOpenDialog::create(
                 (char const *)import_path,
                 Inkscape::UI::Dialogs::IMPORT_TYPES,
                 (char const *)_("Select file to import"));
        }
    bool success = importDialogInstance->show();
    char *fileName = ( success
                       ? g_strdup(importDialogInstance->getFilename())
                       : NULL );
    Inkscape::Extension::Extension *selection =
        importDialogInstance->getSelectionType();

    if (!success) return;
    if (fileName) {
        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError *error;
        gchar *newFileName = g_filename_to_utf8( fileName,
                                                 -1,
                                                 &bytesRead,
                                                 &bytesWritten,
                                                 &error);
        if ( newFileName != NULL )
        {
            g_free(fileName);
            fileName = newFileName;
        }
        else
        {
            // TODO: bulia, please look over
            g_warning( "ERROR CONVERTING OPEN FILENAME TO UTF-8" );
        }


        if ( !g_utf8_validate(fileName, -1, NULL) )
        {
            // TODO: bulia, please look over
            g_warning( "INPUT FILENAME IS NOT UTF-8" );
        }

        g_free(import_path);
        import_path = g_dirname(fileName);
        if (import_path) import_path = g_strconcat(import_path, G_DIR_SEPARATOR_S, NULL);

        file_import(doc, fileName, selection);
        g_free(fileName);
    }

    return;
}



/*######################
## E X P O R T
######################*/

/**
 *
 */
void
sp_file_export_dialog(void *widget)
{
    sp_export_dialog();
}

#include <display/nr-arena-item.h>
#include <display/nr-arena.h>

struct SPEBP {
    int width, height, sheight;
    guchar r, g, b, a;
    NRArenaItem *root;
    guchar *px;
    unsigned (*status) (float, void *);
    void *data;
};


/**
 *
 */
static int
sp_export_get_rows(guchar const **rows, int row, int num_rows, void *data)
{
    struct SPEBP *ebp = (struct SPEBP *) data;

    if (ebp->status) {
        if (!ebp->status((float) row / ebp->height, ebp->data)) return 0;
    }

    num_rows = MIN(num_rows, ebp->sheight);
    num_rows = MIN(num_rows, ebp->height - row);

    /* Set area of interest */
    NRRectL bbox;
    bbox.x0 = 0;
    bbox.y0 = row;
    bbox.x1 = ebp->width;
    bbox.y1 = row + num_rows;
    /* Update to renderable state */
    NRGC gc(NULL);
    nr_matrix_set_identity(&gc.transform);
    nr_arena_item_invoke_update(ebp->root, &bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);

    NRPixBlock pb;
    nr_pixblock_setup_extern(&pb, NR_PIXBLOCK_MODE_R8G8B8A8N,
                             bbox.x0, bbox.y0, bbox.x1, bbox.y1,
                             ebp->px, 4 * ebp->width, FALSE, FALSE);

    for (int r = 0; r < num_rows; r++) {
        guchar *p = NR_PIXBLOCK_PX(&pb) + r * pb.rs;
        for (int c = 0; c < ebp->width; c++) {
            *p++ = ebp->r;
            *p++ = ebp->g;
            *p++ = ebp->b;
            *p++ = ebp->a;
        }
    }

    /* Render */
    nr_arena_item_invoke_render(ebp->root, &bbox, &pb, 0);

    for (int r = 0; r < num_rows; r++) {
        rows[r] = NR_PIXBLOCK_PX(&pb) + r * pb.rs;
    }

    nr_pixblock_release(&pb);

    return num_rows;
}



/**
 *  Render the SVG drawing onto a PNG raster image, then save to
 *  a file.
 */
void
sp_export_png_file(SPDocument *doc, gchar const *filename,
                   double x0, double y0, double x1, double y1,
                   unsigned width, unsigned height,
                   unsigned long bgcolor,
                   unsigned (*status)(float, void *),
                   void *data, bool force_overwrite,
                   SPItem *item_only)
{
    g_return_if_fail(doc != NULL);
    g_return_if_fail(SP_IS_DOCUMENT(doc));
    g_return_if_fail(filename != NULL);
    g_return_if_fail(width >= 1);
    g_return_if_fail(height >= 1);

    if (!force_overwrite && !sp_ui_overwrite_file(filename)) {
        return;
    }

    sp_document_ensure_up_to_date(doc);

    /* Go to document coordinates */
    gdouble t = y0;
    y0 = sp_document_height(doc) - y1;
    y1 = sp_document_height(doc) - t;

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

    //SP_PRINT_MATRIX("SVG2PNG", &affine);

    struct SPEBP ebp;
    ebp.width  = width;
    ebp.height = height;
    ebp.r      = NR_RGBA32_R(bgcolor);
    ebp.g      = NR_RGBA32_G(bgcolor);
    ebp.b      = NR_RGBA32_B(bgcolor);
    ebp.a      = NR_RGBA32_A(bgcolor);

    /* Create new arena */
    NRArena *arena = NRArena::create();
    unsigned dkey = sp_item_display_key_new(1);

    /* Create ArenaItem and set transform */
    if (item_only) {
        ebp.root = sp_item_invoke_show(item_only, arena, dkey, SP_ITEM_SHOW_PRINT);
    } else {
        ebp.root = sp_item_invoke_show(SP_ITEM(sp_document_root(doc)), arena, dkey, SP_ITEM_SHOW_PRINT);
    }
    nr_arena_item_set_transform(ebp.root, &affine);

    ebp.status = status;
    ebp.data   = data;

    if ((width < 256) || ((width * height) < 32768)) {
        ebp.px = nr_pixelstore_64K_new(FALSE, 0);
        ebp.sheight = 65536 / (4 * width);
        sp_png_write_rgba_striped(filename, width, height, sp_export_get_rows, &ebp);
        nr_pixelstore_64K_free(ebp.px);
    } else {
        ebp.px = nr_new(guchar, 4 * 64 * width);
        ebp.sheight = 64;
        sp_png_write_rgba_striped(filename, width, height, sp_export_get_rows, &ebp);
        nr_free(ebp.px);
    }

    /* Free Arena and ArenaItem */
    sp_item_invoke_hide(SP_ITEM(sp_document_root(doc)), dkey);
    nr_arena_item_unref(ebp.root);
    nr_object_unref((NRObject *) arena);
}


/*######################
## P R I N T
######################*/


/**
 *  Print the current document, if any.
 */
void
sp_file_print()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_document(doc, FALSE);
}


/**
 *  Print the current document, if any.  Do not use
 *  the machine's print drivers.
 */
void
sp_file_print_direct()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_document(doc, TRUE);
}


/**
 * Display what the drawing would look like, if
 * printed.
 */
void
sp_file_print_preview(gpointer object, gpointer data)
{

    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_preview_document(doc);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
