#ifndef __NR_SVP_RENDER_H__
#define __NR_SVP_RENDER_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-pixblock.h>
#include <libnr/nr-svp.h>

/* Renders graymask of svp into buffer */
void nr_pixblock_render_svp_mask_or (NRPixBlock *d, NRSVP *svp);

/* Renders graymask of svp into buffer */
void nr_pixblock_render_svl_mask_or (NRPixBlock *d, NRSVL *svl);
/* Renders colored SVP into buffer (has to be RGB/RGBA) */
void nr_pixblock_render_svl_rgba (NRPixBlock *d, NRSVL *svl, guint32 rgba);

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
