#define __SP_TEXT_C__

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

#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-rotate.h>
#include <libnrtype/nr-type-directory.h>
#include <libnrtype/nr-font.h>
#include <libnrtype/font-style-to-pos.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "helper/sp-intl.h"
#include "xml/repr-private.h"
#include "svg/svg.h"
#include "display/nr-arena-group.h"
#include "display/nr-arena-glyphs.h"
#include "attributes.h"
#include "document.h"
#include "desktop.h"
#include "style.h"
#include "version.h"
#include "inkscape.h"
#include "view.h"
#include "print.h"

#include "sp-text.h"



/*#####################################################
#  UTILITY
#####################################################*/
//FIXME: find a better place for these



/**
 *
 */
static void
sp_text_update_length (SPSVGLength *length, gdouble em, gdouble ex, gdouble scale)
{
    if (length->unit == SP_SVG_UNIT_EM)
        length->computed = length->value * em;
    else if (length->unit == SP_SVG_UNIT_EX)
        length->computed = length->value * ex;
    else if (length->unit == SP_SVG_UNIT_PERCENT)
        length->computed = length->value * scale;
}



/**
\brief  Writes a space-separated list of lengths into the key attribute of repr
\param repr   element node repr
\param key    attribute name
\param l        GList where each member has an SPSVGLength as data
 */
unsigned int
sp_repr_set_length_list (SPRepr *repr, const gchar *key, GList *l)
{
    g_return_val_if_fail (repr != NULL, FALSE);
    g_return_val_if_fail (key  != NULL, FALSE);

    gchar c[32];
    gchar *s = NULL;

    for (GList *i = l; i != NULL; i = i->next) {
        if (i->data) {
            g_ascii_formatd (c, sizeof (c), "%.8g", ((SPSVGLength *) i->data)->computed);
            if (i == l) {
                s = g_strdup (c);
            }  else {
                s = g_strjoin (" ", s, c, NULL);
            }
        }
    }
    return sp_repr_set_attr (repr, key, s);
}

bool
sp_length_list_notallzeroes (GList *l)
{
	for (GList *i = l; i != NULL; i = i->next) {
		if (((SPSVGLength *) i->data)->computed != 0.0) {
			return true;
		}
	}
	return false;
}




/*#####################################################
#  DX / DY    MANIPULATION
#####################################################*/

/**
 *
 */
bool
sp_count_chars_recursive (SPObject *o, SPObject *target, unsigned *total)
{
    if ( o == target ) {
        return false; // INT_MAX is a signal to stop searching the tree
    } else if (SP_IS_STRING(o)) {
	*total += SP_STRING(o)->length;
    } else {
        for ( SPObject *child = sp_object_first_child(o) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            if (!sp_count_chars_recursive(child, target, total)) {
                return false;
	    }
        }
    }
    return true;
}




/**
\brief  Recursively counts chars in the object o, stops when runs into target
*/
guint 
sp_count_chars (SPObject *o, SPObject *target)
{
    unsigned total=0;
    sp_count_chars_recursive(o, target, &total);
    return total;
}


// TODO: FIXME: it's even more cumbersome than that. According to the spec, we must
// combine the lists of an object with those of its ancestors if the object's list is
// shorter than the string. So I'll be able to return the object's ly.dx/ly.dy only when
// its length is the same or longer, otherwise a new list will have to be constructed
// and the ancestors traversed upwards, picking list bits from each one with
// corresponding shifts, until the list has sufficient length (or until there are no
// more ancestors).

/**
\brief   Returns the dx list that is effective for the object o, or NULL if
none. According to the spec, the closest ancestor's value has precedence. 
Writes the offset of o relative to its parent that holds the effective dx into
offset. 
*/
GList *
sp_effective_dx (SPObject *o, guint *offset)
{
    GList *l       = NULL;
    SPObject *orig = o;
    *offset        = 0;

    while (SP_IS_STRING(o) || SP_IS_TSPAN(o) || SP_IS_TEXT(o)) {
        if (SP_IS_TSPAN(o) && SP_TSPAN (o)->ly.dx != NULL) {
            l = SP_TSPAN (o)->ly.dx;
            *offset = sp_count_chars (o, orig);
            break;
        }
        if (SP_IS_TEXT(o) && SP_TEXT (o)->ly.dx != NULL) {
            l = SP_TEXT (o)->ly.dx;
            *offset = sp_count_chars (o, orig);
            break;
        }
        o = SP_OBJECT_PARENT(o);
    }
    return l;
}

/**
\brief  Returns the dx shift from the list for character at pos
*/
float
sp_char_dx (GList *dx, guint pos)
{
    if (!dx)
        return 0;
    if (g_list_length(dx) < pos + 1) // pos starts from 0
        return 0;
    return ((SPSVGLength *) g_list_nth(dx, pos)->data)->computed;
}




/**
\brief   Returns the dy list that is effective for the object o, or NULL if
none. According to the spec, the closest ancestor's value has precedence. 
Writes the offset of o relative to its parent that holds the effective dy into
offset. 
*/
GList *
sp_effective_dy (SPObject *o, guint *offset)
{
    GList *l       = NULL;
    SPObject *orig = o;
    *offset        = 0;

    while (SP_IS_STRING(o) || SP_IS_TSPAN(o) || SP_IS_TEXT(o)) {
        if (SP_IS_TSPAN(o) && SP_TSPAN (o)->ly.dy != NULL) {
            l = SP_TSPAN (o)->ly.dy;
            *offset = sp_count_chars (o, orig);
            break;
        }
        if (SP_IS_TEXT(o) && SP_TEXT (o)->ly.dy != NULL) {
            l = SP_TEXT (o)->ly.dy;
            *offset = sp_count_chars (o, orig);
            break;
        }
        o = SP_OBJECT_PARENT(o);
    }
    return l;
}





/**
\brief  Returns the dy shift from the list for character at pos
*/
float
sp_char_dy (GList *dy, guint pos)
{
    if (!dy)
        return 0;
    if (g_list_length(dy) < pos + 1) // pos starts from 0
        return 0;
    return ((SPSVGLength *) g_list_nth(dy, pos)->data)->computed;
}




/**
\brief  Sets the given dx and dy lists for the SPObject o and writes them to its repr
*/
void
sp_set_dxdy (SPObject *o, GList *dx, GList *dy)
{
    if (SP_IS_STRING(o))
        o = SP_OBJECT_PARENT (o);

    if (SP_IS_TEXT(o)) {
        SP_TEXT(o)->ly.dx = dx;
        SP_TEXT(o)->ly.dy = dy;
    }

    if (SP_IS_TSPAN(o)) {
        SP_TSPAN(o)->ly.dx = dx;
        SP_TSPAN(o)->ly.dy = dy;
    }

    if (dy && sp_length_list_notallzeroes (dy)) {
		sp_repr_set_length_list (SP_OBJECT_REPR(o), "dy", dy);
    } else {
		sp_repr_set_attr (SP_OBJECT_REPR(o), "dy", NULL);
    }

    if (dx && sp_length_list_notallzeroes (dx)) {
		sp_repr_set_length_list (SP_OBJECT_REPR(o), "dx", dx);
    } else {
		sp_repr_set_attr (SP_OBJECT_REPR(o), "dx", NULL);
    }

    o->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}




/**
\brief   Ensures the dx and dy lists of child are exactly end positions long, either
appending 0 values or cutting extra values from the lists
*/
void
sp_fill_dxdy (SPObject *child, guint end)
{
    guint dx_offset, dy_offset;
    GList *dx = sp_effective_dx (child, &dx_offset);
    GList *dy = sp_effective_dy (child, &dy_offset);

    // fill in missing...
    GList *dxnew = g_list_copy (g_list_nth (dx, dx_offset));
    for (guint i = g_list_length(dxnew); i < end; i++) {
        SPSVGLength *length = g_new (SPSVGLength, 1);
        length->value = 0;
        length->computed = 0;
        dxnew = g_list_append (dxnew, length);
    }

    // or delete extra
    while (g_list_length(dxnew) > end) {
        if (g_list_length(dxnew) > 1) {
            dxnew = g_list_remove (dxnew, g_list_last(dxnew)->data);
        } else {
            dxnew = NULL;
            break;
        }
    }

    // fill in missing...
    GList *dynew = g_list_copy (g_list_nth (dy, dy_offset));
    for (guint i = g_list_length(dynew); i < end; i++) {
        SPSVGLength *length = g_new (SPSVGLength, 1);
        length->value = 0;
        length->computed = 0;
        dynew = g_list_append (dynew, length);
    }

    // or delete extra
    while (g_list_length(dynew) > end) {
        if (g_list_length(dynew) > 1)
            dynew = g_list_remove (dynew, g_list_last(dynew)->data);
        else {
            dynew = NULL;
            break;
        }
    }

    sp_set_dxdy (child, dxnew, dynew);
}




/**
\brief   Concatenates dx and dy lists of the two objects, assigns the resulting lists to
the first object
*/
void
sp_append_dxdy (SPObject *child1, SPObject *child2)
{
    guint dx_offset1, dy_offset1;
    guint dx_offset2, dy_offset2;
    GList *dxnew1, *dx1 = sp_effective_dx (child1, &dx_offset1);
    GList *dxnew2, *dx2 = sp_effective_dx (child2, &dx_offset2);
    GList *dynew1, *dy1 = sp_effective_dy (child1, &dy_offset1);
    GList *dynew2, *dy2 = sp_effective_dy (child2, &dy_offset2);

    dxnew1 = g_list_copy (g_list_nth (dx1, dx_offset1));
    dxnew2 = g_list_copy (g_list_nth (dx2, dx_offset2));
    dxnew1 = g_list_concat (dxnew1, dxnew2);

    dynew1 = g_list_copy (g_list_nth (dy1, dy_offset1));
    dynew2 = g_list_copy (g_list_nth (dy2, dy_offset2));
    dynew1 = g_list_concat (dynew1, dynew2);

    sp_set_dxdy (child1, dxnew1, dynew1);
}




/**
\brief   Deletes from start to end elements from the object's dx and dy lists
*/
void
sp_delete_dxdy (SPObject *child, gint start, gint end)
{
    guint dx_offset, dy_offset;
    GList *dxnew, *dx = sp_effective_dx (child, &dx_offset);
    GList *dynew, *dy = sp_effective_dy (child, &dy_offset);
    gpointer d;
    gint i;

    if (dx && g_list_length(dx) > dx_offset + start) {
        dxnew = g_list_copy (g_list_nth (dx, dx_offset));
        for (i = start; i < end; i ++) {
            d = g_list_nth_data(dxnew, i);
            if (d)
                dxnew = g_list_remove (dxnew, d);
        }
    } else {
       dxnew = dx;
    }

    if (dy && g_list_length(dy) > dy_offset + start) {
        dynew = g_list_copy (g_list_nth (dy, dy_offset));
        for (i = start; i < end; i ++) {
            d = g_list_nth_data(dynew, i);
            if (d)
                dynew = g_list_remove (dynew, d);
        }
    } else {
        dynew = dy;
    }

    sp_set_dxdy (child, dxnew, dynew);
}



/**
 *
 */
