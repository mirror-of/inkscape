#ifndef __NR_PIXBLOCK_PIXEL_H__
#define __NR_PIXBLOCK_PIXEL_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-pixblock.h>

void nr_compose_pixblock_pixblock_pixel (NRPixBlock *dpb, unsigned char *d, const NRPixBlock *spb, const unsigned char *s);

#endif
