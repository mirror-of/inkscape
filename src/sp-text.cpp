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
//#include <libnrtype/nr-typeface.h>
#include <libnrtype/FontFactory.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/font-style-to-pos.h>

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
#include "fontsize-expansion.h"
#include "style.h"
#include "version.h"
#include "inkscape.h"
#include "view.h"
#include "print.h"
#include "sp-metrics.h"
#include "xml/repr.h"

#include "sp-item.h"
#include "sp-text.h"
#include "sp-tspan.h"
#include "sp-string.h"

#include "text-editing.h"

//#include "sp-use-reference.h"
//#include "prefs-utils.h"


/*#####################################################
#  SPTEXT
#####################################################*/

static void sp_text_class_init (SPTextClass *classname);
static void sp_text_init (SPText *text);
static void sp_text_release (SPObject *object);

static void sp_text_build (SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_text_set (SPObject *object, unsigned key, gchar const *value);
static void sp_text_child_added (SPObject *object, Inkscape::XML::Node *rch, Inkscape::XML::Node *ref);
static void sp_text_remove_child (SPObject *object, Inkscape::XML::Node *rch);
static void sp_text_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_text_modified (SPObject *object, guint flags);
static Inkscape::XML::Node *sp_text_write (SPObject *object, Inkscape::XML::Node *repr, guint flags);

static void sp_text_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags);
static NRArenaItem *sp_text_show (SPItem *item, NRArena *arena, unsigned key, unsigned flags);
static void sp_text_hide (SPItem *item, unsigned key);
static char *sp_text_description (SPItem *item);
static void sp_text_snappoints(SPItem const *item, SnapPointsIter p);
static NR::Matrix sp_text_set_transform(SPItem *item, NR::Matrix const &xform);
static void sp_text_print (SPItem *item, SPPrintContext *gpc);

static SPItemClass *text_parent_class;

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

static void
sp_text_class_init (SPTextClass *classname)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) classname;
    SPItemClass *item_class = (SPItemClass *) classname;

    text_parent_class = (SPItemClass*)g_type_class_ref (SP_TYPE_ITEM);

    sp_object_class->release = sp_text_release;
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

static void
sp_text_init (SPText *text)
{
    new (&text->layout) Inkscape::Text::Layout;
    new (&text->attributes) TextTagAttributes;
}

static void
sp_text_release (SPObject *object)
{
    SPText *text = SP_TEXT(object);
    text->attributes.~TextTagAttributes();
    text->layout.~Layout();

    if (((SPObjectClass *) text_parent_class)->release)
        ((SPObjectClass *) text_parent_class)->release(object);
}

static void
sp_text_build (SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr)
{
    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "dx");
    sp_object_read_attr(object, "dy");
    sp_object_read_attr(object, "rotate");

    if (((SPObjectClass *) text_parent_class)->build)
        ((SPObjectClass *) text_parent_class)->build(object, doc, repr);

    sp_object_read_attr(object, "sodipodi:linespacing");    // has to happen after the styles are read
}

static void
sp_text_set(SPObject *object, unsigned key, gchar const *value)
{
    SPText *text = SP_TEXT (object);

    if (text->attributes.readSingleAttribute(key, value)) {
        object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    } else {
        switch (key) {
            case SP_ATTR_SODIPODI_LINESPACING:
                // convert deprecated tag to css
                if (value) {
                    text->style->line_height.set = TRUE;
                    text->style->line_height.inherit = FALSE;
                    text->style->line_height.normal = FALSE;
                    text->style->line_height.unit = SP_CSS_UNIT_PERCENT;
                    text->style->line_height.value = text->style->line_height.computed = sp_svg_read_percentage (value, 1.0);
                }
                object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
                break;
            default:
                if (((SPObjectClass *) text_parent_class)->set)
                    ((SPObjectClass *) text_parent_class)->set (object, key, value);
                break;
        }
    }
}

