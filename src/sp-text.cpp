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
#include "unit-constants.h"
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

static void sp_text_request_relayout (SPText *text, guint flags);
static void sp_text_update_immediate_state (SPText *text);
static void sp_text_set_shape (SPText *text);

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
    SPText *text = SP_TEXT (object);

    sp_object_read_attr(object, "x");
    sp_object_read_attr(object, "y");
    sp_object_read_attr(object, "dx");
    sp_object_read_attr(object, "dy");
    sp_object_read_attr(object, "rotate");

    if (((SPObjectClass *) text_parent_class)->build)
        ((SPObjectClass *) text_parent_class)->build(object, doc, repr);

    sp_object_read_attr(object, "sodipodi:linespacing");    // has to happen after the styles are read

    sp_text_update_immediate_state(text);
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

    sp_text_request_relayout (text, SP_OBJECT_MODIFIED_FLAG | SP_TEXT_CONTENT_MODIFIED_FLAG);
    /* fixme: Instead of forcing it, do it when needed */
    sp_text_update_immediate_state (text);
}

static void
sp_text_remove_child (SPObject *object, Inkscape::XML::Node *rch)
{
    SPText *text = SP_TEXT (object);

    if (((SPObjectClass *) text_parent_class)->remove_child)
        ((SPObjectClass *) text_parent_class)->remove_child (object, rch);

    sp_text_request_relayout (text, SP_OBJECT_MODIFIED_FLAG | SP_TEXT_CONTENT_MODIFIED_FLAG);
    sp_text_update_immediate_state (text);
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
    if ( text->relayout
         || (flags & ( SP_OBJECT_STYLE_MODIFIED_FLAG |
                       SP_OBJECT_CHILD_MODIFIED_FLAG |
                       SP_TEXT_LAYOUT_MODIFIED_FLAG   )) )
    {
        /* fixme: It is not nice to have it here, but otherwise children content changes does not work */
        /* fixme: Even now it may not work, as we are delayed */
        /* fixme: So check modification flag everywhere immediate state is used */
        sp_text_update_immediate_state (text);
        sp_text_set_shape (text);
        text->relayout = FALSE;
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

static unsigned BuildLayoutInput(SPObject *root, Inkscape::Text::Layout *layout, Inkscape::Text::Layout::OptionalTextTagAttrs const &parent_optional_attrs, unsigned parent_attrs_offset)
{
    unsigned length = 0;
    unsigned child_attrs_offset = 0;
    Inkscape::Text::Layout::OptionalTextTagAttrs optional_attrs;

    if (SP_IS_TEXT(root)) {
        SP_TEXT(root)->attributes.mergeInto(&optional_attrs, parent_optional_attrs, parent_attrs_offset, true, true);
    }
    else if (SP_IS_TSPAN(root)) {
        SPTSpan *tspan = SP_TSPAN(root);
        bool use_xy = tspan->role == SP_TSPAN_ROLE_UNSPECIFIED || !tspan->attributes.singleXYCoordinates();
        tspan->attributes.mergeInto(&optional_attrs, parent_optional_attrs, parent_attrs_offset, use_xy, true);
    }
    else if (SP_IS_TEXTPATH(root)) {
        SP_TEXTPATH(root)->attributes.mergeInto(&optional_attrs, parent_optional_attrs, parent_attrs_offset, false, true);
    }
    else {
        optional_attrs = parent_optional_attrs;
        child_attrs_offset = parent_attrs_offset;
    }

    bool is_line_break = false;
    if (SP_IS_TSPAN(root))
        if (SP_TSPAN(root)->role != SP_TSPAN_ROLE_UNSPECIFIED) {
            is_line_break = true;
            if (layout->inputExists())
                layout->appendControlCode(Inkscape::Text::Layout::PARAGRAPH_BREAK, root);
            if (!root->hasChildren())
                layout->appendText("", root->style, root, &optional_attrs);
        }

    for (SPObject *child = sp_object_first_child(root) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_TSPAN (child) || SP_IS_TEXTPATH (child)) {
            length += BuildLayoutInput(child, layout, optional_attrs, child_attrs_offset + length);
        } else if (SP_IS_STRING (child)) {
            Glib::ustring const &string = SP_STRING(child)->string;
            layout->appendText(string, root->style, child, &optional_attrs, child_attrs_offset + length);
            length += string.length();
        }
    }
    if (is_line_break)
        length++;     // interpreting line breaks as a character for the purposes of x/y/etc attributes
                      // is a liberal interpretation of the svg spec, but a strict reading would mean
                      // that if the first line is empty the second line would take its place at the
                      // start position. Very confusing.
    return length;
}

static void
sp_text_set_shape (SPText *text)
{
    // because the text_styles are held by layout, we need to delete this before, to avoid dangling pointers
    for (SPItemView* v = text->display; v != NULL; v = v->next) {
        text->ClearFlow(NR_ARENA_GROUP(v->arenaitem));
    }

    text->layout.clear();
    Inkscape::Text::Layout::OptionalTextTagAttrs optional_attrs;
    BuildLayoutInput(text, &text->layout, optional_attrs, 0);
    text->layout.calculateFlow();
    for (SPObject *child = text->firstChild() ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_TEXTPATH(child) && SP_TEXTPATH(child)->originalPath != NULL) {
            SPSVGLength offset;
            offset = 0.0;     // FIXME (value is not read by SPTextPath yet)
            //g_print(text->layout.dumpAsText().c_str());
            text->layout.fitToPathAlign(offset, *SP_TEXTPATH(child)->originalPath);
        }
    }
    //g_print(text->layout.dumpAsText().c_str());

    // set the x,y attributes on role:line spans
    for (SPObject *child = text->firstChild() ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if (!SP_IS_TSPAN(child)) continue;
        SPTSpan *tspan = SP_TSPAN(child);
        if (tspan->role == SP_TSPAN_ROLE_UNSPECIFIED) continue;
        if (!tspan->attributes.singleXYCoordinates()) continue;
        Inkscape::Text::Layout::iterator iter = text->layout.sourceToIterator(tspan);
        if (iter == text->layout.end()) continue;
        if (iter.nextCharacter()) {   // line breaks live at the end of their preceding line
            NR::Point anchor_point = text->layout.characterAnchorPoint(iter);
            sp_repr_set_double(SP_OBJECT_REPR(tspan), "x", anchor_point[NR::X]);
            sp_repr_set_double(SP_OBJECT_REPR(tspan), "y", anchor_point[NR::Y]);
        }
    }

    NRRect paintbox;
    sp_item_invoke_bbox(SP_ITEM(text), &paintbox, NR::identity(), TRUE);
    for (SPItemView* v = SP_ITEM(text)->display; v != NULL; v = v->next) {
        // pass the bbox of the text object as paintbox (used for paintserver fills)
        text->layout.show(NR_ARENA_GROUP(v->arenaitem), &paintbox);
    }

}


