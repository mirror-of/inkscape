
#include <config.h>

#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "view.h"
#include "document.h"
#include "sp-object.h"
#include "dir-util.h"

#include "sp-module.h"
#include "sp-module-sys.h"
#include "sp-module-exec-ext.h"

/* Globals */
static GList * module_list = NULL;

/* Prototypes */
#if 0
static void       sp_modulesys_test_init          (void);
#endif
static void       sp_modulesys_init_svg_in        (void);
static void       sp_modulesys_init_svg_out       (void);
static void       sp_modulesys_init_svg_nons_out  (void);
static SPModule * sp_modulesys_autodetect         (GtkType       in_type,
                                                   SPModuleDoc * in_doc);
static gchar *    find_extention                  (gchar *       in_str);
static void       sp_modulesys_ext_init           (void);
static void       sp_modulesys_builtin_init       (void);
void              sp_modulesys_do_filter          (gpointer in_unused,
                                                   gpointer in_module);

/* Module system */
void sp_modulesys_init (void) {
	sp_modulesys_builtin_init();
	sp_modulesys_ext_init();
#if 0
	sp_modulesys_test_init();
#endif

	return;
}

static void sp_modulesys_builtin_init (void) {
	sp_modulesys_init_svg_in();
	sp_modulesys_init_svg_out();
	sp_modulesys_init_svg_nons_out();
	return;
}

/* TODO: This should be done by a configuration file
 *       but right now it's hard coded :(  */
static void sp_modulesys_ext_init (void) {
	SPModule * this_plug;
	SPModuleExecExt * ext;
	SPRepr *r;

	r = sp_repr_new ("input");
	sp_repr_set_attr (r, "id", SP_MODULE_KEY_INPUT_AI);
	sp_repr_set_attr (r, "name", "Adobe Illustrator");
	sp_repr_set_attr (r, "extension", "ai");
	this_plug = sp_module_input_new (r);
	sp_repr_unref (r);

	ext = sp_module_exec_ext_new();
	sp_module_set_exec(this_plug, SP_MODULE_EXEC(ext));
	sp_module_exec_ext_set_command(ext, SODIPODI_EXTENSIONDIR "/ill2svg.pl -l \"mac\"");
	sp_modulesys_list_add(this_plug);

	r = sp_repr_new ("filter");
	sp_repr_set_attr (r, "id", "modules.filters.Roundhole");
	sp_repr_set_attr (r, "name", "Roundhole");
	sp_repr_set_attr (r, "toolbox", "true");
	sp_repr_set_attr (r, "icon", SODIPODI_EXTENSIONDATADIR "/roundhole.xpm");
	this_plug = sp_module_filter_new(r);
	sp_repr_unref (r);

	ext = sp_module_exec_ext_new();
	sp_module_exec_ext_set_command(ext, SODIPODI_EXTENSIONDIR "/roundhole");
	sp_module_set_exec(this_plug, SP_MODULE_EXEC(ext));
	sp_modulesys_list_add(this_plug);

	return;
}

SPModule * sp_modulesys_list_add (SPModule * in_module) {
	g_return_val_if_fail(SP_IS_MODULE(in_module), NULL);
	module_list = g_list_append(module_list, (gpointer)in_module);
	return in_module;
}

