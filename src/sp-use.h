#ifndef __SP_USE_H__
#define __SP_USE_H__

/*
 * SVG <use> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "svg/svg-types.h"
#include "sp-item.h"

class SPUseReference : public Inkscape::URIReference {
public:
	SPUseReference(SPObject *obj) : URIReference(obj) {}
	SPItem *getObject() const {
		return (SPItem *)URIReference::getObject();
	}
protected:
	bool _acceptObject(SPObject *obj) const {
		return SP_IS_ITEM(obj);
	}
};


#define SP_TYPE_USE            (sp_use_get_type ())
#define SP_USE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_USE, SPUse))
#define SP_USE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_USE, SPUseClass))
#define SP_IS_USE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_USE))
#define SP_IS_USE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_USE))

class SPUse;
class SPUseClass;

struct SPUse {
	// this item (invisible)
	SPItem item;

	// item built from the original's repr (the visible clone)
	// relative to the SPUse itself, it is treated as a child, similar to a grouped item relative to its group
	SPObject *child;

	// SVG attrs
	SPSVGLength x;
	SPSVGLength y;
	SPSVGLength width;
	SPSVGLength height;
	gchar *href;

	// the reference to the original object
	SPUseReference *ref;

	// the bbox of the original, for sensing its movements and adjusting
	NR::Rect original;

	// the original's repr that we listen to
	SPRepr *repr;
};

struct SPUseClass {
	SPItemClass parent_class;
};

GType sp_use_get_type (void);

SPItem *sp_use_unlink (SPUse *use);
SPItem *sp_use_get_original (SPUse *use);

#endif
