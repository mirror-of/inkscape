#define __SP_STRING_C__

/*
 * SVG <text> and <tspan> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * fixme:
 *
 * These subcomponents should not be items, or alternately
 * we have to invent set of flags to mark, whether standard
 * attributes are applicable to given item (I even like this
 * idea somewhat - Lauris)
 *
 */

#include "config.h"

#include <string.h>

#include <libnrtype/FlowSrc.h>
#include <libnrtype/FlowStyle.h>
#include <libnrtype/FlowSrcText.h>

#include <glib.h>
#include "xml/repr-private.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "attributes.h"
#include "style.h"

#include "sp-string.h"
#include "sp-text.h"
#include "sp-tspan.h"


/*#####################################################
#  SPSTRING
#####################################################*/

static void sp_string_class_init (SPStringClass *classname);
static void sp_string_init (SPString *string);

static void sp_string_build (SPObject *object, SPDocument *document, SPRepr *repr);
static void sp_string_release (SPObject *object);
static void sp_string_read_content (SPObject *object);
static void sp_string_update (SPObject *object, SPCtx *ctx, unsigned int flags);

static void sp_string_calculate_dimensions (SPString *string);

static SPObjectClass *string_parent_class;

GType
sp_string_get_type ()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof (SPStringClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc) sp_string_class_init,
            NULL,    /* class_finalize */
            NULL,    /* class_data */
            sizeof (SPString),
            16,    /* n_preallocs */
            (GInstanceInitFunc) sp_string_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static (SP_TYPE_OBJECT, "SPString", &info, (GTypeFlags)0);
    }
    return type;
}
static void
sp_string_class_init (SPStringClass *classname)
{
    SPObjectClass *sp_object_class;
    SPItemClass   *item_class;

    sp_object_class = (SPObjectClass *) classname;
    item_class      = (SPItemClass *) classname;

    string_parent_class = (SPObjectClass*)g_type_class_ref (SP_TYPE_OBJECT);

    sp_object_class->build        = sp_string_build;
    sp_object_class->release      = sp_string_release;
    sp_object_class->read_content = sp_string_read_content;
    sp_object_class->update       = sp_string_update;
}

static void
sp_string_init (SPString *string)
{
	new (&string->contents) text_flow_src(SP_OBJECT(string));
	new (&string->svg_contents) partial_text();
}
static void
sp_string_build (SPObject *object, SPDocument *doc, SPRepr *repr)
{
//    SPString *string = SP_STRING(object);
	sp_string_read_content (object);
//    SPObject *parent=SP_OBJECT_PARENT(object);

	if (((SPObjectClass *) string_parent_class)->build)
		((SPObjectClass *) string_parent_class)->build (object, doc, repr);
}

static void
sp_string_release (SPObject *object)
{
	SPString *string = SP_STRING (object);
	
	string->contents.~text_flow_src();
	string->svg_contents.~partial_text();
	
	if (((SPObjectClass *) string_parent_class)->release)
		((SPObjectClass *) string_parent_class)->release (object);
}

static void
sp_string_read_content (SPObject *object)
{
	SPString *string = SP_STRING (object);

	const gchar *t = sp_repr_content (object->repr);
	string->svg_contents.ResetText();
	if ( t ) {
		string->svg_contents.AddSVGInputText((char*)t,-1);
	} else {
		string->svg_contents.AddSVGInputText((char*)"",0);
	}
	string->contents.SetStringText(&string->svg_contents);
	object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}
static void
sp_string_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	if (((SPObjectClass *) string_parent_class)->update)
		((SPObjectClass *) string_parent_class)->update (object, ctx, flags);
	
	if (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_MODIFIED_FLAG)) {
		/* Parent style or we ourselves changed, so recalculate */
		flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B; // won't be "just a transformation" anymore, we're going to recompute "x" and "y" attributes
		sp_string_calculate_dimensions (SP_STRING (object));
	}
}
NR::Point
sp_letterspacing_advance (const SPStyle *style)
{
	NR::Point letterspacing_adv;
	if (style->text->letterspacing.value != 0 && style->text->letterspacing.computed == 0) { // set in em or ex
		if (style->text->letterspacing.unit == SP_CSS_UNIT_EM) {
			if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
				letterspacing_adv = NR::Point(0.0, style->font_size.computed * style->text->letterspacing.value);
			} else {
				letterspacing_adv = NR::Point(style->font_size.computed * style->text->letterspacing.value, 0.0);
			}
		} else if (style->text->letterspacing.unit == SP_CSS_UNIT_EX) {
			// I did not invent this 0.5 multiplier; it's what lauris uses in style.cpp
			// Someone knowledgeable must find out how to extract the real em and ex values from the font!
			if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
				letterspacing_adv = NR::Point(0.0, style->font_size.computed * style->text->letterspacing.value * 0.5);
			} else {
				letterspacing_adv = NR::Point(style->font_size.computed * style->text->letterspacing.value * 0.5, 0.0);
			}
		} else { // unknown unit - should not happen
			letterspacing_adv = NR::Point(0.0, 0.0);
		}
	} else { // there's a real value in .computed, or it's zero
		if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
			letterspacing_adv = NR::Point(0.0, style->text->letterspacing.computed);
		} else {
			letterspacing_adv = NR::Point(style->text->letterspacing.computed, 0.0);
		}
	} 
	return letterspacing_adv;
}


static void
sp_string_calculate_dimensions (SPString */*string*/)
{
}

char*         
sp_string_get_text(SPString* string)
{
   if ( string->contents.cleaned_up.utf8_text )
     return string->contents.cleaned_up.utf8_text;
   return string->svg_contents.utf8_text;
}






