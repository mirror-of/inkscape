#ifndef __SP_KNOTHOLDER_H__
#define __SP_KNOTHOLDER_H__

/*
 * SPKnotHolder - Hold SPKnot list and manage signals
 *
 * Author:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2001 Mitsuru Oka
 *
 * Released under GNU GPL
 *
 */

#include <glib.h>
#include "knot.h"
#include "forward.h"

typedef void (* SPKnotHolderSetFunc) (SPItem *item, const NRPointF *p, guint state);
typedef void (* SPKnotHolderGetFunc) (SPItem *item, NRPointF *p);
/* fixme: Think how to make callbacks most sensitive (Lauris) */
typedef void (* SPKnotHolderReleasedFunc) (SPItem *item);

typedef struct _SPKnotHolderEntity SPKnotHolderEntity;
typedef struct _SPKnotHolder       SPKnotHolder;


struct _SPKnotHolderEntity {
	SPKnot *knot;
	guint   handler_id;
	void (* knot_set) (SPItem *item, const NRPointF *p, guint state);
	void (* knot_get) (SPItem *item, NRPointF *p);
};

struct _SPKnotHolder {
	SPDesktop *desktop;
	SPItem *item;
	GSList *entity;

	SPKnotHolderReleasedFunc released;
};


/* fixme: As a temporary solution, if released is NULL knotholder flushes undo itself (Lauris) */
SPKnotHolder *sp_knot_holder_new (SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);

void sp_knot_holder_destroy (SPKnotHolder *knots);

void sp_knot_holder_add (SPKnotHolder *knot_holder, SPKnotHolderSetFunc knot_set, SPKnotHolderGetFunc knot_get);
void sp_knot_holder_add_full (SPKnotHolder *knot_holder,
			      SPKnotHolderSetFunc knot_set,
			      SPKnotHolderGetFunc knot_get,
			      SPKnotShapeType shape,
			      SPKnotModeType mode);


#endif
