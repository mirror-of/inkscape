#ifndef __SP_MARKER_H__
#define __SP_MARKER_H__

/*
 * SVG <marker> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * This is quite similar in logic to <svg>
 * Maybe we should merge them somehow (Lauris)
 */

#define SP_TYPE_MARKER (sp_marker_get_type ())
#define SP_MARKER(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MARKER, SPMarker))
#define SP_IS_MARKER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MARKER))

class SPMarker;
class SPMarkerClass;
class SPMarkerView;

/**
 * These enums are to allow us to have 4-element arrays that represent
 * a set of marker locations (all, start, mid, and end).  This allows us
 * to iterate through the array in places where we need to do a process
 * across all of the markers, instead of separate code stanzas for each.
 */
enum {
	SP_MARKER_LOC,
	SP_MARKER_LOC_START,
	SP_MARKER_LOC_MID,
	SP_MARKER_LOC_END,
	SP_MARKER_LOC_QTY
};

#include <libnr/nr-matrix.h>
#include <libnr/nr-rect.h>
#include "svg/svg-types.h"
#include "enums.h"
#include "sp-item-group.h"
#include "uri-references.h"

struct SPMarker {
	SPGroup group;

	/* units */
	unsigned int markerUnits_set : 1;
	unsigned int markerUnits : 1;

	/* reference point */
	SPSVGLength refX;
	SPSVGLength refY;

	/* dimensions */
	SPSVGLength markerWidth;
	SPSVGLength markerHeight;

	/* orient */
	unsigned int orient_set : 1;
	unsigned int orient_auto : 1;
	float orient;

	/* viewBox; */
	unsigned int viewBox_set : 1;
	NRRect viewBox;

	/* preserveAspectRatio */
	unsigned int aspect_set : 1;
	unsigned int aspect_align : 4;
	unsigned int aspect_clip : 1;

	/* Child to parent additional transform */
	NRMatrix c2p;

	/* Private views */
	SPMarkerView *views;
};

struct SPMarkerClass {
	SPGroupClass parent_class;
};

GType sp_marker_get_type (void);

class SPMarkerReference : public Inkscape::URIReference {
	SPMarkerReference(SPObject *obj) : URIReference(obj) {}
	SPMarker *getObject() const {
		return (SPMarker *)URIReference::getObject();
	}
protected:
	bool _acceptObject(SPObject *obj) const {
		return SP_IS_MARKER(obj);
	}
};

void sp_marker_show_dimension (SPMarker *marker, unsigned int key, unsigned int size);
NRArenaItem *sp_marker_show_instance (SPMarker *marker, NRArenaItem *parent,
				      unsigned int key, unsigned int pos,
				      NRMatrix *base, float linewidth);
void sp_marker_hide (SPMarker *marker, unsigned int key);

#endif
