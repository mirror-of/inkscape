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
#include "fontsize-expansion.h"
#include "style.h"
#include "version.h"
#include "inkscape.h"
#include "view.h"
#include "print.h"
#include "sp-metrics.h"
#include "xml/repr.h"
#include "xml/attribute-record.h"

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
    new (&text->contents) div_flow_src(SP_OBJECT(text),txt_text);
    new (&text->layout) Inkscape::Text::Layout;
    text->linespacing.set = 0;
    text->linespacing.value = text->linespacing.computed = 1.0;
}

static void
sp_text_release (SPObject *object)
{
    SPText *text = SP_TEXT(object);
    text->layout.~Layout();
    text->contents.~div_flow_src();

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
    sp_object_read_attr(object, "sodipodi:linespacing");

    if (((SPObjectClass *) text_parent_class)->build)
        ((SPObjectClass *) text_parent_class)->build(object, doc, repr);

    sp_text_update_immediate_state(text);
}

static void
sp_text_set(SPObject *object, unsigned key, gchar const *value)
{
    SPText *text = SP_TEXT (object);

    /* fixme: Vectors (Lauris) */
    switch (key) {
        case SP_ATTR_X:
            text->contents.SetX(value);
            if ( text->contents.nb_x > 0 ) {
                text->x=text->contents.x_s[0];
            } else {
                text->x.set=0;
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_Y:
            text->contents.SetY(value);
            if ( text->contents.nb_y > 0 ) {
                text->y=text->contents.y_s[0];
            } else {
                text->y.set=0;
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_DX:
            text->contents.SetDX(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_DY:
            text->contents.SetDY(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_ROTATE:
            text->contents.SetRot(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_SODIPODI_LINESPACING:
            if (value) {
                text->linespacing.set=1;
                text->linespacing.unit=SP_SVG_UNIT_PERCENT;
                text->linespacing.value=text->linespacing.computed=sp_svg_read_percentage (value, 1.0);
            } else {
                text->linespacing.set=0;
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
            break;
        default:
            if (((SPObjectClass *) text_parent_class)->set)
                ((SPObjectClass *) text_parent_class)->set (object, key, value);
            break;
    }
}

static void
sp_text_child_added (SPObject *object, Inkscape::XML::Node *rch, Inkscape::XML::Node *ref)
{
    //SPItem *item = SP_ITEM (object);
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

    char *nlist=NULL;
    if ( (nlist=text->contents.GetX()) ) {
        if (text->x.set)
            sp_repr_set_attr (repr, "x", nlist); 
        else 
            sp_repr_set_attr (repr, "x", NULL);
        g_free(nlist);
    } else {
        if ( text->x.set ) sp_repr_set_double (repr, "x", text->x.computed); else sp_repr_set_attr (repr, "x", NULL);
    }
    if ( text->y.set && (nlist=text->contents.GetY()) ) {
        if (text->y.set)
            sp_repr_set_attr (repr, "y", nlist);
        else 
            sp_repr_set_attr (repr, "y", NULL);
        g_free(nlist);
    } else {
        if ( text->y.set ) sp_repr_set_double (repr, "y", text->y.computed); else sp_repr_set_attr (repr, "y", NULL);
    }
    if ( (nlist=text->contents.GetDX()) ) {
        sp_repr_set_attr(repr,"dx",nlist);
        g_free(nlist);
    } else {
        sp_repr_set_attr (repr, "dx", NULL);
    }
    if ( (nlist=text->contents.GetDY()) ) {
        sp_repr_set_attr(repr,"dy",nlist);
        g_free(nlist);
    } else {
        sp_repr_set_attr (repr, "dy", NULL);
    }
    if ( (nlist=text->contents.GetRot()) ) {
        sp_repr_set_attr(repr,"rotate",nlist);
        g_free(nlist);
    } else {
        sp_repr_set_attr (repr, "rotate", NULL);
    }

    if (((SPObjectClass *) (text_parent_class))->write)
        ((SPObjectClass *) (text_parent_class))->write (object, repr, flags);

    return repr;
}

static void
sp_text_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const /*flags*/)
{
    SPText *group = SP_TEXT(item);
    //RH if ( group->f_res ) group->f_res->BBox(bbox,transform);
    group->layout.getBoundingBox(bbox, transform);
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
    group->BuildFlow(flowed, &paintbox);

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

static void AccumulateAttributeLists(GList **output_list, GList const *parent_list, SPSVGLength *overlay_list, int overlay_list_length)
{
    *output_list = NULL;
    while (parent_list || overlay_list_length) {
        SPSVGLength const *this_item;
        if (overlay_list_length) {
            this_item = overlay_list;
            overlay_list_length--;
            overlay_list++;
            if (parent_list)
                parent_list = parent_list->next;
        } else {
            this_item = (SPSVGLength const *)parent_list->data;
            parent_list = parent_list->next;
        }
        *output_list = g_list_append(*output_list, (void*)this_item);
    }
}

static void IncrementOptionalAttrsFields(Inkscape::Text::Layout::OptionalTextTagAttrs *optional_attrs, int dist)
{
    while (dist) {
        if (optional_attrs->x) optional_attrs->x = optional_attrs->x->next;
        if (optional_attrs->y) optional_attrs->y = optional_attrs->y->next;
        if (optional_attrs->dx) optional_attrs->dx = optional_attrs->dx->next;
        if (optional_attrs->dy) optional_attrs->dy = optional_attrs->dy->next;
        if (optional_attrs->rotate) optional_attrs->rotate = optional_attrs->rotate->next;
        dist--;
    }
}

static int BuildLayoutInput(SPObject *root, Inkscape::Text::Layout *layout, Inkscape::Text::Layout::OptionalTextTagAttrs const &parent_optional_attrs)
{
    int length = 0;

    Inkscape::Text::Layout::OptionalTextTagAttrs optional_attrs_base, optional_attrs;
    optional_attrs_base.x = NULL;
    optional_attrs_base.y = NULL;
    optional_attrs_base.dx = NULL;
    optional_attrs_base.dy = NULL;
    optional_attrs_base.rotate = NULL;

    div_flow_src const *div_src = NULL;
    if (SP_IS_TEXT(root)) {
        AccumulateAttributeLists(&optional_attrs_base.x, parent_optional_attrs.x, &SP_TEXT(root)->x, 1);
        AccumulateAttributeLists(&optional_attrs_base.y, parent_optional_attrs.y, &SP_TEXT(root)->y, 1);
        optional_attrs = optional_attrs_base;
        //div_src = &SP_TEXT(root)->contents;
        // text elements don't read vectors properly. See lauris' fixme above.
    }
    else if (SP_IS_TSPAN(root) && SP_TSPAN(root)->role == SP_TSPAN_ROLE_UNSPECIFIED) div_src = &SP_TSPAN(root)->contents;
    else if (SP_IS_TEXTPATH(root)) div_src = &SP_TEXTPATH(root)->contents;

    if (div_src) {
        AccumulateAttributeLists(&optional_attrs_base.x, parent_optional_attrs.x, div_src->x_s, div_src->nb_x);
        AccumulateAttributeLists(&optional_attrs_base.y, parent_optional_attrs.y, div_src->y_s, div_src->nb_y);
        AccumulateAttributeLists(&optional_attrs_base.dx, parent_optional_attrs.dx, div_src->dx_s, div_src->nb_dx);
        AccumulateAttributeLists(&optional_attrs_base.dy, parent_optional_attrs.dy, div_src->dy_s, div_src->nb_dy);
        AccumulateAttributeLists(&optional_attrs_base.rotate, parent_optional_attrs.rotate, div_src->rot_s, div_src->nb_rot);
        optional_attrs = optional_attrs_base;
    } else if (!SP_IS_TEXT(root)) {
        optional_attrs = parent_optional_attrs;
    }

    if (SP_IS_TSPAN(root))
        if (SP_TSPAN(root)->role != SP_TSPAN_ROLE_UNSPECIFIED) {
            length++;     // interpreting line breaks as a character for the purposes of x/y/etc attributes
                          // is a liberal interpretation of the svg spec, but a strict reading would mean
                          // that if the first line is empty the second line would take its place at the
                          // start position. Very confusing.
            if (layout->inputExists())
                layout->appendControlCode(Inkscape::Text::Layout::PARAGRAPH_BREAK, root);
            if (!root->hasChildren())
                layout->appendText("", root->style, root, &optional_attrs);
        }

    for (SPObject *child = sp_object_first_child(root) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_TSPAN (child) || SP_IS_TEXTPATH (child)) {
            int child_lengths = BuildLayoutInput(child, layout, optional_attrs);
            IncrementOptionalAttrsFields(&optional_attrs, child_lengths);
            length += child_lengths;
        } else if (SP_IS_STRING (child)) {
            Glib::ustring const &string = SP_STRING(child)->string;
            layout->appendText(string, root->style, child, &optional_attrs);
            int string_length = string.length();
            IncrementOptionalAttrsFields(&optional_attrs, string_length);
            length += string_length;
        }
    }
    g_list_free(optional_attrs_base.x);
    g_list_free(optional_attrs_base.y);
    g_list_free(optional_attrs_base.dx);
    g_list_free(optional_attrs_base.dy);
    g_list_free(optional_attrs_base.rotate);
    return length;
}

static void
sp_text_set_shape (SPText *text)
{
    // because the text_styles are held by f_src, we need to delete this before, to avoid dangling pointers
    for (SPItemView* v = text->display; v != NULL; v = v->next) {
        text->ClearFlow(NR_ARENA_GROUP(v->arenaitem));
    }

    Inkscape::Text::Layout::OptionalTextTagAttrs optional_attrs;
    optional_attrs.x = NULL;
    optional_attrs.y = NULL;
    optional_attrs.dx = NULL;
    optional_attrs.dy = NULL;
    optional_attrs.rotate = NULL;
    text->layout.clear();
    BuildLayoutInput(text, &text->layout, optional_attrs);
    text->layout.calculateFlow();
    for (SPObject *child = sp_object_first_child(text) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_TEXTPATH (child) && SP_TEXTPATH(child)->originalPath != NULL) {
            SPSVGLength offset={0};
            g_print(text->layout.dumpAsText().c_str());
            text->layout.fitToPathAlign(offset, *SP_TEXTPATH(child)->originalPath);
        }
    }
    g_print(text->layout.dumpAsText().c_str());
    // todo: set the x,y attributes on role:line spans

    NRRect paintbox;
    sp_item_invoke_bbox(SP_ITEM(text), &paintbox, NR::identity(), TRUE);
    for (SPItemView* v = SP_ITEM(text)->display; v != NULL; v = v->next) {
        // pass the bbox of the text object as paintbox (used for paintserver fills)
        text->BuildFlow(NR_ARENA_GROUP(v->arenaitem), &paintbox);
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
            style->text->letterspacing.computed *= ex;
            style->text->wordspacing.computed *= ex;
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
    div_flow_src *contents = NULL; 
    SPSVGLength *x = NULL, *y = NULL;
    // if someone knows how to make this less ugly, you're welcome:
    if (SP_IS_TSPAN(item)) {
        contents = &(SP_TSPAN(item)->contents);
        x = &(SP_TSPAN(item)->x);
        y = &(SP_TSPAN(item)->y);
    } else if (SP_IS_TEXT(item)) {
        contents = &(SP_TEXT(item)->contents);
        x = &(SP_TEXT(item)->x);
        y = &(SP_TEXT(item)->y);
    }

    if (contents) {
        if (!(SP_IS_TSPAN(item) && SP_TSPAN(item)->role == SP_TSPAN_ROLE_LINE)) {
            // Do not touch x/y for line tspans, because they will get updated automatically from parent

            /* Recalculate x/y lists */
            contents->TransformXY (m, SP_IS_TEXT(item));
            if ( contents->nb_x > 0 ) {
                *x = contents->x_s[0];
                x->set = 1;
            } else {
                x->set = 0;
            }
            if ( contents->nb_y > 0 ) {
                *y = contents->y_s[0];
                y->set = 1;
            } else {
                y->set = 0;
            }
        }

        /* Recalculate dx/dy lists */
        if (SP_IS_TEXT(item)) {
            // ScaleDXDY will update the kerns in the entire chain of linked flow_src's of the text
            // object, therefore we call it only once for the root <text> but not for tspans
            contents->ScaleDXDY (ex);
        }
    }

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
    SPStyle *style = SP_OBJECT_STYLE (text);

    if (!str) str = "";
    gchar *content = g_strdup (str);

    SP_OBJECT_REPR (text)->setContent("");
    while (repr->firstChild()) {
        repr->removeChild(repr->firstChild());
    }

    NR::Point cp(text->x.computed, text->y.computed);

    gchar *p = content;
    while (p) {
        gchar *e = strchr (p, '\n');
        if (is_textpath) {
            if (e) *e = ' '; // no lines for textpath, replace newlines with spaces
        } else {
            if (e) *e = '\0'; // create a tspan for each line
            Inkscape::XML::Node *rtspan = sp_repr_new ("svg:tspan");
            sp_repr_set_double (rtspan, "x", cp[NR::X]);
            sp_repr_set_double (rtspan, "y", cp[NR::Y]);
            if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB) {
                cp[NR::X] -= style->font_size.computed;
            } else {
                cp[NR::Y] += style->font_size.computed;
            }
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


SPTSpan *
sp_text_append_line(SPText *text)
{
    g_return_val_if_fail (text != NULL, NULL);
    g_return_val_if_fail (SP_IS_TEXT (text), NULL);

    SPStyle *style = SP_OBJECT_STYLE (text);

    NR::Point cp(text->x.computed, text->y.computed);

    for (SPObject *child = sp_object_first_child(SP_OBJECT(text)) ; child != NULL; child = SP_OBJECT_NEXT(child)) {
        if (SP_IS_TSPAN (child)) {
            SPTSpan *tspan = SP_TSPAN (child);
            if (tspan->role == SP_TSPAN_ROLE_LINE) {
                cp[NR::X] = tspan->x.computed;
                cp[NR::Y] = tspan->y.computed;
            }
        }
    }

    /* Create <tspan> */
    Inkscape::XML::Node *rtspan = sp_repr_new ("svg:tspan");
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
    Inkscape::XML::Node *rstring = sp_repr_new_text("");
    sp_repr_add_child (rtspan, rstring, NULL);
    sp_repr_unref (rstring);
    /* Append to text */
    SP_OBJECT_REPR (text)->appendChild(rtspan);
    sp_repr_unref (rtspan);

    return (SPTSpan *) SP_OBJECT_DOCUMENT (text)->getObjectByRepr(rtspan);
}

static bool is_line_break_object(SPObject *object)
{
    return    SP_IS_TEXT(object)
           || (SP_IS_TSPAN(object) && SP_TSPAN(object)->role != SP_TSPAN_ROLE_UNSPECIFIED);
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

int
sp_text_insert_line (SPText *text, gint i_ucs4_pos)
{
    // Disable newlines in a textpath; TODO: maybe on Enter in a textpath, separate it into two
    // texpaths attached to the same path, with a vertical shift
    if (SP_IS_TEXT_TEXTPATH (text)) 
        return 0;

    Inkscape::Text::Layout::iterator it_split = text->layout.charIndexToIterator(i_ucs4_pos);
    if (it_split == text->layout.end()) {
        sp_text_append_line(text);
        return 1;
    }
    SPObject *split_obj;
    Glib::ustring::iterator split_text_iter;
    text->layout.getSourceOfCharacter(it_split, (void**)&split_obj, &split_text_iter);

    if (SP_IS_STRING(split_obj)) {
        Glib::ustring *string = &SP_STRING(split_obj)->string;
        // we need to split the entire text tree into two
        SPString *new_string = SP_STRING(split_text_object_tree_at(split_obj));
        SP_OBJECT_REPR(new_string)->setContent(&*split_text_iter.base());   // a little ugly
        string->erase(split_text_iter, string->end());
        SP_OBJECT_REPR(split_obj)->setContent(string->c_str());
    } else if (is_line_break_object(split_obj)) {
        if (SP_OBJECT_PREV(split_obj)) {   // always true
            split_obj = SP_OBJECT_PREV(split_obj);
            Inkscape::XML::Node *new_node = duplicate_node_without_children(SP_OBJECT_REPR(split_obj));
            SP_OBJECT_REPR(SP_OBJECT_PARENT(split_obj))->addChild(new_node, SP_OBJECT_REPR(split_obj));
            sp_repr_unref(new_node);
        }
    } else {
        // TODO
        // I think the only case to put here is arbitrary gaps, which nobody uses yet
    }
    SP_OBJECT(text)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    return 1;
}

gint
sp_text_append (SPText */*text*/, gchar const */*utf8*/)
{
    return 0;
}

void
sp_adjust_kerning_screen (SPText *text, gint i_position, SPDesktop *desktop, NR::Point by)
{
    one_flow_src* cur=&text->contents;

    // divide increment by zoom
    // divide increment by matrix expansion
    gdouble factor = 1 / SP_DESKTOP_ZOOM (desktop);
    NR::Matrix t = sp_item_i2doc_affine (SP_ITEM(text));
    factor = factor / NR::expansion(t);
    by = factor * by;

    SPSVGLength  by_x,by_y;
    by_x.set=by_y.set=1;
    by_x.value=by_x.computed=by[0];
    by_y.value=by_y.computed=by[1];
    while ( cur ) {
        cur->AddValue(i_position, by_x, 2, true, false);
        cur->AddValue(i_position, by_y, 3, true, false);
        cur=cur->next;
    }
    SP_OBJECT(text)->updateRepr();
    SP_OBJECT(text)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
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
    gdouble const zoom = SP_DESKTOP_ZOOM(desktop);
    gdouble const zby = (by
                         / (zoom * (nb_let > 1 ? nb_let - 1 : 1))
                         / NR::expansion(sp_item_i2doc_affine(SP_ITEM(source_obj->parent))));
    val += zby;

    // set back value
    style->text->letterspacing_normal = FALSE;
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
    sp_repr_set_attr (SP_OBJECT_REPR (source_obj->parent), "style", str);
    g_free (str);
}

void
sp_adjust_linespacing_screen (SPText *text, SPDesktop *desktop, gdouble by)
{
    SPStyle *style = SP_OBJECT_STYLE (text);

    // the value is stored as multiple of font size (i.e. in em)
    double val = style->font_size.computed * text->linespacing.value;

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
    sp_repr_set_double (SP_OBJECT_REPR (text), "sodipodi:linespacing", val / style->font_size.computed);
    SP_OBJECT (text)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);
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

void SPText::BuildFlow(NRArenaGroup* in_arena, NRRect *paintbox)
{  //RH: probably doesn't need to be its own function any more
    layout.show(in_arena, paintbox);
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
