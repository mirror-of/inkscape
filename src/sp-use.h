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

#include <sigc++/sigc++.h>
#include "svg/svg-types.h"
#include "sp-item.h"
#include "uri-references.h"

class SPUseReference : public Inkscape::URIReference {
public:
	SPUseReference(SPObject *owner) : URIReference(owner) {}

	SPItem *getObject() const {
		return (SPItem *)URIReference::getObject();
	}
protected:
	virtual bool _acceptObject(SPObject * const obj) const {
		if (SP_IS_ITEM(obj)) {
			SPObject * const owner = getOwner();
			/* Refuse references to us or to an ancestor. */
			for ( SPObject *iter = owner ; iter ; iter = SP_OBJECT_PARENT(iter) ) {
				if ( iter == obj ) {
					return false;
				}
			}
			return true;
		} else {
			return false;
		}
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

	// a sigc connection for delete notifications
	SigC::Connection _delete_connection;
	SigC::Connection _changed_connection;

	// a sigc connection for transformed signal, used to do move compensation
	SigC::Connection _transformed_connection;
};

struct SPUseClass {
	SPItemClass parent_class;
};

GType sp_use_get_type (void);

SPItem *sp_use_unlink (SPUse *use);
SPItem *sp_use_get_original (SPUse *use);

#endif
