#ifndef __SODIPODI_MODULE_H__
#define __SODIPODI_MODULE_H__

#include "module.h"

SPModuleHandler *   sp_module_set_exec      (SPModule *     object,
		                                  SPModuleHandler * exec);
#define sp_module_input_new(r) sp_module_new (SP_TYPE_MODULE_INPUT, r)

#define sp_module_output_new(r) sp_module_new (SP_TYPE_MODULE_OUTPUT, r)

#define sp_module_filter_new(r) sp_module_new (SP_TYPE_MODULE_FILTER, r)

#endif  /* __SODIPODI_MODULE_H__ */
