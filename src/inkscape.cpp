#define __INKSCAPE_C__

/*
 * Interface to main application
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 authors
 * g++ port Copyright (C) 2003 Nathan Hurst
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#define HAS_PROC_SELF_EXE  //to get path of executable
#else
#include <direct.h>
#define _WIN32_IE 0x0400
#define HAS_SHGetSpecialFolderPath
#define HAS_GetModuleFileName
#include <shlobj.h> //to get appdata path
#endif

#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>

#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmessagedialog.h>

#include "helper/sp-intl.h"
#include "helper/sp-marshal.h"
#include "xml/repr-private.h"

#include "shortcuts.h"

#include "file.h"
#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "event-context.h"
#include "inkscape.h"
#include "inkscape-private.h"

#ifdef WIN32
#define mkdir(d,m) _mkdir((d))
#endif

/* Backbones of configuration xml data */
#include "preferences-skeleton.h"

enum {
	MODIFY_SELECTION,
	CHANGE_SELECTION,
	SET_SELECTION,
	SET_EVENTCONTEXT,
	NEW_DESKTOP,
	DESTROY_DESKTOP,
	ACTIVATE_DESKTOP,
	DEACTIVATE_DESKTOP,
	NEW_DOCUMENT,
	DESTROY_DOCUMENT,
	COLOR_SET,
	SHUTDOWN_SIGNAL,
	DIALOGS_HIDE,
	DIALOGS_UNHIDE,
	LAST_SIGNAL
};

#define DESKTOP_IS_ACTIVE(d) ((d) == inkscape->desktops->data)

static void inkscape_class_init (Inkscape::ApplicationClass *klass);
static void inkscape_init (SPObject *object);
static void inkscape_dispose (GObject *object);

static void inkscape_activate_desktop_private (Inkscape::Application *inkscape, SPDesktop *desktop);
static void inkscape_deactivate_desktop_private (Inkscape::Application *inkscape, SPDesktop *desktop);

static void inkscape_init_config (SPReprDoc *doc, const gchar *config_name, const gchar *skeleton, unsigned int skel_size,
				  const gchar *e_mkdir,
				  const gchar *e_notdir,
				  const gchar *e_ccf,
				  const gchar *e_cwf);

static void inkscape_init_preferences (Inkscape::Application *inkscape);

static void inkscape_init_preferences (Inkscape::Application * inkscape);

static gchar *profile_path(const char *filename);

struct Inkscape::Application {
	GObject object;
	SPReprDoc *preferences;
	GSList *documents;
	GSList *desktops;
	gchar *argv0;
	gboolean dialogs_toggle;
};

struct Inkscape::ApplicationClass {
	GObjectClass object_class;

	/* Signals */
	void (* change_selection) (Inkscape::Application * inkscape, SPSelection * selection);
	void (* modify_selection) (Inkscape::Application * inkscape, SPSelection * selection, guint flags);
	void (* set_selection) (Inkscape::Application * inkscape, SPSelection * selection);
	void (* set_eventcontext) (Inkscape::Application * inkscape, SPEventContext * eventcontext);
	void (* new_desktop) (Inkscape::Application * inkscape, SPDesktop * desktop);
	void (* destroy_desktop) (Inkscape::Application * inkscape, SPDesktop * desktop);
	void (* activate_desktop) (Inkscape::Application * inkscape, SPDesktop * desktop);
	void (* deactivate_desktop) (Inkscape::Application * inkscape, SPDesktop * desktop);
	void (* new_document) (Inkscape::Application *inkscape, SPDocument *doc);
	void (* destroy_document) (Inkscape::Application *inkscape, SPDocument *doc);
	void (* color_set) (Inkscape::Application *inkscape, SPColor *color, double opacity);
	void (* shut_down) (Inkscape::Application *inkscape);
	void (* dialogs_hide) (Inkscape::Application *inkscape);
	void (* dialogs_unhide) (Inkscape::Application *inkscape);
};

static GObjectClass * parent_class;
static guint inkscape_signals[LAST_SIGNAL] = {0};

Inkscape::Application *inkscape = NULL;

static void (* segv_handler) (int) = NULL;

#ifdef WIN32
#define INKSCAPE_PROFILE_DIR "Inkscape"
#else
#define INKSCAPE_PROFILE_DIR ".inkscape"
#endif