static void
sp_text_child_added (SPObject *object, Inkscape::XML::Node *rch, Inkscape::XML::Node *ref)
{
    SPText *text = SP_TEXT (object);

    if (((SPObjectClass *) text_parent_class)->child_added)
        ((SPObjectClass *) text_parent_class)->child_added (object, rch, ref);

    text->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_CONTENT_MODIFIED_FLAG);
}

static void
sp_text_remove_child (SPObject *object, Inkscape::XML::Node *rch)
{
    SPText *text = SP_TEXT (object);

    if (((SPObjectClass *) text_parent_class)->remove_child)
        ((SPObjectClass *) text_parent_class)->remove_child (object, rch);

    text->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_CONTENT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
}

static void
sp_text_update (SPObject *object, SPCtx *ctx, guint flags)
{
    SPText *text = SP_TEXT (object);

    if (((SPObjectClass *) text_parent_class)->update)
        ((SPObjectClass *) text_parent_class)->update (object, ctx, flags);

    guint cflags = (flags & SP_OBJECT_MODIFIED_CASCADE);
    if (flags & SP_OBJECT_MODIFIED_FLAG) cflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;


    /* Create temporary list of children */
    GSList *l = NULL;
    for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        sp_object_ref (SP_OBJECT (child), object);
        l = g_slist_prepend (l, child);
    }
    l = g_slist_reverse (l);
    while (l) {
        SPObject *child = SP_OBJECT (l->data);
        l = g_slist_remove (l, child);
        if (cflags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            /* fixme: Do we need transform? */
            child->updateDisplay(ctx, cflags);
        }
        sp_object_unref (SP_OBJECT (child), object);
    }
    if (flags & ( SP_OBJECT_STYLE_MODIFIED_FLAG |
                  SP_OBJECT_CHILD_MODIFIED_FLAG |
                  SP_TEXT_LAYOUT_MODIFIED_FLAG   ) )
    {
        /* fixme: It is not nice to have it here, but otherwise children content changes does not work */
        /* fixme: Even now it may not work, as we are delayed */
        /* fixme: So check modification flag everywhere immediate state is used */
        text->rebuildLayout();

        NRRect paintbox;
        sp_item_invoke_bbox(text, &paintbox, NR::identity(), TRUE);
        for (SPItemView* v = text->display; v != NULL; v = v->next) {
            text->_clearFlow(NR_ARENA_GROUP(v->arenaitem));
            // pass the bbox of the text object as paintbox (used for paintserver fills)
            text->layout.show(NR_ARENA_GROUP(v->arenaitem), &paintbox);
        }
    }
}

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

static Inkscape::XML::Node *
sp_text_write (SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPText *text = SP_TEXT (object);

    if (flags & SP_OBJECT_WRITE_BUILD) {
        if (!repr)
            repr = sp_repr_new ("svg:text");
        GSList *l = NULL;
        for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            Inkscape::XML::Node *crepr = NULL;
            if (SP_IS_TSPAN (child)) {
                crepr = child->updateRepr(NULL, flags);
            } else if (SP_IS_TEXTPATH (child)) {
                crepr = child->updateRepr(NULL, flags);
            } else {
                crepr = sp_repr_new_text(SP_STRING(child)->string.c_str());
            }
            if (crepr) l = g_slist_prepend (l, crepr);
        }
        while (l) {
            sp_repr_add_child (repr, (Inkscape::XML::Node *) l->data, NULL);
            sp_repr_unref ((Inkscape::XML::Node *) l->data);
            l = g_slist_remove (l, l->data);
        }
    } else {
        for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            if (SP_IS_TSPAN (child)) {
                child->updateRepr(flags);
            } else if (SP_IS_TEXTPATH (child)) {
                child->updateRepr(flags);
            } else {
                SP_OBJECT_REPR (child)->setContent(SP_STRING(child)->string.c_str());
            }
        }
    }

    text->attributes.writeTo(repr);

    // deprecated attribute, but keep it around for backwards compatibility
    if (text->style->line_height.set && !text->style->line_height.inherit && !text->style->line_height.normal && text->style->line_height.unit == SP_CSS_UNIT_PERCENT) {
	    Inkscape::SVGOStringStream os;
        os << (text->style->line_height.value * 100.0) << "%";
        SP_OBJECT_REPR(text)->setAttribute("sodipodi:linespacing", os.str().c_str());
    }
    else
        SP_OBJECT_REPR(text)->setAttribute("sodipodi:linespacing", NULL);

    if (((SPObjectClass *) (text_parent_class))->write)
        ((SPObjectClass *) (text_parent_class))->write (object, repr, flags);

    return repr;
}

