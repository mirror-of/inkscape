/*
 * Parent class for text and flowtext
 *
 * Author:
 *   bulia byak
 *
 * Copyright (C) 2004 author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <string.h>

#include <libnr/nr-rect.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-rotate.h>
//#include <libnrtype/nr-typeface.h>
#include <libnrtype/FontFactory.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/font-style-to-pos.h>
#include <libnrtype/FlowDefs.h>

#include <libnrtype/FlowRes.h>
#include <libnrtype/FlowSrc.h>
#include <libnrtype/FlowEater.h>
#include <libnrtype/FlowStyle.h>
#include <libnrtype/FlowBoxes.h>

#include <libnrtype/TextWrapper.h>

#include <livarot/LivarotDefs.h>
#include <livarot/Shape.h>
#include <livarot/Path.h>

#include <glib.h>
//#include <gtk/gtk.h>

#include <glibmm/i18n.h>
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "display/curve.h"
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

#include "xml/repr.h"

#include "sp-shape.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "sp-tspan.h"
#include "sp-string.h"

#include "text-editing.h"

Inkscape::Text::Layout const * te_get_layout (SPItem *item)
{
    if (SP_IS_TEXT(item)) {
        return &(SP_TEXT(item)->layout);
    } else if (SP_IS_FLOWTEXT (item)) {
        return &(SP_FLOWTEXT(item)->layout);
    }
    return NULL;
}

bool
sp_te_is_empty (SPItem *item)
{
    int tlen = sp_te_get_length (item);
    if ( tlen > 0 ) return false;
    return true;
}


/* This gives us SUM (strlen (STRING)) + (LINES - 1) */
gint
sp_te_get_length (SPItem *item)
{  //RH: is it OK to rely on the output having been built?
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    if (layout) return layout->iteratorToCharIndex(layout->end());
    return 0;
}

gint
sp_te_up (SPItem *item, gint i_position)
{
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    //RH: we must store the iterator itself, not the position because as it is this
    //    code loses the x-coordinate for repeated up/down movement
    Inkscape::Text::Layout::iterator it = layout->charIndexToIterator(i_position);
    it.cursorUp();
    return layout->iteratorToCharIndex(it);
}

gint
sp_te_down (SPItem *item, gint i_position)
{
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    //RH: we must store the iterator itself, not the position because as it is this
    //    code loses the x-coordinate for repeated up/down movement
    Inkscape::Text::Layout::iterator it = layout->charIndexToIterator(i_position);
    it.cursorDown();
    return layout->iteratorToCharIndex(it);
}

gint
sp_te_start_of_line (SPItem *item, gint i_position)
{
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    Inkscape::Text::Layout::iterator it = layout->charIndexToIterator(i_position);
    it.thisStartOfLine();
    return layout->iteratorToCharIndex(it);
}

gint
sp_te_end_of_line (SPItem *item, gint i_position)
{
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    Inkscape::Text::Layout::iterator it = layout->charIndexToIterator(i_position);
    if (it.nextStartOfLine())
        it.prevCursorPosition();
    return layout->iteratorToCharIndex(it);
}

guint
sp_te_get_position_by_coords (SPItem *item, NR::Point &i_p)
{
    NR::Matrix  im=sp_item_i2d_affine (item);
    im = im.inverse();

    NR::Point p = i_p * im;
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    Inkscape::Text::Layout::iterator it = layout->getNearestCursorPositionTo(p);
    return layout->iteratorToCharIndex(it);
}

/*
 * for debugging input
 *
char * dump_hexy(const gchar * utf8)
{
    static char buffer[1024];

    buffer[0]='\0';
    for (const char *ptr=utf8; *ptr; ptr++) {
        sprintf(buffer+strlen(buffer),"x%02X",(unsigned char)*ptr);
    }
    return buffer;
}
*/