void
sp_insert_dxdy (SPObject *child, guint pos, float dx_computed, float dy_computed, bool ins = true)
{
    guint dx_offset, dy_offset;
    GList *dxnew, *dx = sp_effective_dx (child, &dx_offset);
    GList *dynew, *dy = sp_effective_dy (child, &dy_offset);

    dxnew = g_list_copy (g_list_nth (dx, dx_offset));
    dynew = g_list_copy (g_list_nth (dy, dy_offset));

    if (g_list_length (dxnew) < pos) {
        if (dx_computed != 0) {
            for (guint i = g_list_length(dxnew); i <= pos; i++) {

                SPSVGLength *length = g_new (SPSVGLength, 1);
                length->unit = SP_SVG_UNIT_PX;
                if (i == pos)    
                    length->value = length->computed = dx_computed;
                else 
                    length->value = length->computed = 0;

                dxnew = g_list_append (dxnew, length);
            }
        }
    } else {

        SPSVGLength *length = g_new (SPSVGLength, 1);
        length->unit = SP_SVG_UNIT_PX;
        length->value = length->computed = dx_computed;

        if (!ins) {
            dxnew = g_list_remove (dxnew, g_list_nth_data (dxnew, pos));
        }
        dxnew = g_list_insert (dxnew, (gpointer) length, pos);
    }

    if (g_list_length (dynew) < pos) {
        if (dy_computed != 0) {
            for (guint i = g_list_length(dynew); i <= pos; i++) {

                SPSVGLength *length = g_new (SPSVGLength, 1);
                length->unit = SP_SVG_UNIT_PX;
                if (i == pos)
                    length->value = length->computed = dy_computed;
                else 
                    length->value = length->computed = 0;

                dynew = g_list_append (dynew, length);
            }
        }
    } else {

        SPSVGLength *length = g_new (SPSVGLength, 1);
        length->unit = SP_SVG_UNIT_PX;
        length->value = length->computed = dy_computed;

        if (!ins) {
            dynew = g_list_remove (dynew, g_list_nth_data (dynew, pos));
        }
        dynew = g_list_insert (dynew, (gpointer) length, pos);
    }

    sp_set_dxdy (child, dxnew, dynew);
}


/**
 *
 */
void
sp_insert_multiple_dxdy (SPObject *child, guint pos, guint len, float dx_computed, float dy_computed)
{
    for (guint i = 0; i < len; i++)
        sp_insert_dxdy (child, pos + i, dx_computed, dy_computed);
}



/**
\brief  Split the dx/dy of child1 at pos, assign the remainder to child2 (assuming it had none of its own)
*/
void
sp_split_dxdy (SPObject *child1, SPObject *child2, guint pos)
{
    guint dx_offset, dy_offset;
    GList *dxnew, *dx = sp_effective_dx (child1, &dx_offset);
    GList *dynew, *dy = sp_effective_dy (child1, &dy_offset);

    dxnew = g_list_copy (g_list_nth (dx, dx_offset + pos));
    dynew = g_list_copy (g_list_nth (dy, dy_offset + pos));

    sp_set_dxdy (child2, dxnew, dynew);

    sp_fill_dxdy (child1, pos); 
}



/**
\brief  There's no limited copy of lists in glib. I had to write it myself.
*/
GList *
sp_list_copy_n (GList *src, guint len)
{
    GList *l = NULL;
    while (src && len) {
        l = g_list_append (l, src->data);
        src = src->next;
        len --;
    }
    return l;
}



/**
\brief   Writes dx and dy lists from a text object to its tspan children, with appropriate
offsets. Inkscape-created text always stores dx and dy in tspans, so this is only needed
for foreign SVG. Text display works correctly no matter where dx and dy are stored, so
this function is only called when you attempt to edit text, because editing functions
assume for simplicity that each string has its dx and dy in its immediate parent (tspan
or text). If a text object has only tspan children (no child text nodes) and therefore
all of its dx/dy information is distributed into tspans, it is deleted from the text
object.
*/
void
sp_distribute_dxdy (SPText *text)
{
    if (text->ly.dx == NULL && text->ly.dy == NULL) return;

    bool tspans_only = true;
    SPObject *child;
    GList *dxnew, *dynew, *dx, *dy;
    guint dx_offset, dy_offset;

    for (child = sp_object_first_child(SP_OBJECT(text)) ; child; child = SP_OBJECT_NEXT(child) ) {

        if (!SP_IS_TSPAN(child)) {
            tspans_only = false;
            continue;
        }

        if (SP_TSPAN(child)->ly.dx == NULL) {
            dx = sp_effective_dx (child, &dx_offset);
            dxnew = sp_list_copy_n (g_list_nth (dx, dx_offset), SP_TEXT_CHILD_STRING(child)->length);
        } else 
            dxnew = SP_TSPAN(child)->ly.dx;

        if (SP_TSPAN(child)->ly.dy == NULL) {
            dy = sp_effective_dy (child, &dy_offset);
            dynew = sp_list_copy_n (g_list_nth (dy, dy_offset), SP_TEXT_CHILD_STRING(child)->length);
        } else 
            dynew = SP_TSPAN(child)->ly.dy;

        sp_set_dxdy (child, dxnew, dynew);
    }

    if (tspans_only) {
        sp_set_dxdy (SP_OBJECT(text), NULL, NULL);
    }
}






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
static void sp_string_set_shape (SPString *string, SPLayoutData *ly, NR::Point &cp, gboolean *inspace);

static SPCharsClass *string_parent_class;

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
        type = g_type_register_static (SP_TYPE_CHARS, "SPString", &info, (GTypeFlags)0);
    }
    return type;
}



/**
 *
 */
static void
sp_string_class_init (SPStringClass *classname)
{
    SPObjectClass *sp_object_class;
    SPItemClass   *item_class;

    sp_object_class = (SPObjectClass *) classname;
    item_class      = (SPItemClass *) classname;

    string_parent_class = (SPCharsClass*)g_type_class_ref (SP_TYPE_CHARS);

    sp_object_class->build        = sp_string_build;
    sp_object_class->release      = sp_string_release;
    sp_object_class->read_content = sp_string_read_content;
    sp_object_class->update       = sp_string_update;
}



/**
 *
 */
static void
sp_string_init (SPString *string)
{
    string->text = NULL;
    string->p = NULL;
    string->start = 0;
    string->length = 0;
    string->bbox.x0 = string->bbox.y0 = 0.0;
    string->bbox.x1 = string->bbox.y1 = 0.0;
    string->advance = NR::Point(0, 0);
}



/**
 *
 */
static void
sp_string_build (SPObject *object, SPDocument *doc, SPRepr *repr)
{
    SPString *string = SP_STRING(object);

    sp_string_read_content (object);

    SPObject *parent=SP_OBJECT_PARENT(object);

    if (SP_IS_TEXT(parent)) {
        string->ly = &SP_TEXT(parent)->ly;
    } else if (SP_IS_TSPAN(parent)) {
	string->ly = &SP_TSPAN(parent)->ly;
    } else {
        string->ly = NULL;
    }

    if (((SPObjectClass *) string_parent_class)->build)
        ((SPObjectClass *) string_parent_class)->build (object, doc, repr);

    /* fixme: This can be waste here, but ensures loaded documents are up-to-date */
    sp_string_calculate_dimensions (SP_STRING (object));
}




/**
 *
 */
static void
sp_string_release (SPObject *object)
{
    SPString *string = SP_STRING (object);

    g_free (string->p);
    g_free (string->text);

    if (((SPObjectClass *) string_parent_class)->release)
        ((SPObjectClass *) string_parent_class)->release (object);
}




/**
 * fixme: We have to notify parents that we changed
 */
static void
sp_string_read_content (SPObject *object)
{
    SPString *string = SP_STRING (object);

    g_free (string->p);
    string->p = NULL;
    g_free (string->text);
    const gchar *t = sp_repr_content (object->repr);
    string->text = (t) ? g_strdup (t) : NULL;
    string->length = (t) ? g_utf8_strlen(t, -1) : 0;

    /* Is this correct? I think so (Lauris) */
    /* Virtual method will be invoked BEFORE signal, so we can update there */
    object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}




/**
 * This happen before parent does layouting but after styles have been set 
 * So it is the right place to calculate untransformed string dimensions
 */
static void
sp_string_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
    if (((SPObjectClass *) string_parent_class)->update)
        ((SPObjectClass *) string_parent_class)->update (object, ctx, flags);

    if (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_MODIFIED_FLAG)) {
        /* Parent style or we ourselves changed, so recalculate */
      flags&=~SP_OBJECT_USER_MODIFIED_FLAG_B; // won't be "just a transformation" anymore, we're going to recompute "x" and "y" attributes
        sp_string_calculate_dimensions (SP_STRING (object));
    }
}




/**
\brief Provides the extra letterspacing advance to be added for each char using the given
style. Returns a NR::Point where one of the components is zero. Calculates em and ex
values based on the font size stored in the style.
*/
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





/**
 *
 */
static void
sp_string_calculate_dimensions (SPString *string)
{
    string->bbox.x0 = string->bbox.y0 = 1e18;
    string->bbox.x1 = string->bbox.y1 = -1e18;
    string->advance = NR::Point(0, 0);

    const SPStyle *style = SP_OBJECT_STYLE (SP_OBJECT_PARENT (string));

    if (!style) {
        return;
    }

    guint dx_offset, dy_offset;
    GList *dx = sp_effective_dx (SP_OBJECT(string), &dx_offset);
    GList *dy = sp_effective_dy (SP_OBJECT(string), &dy_offset);

    /* fixme: Adjusted value (Lauris) */
    const gdouble size = style->font_size.computed;
    NRTypeFace *face = nr_type_directory_lookup_fuzzy(style->text->font_family.value,
                          font_style_to_pos(*style));
    
    unsigned int metrics;
    if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
        metrics = NR_TYPEFACE_METRICS_VERTICAL;
    } else {
        metrics = NR_TYPEFACE_METRICS_HORIZONTAL;
    }
    NRFont *font = nr_font_new_default (face, metrics, size);

    // calculating letterspacing advance
    NR::Point letterspacing_adv = sp_letterspacing_advance (style);

    NR::Point spadv;
    if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
        spadv = NR::Point(0.0, size);
    } else {
        spadv = NR::Point(size, 0.0);
    }
    gint spglyph = nr_typeface_lookup_default (face, ' ');
    spadv = nr_font_glyph_advance_get (font, spglyph) + letterspacing_adv;

    if (string->text) {
        const gchar *p;
        gboolean preserve, inspace, intext;

        preserve = (((SPObject*)string)->xml_space.value == SP_XML_SPACE_PRESERVE);
        inspace = FALSE;
        intext = FALSE;

        guint pos = 0;

        for (p = string->text; p && *p; p = g_utf8_next_char (p), pos++) {
            gunichar unival;
            
            unival = g_utf8_get_char (p);

            if (g_unichar_isspace (unival) && (unival != g_utf8_get_char ("\302\240"))) { // space but not non-break space
                if (preserve) {
                    string->advance += spadv + NR::Point(sp_char_dx (dx, dx_offset + pos), sp_char_dy (dy, dy_offset + pos));
                }
                if (unival != '\n' && unival != '\r') inspace = TRUE;
            } else {
                NRRect bbox;
                NR::Point adv;
                gint glyph;

                glyph = nr_typeface_lookup_default (face, unival);

                if (!preserve && inspace && intext) {
                    string->advance += spadv;
                }

                if (nr_font_glyph_area_get (font, glyph, &bbox)) {
                    string->bbox.x0 = MIN (string->bbox.x0, string->advance[NR::X] + bbox.x0);
                    string->bbox.y0 = MIN (string->bbox.y0, string->advance[NR::Y] - bbox.y1);
                    string->bbox.x1 = MAX (string->bbox.x1, string->advance[NR::X] + bbox.x1);
                    string->bbox.y1 = MAX (string->bbox.y1, string->advance[NR::Y] - bbox.y0);
                }
                adv = nr_font_glyph_advance_get (font, glyph ) + letterspacing_adv;

                adv += NR::Point(sp_char_dx (dx, dx_offset + pos), sp_char_dy (dy, dy_offset + pos));

                string->advance += adv;
                
                inspace = FALSE;
                intext = TRUE;
            }
        }
    }

    //nr_font_unref (font);
    //nr_typeface_unref (face);

    if (nr_rect_d_test_empty (&string->bbox)) {
        string->bbox.x0 = string->bbox.y0 = 0.0;
        string->bbox.x1 = string->bbox.y1 = 0.0;
    }

}




