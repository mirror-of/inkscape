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
#include "xml/attribute-record.h"

#include "sp-shape.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "sp-flowdiv.h"
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

static bool is_line_break_object(SPObject *object)
{
    return    SP_IS_TEXT(object)
           || (SP_IS_TSPAN(object) && SP_TSPAN(object)->role != SP_TSPAN_ROLE_UNSPECIFIED)
           || SP_IS_TEXTPATH(object)
           || SP_IS_FLOWDIV(object)
           || SP_IS_FLOWPARA(object)
           || SP_IS_FLOWLINE(object)
           || SP_IS_FLOWREGIONBREAK(object);
}

static const char * span_name_for_text_object(SPObject *object)
{
    if (SP_IS_TEXT(object)) return "svg:tspan";
    else if (SP_IS_FLOWTEXT(object)) return "svg:flowSpan";
    return NULL;
}

unsigned sp_text_get_length(SPObject *item)
{
    unsigned length = 0;

    if (SP_IS_STRING(item)) return SP_STRING(item)->string.length();
    if (is_line_break_object(item)) length++;
    for (SPObject *child = item->firstChild() ; child ; child = SP_OBJECT_NEXT(child)) {
        if (SP_IS_STRING(child)) length += SP_STRING(child)->string.length();
        else length += sp_text_get_length(child);
    }
    return length;
}

static Inkscape::XML::Node* duplicate_node_without_children(Inkscape::XML::Node const *old_node)
{
    switch (old_node->type()) {
        case Inkscape::XML::ELEMENT_NODE: {
            Inkscape::XML::Node *new_node = sp_repr_new(old_node->name());
            Inkscape::Util::List<Inkscape::XML::AttributeRecord const> attributes = old_node->attributeList();
            GQuark const id_key = g_quark_from_string("id");
            for ( ; attributes ; attributes++) {
                if (attributes->key == id_key) continue;
                new_node->setAttribute(g_quark_to_string(attributes->key), attributes->value);
            }
            return new_node;
        }

        case Inkscape::XML::TEXT_NODE:
            return sp_repr_new_text(old_node->content());

        case Inkscape::XML::COMMENT_NODE:
            return sp_repr_new_comment(old_node->content());

        case Inkscape::XML::DOCUMENT_NODE:
            return NULL;   // this had better never happen
    }
    return NULL;
}

/** recursively divides the XML node tree into two objects: the original will
contain all objects up to and including \a split_obj and the returned value
will be the new leaf which represents the copy of \a split_obj and extends
down the tree with new elements all the way to the common root which is the
parent of the first line break node encountered.
*/
static SPObject* split_text_object_tree_at(SPObject *split_obj)
{
    if (is_line_break_object(split_obj)) {
        Inkscape::XML::Node *new_node = duplicate_node_without_children(SP_OBJECT_REPR(split_obj));
        SP_OBJECT_REPR(SP_OBJECT_PARENT(split_obj))->addChild(new_node, SP_OBJECT_REPR(split_obj));
        sp_repr_unref(new_node);
        return SP_OBJECT_NEXT(split_obj);
    }

    SPObject *duplicate_obj = split_text_object_tree_at(SP_OBJECT_PARENT(split_obj));
    // copy the split node
    if (SP_IS_TSPAN(duplicate_obj) && duplicate_obj->hasChildren()) {
        // workaround for the old code adding a string child we don't want
        SP_OBJECT_REPR(duplicate_obj)->removeChild(SP_OBJECT_REPR(duplicate_obj->firstChild()));
    }
    Inkscape::XML::Node *new_node = duplicate_node_without_children(SP_OBJECT_REPR(split_obj));
    SP_OBJECT_REPR(duplicate_obj)->appendChild(new_node);
    sp_repr_unref(new_node);
    // TODO: I think this an appropriate place to sort out the copied attributes (x/y/dx/dy/rotate)

    // then move all the subsequent nodes
    split_obj = SP_OBJECT_NEXT(split_obj);
    while (split_obj) {
        Inkscape::XML::Node *move_repr = SP_OBJECT_REPR(split_obj);
        SPObject *next_obj = SP_OBJECT_NEXT(split_obj);  // this is about to become invalidated by removeChild()
        sp_repr_ref(move_repr);
        SP_OBJECT_REPR(SP_OBJECT_PARENT(split_obj))->removeChild(move_repr);
        SP_OBJECT_REPR(duplicate_obj)->appendChild(move_repr);
        sp_repr_unref(move_repr);

        split_obj = next_obj;
    }
    return duplicate_obj->firstChild();
}

