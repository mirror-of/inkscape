#ifndef __SP_ROOT_H__
#define __SP_ROOT_H__

/*
 * SVG <svg> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_ROOT (sp_root_get_type ())
#define SP_ROOT(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_ROOT, SPRoot))
#define SP_ROOT_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), SP_TYPE_ROOT, SPRootClass))
#define SP_IS_ROOT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_ROOT))
#define SP_IS_ROOT_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SP_TYPE_ROOT))

#include <libnr/nr-matrix.h>
#include "svg/svg-types.h"
#include "enums.h"
#include "sp-item-group.h"
#include "version.h"

struct _SPRoot {
	SPGroup group;

	struct {
		SPVersion svg, sodipodi, inkscape;
	} version, original;

	SPSVGLength x;
	SPSVGLength y;
	SPSVGLength width;
	SPSVGLength height;

	/* viewBox; */
	unsigned int viewBox_set : 1;
	NRRect viewBox;

	/* preserveAspectRatio */
	unsigned int aspect_set : 1;
	unsigned int aspect_align : 4;
	unsigned int aspect_clip : 1;

	/* Child to parent additional transform */
	NRMatrix c2p;

	/* List of namedviews */
	/* fixme: use single container instead */
	/* GSList *namedviews; */
	/* Root-level <defs> node */
	SPDefs *defs;
};

struct _SPRootClass {
	SPGroupClass parent_class;
};

GType sp_root_get_type (void);

#endif