#if 0
static void sp_modulesys_test_init (void) {
	SPModule * this_plug;

	this_plug = SP_MODULE(sp_module_input_new());
    sp_module_set_name(this_plug, "Ted's Input Module");
	sp_module_set_about(this_plug, FALSE);
	sp_module_set_exec(this_plug, SP_MODULE_EXEC(sp_module_exec_builtin_new()));
    sp_module_input_set_extention(SP_MODULE_INPUT(this_plug), "ted");
    sp_module_input_set_mimetype (SP_MODULE_INPUT(this_plug), "application/x-ted");
	sp_modulesys_list_add(this_plug);
#if 0
	sp_modulesys_do_save (this_plug, NULL, "Some test data");
#endif

	this_plug = SP_MODULE(sp_module_output_new());
    sp_module_set_name(this_plug, "Ted's Output Module");
	sp_modulesys_list_add(this_plug);

	this_plug = SP_MODULE(sp_module_input_new());
    sp_module_set_name(this_plug, "Ted's Input Module 2");
    sp_module_input_set_extention(SP_MODULE_INPUT(this_plug), "ted2");
    sp_module_input_set_mimetype (SP_MODULE_INPUT(this_plug), "application/x-ted-2");
	sp_modulesys_list_add(this_plug);

	this_plug = SP_MODULE(sp_module_output_new());
    sp_module_set_name(this_plug, "Ted's Output Module 2");
	sp_modulesys_list_add(this_plug);

	return;
}
#endif

static void module_about (gpointer widget, gpointer in_module) {
#if 0
	SPModule * module = (SPModule *)in_module;
#endif
	GtkWidget * about = NULL;
#if 0
	static gchar * author_null[] = {NULL};
#endif

#if 0
	about = gnome_about_new(module->name,
			                module->version,
							module->copyright,
							(const gchar **)author_null,
							module->description,
							(gchar *)NULL);
#endif

	if (about != NULL) {
		gtk_widget_show(about);
	}
	return;
}

static void module_place_about (gpointer in_module, gpointer in_menu) {
	GtkMenu * menu = (GtkMenu *)in_menu;
	SPModule * module = (SPModule *)in_module;
	GtkWidget * item;

	g_return_if_fail(SP_IS_MODULE(module));
	g_return_if_fail(GTK_IS_MENU(menu));

	if (module->about == TRUE) {
		item = gtk_menu_item_new_with_label(module->name);
		gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(module_about), in_module);
		gtk_widget_show(item);
		gtk_menu_append(menu, item);
	}

	return;
}

GtkMenu * sp_modulesys_menu_about (void) {
	GtkWidget * menu = NULL;

	menu = gtk_menu_new ();
	g_list_foreach(module_list, module_place_about, (gpointer)menu);
	gtk_widget_show (menu);

	return GTK_MENU(menu);
}

static SPModule * save_module = NULL;
static void module_set_save_module (gpointer widget, gpointer in_module) {
	save_module = SP_MODULE(in_module);
	return;
}

SPModule * sp_modulesys_get_save_module (void) {
	return save_module;
}

static void module_place_save (gpointer in_module, gpointer in_menu) {
	GtkMenu * menu = (GtkMenu *)in_menu;
	SPModule * module = (SPModule *)in_module;
	GtkWidget * item;

	if (SP_IS_MODULE_OUTPUT(in_module)) {
		item = gtk_menu_item_new_with_label(module->name);
		gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(module_set_save_module), in_module);
		gtk_widget_show(item);
		gtk_menu_append(menu, item);
	}

	return;
}

GtkMenu * sp_modulesys_menu_save (void) {
	GtkWidget * menu;
	GtkWidget * item;

	menu = gtk_menu_new ();

	item = gtk_menu_item_new_with_label("Autodetect");
	gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(module_set_save_module), NULL);
	gtk_widget_show(item);
	gtk_menu_append(GTK_MENU(menu), item);

	item = gtk_separator_menu_item_new ();
	gtk_widget_show(item);
	gtk_menu_append(GTK_MENU(menu), item);

	g_list_foreach(module_list, module_place_save, (gpointer)menu);

	gtk_widget_show (menu);
	return GTK_MENU(menu);
}

static SPModule * open_module = NULL;
static void module_set_open_module (gpointer widget, gpointer in_module) {
	open_module = SP_MODULE(in_module);
	return;
}

SPModule * sp_modulesys_get_open_module (void) {
	return open_module;
}

