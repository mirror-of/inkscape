#define __NR_TYPEFACE_C__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <string.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>
#include "nr-typeface.h"
#include "nr-type-directory.h"

#include "FontInstance.h"

static void nr_typeface_class_init (NRTypeFaceClass *klass);
static void nr_typeface_init (NRTypeFace *tface);
static void nr_typeface_finalize (NRObject *object);

static void nr_typeface_setup (NRTypeFace *tface, NRTypeFaceDef *def);

static NRObjectClass *parent_class;

/**
 * Gets the typeface type, and registers it if it hasn't been registered yet
 */
NRType
nr_typeface_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_OBJECT,
						"NRTypeFace",
						sizeof (NRTypeFaceClass),
						sizeof (NRTypeFace),
						(void (*) (NRObjectClass *)) nr_typeface_class_init,
						(void (*) (NRObject *)) nr_typeface_init);
	}
	return type;
}

/**
 * Initializes the typeface class by setting up all of its 
 * virtual functions.
 */
static void
nr_typeface_class_init (NRTypeFaceClass *klass)
{
	NRObjectClass *object_class;

	object_class = (NRObjectClass *) klass;

	parent_class = ((NRObjectClass *) klass)->parent;

	object_class->finalize = nr_typeface_finalize;

	klass->setup = nr_typeface_setup;

	klass->font_glyph_outline_get = nr_font_generic_glyph_outline_get;
	klass->font_glyph_outline_unref = nr_font_generic_glyph_outline_unref;
	klass->font_glyph_advance_get = nr_font_generic_glyph_advance_get;
	klass->font_glyph_area_get = nr_font_generic_glyph_area_get;

	klass->rasterfont_new = nr_font_generic_rasterfont_new;
	klass->rasterfont_free = nr_font_generic_rasterfont_free;

	klass->rasterfont_glyph_advance_get = nr_rasterfont_generic_glyph_advance_get;
	klass->rasterfont_glyph_area_get = nr_rasterfont_generic_glyph_area_get;
	klass->rasterfont_glyph_mask_render = nr_rasterfont_generic_glyph_mask_render;
}

/**
 * Initializes the typeface object by giving it a null definition and
 * no glyphs
 */
static void
nr_typeface_init (NRTypeFace *tface)
{
	tface->def = NULL;
	tface->nglyphs = 0;
}

/**
 * Does the finalization of the object prior to destroying it
 */
static void
nr_typeface_finalize (NRObject *object)
{
	NRTypeFace *tface;

	tface = (NRTypeFace *) object;

	nr_type_directory_forget_face (tface);

	((NRObjectClass *) (parent_class))->finalize (object);
}

/**
 * Sets the definition object for the typeface to def
 */
static void
nr_typeface_setup (NRTypeFace *tface, NRTypeFaceDef *def)
{
	tface->def = def;
}

/**
 * Creates a new typeface object from the given typeface definition
 */
NRTypeFace *
nr_typeface_new (NRTypeFaceDef *def)
{
	NRTypeFace *tface;

	tface = (NRTypeFace *) nr_object_new (def->type);

	((NRTypeFaceClass *) ((NRObject *) tface)->klass)->setup (tface, def);

	return tface;
}
NRTypeFace* nr_typeface_ref(NRTypeFace* t)
{
  if ( t == NULL ) return NULL;
  ((font_instance*)t)->Ref();
  return t;
}
NRTypeFace* nr_typeface_unref(NRTypeFace* t)
{
  if ( t == NULL ) return NULL;
  ((font_instance*)t)->Unref();
  return NULL;
}

/**
 * Gets the name matching str of the given typeface
 */
unsigned int
nr_typeface_name_get (NRTypeFace *tf, gchar *str, unsigned int size)
{
  if ( tf == NULL ) return 0;
	return ((font_instance*)tf)->Name(str,size);
#if 0
	return nr_typeface_attribute_get (tf, "name", str, size);
#endif
}

/**
 * Gets the familyname matching str for the given typeface
 */
unsigned int
nr_typeface_family_name_get (NRTypeFace *tf, gchar *str, unsigned int size)
{
  if ( tf == NULL ) return 0;
	return ((font_instance*)tf)->Family(str,size);
#if 0
	return nr_typeface_attribute_get (tf, "family", str, size);
#endif
}

/**
 * Gets the value of an attribute named key with value str from the typeface
 */
unsigned int
nr_typeface_attribute_get (NRTypeFace *tf, const gchar *key, gchar *str, unsigned int size)
{
  if ( tf == NULL ) return 0;
	return ((font_instance*)tf)->Attribute(key,str,size);
#if 0
	return ((NRTypeFaceClass *) ((NRObject *) tf)->klass)->attribute_get (tf, key, str, size);
#endif
}

/**
 * Gets the outline for the glyph for the typeface with the given metrics and path
 */
NRBPath *
nr_typeface_glyph_outline_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRBPath *d, unsigned int ref)
{
	return NULL;
#if 0
	return ((NRTypeFaceClass *) ((NRObject *) tf)->klass)->glyph_outline_get (tf, glyph, metrics, d, ref);
#endif
}