static void sp_text_snappoints(SPItem const *item, SnapPointsIter p)
{
    if (((SPItemClass *) text_parent_class)->snappoints) {
        ((SPItemClass *) text_parent_class)->snappoints (item, p);
    }
}

void
sp_text_adjust_fontsize_recursive (SPItem *item, double ex)
{
    SPStyle *style = SP_OBJECT_STYLE (item);

    if (style && !NR_DF_TEST_CLOSE (ex, 1.0, NR_EPSILON)) {
        style->font_size.computed *= ex;
        if (style->text) {
            style->letter_spacing.computed *= ex;
            style->word_spacing.computed *= ex;
        }
        SP_OBJECT(item)->updateRepr();
    }

    for (SPObject *o = SP_OBJECT(item)->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o))
            sp_text_adjust_fontsize_recursive (SP_ITEM(o), ex);
    }
}

void
sp_text_adjust_coords_recursive (SPItem *item, NR::Matrix const &m, double ex)
{
    if (SP_IS_TSPAN(item))
        SP_TSPAN(item)->attributes.transform(m, ex, ex);
              // it doesn't matter if we change the x,y for role=line spans because we'll just overwrite them anyway
    else if (SP_IS_TEXT(item))
        SP_TEXT(item)->attributes.transform(m, ex, ex);
    else if (SP_IS_TEXTPATH(item))
        SP_TEXTPATH(item)->attributes.transform(m, ex, ex);

    for (SPObject *o = SP_OBJECT(item)->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o))
            sp_text_adjust_coords_recursive (SP_ITEM(o), m, ex);
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
    sp_text_adjust_coords_recursive (item, xform * ret.inverse(), ex);

    // Adjust font size
    sp_text_adjust_fontsize_recursive (item, ex);

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