/**
 * fixme: Should values be parsed by parent?
 */
static void
sp_string_set_shape (SPString *string, SPLayoutData *ly, NR::Point &cp, gboolean *pinspace)
{

    SPChars *chars = SP_CHARS (string);
    const SPStyle *style = SP_OBJECT_STYLE (SP_OBJECT_PARENT (string));

    guint dx_offset, dy_offset;
    GList *dx = sp_effective_dx (SP_OBJECT(string), &dx_offset);
    GList *dy = sp_effective_dy (SP_OBJECT(string), &dy_offset);

    sp_chars_clear (chars);

    if (!string->text || !*string->text) return;
    const gint len = g_utf8_strlen (string->text, -1);
    if (!len) return;
    g_free (string->p);
    string->p = g_new (NR::Point, len + 1);

    /* fixme: Adjusted value (Lauris) */
    const gdouble size = style->font_size.computed;
    NRTypeFace *face = nr_type_directory_lookup_fuzzy(style->text->font_family.value,
                                font_style_to_pos(*style));
    
    unsigned int metrics;
    if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
        metrics = NR_TYPEFACE_METRICS_VERTICAL;
    } else {
        metrics = NR_TYPEFACE_METRICS_HORIZONTAL;
    }
    NRFont *font = nr_font_new_default (face, metrics, size);

    // calculating letterspacing advance
    NR::Point letterspacing_adv = sp_letterspacing_advance (style);

    NR::Point spadv;
    gint spglyph = nr_typeface_lookup_default (face, ' ');
    spadv = nr_font_glyph_advance_get (font, spglyph) + letterspacing_adv;

    /* fixme: Find a way how to manipulate these */
    NR::Point pt = cp;

    /* fixme: SPChars should do this upright instead */
    NR::scale const flip_y(1.0, -1.0);

    gboolean intext = FALSE;
    gboolean preserve = (((SPObject*)string)->xml_space.value == SP_XML_SPACE_PRESERVE);
    gboolean inspace = pinspace ? *pinspace : FALSE;
    guint pos = 0;
    for (gchar const* cur_char = string->text; cur_char && *cur_char; cur_char = g_utf8_next_char (cur_char), pos++) {
        gunichar unival;
             if (!preserve && inspace && intext) {
                       /* SP_XML_SPACE_DEFAULT */
            //string->p[pos] = pt + NR::Point(spadv[NR::X] + sp_char_dx (dx, dx_offset + pos), -spadv[NR::Y] + sp_char_dy (dy, dy_offset + pos));
            string->p[pos] = pt + NR::Point(spadv[NR::X], -spadv[NR::Y]);
        } else {
            string->p[pos] = pt;// + NR::Point(sp_char_dx (dx, dx_offset + pos), sp_char_dy (dy, dy_offset + pos));
        }
        unival = g_utf8_get_char (cur_char);
             if (g_unichar_isspace(unival) && (unival != g_utf8_get_char ("\302\240"))) { // space but not non-break space
                       if (preserve) {
                    pt += NR::Point(spadv[NR::X] + sp_char_dx (dx, dx_offset + pos), -spadv[NR::Y] + sp_char_dy (dy, dy_offset + pos));
                       }
                       if (unival != '\n' && unival != '\r') inspace = TRUE;
        } else {
            gint glyph = nr_typeface_lookup_default (face, unival);

            if (!preserve && inspace && intext) {
                pt = pt + NR::Point(spadv[NR::X], -spadv[NR::Y]);
            }

            NR::Matrix add_rot=NR::identity();
            if ( ly->rotate_set ) add_rot=NR::identity()*NR::rotate(M_PI*ly->rotate/180); // angle default to degrees
            NR::Matrix const a( NR::Matrix(flip_y) * add_rot
                        * NR::translate(pt)
                        * NR::translate(sp_char_dx(dx, dx_offset + pos),
                                sp_char_dy(dy, dy_offset + pos)) );
            sp_chars_add_element (chars, glyph, font, a);
            NR::Point adv = nr_font_glyph_advance_get (font, glyph) + letterspacing_adv;

            adv += NR::Point(sp_char_dx (dx, dx_offset + pos), -sp_char_dy (dy, dy_offset + pos));

            pt = pt + NR::Point(adv[NR::X], -adv[NR::Y]);
            
            inspace = FALSE;
            intext = TRUE;
        }
    }

    nr_font_unref (font);
    nr_typeface_unref (face);

    cp = string->p[pos] = pt;

    if (pinspace)
        *pinspace = inspace;
}





/*#####################################################
#  SPTSPAN
#####################################################*/

static void sp_tspan_class_init (SPTSpanClass *classname);
static void sp_tspan_init (SPTSpan *tspan);

static void sp_tspan_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_tspan_release (SPObject *object);
static void sp_tspan_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_tspan_child_added (SPObject *object, SPRepr *rch, SPRepr *ref);
static void sp_tspan_remove_child (SPObject *object, SPRepr *rch);
static void sp_tspan_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_tspan_modified (SPObject *object, unsigned int flags);
static SPRepr *sp_tspan_write (SPObject *object, SPRepr *repr, guint flags);

static void sp_tspan_bbox(SPItem *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags);
static NRArenaItem *sp_tspan_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_tspan_hide (SPItem *item, unsigned int key);

static void sp_tspan_set_shape (SPTSpan *tspan, SPLayoutData *ly, NR::Point &cp, gboolean firstline, gboolean *inspace);

static SPItemClass *tspan_parent_class;




/**
 *
 */
GType
sp_tspan_get_type ()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof (SPTSpanClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc) sp_tspan_class_init,
            NULL,    /* class_finalize */
            NULL,    /* class_data */
            sizeof (SPTSpan),
            16,    /* n_preallocs */
            (GInstanceInitFunc) sp_tspan_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static (SP_TYPE_ITEM, "SPTSpan", &info, (GTypeFlags)0);
    }
    return type;
}



/**
 *
 */
static void
sp_tspan_class_init (SPTSpanClass *classname)
{
    SPObjectClass * sp_object_class;
    SPItemClass * item_class;

    sp_object_class = (SPObjectClass *) classname;
    item_class = (SPItemClass *) classname;

    tspan_parent_class = (SPItemClass*)g_type_class_ref (SP_TYPE_ITEM);

    sp_object_class->build = sp_tspan_build;
    sp_object_class->release = sp_tspan_release;
    sp_object_class->set = sp_tspan_set;
    sp_object_class->child_added = sp_tspan_child_added;
    sp_object_class->remove_child = sp_tspan_remove_child;
    sp_object_class->update = sp_tspan_update;
    sp_object_class->modified = sp_tspan_modified;
    sp_object_class->write = sp_tspan_write;

    item_class->bbox = sp_tspan_bbox;
    item_class->show = sp_tspan_show;
    item_class->hide = sp_tspan_hide;
}



/**
 *
 */
static void
sp_tspan_init (SPTSpan *tspan)
{
    /* fixme: Initialize layout */
    sp_svg_length_unset (&tspan->ly.x, SP_SVG_UNIT_NONE, 0.0, 0.0);
    sp_svg_length_unset (&tspan->ly.y, SP_SVG_UNIT_NONE, 0.0, 0.0);
    tspan->ly.dx = NULL;
    tspan->ly.dy = NULL;
    tspan->ly.linespacing = 1.0;
    tspan->string = NULL;
}




/**
 *
 */
static void
sp_tspan_build (SPObject *object, SPDocument *doc, SPRepr *repr)
{
    SPTSpan *tspan = SP_TSPAN (object);

    sp_object_read_attr (object, "x");
    sp_object_read_attr (object, "y");
    sp_object_read_attr (object, "dx");
    sp_object_read_attr (object, "dy");
    sp_object_read_attr (object, "rotate");
    sp_object_read_attr (object, "sodipodi:role");
    
    SPRepr *rch;
    for (rch = repr->children; rch != NULL; rch = rch->next) {
        if (rch->type == SP_XML_TEXT_NODE) break;
    }

    if (!rch) {
        rch = sp_xml_document_createTextNode (sp_repr_document (repr), "");
        sp_repr_add_child (repr, rch, NULL);
    }

    if (((SPObjectClass *) tspan_parent_class)->build)
        ((SPObjectClass *) tspan_parent_class)->build (object, doc, repr);

    SPObject *ochild;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
	if (SP_IS_STRING(ochild)) {
	    tspan->string = ochild;
	    break;
	}
    }
}




/**
 *
 */
static void
sp_tspan_release (SPObject *object)
{
    SPTSpan *tspan = SP_TSPAN (object);

    if (tspan->string) {
	tspan->string = NULL;
    } else {
        g_print ("NULL tspan content\n");
    }

    if (((SPObjectClass *) tspan_parent_class)->release)
        ((SPObjectClass *) tspan_parent_class)->release (object);
}




/**
 *
 */
static void
sp_tspan_set (SPObject *object, unsigned int key, const gchar *value)
{
    SPTSpan *tspan = SP_TSPAN (object);

    /* fixme: Vectors */
    switch (key) {
    case SP_ATTR_X:
        if (!sp_svg_length_read (value, &tspan->ly.x)) {
            sp_svg_length_unset (&tspan->ly.x, SP_SVG_UNIT_NONE, 0.0, 0.0);
        }
        /* fixme: Re-layout it */
        if (tspan->role != SP_TSPAN_ROLE_LINE) object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;
    case SP_ATTR_Y:
        if (!sp_svg_length_read (value, &tspan->ly.y)) {
            sp_svg_length_unset (&tspan->ly.y, SP_SVG_UNIT_NONE, 0.0, 0.0);
        }
        /* fixme: Re-layout it */
        if (tspan->role != SP_TSPAN_ROLE_LINE) object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;
    case SP_ATTR_DX:
        if (!(tspan->ly.dx = sp_svg_length_list_read (value))) {
            tspan->ly.dx = NULL;
        }
        /* fixme: Re-layout it */
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;
    case SP_ATTR_DY:
        if (!(tspan->ly.dy = sp_svg_length_list_read (value))) {
            tspan->ly.dy = NULL;
        }
        /* fixme: Re-layout it */
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;
    case SP_ATTR_ROTATE:
        /* fixme: Implement SVGNumber or something similar (Lauris) */
        tspan->ly.rotate = (value) ? g_ascii_strtod (value, NULL) : 0.0;
        tspan->ly.rotate_set = (value != NULL);
        /* fixme: Re-layout it */
        break;
    case SP_ATTR_SODIPODI_ROLE:
        if (value && (!strcmp (value, "line") || !strcmp (value, "paragraph"))) {
            tspan->role = SP_TSPAN_ROLE_LINE;
        } else {
            tspan->role = SP_TSPAN_ROLE_UNSPECIFIED;
        }
        break;
    default:
        if (((SPObjectClass *) tspan_parent_class)->set)
            (((SPObjectClass *) tspan_parent_class)->set) (object, key, value);
        break;
    }
}