bool sp_te_insert_line (SPItem *item, gint i_ucs4_pos)
{
    // Disable newlines in a textpath; TODO: maybe on Enter in a textpath, separate it into two
    // texpaths attached to the same path, with a vertical shift
    if (SP_IS_TEXT_TEXTPATH (item)) 
        return 0;

    Inkscape::Text::Layout const *layout = te_get_layout(item);
    SPObject *split_obj;
    Glib::ustring::iterator split_text_iter;
    Inkscape::Text::Layout::iterator it_split = layout->charIndexToIterator(i_ucs4_pos);
    if (it_split == layout->end())
        split_obj = NULL;
    else
        layout->getSourceOfCharacter(it_split, (void**)&split_obj, &split_text_iter);

    if (split_obj == NULL || is_line_break_object(split_obj)) {
        if (split_obj == NULL) split_obj = item->lastChild();
        if (split_obj) {
            Inkscape::XML::Node *new_node = duplicate_node_without_children(SP_OBJECT_REPR(split_obj));
            SP_OBJECT_REPR(SP_OBJECT_PARENT(split_obj))->addChild(new_node, SP_OBJECT_REPR(split_obj));
            sp_repr_unref(new_node);
        }
    } else if (SP_IS_STRING(split_obj)) {
        Glib::ustring *string = &SP_STRING(split_obj)->string;
        // we need to split the entire text tree into two
        SPString *new_string = SP_STRING(split_text_object_tree_at(split_obj));
        SP_OBJECT_REPR(new_string)->setContent(&*split_text_iter.base());   // a little ugly
        string->erase(split_text_iter, string->end());
        SP_OBJECT_REPR(split_obj)->setContent(string->c_str());
        // TODO: if the split point was at the beginning of a span we have a whole load of empty elements to clean up
    } else {
        // TODO
        // I think the only case to put here is arbitrary gaps, which nobody uses yet
    }
    SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    return 1;
}

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
        // the not-so-simple case where we're at a line break or other control char; add to the next child/sibling SPString
        if (cursor_at_start) {
            source_obj = item;
            if (source_obj->hasChildren())
                source_obj = source_obj->firstChild();
        } else
            source_obj = SP_OBJECT_NEXT(source_obj);

        if (source_obj) {  // never fails
            SPString *string_item = sp_te_seek_next_string_recursive(source_obj);
            if (string_item == NULL) {
                // need to add an SPString in this (pathological) case
                Inkscape::XML::Node *rstring = sp_repr_new_text("");
                SP_OBJECT_REPR(source_obj)->addChild(rstring, NULL);
                sp_repr_unref(rstring);
                g_assert(SP_IS_STRING(source_obj->firstChild()));
                string_item = SP_STRING(source_obj->firstChild());
            }
            SP_STRING(string_item)->string.insert(0, utf8);
        }
    }

    SP_OBJECT(item)->updateRepr(SP_OBJECT_REPR(SP_OBJECT(item)),SP_OBJECT_WRITE_EXT);
    SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    return i_ucs4_pos + g_utf8_strlen(utf8, -1);
}

static void move_child_nodes(Inkscape::XML::Node *from_repr, Inkscape::XML::Node *to_repr, bool prepend = false)
{
    while (from_repr->childCount()) {
        Inkscape::XML::Node *child = prepend ? from_repr->lastChild() : from_repr->firstChild();
        sp_repr_ref(child);
        from_repr->removeChild(child);
        if (prepend) to_repr->addChild(child, NULL);
        else to_repr->appendChild(child);
        sp_repr_unref(child);
    }
}

