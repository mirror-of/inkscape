#ifndef __NR_PIXOPS_H__
#define __NR_PIXOPS_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define NR_RGBA32_R(v) (unsigned char) (((v) >> 24) & 0xff)
#define NR_RGBA32_G(v) (unsigned char) (((v) >> 16) & 0xff)
#define NR_RGBA32_B(v) (unsigned char) (((v) >> 8) & 0xff)
#define NR_RGBA32_A(v) (unsigned char) ((v) & 0xff)

#define NR_PREMUL(c,a) (((c) * (a) + 127) / 255)
#define NR_A7(fa,ba) (65025 - (255 - fa) * (255 - ba))
#define NR_COMPOSENNN_A7(fc,fa,bc,ba,a) (((255 - (fa)) * (bc) * (ba) + (fa) * (fc) * 255 + 127) / a)
#define NR_COMPOSEPNN_A7(fc,fa,bc,ba,a) (((255 - (fa)) * (bc) * (ba) + (fc) * 65025 + 127) / a)
#define NR_COMPOSENNP(fc,fa,bc,ba) (((255 - (fa)) * (bc) * (ba) + (fa) * (fc) * 255 + 32512) / 65025)
#define NR_COMPOSEPNP(fc,fa,bc,ba) (((255 - (fa)) * (bc) * (ba) + (fc) * 65025 + 32512) / 65025)
#define NR_COMPOSENPP(fc,fa,bc,ba) (((255 - (fa)) * (bc) + (fa) * (fc) + 127) / 255)
#define NR_COMPOSEPPP(fc,fa,bc,ba) (((255 - (fa)) * (bc) + (fc) * 255 + 127) / 255)
#define NR_COMPOSEP11(fc,fa,bc) (((255 - (fa)) * (bc) + (fc) * 255 + 127) / 255)
#define NR_COMPOSEN11(fc,fa,bc) (((255 - (fa)) * (bc) + (fc) * (fa) + 127) / 255)

#endif