#define PREFERENCES_FILE "preferences.xml"

GType
inkscape_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (Inkscape::ApplicationClass),
			NULL, NULL,
			(GClassInitFunc) inkscape_class_init,
			NULL, NULL,
			sizeof (Inkscape::Application),
			4,
			(GInstanceInitFunc) inkscape_init,
			NULL
		};
		type = g_type_register_static (G_TYPE_OBJECT, "Inkscape_Application", &info, (GTypeFlags)0);
	}
	return type;
}

static void
inkscape_class_init (Inkscape::ApplicationClass * klass)
{
	GObjectClass * object_class;

	object_class = (GObjectClass *) klass;

	parent_class = (GObjectClass *)g_type_class_peek_parent (klass);

	inkscape_signals[MODIFY_SELECTION] = g_signal_new ("modify_selection",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, modify_selection),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER_UINT,
							   G_TYPE_NONE, 2,
							   G_TYPE_POINTER, G_TYPE_UINT);
	inkscape_signals[CHANGE_SELECTION] = g_signal_new ("change_selection",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, change_selection),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	inkscape_signals[SET_SELECTION] =    g_signal_new ("set_selection",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, set_selection),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	inkscape_signals[SET_EVENTCONTEXT] = g_signal_new ("set_eventcontext",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, set_eventcontext),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	inkscape_signals[NEW_DESKTOP] =      g_signal_new ("new_desktop",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, new_desktop),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	inkscape_signals[DESTROY_DESKTOP] =  g_signal_new ("destroy_desktop",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, destroy_desktop),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	inkscape_signals[ACTIVATE_DESKTOP] = g_signal_new ("activate_desktop",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, activate_desktop),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	inkscape_signals[DEACTIVATE_DESKTOP] = g_signal_new ("deactivate_desktop",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, deactivate_desktop),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	inkscape_signals[NEW_DOCUMENT] =     g_signal_new ("new_document",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, new_document),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	inkscape_signals[DESTROY_DOCUMENT] = g_signal_new ("destroy_document",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, destroy_document),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	inkscape_signals[COLOR_SET] =        g_signal_new ("color_set",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, color_set),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER_DOUBLE,
							   G_TYPE_NONE, 2,
							   G_TYPE_POINTER, G_TYPE_DOUBLE);
	inkscape_signals[SHUTDOWN_SIGNAL] =        g_signal_new ("shut_down",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, shut_down),
							   NULL, NULL,
							   g_cclosure_marshal_VOID__VOID,
							   G_TYPE_NONE, 0);
	inkscape_signals[DIALOGS_HIDE] =        g_signal_new ("dialogs_hide",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, dialogs_hide),
							   NULL, NULL,
							   g_cclosure_marshal_VOID__VOID,
							   G_TYPE_NONE, 0);
	inkscape_signals[DIALOGS_UNHIDE] =        g_signal_new ("dialogs_unhide",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (Inkscape::ApplicationClass, dialogs_unhide),
							   NULL, NULL,
							   g_cclosure_marshal_VOID__VOID,
							   G_TYPE_NONE, 0);

	object_class->dispose = inkscape_dispose;

	klass->activate_desktop = inkscape_activate_desktop_private;
	klass->deactivate_desktop = inkscape_deactivate_desktop_private;
}

static void
inkscape_init (SPObject * object)
{
	if (!inkscape) {
		inkscape = (Inkscape::Application *) object;
	} else {
		g_assert_not_reached ();
	}

	inkscape->preferences = sp_repr_read_mem (preferences_skeleton, PREFERENCES_SKELETON_SIZE, NULL);

	inkscape->documents = NULL;
	inkscape->desktops = NULL;
}

static void
inkscape_dispose (GObject *object)
{
	Inkscape::Application *inkscape;

	inkscape = (Inkscape::Application *) object;

	while (inkscape->documents) {
		g_object_unref (G_OBJECT (inkscape->documents->data));
	}

	g_assert (!inkscape->desktops);

	if (inkscape->preferences) {
		/* fixme: This is not the best place */
		inkscape_save_preferences (inkscape);
		sp_repr_document_unref (inkscape->preferences);
		inkscape->preferences = NULL;
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);

	gtk_main_quit ();
}