/** if \a object and \a other are the same type and the same style and \a other
isn't needed for layout reasons (ie it's a line break) then all the children
of \a other are removed and put in to object and other is deleted. It works
if \a object and \a other are both SPString objects too. */
static bool merge_nodes_if_possible(SPObject *object, SPObject *other)
{
    if (object == NULL || other == NULL) return false;

    Inkscape::XML::Node *object_repr = SP_OBJECT_REPR(object);
    Inkscape::XML::Node *other_repr = SP_OBJECT_REPR(other);

    if (object_repr->type() != other_repr->type()) return false;

    if (SP_IS_STRING(object) && SP_IS_STRING(other)) {
        // also amalgamate consecutive SPStrings into one
        SP_STRING(object)->string.append(SP_STRING(other)->string);
        other_repr->parent()->removeChild(other_repr);
        return true;
    }

    // merge consecutive spans with identical styles into one
    if (object_repr->type() != Inkscape::XML::ELEMENT_NODE) return false;
    if (strcmp(object_repr->name(), other_repr->name()) != 0) return false;
    if (is_line_break_object(other)) return false;
    gchar const *object_style = object_repr->attribute("style");
    gchar const *other_style = other_repr->attribute("style");
    if (!((object_style == NULL && other_style == NULL)
          || (object_style != NULL && other_style != NULL && !strcmp(object_style, other_style))))
        return false;
    if (SP_IS_TSPAN(other))    // actually in these cases it would be possible to move the attributes over to the preceding element
        if (SP_TSPAN(other)->attributes.anyAttributesSet())
            return false;
    if (SP_IS_TEXT(other))
        if (SP_TEXT(other)->attributes.anyAttributesSet())
            return false;
    if (SP_IS_TEXTPATH(other))
        if (SP_TEXTPATH(other)->attributes.anyAttributesSet())
            return false;
    // all our tests passed: do the merge
    move_child_nodes(other_repr, object_repr);
    other_repr->parent()->removeChild(other_repr);
    return true;
}

/** positions \a para_obj and \a text_iter to be pointing at the end
of the last string in the last leaf object of \a para_obj. If the last
leaf is not an SPString then \a text_iter will be unchanged. */
void move_to_end_of_paragraph(SPObject **para_obj, Glib::ustring::iterator *text_iter)
{
    while ((*para_obj)->hasChildren())
        *para_obj = (*para_obj)->lastChild();
    if (SP_IS_STRING(*para_obj))
        *text_iter = SP_STRING(*para_obj)->string.end();
}