static void
sp_text_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const /*flags*/)
{
    SP_TEXT(item)->layout.getBoundingBox(bbox, transform);
}


static NRArenaItem *
sp_text_show(SPItem *item, NRArena *arena, unsigned /* key*/, unsigned /*flags*/)
{
    SPText *group = (SPText *) item;

    NRArenaGroup *flowed = NRArenaGroup::create(arena);
    nr_arena_group_set_transparent (flowed, FALSE);

    // pass the bbox of the text object as paintbox (used for paintserver fills)
    NRRect paintbox;
    sp_item_invoke_bbox(item, &paintbox, NR::identity(), TRUE);
    group->layout.show(flowed, &paintbox);

    return flowed;
}

static void
sp_text_hide(SPItem *item, unsigned key)
{
    if (((SPItemClass *) text_parent_class)->hide)
        ((SPItemClass *) text_parent_class)->hide (item, key);
}

static char *
sp_text_description(SPItem *item)
{
    SPText *text = (SPText *) item;
    SPStyle *style = SP_OBJECT_STYLE(text);

    font_instance *tf = (font_factory::Default())->Face(style->text->font_family.value,
                                                        font_style_to_pos(*style));
    char name_buf[256];
    char const *n;
    if (tf) {
        tf->Name(name_buf, sizeof(name_buf));
        n = name_buf;
        tf->Unref();
    } else {
        n = _("<no name found>");
    }

    GString *xs = SP_PX_TO_METRIC_STRING(style->font_size.computed, sp_desktop_get_default_metric(SP_ACTIVE_DESKTOP));

    if (SP_IS_TEXT_TEXTPATH(item)) {
        return g_strdup_printf (_("<b>Text on path</b> (%s, %s)"), n, xs->str);
    } else {
        return g_strdup_printf (_("<b>Text</b> (%s, %s)"), n, xs->str);
    }
}

static void sp_text_snappoints(SPItem const *item, SnapPointsIter p)
{
    if (((SPItemClass *) text_parent_class)->snappoints) {
        ((SPItemClass *) text_parent_class)->snappoints (item, p);
    }
}

static NR::Matrix
sp_text_set_transform (SPItem *item, NR::Matrix const &xform)
{
    SPText *text = SP_TEXT(item);

    // we cannot optimize textpath because changing its fontsize will break its match to the path
    if (SP_IS_TEXT_TEXTPATH (text))
        return xform;

    /* This function takes care of scaling & translation only, we return whatever parts we can't
       handle. */

// TODO: pjrm tried to use fontsize_expansion(xform) here and it works for text in that font size
// is scaled more intuitively when scaling non-uniformly; however this necessitated using
// fontsize_expansion instead of expansion in other places too, where it was not appropriate
// (e.g. it broke stroke width on copy/pasting of style from horizontally stretched to vertically
// stretched shape). Using fontsize_expansion only here broke setting the style via font
// dialog. This needs to be investigated further.
    double const ex = NR::expansion(xform); 
    if (ex == 0) {
        return xform;
    }

    NR::Matrix ret(NR::transform(xform));
    ret[0] /= ex;
    ret[1] /= ex;
    ret[2] /= ex;
    ret[3] /= ex;

    // Adjust x/y, dx/dy
    text->_adjustCoordsRecursive (item, xform * ret.inverse(), ex);

    // Adjust font size
    text->_adjustFontsizeRecursive (item, ex);

    // Adjust stroke width
    sp_item_adjust_stroke(item, ex);

    // Adjust pattern fill
    sp_item_adjust_pattern(item, xform * ret.inverse());

    // Adjust gradient fill
    sp_item_adjust_gradient(item, xform * ret.inverse());

    item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);

    return ret;
}