/**
 * Dereferences the given glyph and metrics from the typeface
 */
void
nr_typeface_glyph_outline_unref (NRTypeFace *tf, unsigned int glyph, unsigned int metrics)
{
	return;
#if 0
	((NRTypeFaceClass *) ((NRObject *) tf)->klass)->glyph_outline_unref (tf, glyph, metrics);
#endif
}

/**
 * Retrieves the horizontal positional advancement for the glyph in the
 * given font.
 */
NR::Point nr_typeface_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics)
{
  if ( tf == NULL ) return NR::Point(0,0);
	if ( metrics == NR_TYPEFACE_METRICS_VERTICAL ) {
		return NR::Point(0,-(((font_instance*)tf)->Advance(glyph,true)));  // inverse of the advance because origin is in bottomleft corner
	} else {
		return NR::Point(((font_instance*)tf)->Advance(glyph,false),0);
	}
#if 0
	return ((NRTypeFaceClass *) ((NRObject *) tf)->klass)->glyph_advance_get (tf, glyph, metrics);
#endif
}

/**
 * Looks up something for the given typeface and unival
 *
 * Q:  What is unival?
 * Q:  What data is looked up and returned?
 */
unsigned int
nr_typeface_lookup_default (NRTypeFace *tf, unsigned int unival)
{
  if ( tf == NULL ) return 0;
	return ((font_instance*)tf)->MapUnicodeChar(unival);
#if 0
	return ((NRTypeFaceClass *) ((NRObject *) tf)->klass)->lookup (tf, NR_TYPEFACE_LOOKUP_RULE_DEFAULT, unival);
#endif
}

/**
 * Creates a new default font object from the given typeface, metrics, and size
 */
NRFont *
nr_font_new_default (NRTypeFace *tf, unsigned int metrics, float size)
{
  if ( tf == NULL ) return NULL;
	((font_instance*)tf)->Ref();
	return ((NRFont*)tf);
#if 0
	NR::Matrix const scale(NR::scale(size, size));

	return ((NRTypeFaceClass *) ((NRObject *) tf)->klass)->font_new (tf, metrics, scale);
#endif
}

/* NRTypeFaceEmpty */

#define NR_TYPE_TYPEFACE_EMPTY (nr_typeface_empty_get_type ())

/**
 * This structure is the instance of an empty typeface.
 * 
 * Q:  Why is this necessary?
 */
struct NRTypeFaceEmpty {
	NRTypeFace typeface;
};

/**
 * This structure defines a class for empty typefaces.
 *
 * Q:  What is an empty typeface and why is a class for it needed?
 */
struct NRTypeFaceEmptyClass {
	NRTypeFaceClass typeface_class;
};

static NRType nr_typeface_empty_get_type (void);

static void nr_typeface_empty_class_init (NRTypeFaceEmptyClass *klass);
static void nr_typeface_empty_init (NRTypeFaceEmpty *tfe);
static void nr_typeface_empty_finalize (NRObject *object);

static unsigned int nr_typeface_empty_attribute_get (NRTypeFace *tf, const gchar *key, gchar *str, unsigned int size);
static NRBPath *nr_typeface_empty_glyph_outline_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRBPath *d, unsigned int ref);
static void nr_typeface_empty_glyph_outline_unref (NRTypeFace *tf, unsigned int glyph, unsigned int metrics);
static NR::Point nr_typeface_empty_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics);
static unsigned int nr_typeface_empty_lookup (NRTypeFace *tf, unsigned int rule, unsigned int unival);

static NRFont *nr_typeface_empty_font_new (NRTypeFace *tf, unsigned int metrics, NR::Matrix const transform);
static void nr_typeface_empty_font_free (NRFont *font);

static NRTypeFaceClass *empty_parent_class;

/**
 * Gets the NRType for empty typefaces, registering it if necessary
 */
static NRType
nr_typeface_empty_get_type (void)
{
	static NRType type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_TYPEFACE,
						"NRTypeFaceEmpty",
						sizeof (NRTypeFaceEmptyClass),
						sizeof (NRTypeFaceEmpty),
						(void (*) (NRObjectClass *)) nr_typeface_empty_class_init,
						(void (*) (NRObject *)) nr_typeface_empty_init);
	}
	return type;
}

/**
 * Initializes the empty typeface by assigning all of its virtual functions
 * as appropriate.
 */
static void
nr_typeface_empty_class_init (NRTypeFaceEmptyClass *klass)
{
	NRObjectClass *object_class;
	NRTypeFaceClass *tface_class;

	object_class = (NRObjectClass *) klass;
	tface_class = (NRTypeFaceClass *) klass;

	empty_parent_class = (NRTypeFaceClass *) (((NRObjectClass *) klass)->parent);

	object_class->finalize = nr_typeface_empty_finalize;

	tface_class->attribute_get = nr_typeface_empty_attribute_get;
	tface_class->glyph_outline_get = nr_typeface_empty_glyph_outline_get;
	tface_class->glyph_outline_unref = nr_typeface_empty_glyph_outline_unref;
	tface_class->glyph_advance_get = nr_typeface_empty_glyph_advance_get;
	tface_class->lookup = nr_typeface_empty_lookup;

	tface_class->font_new = nr_typeface_empty_font_new;
	tface_class->font_free = nr_typeface_empty_font_free;
}

