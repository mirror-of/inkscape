#include <config.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkcolorsel.h>
#include "../color.h"
#include "../helper/sp-intl.h"
#include "sp-color-preview.h"
#include "sp-color-selector.h"

enum {
	GRABBED,
	DRAGGED,
	RELEASED,
	CHANGED,
	LAST_SIGNAL
};


static void sp_color_selector_class_init (SPColorSelectorClass *klass);
static void sp_color_selector_init (SPColorSelector *csel);
static void sp_color_selector_destroy (GtkObject *object);

static void sp_color_selector_show_all (GtkWidget *widget);
static void sp_color_selector_hide_all (GtkWidget *widget);

static GtkVBoxClass *parent_class;
static guint csel_signals[LAST_SIGNAL] = {0};

GType
sp_color_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (SPColorSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) sp_color_selector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (SPColorSelector),
			0,	  /* n_preallocs */
			(GInstanceInitFunc) sp_color_selector_init,
		};

		type = g_type_register_static (GTK_TYPE_VBOX,
									   "SPColorSelector",
									   &info,
									   INK_STATIC_CAST(GTypeFlags, 0));
	}
	return type;
}

static void
sp_color_selector_class_init (SPColorSelectorClass *klass)
{
	static const gchar* nameset[] = {_("Unnamed"), 0};
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = (GtkVBoxClass*)gtk_type_class (GTK_TYPE_VBOX);

	csel_signals[GRABBED] = gtk_signal_new ("grabbed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, grabbed),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	csel_signals[DRAGGED] = gtk_signal_new ("dragged",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, dragged),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	csel_signals[RELEASED] = gtk_signal_new ("released",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, released),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);
	csel_signals[CHANGED] = gtk_signal_new ("changed",
						 (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
						 GTK_CLASS_TYPE(object_class),
						 GTK_SIGNAL_OFFSET (SPColorSelectorClass, changed),
						 gtk_marshal_NONE__NONE,
						 GTK_TYPE_NONE, 0);

	klass->name = nameset;
	klass->submode_count = 1;

	klass->set_submode = 0;
	klass->get_submode = 0;

	klass->set_color_alpha = 0;
	klass->get_color_alpha = 0;


	object_class->destroy = sp_color_selector_destroy;

	widget_class->show_all = sp_color_selector_show_all;
	widget_class->hide_all = sp_color_selector_hide_all;

}

void sp_color_selector_init (SPColorSelector *csel)
{
/*	   gtk_signal_connect (GTK_OBJECT (csel->rgbae), "changed", GTK_SIGNAL_FUNC (sp_color_selector_rgba_entry_changed), csel); */
}

static void
sp_color_selector_destroy (GtkObject *object)
{
	SPColorSelector *csel;

	csel = SP_COLOR_SELECTOR (object);

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_selector_show_all (GtkWidget *widget)
{
	gtk_widget_show (widget);
}

static void
sp_color_selector_hide_all (GtkWidget *widget)
{
	gtk_widget_hide (widget);
}

GtkWidget *
sp_color_selector_new (GType selector_type, SPColorSpaceType colorspace)
{
	SPColorSelector *csel;
	SPColor color;
	g_return_val_if_fail (g_type_is_a (selector_type, SP_TYPE_COLOR_SELECTOR), NULL);

	csel = SP_COLOR_SELECTOR (g_object_new (selector_type, NULL));

	sp_color_set_rgb_rgba32 (&color, 0);
	sp_color_selector_set_color_alpha (SP_COLOR_SELECTOR(csel), &color, 1.0);

	return GTK_WIDGET (csel);
}


void sp_color_selector_set_submode (SPColorSelector* csel, guint submode)
{
	if ( csel && SP_IS_COLOR_SELECTOR (csel) )
	{
		SPColorSelectorClass* selector_class = SP_COLOR_SELECTOR_GET_CLASS (csel);
		if ( selector_class->set_submode )
		{
			(selector_class->set_submode)(csel, submode);
		}
	}
}

guint sp_color_selector_get_submode (SPColorSelector* csel)
{
	guint mode = 0;
	if ( csel && SP_IS_COLOR_SELECTOR (csel) )
	{
		SPColorSelectorClass* selector_class = SP_COLOR_SELECTOR_GET_CLASS (csel);
		if ( selector_class->get_submode )
		{
			mode = (selector_class->get_submode)(csel);
		}
	}
	return mode;
}

void sp_color_selector_set_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha)
{
	if ( csel && SP_IS_COLOR_SELECTOR (csel) )
	{
		SPColorSelectorClass* selector_class = SP_COLOR_SELECTOR_GET_CLASS (csel);
		if ( selector_class->set_color_alpha )
		{
			(selector_class->set_color_alpha)(csel, color, alpha);
		}
		else
		{
			if ( !sp_color_is_close (color, &csel->color, 1e-4)
				 || (fabs ((csel->alpha) - (alpha)) >= 1e-4) )
			{
				sp_color_copy (&csel->color, color);
				csel->alpha = alpha;
				g_warning ("file %s: line %d: About to signal CHANGED to color %08x", __FILE__, __LINE__, sp_color_get_rgba32_falpha(color,alpha));
				gtk_signal_emit (GTK_OBJECT (csel), csel_signals[CHANGED]);
			}
		}
	}
}

void sp_color_selector_get_color_alpha (SPColorSelector *csel, SPColor *color, gfloat *alpha)
{
	gint i = 0;
	if ( csel && SP_IS_COLOR_SELECTOR (csel) )
	{
		SPColorSelectorClass* selector_class = SP_COLOR_SELECTOR_GET_CLASS (csel);
		if ( selector_class->get_color_alpha )
		{
			(selector_class->get_color_alpha)(csel, color, alpha);
		}
		else
		{
			sp_color_copy (color, &csel->color);
			*alpha = csel->alpha;
		}
		// Try to catch uninitialized value usage
		if ( color->colorspace )
		{
			i++;
		}
		if ( color->v.c[0] )
		{
			i++;
		}
		if ( color->v.c[1] )
		{
			i++;
		}
		if ( color->v.c[2] )
		{
			i++;
		}
		if ( color->v.c[3] )
		{
			i++;
		}
		if ( alpha && *alpha )
		{
			i++;
		}
	}
}

