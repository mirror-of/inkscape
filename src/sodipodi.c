#define __SODIPODI_C__

/*
 * Interface to main application
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#else
#include "monostd.h"
#endif
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

#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "event-context.h"
#include "sodipodi.h"
#include "sodipodi-private.h"

/* Backbones of configuration xml data */
#include "preferences-skeleton.h"
#include "extensions-skeleton.h"

enum {
	MODIFY_SELECTION,
	CHANGE_SELECTION,
	SET_SELECTION,
	SET_EVENTCONTEXT,
	NEW_DESKTOP,
	DESTROY_DESKTOP,
	ACTIVATE_DESKTOP,
	DESACTIVATE_DESKTOP,
	NEW_DOCUMENT,
	DESTROY_DOCUMENT,
	COLOR_SET,
	LAST_SIGNAL
};

#define DESKTOP_IS_ACTIVE(d) ((d) == sodipodi->desktops->data)

static void sodipodi_class_init (SodipodiClass *klass);
static void sodipodi_init (SPObject *object);
static void sodipodi_dispose (GObject *object);

static void sodipodi_activate_desktop_private (Sodipodi *sodipodi, SPDesktop *desktop);
static void sodipodi_desactivate_desktop_private (Sodipodi *sodipodi, SPDesktop *desktop);

static void sodipodi_init_config (SPReprDoc *doc, const gchar *config_name, const gchar *skeleton, int skel_size,
				  const unsigned char *e_mkdir,
				  const unsigned char *e_notdir,
				  const unsigned char *e_ccf,
				  const unsigned char *e_cwf);
static void sodipodi_init_preferences (Sodipodi *sodipodi);
static void sodipodi_init_extensions (Sodipodi *sodipodi);

static void sodipodi_init_preferences (Sodipodi * sodipodi);

struct _Sodipodi {
	GObject object;
	SPReprDoc *preferences;
	SPReprDoc *extensions;
	GSList *documents;
	GSList *desktops;
};

struct _SodipodiClass {
	GObjectClass object_class;

	/* Signals */
	void (* change_selection) (Sodipodi * sodipodi, SPSelection * selection);
	void (* modify_selection) (Sodipodi * sodipodi, SPSelection * selection, guint flags);
	void (* set_selection) (Sodipodi * sodipodi, SPSelection * selection);
	void (* set_eventcontext) (Sodipodi * sodipodi, SPEventContext * eventcontext);
	void (* new_desktop) (Sodipodi * sodipodi, SPDesktop * desktop);
	void (* destroy_desktop) (Sodipodi * sodipodi, SPDesktop * desktop);
	void (* activate_desktop) (Sodipodi * sodipodi, SPDesktop * desktop);
	void (* desactivate_desktop) (Sodipodi * sodipodi, SPDesktop * desktop);
	void (* new_document) (Sodipodi *sodipodi, SPDocument *doc);
	void (* destroy_document) (Sodipodi *sodipodi, SPDocument *doc);

	void (* color_set) (Sodipodi *sodipodi, SPColor *color, double opacity);
};

static GObjectClass * parent_class;
static guint sodipodi_signals[LAST_SIGNAL] = {0};

Sodipodi *sodipodi = NULL;

static void (* segv_handler) (int) = NULL;

GType
sodipodi_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SodipodiClass),
			NULL, NULL,
			(GClassInitFunc) sodipodi_class_init,
			NULL, NULL,
			sizeof (Sodipodi),
			4,
			(GInstanceInitFunc) sodipodi_init,
		};
		type = g_type_register_static (G_TYPE_OBJECT, "Sodipodi", &info, 0);
	}
	return type;
}

