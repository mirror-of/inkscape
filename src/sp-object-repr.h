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

#include "forward.h"
#include <xml/xml-forward.h>

SPObject *sp_object_repr_build_tree (SPDocument *document, SPRepr *repr);

GType sp_repr_type_lookup (SPRepr *repr);
GType sp_object_type_lookup (const gchar *name);

void sp_object_type_register(gchar const *name, GType type);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