/**
 *
 */
static void
sp_tspan_child_added (SPObject *object, SPRepr *rch, SPRepr *ref)
{
    SPTSpan *tspan = SP_TSPAN (object);

    if (((SPObjectClass *) tspan_parent_class)->child_added)
        ((SPObjectClass *) tspan_parent_class)->child_added (object, rch, ref);

    SPObject *ochild;
    tspan->string = NULL;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if (SP_IS_STRING(ochild)) {
            tspan->string = ochild;
	    break;
	}
    }
}



/**
 *
 */
static void
sp_tspan_remove_child (SPObject *object, SPRepr *rch)
{
    SPTSpan *tspan = SP_TSPAN (object);

    if (((SPObjectClass *) tspan_parent_class)->remove_child)
        ((SPObjectClass *) tspan_parent_class)->remove_child (object, rch);

    SPObject *ochild;
    tspan->string = NULL;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if (SP_IS_STRING(ochild)) {
            tspan->string = ochild;
	    break;
	}
    }
}



/**
 *
 */
static void
sp_tspan_update (SPObject *object, SPCtx *ctx, guint flags)
{
    SPTSpan *tspan = SP_TSPAN (object);
    SPStyle *style = SP_OBJECT_STYLE (object);
    SPItemCtx *ictx = (SPItemCtx *) ctx;
    GList *i;

    if (((SPObjectClass *) tspan_parent_class)->update)
        ((SPObjectClass *) tspan_parent_class)->update (object, ctx, flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG)
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    /* Update relative distances */
    const double d = 1.0 / NR_MATRIX_DF_EXPANSION (&ictx->i2vp);
    sp_text_update_length (&tspan->ly.x, style->font_size.computed, style->font_size.computed * 0.5, d);
    sp_text_update_length (&tspan->ly.y, style->font_size.computed, style->font_size.computed * 0.5, d);

    for (i = tspan->ly.dx; i != NULL; i = i->next)
        sp_text_update_length ((SPSVGLength *) i->data, style->font_size.computed, style->font_size.computed * 0.5, d);
    for (i = tspan->ly.dy; i != NULL; i = i->next)
        sp_text_update_length ((SPSVGLength *) i->data, style->font_size.computed, style->font_size.computed * 0.5, d);

    SPObject *ochild;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if ( flags || ( ochild->uflags & SP_OBJECT_MODIFIED_FLAG )) {
	    ochild->updateDisplay(ctx, flags);
        }
    }
}



/**
 *
 */
static void
sp_tspan_modified (SPObject *object, unsigned int flags)
{
    if (((SPObjectClass *) tspan_parent_class)->modified)
        ((SPObjectClass *) tspan_parent_class)->modified (object, flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG)
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    SPObject *ochild;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if (flags || (ochild->mflags & SP_OBJECT_MODIFIED_FLAG)) {
            ochild->emitModified(flags);
        }
    }
}



/**
 *
 */
static SPRepr *
sp_tspan_write (SPObject *object, SPRepr *repr, guint flags)
{
    SPTSpan *tspan = SP_TSPAN (object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = sp_repr_new ("tspan");
    }

    if (tspan->ly.x.set) sp_repr_set_double (repr, "x", tspan->ly.x.computed);
    if (tspan->ly.y.set) sp_repr_set_double (repr, "y", tspan->ly.y.computed);

    if (tspan->ly.dx && sp_length_list_notallzeroes (tspan->ly.dx)) {
		sp_repr_set_length_list (repr, "dx", tspan->ly.dx);
    } else {
		sp_repr_set_attr (repr, "dx", NULL);
    }
    if (tspan->ly.dy && sp_length_list_notallzeroes (tspan->ly.dy)) {
		sp_repr_set_length_list (repr, "dy", tspan->ly.dy);
    } else {
		sp_repr_set_attr (repr, "dy", NULL);
    }


    if (tspan->ly.rotate_set) sp_repr_set_double (repr, "rotate", tspan->ly.rotate);
    if (flags & SP_OBJECT_WRITE_EXT) {
        sp_repr_set_attr (repr, "sodipodi:role", (tspan->role != SP_TSPAN_ROLE_UNSPECIFIED) ? "line" : NULL);
    }

    /* TODO: we should really hand this off to SPString's own ::build */
    if (flags & SP_OBJECT_WRITE_BUILD) {
	SPObject *ochild;
	for ( ochild = sp_object_first_child(SP_OBJECT(tspan)) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
	    if (SP_IS_STRING(ochild)) {
                SPRepr *rstr;
                /* TEXT element */
                rstr = sp_xml_document_createTextNode (sp_repr_document (repr), SP_STRING_TEXT(SP_STRING(ochild)));
                sp_repr_append_child (repr, rstr);
                sp_repr_unref (rstr);
            }
	}
    } else {
	SPObject *ochild;
	for ( ochild = sp_object_first_child(SP_OBJECT(tspan)) ; ochild ; ochild = SP_OBJECT_NEXT(ochild) ) {
	    if (SP_IS_STRING(ochild)) {
		sp_repr_set_content (SP_OBJECT_REPR (ochild), SP_STRING_TEXT (SP_STRING(ochild)));
	    }
	}
    }

    /* fixme: Strictly speaking, item class write 'transform' too */
    /* fixme: This is harmless as long as tspan affine is identity (lauris) */
    if (((SPObjectClass *) tspan_parent_class)->write)
        ((SPObjectClass *) tspan_parent_class)->write (object, repr, flags);

    return repr;
}


/**
 *
 */
static void
sp_tspan_bbox(SPItem *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags)
{
    SPTSpan *tspan = SP_TSPAN(item);

    if (tspan->string) {
        sp_item_invoke_bbox_full(SP_ITEM(tspan->string), bbox, transform, flags, FALSE);
    }
}


/**
 *
 */
static NRArenaItem *
sp_tspan_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
    SPTSpan *tspan = SP_TSPAN (item);

    if (tspan->string) {
        NRArenaItem *ai, *ac;
        ai = NRArenaGroup::create(arena);
        nr_arena_group_set_transparent (NR_ARENA_GROUP (ai), FALSE);
        ac = sp_item_invoke_show (SP_ITEM (tspan->string), arena, key, flags);
        if (ac) {
            nr_arena_item_add_child (ai, ac, NULL);
            nr_arena_item_unref (ac);
        }
        return ai;
    }

    return NULL;
}


/**
 *
 */
static void
sp_tspan_hide (SPItem *item, unsigned int key)
{
    SPTSpan *tspan = SP_TSPAN (item);

    if (tspan->string)
        sp_item_invoke_hide (SP_ITEM (tspan->string), key);

    if (((SPItemClass *) tspan_parent_class)->hide)
        ((SPItemClass *) tspan_parent_class)->hide (item, key);
}


/**
 *
 */
static void
sp_tspan_set_shape (SPTSpan *tspan, SPLayoutData *ly, NR::Point &cp, gboolean firstline, gboolean *inspace)
{
    sp_string_set_shape (SP_STRING (tspan->string), &tspan->ly, cp, inspace);
}






/*#####################################################
#  SPTEXT
#####################################################*/

static void sp_text_class_init (SPTextClass *classname);
static void sp_text_init (SPText *text);

static void sp_text_build (SPObject * object, SPDocument * document, SPRepr * repr);
static void sp_text_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_text_child_added (SPObject *object, SPRepr *rch, SPRepr *ref);
static void sp_text_remove_child (SPObject *object, SPRepr *rch);
static void sp_text_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_text_modified (SPObject *object, guint flags);
static SPRepr *sp_text_write (SPObject *object, SPRepr *repr, guint flags);

static void sp_text_bbox(SPItem *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags);
static NRArenaItem *sp_text_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_text_hide (SPItem *item, unsigned int key);
static char * sp_text_description (SPItem *item);
static std::vector<NR::Point> sp_text_snappoints(SPItem *item);
static NR::Matrix sp_text_set_transform (SPItem *item, NR::Matrix const &xform);
static void sp_text_print (SPItem *item, SPPrintContext *gpc);

static void sp_text_request_relayout (SPText *text, guint flags);
static void sp_text_update_immediate_state (SPText *text);
static void sp_text_set_shape (SPText *text);

static SPObject *sp_text_get_child_by_position (SPText *text, gint pos);

static SPItemClass *text_parent_class;


/**
 *
 */
GType
sp_text_get_type ()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof (SPTextClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc) sp_text_class_init,
            NULL,    /* class_finalize */
            NULL,    /* class_data */
            sizeof (SPText),
            16,    /* n_preallocs */
            (GInstanceInitFunc) sp_text_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static (SP_TYPE_ITEM, "SPText", &info, (GTypeFlags)0);
    }
    return type;
}


/**
 *
 */
static void
sp_text_class_init (SPTextClass *classname)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) classname;
    SPItemClass *item_class = (SPItemClass *) classname;

    text_parent_class = (SPItemClass*)g_type_class_ref (SP_TYPE_ITEM);

    sp_object_class->build = sp_text_build;
    sp_object_class->set = sp_text_set;
    sp_object_class->child_added = sp_text_child_added;
    sp_object_class->remove_child = sp_text_remove_child;
    sp_object_class->update = sp_text_update;
    sp_object_class->modified = sp_text_modified;
    sp_object_class->write = sp_text_write;

    item_class->bbox = sp_text_bbox;
    item_class->show = sp_text_show;
    item_class->hide = sp_text_hide;
    item_class->description = sp_text_description;
    item_class->snappoints = sp_text_snappoints;
    item_class->set_transform = sp_text_set_transform;
    item_class->print = sp_text_print;
}


/**
 *
 */
static void
sp_text_init (SPText *text)
{
    /* fixme: Initialize layout */
    sp_svg_length_unset (&text->ly.x, SP_SVG_UNIT_NONE, 0.0, 0.0);
    sp_svg_length_unset (&text->ly.y, SP_SVG_UNIT_NONE, 0.0, 0.0);
    text->ly.dx = NULL;
    text->ly.dy = NULL;
    text->ly.linespacing = 1.0;
}


/**
 *
 */
static void
sp_text_build (SPObject *object, SPDocument *doc, SPRepr *repr)
{
    SPText *text = SP_TEXT (object);

    sp_object_read_attr (object, "x");
    sp_object_read_attr (object, "y");
    sp_object_read_attr (object, "dx");
    sp_object_read_attr (object, "dy");
    sp_object_read_attr (object, "rotate");
    sp_object_read_attr (object, "sodipodi:linespacing");

    if (((SPObjectClass *) text_parent_class)->build)
        ((SPObjectClass *) text_parent_class)->build (object, doc, repr);

    SPVersion version = sp_object_get_sodipodi_version (object);

    SPRepr *rch;
    if (sp_version_inside_range (version, 0, 0, 0, 25)) {
        const gchar *content;
        for (rch = repr->children; rch != NULL; rch = rch->next) {
            if (rch->type == SP_XML_TEXT_NODE) {
                content = sp_repr_content (rch);
                sp_text_set_repr_text_multiline (text, content);
                break;
            }
        }
    }

    SPObject *ochild;
    SPObject *next;
    for ( ochild = sp_object_first_child(object) ; ochild ; ochild = next ) {
        next = SP_OBJECT_NEXT(ochild);
        if (SP_IS_STRING(ochild)) {
	    SP_STRING(ochild)->ly = &text->ly;
        } else if (!SP_IS_TSPAN(ochild)) {
	    /* at present we don't know what to do with non-tspan children */
            sp_object_detach_unref(object, ochild);
	}
    }

    sp_text_update_immediate_state (text);
}