static void module_place_open (gpointer in_module, gpointer in_menu) {
	GtkMenu * menu = (GtkMenu *)in_menu;
	SPModule * module = (SPModule *)in_module;
	SPModuleInput * modulein = (SPModuleInput *)in_module;
	GtkWidget * item;
	gchar * tempstr;

	if (SP_IS_MODULE_INPUT(in_module)) {
		tempstr = g_strdup_printf("%s (*.%s)", module->name, modulein->extention);
		item = gtk_menu_item_new_with_label(tempstr);
		gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(module_set_open_module), in_module);
		gtk_menu_append(menu, item);
		gtk_widget_show(item);
		g_free(tempstr);
	}

	return;
}

GtkMenu * sp_modulesys_menu_open (void) {
	GtkWidget * menu;
	GtkWidget * item;

	menu = gtk_menu_new ();

	item = gtk_menu_item_new_with_label("Autodetect");
	gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(module_set_open_module), NULL);
	gtk_widget_show(item);
	gtk_menu_append(GTK_MENU(menu), item);

	item = gtk_separator_menu_item_new ();
	gtk_widget_show(item);
	gtk_menu_append(GTK_MENU(menu), item);

	g_list_foreach(module_list, module_place_open, (gpointer)menu);

	gtk_widget_show (menu);
	return GTK_MENU(menu);
}

static void module_place_filter (gpointer in_module, gpointer in_menu) {
	GtkMenu * menu = (GtkMenu *)in_menu;
	SPModule * module = (SPModule *)in_module;
	GtkWidget * item;

	if (SP_IS_MODULE_FILTER(in_module)) {
		item = gtk_menu_item_new_with_label(module->name);
		g_signal_connect(G_OBJECT(item), "activate", GTK_SIGNAL_FUNC(sp_modulesys_do_filter), in_module);
		gtk_menu_append(menu, item);
		gtk_widget_show(item);
	}

	return;
}

GtkMenu * sp_modulesys_menu_filter (void) {
	GtkWidget * menu;
	GtkWidget * item;

	menu = gtk_menu_new ();

	item = gtk_menu_item_new_with_label("Run Last Filter");
	gtk_widget_show(item);
	gtk_menu_append(GTK_MENU(menu), item);

	item = gtk_menu_item_new_with_label("Filter Editor...");
	gtk_widget_show(item);
	gtk_menu_append(GTK_MENU(menu), item);

	item = gtk_separator_menu_item_new ();
	gtk_widget_show(item);
	gtk_menu_append(GTK_MENU(menu), item);

	g_list_foreach(module_list, module_place_filter, (gpointer)menu);

	gtk_widget_show (menu);
	return GTK_MENU(menu);
}

SPDocument * sp_modulesys_do_open (SPModule * object, SPModuleDoc * doc) {
	SPModuleExecClass * myclass;

	g_return_val_if_fail(SP_IS_MODULE_DOC(doc), NULL);

	if (object == NULL) {
		object = sp_modulesys_autodetect(SP_TYPE_MODULE_INPUT, doc);
	}

	if (object == NULL) {
		GtkWidget * warning;
		/* basically means that we coudln't autodetect */
		warning = gtk_message_dialog_new(NULL,
				                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										 GTK_MESSAGE_ERROR,
										 GTK_BUTTONS_CLOSE,
										 "Unable to find a way to open %s, with the extention of %s.  Perhaps you should specify the file type.",
										 sp_module_doc_get_filename(doc),
										 find_extention(sp_module_doc_get_filename(doc))
										 );
		gtk_dialog_run(GTK_DIALOG(warning));
		gtk_widget_destroy(warning);
		return NULL;
	}

	g_return_val_if_fail(SP_IS_MODULE(object), NULL);
	g_return_val_if_fail(SP_IS_MODULE_EXEC(object->exec), NULL);

	myclass = (SPModuleExecClass *) G_OBJECT_GET_CLASS(object->exec);

	/* Try and load the plug-in if we need to */
	if (object->exec->state == SP_MODULE_EXEC_UNLOADED) {
		myclass->load(object);
	}
	/* Fail if we still don't have a loaded module */
	g_return_val_if_fail(object->exec->state == SP_MODULE_EXEC_LOADED, NULL);

	myclass->prefs(object, doc);

	return sp_module_doc_get_document(doc);
}