/** finds the first SPString after the given position, including children, excluding parents */
static SPString* sp_te_seek_next_string_recursive(SPObject *start_obj)
{
    while (start_obj) {
        if (start_obj->hasChildren()) {
            SPString *found_string = sp_te_seek_next_string_recursive(start_obj->firstChild());
            if (found_string) return found_string;
        }
        if (SP_IS_STRING(start_obj)) return SP_STRING(start_obj);
        start_obj = SP_OBJECT_NEXT(start_obj);
        if (SP_IS_TSPAN(start_obj) && SP_TSPAN(start_obj)->role != SP_TSPAN_ROLE_UNSPECIFIED)
            break;   // don't cross line breaks
    }
    return NULL;
}

/**
 * \pre \a utf8[] is valid UTF-8 text.
Returns position after inserted
 */
gint
sp_te_insert(SPItem *item, gint i_ucs4_pos, gchar const *utf8)
{
    if ( g_utf8_validate(utf8,-1,NULL) != TRUE ) {
        g_warning("Trying to insert invalid utf8");
        return i_ucs4_pos;
    }

    Inkscape::Text::Layout const *layout = te_get_layout(item);
    Inkscape::Text::Layout::iterator it = layout->charIndexToIterator(i_ucs4_pos);
    SPObject *source_obj;
    Glib::ustring::iterator iter_text;
    // we want to insert after the previous char, not before the current char.
    // it makes a difference at span boundaries
    bool cursor_at_start = !it.prevCharacter();
    layout->getSourceOfCharacter(it, (void**)&source_obj, &iter_text);
    if (SP_IS_STRING(source_obj)) {
        // the simple case
        Glib::ustring *string = &SP_STRING(source_obj)->string;
        if (!cursor_at_start) iter_text++;
        string->replace(iter_text, iter_text, utf8);
    } else {
        // the not-so-simple case where we're at a line break or other control char; add to the last child/sibling SPString
        if (cursor_at_start) {
            source_obj = item;
            if (source_obj->hasChildren())
                source_obj = source_obj->firstChild();
        }
        SPString *string_item = sp_te_seek_next_string_recursive(source_obj);
        if (string_item == NULL) {
            // need to add one in this (pathological) case
            Inkscape::XML::Node *rstring = sp_repr_new_text("");
            SP_OBJECT_REPR(source_obj)->addChild(rstring, NULL);        // this magically adds to the spobject tree too
            sp_repr_unref(rstring);
            g_assert(SP_IS_STRING(source_obj->firstChild()));
            string_item = SP_STRING(source_obj->firstChild());
        }
        SP_STRING(string_item)->string.insert(0, utf8);
    }

    SP_OBJECT(item)->updateRepr(SP_OBJECT_REPR(SP_OBJECT(item)),SP_OBJECT_WRITE_EXT);
    SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    return i_ucs4_pos + g_utf8_strlen(utf8, -1);
}