void
inkscape_ref (void)
{
	g_object_ref (G_OBJECT (inkscape));
}

void
inkscape_unref (void)
{
	g_object_unref (G_OBJECT (inkscape));
}

static void
inkscape_activate_desktop_private (Inkscape::Application *inkscape, SPDesktop *desktop)
{
	sp_desktop_set_active (desktop, TRUE);
}

static void
inkscape_deactivate_desktop_private (Inkscape::Application *inkscape, SPDesktop *desktop)
{
	sp_desktop_set_active (desktop, FALSE);
}

/* fixme: This is EVIL, and belongs to main after all */

#define SP_INDENT 8

static void
inkscape_segv_handler (int signum)
{
	static gint recursion = FALSE;
	GSList *savednames, *failednames, *l;
	const gchar *home;
	gint count, nllen, len, pos;
	time_t sptime;
	struct tm *sptm;
	gchar sptstr[256];
	GtkWidget *msgbox;

	/* Kill loops */
	if (recursion) abort ();
	recursion = TRUE;

	g_warning ("Emergency save activated");

	home = g_get_home_dir ();
	sptime = time (NULL);
	sptm = localtime (&sptime);
	strftime (sptstr, 256, "%Y_%m_%d_%H_%M_%S", sptm);

	count = 0;
	savednames = NULL;
	failednames = NULL;
	for (l = inkscape->documents; l != NULL; l = l->next) {
		SPDocument *doc;
		SPRepr *repr;
		doc = (SPDocument *) l->data;
		repr = sp_document_repr_root (doc);
		if (sp_repr_attr (repr, "sodipodi:modified")) {
			const gchar *docname, *d0, *d;
			gchar n[64], c[1024];
			FILE *file;

			/* originally, the document name was retrieved from
			 * the sodipod:docname attribute */
			docname = doc->name;
			if (docname) {
				/* fixme: Quick hack to remove emergency file suffix */
				d0 = strrchr ((char*)docname, '.');
				if (d0 && (d0 > docname)) {
					d0 = strrchr ((char*)(d0 - 1), '.');
					if (d0 && (d0 > docname)) {
						d = d0;
						while (isdigit (*d) || (*d == '.') || (*d == '_')) d += 1;
						if (*d) {
							memcpy (n, docname, MIN (d0 - docname - 1, 64));
							n[64] = '\0';
							docname = n;
						}
					}
				}
			}

			if (!docname || !*docname) docname = "emergency";
			g_snprintf (c, 1024, "%s/.inkscape/%.256s.%s.%d", home, docname, sptstr, count);
			file = fopen (c, "w");
			if (!file) {
				g_snprintf (c, 1024, "%s/inkscape-%.256s.%s.%d", home, docname, sptstr, count);
				file = fopen (c, "w");
			}
			if (!file) {
				g_snprintf (c, 1024, "/tmp/inkscape-%.256s.%s.%d", docname, sptstr, count);
				file = fopen (c, "w");
			}
			if (file) {
				sp_repr_save_stream (sp_repr_document (repr), file);
				savednames = g_slist_prepend (savednames, g_strdup (c));
				fclose (file);
			} else {
				docname = sp_repr_attr (repr, "sodipodi:docname");
				failednames = g_slist_prepend (failednames, (docname) ? g_strdup (docname) : g_strdup (_("Untitled document")));
			}
			count++;
		}
	}

	savednames = g_slist_reverse (savednames);
	failednames = g_slist_reverse (failednames);
	if (savednames) {
		fprintf (stderr, "\nEmergency save locations:\n");
		for (l = savednames; l != NULL; l = l->next) {
			fprintf (stderr, "  %s\n", (gchar *) l->data);
		}
	}
	if (failednames) {
		fprintf (stderr, "Failed to do emergency save for:\n");
		for (l = failednames; l != NULL; l = l->next) {
			fprintf (stderr, "  %s\n", (gchar *) l->data);
		}
	}

	if (inkscape->preferences) {
		inkscape_save_preferences (inkscape);
	}

	g_print ("Emergency save completed. Inkscape will close now.\n");
	g_print ("If you can reproduce this crash, please file a bug at www.inkscape.org\n");
      	g_print ("with a detailed description of the steps leading to the crash, so we can fix it.\n");

	/* Show nice dialog box */

	char const *istr = N_("Inkscape encountered an internal error and will close now.\n");
	char const *sstr = N_("Automatic backups of unsaved documents were done to following locations:\n");
	char const *fstr = N_("Automatic backup of following documents failed:\n");
	nllen = strlen ("\n");
	len = strlen (istr) + strlen (sstr) + strlen (fstr);
	for (l = savednames; l != NULL; l = l->next) {
		len = len + SP_INDENT + strlen ((gchar *) l->data) + nllen;
	}
	for (l = failednames; l != NULL; l = l->next) {
		len = len + SP_INDENT + strlen ((gchar *) l->data) + nllen;
	}
	len += 1;
	gchar *b = g_new (gchar, len);
	pos = 0;
	len = strlen (istr);
	memcpy (b + pos, istr, len);
	pos += len;
	if (savednames) {
		len = strlen (sstr);
		memcpy (b + pos, sstr, len);
		pos += len;
		for (l = savednames; l != NULL; l = l->next) {
			memset (b + pos, ' ', SP_INDENT);
			pos += SP_INDENT;
			len = strlen ((gchar *) l->data);
			memcpy (b + pos, l->data, len);
			pos += len;
			memcpy (b + pos, "\n", nllen);
			pos += nllen;
		}
	}
	if (failednames) {
		len = strlen (fstr);
		memcpy (b + pos, fstr, len);
		pos += len;
		for (l = failednames; l != NULL; l = l->next) {
			memset (b + pos, ' ', SP_INDENT);
			pos += SP_INDENT;
			len = strlen ((gchar *) l->data);
			memcpy (b + pos, l->data, len);
			pos += len;
			memcpy (b + pos, "\n", nllen);
			pos += nllen;
		}
	}
	*(b + pos) = '\0';
	msgbox = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", b);
	gtk_dialog_run (GTK_DIALOG (msgbox));
	gtk_widget_destroy (msgbox);
	g_free (b);

	(* segv_handler) (signum);
}

