
#include "sp-module-sys.h"
#include "sp-module-exec.h"

/* Module Exec */
static void sp_module_exec_class_init          (SPModuleExecClass         * klass);
static void sp_module_exec_init                (SPModuleExec              * object);
static void sp_module_exec_finalize             (GObject                 * object);
static void sp_module_exec_load                (SPModule * object);
static void sp_module_exec_unload              (SPModule * object);
static void sp_module_exec_prefs               (SPModule * object, SPModuleDoc * doc);
static void sp_module_exec_exec                (SPModule * object, SPModuleDoc * doc);
/* Module Exec Builtin */
static void sp_module_exec_builtin_class_init  (SPModuleExecBuiltinClass  * klass);
static void sp_module_exec_builtin_init        (SPModuleExecBuiltin       * object);
static void sp_module_exec_builtin_finalize     (GObject                 * object);
static void sp_module_exec_builtin_load        (SPModule * object);
static void sp_module_exec_builtin_unload      (SPModule * object);
static void sp_module_exec_builtin_prefs       (SPModule * object, SPModuleDoc * doc);
static void sp_module_exec_builtin_exec        (SPModule * object, SPModuleDoc * doc);

/* Globals */
static GObjectClass * base_parent_class;
static SPModuleExecClass * builtin_parent_class;

/* Module Exec functions */

GType
sp_module_handler_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModuleHandlerClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_exec_class_init,
			NULL, NULL,
			sizeof (SPModuleHandler),
			16,
			(GInstanceInitFunc) sp_module_exec_init,
		};
		type = g_type_register_static (G_TYPE_OBJECT, "SPModuleHandler", &info, 0);
	}
	return type;
}

static void sp_module_exec_class_init (SPModuleExecClass * klass) {
    GObjectClass * g_object_class;

    g_object_class = (GObjectClass *)klass;

    base_parent_class = g_type_class_peek_parent (klass);

    g_object_class->finalize = sp_module_exec_finalize;

    klass->load = sp_module_exec_load;
    klass->unload = sp_module_exec_unload;
    klass->prefs = sp_module_exec_prefs;
    klass->exec = sp_module_exec_exec;

    return;
}

static void sp_module_exec_init (SPModuleExec * object) {
    g_return_if_fail(SP_IS_MODULE_EXEC(object));
    object->prefs = NULL;
	object->state = SP_MODULE_EXEC_UNLOADED;
    return;
}

SPModuleExec * sp_module_exec_new (void) {
    SPModuleExec * retval;

    retval = (SPModuleExec *) g_object_new (SP_TYPE_MODULE_EXEC, NULL);

    return retval;
}

static void sp_module_exec_finalize (GObject * object) {
    /* Need to destroy the prefs list */
	G_OBJECT_CLASS (base_parent_class)->finalize (object);
}

static void sp_module_exec_load (SPModule * object) {
	g_return_if_fail(SP_IS_MODULE(object));
	g_return_if_fail(SP_IS_MODULE_EXEC(object->exec));

	object->exec->state = SP_MODULE_EXEC_LOADED;
    return;
}

static void sp_module_exec_unload (SPModule * object) {
	g_return_if_fail(SP_IS_MODULE(object));
	g_return_if_fail(SP_IS_MODULE_EXEC(object->exec));

	object->exec->state = SP_MODULE_EXEC_UNLOADED;
    return;
}

static void sp_module_exec_prefs (SPModule * object, SPModuleDoc * doc) {
	g_return_if_fail(SP_IS_MODULE(object));
	g_return_if_fail(SP_IS_MODULE_DOC(doc));

	sp_module_sys_prefs_complete(object, doc, TRUE);
	return;
}

static void sp_module_exec_exec (SPModule * object, SPModuleDoc * doc) {
	g_return_if_fail(SP_IS_MODULE(object));
	g_return_if_fail(SP_IS_MODULE_DOC(doc));

       g_object_unref (G_OBJECT(doc));
    return;
}

/* Module Exec Builtin functions */

GType
sp_module_exec_builtin_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModuleExecBuiltinClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_exec_builtin_class_init,
			NULL, NULL,
			sizeof (SPModuleExecBuiltin),
			16,
			(GInstanceInitFunc) sp_module_exec_builtin_init,
		};
		type = g_type_register_static (SP_TYPE_MODULE_EXEC, "SPModuleExecBuiltin", &info, 0);
	}
	return type;
}

static void sp_module_exec_builtin_class_init (SPModuleExecBuiltinClass * klass) {
    GObjectClass * g_object_class;
    SPModuleExecClass * module;

    g_object_class = (GObjectClass *)klass;
    module = (SPModuleExecClass *)klass;

    builtin_parent_class = g_type_class_peek_parent (klass);

    g_object_class->finalize = sp_module_exec_builtin_finalize;

    module->load = sp_module_exec_builtin_load;
    module->unload = sp_module_exec_builtin_unload;
    module->prefs = sp_module_exec_builtin_prefs;
    module->exec = sp_module_exec_builtin_exec;
}