/**
 * Initializes an empty typeface by setting its nglyphs member to 1.
 */
static void
nr_typeface_empty_init (NRTypeFaceEmpty *tfe)
{
	NRTypeFace *tface;

	tface = (NRTypeFace *) tfe;

	tface->nglyphs = 1;
}

/**
 * Finalizes the object
 *
 * Q:  What does 'finalize' mean in this context?
 */
static void
nr_typeface_empty_finalize (NRObject *object)
{
	NRTypeFace *tface;

	tface = (NRTypeFace *) object;

	((NRObjectClass *) (parent_class))->finalize (object);
}

static NRFont *empty_fonts = NULL;

/**
 * Gets an attribute for an empty typeface
 * 
 * Q:  Why is this function needed?
 */
static unsigned int
nr_typeface_empty_attribute_get (NRTypeFace *tf, const gchar *key, gchar *str, unsigned int size)
{
	const gchar *val;
	int len;

	if (!strcmp (key, "name")) {
		val = tf->def->name;
	} else if (!strcmp (key, "family")) {
		val = tf->def->family;
	} else if (!strcmp (key, "weight")) {
		val = "normal";
	} else if (!strcmp (key, "style")) {
		val = "normal";
	} else {
		val = "";
	}

	len = MIN (size - 1, strlen (val));
	if (len > 0) {
		memcpy (str, val, len);
	}
	if (size > 0) {
		str[len] = '\0';
	}

	return strlen (val);
}

/**
 * Gets the outline path for an empty glyph
 */
static NRBPath *
nr_typeface_empty_glyph_outline_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics, NRBPath *d, unsigned int ref)
{
	static const NArtBpath gol[] = {
		{NR_MOVETO, 0, 0, 0, 0, 100.0, 100.0},
		{NR_LINETO, 0, 0, 0, 0, 100.0, 900.0},
		{NR_LINETO, 0, 0, 0, 0, 900.0, 900.0},
		{NR_LINETO, 0, 0, 0, 0, 900.0, 100.0},
		{NR_LINETO, 0, 0, 0, 0, 100.0, 100.0},
		{NR_MOVETO, 0, 0, 0, 0, 150.0, 150.0},
		{NR_LINETO, 0, 0, 0, 0, 850.0, 150.0},
		{NR_LINETO, 0, 0, 0, 0, 850.0, 850.0},
		{NR_LINETO, 0, 0, 0, 0, 150.0, 850.0},
		{NR_LINETO, 0, 0, 0, 0, 150.0, 150.0},
		{NR_END, 0, 0, 0, 0, 0, 0}
	};

	d->path = (NArtBpath *) gol;

	return d;
}

/**
 * Q:  Why does this function exist?
 */
static void
nr_typeface_empty_glyph_outline_unref (NRTypeFace *tf, unsigned int glyph, unsigned int metrics)
{
}

/**
 * Retrieves the horizontal positional advancement for the glyph in the
 * given font.
 */
static NR::Point nr_typeface_empty_glyph_advance_get (NRTypeFace *tf, unsigned int glyph, unsigned int metrics)
{
	if (metrics == NR_TYPEFACE_METRICS_VERTICAL) {
		return NR::Point(0.0, -1000.0);
	}
	return NR::Point(1000.0, 0.0);
}

/**
 * Q:  Why does this function exist?
 */
static unsigned int
nr_typeface_empty_lookup (NRTypeFace *tf, unsigned int rule, unsigned int unival)
{
	return 0;
}

/**
 * Q:  What does this function do?
 */
static NRFont *
nr_typeface_empty_font_new (NRTypeFace *tf, unsigned int metrics, NR::Matrix const transform)
{
	double size = NR::expansion(transform);

	NRFont *font = empty_fonts;
	while (font != NULL) {
		if (NR_DF_TEST_CLOSE (size, font->size, 0.001 * size) && (font->metrics == metrics)) {
			return nr_font_ref (font);
		}
		font = font->next;
	}
	
	font = nr_font_generic_new (tf, metrics, transform);

	font->next = empty_fonts;
	empty_fonts = font;

	return font;
}

/**
 * Empties and frees the font object
 */
static void
nr_typeface_empty_font_free (NRFont *font)
{
	if (empty_fonts == font) {
		empty_fonts = font->next;
	} else {
		NRFont *ref;
		ref = empty_fonts;
		while (ref->next != font) ref = ref->next;
		ref->next = font->next;
	}

	font->next = NULL;

	nr_font_generic_free (font);
}

/**
 * Fills in the def item with the name and family, and sets the
 * type to indicate it is empty, and the typeface to NULL.
 *
 * Q:  Does this leave dangling pointers?
 */
void
nr_type_empty_build_def (NRTypeFaceDef *def, const gchar *name, const gchar *family)
{
	def->type = NR_TYPE_TYPEFACE_EMPTY;
	def->name = strdup (name);
	def->family = strdup (family);
	def->typeface = NULL;
}