Inkscape::Application *
inkscape_application_new (const gchar *argv0)
{
	Inkscape::Application *sp;

	sp = (Inkscape::Application *)g_object_new (SP_TYPE_INKSCAPE, NULL);
	/* fixme: load application defaults */

#ifndef WIN32
	segv_handler = signal (SIGSEGV, inkscape_segv_handler);
	signal (SIGFPE, inkscape_segv_handler);
	signal (SIGILL, inkscape_segv_handler);
#endif

	sp->argv0 = g_strdup(argv0);

	return sp;
}

/* Preference management */
/* We use '.' as separator */

static void
inkscape_load_config (const gchar *filename, SPReprDoc *config, const gchar *skeleton, unsigned int skel_size,
		      const gchar *e_notreg, const gchar *e_notxml, const gchar *e_notsp)
{
	gchar *fn;
	GtkWidget * w;
	SPReprDoc * doc;
	SPRepr * root;

	fn = profile_path(filename);
	if (!g_file_test(fn, G_FILE_TEST_EXISTS)) {
		/* No such file */
		inkscape_init_preferences (INKSCAPE);
		g_free (fn);
		return;
	}

	if (!g_file_test(fn, G_FILE_TEST_IS_REGULAR)) {
		/* Not a regular file */
		w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_notreg, fn);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		g_free (fn);
		return;
	}

	doc = sp_repr_read_file (fn, NULL);
	if (doc == NULL) {
		/* Not an valid xml file */
		w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_notxml, fn);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		g_free (fn);
		return;
	}

	root = sp_repr_document_root (doc);
	if (strcmp (sp_repr_name (root), "inkscape")) {
		w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_notsp, fn);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		sp_repr_document_unref (doc);
		g_free (fn);
		return;
	}

	sp_repr_document_merge (config, doc, "id");
	sp_repr_document_unref (doc);
	g_free (fn);
}

/* Preferences management */