static void sp_module_exec_builtin_init (SPModuleExecBuiltin * object) {
	g_return_if_fail(SP_IS_MODULE_EXEC_BUILTIN(object));
    object->load = NULL;
    object->load_data = NULL;
    object->unload = NULL;
    object->unload_data = NULL;
    object->prefs = NULL;
    object->prefs_data = NULL;
    object->exec = NULL;
    object->exec_data = NULL;
    return;
}

SPModuleExecBuiltin * sp_module_exec_builtin_new (void) {
    SPModuleExecBuiltin * retval;

    retval = (SPModuleExecBuiltin *) g_object_new (SP_TYPE_MODULE_EXEC_BUILTIN, NULL);

    return retval;
}

static void sp_module_exec_builtin_finalize (GObject * object) {

	G_OBJECT_CLASS (builtin_parent_class)->finalize (object);
}

static void sp_module_exec_builtin_load (SPModule * object) {
    SPModuleExecBuiltin * module;

    g_return_if_fail(SP_IS_MODULE(object));
    g_return_if_fail(SP_IS_MODULE_EXEC_BUILTIN(object->exec));

    module = SP_MODULE_EXEC_BUILTIN(object->exec);
    /* returns are used here so that the stack doesn't grow */
    if (module->load != NULL) {
        module->load(object, module->load_data);
    } else {
        sp_module_exec_load(object);
    }
}

static void sp_module_exec_builtin_unload (SPModule * object) {
    SPModuleExecBuiltin * module;

    g_return_if_fail(SP_IS_MODULE(object));
    g_return_if_fail(SP_IS_MODULE_EXEC_BUILTIN(object->exec));

    module = SP_MODULE_EXEC_BUILTIN(object->exec);
    /* returns are used here so that the stack doesn't grow */
    if (module->unload != NULL) {
        module->unload(object, module->unload_data);
    } else {
        sp_module_exec_unload(object);
    }
}

static void sp_module_exec_builtin_prefs (SPModule * object, SPModuleDoc * doc) {
    SPModuleExecBuiltin * module;

    g_return_if_fail(SP_IS_MODULE(object));
    g_return_if_fail(SP_IS_MODULE_EXEC_BUILTIN(object->exec));
	g_return_if_fail(SP_IS_MODULE_DOC(doc));

    module = SP_MODULE_EXEC_BUILTIN(object->exec);
    /* returns are used here so that the stack doesn't grow */
    if (module->prefs != NULL) {
        module->prefs(object, doc, module->prefs_data);
    } else {
        sp_module_exec_prefs(object, doc);
    }
}

static void sp_module_exec_builtin_exec (SPModule * object, SPModuleDoc * doc) {
    SPModuleExecBuiltin * module;

    g_return_if_fail(SP_IS_MODULE(object));
    g_return_if_fail(SP_IS_MODULE_EXEC_BUILTIN(object->exec));
	g_return_if_fail(SP_IS_MODULE_DOC(doc));

    module = SP_MODULE_EXEC_BUILTIN(object->exec);
    /* returns are used here so that the stack doesn't grow */
    if (module->exec != NULL) {
        module->exec(object, doc, module->exec_data);
    } else {
        sp_module_exec_exec(object, doc);
    }
}

void sp_module_exec_builtin_set_load (SPModuleExecBuiltin * object,
                                      void (*in_func)(SPModule *, gpointer),
                                      gpointer in_data) {
    g_return_if_fail(SP_IS_MODULE_EXEC_BUILTIN(object));
    g_return_if_fail(in_func != NULL);

    object->load =      in_func;
    object->load_data = in_data;

    return;
}

void sp_module_exec_builtin_set_unload (SPModuleExecBuiltin * object,
                                        void (*in_func)(SPModule *, gpointer),
                                        gpointer in_data) {
    g_return_if_fail(SP_IS_MODULE_EXEC_BUILTIN(object));
    g_return_if_fail(in_func != NULL);

    object->unload =      in_func;
    object->unload_data = in_data;

    return;
}

void sp_module_exec_builtin_set_prefs (SPModuleExecBuiltin * object,
                                       void (*in_func)(SPModule *, SPModuleDoc *, gpointer),
                                       gpointer in_data) {
    g_return_if_fail(SP_IS_MODULE_EXEC_BUILTIN(object));
    g_return_if_fail(in_func != NULL);

    object->prefs =      in_func;
    object->prefs_data = in_data;

    return;
}

void sp_module_exec_builtin_set_exec (SPModuleExecBuiltin * object,
                                      void (*in_func)(SPModule *, SPModuleDoc *, gpointer),
                                      gpointer in_data) {
    g_return_if_fail(SP_IS_MODULE_EXEC_BUILTIN(object));
    g_return_if_fail(in_func != NULL);

    object->exec =      in_func;
    object->exec_data = in_data;

    return;
}
