#ifndef __SP_OBJECT_REPR_H__
#define __SP_OBJECT_REPR_H__

/*
 * Object type dictionary and build frontend
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <xml/repr.h>
#include "forward.h"

SPObject *sp_object_repr_build_tree (SPDocument *document, SPRepr *repr);

GType sp_repr_type_lookup (SPRepr *repr);
GType sp_object_type_lookup (const unsigned char *name);

/* Return TRUE on success */

GType sp_object_type_register (const unsigned char *name, GType type);

#endif
