#ifndef __NR_SVP_UNCROSS_H__
#define __NR_SVP_UNCROSS_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-svp.h>

NRSVL *nr_svl_uncross_full (NRSVL *svp, NRFlat *flats, unsigned int windrule);

#endif