void
inkscape_load_preferences (Inkscape::Application *inkscape)
{
	inkscape_load_config (PREFERENCES_FILE, inkscape->preferences, preferences_skeleton, PREFERENCES_SKELETON_SIZE,
			      _("%s is not regular file.\n"
				"Although inkscape will run, you can\n"
				"neither load nor save preferences\n"),
			      _("%s either is not valid xml file or\n"
				"you do not have read premissions on it.\n"
				"Although inkscape will run, you\n"
				"are neither able to load nor save\n"
				"preferences."),
			      _("%s is not valid inkscape preferences file.\n"
				"Although inkscape will run, you\n"
				"are neither able to load nor save\n"
				"preferences."));
}

void
inkscape_save_preferences (Inkscape::Application * inkscape)
{
	gchar * fn;

	fn = profile_path(PREFERENCES_FILE);
	sp_repr_save_file (inkscape->preferences, fn);

	g_free (fn);
}

/* We use '.' as separator */
SPRepr *
inkscape_get_repr (Inkscape::Application *inkscape, const gchar *key)
{
	SPRepr * repr;
	gchar const *s;

	if (key == NULL) return NULL;

	repr = sp_repr_document_root (inkscape->preferences);
	g_assert (!(strcmp (sp_repr_name (repr), "inkscape")));

	s = key;
	while ((s) && (*s)) {
		gchar const *e;
		SPRepr * child;
		guint len;

		/* Find next name */
		if ((e = strchr (s, '.'))) {
			len = e++ - s;
		} else {
			len = strlen (s);
		}
		for (child = repr->children; child != NULL; child = child->next) {
			gchar const *id = sp_repr_attr (child, "id");
			if ((id) && (strlen (id) == len) && (!strncmp (id, s, len)))
			{
				break;
			}
		}
		if (child == NULL) return NULL;

		repr = child;
		s = e;
	}
	return repr;
}

void
inkscape_selection_modified (SPSelection *selection, guint flags)
{
	g_return_if_fail (inkscape != NULL);
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	if (DESKTOP_IS_ACTIVE (selection->desktop)) {
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[MODIFY_SELECTION], 0, selection, flags);
	}
}

void
inkscape_selection_changed (SPSelection * selection)
{
	g_return_if_fail (inkscape != NULL);
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	if (DESKTOP_IS_ACTIVE (selection->desktop)) {
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[CHANGE_SELECTION], 0, selection);
	}
}

void
inkscape_selection_set (SPSelection * selection)
{
	g_return_if_fail (inkscape != NULL);
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	if (DESKTOP_IS_ACTIVE (selection->desktop)) {
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[SET_SELECTION], 0, selection);
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[CHANGE_SELECTION], 0, selection);
	}
}

void
inkscape_eventcontext_set (SPEventContext * eventcontext)
{
	g_return_if_fail (inkscape != NULL);
	g_return_if_fail (eventcontext != NULL);
	g_return_if_fail (SP_IS_EVENT_CONTEXT (eventcontext));

	if (DESKTOP_IS_ACTIVE (eventcontext->desktop)) {
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[SET_EVENTCONTEXT], 0, eventcontext);
	}
}

void
inkscape_add_desktop (SPDesktop * desktop)
{
	g_return_if_fail (inkscape != NULL);
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	g_assert (!g_slist_find (inkscape->desktops, desktop));

	inkscape->desktops = g_slist_append (inkscape->desktops, desktop);

	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[NEW_DESKTOP], 0, desktop);

	if (DESKTOP_IS_ACTIVE (desktop)) {
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[ACTIVATE_DESKTOP], 0, desktop);
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[SET_EVENTCONTEXT], 0, SP_DT_EVENTCONTEXT (desktop));
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[SET_SELECTION], 0, SP_DT_SELECTION (desktop));
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[CHANGE_SELECTION], 0, SP_DT_SELECTION (desktop));
	}
}

