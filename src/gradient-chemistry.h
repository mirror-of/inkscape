#ifndef __SP_GRADIENT_CHEMISTRY_H__
#define __SP_GRADIENT_CHEMISTRY_H__

/*
 * Various utility methods for gradients
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"
#include "sp-gradient.h"

/*
 * Either normalizes given gradient to vector, or returns fresh normalized
 * vector - in latter case, original gradient is flattened and stops cleared
 * No transrefing - i.e. caller shouldn't hold reference to original and
 * does not get one to new automatically (doc holds ref of every object anyways)
 */

SPGradient *sp_gradient_ensure_vector_normalized (SPGradient *gradient);


/*
 * Sets item fill/stroke to lineargradient with given vector, creating
 * new private gradient, if needed
 * gr has to be normalized vector
 */

SPGradient *sp_item_set_gradient (SPItem *item, SPGradient *gr, SPGradientType type, bool is_fill);

/*
 * Get default normalized gradient vector of document, create if there is none
 */

SPGradient *sp_document_default_gradient_vector (SPDocument *document, guint32 color = 0);
SPGradient *sp_gradient_vector_for_object (SPDocument *doc, SPDesktop *desktop, SPObject *o, bool is_fill);

/*
 * Get vector gradient of given gradient
 */

SPGradient *sp_gradient_get_vector (SPGradient *gradient, gboolean force_private);

void sp_object_ensure_fill_gradient_normalized (SPObject *object);
void sp_object_ensure_stroke_gradient_normalized (SPObject *object);

SPGradient *sp_gradient_convert_to_userspace (SPGradient *gr, SPItem *item, const gchar *property);

void sp_gradient_transform_multiply (SPGradient *gradient, NR::Matrix postmul, bool set);

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
