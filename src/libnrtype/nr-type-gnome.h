#ifndef __NR_TYPE_GNOME_H__
#define __NR_TYPE_GNOME_H__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define NR_TYPE_TYPEFACE_GNOME (nr_typeface_gnome_get_type ())
#define NR_TYPEFACE_GNOME(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_TYPEFACE_GNOME, NRTypeFaceGnome))
#define NR_IS_TYPEFACE_GNOME(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_TYPEFACE_GNOME))

typedef struct _NRTypeFaceGnome NRTypeFaceGnome;
typedef struct _NRTypeFaceGnomeClass NRTypeFaceGnomeClass;

#include <libnrtype/nr-rasterfont.h>
#include <libnrtype/nr-type-directory.h>
#include <libgnomeprint/gnome-font.h>

struct _NRTypeFaceGnome {
	NRTypeFace typeface;

	GnomeFontFace *face;
	NRFont *fonts;

	NRBPath *voutlines;
};

struct _NRTypeFaceGnomeClass {
	NRTypeFaceClass typeface_class;
};

NRType nr_typeface_gnome_get_type (void);

NRNameList *nr_type_gnome_typefaces_get (NRNameList *typefaces);
NRNameList *nr_type_gnome_families_get (NRNameList *families);
void nr_type_gnome_build_def (NRTypeFaceDef *def, const unsigned char *name, const unsigned char *family);

void nr_type_read_gnome_list (void);

#endif
