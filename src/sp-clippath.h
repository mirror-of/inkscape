#ifndef __SP_CLIPPATH_H__
#define __SP_CLIPPATH_H__

/*
 * SVG <clipPath> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_CLIPPATH (sp_clippath_get_type ())
#define SP_CLIPPATH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CLIPPATH, SPClipPath))
#define SP_CLIPPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_CLIPPATH, SPClipPathClass))
#define SP_IS_CLIPPATH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CLIPPATH))
#define SP_IS_CLIPPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_CLIPPATH))

typedef struct _SPClipPathView SPClipPathView;

#include "display/nr-arena-forward.h"
#include "sp-object-group.h"

struct _SPClipPath {
	SPObjectGroup group;

	unsigned int clipPathUnits_set : 1;
	unsigned int clipPathUnits : 1;

	SPClipPathView *display;
};

struct _SPClipPathClass {
	SPObjectGroupClass parent_class;
};

GType sp_clippath_get_type (void);

NRArenaItem *sp_clippath_show (SPClipPath *cp, NRArena *arena, unsigned int key);
void sp_clippath_hide (SPClipPath *cp, unsigned int key);

void sp_clippath_set_bbox (SPClipPath *cp, unsigned int key, NRRectF *bbox);

#endif
