#ifndef SP_EMBEDDABLE_DESKTOP_H
#define SP_EMBEDDABLE_DESKTOP_H

#include <config.h>
#include <gnome.h>
#include <bonobo.h>
#include "../forward.h"
#include "../desktop.h"
#include "embeddable-document.h"

#define SP_EMBEDDABLE_DESKTOP_TYPE	(sp_embeddable_desktop_get_type ())
#define SP_EMBEDDABLE_DESKTOP(o)  	(GTK_CHECK_CAST ((o), SP_EMBEDDABLE_DESKTOP_TYPE, SPEmbeddableDesktop))
#define SP_EMBEDDABLE_DESKTOP_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), SP_EMBEDDABLE_DESKTOP_TYPE, SPEmbeddableDesktopClass))
#define IS_SP_EMBEDDABLE_DESKTOP(o)       (GTK_CHECK_TYPE ((o), SP_EMBEDDABLE_DESKTOP_TYPE))
#define IS_SP_EMBEDDABLE_DESKTOP_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_EMBEDDABLE_DESKTOP_TYPE))

/*
 * SPEmbeddableDesktop
 *
 */

typedef struct _SPEmbeddableDesktop SPEmbeddableDesktop;
typedef struct _SPEmbeddableDesktopClass SPEmbeddableDesktopClass;

struct _SPEmbeddableDesktop {
	BonoboView view;
	SPEmbeddableDocument * document;
	SPDesktopWidget * desktop;
};

struct _SPEmbeddableDesktopClass {
	BonoboViewClass parent_class;
};

GtkType sp_embeddable_desktop_get_type (void);

/*
 * Constructor
 */

BonoboView * sp_embeddable_desktop_factory (BonoboEmbeddable * embeddable,
	const Bonobo_ViewFrame view_frame, gpointer data);

/*
 * Notify desktop, that underlying SPDocument has changed
 */

void sp_embeddable_desktop_new_doc (BonoboView * view, gpointer data);

#endif