static void
sp_text_print (SPItem *item, SPPrintContext *ctx)
{
    NRRect     pbox, dbox, bbox;
    SPText *group = SP_TEXT (item);

    sp_item_invoke_bbox(item, &pbox, NR::identity(), TRUE);
    sp_item_bbox_desktop (item, &bbox);
    dbox.x0 = 0.0;
    dbox.y0 = 0.0;
    dbox.x1 = sp_document_width (SP_OBJECT_DOCUMENT (item));
    dbox.y1 = sp_document_height (SP_OBJECT_DOCUMENT (item));
    NRMatrix ctm;
    sp_item_i2d_affine (item, &ctm);

    group->layout.print(ctx,&pbox,&dbox,&bbox,ctm);
}

/*
 * Member functions
 */

unsigned SPText::_buildLayoutInput(SPObject *root, Inkscape::Text::Layout::OptionalTextTagAttrs const &parent_optional_attrs, unsigned parent_attrs_offset, bool in_textpath)
{
    unsigned length = 0;
    int child_attrs_offset = 0;
    Inkscape::Text::Layout::OptionalTextTagAttrs optional_attrs;

    if (SP_IS_TEXT(root)) {
        SP_TEXT(root)->attributes.mergeInto(&optional_attrs, parent_optional_attrs, parent_attrs_offset, true, true);
    }
    else if (SP_IS_TSPAN(root)) {
        SPTSpan *tspan = SP_TSPAN(root);
        bool use_xy = !in_textpath && (tspan->role == SP_TSPAN_ROLE_UNSPECIFIED || !tspan->attributes.singleXYCoordinates());
        tspan->attributes.mergeInto(&optional_attrs, parent_optional_attrs, parent_attrs_offset, use_xy, true);
    }
    else if (SP_IS_TEXTPATH(root)) {
        in_textpath = true;
        SP_TEXTPATH(root)->attributes.mergeInto(&optional_attrs, parent_optional_attrs, parent_attrs_offset, false, true);
        optional_attrs.x.clear();
        optional_attrs.y.clear();
    }
    else {
        optional_attrs = parent_optional_attrs;
        child_attrs_offset = parent_attrs_offset;
    }

    if (SP_IS_TSPAN(root))
        if (SP_TSPAN(root)->role != SP_TSPAN_ROLE_UNSPECIFIED) {
            // we need to allow the first line not to have role=line, but still set the source_cookie to the right value
            SPObject *prev_object = SP_OBJECT_PREV(root);
            if (prev_object && SP_IS_TSPAN(prev_object)) {
                if (!layout.inputExists())
                    layout.appendText("", prev_object->style, prev_object, &optional_attrs);
                layout.appendControlCode(Inkscape::Text::Layout::PARAGRAPH_BREAK, prev_object);
            }
            if (!root->hasChildren())
                layout.appendText("", root->style, root, &optional_attrs);
            length++;     // interpreting line breaks as a character for the purposes of x/y/etc attributes
                          // is a liberal interpretation of the svg spec, but a strict reading would mean
                          // that if the first line is empty the second line would take its place at the
                          // start position. Very confusing.
            child_attrs_offset--;
        }

    for (SPObject *child = sp_object_first_child(root) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_TSPAN (child) || SP_IS_TEXTPATH (child)) {
            length += _buildLayoutInput(child, optional_attrs, child_attrs_offset + length, in_textpath);
        } else if (SP_IS_STRING (child)) {
            Glib::ustring const &string = SP_STRING(child)->string;
            layout.appendText(string, root->style, child, &optional_attrs, child_attrs_offset + length);
            length += string.length();
        }
    }

    return length;
}