void
inkscape_remove_desktop (SPDesktop * desktop)
{
	g_return_if_fail (inkscape != NULL);
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	g_assert (g_slist_find (inkscape->desktops, desktop));

	if (DESKTOP_IS_ACTIVE (desktop)) {
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[DEACTIVATE_DESKTOP], 0, desktop);
		if (inkscape->desktops->next != NULL) {
			SPDesktop * new_desktop;
			new_desktop = (SPDesktop *) inkscape->desktops->next->data;
			inkscape->desktops = g_slist_remove (inkscape->desktops, new_desktop);
			inkscape->desktops = g_slist_prepend (inkscape->desktops, new_desktop);
			g_signal_emit (G_OBJECT (inkscape), inkscape_signals[ACTIVATE_DESKTOP], 0, new_desktop);
			g_signal_emit (G_OBJECT (inkscape), inkscape_signals[SET_EVENTCONTEXT], 0, SP_DT_EVENTCONTEXT (new_desktop));
			g_signal_emit (G_OBJECT (inkscape), inkscape_signals[SET_SELECTION], 0, SP_DT_SELECTION (new_desktop));
			g_signal_emit (G_OBJECT (inkscape), inkscape_signals[CHANGE_SELECTION], 0, SP_DT_SELECTION (new_desktop));
		} else {
			g_signal_emit (G_OBJECT (inkscape), inkscape_signals[SET_EVENTCONTEXT], 0, NULL);
			g_signal_emit (G_OBJECT (inkscape), inkscape_signals[SET_SELECTION], 0, NULL);
			g_signal_emit (G_OBJECT (inkscape), inkscape_signals[CHANGE_SELECTION], 0, NULL);
		}
	}

	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[DESTROY_DESKTOP], 0, desktop);

	inkscape->desktops = g_slist_remove (inkscape->desktops, desktop);

	// if this was the last desktop, shut down the program
	if (inkscape->desktops == NULL) {
		inkscape_exit (inkscape);
	}
}

void
inkscape_activate_desktop (SPDesktop * desktop)
{
	SPDesktop * current;

	g_return_if_fail (inkscape != NULL);
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	if (DESKTOP_IS_ACTIVE (desktop)) return;

	g_assert (g_slist_find (inkscape->desktops, desktop));

	current = (SPDesktop *) inkscape->desktops->data;

	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[DEACTIVATE_DESKTOP], 0, current);

	inkscape->desktops = g_slist_remove (inkscape->desktops, desktop);
	inkscape->desktops = g_slist_prepend (inkscape->desktops, desktop);

	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[ACTIVATE_DESKTOP], 0, desktop);
	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[SET_EVENTCONTEXT], 0, SP_DT_EVENTCONTEXT (desktop));
	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[SET_SELECTION], 0, SP_DT_SELECTION (desktop));
	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[CHANGE_SELECTION], 0, SP_DT_SELECTION (desktop));
}

/**
\brief  Resends ACTIVATE_DESKTOP for current desktop; needed when a new desktop has got its window that dialogs will transientize to
*/
void
inkscape_reactivate_desktop (SPDesktop * desktop)
{
	g_return_if_fail (inkscape != NULL);
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	if (DESKTOP_IS_ACTIVE (desktop))
		g_signal_emit (G_OBJECT (inkscape), inkscape_signals[ACTIVATE_DESKTOP], 0, desktop);
}

SPDesktop *
inkscape_find_desktop_by_dkey (unsigned int dkey)
{
	GSList *r;

	for (r = inkscape->desktops; r; r = r->next) {
		if (((SPDesktop *) r->data)->dkey == dkey)
			return ((SPDesktop *) r->data);
	}
	return NULL;
}

unsigned int
inkscape_maximum_dkey()
{
	GSList *r;
	unsigned int dkey = 0;

	for (r = inkscape->desktops; r; r = r->next) {
		if (((SPDesktop *) r->data)->dkey > dkey)
			dkey = ((SPDesktop *) r->data)->dkey;
	}

	g_assert (dkey > 0);

	return dkey;
}

SPDesktop *
inkscape_next_desktop ()
{
	SPDesktop *d = NULL;
	unsigned int dkey_current = ((SPDesktop *) inkscape->desktops->data)->dkey;
	unsigned int i;

	if (dkey_current < inkscape_maximum_dkey()) {
		// find next existing
		for (i = dkey_current + 1; i <= inkscape_maximum_dkey(); i++) {
			d = inkscape_find_desktop_by_dkey (i);
			if (d) break;
		}
	} else {
		// find first existing
		for (i = 1; i <= inkscape_maximum_dkey(); i++) {
			d = inkscape_find_desktop_by_dkey (i);
			if (d) break;
		}
	}

	g_assert (d);

	return d;
}