void sp_modulesys_do_save (SPModule * object, SPModuleDoc * doc) {
	SPModuleExecClass * myclass;

	g_return_if_fail(SP_IS_MODULE_DOC(doc));

	if (object == NULL) {
		/* Autodetect case */
		object = sp_modulesys_autodetect(SP_TYPE_MODULE_OUTPUT, doc);
	}

	g_return_if_fail(SP_IS_MODULE(object));
	g_return_if_fail(object->exec != NULL);

	myclass = (SPModuleExecClass *) G_OBJECT_GET_CLASS(object->exec);

	/* Try and load the plug-in if we need to */
	if (object->exec->state == SP_MODULE_EXEC_UNLOADED) {
		myclass->load(object);
	}
	/* Fail if we still don't have a loaded module */
	g_return_if_fail(object->exec->state == SP_MODULE_EXEC_LOADED);

	myclass->prefs(object, doc);

	return;
}

void sp_modulesys_do_filter (gpointer in_unused, gpointer in_module) {
	SPModule * object;
	SPModuleExecClass * myclass;
	SPModuleDoc * doc;

	g_return_if_fail(SP_IS_MODULE(in_module));

	object = SP_MODULE(in_module);
	g_return_if_fail(SP_IS_MODULE_EXEC(object->exec));

	myclass = (SPModuleExecClass *) G_OBJECT_GET_CLASS(object->exec);

	/* Try and load the plug-in if we need to */
	if (object->exec->state == SP_MODULE_EXEC_UNLOADED) {
		myclass->load(object);
	}

	/* Fail if we still don't have a loaded module */
	g_return_if_fail(object->exec->state == SP_MODULE_EXEC_LOADED);

	/* Creation of the document, it needs to get some data in it
	   eventually */
	doc = sp_module_doc_new();

	myclass->prefs(SP_MODULE(in_module), doc);
	return;
}

void sp_module_sys_prefs_complete (SPModule * object, SPModuleDoc * doc, gboolean success) {
	SPModuleExecClass * myclass;

	/* TODO: This function should probably do some clean up of the
	 *       different windows.  This will have to happen when there
	 *       are a little more definition of the SPModuleDoc.
	 */

	g_return_if_fail(SP_IS_MODULE(object));
	g_return_if_fail(SP_IS_MODULE_EXEC(object->exec));

	myclass = (SPModuleExecClass *) G_OBJECT_GET_CLASS(object->exec);

	if (success == TRUE) {
		myclass->exec(object, doc);
	}
}

typedef struct _modulesys_autodetect_t modulesys_autodetect_t;
struct _modulesys_autodetect_t {
	gchar * extention;
	GtkType type;
	SPModule * module;
};

static void module_autodetect (gpointer in_module, gpointer in_struct) {
	modulesys_autodetect_t * localstruct = (modulesys_autodetect_t *)in_struct;
	SPModule * module = (SPModule *)in_module;

	if (GTK_CHECK_TYPE(module, localstruct->type)) {
		gchar * plug_extention;
		/* This is an awful hack, but I can't think of a better
		 * way to do it right now.  I'd like to think that the
		 * compiler would realize the similarities between these
		 * two cases, but I don't think it's smart enough.  TODO... */
		if (SP_IS_MODULE_INPUT(in_module)) {
			SPModuleInput * lplug;
			lplug = SP_MODULE_INPUT(module);
			plug_extention = lplug->extention;
		} else {
			SPModuleOutput * lplug;
			lplug = SP_MODULE_OUTPUT(module);
			plug_extention = lplug->extention;
		}

		if (!strcmp(plug_extention, localstruct->extention)) {
			localstruct->module = in_module;
#if 0
			printf("Autodetect: Found module %8.8X\n", in_module);
#endif
		}
	}

	return;
}