/** delete the line break pointed to \a item by merging its children into
the next suitable object and deleting \a item. Returns the object after the
ones that have just been moved and sets \a next_is_sibling accordingly. */
static SPObject* delete_line_break(SPObject *root, SPObject *item, bool *next_is_sibling)
{
    Inkscape::XML::Node *this_repr = SP_OBJECT_REPR(item);
    SPObject *next_item = NULL;
    bool done = false;
    if (SP_OBJECT_NEXT(item) && !SP_IS_STRING(SP_OBJECT_NEXT(item))) {
        Inkscape::XML::Node *next_repr = SP_OBJECT_REPR(SP_OBJECT_NEXT(item));
        gchar const *this_style = this_repr->attribute("style");
        gchar const *next_style = next_repr->attribute("style");
        if ((this_style == NULL && next_style == NULL) || (this_style != NULL && next_style != NULL && !strcmp(this_style, next_style))) {
            // the two paragraphs are compatible, we can just block-move the children
            next_item = SP_OBJECT_NEXT(item)->firstChild();
            *next_is_sibling = true;
            if (next_item == NULL) {
                next_item = SP_OBJECT_NEXT(item);
                *next_is_sibling = false;
            }
            move_child_nodes(this_repr, next_repr, true);
            done = true;
        }
    }
    if (!done) {
        // no compatible place to move it: create a new span then find somewhere to put it
        /* some sample cases (the div is the item to be deleted, the * represents where to put the new span):
          <div></div><p>*text</p>
          <p><div></div>*text</p>
          <p><div></div></p><p>*text</p>
        */
        Inkscape::XML::Node *new_span_repr = sp_repr_new(span_name_for_text_object(root));

        SPObject *following_item = item;
        while(SP_OBJECT_NEXT(following_item) == NULL) {
            following_item = SP_OBJECT_PARENT(following_item);
            g_assert(following_item != root);
        }
        following_item = SP_OBJECT_NEXT(following_item);

        SPObject *new_parent_item;
        if (SP_IS_STRING(following_item)) {
            new_parent_item = SP_OBJECT_PARENT(following_item);
            SP_OBJECT_REPR(new_parent_item)->addChild(new_span_repr, SP_OBJECT_PREV(following_item) ? SP_OBJECT_REPR(SP_OBJECT_PREV(following_item)) : NULL);
            next_item = following_item;
            *next_is_sibling = true;
        } else {
            new_parent_item = following_item;
            next_item = new_parent_item->firstChild();
            *next_is_sibling = true;
            if (next_item == NULL) {
                next_item = new_parent_item;
                *next_is_sibling = false;
            }
            SP_OBJECT_REPR(new_parent_item)->addChild(new_span_repr, NULL);
        }

        gchar *style = sp_style_write_difference(SP_OBJECT_STYLE(new_parent_item), SP_OBJECT_STYLE(item));
        new_span_repr->setAttribute("style", style);
        g_free(style);
        move_child_nodes(this_repr, new_span_repr);
    }
    while (!item->hasChildren()) {
        SPObject *parent = SP_OBJECT_PARENT(item);
        this_repr->parent()->removeChild(this_repr);
        item = parent;
        this_repr = SP_OBJECT_REPR(item);
    }
    return next_item;
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
    if (is_line_break_object(start_item))
        move_to_end_of_paragraph(&start_item, &start_text_iter);
    if (end_item == NULL) {
        end_item = item->lastChild();
        move_to_end_of_paragraph(&end_item, &end_text_iter);
    }
    else if (is_line_break_object(end_item))
        move_to_end_of_paragraph(&end_item, &end_text_iter);

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
                    SPObject *prev_item = SP_OBJECT_PREV(sub_item);
                    if (merge_nodes_if_possible(prev_item, sub_item))
                        sub_item = prev_item;
                }
                do {
                    sub_item = SP_OBJECT_PARENT(sub_item);
                    if (sub_item == item) break;
                    SPObject *prev_item = SP_OBJECT_PREV(sub_item);
                    if (prev_item) {
                        SPObject *last_child = prev_item->lastChild();
                        if (merge_nodes_if_possible(prev_item, sub_item)) {
                            sub_item = prev_item;
                            if (last_child)
                                merge_nodes_if_possible(last_child, SP_OBJECT_NEXT(last_child));
                        }
                    }
                } while (!is_line_break_object(sub_item));
                break;
            }
            if (SP_IS_STRING(sub_item)) {
                Glib::ustring *string = &SP_STRING(sub_item)->string;
                if (sub_item == start_item)
                    string->erase(start_text_iter, string->end());
                else
                    string->erase();
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

                    Inkscape::XML::Node *this_repr = SP_OBJECT_REPR(sub_item);
                    if (is_line_break_object(sub_item))
                        next_item = delete_line_break(item, sub_item, &is_sibling);

                    else if (SP_IS_STRING(sub_item) && SP_STRING(sub_item)->string.empty())
                        this_repr->parent()->removeChild(this_repr);

                    else if (!merge_nodes_if_possible(SP_OBJECT_PREV(sub_item), sub_item)) {
                        if (!SP_IS_STRING(sub_item) && !sub_item->hasChildren() && !is_line_break_object(sub_item))
                            this_repr->parent()->removeChild(this_repr);
                    }

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
    double height, rotation;
    layout->queryCursorShape(layout->charIndexToIterator(i_position), &p0, &height, &rotation);
    p1 = NR::Point(p0[NR::X] + height * sin(rotation), p0[NR::Y] - height * cos(rotation));
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
