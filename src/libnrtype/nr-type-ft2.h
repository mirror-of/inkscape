#ifndef __NR_TYPE_FT2_H__
#define __NR_TYPE_FT2_H__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define NR_TYPE_TYPEFACE_FT2 (nr_typeface_ft2_get_type ())
#define NR_TYPEFACE_FT2(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_TYPEFACE_FT2, NRTypeFaceFT2))
#define NR_IS_TYPEFACE_FT2(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_TYPEFACE_FT2))

struct NRTypeFaceFT2;
struct NRTypeFaceFT2Class;

struct NRTypeFaceDefFT2;
struct NRTypeFaceGlyphFT2;

#include <ft2build.h>
#include FT_FREETYPE_H
#include <libnrtype/nr-typeface.h>
#include <libnr/nr-point.h>
#include <libnr/nr-rect.h>

struct NRTypeFaceDefFT2 : public NRTypeFaceDef {
	unsigned int is_file : 1;
	union {
		gchar *file;
		const gchar *data;
	} data;
	unsigned int size;
	unsigned int face;
};

struct NRTypeFaceGlyphFT2 {
	NRRect area;
	NR::Point advance;
	int olref;
	NRBPath outline;
};

struct NRTypeFaceFT2 : public NRTypeFace {
	FT_Face ft_face;
	double ft2ps;
	unsigned int unimap : 1;
	unsigned int freelo : 1;

	NRFont *fonts;

	int *hgidx;
	int *vgidx;

	NRTypeFaceGlyphFT2 *slots;
	unsigned int slots_length;
	unsigned int slots_size;
};

struct NRTypeFaceFT2Class {
	NRTypeFaceClass typeface_class;
};

void
nr_type_ft2_build_def (NRTypeFaceDefFT2 *dft2,
		       const gchar *name,
		       const gchar *family,
		       const gchar *file,
		       unsigned int face);

void
nr_type_ft2_build_def_data (NRTypeFaceDefFT2 *dft2,
			    const gchar *name,
			    const gchar *family,
			    const gchar *data,
			    unsigned int size,
			    unsigned int face);
#endif
