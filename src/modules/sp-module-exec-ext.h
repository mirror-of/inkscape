#ifndef __SP_MODULE_EXEC_EXT_H__
#define __SP_MODULE_EXEC_EXT_H__

#include "sp-module-exec.h"


typedef struct _SPModuleExecExt      SPModuleExecExt;
typedef struct _SPModuleExecExtClass SPModuleExecExtClass;

struct _SPModuleExecExt {
    SPModuleExec object;
	gchar *      command;
};

struct _SPModuleExecExtClass {
    SPModuleExecClass parent_class;
};

#define SP_TYPE_MODULE_EXEC_EXT (sp_module_exec_ext_get_type())
#define SP_MODULE_EXEC_EXT(o)  (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_EXEC_EXT, SPModuleExecExt))
#define SP_IS_MODULE_EXEC_EXT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_EXEC_EXT))


GType               sp_module_exec_ext_get_type    (void);
SPModuleExecExt *     sp_module_exec_ext_new         (void);
gchar *               sp_module_exec_ext_set_command (SPModuleExecExt * object,
                                                      gchar *           command);

#endif /*  __SP_MODULE_EXEC_EXT_H__ */