/**
 *
 */
static void
sp_text_set (SPObject *object, unsigned int key, const gchar *value)
{
    SPText *text = SP_TEXT (object);

    /* fixme: Vectors (Lauris) */
    switch (key) {
    case SP_ATTR_X:
        if (!sp_svg_length_read (value, &text->ly.x)) {
            sp_svg_length_unset (&text->ly.x, SP_SVG_UNIT_NONE, 0.0, 0.0);
        }
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
        break;
    case SP_ATTR_Y:
        if (!sp_svg_length_read (value, &text->ly.y)) {
            sp_svg_length_unset (&text->ly.y, SP_SVG_UNIT_NONE, 0.0, 0.0);
        }
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
        break;
    case SP_ATTR_DX:
        if (!(text->ly.dx = sp_svg_length_list_read (value))) {
            text->ly.dx = NULL;
        }
        /* fixme: Re-layout it */
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
        break;
    case SP_ATTR_DY:
        if (!(text->ly.dy = sp_svg_length_list_read (value))) {
            text->ly.dy = NULL;
        }
        /* fixme: Re-layout it */
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
        break;
    case SP_ATTR_ROTATE:
        /* fixme: Implement SVGNumber or something similar (Lauris) */
        text->ly.rotate = (value) ? g_ascii_strtod (value, NULL) : 0.0;
        text->ly.rotate_set = (value != NULL);
        /* fixme: Re-layout it */
        break;
    case SP_ATTR_SODIPODI_LINESPACING:
        text->ly.linespacing = 1.0;
        if (value) {
            text->ly.linespacing = sp_svg_read_percentage (value, 1.0);
            text->ly.linespacing = CLAMP (text->ly.linespacing, 0.0, 1000.0);
        }
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
        break;
    default:
        if (((SPObjectClass *) text_parent_class)->set)
            ((SPObjectClass *) text_parent_class)->set (object, key, value);
        break;
    }
}


/**
 *
 */
static void
sp_text_child_added (SPObject *object, SPRepr *rch, SPRepr *ref)
{
    SPItem *item = SP_ITEM (object);
    SPText *text = SP_TEXT (object);

    if (((SPObjectClass *) text_parent_class)->child_added)
        ((SPObjectClass *) text_parent_class)->child_added (object, rch, ref);

    SPObject *ochild = sp_object_get_child_by_repr(object, rch);
    if (ochild) {
        if (SP_IS_STRING(ochild)) {
	    SPString *string=SP_STRING(ochild);
            string->ly = &text->ly;
	} else if (!SP_IS_TSPAN(ochild)) {
            /* ugly, but right now SPText assumes it won't have any non-tspan children :/ */
	    sp_object_detach_unref(object, ochild);
	    ochild = NULL;
	}
    }

    if (ochild) {
        SPItemView *v;
        NRArenaItem *ac;

        for (v = item->display; v != NULL; v = v->next) {
            ac = sp_item_invoke_show (SP_ITEM (ochild), NR_ARENA_ITEM_ARENA (v->arenaitem), v->key, v->flags);
            if (ac) {
                nr_arena_item_add_child (v->arenaitem, ac, NULL);
                nr_arena_item_unref (ac);
            }
        }
    }

    sp_text_request_relayout (text, SP_OBJECT_MODIFIED_FLAG | SP_TEXT_CONTENT_MODIFIED_FLAG);
    /* fixme: Instead of forcing it, do it when needed */
    sp_text_update_immediate_state (text);
}


/**
 *
 */
static void
sp_text_remove_child (SPObject *object, SPRepr *rch)
{
    SPText *text = SP_TEXT (object);

    if (((SPObjectClass *) text_parent_class)->remove_child)
        ((SPObjectClass *) text_parent_class)->remove_child (object, rch);

    sp_text_request_relayout (text, SP_OBJECT_MODIFIED_FLAG | SP_TEXT_CONTENT_MODIFIED_FLAG);
    sp_text_update_immediate_state (text);
}




/**
 * fixme: This is wrong, as we schedule relayout every time something changes
 */
static void
sp_text_update (SPObject *object, SPCtx *ctx, guint flags)
{
    SPItemCtx *ictx;
    SPObject *child;
    GSList *l; 
    GList *i;

    SPText *text = SP_TEXT (object);
    SPStyle *style = SP_OBJECT_STYLE (text);
    ictx = (SPItemCtx *) ctx;

    if (((SPObjectClass *) text_parent_class)->update)
        ((SPObjectClass *) text_parent_class)->update (object, ctx, flags);

    guint cflags = (flags & SP_OBJECT_MODIFIED_CASCADE);
    if (flags & SP_OBJECT_MODIFIED_FLAG)
        cflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;

    /* Update relative distances */
    double d = 1.0 / NR_MATRIX_DF_EXPANSION (&ictx->i2vp);
    sp_text_update_length (&text->ly.x, style->font_size.computed, style->font_size.computed * 0.5, d);
    sp_text_update_length (&text->ly.y, style->font_size.computed, style->font_size.computed * 0.5, d);

    for (i = text->ly.dx; i != NULL; i = i->next)
        sp_text_update_length ((SPSVGLength *) i->data, style->font_size.computed, style->font_size.computed * 0.5, d);
    for (i = text->ly.dy; i != NULL; i = i->next)
        sp_text_update_length ((SPSVGLength *) i->data, style->font_size.computed, style->font_size.computed * 0.5, d);

    /* Create temporary list of children */
    l = NULL;
    for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        sp_object_ref (SP_OBJECT (child), object);
        l = g_slist_prepend (l, child);
    }
    l = g_slist_reverse (l);
    while (l) {
        child = SP_OBJECT (l->data);
        l = g_slist_remove (l, child);
        if (cflags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            /* fixme: Do we need transform? */
	    child->updateDisplay(ctx, cflags);
        }
        sp_object_unref (SP_OBJECT (child), object);
    }
    if (text->relayout || (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG))) {
        /* fixme: It is not nice to have it here, but otherwise children content changes does not work */
        /* fixme: Even now it may not work, as we are delayed */
        /* fixme: So check modification flag everywhere immediate state is used */
        sp_text_update_immediate_state (text);
        sp_text_set_shape (text);
        text->relayout = FALSE;
    }
}



/**
 *
 */
static void
sp_text_modified (SPObject *object, guint flags)
{
    if (((SPObjectClass *) text_parent_class)->modified)
        ((SPObjectClass *) text_parent_class)->modified (object, flags);

    guint cflags = (flags & SP_OBJECT_MODIFIED_CASCADE);
    if (flags & SP_OBJECT_MODIFIED_FLAG) cflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;

    /* Create temporary list of children */
    GSList *l = NULL;
    SPObject *child;
    for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        sp_object_ref (SP_OBJECT (child), object);
        l = g_slist_prepend (l, child);
    }
    l = g_slist_reverse (l);
    while (l) {
        child = SP_OBJECT (l->data);
        l = g_slist_remove (l, child);
        if (cflags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(cflags);
        }
        sp_object_unref (SP_OBJECT (child), object);
    }
}



/**
 *
 */
static SPRepr *
sp_text_write (SPObject *object, SPRepr *repr, guint flags)
{
    SPObject *child;
    SPRepr *crepr;

    SPText *text = SP_TEXT (object);

    if (flags & SP_OBJECT_WRITE_BUILD) {
        if (!repr)
            repr = sp_repr_new ("text");
        GSList *l = NULL;
        for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            if (SP_IS_TSPAN (child)) {
                crepr = child->updateRepr(NULL, flags);
                if (crepr) l = g_slist_prepend (l, crepr);
            } else {
                crepr = sp_xml_document_createTextNode (sp_repr_document (repr), SP_STRING_TEXT (child));
            }
        }
        while (l) {
            sp_repr_add_child (repr, (SPRepr *) l->data, NULL);
            sp_repr_unref ((SPRepr *) l->data);
            l = g_slist_remove (l, l->data);
        }
    } else {
        for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            if (SP_IS_TSPAN (child)) {
                child->updateRepr(flags);
            } else {
                sp_repr_set_content (SP_OBJECT_REPR (child), SP_STRING_TEXT (child));
            }
        }
    }

    if (text->ly.x.set)
        sp_repr_set_double (repr, "x", text->ly.x.computed);
    if (text->ly.y.set)
        sp_repr_set_double (repr, "y", text->ly.y.computed);

    if (text->ly.dx && sp_length_list_notallzeroes (text->ly.dx)) {
		sp_repr_set_length_list (repr, "dx", text->ly.dx);
    } else {
		sp_repr_set_attr (repr, "dx", NULL);
    }
    if (text->ly.dy && sp_length_list_notallzeroes (text->ly.dy)) {
		sp_repr_set_length_list (repr, "dy", text->ly.dy);
    } else {
		sp_repr_set_attr (repr, "dy", NULL);
    }

    if (text->ly.rotate_set)
        sp_repr_set_double (repr, "rotate", text->ly.rotate);

    if (((SPObjectClass *) (text_parent_class))->write)
        ((SPObjectClass *) (text_parent_class))->write (object, repr, flags);

    return repr;
}



/**
 *
 */
static void
sp_text_bbox(SPItem *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags)
{
    for (SPObject *o = sp_object_first_child(SP_OBJECT(item)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        SPItem *child = SP_ITEM(o);
        NR::Matrix const a(child->transform * transform);
        sp_item_invoke_bbox_full(child, bbox, a, flags, FALSE);
    }
}



/**
 *
 */
static NRArenaItem *
sp_text_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags)
{
    NRArenaItem *ai = NRArenaGroup::create(arena);
    nr_arena_group_set_transparent (NR_ARENA_GROUP (ai), FALSE);

    NRArenaItem *ar = NULL;
    for (SPObject *o = sp_object_first_child(SP_OBJECT(item)) ; o != NULL; o = SP_OBJECT_NEXT(o) ) {
        if (SP_IS_ITEM (o)) {
            SPItem *child = SP_ITEM (o);
            NRArenaItem *ac = sp_item_invoke_show (child, arena, key, flags);
            if (ac) {
                nr_arena_item_add_child (ai, ac, ar);
                ar = ac;
                nr_arena_item_unref (ac);
            }
        }
    }

    return ai;
}



/**
 *
 */