static void
sodipodi_class_init (SodipodiClass * klass)
{
	GObjectClass * object_class;

	object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	sodipodi_signals[MODIFY_SELECTION] = g_signal_new ("modify_selection",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, modify_selection),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER_UINT,
							   G_TYPE_NONE, 2,
							   G_TYPE_POINTER, G_TYPE_UINT);
	sodipodi_signals[CHANGE_SELECTION] = g_signal_new ("change_selection",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, change_selection),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	sodipodi_signals[SET_SELECTION] =    g_signal_new ("set_selection",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, set_selection),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	sodipodi_signals[SET_EVENTCONTEXT] = g_signal_new ("set_eventcontext",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, set_eventcontext),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	sodipodi_signals[NEW_DESKTOP] =      g_signal_new ("new_desktop",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, new_desktop),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	sodipodi_signals[DESTROY_DESKTOP] =  g_signal_new ("destroy_desktop",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, destroy_desktop),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	sodipodi_signals[ACTIVATE_DESKTOP] = g_signal_new ("activate_desktop",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, activate_desktop),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	sodipodi_signals[DESACTIVATE_DESKTOP] = g_signal_new ("desactivate_desktop",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, desactivate_desktop),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	sodipodi_signals[NEW_DOCUMENT] =     g_signal_new ("new_document",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, new_document),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	sodipodi_signals[DESTROY_DOCUMENT] = g_signal_new ("destroy_document",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, destroy_document),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER,
							   G_TYPE_NONE, 1,
							   G_TYPE_POINTER);
	sodipodi_signals[COLOR_SET] =        g_signal_new ("color_set",
							   G_TYPE_FROM_CLASS (klass),
							   G_SIGNAL_RUN_FIRST,
							   G_STRUCT_OFFSET (SodipodiClass, color_set),
							   NULL, NULL,
							   sp_marshal_NONE__POINTER_DOUBLE,
							   G_TYPE_NONE, 2,
							   G_TYPE_POINTER, G_TYPE_DOUBLE);

	object_class->dispose = sodipodi_dispose;

	klass->activate_desktop = sodipodi_activate_desktop_private;
	klass->desactivate_desktop = sodipodi_desactivate_desktop_private;
}

static void
sodipodi_init (SPObject * object)
{
	if (!sodipodi) {
		sodipodi = (Sodipodi *) object;
	} else {
		g_assert_not_reached ();
	}

	sodipodi->preferences = sp_repr_read_mem (preferences_skeleton, PREFERENCES_SKELETON_SIZE, NULL);

	sodipodi->extensions = sp_repr_read_mem (extensions_skeleton, EXTENSIONS_SKELETON_SIZE, NULL);

	/* Initialize shortcuts */
	sp_shortcut_table_load (NULL);

	sodipodi->documents = NULL;
	sodipodi->desktops = NULL;
}

static void
sodipodi_dispose (GObject *object)
{
	Sodipodi *sodipodi;

	sodipodi = (Sodipodi *) object;

	while (sodipodi->documents) {
		g_object_unref (G_OBJECT (sodipodi->documents->data));
	}

	g_assert (!sodipodi->desktops);

	if (sodipodi->extensions) {
		sp_repr_document_unref (sodipodi->extensions);
		sodipodi->extensions = NULL;
	}

	if (sodipodi->preferences) {
		/* fixme: This is not the best place */
		sodipodi_save_preferences (sodipodi);
		sp_repr_document_unref (sodipodi->preferences);
		sodipodi->preferences = NULL;
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);

	gtk_main_quit ();
}

void
sodipodi_ref (void)
{
	g_object_ref (G_OBJECT (sodipodi));
}

void
sodipodi_unref (void)
{
	g_object_unref (G_OBJECT (sodipodi));
}

static void
sodipodi_activate_desktop_private (Sodipodi *sodipodi, SPDesktop *desktop)
{
	sp_desktop_set_active (desktop, TRUE);
}

static void
sodipodi_desactivate_desktop_private (Sodipodi *sodipodi, SPDesktop *desktop)
{
	sp_desktop_set_active (desktop, FALSE);
}

/* fixme: This is EVIL, and belongs to main after all */

#define SP_INDENT 8

