#ifndef __NR_TYPE_XFT_H__
#define __NR_TYPE_XFT_H__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-macros.h>

#include "nr-type-primitives.h"
#include "nr-type-ft2.h"

void nr_type_xft_typefaces_get (NRNameList *names);
void nr_type_xft_families_get (NRNameList *names);

void nr_type_xft_build_def (NRTypeFaceDefFT2 *dft2, const gchar *name, const gchar *family);

void nr_type_read_xft_list (void);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
