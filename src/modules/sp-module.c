#include "sp-module-exec.h"

SPModuleHandler *
sp_module_set_exec (SPModule * object, 
		                           SPModuleHandler * exec) {
	g_return_val_if_fail(SP_IS_MODULE(object), NULL);
	g_return_val_if_fail(SP_IS_MODULE_EXEC(exec), NULL);

	if (object->exec != NULL) {
		/* TODO: Destroy here */

	}
	object->exec = exec;

	return exec;
}

/* Module Input */

/* Module Output */

/* Module Filter */