SPDesktop *
inkscape_prev_desktop ()
{
	SPDesktop *d = NULL;
	unsigned int dkey_current = ((SPDesktop *) inkscape->desktops->data)->dkey;
	unsigned int i;

	if (dkey_current > 1) {
		// find prev existing
		for (i = dkey_current - 1; i >= 1; i--) {
			d = inkscape_find_desktop_by_dkey (i);
			if (d) break;
		}
	}
	if (!d) {
		// find last existing
		d = inkscape_find_desktop_by_dkey (inkscape_maximum_dkey());
	}

	g_assert (d);

	return d;
}

void
inkscape_switch_desktops_next ()
{
	GtkWindow *w;

	w = (GtkWindow *) g_object_get_data (G_OBJECT (inkscape_next_desktop ()), "window");
	gtk_window_present (w);
}

void
inkscape_switch_desktops_prev ()
{
	GtkWindow *w;

	w = (GtkWindow *) g_object_get_data (G_OBJECT (inkscape_prev_desktop ()), "window");
	gtk_window_present (w);
}

void
inkscape_dialogs_hide ()
{
	g_return_if_fail (inkscape != NULL);

	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[DIALOGS_HIDE], 0);
	inkscape->dialogs_toggle = FALSE;
}

void
inkscape_dialogs_unhide ()
{
	g_return_if_fail (inkscape != NULL);

	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[DIALOGS_UNHIDE], 0);
	inkscape->dialogs_toggle = TRUE;
}

void
inkscape_dialogs_toggle ()
{
	if (inkscape->dialogs_toggle)
		inkscape_dialogs_hide ();
	else
		inkscape_dialogs_unhide ();
}

/* fixme: These need probably signals too */
void
inkscape_add_document (SPDocument *document)
{
	g_return_if_fail (inkscape != NULL);
	g_return_if_fail (document != NULL);
	g_return_if_fail (SP_IS_DOCUMENT (document));

	g_assert (!g_slist_find (inkscape->documents, document));

	inkscape->documents = g_slist_append (inkscape->documents, document);

	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[NEW_DOCUMENT], 0, document);
}

//TODO: make settable in preferences
#define MAX_RECENT_DOCUMENTS 15

void
inkscape_remove_document (SPDocument *document)
{
	g_return_if_fail (inkscape != NULL);
	g_return_if_fail (document != NULL);
	g_return_if_fail (SP_IS_DOCUMENT (document));
	gint i;

	g_assert (g_slist_find (inkscape->documents, document));

	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[DESTROY_DOCUMENT], 0, document);

	inkscape->documents = g_slist_remove (inkscape->documents, document);

	if (document->advertize && SP_DOCUMENT_URI (document)) {
		SPRepr *recent;
		recent = inkscape_get_repr (INKSCAPE, "documents.recent");
		if (recent) {
			SPRepr *child;
			child = sp_repr_lookup_child (recent, "uri", SP_DOCUMENT_URI (document));
			if (child) {
				sp_repr_change_order (recent, child, NULL);
			} else {
				if (sp_repr_n_children (recent) >= MAX_RECENT_DOCUMENTS) {
					child = recent->children;
					// count to the last
					for (i = 0; i < MAX_RECENT_DOCUMENTS - 2; i ++) child = child->next;
					// remove all after the last
					while (child->next) sp_repr_unparent (child->next);
				}
				child = sp_repr_new ("document");
				sp_repr_set_attr (child, "uri", SP_DOCUMENT_URI (document));
				sp_repr_add_child (recent, child, NULL);
			}
			sp_repr_set_attr (child, "name", SP_DOCUMENT_NAME (document));
		}
	}
}

void
inkscape_set_color (SPColor *color, float opacity)
{
	g_signal_emit (G_OBJECT (inkscape), inkscape_signals[COLOR_SET], 0, color, (double) opacity);
}

SPDesktop *
inkscape_active_desktop (void)
{
	if (inkscape->desktops == NULL) return NULL;

	return (SPDesktop *) inkscape->desktops->data;
}

SPDocument *
inkscape_active_document (void)
{
	if (SP_ACTIVE_DESKTOP) return SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);

	return NULL;
}

SPEventContext *
inkscape_active_event_context (void)
{
	if (SP_ACTIVE_DESKTOP) return SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP);

	return NULL;
}

/* Helpers */