static void
sp_text_hide (SPItem *item, unsigned int key)
{
    for (SPObject *o = sp_object_first_child(SP_OBJECT(item)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        if (SP_IS_ITEM (o)) {
            SPItem *child = SP_ITEM (o);
            sp_item_invoke_hide (child, key);
        }
    }

    if (((SPItemClass *) text_parent_class)->hide)
        ((SPItemClass *) text_parent_class)->hide (item, key);
}



/**
 *
 */
static char *
sp_text_description (SPItem * item)
{
    gchar n[256];

    SPText *text = (SPText *) item;
    SPStyle *style = SP_OBJECT_STYLE (text);

    NRTypeFace *tf = nr_type_directory_lookup_fuzzy(style->text->font_family.value,
                            font_style_to_pos(*style));
    NRFont *font = nr_font_new_default (tf, NR_TYPEFACE_METRICS_HORIZONTAL, style->font_size.computed);
    nr_typeface_name_get (NR_FONT_TYPEFACE (font), n, 256);
    nr_typeface_unref (tf);

    return g_strdup_printf (_("Text (%s, %.5gpt)"), n, NR_FONT_SIZE (font));
}



/**
 * fixme: Do text chunks here (Lauris)
 * fixme: We'll remove string bbox adjustment and bring it here for the whole chunk (Lauris)
 */
static void
sp_text_set_shape (SPText *text)
{
    /* The logic should be: */
    /* 1. Calculate attributes */
    /* 2. Iterate through children asking them to set shape */

    NR::Point cp = NR::Point(text->ly.x.computed, text->ly.y.computed);

    gboolean isfirstline = TRUE;
    gboolean inspace = FALSE;

    SPObject *child = sp_object_first_child(SP_OBJECT(text));
    while (child != NULL) {
        SPObject *next, *spnew;
        SPString *string;
        if (SP_IS_TSPAN (child)) {
            SPTSpan *tspan;
            /* fixme: Maybe break this up into 2 pieces - relayout and set shape (Lauris) */
            tspan = SP_TSPAN (child);
            string = SP_TSPAN_STRING (tspan);
            switch (tspan->role) {
            case SP_TSPAN_ROLE_PARAGRAPH:
            case SP_TSPAN_ROLE_LINE:
                if (!isfirstline) {
                    if (child->style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
                        cp[NR::X] -= child->style->font_size.computed * text->ly.linespacing;
                        cp[NR::Y] = text->ly.y.computed;
                    } else {
                        cp[NR::X] = text->ly.x.computed;
                        cp[NR::Y] += child->style->font_size.computed * text->ly.linespacing;
                    }
                }
                /* fixme: This is extremely (EXTREMELY) dangerous (Lauris) */
                /* fixme: Our only hope is to ensure LINE tspans do not request ::modified */
                sp_document_set_undo_sensitive (SP_OBJECT_DOCUMENT (tspan), FALSE);
                sp_repr_set_double (SP_OBJECT_REPR (tspan), "x", cp[NR::X]);
                sp_repr_set_double (SP_OBJECT_REPR (tspan), "y", cp[NR::Y]);
                sp_document_set_undo_sensitive (SP_OBJECT_DOCUMENT (tspan), TRUE);
                break;
            case SP_TSPAN_ROLE_UNSPECIFIED:
                if (tspan->ly.x.set) {
                    cp[NR::X] = tspan->ly.x.computed;
                } else {
                    tspan->ly.x.computed = cp[NR::X];
                }
                if (tspan->ly.y.set) {
                    cp[NR::Y] = tspan->ly.y.computed;
                } else {
                    tspan->ly.y.computed = cp[NR::Y];
                }
                break;
            default:
                /* Error */
                break;
            }
        } else {
            string = SP_STRING (child);
        }
        /* Calculate block bbox */
        NR::Point advance = NR::Point(string->ly->dx ? ((SPSVGLength *) string->ly->dx->data)->computed : 0.0,
                                     string->ly->dy ? ((SPSVGLength *) string->ly->dy->data)->computed : 0.0);

        NRRect bbox;
        bbox.x0 = string->bbox.x0 + advance[NR::X];
        bbox.y0 = string->bbox.y0 + advance[NR::Y];
        bbox.x1 = string->bbox.x1 + advance[NR::X];
        bbox.y1 = string->bbox.y1 + advance[NR::Y];
        advance += string->advance;
        for (next = child->next; next != NULL; next = next->next) {
            SPString *string;
            if (SP_IS_TSPAN (next)) {
                SPTSpan *tspan;
                tspan = SP_TSPAN (next);
                if (tspan->role != SP_TSPAN_ROLE_UNSPECIFIED)
                    break;
                if ((tspan->ly.x.set) || (tspan->ly.y.set))
                    break;

                string = SP_TSPAN_STRING (tspan);
            } else {
                string = SP_STRING (next);
            }
            bbox.x0 = MIN (bbox.x0, string->bbox.x0 + advance[NR::X]);
            bbox.y0 = MIN (bbox.y0, string->bbox.y0 + advance[NR::Y]);
            bbox.x1 = MAX (bbox.x1, string->bbox.x1 + advance[NR::X]);
            bbox.y1 = MAX (bbox.y1, string->bbox.y1 + advance[NR::Y]);
            advance += string->advance;
        }
        spnew = next;
        /* Calculate starting position */
        switch (child->style->text_anchor.computed) {
        case SP_CSS_TEXT_ANCHOR_START:
            break;
        case SP_CSS_TEXT_ANCHOR_MIDDLE:
            /* Ink midpoint */
            if (child->style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
                cp[NR::Y] -= (bbox.y0 + bbox.y1) / 2;
            } else {
                cp[NR::X] -= (bbox.x0 + bbox.x1) / 2;
            }
            break;
        case SP_CSS_TEXT_ANCHOR_END:
            /* Ink endpoint */
            if (child->style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
                cp[NR::Y] -= bbox.y1;
            } else {
                cp[NR::X] -= bbox.x1;
            }
            break;
        default:
            break;
        }
        /* Set child shapes */
        for (next = child; (next != NULL) && (next != spnew); next = next->next) {
            if (SP_IS_STRING (next)) {
                sp_string_set_shape (SP_STRING (next), &text->ly, cp, &inspace);
            } else {
                SPTSpan *tspan;
                tspan = SP_TSPAN (next);

                sp_tspan_set_shape (tspan, &text->ly, cp, isfirstline, &inspace);
            }
        }
        child = next;
        isfirstline = FALSE;
    }

    NRRect paintbox;
    sp_item_invoke_bbox(SP_ITEM(text), &paintbox, NR::identity(), TRUE);

    for (child = sp_object_first_child(SP_OBJECT(text)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        SPString *string;
        if (SP_IS_TSPAN (child)) {
            string = SP_TSPAN_STRING (child);
        } else {
            string = SP_STRING (child);
        }
        sp_chars_set_paintbox (SP_CHARS (string), &paintbox);
    }
}



/**
 *
 */
static std::vector<NR::Point>
sp_text_snappoints(SPItem *item)
{
    /* We use corners of item. */
    /* (An older version of this file added a snappoint at the baseline of the first line.
       Maybe we should have a snappoint at the baseline of each line?) */
     std::vector<NR::Point> p;
   
     if (((SPItemClass *) text_parent_class)->snappoints) {
         p = ((SPItemClass *) text_parent_class)->snappoints (item);
     }
     
     return p;
}




/*
 * Initially we'll do:
 * Transform x, y, set x, y, clear translation
 */
static NR::Matrix
sp_text_set_transform (SPItem *item, NR::Matrix const &xform)
{
    SPText *text = SP_TEXT (item);

    NR::Matrix i2p(xform);
    // translate?
    i2p[4] = 0.0;
    i2p[5] = 0.0;
    NR::Matrix p2i=i2p.inverse();

    NR::Point pos=NR::Point(text->ly.x.computed, text->ly.y.computed) * xform * p2i;
    text->ly.x = pos[NR::X];
    text->ly.y = pos[NR::Y];

    for (SPObject *child = sp_object_first_child(SP_OBJECT(text)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) { 
        if (SP_IS_TSPAN (child)) {
            SPTSpan *tspan;
            tspan = SP_TSPAN (child);
            if (tspan->ly.x.set || tspan->ly.y.set) {
		double x, y;
                x = (tspan->ly.x.set) ? tspan->ly.x.computed : text->ly.x.computed;
                y = (tspan->ly.y.set) ? tspan->ly.y.computed : text->ly.y.computed;
		pos = NR::Point(x, y) * xform * p2i;
		tspan->ly.x = pos[NR::X];
		tspan->ly.y = pos[NR::Y];

		SP_OBJECT(tspan)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
            }
        }
    }

    SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);

    return i2p;
}



/**
 *
 */
static void
sp_text_print (SPItem *item, SPPrintContext *ctx)
{
    NRRect pbox, dbox, bbox;
    SPText *text = SP_TEXT (item);

    /* fixme: Think (Lauris) */
    sp_item_invoke_bbox(item, &pbox, NR::identity(), TRUE);
    sp_item_bbox_desktop (item, &bbox);
    dbox.x0 = 0.0;
    dbox.y0 = 0.0;
    dbox.x1 = sp_document_width (SP_OBJECT_DOCUMENT (item));
    dbox.y1 = sp_document_height (SP_OBJECT_DOCUMENT (item));
    NRMatrix ctm;
    sp_item_i2d_affine (item, &ctm);

    bool text_to_path;
    sp_print_get_param(ctx, "textToPath", &text_to_path);
    if (text_to_path) {
      for (SPObject *ch = sp_object_first_child(SP_OBJECT(text)) ; ch != NULL ; ch = SP_OBJECT_NEXT(ch) ) {
        if (SP_IS_TSPAN (ch)) {
	  sp_chars_do_print (SP_CHARS (SP_TSPAN (ch)->string), ctx, &ctm, &pbox, &dbox, &bbox);
        } else if (SP_IS_STRING (ch)) {
	  sp_chars_do_print (SP_CHARS (ch), ctx, &ctm, &pbox, &dbox, &bbox);
        }
      }
    } else {
      for (SPObject *ch = sp_object_first_child (SP_OBJECT(text));
	   ch != NULL;
	   ch = SP_OBJECT_NEXT (ch)) {

	SPString* s = NULL;
	if (SP_IS_STRING(ch)) {
	  s = SP_STRING(ch);
	} else if (SP_IS_TSPAN (ch)) {
	  s = SP_STRING(SP_TSPAN(ch)->string);
	}

	if (s && strlen(s->text) > 0) {
	  sp_print_text (ctx, s->text, *(s->p), SP_OBJECT_STYLE (ch));
	}
      }
    }
}



/**
 *
 */
int
sp_text_is_empty (SPText *text)
{
    for (SPObject *ch = sp_object_first_child(SP_OBJECT(text)) ; ch != NULL; ch = SP_OBJECT_NEXT(ch) ) {
        SPString *str;
        gchar *p;
        str = SP_TEXT_CHILD_STRING (ch);
        for (p = str->text; p && *p; p = g_utf8_next_char (p)) {
            gunichar unival;
            unival = g_utf8_get_char (p);
            if ((unival > 0xe000) && (unival <= 0xf8ff)) return FALSE;
            if (g_unichar_isgraph (unival)) return FALSE;
        }
    }

    return TRUE;
}



/**
 *
 */
gchar *
sp_text_get_string_multiline (SPText *text)
{
    GSList *strs = NULL;
    for (SPObject *ch = sp_object_first_child(SP_OBJECT(text)) ; ch != NULL; ch = SP_OBJECT_NEXT(ch) ) {
        if (SP_IS_TSPAN (ch)) {
            SPTSpan *tspan = SP_TSPAN (ch);
            if (tspan->string && SP_STRING (tspan->string)->text) {
                strs = g_slist_prepend (strs, SP_STRING (tspan->string)->text);
            }
        } else if (SP_IS_STRING (ch) && SP_STRING (ch)->text) {
            strs = g_slist_prepend (strs, SP_STRING (ch)->text);
        } else {
            continue;
        }
    }

    gint len = 0;
    for (GSList *l = strs; l != NULL; l = l->next) {
        len += strlen ((const gchar*)l->data);
        len += strlen ("\n");
    }

    len += 1;

    strs = g_slist_reverse (strs);

    gchar *str = g_new (gchar, len);
    gchar *p = str;
    while (strs) {
        memcpy (p, strs->data, strlen ((const gchar*)strs->data));
        p += strlen ((const gchar*)strs->data);
        strs = g_slist_remove (strs, strs->data);
        if (strs) *p++ = '\n';
    }
    *p++ = '\0';

    return str;
}



/**
 *
 */
void
sp_text_set_repr_text_multiline (SPText *text, const gchar *str)
{
    g_return_if_fail (text != NULL);
    g_return_if_fail (SP_IS_TEXT (text));

    SPRepr *repr = SP_OBJECT_REPR (text);
    SPStyle *style = SP_OBJECT_STYLE (text);

    if (!str) str = "";
    gchar *content = g_strdup (str);

    sp_repr_set_content (SP_OBJECT_REPR (text), NULL);
    while (repr->children) {
        sp_repr_remove_child (repr, repr->children);
    }

    NR::Point cp(text->ly.x.computed, text->ly.y.computed);

    gchar *p = content;
    while (p) {
        SPRepr *rtspan, *rstr;
        gchar *e = strchr (p, '\n');
        if (e)
            *e = '\0';
        rtspan = sp_repr_new ("tspan");
        sp_repr_set_double (rtspan, "x", cp[NR::X]);
        sp_repr_set_double (rtspan, "y", cp[NR::Y]);
        if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
            /* fixme: real line height */
            /* fixme: What to do with mixed direction tspans? */
            cp[NR::X] -= style->font_size.computed;
        } else {
            cp[NR::Y] += style->font_size.computed;
        }
        sp_repr_set_attr (rtspan, "sodipodi:role", "line");
        rstr = sp_xml_document_createTextNode (sp_repr_document (repr), p);
        sp_repr_add_child (rtspan, rstr, NULL);
        sp_repr_append_child (repr, rtspan);
        p = (e) ? e + 1 : NULL;
    }

    g_free (content);

    /* fixme: Calculate line positions (Lauris) */
}



/**
 *
 */
SPCurve *
sp_text_normalized_bpath (SPText *text)
{
    g_return_val_if_fail (text != NULL, NULL);
    g_return_val_if_fail (SP_IS_TEXT (text), NULL);

    GSList *cc = NULL;
    for (SPObject *child = sp_object_first_child(SP_OBJECT(text)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        SPCurve *c;
        if (SP_IS_STRING (child)) {
            c = sp_chars_normalized_bpath (SP_CHARS (child));
        } else if (SP_IS_TSPAN (child)) {
            SPTSpan *tspan;
            tspan = SP_TSPAN (child);
            c = sp_chars_normalized_bpath (SP_CHARS (tspan->string));
        } else {
            c = NULL;
        }
        if (c) cc = g_slist_prepend (cc, c);
    }

    cc = g_slist_reverse (cc);
    
    SPCurve *curve;
    if ( cc ) {
        curve = sp_curve_concat (cc);
    } else {
        curve=sp_curve_new();
    }
    
    while (cc) {
        sp_curve_unref ((SPCurve *) cc->data);
        cc = g_slist_remove (cc, cc->data);
    }

    return curve;
}



/**
 *
 */
static void
sp_text_update_immediate_state (SPText *text)
{
    guint start = 0;
    for (SPObject *child = sp_object_first_child(SP_OBJECT(text)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
        SPString *string;
        if (SP_IS_TSPAN (child)) {
            string = SP_TSPAN_STRING (child);
        } else {
            string = SP_STRING (child);
        }
        string->start = start;
        start += string->length;
        /* Count newlines as well */
        if (SP_OBJECT_NEXT(child)) {
            start += 1;
        }
    }
}



/**
 *
 */
static void
sp_text_request_relayout (SPText *text, guint flags)
{
    text->relayout = TRUE;

    SP_OBJECT (text)->requestDisplayUpdate(flags);
}



/**
 * fixme: Think about these (Lauris)
 */
gint
sp_text_get_length (SPText *text)
{
    g_return_val_if_fail (text != NULL, 0);
    g_return_val_if_fail (SP_IS_TEXT (text), 0);

    SPObject *child = sp_object_first_child(SP_OBJECT(text));
    if (!child) return 0;
    while (SP_OBJECT_NEXT(child)) {
        child = SP_OBJECT_NEXT(child);
    }

    SPString *string;
    if (SP_IS_STRING (child)) {
        string = SP_STRING (child);
    } else {
        string = SP_TSPAN_STRING (child);
    }

    return string->start + string->length;
}




/**
 *
 */
SPTSpan *
sp_text_append_line (SPText *text)
{
    g_return_val_if_fail (text != NULL, NULL);
    g_return_val_if_fail (SP_IS_TEXT (text), NULL);

    SPStyle *style = SP_OBJECT_STYLE (text);

    NR::Point cp(text->ly.x.computed, text->ly.y.computed);

    for (SPObject *child = sp_object_first_child(SP_OBJECT(text)) ; child != NULL; child = SP_OBJECT_NEXT(child)) {
        if (SP_IS_TSPAN (child)) {
            SPTSpan *tspan = SP_TSPAN (child);
            if (tspan->role == SP_TSPAN_ROLE_LINE) {
                cp[NR::X] = tspan->ly.x.computed;
                cp[NR::Y] = tspan->ly.y.computed;
            }
        }
    }

    /* Create <tspan> */
    SPRepr *rtspan = sp_repr_new ("tspan");
    if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
        /* fixme: real line height */
        /* fixme: What to do with mixed direction tspans? */
        sp_repr_set_double (rtspan, "x", cp[NR::X] - style->font_size.computed);
        sp_repr_set_double (rtspan, "y", cp[NR::Y]);
    } else {
        sp_repr_set_double (rtspan, "x", cp[NR::X]);
        sp_repr_set_double (rtspan, "y", cp[NR::Y] + style->font_size.computed);
    }
    sp_repr_set_attr (rtspan, "sodipodi:role", "line");

    /* Create TEXT */
    SPRepr *rstring = sp_xml_document_createTextNode (sp_repr_document (rtspan), "");
    sp_repr_add_child (rtspan, rstring, NULL);
    sp_repr_unref (rstring);
    /* Append to text */
    sp_repr_append_child (SP_OBJECT_REPR (text), rtspan);
    sp_repr_unref (rtspan);

    return (SPTSpan *) SP_OBJECT_DOCUMENT (text)->getObjectByRepr(rtspan);
}



/**
 *
 */
SPTSpan *
sp_text_insert_line (SPText *text, gint pos)
{
    g_return_val_if_fail (text != NULL, NULL);
    g_return_val_if_fail (SP_IS_TEXT (text), NULL);
    g_return_val_if_fail (pos >= 0, NULL);

    SPObject *child = sp_text_get_child_by_position (text, pos);
    SPString *string = SP_TEXT_CHILD_STRING (child);

    /* Create <tspan> */
    SPRepr *rtspan = sp_repr_new ("tspan");
    sp_repr_set_attr (rtspan, "sodipodi:role", "line");
    /* Create TEXT */
    SPRepr *rstring = sp_xml_document_createTextNode (sp_repr_document (rtspan), "");
    sp_repr_add_child (rtspan, rstring, NULL);
    sp_repr_unref (rstring);

    sp_repr_add_child (SP_OBJECT_REPR (text), rtspan, SP_OBJECT_REPR (child));
    sp_repr_unref (rtspan);

    SPTSpan *newline = (SPTSpan *) SP_OBJECT_DOCUMENT (text)->getObjectByRepr(rtspan);

    if (string->text) {
        gchar *ip = g_utf8_offset_to_pointer (string->text, pos - string->start);
        sp_repr_set_content (rstring, ip);
        *ip = '\0';
        sp_repr_set_content (SP_OBJECT_REPR (string), string->text);

        sp_split_dxdy (child, (SPObject *) newline, (guint) pos - string->start);
    }

    return newline;
}



/**
 *
 */
gint
sp_text_append (SPText *text, const gchar *utf8)
{
    gchar *p;

    g_return_val_if_fail (text != NULL, -1);
    g_return_val_if_fail (SP_IS_TEXT (text), -1);
    g_return_val_if_fail (utf8 != NULL, -1);

    if (!sp_object_first_child(SP_OBJECT(text))) {
        SPRepr *rtspan, *rstring;
        /* Create <tspan> */
        rtspan = sp_repr_new ("tspan");
        /* Create TEXT */
        rstring = sp_xml_document_createTextNode (sp_repr_document (rtspan), "");
        sp_repr_add_child (rtspan, rstring, NULL);
        sp_repr_unref (rstring);
        /* Add string */
        sp_repr_add_child (SP_OBJECT_REPR (text), rtspan, NULL);
        sp_repr_unref (rtspan);
    }

    SPObject *child = sp_object_first_child(SP_OBJECT(text));
    g_assert(child != NULL);

    while (SP_OBJECT_NEXT(child)) {
        child = SP_OBJECT_NEXT(child);
    }
    
    SPString *string;
    if (SP_IS_STRING (child)) {
        string = SP_STRING (child);
    } else {
        string = SP_TSPAN_STRING (child);
    }
        
    const gchar *content = sp_repr_content (SP_OBJECT_REPR (string));

    gint clen = (content) ? strlen (content) : 0;
    gint cchars = (content) ? g_utf8_strlen (content, clen) : 0;

    gint ulen = (utf8) ? strlen (utf8) : 0;
    gint uchars = (utf8) ? g_utf8_strlen (utf8, ulen) : 0;

    if (ulen < 1)
        return cchars;

    p = g_new (gchar, clen + ulen + 1);

    if (clen > 0)
        memcpy (p, content, clen);
    if (ulen > 0)
        memcpy (p + clen, utf8, ulen);
    *(p + clen + ulen) = '\0';

    sp_repr_set_content (SP_OBJECT_REPR (string), p);

    g_free (p);

    return string->start + cchars + uchars;
}



/**
 * Returns position after inserted
 */
gint
sp_text_insert (SPText *text, gint pos, const gchar *utf8)
{
    g_return_val_if_fail (text != NULL, -1);
    g_return_val_if_fail (SP_IS_TEXT (text), -1);
    g_return_val_if_fail (pos >= 0, -1);

    if (!utf8)
        return pos;
    if (!*utf8)
        return pos;

    gunichar u = g_utf8_get_char (utf8);
    if (u == (gunichar) -1) {
        g_warning ("Bad UTF-8 character"); // this can only happen due to a bug, so it goes to the console
        return pos;
    }

    sp_distribute_dxdy (text);

    SPObject *child = sp_text_get_child_by_position (text, pos);
    if (!child) return sp_text_append (text, utf8);
    SPString *string = SP_TEXT_CHILD_STRING (child);

    gchar *ip = g_utf8_offset_to_pointer (string->text, pos - string->start);

    int slen = ip - string->text;
    int ulen = strlen (utf8);

    gchar *spnew = g_new (gchar, strlen (string->text) + ulen + 1);
    /* Copy start */
    memcpy (spnew, string->text, slen);
    /* Copy insertion */
    memcpy (spnew + slen, utf8, ulen);
    /* Copy end */
    strcpy (spnew + slen + ulen, ip);
    sp_repr_set_content (SP_OBJECT_REPR (string), spnew);
    g_free (spnew);

    sp_insert_multiple_dxdy (child, pos - string->start, g_utf8_strlen (utf8, -1), 0, 0);

    return pos + g_utf8_strlen (utf8, -1);
}




/**
 * Returns start position
 */
gint 
sp_text_delete (SPText *text, gint start, gint end)
{
    g_return_val_if_fail (text != NULL, -1);
    g_return_val_if_fail (SP_IS_TEXT (text), -1);
    g_return_val_if_fail (start >= 0, -1);
    g_return_val_if_fail (end >= start, -1);

    if (!sp_object_first_child(SP_OBJECT(text))) return 0;
    if (start == end) return start;

    sp_distribute_dxdy (text);

    SPObject *schild = sp_text_get_child_by_position (text, start);
    SPObject *echild = sp_text_get_child_by_position (text, end);

    if (schild != echild) {
        SPString *sstring, *estring;
        SPObject *child;
        gchar *utf8, *sp, *ep;
        GSList *cl;
        /* Easy case */
        sstring = SP_TEXT_CHILD_STRING (schild);
        estring = SP_TEXT_CHILD_STRING (echild);

        sp = g_utf8_offset_to_pointer (sstring->text, start - sstring->start);
        ep = g_utf8_offset_to_pointer (estring->text, end - estring->start);
        utf8 = g_new (gchar, (sp - sstring->text) + strlen (ep) + 1);
        if (sp > sstring->text) memcpy (utf8, sstring->text, sp - sstring->text);
        memcpy (utf8 + (sp - sstring->text), ep, strlen (ep) + 1);
        sp_repr_set_content (SP_OBJECT_REPR (sstring), utf8);
        g_free (utf8);

        sp_fill_dxdy (schild, start - sstring->start);
        sp_delete_dxdy (echild, 0, end - estring->start);
        sp_append_dxdy (schild, echild);

        /* Delete nodes */
        cl = NULL;
        for (child = schild->next; child != echild; child = child->next) {
            cl = g_slist_prepend (cl, SP_OBJECT_REPR (child));
        }
        cl = g_slist_prepend (cl, SP_OBJECT_REPR (child));
        while (cl) {
            sp_repr_unparent ((SPRepr *) cl->data);
            cl = g_slist_remove (cl, cl->data);
        }
    } else {
        SPString *string;
        gchar *sp, *ep;
        /* Easy case */
        string = SP_TEXT_CHILD_STRING (schild);
        sp = g_utf8_offset_to_pointer (string->text, start - string->start);
        ep = g_utf8_offset_to_pointer (string->text, end - string->start);
        memmove (sp, ep, strlen (ep) + 1);
        sp_repr_set_content (SP_OBJECT_REPR (string), string->text);

        sp_delete_dxdy (schild, start - string->start, end - string->start);
    }

    return start;
}



/**
 * fixme: Should look roles here
 */
gint
sp_text_up (SPText *text, gint pos)
{
    SPObject *child = sp_text_get_child_by_position (text, pos);
    if (!child || child == sp_object_first_child(SP_OBJECT(text)))
        return pos;
    
    SPString *string = SP_TEXT_CHILD_STRING (child);
    gint col = pos - string->start;

    SPObject *up = sp_object_first_child(SP_OBJECT(text));
    while ( SP_OBJECT_NEXT(up) != child ) {
        up = SP_OBJECT_NEXT(up);
	g_assert(up);
    }
    string = SP_TEXT_CHILD_STRING (up);
    col = MIN (col, static_cast< gint > (string->length) );

    return string->start + col;
}



/**
 *
 */
gint
sp_text_down (SPText *text, gint pos)
{
    SPObject *child = sp_text_get_child_by_position (text, pos);
    if (!child || !child->next)
        return pos;
    
    SPString *string = SP_TEXT_CHILD_STRING (child);
    gint col = pos - string->start;
    
    child = child->next;
    
    string = SP_TEXT_CHILD_STRING (child);
    col = MIN (col, static_cast< gint > (string->length) );
    
    return string->start + col;
}



/**
 *
 */
gint
sp_text_start_of_line (SPText *text, gint pos)
{
        SPObject *child = sp_text_get_child_by_position (text, pos);
        if (!child)
        return 0;
        SPString *string = SP_TEXT_CHILD_STRING (child);

        return string->start;
}



/**
 *
 */
gint
sp_text_end_of_line (SPText *text, gint pos)
{
        SPObject *child = sp_text_get_child_by_position (text, pos);
        if (!child) return sp_text_get_length (text);
        SPString *string = SP_TEXT_CHILD_STRING (child);

        return string->start + string->length;
}



/**
 Returns coordinates of the two ends of the cursor line for given position in the given text object
 */
void
sp_text_get_cursor_coords (SPText *text, gint position, NR::Point &p0, NR::Point &p1)
{
    SPObject *child = sp_text_get_child_by_position (text, position);
    SPString *str = SP_TEXT_CHILD_STRING (child);

    NR::Point p;
    if (!str->p) {
        p = NR::Point(str->ly->x.computed, str->ly->y.computed);
    } else {
        p = str->p[position - str->start];
    }

    if (child->style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
        NR::Point dp(child->style->font_size.computed / 2.0, 0);
        p0 = p - dp;
        p1 = p + dp;
    } else {
        // XXX: FIXME: why is this different to above?
        p0 = p - NR::Point(0, child->style->font_size.computed);
        p1 = p;
    }
}



/**
 *
 */
static SPObject *
sp_text_get_child_by_position (SPText *text, gint pos)
{
    SPObject *child;

    for (child = sp_object_first_child(SP_OBJECT(text)) ; child && SP_OBJECT_NEXT(child) ; child = SP_OBJECT_NEXT(child) ) {
        SPString *str;
        if (SP_IS_STRING (child)) {
            str = SP_STRING (child);
        } else {
            str = SP_TSPAN_STRING (child);
        }
        if (pos <= static_cast< gint > (str->start + str->length))
        {
            return child;
        }
    }

    return child;
}

/**
Returns the position of the cursor in the given text obhect closest to the given point
*/
guint
sp_text_get_position_by_coords (SPText *text, NR::Point &p)
{
	guint posmax = sp_text_get_length (text);
	gdouble dmin = 1e18, d;
	guint posmin = 0;

	// Do not try to optimize this by counting first by lines and then by chars in a line;
	// this brute force method is the only one that works for text objects with arbitrary hor/vert kerns
	for (guint pos = 0; pos <= posmax; pos ++) {
		NR::Point p0, p1;

		sp_text_get_cursor_coords (text, pos, p0, p1);
		p1 = (p1 + p0) / 2;
		p1 = p1 * sp_item_i2d_affine(SP_ITEM(text));

		d = NR::LInfty( p1 - p );

		if (d < dmin) {
			posmin = pos;
			dmin = d;
		}
	}

	return posmin;
}


/**
\brief   Adjusts kerning of the pos'th character of text by by visible pixels at the current zoom
*/
void
sp_adjust_kerning_screen (SPText *text, gint pos, SPDesktop *desktop, NR::Point by)
{
    SPObject *child = sp_text_get_child_by_position (text, pos);
    if (!child) return; //FIXME: we should be able to set kerning on non-Inkscape text that has no tspans, too

    SPString *string = SP_TEXT_CHILD_STRING (child);

    guint dx_offset, dy_offset;
    GList *dx = sp_effective_dx (SP_OBJECT(string), &dx_offset);
    GList *dy = sp_effective_dy (SP_OBJECT(string), &dy_offset);

    // obtain value
    gdouble dx_a = sp_char_dx (dx, dx_offset + (pos - string->start));
    gdouble dy_a = sp_char_dy (dy, dy_offset + (pos - string->start));

    // divide increment by zoom 
    // divide increment by matrix expansion 
    gdouble factor = 1 / SP_DESKTOP_ZOOM (desktop);
    NR::Matrix t = sp_item_i2doc_affine (SP_ITEM(child));
    factor = factor / NR::expansion(t);
    by = factor * by;

    dx_a += by[NR::X];
    dy_a += by[NR::Y];

    // set back value
    sp_insert_dxdy (child, pos - string->start, dx_a, dy_a, false);
}



/**
\brief   Adjusts letterspacing in the line of text containing pos so that the length of the line is changed by by visible pixels at the current zoom
*/
void
sp_adjust_tspan_letterspacing_screen (SPText *text, gint pos, SPDesktop *desktop, gdouble by)
{
    gdouble val;

    SPObject *child = sp_text_get_child_by_position (text, pos);
    if (!child) return; //FIXME: we should be able to set lspacing on non-Inkscape text that has no tspans, too

    SPStyle *style = SP_OBJECT_STYLE (child);
    SPString *string = SP_TEXT_CHILD_STRING (child);

    // calculate real value
    if (style->text->letterspacing.value != 0 && style->text->letterspacing.computed == 0) { // set in em or ex
        if (style->text->letterspacing.unit == SP_CSS_UNIT_EM) {
            val = style->font_size.computed * style->text->letterspacing.value;
        } else if (style->text->letterspacing.unit == SP_CSS_UNIT_EX) {
            val = style->font_size.computed * style->text->letterspacing.value * 0.5;
        } else { // unknown unit - should not happen
            val = 0.0;
        }
    } else { // there's a real value in .computed, or it's zero
        val = style->text->letterspacing.computed;
    }

    // divide increment by zoom and by the number of characters in the line,
    // so that the entire line is expanded by by pixels, no matter what its length
    gdouble zoom = SP_DESKTOP_ZOOM (desktop);
    gdouble zby = by / (zoom * (string->length > 1 ? string->length - 1 : 1));

    // divide increment by matrix expansion 
    NR::Matrix t = sp_item_i2doc_affine (SP_ITEM(child));
    zby = zby / NR::expansion(t);

    val += zby;

    // set back value
    if (style->text->letterspacing.value != 0 && style->text->letterspacing.computed == 0) { // set in em or ex
        if (style->text->letterspacing.unit == SP_CSS_UNIT_EM) {
            style->text->letterspacing.value = val / style->font_size.computed;
        } else if (style->text->letterspacing.unit == SP_CSS_UNIT_EX) {
            style->text->letterspacing.value = val / style->font_size.computed * 2;
        } 
    } else { 
        style->text->letterspacing.computed = val;
    }

    style->text->letterspacing.set = TRUE;

    gchar *str = sp_style_write_difference (style, SP_OBJECT_STYLE (SP_OBJECT (text)));
    sp_repr_set_attr (SP_OBJECT_REPR (child), "style", str);
    g_free (str);
}


/**
\brief   Adjusts linespacing in the text object so that the total height is changed by by visible pixels at the current zoom. 
*/
void
sp_adjust_linespacing_screen (SPText *text, SPDesktop *desktop, gdouble by)
{
    SPStyle *style = SP_OBJECT_STYLE (text);

	// the value is stored as multiple of font size (i.e. in em)
	double val = style->font_size.computed * text->ly.linespacing;

	// calculate the number of lines
	SPObject *child;
	int lines = 0;
	for (child = sp_object_first_child(SP_OBJECT(text)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		if (SP_IS_TSPAN (child) && SP_TSPAN (child)->role == SP_TSPAN_ROLE_LINE) {
			lines ++;
		}
	}

    // divide increment by zoom and by the number of lines,
    // so that the entire object is expanded by by pixels
    gdouble zoom = SP_DESKTOP_ZOOM (desktop);
    gdouble zby = by / (zoom * (lines > 1 ? lines - 1 : 1));

    // divide increment by matrix expansion 
    NR::Matrix t = sp_item_i2doc_affine (SP_ITEM(text));
    zby = zby / NR::expansion(t);

    val += zby;

	// fixme: why not allow it to be negative? needs fixing in many places, though
	if (val < 0) val = 0;

	// set back value
	text->ly.linespacing = val / style->font_size.computed;
      SP_OBJECT (text)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
      sp_repr_set_double (SP_OBJECT_REPR (text), "sodipodi:linespacing", text->ly.linespacing);
}




