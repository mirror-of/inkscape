/* This is done to solve a dependency problem between the two files
 * there shouldn't be any more issues  */
#include "sp-module.h"

#ifndef __SODIPODI_MODULE_EXEC_H__
#define __SODIPODI_MODULE_EXEC_H__

#include "sp-module-doc.h"

/* Module Exec */
typedef struct _SPModuleHandler SPModuleExec;
typedef struct _SPModuleHandlerClass SPModuleExecClass;

typedef enum {
	SP_MODULE_EXEC_UNLOADED,
	SP_MODULE_EXEC_LOADED
} SPModuleExecState;

struct _SPModuleHandler {
	GObject           object;
	GList *             prefs;
	SPModuleExecState   state;
};

struct _SPModuleHandlerClass {
	GObjectClass parent_class;
	void        (* load)(SPModule *);
	void        (* unload)(SPModule *);
	void        (* prefs)(SPModule *, SPModuleDoc *);
	void        (* exec)(SPModule *,  SPModuleDoc *);
};

#define SP_TYPE_MODULE_EXEC SP_TYPE_MODULE_HANDLER
#define SP_MODULE_EXEC SP_MODULE_HANDLER
#define SP_IS_MODULE_EXEC SP_IS_MODULE_HANDLER

GType        sp_module_handler_get_type  (void);

/* Module Exec Builtin */
typedef struct _SPModuleExecBuiltin      SPModuleExecBuiltin;
typedef struct _SPModuleExecBuiltinClass SPModuleExecBuiltinClass;

struct _SPModuleExecBuiltin {
    SPModuleExec object;
    void   (* load)(SPModule *, gpointer data);
    gpointer  load_data;
    void   (* unload)(SPModule *, gpointer data);
    gpointer  unload_data;
    void   (* prefs)(SPModule *, SPModuleDoc *,  gpointer data);
    gpointer  prefs_data;
    void   (* exec)(SPModule *, SPModuleDoc *,  gpointer data);
    gpointer  exec_data;
};

struct _SPModuleExecBuiltinClass {
    SPModuleExecClass parent_class;
};

#define SP_TYPE_MODULE_EXEC_BUILTIN (sp_module_exec_builtin_get_type())
#define SP_MODULE_EXEC_BUILTIN(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_EXEC_BUILTIN, SPModuleExecBuiltin))
#define SP_IS_MODULE_EXEC_BUILTIN(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_EXEC_BUILTIN))

GType                 sp_module_exec_builtin_get_type   (void);
SPModuleExecBuiltin * sp_module_exec_builtin_new        (void);
void                  sp_module_exec_builtin_set_load   (SPModuleExecBuiltin * object,
                                                         void (*in_func)(SPModule *, gpointer data),
                                                         gpointer in_data);
void                  sp_module_exec_builtin_set_unload (SPModuleExecBuiltin * object,
                                                         void (*in_func)(SPModule *, gpointer data),
                                                         gpointer in_data);
void                  sp_module_exec_builtin_set_prefs  (SPModuleExecBuiltin * object,
                                                         void (*in_func)(SPModule *, SPModuleDoc *,  gpointer data),
                                                         gpointer in_data);
void                  sp_module_exec_builtin_set_exec   (SPModuleExecBuiltin * object,
                                                         void (*in_func)(SPModule *, SPModuleDoc *,  gpointer data),
                                                         gpointer in_data);


#endif /* __SODIPODI_MODULE_EXEC_H__ */
