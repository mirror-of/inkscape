#ifndef __SP_RULER_H__
#define __SP_RULER_H__

/*
 * Customized ruler class for sodipodi
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtkruler.h>
#include "sp-metrics.h"

void sp_ruler_set_metric (GtkRuler * ruler, SPMetric  metric);


#define SP_HRULER(obj)          GTK_CHECK_CAST (obj, sp_hruler_get_type (), SPHRuler)
#define SP_HRULER_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, sp_hruler_get_type (), SPHRulerClass)
#define SP_IS_HRULER(obj)       GTK_CHECK_TYPE (obj, sp_hruler_get_type ())


typedef struct _SPHRuler       SPHRuler;
typedef struct _SPHRulerClass  SPHRulerClass;

struct _SPHRuler
{
  GtkRuler ruler;
};

struct _SPHRulerClass
{
  GtkRulerClass parent_class;
};


GtkType    sp_hruler_get_type (void);
GtkWidget* sp_hruler_new      (void);



// vruler



#define SP_VRULER(obj)          GTK_CHECK_CAST (obj, sp_vruler_get_type (), SPVRuler)
#define SP_VRULER_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, sp_vruler_get_type (), SPVRulerClass)
#define SP_IS_VRULER(obj)       GTK_CHECK_TYPE (obj, sp_vruler_get_type ())


typedef struct _SPVRuler       SPVRuler;
typedef struct _SPVRulerClass  SPVRulerClass;

struct _SPVRuler
{
  GtkRuler ruler;
};

struct _SPVRulerClass
{
  GtkRulerClass parent_class;
};


GtkType    sp_vruler_get_type (void);
GtkWidget* sp_vruler_new      (void);


#endif /* __SP_RULER_H__ */