void SPText::rebuildLayout()
{
    layout.clear();
    Inkscape::Text::Layout::OptionalTextTagAttrs optional_attrs;
    _buildLayoutInput(this, optional_attrs, 0, false);
    layout.calculateFlow();
    for (SPObject *child = firstChild() ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_TEXTPATH(child)) {
            SPTextPath const *textpath = SP_TEXTPATH(child);
            if (textpath->originalPath != NULL) {
                g_print(layout.dumpAsText().c_str());
                layout.fitToPathAlign(textpath->startOffset, *textpath->originalPath);
            }
        }
    }
    g_print(layout.dumpAsText().c_str());

    // set the x,y attributes on role:line spans
    for (SPObject *child = firstChild() ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if (!SP_IS_TSPAN(child)) continue;
        SPTSpan *tspan = SP_TSPAN(child);
        if (tspan->role == SP_TSPAN_ROLE_UNSPECIFIED) continue;
        if (!tspan->attributes.singleXYCoordinates()) continue;
        Inkscape::Text::Layout::iterator iter = layout.sourceToIterator(tspan);
        NR::Point anchor_point = layout.chunkAnchorPoint(iter);
        sp_repr_set_double(SP_OBJECT_REPR(tspan), "x", anchor_point[NR::X]);
        sp_repr_set_double(SP_OBJECT_REPR(tspan), "y", anchor_point[NR::Y]);
    }
}


void SPText::_adjustFontsizeRecursive(SPItem *item, double ex)
{
    SPStyle *style = SP_OBJECT_STYLE (item);

    if (style && !NR_DF_TEST_CLOSE (ex, 1.0, NR_EPSILON)) {
        style->font_size.computed *= ex;
        if (style->text) {
            style->letter_spacing.computed *= ex;
            style->word_spacing.computed *= ex;
        }
        item->updateRepr();
    }

    for (SPObject *o = item->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o))
            _adjustFontsizeRecursive(SP_ITEM(o), ex);
    }
}

void SPText::_adjustCoordsRecursive(SPItem *item, NR::Matrix const &m, double ex)
{
    if (SP_IS_TSPAN(item))
        SP_TSPAN(item)->attributes.transform(m, ex, ex);
              // it doesn't matter if we change the x,y for role=line spans because we'll just overwrite them anyway
    else if (SP_IS_TEXT(item))
        SP_TEXT(item)->attributes.transform(m, ex, ex);
    else if (SP_IS_TEXTPATH(item))
        SP_TEXTPATH(item)->attributes.transform(m, ex, ex);

    for (SPObject *o = item->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o))
            _adjustCoordsRecursive(SP_ITEM(o), m, ex);
    }
}


void SPText::_clearFlow(NRArenaGroup *in_arena)
{
    nr_arena_item_request_render (in_arena);
    for (NRArenaItem *child = in_arena->children; child != NULL; ) {
        NRArenaItem *nchild = child->next;

        nr_arena_glyphs_group_clear(NR_ARENA_GLYPHS_GROUP(child));
        nr_arena_item_remove_child (in_arena, child);

        child=nchild;
    }
}


/*
 * TextTagAttributes implementation
 */

void TextTagAttributes::readFrom(Inkscape::XML::Node const *node)
{
    readSingleAttribute(SP_ATTR_X, node->attribute("x"));
    readSingleAttribute(SP_ATTR_Y, node->attribute("y"));
    readSingleAttribute(SP_ATTR_DX, node->attribute("dx"));
    readSingleAttribute(SP_ATTR_DY, node->attribute("dy"));
    readSingleAttribute(SP_ATTR_ROTATE, node->attribute("rotate"));
}