/* Returns start position */
gint
sp_te_delete (SPItem *item, gint i_start, gint i_end)
{
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    Inkscape::Text::Layout::iterator it_start = layout->charIndexToIterator(i_start);
    Inkscape::Text::Layout::iterator it_end = layout->charIndexToIterator(i_end);
    SPObject *start_item, *end_item;
    Glib::ustring::iterator start_text_iter, end_text_iter;
    layout->getSourceOfCharacter(it_start, (void**)&start_item, &start_text_iter);
    layout->getSourceOfCharacter(it_end, (void**)&end_item, &end_text_iter);

    if (start_item == end_item) {
        // the quick case where we're deleting stuff all from the same string
        if (SP_IS_STRING(start_item)) {     // always true (if it_start != it_end anyway)
            SP_STRING(start_item)->string.erase(start_text_iter, end_text_iter);
        }
    } else {
        SPObject *sub_item = start_item;
        // walk the tree from start_item to end_item, deleting as we go
        while (sub_item != item) {
            if (sub_item == end_item) {
                if (SP_IS_STRING(sub_item)) {
                    Glib::ustring *string = &SP_STRING(sub_item)->string;
                    string->erase(string->begin(), end_text_iter);
                    if (SP_OBJECT_PREV(sub_item) && SP_IS_STRING(SP_OBJECT_PREV(sub_item))) {
                        // consecutive SPString amalgamation below only works on leaving the second string.
                        // the last string doesn't leave in the same way, so amalgamate here
                        SP_STRING(SP_OBJECT_PREV(sub_item))->string.append(*string);
                        SP_OBJECT_REPR(SP_OBJECT_PARENT(sub_item))->removeChild(SP_OBJECT_REPR(sub_item));
                    }
                }
                break;
            }
            if (SP_IS_STRING(sub_item)) {
                Glib::ustring *string = &SP_STRING(sub_item)->string;
                if (sub_item == start_item)
                    string->erase(start_text_iter, string->end());
                else
                    string->erase();   // the SPObject itself will be deleted below
            }
            if (SP_IS_TSPAN(sub_item) && SP_TSPAN(sub_item)->role != SP_TSPAN_ROLE_UNSPECIFIED) {
                Inkscape::XML::Node *tspan_repr = SP_OBJECT_REPR(sub_item);
                tspan_repr->setAttribute("sodipodi:role", NULL);
                tspan_repr->setAttribute("x", NULL);
                tspan_repr->setAttribute("y", NULL);
                // the actual merging of two tspans will be done below (if they're the same style) (TODO)
            }
            // walk to the next item in the tree
            if (sub_item->hasChildren())
                sub_item = sub_item->firstChild();
            else {
                SPObject *next_item;
                do {
                    bool is_sibling = true;
                    next_item = SP_OBJECT_NEXT(sub_item);
                    if (next_item == NULL) {
                        next_item = SP_OBJECT_PARENT(sub_item);
                        is_sibling = false;
                    }

                    // delete empty objects we're leaving
                    if (SP_IS_STRING(sub_item)) {
                        if (SP_STRING(sub_item)->string.empty())
                            SP_OBJECT_REPR(SP_OBJECT_PARENT(sub_item))->removeChild(SP_OBJECT_REPR(sub_item));
                        else if (SP_OBJECT_PREV(sub_item) && SP_IS_STRING(SP_OBJECT_PREV(sub_item))) {
                            // also amalgamate consecutive SPStrings into one
                            SP_STRING(SP_OBJECT_PREV(sub_item))->string.append(SP_STRING(sub_item)->string);
                            SP_OBJECT_REPR(SP_OBJECT_PARENT(sub_item))->removeChild(SP_OBJECT_REPR(sub_item));
                        }
                    } else if (!sub_item->hasChildren()) {
                        if (!SP_IS_TSPAN(sub_item) || SP_TSPAN(sub_item)->role == SP_TSPAN_ROLE_UNSPECIFIED)
                            SP_OBJECT_REPR(SP_OBJECT_PARENT(sub_item))->removeChild(SP_OBJECT_REPR(sub_item));
                    }
                    // TODO: merging of consecutive spans with identical styles into one

                    sub_item = next_item;
                    if (is_sibling) break;
                    // no more siblings, go up a parent
                } while (sub_item != item && sub_item != end_item);
            }
        }
    }

    SP_OBJECT(item)->updateRepr(SP_OBJECT_REPR(SP_OBJECT(item)),SP_OBJECT_WRITE_EXT);
    SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    return i_start;
}

void
sp_te_get_cursor_coords (SPItem *item, gint i_position, NR::Point &p0, NR::Point &p1)
{
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    if (!layout->outputExists()) {
        if (SP_IS_TEXT(item)) {
            p0[0] = SP_TEXT(item)->x.computed;
            p0[1] = SP_TEXT(item)->y.computed;
        } else if (SP_IS_FLOWTEXT(item)) {
            p0[0] = 0.0;  // fixme
            p0[1] = 0.0;
        }
        p1 = p0;
        p1[1] -= item->style->font_size.computed;
        return;
    }
    double height, rotation;
    layout->queryCursorShape(layout->charIndexToIterator(i_position), &p0, &height, &rotation);
    p1 = NR::Point(p0[0] + height * sin(rotation), p0[1] - height * cos(rotation));
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