static void
sodipodi_segv_handler (int signum)
{
	static gint recursion = FALSE;
	GSList *savednames, *failednames, *l;
	const gchar *home;
	gchar *istr, *sstr, *fstr, *b;
	gint count, nllen, len, pos;
	time_t sptime;
	struct tm *sptm;
	char sptstr[256];
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
	for (l = sodipodi->documents; l != NULL; l = l->next) {
		SPDocument *doc;
		SPRepr *repr;
		doc = (SPDocument *) l->data;
		repr = sp_document_repr_root (doc);
		if (sp_repr_attr (repr, "sodipodi:modified")) {
			const guchar *docname, *d0, *d;
			gchar n[64], c[1024];
			FILE *file;
#if 0
			docname = sp_repr_attr (repr, "sodipodi:docname");
			if (docname) {
				docname = g_basename (docname);
			}
#else
			docname = doc->name;
			if (docname) {
				/* fixme: Quick hack to remove emergency file suffix */
				d0 = strrchr (docname, '.');
				if (d0 && (d0 > docname)) {
					d0 = strrchr (d0 - 1, '.');
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
#endif
			if (!docname || !*docname) docname = "emergency";
			g_snprintf (c, 1024, "%s/.sodipodi/%.256s.%s.%d", home, docname, sptstr, count);
			file = fopen (c, "w");
			if (!file) {
				g_snprintf (c, 1024, "%s/sodipodi-%.256s.%s.%d", home, docname, sptstr, count);
				file = fopen (c, "w");
			}
			if (!file) {
				g_snprintf (c, 1024, "/tmp/sodipodi-%.256s.%s.%d", docname, sptstr, count);
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

	g_warning ("Emergency save completed, now crashing...");

	/* Show nice dialog box */

	istr = N_("Sodipodi encountered an internal error and will close now.\n");
	sstr = N_("Automatic backups of unsaved documents were done to following locations:\n");
	fstr = N_("Automatic backup of following documents failed:\n");
	nllen = strlen ("\n");
	len = strlen (istr) + strlen (sstr) + strlen (fstr);
	for (l = savednames; l != NULL; l = l->next) {
		len = len + SP_INDENT + strlen ((gchar *) l->data) + nllen;
	}
	for (l = failednames; l != NULL; l = l->next) {
		len = len + SP_INDENT + strlen ((gchar *) l->data) + nllen;
	}
	len += 1;
	b = g_new (gchar, len);
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

Sodipodi *
sodipodi_application_new (void)
{
	Sodipodi *sp;

	sp = g_object_new (SP_TYPE_SODIPODI, NULL);
	/* fixme: load application defaults */

#ifndef WIN32
	segv_handler = signal (SIGSEGV, sodipodi_segv_handler);
	signal (SIGFPE, sodipodi_segv_handler);
	signal (SIGILL, sodipodi_segv_handler);
#endif

	return sp;
}

/* Preference management */
/* We use '.' as separator */

static void
sodipodi_load_config (const unsigned char *filename, SPReprDoc *config, const unsigned char *skeleton, unsigned int skel_size,
		      const unsigned char *e_notreg, const unsigned char *e_notxml, const unsigned char *e_notsp)
{
	gchar *fn;
	struct stat s;
	GtkWidget * w;
	SPReprDoc * doc;
	SPRepr * root;

#ifdef WIN32
	fn = g_strdup_printf ("sodipodi/%s", filename);
#else
	fn = g_build_filename (g_get_home_dir (), ".sodipodi", filename, NULL);
#endif
	if (stat (fn, &s)) {
		/* No such file */
		/* fixme: Think out something (Lauris) */
		if (!strcmp (filename, "extensions")) {
			sodipodi_init_extensions (SODIPODI);
		} else {
			sodipodi_init_preferences (SODIPODI);
		}
		g_free (fn);
		return;
	}

	if (!S_ISREG (s.st_mode)) {
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
	if (strcmp (sp_repr_name (root), "sodipodi")) {
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
sodipodi_load_preferences (Sodipodi *sodipodi)
{
	sodipodi_load_config ("preferences", sodipodi->preferences, preferences_skeleton, PREFERENCES_SKELETON_SIZE,
			      _("%s is not regular file.\n"
				"Although sodipodi will run, you can\n"
				"neither load nor save preferences\n"),
			      _("%s either is not valid xml file or\n"
				"you do not have read premissions on it.\n"
				"Although sodipodi will run, you\n"
				"are neither able to load nor save\n"
				"preferences."),
			      _("%s is not valid sodipodi preferences file.\n"
				"Although sodipodi will run, you\n"
				"are neither able to load nor save\n"
				"preferences."));
}

/* Extensions management */

void
sodipodi_load_extensions (Sodipodi *sodipodi)
{
	sodipodi_load_config ("extensions", sodipodi->extensions, extensions_skeleton, EXTENSIONS_SKELETON_SIZE,
			      _("%s is not regular file.\n"
				"Although sodipodi will run, you are\n"
				"not able to use extensions (plugins)\n"),
			      _("%s either is not valid xml file or\n"
				"you do not have read premissions on it.\n"
				"Although sodipodi will run, you are\n"
				"not able to use extensions (plugins)\n"),
			      _("%s is not valid sodipodi extensions file.\n"
				"Although sodipodi will run, you are\n"
				"not able to use extensions (plugins)\n"));
}

void
sodipodi_save_preferences (Sodipodi * sodipodi)
{
	gchar * fn;

#ifdef WIN32
	fn = g_strdup ("sodipodi/preferences");
#else
	fn = g_build_filename (g_get_home_dir (), ".sodipodi/preferences", NULL);
#endif

	sp_repr_save_file (sodipodi->preferences, fn);

	g_free (fn);
}

/* We use '.' as separator */
SPRepr *
sodipodi_get_repr (Sodipodi *sodipodi, const unsigned char *key)
{
	SPRepr * repr;
	const gchar * id, * s, * e;
	gint len;

	if (key == NULL) return NULL;

	if (!strncmp (key, "extensions", 10) && (!key[10] || (key[10] == '.'))) {
		repr = sp_repr_document_root (sodipodi->extensions);
	} else {
		repr = sp_repr_document_root (sodipodi->preferences);
	}
	g_assert (!(strcmp (sp_repr_name (repr), "sodipodi")));

	s = key;
	while ((s) && (*s)) {
		SPRepr * child;
		/* Find next name */
		if ((e = strchr (s, '.'))) {
			len = e++ - s;
		} else {
			len = strlen (s);
		}
		for (child = repr->children; child != NULL; child = child->next) {
			id = sp_repr_attr (child, "id");
			if ((id) && (strlen (id) == len) && (!strncmp (id, s, len))) break;
		}
		if (child == NULL) return NULL;

		repr = child;
		s = e;
	}
	return repr;
}

void
sodipodi_selection_modified (SPSelection *selection, guint flags)
{
	g_return_if_fail (sodipodi != NULL);
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	if (DESKTOP_IS_ACTIVE (selection->desktop)) {
		g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[MODIFY_SELECTION], 0, selection, flags);
	}
}

void
sodipodi_selection_changed (SPSelection * selection)
{
	g_return_if_fail (sodipodi != NULL);
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	if (DESKTOP_IS_ACTIVE (selection->desktop)) {
		g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[CHANGE_SELECTION], 0, selection);
	}
}

void
sodipodi_selection_set (SPSelection * selection)
{
	g_return_if_fail (sodipodi != NULL);
	g_return_if_fail (selection != NULL);
	g_return_if_fail (SP_IS_SELECTION (selection));

	if (DESKTOP_IS_ACTIVE (selection->desktop)) {
		g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[SET_SELECTION], 0, selection);
		g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[CHANGE_SELECTION], 0, selection);
	}
}

void
sodipodi_eventcontext_set (SPEventContext * eventcontext)
{
	g_return_if_fail (sodipodi != NULL);
	g_return_if_fail (eventcontext != NULL);
	g_return_if_fail (SP_IS_EVENT_CONTEXT (eventcontext));

	if (DESKTOP_IS_ACTIVE (eventcontext->desktop)) {
		g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[SET_EVENTCONTEXT], 0, eventcontext);
	}
}

void
sodipodi_add_desktop (SPDesktop * desktop)
{
	g_return_if_fail (sodipodi != NULL);
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	g_assert (!g_slist_find (sodipodi->desktops, desktop));

	sodipodi->desktops = g_slist_append (sodipodi->desktops, desktop);

	g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[NEW_DESKTOP], 0, desktop);

	if (DESKTOP_IS_ACTIVE (desktop)) {
		g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[ACTIVATE_DESKTOP], 0, desktop);
		g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[SET_EVENTCONTEXT], 0, SP_DT_EVENTCONTEXT (desktop));
		g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[SET_SELECTION], 0, SP_DT_SELECTION (desktop));
		g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[CHANGE_SELECTION], 0, SP_DT_SELECTION (desktop));
	}
}