bool TextTagAttributes::readSingleAttribute(unsigned key, gchar const *value)
{
    std::vector<SPSVGLength> *attr_vector;
    switch (key) {
        case SP_ATTR_X:      attr_vector = &attributes.x; break;
        case SP_ATTR_Y:      attr_vector = &attributes.y; break;
        case SP_ATTR_DX:     attr_vector = &attributes.dx; break;
        case SP_ATTR_DY:     attr_vector = &attributes.dy; break;
        case SP_ATTR_ROTATE: attr_vector = &attributes.rotate; break;
        default: return false;
    }

    GList *list_base = sp_svg_length_list_read(value);     // FIXME: sp_svg_length_list_read() amalgamates repeated separators. This prevents unset values.
    // simple GList to std::vector<> converter:
    attr_vector->clear();
	attr_vector->reserve(g_list_length(list_base));
    for (GList *list = list_base ; list ; list = list->next) {
        attr_vector->push_back(*reinterpret_cast<SPSVGLength*>(list->data));
        g_free(list->data);
    }
	g_list_free(list_base);
    return true;
}

void TextTagAttributes::writeTo(Inkscape::XML::Node *node) const
{
    writeSingleAttribute(node, "x", attributes.x);
    writeSingleAttribute(node, "y", attributes.y);
    writeSingleAttribute(node, "dx", attributes.dx);
    writeSingleAttribute(node, "dy", attributes.dy);
    writeSingleAttribute(node, "rotate", attributes.rotate);
}

void TextTagAttributes::writeSingleAttribute(Inkscape::XML::Node *node, gchar const *key, std::vector<SPSVGLength> const &attr_vector)
{
	if (attr_vector.empty())
        node->setAttribute(key, NULL);
    else {
        Glib::ustring string;
	    gchar single_value_string[32];
	    
        // FIXME: this has no concept of unset values because sp_svg_length_list_read() can't read them back in
	    for (std::vector<SPSVGLength>::const_iterator it = attr_vector.begin() ; it != attr_vector.end() ; it++) {
		    g_ascii_formatd(single_value_string, sizeof (single_value_string), "%.8g", it->computed);
            if (!string.empty()) string += ' ';
            string += single_value_string;
	    }
        node->setAttribute(key, string.c_str());
    }
}

bool TextTagAttributes::singleXYCoordinates() const
{
    return attributes.x.size() <= 1 && attributes.y.size() <= 1;
}

bool TextTagAttributes::anyAttributesSet() const
{
    return !attributes.x.empty() || !attributes.y.empty() || !attributes.dx.empty() || !attributes.dy.empty() || !attributes.rotate.empty();
}

NR::Point TextTagAttributes::firstXY() const
{
    NR::Point point;
    if (attributes.x.empty()) point[NR::X] = 0.0;
    else point[NR::X] = attributes.x[0].computed;
    if (attributes.y.empty()) point[NR::Y] = 0.0;
    else point[NR::Y] = attributes.y[0].computed;
    return point;
}

void TextTagAttributes::mergeInto(Inkscape::Text::Layout::OptionalTextTagAttrs *output, Inkscape::Text::Layout::OptionalTextTagAttrs const &parent_attrs, unsigned parent_attrs_offset, bool copy_xy, bool copy_dxdyrotate) const
{
    mergeSingleAttribute(&output->x,      parent_attrs.x,      parent_attrs_offset, copy_xy ? &attributes.x : NULL);
    mergeSingleAttribute(&output->y,      parent_attrs.y,      parent_attrs_offset, copy_xy ? &attributes.y : NULL);
    mergeSingleAttribute(&output->dx,     parent_attrs.dx,     parent_attrs_offset, copy_dxdyrotate ? &attributes.dx : NULL);
    mergeSingleAttribute(&output->dy,     parent_attrs.dy,     parent_attrs_offset, copy_dxdyrotate ? &attributes.dy : NULL);
    mergeSingleAttribute(&output->rotate, parent_attrs.rotate, parent_attrs_offset, copy_dxdyrotate ? &attributes.rotate : NULL);
}