static void
inkscape_init_config (SPReprDoc *doc, const gchar *config_name, const gchar *skeleton, unsigned int skel_size,
		      const gchar *e_mkdir, const gchar *e_notdir, const gchar *e_ccf, const gchar *e_cwf)
{
	gchar * dn, *fn;
	FILE *fh;
	GtkWidget * w;

	dn = profile_path(NULL);
	if (!g_file_test(dn, G_FILE_TEST_EXISTS)) {
		if (mkdir (dn, S_IRWXU | S_IRGRP | S_IXGRP))
		{
			/* Cannot create directory */
			w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_mkdir, dn);
			gtk_dialog_run (GTK_DIALOG (w));
			gtk_widget_destroy (w);
			g_free (dn);
			return;
		}
	} else if (!g_file_test(dn, G_FILE_TEST_IS_DIR)) {
		/* Not a directory */
		w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_notdir, dn);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		g_free (dn);
		return;
	}
	g_free (dn);

	fn = profile_path(config_name);

	fh = fopen(fn, "w");
	if (!fh) {
		/* Cannot create file */
		w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_ccf, fn);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		g_free (fn);
		return;
	}
	if ( fwrite(skeleton, 1, skel_size, fh) != skel_size ) {
		/* Cannot create file */
		w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_cwf, fn);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		g_free (fn);
		fclose(fh);
		return;
	}

	g_free (fn);
	fclose(fh);
}

/* This routine should be obsoleted in favor of the generic version */

static void
inkscape_init_preferences (Inkscape::Application *inkscape)
{
	inkscape_init_config (inkscape->preferences, PREFERENCES_FILE, preferences_skeleton, PREFERENCES_SKELETON_SIZE,
			      _("Cannot create directory %s.\n"
				"Although inkscape will run, you\n"
				"are neither able to load nor save\n"
				"%s."),
			      _("%s is not a valid directory.\n"
				"Although inkscape will run, you\n"
				"are neither able to load nor save\n"
				"preferences."),
			      _("Cannot create file %s.\n"
				"Although inkscape will run, you\n"
				"are neither able to load nor save\n"
				"preferences."),
			      _("Cannot write file %s.\n"
				"Although inkscape will run, you\n"
				"are neither able to load nor save\n"
				"preferences."));
}

void
inkscape_refresh_display (Inkscape::Application *inkscape)
{
	GSList *l;

	for (l = inkscape->desktops; l != NULL; l = l->next) {
		sp_view_request_redraw (SP_VIEW (l->data));
	}
}

void
inkscape_exit (Inkscape::Application *inkscape)
{
	//emit shutdown signal so that dialogs could remember layout
	g_signal_emit (G_OBJECT (INKSCAPE), inkscape_signals[SHUTDOWN_SIGNAL], 0);

	if (inkscape->preferences) {
		inkscape_save_preferences (INKSCAPE);
	}
	gtk_main_quit ();
}

gchar *
profile_path(const char *filename)
{
	static const gchar *homedir=NULL;
	if (!homedir) {
		homedir = g_get_home_dir();
#ifdef HAS_SHGetSpecialFolderPath
		if (!homedir) { //only try this is previous attempt fails
			char pathBuf[MAX_PATH+1];
			if (SHGetSpecialFolderPath(NULL, pathBuf, CSIDL_APPDATA, 1))
				homedir = g_strdup(pathBuf);
		}
#endif
		if (!homedir) {
			homedir = g_path_get_dirname(INKSCAPE->argv0);
		}
	}
	return g_build_filename(homedir, INKSCAPE_PROFILE_DIR, filename, NULL);
}

gchar *
executable_path(void)
{
  gchar *path  = "";
  int   strLen = 0;
#ifdef HAS_PROC_SELF_EXE
  char  pathBuf[PATH_MAX+1];
  strLen = readlink("/proc/self/exe", pathBuf, PATH_MAX);
#else
#ifdef HAS_GetModuleFileName
  char  pathBuf[MAX_PATH+1];
  strLen = (int)GetModuleFileName(NULL, pathBuf, MAX_PATH);
#endif
#endif
  if (strLen <= 0)
    {
    g_print("Unable to retrieve the path of the executable.\n");
    return NULL;
    }
  path = g_strdup(pathBuf);
  return path;
}


