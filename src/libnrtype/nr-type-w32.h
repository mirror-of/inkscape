#ifndef __NR_TYPE_W32_H__
#define __NR_TYPE_W32_H__

/*
 * Wrapper around Win32 font subsystem
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define NR_TYPE_TYPEFACE_W32 (nr_typeface_w32_get_type ())
#define NR_TYPEFACE_W322(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_TYPEFACE_W32, NRTypeFaceW32))
#define NR_IS_TYPEFACE_W32(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_TYPEFACE_W32))

struct NRTypeFaceW32;
struct NRTypeFaceW32Class;

struct NRTypeFaceGlyphW32;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <libnrtype/nr-type-primitives.h>
#include <libnrtype/nr-typeface.h>

enum {
    NR_TYPE_W32_CP_1250,
    NR_TYPE_W32_CP_1251,
    NR_TYPE_W32_CP_1252,
    NR_TYPE_W32_CP_1253,
    NR_TYPE_W32_CP_1254,
    NR_TYPE_W32_CP_1255,
    NR_TYPE_W32_CP_1256,
    NR_TYPE_W32_CP_1257,
    NR_TYPE_W32_CP_1258,
    NR_TYPE_W32_CP_874,
    NR_TYPE_W32_CP_932,
    NR_TYPE_W32_CP_936,
    NR_TYPE_W32_CP_949,
    NR_TYPE_W32_CP_950,
    NR_TYPE_W32_CP_LAST
};

struct NRTypeFaceGlyphW32 {
	NRRect area;
	NR::Point advance;
	int olref;
	NRBPath outline;
};

struct NRTypeFaceW32 : public NRTypeFace {
	NRFont *fonts;

	/* Glyph slots */
	int *hgidx;
	int *vgidx;
	NRTypeFaceGlyphW32 *slots;
	unsigned int slots_length;
	unsigned int slots_size;

	LOGFONT *logfont;
	HFONT hfont;
	LPOUTLINETEXTMETRIC otm;
};

struct NRTypeFaceW32Class {
	NRTypeFaceClass typeface_class;
};

void nr_type_w32_typefaces_get (NRNameList *names);
void nr_type_w32_families_get (NRNameList *names);

void nr_type_w32_build_def (NRTypeFaceDef *def, const unsigned char *name, const unsigned char *family);

void nr_type_read_w32_list (void);

#endif

