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
#include "unit-constants.h"

#include "xml/repr.h"
#include "xml/attribute-record.h"

#include "sp-shape.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "sp-flowdiv.h"
#include "sp-tspan.h"
#include "sp-string.h"
#include "display/nr-arena-shape.h"

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

static void te_update_layout_now (SPItem *item)
{
    if (SP_IS_TEXT(item))
        SP_TEXT(item)->rebuildLayout();
    else if (SP_IS_FLOWTEXT (item))
        SP_FLOWTEXT(item)->rebuildLayout();
}

bool
sp_te_is_empty (SPItem *item)
{
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    return layout->begin() == layout->end();
}


Inkscape::Text::Layout::iterator
sp_te_get_position_by_coords (SPItem *item, NR::Point &i_p)
{
    NR::Matrix  im=sp_item_i2d_affine (item);
    im = im.inverse();

    NR::Point p = i_p * im;
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    return layout->getNearestCursorPositionTo(p);
}

NRArenaShape* sp_te_create_selection_arena_item(SPItem *item, NRArena *arena, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, NR::Matrix const &transform)
{
    if (start == end)
        return NULL;
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    if (layout == NULL)
        return NULL;

    SPCurve *sel_curve;
    if (start < end)
        sel_curve = layout->createSelectionShape(start, end, transform);
    else
        sel_curve = layout->createSelectionShape(end, start, transform);

    NRRect  paintbox;
    NRBPath bp;
    bp.path = SP_CURVE_BPATH(sel_curve);
    paintbox.x0 = paintbox.y0 = NR_HUGE;
    paintbox.x1 = paintbox.y1 = -NR_HUGE;
    nr_path_matrix_bbox_union(&bp, NR::identity(), &paintbox);

    SPStyle *style = sp_style_new();
    sp_style_merge_from_style_string(style, "fill:blue;fill-opacity:0.5");

    NRArenaShape *arena_shape = NRArenaShape::create(arena);
    nr_arena_shape_set_style (arena_shape, style);
    sp_style_unref(style);   // arena takes a reference
    nr_arena_shape_set_path(arena_shape, sel_curve, false);
    nr_arena_shape_set_paintbox (arena_shape, &paintbox);

    sp_curve_unref(sel_curve);  // arena takes a reference
    return arena_shape;
    // with the return value do:
    //   nr_arena_item_add_child(flowed, arena_shape, NULL);
    //   nr_arena_item_unref(arena_shape);
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

Inkscape::Text::Layout::iterator sp_te_replace(SPItem *item, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, gchar const *utf8)
{
    Inkscape::Text::Layout::iterator new_start = sp_te_delete(item, start, end);
    return sp_te_insert(item, new_start, utf8);
}

static bool is_line_break_object(SPObject const*object)
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

/** returns the sum of the (recursive) lengths of all the SPStrings prior
to \a item at the same level. */
static unsigned sum_sibling_text_lengths_before(SPObject *item)
{
    unsigned char_index = 0;
    for (SPObject *sibling = SP_OBJECT_PARENT(item)->firstChild() ; sibling && sibling != item ; sibling = SP_OBJECT_NEXT(sibling))
        char_index += sp_text_get_length(sibling);
    return char_index;
}

/** splits the attributes for the first object at the given \a char_index
and moves the ones after that point into \a second_item. */
static void split_attributes(SPObject *first_item, SPObject *second_item, unsigned char_index)
{
    TextTagAttributes *first_attrs = NULL, *second_attrs;
    if (SP_IS_TEXT(first_item)) {
        first_attrs = &SP_TEXT(first_item)->attributes;
        second_attrs = &SP_TEXT(second_item)->attributes;
    } else if (SP_IS_TSPAN(first_item)) {
        first_attrs = &SP_TSPAN(first_item)->attributes;
        second_attrs = &SP_TSPAN(second_item)->attributes;
    } else if (SP_IS_TEXTPATH(first_item)) {
        first_attrs = &SP_TEXTPATH(first_item)->attributes;
        second_attrs = &SP_TEXTPATH(second_item)->attributes;
    } else
        return;

    first_attrs->split(char_index, second_attrs);
}

/** recursively divides the XML node tree into two objects: the original will
contain all objects up to and including \a split_obj and the returned value
will be the new leaf which represents the copy of \a split_obj and extends
down the tree with new elements all the way to the common root which is the
parent of the first line break node encountered.
*/
static SPObject* split_text_object_tree_at(SPObject *split_obj, unsigned char_index)
{
    if (is_line_break_object(split_obj)) {
        Inkscape::XML::Node *new_node = duplicate_node_without_children(SP_OBJECT_REPR(split_obj));
        SP_OBJECT_REPR(SP_OBJECT_PARENT(split_obj))->addChild(new_node, SP_OBJECT_REPR(split_obj));
        sp_repr_unref(new_node);
        split_attributes(split_obj, SP_OBJECT_NEXT(split_obj), char_index);
        return SP_OBJECT_NEXT(split_obj);
    }

    unsigned char_count_before = sum_sibling_text_lengths_before(split_obj);
    SPObject *duplicate_obj = split_text_object_tree_at(SP_OBJECT_PARENT(split_obj), char_index + char_count_before);
    // copy the split node
    Inkscape::XML::Node *new_node = duplicate_node_without_children(SP_OBJECT_REPR(split_obj));
    SP_OBJECT_REPR(duplicate_obj)->appendChild(new_node);
    sp_repr_unref(new_node);

    // sort out the copied attributes (x/y/dx/dy/rotate)
    split_attributes(split_obj, duplicate_obj->firstChild(), char_index);

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

Inkscape::Text::Layout::iterator sp_te_insert_line (SPItem *item, Inkscape::Text::Layout::iterator const &position)
{
    // Disable newlines in a textpath; TODO: maybe on Enter in a textpath, separate it into two
    // texpaths attached to the same path, with a vertical shift
    if (SP_IS_TEXT_TEXTPATH (item)) 
        return position;

    Inkscape::Text::Layout const *layout = te_get_layout(item);
    SPObject *split_obj;
    Glib::ustring::iterator split_text_iter;
    if (position == layout->end())
        split_obj = NULL;
    else
        layout->getSourceOfCharacter(position, (void**)&split_obj, &split_text_iter);

    if (split_obj == NULL || is_line_break_object(split_obj)) {
        if (split_obj == NULL) split_obj = item->lastChild();
        if (split_obj) {
            Inkscape::XML::Node *new_node = duplicate_node_without_children(SP_OBJECT_REPR(split_obj));
            SP_OBJECT_REPR(SP_OBJECT_PARENT(split_obj))->addChild(new_node, SP_OBJECT_REPR(split_obj));
            sp_repr_unref(new_node);
        }
    } else if (SP_IS_STRING(split_obj)) {
        Glib::ustring *string = &SP_STRING(split_obj)->string;
        unsigned char_index = 0;
        for (Glib::ustring::iterator it = string->begin() ; it != split_text_iter ; it++)
            char_index++;
        // we need to split the entire text tree into two
        SPString *new_string = SP_STRING(split_text_object_tree_at(split_obj, char_index));
        SP_OBJECT_REPR(new_string)->setContent(&*split_text_iter.base());   // a little ugly
        string->erase(split_text_iter, string->end());
        SP_OBJECT_REPR(split_obj)->setContent(string->c_str());
        // TODO: if the split point was at the beginning of a span we have a whole load of empty elements to clean up
    } else {
        // TODO
        // I think the only case to put here is arbitrary gaps, which nobody uses yet
    }
    item->updateRepr(SP_OBJECT_REPR(item),SP_OBJECT_WRITE_EXT);
    unsigned char_index = layout->iteratorToCharIndex(position);
    te_update_layout_now(item);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    return layout->charIndexToIterator(char_index + 1);
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

/** inserts the given characters into the given string and inserts
corresponding new x/y/dx/dy/rotate attributes into all its parents. */
static void insert_into_spstring(SPString *string_item, Glib::ustring::iterator iter_at, gchar const *utf8)
{
    unsigned char_index = 0;
    unsigned char_count = g_utf8_strlen(utf8, -1);
    Glib::ustring *string = &SP_STRING(string_item)->string;

    for (Glib::ustring::iterator it = string->begin() ; it != iter_at ; it++)
        char_index++;
    string->replace(iter_at, iter_at, utf8);

    SPObject *parent_item = string_item;
    for ( ; ; ) {
        parent_item = SP_OBJECT_PARENT(parent_item);
        TextTagAttributes *attributes;
        if (SP_IS_TEXT(parent_item)) attributes = &SP_TEXT(parent_item)->attributes;
        else if(SP_IS_TSPAN(parent_item)) attributes = &SP_TSPAN(parent_item)->attributes;
        else if(SP_IS_TEXTPATH(parent_item)) attributes = &SP_TEXTPATH(parent_item)->attributes;
        else break;

        attributes->insert(char_index, char_count);
        char_index += sum_sibling_text_lengths_before(parent_item);
    }
}

/**
 * \pre \a utf8[] is valid UTF-8 text.
Returns position after inserted
 */
Inkscape::Text::Layout::iterator
sp_te_insert(SPItem *item, Inkscape::Text::Layout::iterator const &position, gchar const *utf8)
{
    if ( g_utf8_validate(utf8,-1,NULL) != TRUE ) {
        g_warning("Trying to insert invalid utf8");
        return position;
    }

    Inkscape::Text::Layout const *layout = te_get_layout(item);
    SPObject *source_obj;
    Glib::ustring::iterator iter_text;
    // we want to insert after the previous char, not before the current char.
    // it makes a difference at span boundaries
    Inkscape::Text::Layout::iterator it_prev_char = position;
    bool cursor_at_start = !it_prev_char.prevCharacter();
    layout->getSourceOfCharacter(it_prev_char, (void**)&source_obj, &iter_text);
    if (SP_IS_STRING(source_obj)) {
        // the simple case
        if (!cursor_at_start) iter_text++;
        insert_into_spstring(SP_STRING(source_obj), iter_text, utf8);
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
            insert_into_spstring(string_item, string_item->string.begin(), utf8);
        }
    }

    item->updateRepr(SP_OBJECT_REPR(item),SP_OBJECT_WRITE_EXT);
    unsigned char_index = layout->iteratorToCharIndex(position);
    te_update_layout_now(item);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    return layout->charIndexToIterator(char_index + g_utf8_strlen(utf8, -1));
}

/** moves all the children of \a from_repr to \a to_repr, either before
the existing children or after them. Order is maintained. The empty
\a from_repr is not deleted. */
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

/** returns the attributes for an object, or NULL if it isn't a text,
tspan or textpath. */
static TextTagAttributes* attributes_for_object(SPObject *object)
{
    if (SP_IS_TSPAN(object))    // actually in these cases it would be possible to move the attributes over to the preceding element
        return &SP_TSPAN(object)->attributes;
    if (SP_IS_TEXT(object))
        return &SP_TEXT(object)->attributes;
    if (SP_IS_TEXTPATH(object))
        return &SP_TEXTPATH(object)->attributes;
    return NULL;
}

/** returns true if there are any x/y/dx/dy/rotate attributes set on the
given object. */
static bool any_attributes_set(SPObject *object)
{
    TextTagAttributes *attrs = attributes_for_object(object);
    if (attrs) return attrs->anyAttributesSet();
    return false;
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
    if (any_attributes_set(other))
        return false;    // actually in these cases it would be possible to move the attributes over to the preceding element
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
    unsigned moved_char_count = sp_text_get_length(item) - 1;   // the -1 is because it's going to count the line break

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
            TextTagAttributes *attributes_this = attributes_for_object(item);
            TextTagAttributes *attributes_next = attributes_for_object(SP_OBJECT_NEXT(item));
            if (attributes_next) {
                TextTagAttributes attributes_next_copy = *attributes_next;
                attributes_next->join(*attributes_this, attributes_next_copy, moved_char_count);
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

        gchar *style = sp_style_write_difference(SP_OBJECT_STYLE(item), SP_OBJECT_STYLE(new_parent_item));
        new_span_repr->setAttribute("style", style);
        g_free(style);
        TextTagAttributes *attributes = attributes_for_object(new_parent_item);
        if (attributes)
            attributes->insert(0, moved_char_count);
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

/** erases the given characters from the given string and deletes the
corresponding x/y/dx/dy/rotate attributes from all its parents. */
static void erase_from_spstring(SPString *string_item, Glib::ustring::iterator iter_from, Glib::ustring::iterator iter_to)
{
    unsigned char_index = 0;
    unsigned char_count = 0;
    Glib::ustring *string = &SP_STRING(string_item)->string;

    for (Glib::ustring::iterator it = string->begin() ; it != iter_from ; it++)
        char_index++;
    for (Glib::ustring::iterator it = iter_from ; it != iter_to ; it++)
        char_count++;
    string->erase(iter_from, iter_to);

    SPObject *parent_item = string_item;
    for ( ; ; ) {
        parent_item = SP_OBJECT_PARENT(parent_item);
        TextTagAttributes *attributes = attributes_for_object(parent_item);
        if (attributes == NULL) break;

        attributes->erase(char_index, char_count);
        for (SPObject *sibling = SP_OBJECT_PARENT(parent_item)->firstChild() ; sibling && sibling != parent_item ; sibling = SP_OBJECT_NEXT(sibling))
            char_index += sp_text_get_length(sibling);
    }
}

/* Returns start position */
Inkscape::Text::Layout::iterator
sp_te_delete (SPItem *item, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end)
{
    if (start == end) return start;
    Inkscape::Text::Layout::iterator first, last;
    if (start < end) {
        first = start;
        last = end;
    } else {
        first = end;
        last = start;
    }
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    SPObject *start_item, *end_item;
    Glib::ustring::iterator start_text_iter, end_text_iter;
    layout->getSourceOfCharacter(first, (void**)&start_item, &start_text_iter);
    layout->getSourceOfCharacter(last, (void**)&end_item, &end_text_iter);
    if (start_item == NULL)
        return first;   // start is at end of text
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
            erase_from_spstring(SP_STRING(start_item), start_text_iter, end_text_iter);
        }
    } else {
        SPObject *sub_item = start_item;
        // walk the tree from start_item to end_item, deleting as we go
        while (sub_item != item) {
            if (sub_item == end_item) {
                if (SP_IS_STRING(sub_item)) {
                    Glib::ustring *string = &SP_STRING(sub_item)->string;
                    erase_from_spstring(SP_STRING(sub_item), string->begin(), end_text_iter);
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
                SPString *string = SP_STRING(sub_item);
                if (sub_item == start_item)
                    erase_from_spstring(string, start_text_iter, string->string.end());
                else
                    erase_from_spstring(string, string->string.begin(), string->string.end());
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

    item->updateRepr(SP_OBJECT_REPR(item),SP_OBJECT_WRITE_EXT);
    te_update_layout_now(item);
    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    layout->validateIterator(&first);
    return first;
}

void
sp_te_get_cursor_coords (SPItem *item, Inkscape::Text::Layout::iterator const &position, NR::Point &p0, NR::Point &p1)
{
    Inkscape::Text::Layout const *layout = te_get_layout(item);
    double height, rotation;
    layout->queryCursorShape(position, &p0, &height, &rotation);
    p1 = NR::Point(p0[NR::X] + height * sin(rotation), p0[NR::Y] - height * cos(rotation));
}


static void sp_te_get_ustring_multiline(SPObject const *root, Glib::ustring *string)
{
    if (!SP_IS_TEXT(root) && !SP_IS_TEXTPATH(root) && is_line_break_object(root))
        *string += '\n';
    for (SPObject const *child = root->firstChild() ; child ; child = SP_OBJECT_NEXT(child)) {
        if (SP_IS_STRING(child))
            *string += SP_STRING(child)->string;
        else
            sp_te_get_ustring_multiline(child, string);
    }
}

gchar *
sp_te_get_string_multiline (SPItem *text)
{
    Glib::ustring string;

    if (!SP_IS_TEXT(text) && !SP_IS_FLOWTEXT(text)) return NULL;
    sp_te_get_ustring_multiline(text, &string);
    if (string.empty()) return NULL;
    return strdup(string.data() + 1);    // the first char will always be an unwanted line break
}

void
sp_te_set_repr_text_multiline(SPItem *text, gchar const *str)
{
    g_return_if_fail (text != NULL);
    g_return_if_fail (SP_IS_TEXT(text) || SP_IS_FLOWTEXT(text));

    Inkscape::XML::Node *repr;
    bool is_textpath = false;
    if (SP_IS_TEXT_TEXTPATH (text)) {
        repr = SP_OBJECT_REPR (sp_object_first_child(SP_OBJECT (text)));
        is_textpath = true;
    } else {
        repr = SP_OBJECT_REPR (text);
    }

    if (!str) str = "";
    gchar *content = g_strdup (str);

    SP_OBJECT_REPR (text)->setContent("");
    while (repr->firstChild()) {
        repr->removeChild(repr->firstChild());
    }

    gchar *p = content;
    while (p) {
        gchar *e = strchr (p, '\n');
        if (is_textpath) {
            if (e) *e = ' '; // no lines for textpath, replace newlines with spaces
        } else {
            if (e) *e = '\0'; // create a tspan for each line
            Inkscape::XML::Node *rtspan;
            if (SP_IS_TEXT(text)) {
                rtspan = sp_repr_new ("svg:tspan");
                sp_repr_set_attr (rtspan, "sodipodi:role", "line");
            } else
                rtspan = sp_repr_new ("svg:flowDiv");
            Inkscape::XML::Node *rstr = sp_repr_new_text(p);
            sp_repr_add_child (rtspan, rstr, NULL);
            sp_repr_unref(rstr);
            repr->appendChild(rtspan);
            sp_repr_unref(rtspan);
        }
        p = (e) ? e + 1 : NULL;
    }
    if (is_textpath) { 
        Inkscape::XML::Node *rstr = sp_repr_new_text(content);
        sp_repr_add_child (repr, rstr, NULL);
        sp_repr_unref(rstr);
    }

    g_free (content);
}

void
sp_te_adjust_kerning_screen (SPItem *item, Inkscape::Text::Layout::iterator const &position, SPDesktop *desktop, NR::Point by)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_TEXT(item));   // flowtext doesn't support kerning yet
    SPText *text = SP_TEXT(item);

    SPObject *source_item;
    Glib::ustring::iterator source_text_iter;
    text->layout.getSourceOfCharacter(position, (void**)&source_item, &source_text_iter);

    if (!SP_IS_STRING(source_item)) return;
    Glib::ustring *string = &SP_STRING(source_item)->string;
    unsigned char_index = sum_sibling_text_lengths_before(source_item);
    for (Glib::ustring::iterator it = string->begin() ; it != source_text_iter ; it++)
        char_index++;

    // divide increment by zoom
    // divide increment by matrix expansion
    gdouble factor = 1 / SP_DESKTOP_ZOOM(desktop);
    NR::Matrix t = sp_item_i2doc_affine(text);
    factor = factor / NR::expansion(t);
    by = factor * by;

    source_item = SP_OBJECT_PARENT(source_item);
    TextTagAttributes *attributes = attributes_for_object(source_item);
    if (attributes) attributes->addToDxDy(char_index, by);

    text->updateRepr();
    text->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
sp_te_adjust_tspan_letterspacing_screen(SPItem *text, Inkscape::Text::Layout::iterator const &position, SPDesktop *desktop, gdouble by)
{
    g_return_if_fail (text != NULL);
    g_return_if_fail (SP_IS_TEXT(text) || SP_IS_FLOWTEXT(text));

    Inkscape::Text::Layout const *layout = te_get_layout(text);

    gdouble val;
    SPObject *source_obj;
    unsigned nb_let;
    layout->getSourceOfCharacter(position, (void**)&source_obj);
    if (source_obj == NULL) {    // end of text
        source_obj = text->lastChild();
        nb_let = sp_text_get_length(source_obj);
    } else if (SP_IS_STRING(source_obj)) {
        nb_let = SP_STRING(source_obj)->string.length();
        source_obj = source_obj->parent;
    } else {  // line break
        nb_let = sp_text_get_length(source_obj);
    }
    SPStyle *style = SP_OBJECT_STYLE (source_obj);

    // calculate real value
    /* TODO: Consider calculating val unconditionally, i.e. drop the first `if' line, and
       get rid of the `else val = 0.0'.  Similarly below and in sp-string.cpp. */
    if (style->letter_spacing.value != 0 && style->letter_spacing.computed == 0) { // set in em or ex
        if (style->letter_spacing.unit == SP_CSS_UNIT_EM) {
            val = style->font_size.computed * style->letter_spacing.value;
        } else if (style->letter_spacing.unit == SP_CSS_UNIT_EX) {
            val = style->font_size.computed * style->letter_spacing.value * 0.5;
        } else { // unknown unit - should not happen
            val = 0.0;
        }
    } else { // there's a real value in .computed, or it's zero
        val = style->letter_spacing.computed;
    }

    // divide increment by zoom and by the number of characters in the line,
    // so that the entire line is expanded by by pixels, no matter what its length
    gdouble const zoom = SP_DESKTOP_ZOOM(desktop);
    gdouble const zby = (by
                         / (zoom * (nb_let > 1 ? nb_let - 1 : 1))
                         / NR::expansion(sp_item_i2doc_affine(SP_ITEM(source_obj))));
    val += zby;

    // set back value
    style->letter_spacing.normal = FALSE;
    if (style->letter_spacing.value != 0 && style->letter_spacing.computed == 0) { // set in em or ex
        if (style->letter_spacing.unit == SP_CSS_UNIT_EM) {
            style->letter_spacing.value = val / style->font_size.computed;
        } else if (style->letter_spacing.unit == SP_CSS_UNIT_EX) {
            style->letter_spacing.value = val / style->font_size.computed * 2;
        }
    } else {
        style->letter_spacing.computed = val;
    }

    style->letter_spacing.set = TRUE;

    text->updateRepr();
    text->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
}

void
sp_te_adjust_linespacing_screen (SPItem *text, SPDesktop *desktop, gdouble by)
{
    g_return_if_fail (text != NULL);
    g_return_if_fail (SP_IS_TEXT(text) || SP_IS_FLOWTEXT(text));

    Inkscape::Text::Layout const *layout = te_get_layout(text);
    SPStyle *style = SP_OBJECT_STYLE (text);

    if (!style->line_height.set || style->line_height.inherit || style->line_height.normal) {
        style->line_height.set = TRUE;
        style->line_height.inherit = FALSE;
        style->line_height.normal = FALSE;
        style->line_height.unit = SP_CSS_UNIT_PERCENT;
        style->line_height.value = style->line_height.computed = Inkscape::Text::Layout::LINE_HEIGHT_NORMAL;
    }

    unsigned line_count = layout->lineIndex(layout->end());
    double all_lines_height = layout->characterAnchorPoint(layout->end())[NR::Y] - layout->characterAnchorPoint(layout->begin())[NR::Y];
    double average_line_height = all_lines_height / (line_count == 0 ? 1 : line_count);
    if (fabs(average_line_height) < 0.001) average_line_height = 0.001;

    // divide increment by zoom and by the number of lines,
    // so that the entire object is expanded by by pixels
    gdouble zby = by / (SP_DESKTOP_ZOOM (desktop) * (line_count == 0 ? 1 : line_count));

    // divide increment by matrix expansion
    NR::Matrix t = sp_item_i2doc_affine (SP_ITEM(text));
    zby = zby / NR::expansion(t);

    switch (style->line_height.unit) {
        case SP_CSS_UNIT_NONE:
        default:
            // multiplier-type units, stored in computed
            if (fabs(style->line_height.computed) < 0.001) style->line_height.computed = by < 0.0 ? -0.001 : 0.001;    // the formula below could get stuck at zero
            else style->line_height.computed *= (average_line_height + zby) / average_line_height;
            style->line_height.value = style->line_height.computed;
            break;
        case SP_CSS_UNIT_EM:
        case SP_CSS_UNIT_EX:
        case SP_CSS_UNIT_PERCENT:
            // multiplier-type units, stored in value
            if (fabs(style->line_height.value) < 0.001) style->line_height.value = by < 0.0 ? -0.001 : 0.001;
            else style->line_height.value *= (average_line_height + zby) / average_line_height;
            break;
            // absolute-type units
	    case SP_CSS_UNIT_PX:
            style->line_height.computed += zby;
            style->line_height.value = style->line_height.computed;
            break;
	    case SP_CSS_UNIT_PT:
            style->line_height.computed += zby * PT_PER_PX;
            style->line_height.value = style->line_height.computed;
            break;
	    case SP_CSS_UNIT_PC:
            style->line_height.computed += zby * (PT_PER_PX / 12);
            style->line_height.value = style->line_height.computed;
            break;
	    case SP_CSS_UNIT_MM:
            style->line_height.computed += zby * MM_PER_PX;
            style->line_height.value = style->line_height.computed;
            break;
	    case SP_CSS_UNIT_CM:
            style->line_height.computed += zby * CM_PER_PX;
            style->line_height.value = style->line_height.computed;
            break;
	    case SP_CSS_UNIT_IN:
            style->line_height.computed += zby * IN_PER_PX;
            style->line_height.value = style->line_height.computed;
            break;
    }
    text->updateRepr();
    text->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
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