void
sodipodi_remove_desktop (SPDesktop * desktop)
{
	g_return_if_fail (sodipodi != NULL);
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	g_assert (g_slist_find (sodipodi->desktops, desktop));

	if (DESKTOP_IS_ACTIVE (desktop)) {
		g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[DESACTIVATE_DESKTOP], 0, desktop);
		if (sodipodi->desktops->next != NULL) {
			SPDesktop * new;
			new = (SPDesktop *) sodipodi->desktops->next->data;
			sodipodi->desktops = g_slist_remove (sodipodi->desktops, new);
			sodipodi->desktops = g_slist_prepend (sodipodi->desktops, new);
			g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[ACTIVATE_DESKTOP], 0, new);
			g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[SET_EVENTCONTEXT], 0, SP_DT_EVENTCONTEXT (new));
			g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[SET_SELECTION], 0, SP_DT_SELECTION (new));
			g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[CHANGE_SELECTION], 0, SP_DT_SELECTION (new));
		} else {
			g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[SET_EVENTCONTEXT], 0, NULL);
			g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[SET_SELECTION], 0, NULL);
			g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[CHANGE_SELECTION], 0, NULL);
		}
	}

	g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[DESTROY_DESKTOP], 0, desktop);

	sodipodi->desktops = g_slist_remove (sodipodi->desktops, desktop);
}

