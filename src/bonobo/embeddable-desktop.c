#define SP_EMBEDDABLE_DESKTOP_C

#include "../sodipodi.h"
#include "../document.h"
#include "../desktop.h"
#include "embeddable-desktop.h"

static void sp_embeddable_desktop_class_init (GtkObjectClass * klass);
static void sp_embeddable_desktop_init (GtkObject * object);

static void sp_embeddable_desktop_destroyed (SPEmbeddableDesktop * desktop);
static void sp_embeddable_desktop_activate (BonoboView * view, gboolean activate, gpointer data);

/* fixme: this should go to main desktop */
void sp_embeddable_desktop_size_allocate (GtkWidget * widget, GtkAllocation * allocation, gpointer data);

GtkType
sp_embeddable_desktop_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPEmbeddableDesktop",
			sizeof (SPEmbeddableDesktop),
			sizeof (SPEmbeddableDesktopClass),
			(GtkClassInitFunc) sp_embeddable_desktop_class_init,
			(GtkObjectInitFunc) sp_embeddable_desktop_init,
			NULL, NULL, NULL
		};
		type = bonobo_x_type_unique (BONOBO_EMBEDDABLE_TYPE, NULL, NULL, 0, &info);
	}
	return type;
}

static void
sp_embeddable_desktop_class_init (GtkObjectClass * klass)
{
}

static void
sp_embeddable_desktop_init (GtkObject * object)
{
	SPEmbeddableDesktop * desktop;

	desktop = (SPEmbeddableDesktop *) object;

	desktop->document = NULL;
	desktop->desktop = NULL;
}

BonoboView *
sp_embeddable_desktop_factory (BonoboEmbeddable * embeddable,
	const Bonobo_ViewFrame view_frame, gpointer data)
{
	SPEmbeddableDesktop * desktop;
	SPNamedView * namedview;

	desktop = gtk_type_new (SP_EMBEDDABLE_DESKTOP_TYPE);

	desktop->document = SP_EMBEDDABLE_DOCUMENT (embeddable);

	/* Create SPesktop */

	namedview = sp_document_namedview (desktop->document->document, NULL);
	g_assert (namedview != NULL);

	desktop->desktop = (SPDesktopWidget *) sp_desktop_widget_new (namedview);

	/* Hide scrollbars and rulers */

	sp_desktop_show_decorations (desktop->desktop->desktop, FALSE);

	gtk_signal_connect (GTK_OBJECT (desktop->desktop), "size_allocate",
		GTK_SIGNAL_FUNC (sp_embeddable_desktop_size_allocate), desktop);

	bonobo_view_construct (BONOBO_VIEW (desktop),
		GTK_WIDGET (desktop->desktop));

	gtk_widget_show (GTK_WIDGET (desktop->desktop));

#if 0
	gnome_mdi_register (SODIPODI, GTK_OBJECT (desktop));
#endif

	gtk_signal_connect (GTK_OBJECT (desktop), "destroy",
		GTK_SIGNAL_FUNC (sp_embeddable_desktop_destroyed), NULL);
	gtk_signal_connect (GTK_OBJECT (desktop), "activate",
		GTK_SIGNAL_FUNC (sp_embeddable_desktop_activate), NULL);

	return BONOBO_VIEW (desktop);
}

static void
sp_embeddable_desktop_destroyed (SPEmbeddableDesktop * desktop)
{
#if 0
	gnome_mdi_unregister (SODIPODI, GTK_OBJECT (desktop));
#endif
}

static void
sp_embeddable_desktop_activate (BonoboView * view, gboolean activate, gpointer data)
{
	SPEmbeddableDesktop * desktop;

	desktop = SP_EMBEDDABLE_DESKTOP (view);

	sp_desktop_show_decorations (desktop->desktop->desktop, activate);

	bonobo_view_activate_notify (view, activate);
}

void
sp_embeddable_desktop_new_doc (BonoboView * view, gpointer data)
{
	SPEmbeddableDesktop * desktop;

	desktop = SP_EMBEDDABLE_DESKTOP (view);

	sp_desktop_change_document (desktop->desktop->desktop, desktop->document->document);
}

/* fixme:
 * Different aspect modes
 * Different anchor modes
 * Change only if not activated
 */

void
sp_embeddable_desktop_size_allocate (GtkWidget * widget, GtkAllocation * allocation, gpointer data)
{
	SPEmbeddableDesktop * ed;
	SPDesktopWidget *dtw;

	dtw = SP_DESKTOP_WIDGET (widget);
	ed = SP_EMBEDDABLE_DESKTOP (data);
	g_assert (ed->desktop == dtw);
	
	sp_desktop_show_region (dtw->desktop, 0.0, 0.0,
		sp_document_width (ed->document->document),
		sp_document_height (ed->document->document),
				10);
}