static gchar * find_extention (gchar * in_str) {
	int i;
	for (i = strlen(in_str); i >= 0 && in_str[i] != '.'; i--);
	return &in_str[i+1];
}

static SPModule * sp_modulesys_autodetect (GtkType in_type, SPModuleDoc * in_doc) {
	modulesys_autodetect_t localstruct;

	localstruct.extention = find_extention(sp_module_doc_get_filename(in_doc));
	localstruct.type      = in_type;
	localstruct.module    = NULL;

#if 0
	printf("Autodetect: Looking for a '%s'\n", localstruct.extention);
#endif

	g_list_foreach(module_list, module_autodetect, (gpointer)&localstruct);

	return localstruct.module;
}

/* SVG in module - it's a built in */

static void svg_in_exec (SPModule *, SPModuleDoc *,  gpointer data);

static void sp_modulesys_init_svg_in(void) {
	SPModule * this_plug;
	SPModuleExec * this_exec;
	SPRepr *r;

	r = sp_repr_new ("input");
	sp_repr_set_attr (r, "id", SP_MODULE_KEY_INPUT_SVG);
	sp_repr_set_attr (r, "name", "Scalar Vector Graphics");
	sp_repr_set_attr (r, "extension", "svg");
	sp_repr_set_attr (r, "mimetype", "image/x-svg");
	this_plug = sp_module_input_new (r);
	sp_repr_unref (r);
	this_exec = SP_MODULE_EXEC(sp_module_exec_builtin_new());

	/* sp_module_set_about(this_plug, FALSE); */
	sp_module_set_exec(this_plug, this_exec);

	sp_module_exec_builtin_set_exec(SP_MODULE_EXEC_BUILTIN(this_exec), svg_in_exec, NULL);

	sp_modulesys_list_add(this_plug);
	return;
}

static void svg_in_exec (SPModule * in_module, SPModuleDoc * in_doc,  gpointer in_data) {
	SPDocument * doc;

	g_return_if_fail(SP_IS_MODULE_DOC(in_doc));

	doc = sp_document_new (sp_module_doc_get_filename(in_doc), TRUE, TRUE);
	g_return_if_fail (doc != NULL);

	sp_module_doc_set_document(in_doc, doc);

	return;
}

/* SVG out Sodipodi Namespace */
#include "sp-object-repr.h"

static void svg_out_exec (SPModule *, SPModuleDoc *,  gpointer data);
static void sp_modulesys_init_svg_out(void) {
	SPModule * this_plug;
	SPModuleExec * this_exec;
	SPRepr *r;

	r = sp_repr_new ("output");
	sp_repr_set_attr (r, "id", SP_MODULE_KEY_OUTPUT_SVG_SODIPODI);
	sp_repr_set_attr (r, "name", "Scalar Vector Graphics (Sodipodi Namespace)");
	sp_repr_set_attr (r, "extension", "svg");
	sp_repr_set_attr (r, "mimetype", "image/x-svg");
	this_plug = sp_module_output_new (r);
	sp_repr_unref (r);
	this_exec = SP_MODULE_EXEC(sp_module_exec_builtin_new());

	/* sp_module_set_about(this_plug, FALSE); */
	sp_module_set_exec(this_plug, this_exec);

	sp_module_exec_builtin_set_exec(SP_MODULE_EXEC_BUILTIN(this_exec), svg_out_exec, NULL);

	sp_modulesys_list_add(this_plug);
	return;
}