void
sodipodi_activate_desktop (SPDesktop * desktop)
{
	SPDesktop * current;

	g_return_if_fail (sodipodi != NULL);
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	if (DESKTOP_IS_ACTIVE (desktop)) return;

	g_assert (g_slist_find (sodipodi->desktops, desktop));

	current = (SPDesktop *) sodipodi->desktops->data;

	g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[DESACTIVATE_DESKTOP], 0, current);

	sodipodi->desktops = g_slist_remove (sodipodi->desktops, desktop);
	sodipodi->desktops = g_slist_prepend (sodipodi->desktops, desktop);

	g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[ACTIVATE_DESKTOP], 0, desktop);
	g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[SET_EVENTCONTEXT], 0, SP_DT_EVENTCONTEXT (desktop));
	g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[SET_SELECTION], 0, SP_DT_SELECTION (desktop));
	g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[CHANGE_SELECTION], 0, SP_DT_SELECTION (desktop));
}

/* fixme: These need probably signals too */

void
sodipodi_add_document (SPDocument *document)
{
	g_return_if_fail (sodipodi != NULL);
	g_return_if_fail (document != NULL);
	g_return_if_fail (SP_IS_DOCUMENT (document));

	g_assert (!g_slist_find (sodipodi->documents, document));

	sodipodi->documents = g_slist_append (sodipodi->documents, document);

	g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[NEW_DOCUMENT], 0, document);
}