static void sp_text_get_ustring_multiline(SPObject const *root, Glib::ustring &string)
{
    if (SP_IS_TSPAN(root) && SP_TSPAN(root)->role != SP_TSPAN_ROLE_UNSPECIFIED)
        string += '\n';
    for (SPObject const *child = root->firstChild() ; child ; child = SP_OBJECT_NEXT(child)) {
        if (SP_IS_STRING(child))
            string += SP_STRING(child)->string;
        else if (child->hasChildren())
            sp_text_get_ustring_multiline(child, string);
        else if (SP_IS_TSPAN(root) && SP_TSPAN(root)->role != SP_TSPAN_ROLE_UNSPECIFIED)
            string += '\n';
    }
}

gchar *
sp_text_get_string_multiline (SPText *text)
{
    Glib::ustring string;

    sp_text_get_ustring_multiline(text, string);
    if (string.empty()) return NULL;
    return strdup(string.data() + 1);    // the first char will always be an unwanted line break
}

void
sp_text_set_repr_text_multiline(SPText *text, gchar const *str)
{
    g_return_if_fail (text != NULL);
    g_return_if_fail (SP_IS_TEXT (text));

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
            Inkscape::XML::Node *rtspan = sp_repr_new ("svg:tspan");
            sp_repr_set_attr (rtspan, "sodipodi:role", "line");
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

SPCurve *
sp_text_normalized_bpath (SPText *text)
{
    g_return_val_if_fail (text != NULL, NULL);
    g_return_val_if_fail (SP_IS_TEXT (text), NULL);

    return text->layout.convertToCurves();
}


static void
sp_text_update_immediate_state (SPText */*text*/)
{
}


static void
sp_text_request_relayout (SPText *text, guint flags)
{
    text->relayout = TRUE;

    SP_OBJECT (text)->requestDisplayUpdate(flags);
}


void
sp_adjust_kerning_screen (SPText *text, gint i_position, SPDesktop *desktop, NR::Point by)
{
    SPObject *source_item;
    Glib::ustring::iterator source_text_iter;
    Inkscape::Text::Layout::iterator iter_layout = text->layout.charIndexToIterator(i_position);
    text->layout.getSourceOfCharacter(iter_layout, (void**)&source_item, &source_text_iter);

    if (!SP_IS_STRING(source_item)) return;
    Glib::ustring *string = &SP_STRING(source_item)->string;
    unsigned char_index = 0;
    for (Glib::ustring::iterator it = string->begin() ; it != source_text_iter ; it++)
        char_index++;

    // divide increment by zoom
    // divide increment by matrix expansion
    gdouble factor = 1 / SP_DESKTOP_ZOOM(desktop);
    NR::Matrix t = sp_item_i2doc_affine(text);
    factor = factor / NR::expansion(t);
    by = factor * by;
    // need to change the dx/dy in all the ancestors too, so that if the topmost span ever gets deleted the
    // following text will keep the new kern
    for ( ; ; ) {
        source_item = SP_OBJECT_PARENT(source_item);
        TextTagAttributes *attributes;
        if (SP_IS_TEXT(source_item)) attributes = &SP_TEXT(source_item)->attributes;
        else if(SP_IS_TSPAN(source_item)) attributes = &SP_TSPAN(source_item)->attributes;
        else if(SP_IS_TEXTPATH(source_item)) attributes = &SP_TEXTPATH(source_item)->attributes;
        else break;

        attributes->addToDxDy(char_index, by);
        for (SPObject *sibling = SP_OBJECT_PARENT(source_item)->firstChild() ; sibling && sibling != source_item ; sibling = SP_OBJECT_NEXT(sibling))
            char_index += sp_text_get_length(sibling);
    }
    text->updateRepr();
    text->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void
sp_adjust_tspan_letterspacing_screen(SPText *text, gint i_position, SPDesktop *desktop, gdouble by)
{
    gdouble val;
    Inkscape::Text::Layout::iterator it = text->layout.charIndexToIterator(i_position);
    SPObject *source_obj;
    text->layout.getSourceOfCharacter(it, (void**)&source_obj);
    if (!SP_IS_STRING(source_obj)) return;   // FIXME? we could take a guess at which side the user wants

    int nb_let = SP_STRING(source_obj)->string.length();
    SPStyle *style = SP_OBJECT_STYLE (source_obj->parent);

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
                         / NR::expansion(sp_item_i2doc_affine(SP_ITEM(source_obj->parent))));
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
sp_adjust_linespacing_screen (SPText *text, SPDesktop *desktop, gdouble by)
{
    SPStyle *style = SP_OBJECT_STYLE (text);

    if (!style->line_height.set || style->line_height.inherit || style->line_height.normal) {
        style->line_height.set = TRUE;
        style->line_height.inherit = FALSE;
        style->line_height.normal = FALSE;
        style->line_height.unit = SP_CSS_UNIT_PERCENT;
        style->line_height.value = style->line_height.computed = 1.0;
    }

    unsigned line_count = text->layout.lineIndex(text->layout.end());

    // divide increment by zoom and by the number of lines,
    // so that the entire object is expanded by by pixels
    gdouble zby = by / (SP_DESKTOP_ZOOM (desktop) * (line_count > 1 ? line_count - 1 : 1));

    // divide increment by matrix expansion
    NR::Matrix t = sp_item_i2doc_affine (SP_ITEM(text));
    zby = zby / NR::expansion(t);

    switch (style->line_height.unit) {
        case SP_CSS_UNIT_NONE:
        default:
            // multiplier-type units, stored in computed
            style->line_height.computed += zby;
            style->line_height.value = style->line_height.computed;
            break;
        case SP_CSS_UNIT_EM:
        case SP_CSS_UNIT_EX:
        case SP_CSS_UNIT_PERCENT:
            // multiplier-type units, stored in value
            style->line_height.value += zby;
            break;
            // absolute-type units
	    case SP_CSS_UNIT_PX:
            style->line_height.computed += zby / style->font_size.computed;
            style->line_height.value = style->line_height.computed;
            break;
	    case SP_CSS_UNIT_PT:
            style->line_height.computed += zby / style->font_size.computed * PT_PER_PX;
            style->line_height.value = style->line_height.computed;
            break;
	    case SP_CSS_UNIT_PC:
            style->line_height.computed += zby / style->font_size.computed * (PT_PER_PX / 12);
            style->line_height.value = style->line_height.computed;
            break;
	    case SP_CSS_UNIT_MM:
            style->line_height.computed += zby / style->font_size.computed * MM_PER_PX;
            style->line_height.value = style->line_height.computed;
            break;
	    case SP_CSS_UNIT_CM:
            style->line_height.computed += zby / style->font_size.computed * CM_PER_PX;
            style->line_height.value = style->line_height.computed;
            break;
	    case SP_CSS_UNIT_IN:
            style->line_height.computed += zby / style->font_size.computed * IN_PER_PX;
            style->line_height.value = style->line_height.computed;
            break;
    }
    text->updateRepr();
    text->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
}

/*
 *
 */


void SPText::ClearFlow(NRArenaGroup *in_arena)
{
    nr_arena_item_request_render (NR_ARENA_ITEM (in_arena));
    for (NRArenaItem *child = in_arena->children; child != NULL; ) {
        NRArenaItem *nchild = child->next;

        nr_arena_glyphs_group_clear(NR_ARENA_GLYPHS_GROUP(child));
        nr_arena_item_remove_child (NR_ARENA_ITEM (in_arena), child);

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
    eraseSingleAttribute(&attributes.x, start_index, n);
    eraseSingleAttribute(&attributes.y, start_index, n);
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
    insertSingleAttribute(&attributes.x, start_index, n, true);
    insertSingleAttribute(&attributes.y, start_index, n, true);
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