static void
svg_out_exec (SPModule * in_module, SPModuleDoc * in_doc,  gpointer in_data) {
	SPRepr *repr;
	gboolean spns;
	const GSList *images, *l;
	SPReprDoc *rdoc;
	SPDocument *doc;
	gchar * filename;
	gchar * save_path;

	g_return_if_fail(SP_IS_MODULE(in_module));
	g_return_if_fail(SP_IS_MODULE_DOC(in_doc));

	spns = TRUE;

	doc = sp_module_doc_get_document(in_doc);
	filename = sp_module_doc_get_filename(in_doc);

	save_path = g_dirname (filename);
	if (save_path) save_path = g_strconcat (save_path, "/", NULL);

	rdoc = NULL;
	repr = sp_document_repr_root (doc);
	sp_repr_set_attr (repr, "sodipodi:docbase", save_path);
	sp_repr_set_attr (repr, "sodipodi:docname", filename);

	images = sp_document_get_resource_list (doc, "image");
	for (l = images; l != NULL; l = l->next) {
		SPRepr *ir;
		const guchar *href, *relname;
		ir = SP_OBJECT_REPR (l->data);
		href = sp_repr_attr (ir, "xlink:href");
		if (spns && !g_path_is_absolute (href)) {
			href = sp_repr_attr (ir, "sodipodi:absref");
		}
		if (href && g_path_is_absolute (href)) {
			relname = sp_relative_path_from_path (href, save_path);
			sp_repr_set_attr (ir, "xlink:href", relname);
		}
	}

	sp_repr_save_file (sp_repr_document (repr), filename);

	sp_document_set_uri (doc, filename);
	return;
}

/* SVG out non-Sodipodi Namespace */
static void svg_nons_out_exec (SPModule *, SPModuleDoc *,  gpointer data);
static void sp_modulesys_init_svg_nons_out(void) {
	SPModule * this_plug;
	SPModuleExec * this_exec;
	SPRepr *r;

	r = sp_repr_new ("output");
	sp_repr_set_attr (r, "id", SP_MODULE_KEY_OUTPUT_SVG);
	sp_repr_set_attr (r, "name", "Scalar Vector Graphics");
	sp_repr_set_attr (r, "extension", "svg");
	sp_repr_set_attr (r, "mimetype", "image/x-svg");
	this_plug = sp_module_output_new (r);
	sp_repr_unref (r);

	this_exec = SP_MODULE_EXEC(sp_module_exec_builtin_new());

	/* sp_module_set_about(this_plug, FALSE); */
	sp_module_set_exec(this_plug, this_exec);

	sp_module_exec_builtin_set_exec(SP_MODULE_EXEC_BUILTIN(this_exec), svg_nons_out_exec, NULL);

	sp_modulesys_list_add(this_plug);
	return;
}

static void
svg_nons_out_exec (SPModule * in_module, SPModuleDoc * in_doc,  gpointer in_data) {
	SPRepr *repr;
	gboolean spns;
	const GSList *images, *l;
	SPReprDoc *rdoc;
	SPDocument *doc;
	gchar * filename;
	gchar * save_path;

	g_return_if_fail(SP_IS_MODULE(in_module));
	g_return_if_fail(SP_IS_MODULE_DOC(in_doc));

	spns = FALSE;

	doc = sp_module_doc_get_document(in_doc);
	filename = sp_module_doc_get_filename(in_doc);

	save_path = g_dirname (filename);
	if (save_path) save_path = g_strconcat (save_path, "/", NULL);

	rdoc = sp_repr_document_new ("svg");
	repr = sp_repr_document_root (rdoc);
	repr = sp_object_invoke_write (sp_document_root (doc), repr, SP_OBJECT_WRITE_BUILD);

	images = sp_document_get_resource_list (doc, "image");
	for (l = images; l != NULL; l = l->next) {
		SPRepr *ir;
		const guchar *href, *relname;
		ir = SP_OBJECT_REPR (l->data);
		href = sp_repr_attr (ir, "xlink:href");
		if (spns && !g_path_is_absolute (href)) {
			href = sp_repr_attr (ir, "sodipodi:absref");
		}
		if (href && g_path_is_absolute (href)) {
			relname = sp_relative_path_from_path (href, save_path);
			sp_repr_set_attr (ir, "xlink:href", relname);
		}
	}

	sp_repr_save_file (sp_repr_document (repr), filename);

	sp_document_set_uri (doc, filename);

	sp_repr_document_unref (rdoc);
	return;
}
