#define SP_APPLICATION_C

#include "application.h"

SPApp * sp_app = NULL;

SPApp *
sp_app_new (void)
{
	sp_app = g_new (SPApp, 1);

	return sp_app;
}
