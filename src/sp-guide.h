#ifndef SP_GUIDE_H
#define SP_GUIDE_H

/*
 * SPGuide
 *
 * A guideline
 *
 * Copyright (C) Lauris Kaplinski 2000
 *
 */

#include "helper/helper-forward.h"
#include "sp-object.h"

typedef enum {
	SP_GUIDE_HORIZONTAL,
	SP_GUIDE_VERTICAL
} SPGuideOrientation;

#define SP_TYPE_GUIDE            (sp_guide_get_type ())
#define SP_GUIDE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_GUIDE, SPGuide))
#define SP_GUIDE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_GUIDE, SPGuideClass))
#define SP_IS_GUIDE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_GUIDE))
#define SP_IS_GUIDE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_GUIDE))

struct _SPGuide {
	SPObject object;
	SPGuideOrientation orientation;
	gdouble position;
	guint32 color;
	guint32 hicolor;
	GSList * views;
};

struct _SPGuideClass {
	SPObjectClass parent_class;
};

GType sp_guide_get_type (void);

void sp_guide_show (SPGuide * guide, SPCanvasGroup * group, gpointer handler);
void sp_guide_hide (SPGuide * guide, SPCanvas * canvas);
void sp_guide_sensitize (SPGuide * guide, SPCanvas * canvas, gboolean sensitive);

void sp_guide_moveto (SPGuide * guide, gdouble x, gdouble y);
void sp_guide_position_set (SPGuide * guide, gdouble x, gdouble y);
void sp_guide_remove (SPGuide * guide);

gint sp_guide_compare (gconstpointer a, gconstpointer b);

#endif