void
sodipodi_remove_document (SPDocument *document)
{
	g_return_if_fail (sodipodi != NULL);
	g_return_if_fail (document != NULL);
	g_return_if_fail (SP_IS_DOCUMENT (document));

	g_assert (g_slist_find (sodipodi->documents, document));

	g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[DESTROY_DOCUMENT], 0, document);

	sodipodi->documents = g_slist_remove (sodipodi->documents, document);

	if (document->advertize && SP_DOCUMENT_URI (document)) {
		SPRepr *recent;
		recent = sodipodi_get_repr (SODIPODI, "documents.recent");
		if (recent) {
			SPRepr *child;
			child = sp_repr_lookup_child (recent, "uri", SP_DOCUMENT_URI (document));
			if (child) {
				sp_repr_change_order (recent, child, NULL);
			} else {
				if (sp_repr_n_children (recent) >= 4) {
					child = recent->children->next->next;
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
sodipodi_set_color (SPColor *color, float opacity)
{
	g_signal_emit (G_OBJECT (sodipodi), sodipodi_signals[COLOR_SET], 0, color, (double) opacity);
}

SPDesktop *
sodipodi_active_desktop (void)
{
	if (sodipodi->desktops == NULL) return NULL;

	return (SPDesktop *) sodipodi->desktops->data;
}

SPDocument *
sodipodi_active_document (void)
{
	if (SP_ACTIVE_DESKTOP) return SP_DT_DOCUMENT (SP_ACTIVE_DESKTOP);

	return NULL;
}

SPEventContext *
sodipodi_active_event_context (void)
{
	if (SP_ACTIVE_DESKTOP) return SP_DT_EVENTCONTEXT (SP_ACTIVE_DESKTOP);

	return NULL;
}

/* Helpers */

static void
sodipodi_init_config (SPReprDoc *doc, const gchar *config_name, const gchar *skeleton, int skel_size,
		      const unsigned char *e_mkdir, const unsigned char *e_notdir, const unsigned char *e_ccf, const unsigned char *e_cwf)
{
	gchar * dn, *fn;
	struct stat s;
	int fh;
	GtkWidget * w;

#ifdef WIN32
	dn = g_strdup ("sodipodi");
#else
	dn = g_build_filename (g_get_home_dir (), ".sodipodi", NULL);
#endif
	if (stat (dn, &s)) {
		if (mkdir (dn, S_IRWXU | S_IRGRP | S_IXGRP))
		{
			/* Cannot create directory */
			w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_mkdir, dn);
			gtk_dialog_run (GTK_DIALOG (w));
			gtk_widget_destroy (w);
			g_free (dn);
			return;
		}
	} else if (!S_ISDIR (s.st_mode)) {
		/* Not a directory */
		w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_notdir, dn);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		g_free (dn);
		return;
	}
	g_free (dn);

#ifdef WIN32
	fn = g_strdup_printf ("sodipodi/%s", config_name);
	fh = creat (fn, S_IREAD | S_IWRITE);
#else
	fn = g_build_filename (g_get_home_dir (), ".sodipodi", config_name, NULL);
	fh = creat (fn, S_IRUSR | S_IWUSR | S_IRGRP);
#endif
	if (fh < 0) {
		/* Cannot create file */
		w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_ccf, fn);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		g_free (fn);
		return;
	}
	if (write (fh, skeleton, skel_size) != skel_size) {
		/* Cannot create file */
		w = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, e_cwf, fn);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		g_free (fn);
		close (fh);
		return;
	}

	g_free (fn);
	close (fh);
}

/* This routine should be obsoleted in favor of the generic version */

static void
sodipodi_init_preferences (Sodipodi *sodipodi)
{
	sodipodi_init_config (sodipodi->preferences, "preferences", preferences_skeleton, PREFERENCES_SKELETON_SIZE,
			      _("Cannot create directory %s.\n"
				"Although sodipodi will run, you\n"
				"are neither able to load nor save\n"
				"%s."),
			      _("%s is not a valid directory.\n"
				"Although sodipodi will run, you\n"
				"are neither able to load nor save\n"
				"preferences."),
			      _("Cannot create file %s.\n"
				"Although sodipodi will run, you\n"
				"are neither able to load nor save\n"
				"preferences."),
			      _("Cannot write file %s.\n"
				"Although sodipodi will run, you\n"
				"are neither able to load nor save\n"
				"preferences."));
}

static void
sodipodi_init_extensions (Sodipodi *sodipodi)
{
	sodipodi_init_config (sodipodi->extensions, "extensions", extensions_skeleton, EXTENSIONS_SKELETON_SIZE,
			      _("Cannot create directory %s.\n"
				"Although sodipodi will run, you are\n"
				"not able to use extensions (plugins)\n"),
			      _("%s is not a valid directory.\n"
				"Although sodipodi will run, you are\n"
				"not able to use extensions (plugins)\n"),
			      _("Cannot create file %s.\n"
				"Although sodipodi will run, you are\n"
				"not able to use extensions (plugins)\n"),
			      _("Cannot write file %s.\n"
				"Although sodipodi will run, you are\n"
				"not able to use extensions (plugins)\n"));
}

void
sodipodi_refresh_display (Sodipodi *sodipodi)
{
	GSList *l;

	for (l = sodipodi->desktops; l != NULL; l = l->next) {
		sp_view_request_redraw (SP_VIEW (l->data));
	}
}

void
sodipodi_exit (Sodipodi *sodipodi)
{
	gtk_main_quit ();
}