void TextTagAttributes::mergeSingleAttribute(std::vector<SPSVGLength> *output_list, std::vector<SPSVGLength> const &parent_list, unsigned parent_offset, std::vector<SPSVGLength> const *overlay_list)
{
    if (overlay_list == NULL) {
        output_list->resize(std::max(0, (int)parent_list.size() - (int)parent_offset));
        std::copy(parent_list.begin() + parent_offset, parent_list.end(), output_list->begin());
    } else {
        output_list->clear();
        output_list->reserve(std::max((int)parent_list.size() - (int)parent_offset, (int)overlay_list->size()));
        unsigned overlay_offset = 0;
        while (parent_offset < parent_list.size() || overlay_offset < overlay_list->size()) {
            SPSVGLength const *this_item;
            if (overlay_offset < overlay_list->size()) {
                this_item = &(*overlay_list)[overlay_offset];
                overlay_offset++;
                parent_offset++;
            } else {
                this_item = &parent_list[parent_offset];
                parent_offset++;
            }
            output_list->push_back(*this_item);
        }
    }
}

void TextTagAttributes::erase(unsigned start_index, unsigned n)
{
    if (n == 0) return;
    if (!singleXYCoordinates()) {
        eraseSingleAttribute(&attributes.x, start_index, n);
        eraseSingleAttribute(&attributes.y, start_index, n);
    }
    eraseSingleAttribute(&attributes.dx, start_index, n);
    eraseSingleAttribute(&attributes.dy, start_index, n);
    eraseSingleAttribute(&attributes.rotate, start_index, n);
}

void TextTagAttributes::eraseSingleAttribute(std::vector<SPSVGLength> *attr_vector, unsigned start_index, unsigned n)
{
    if (attr_vector->size() <= start_index) return;
    if (attr_vector->size() <= start_index + n)
        attr_vector->erase(attr_vector->begin() + start_index, attr_vector->end());
    else
        attr_vector->erase(attr_vector->begin() + start_index, attr_vector->begin() + start_index + n);
}

void TextTagAttributes::insert(unsigned start_index, unsigned n)
{
    if (n == 0) return;
    if (!singleXYCoordinates()) {
        insertSingleAttribute(&attributes.x, start_index, n, true);
        insertSingleAttribute(&attributes.y, start_index, n, true);
    }
    insertSingleAttribute(&attributes.dx, start_index, n, false);
    insertSingleAttribute(&attributes.dy, start_index, n, false);
    insertSingleAttribute(&attributes.rotate, start_index, n, false);
}

void TextTagAttributes::insertSingleAttribute(std::vector<SPSVGLength> *attr_vector, unsigned start_index, unsigned n, bool is_xy)
{
    if (attr_vector->size() <= start_index) return;
    SPSVGLength zero_length;
    zero_length = 0.0;
    attr_vector->insert(attr_vector->begin() + start_index, n, zero_length);
    if (is_xy) {
        double begin = start_index == 0 ? (*attr_vector)[start_index + n].computed : (*attr_vector)[start_index - 1].computed;
        double diff = ((*attr_vector)[start_index + n].computed - begin) / n;   // n tested for nonzero in insert()
        for (unsigned i = 0 ; i < n ; i++)
            (*attr_vector)[start_index + i] = begin + diff * i;
    }
}

void TextTagAttributes::split(unsigned index, TextTagAttributes *second)
{
    if (!singleXYCoordinates()) {
        splitSingleAttribute(&attributes.x, index, &second->attributes.x, false);
        splitSingleAttribute(&attributes.y, index, &second->attributes.y, false);
    }
    splitSingleAttribute(&attributes.dx, index, &second->attributes.dx, true);
    splitSingleAttribute(&attributes.dy, index, &second->attributes.dy, true);
    splitSingleAttribute(&attributes.rotate, index, &second->attributes.rotate, true);
}

