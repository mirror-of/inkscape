#ifndef SP_APPLICATION_H
#define SP_APPLICATION_H

/*
 * SPApp
 *
 * It is the parent of all documents, desktops et al.
 * Currently completely emty thing, event not an GtkObject
 * In future it should be GnomeMDI or something similar
 *
 */

#include <glib.h>

typedef struct _SPApp SPApp;

struct _SPApp {
	gint dummy;
};

#ifndef SP_APPLICATION_C
extern SPApp * sp_app;
#endif

SPApp * sp_app_new (void);

#define SP_APP_MAIN sp_app

#endif
