#ifndef __SP_OFFSET_H__
#define __SP_OFFSET_H__

/*
 * <sodipodi:offset> implementation
 *
 * Authors (of the sp-spiral.h upon which this file was created):
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-shape.h"



#define SP_TYPE_OFFSET            (sp_offset_get_type ())
#define SP_OFFSET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_OFFSET, SPOffset))
#define SP_OFFSET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_OFFSET, SPOffsetClass))
#define SP_IS_OFFSET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_OFFSET))
#define SP_IS_OFFSET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_OFFSET))

typedef struct _SPOffset SPOffset;
typedef struct _SPOffsetClass SPOffsetClass;

struct _SPOffset
{
  SPShape shape;

  /*
   * offset is defined by curve and radius
   * the original curve is kept as a path in a sodipodi:original attribute
   * it's not possible to change the original curve
   */
  void *originalPath;
  char *original;
  float rad;			/* offset radius */
  char *sourceObject;
  SPRepr *sourceRepr;
  bool   sourceDirty;

  bool knotSet;
  NR::Point knot;
};

struct _SPOffsetClass
{
  SPShapeClass parent_class;
};


/* Standard Gtk function */
GType sp_offset_get_type (void);

double sp_offset_distance_to_original (SPOffset * offset, NR::Point px);
void sp_offset_top_point (SPOffset * offset, NR::Point *px);


#endif