void TextTagAttributes::splitSingleAttribute(std::vector<SPSVGLength> *first_vector, unsigned index, std::vector<SPSVGLength> *second_vector, bool trimZeros)
{
    second_vector->clear();
    if (first_vector->size() <= index) return;
    second_vector->resize(first_vector->size() - index);
    std::copy(first_vector->begin() + index, first_vector->end(), second_vector->begin());
    first_vector->resize(index);
    if (trimZeros)
        while (!first_vector->empty() && (!first_vector->back().set || first_vector->back().value == 0.0))
            first_vector->resize(first_vector->size() - 1);
}

void TextTagAttributes::join(TextTagAttributes const &first, TextTagAttributes const &second, unsigned second_index)
{
    if (second.singleXYCoordinates()) {
        attributes.x = first.attributes.x;
        attributes.y = first.attributes.y;
    } else {
        joinSingleAttribute(&attributes.x, first.attributes.x, second.attributes.x, second_index);
        joinSingleAttribute(&attributes.y, first.attributes.y, second.attributes.y, second_index);
    }
    joinSingleAttribute(&attributes.dx, first.attributes.dx, second.attributes.dx, second_index);
    joinSingleAttribute(&attributes.dy, first.attributes.dy, second.attributes.dy, second_index);
    joinSingleAttribute(&attributes.rotate, first.attributes.rotate, second.attributes.rotate, second_index);
}

void TextTagAttributes::joinSingleAttribute(std::vector<SPSVGLength> *dest_vector, std::vector<SPSVGLength> const &first_vector, std::vector<SPSVGLength> const &second_vector, unsigned second_index)
{
    if (second_vector.empty())
        *dest_vector = first_vector;
    else {
        dest_vector->resize(second_index + second_vector.size());
        if (first_vector.size() < second_index) {
            std::copy(first_vector.begin(), first_vector.end(), dest_vector->begin());
            SPSVGLength zero_length;
            zero_length = 0.0;
            std::fill(dest_vector->begin() + first_vector.size(), dest_vector->begin() + second_index, zero_length);
        } else
            std::copy(first_vector.begin(), first_vector.begin() + second_index, dest_vector->begin());
        std::copy(second_vector.begin(), second_vector.end(), dest_vector->begin() + second_index);
    }
}

void TextTagAttributes::transform(NR::Matrix const &matrix, double scale_x, double scale_y)
{
    SPSVGLength zero_length;
    zero_length = 0.0;

    // we can't apply a matrix to only one coordinate, so extend the shorter of x or y if necessary
    if (attributes.x.size() < attributes.y.size()) {
        if (attributes.x.empty()) attributes.x.resize(attributes.y.size(), zero_length);
        else attributes.x.resize(attributes.y.size(), attributes.x.back());
    } else if (attributes.y.size() < attributes.x.size()) {
        if (attributes.y.empty()) attributes.y.resize(attributes.x.size(), zero_length);
        else attributes.y.resize(attributes.x.size(), attributes.y.back());
    }
    for (unsigned i = 0 ; i < attributes.x.size() ; i++) {
        NR::Point point(attributes.x[i].computed, attributes.y[i].computed);
        point *= matrix;
        attributes.x[i] = point[NR::X];
        attributes.y[i] = point[NR::Y];
    }
    for (std::vector<SPSVGLength>::iterator it = attributes.dx.begin() ; it != attributes.dx.end() ; it++)
        *it = it->computed * scale_x;
    for (std::vector<SPSVGLength>::iterator it = attributes.dy.begin() ; it != attributes.dy.end() ; it++)
        *it = it->computed * scale_y;
}

void TextTagAttributes::addToDxDy(unsigned index, NR::Point const &adjust)
{
    SPSVGLength zero_length;
    zero_length = 0.0;

    if (adjust[NR::X] != 0.0) {
        if (attributes.dx.size() < index + 1) attributes.dx.resize(index + 1, zero_length);
        attributes.dx[index] = attributes.dx[index].computed + adjust[NR::X];
    }
    if (adjust[NR::Y] != 0.0) {
        if (attributes.dy.size() < index + 1) attributes.dy.resize(index + 1, zero_length);
        attributes.dy[index] = attributes.dy[index].computed + adjust[NR::Y];
    }
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
