/*
 */

#include <config.h>
#include <string.h>

#include "attributes.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "style.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "print.h"

#include "libnr/nr-matrix.h"
#include "libnr/nr-point.h"
#include "xml/repr.h"
#include "xml/repr-private.h"

#include "sp-flowdiv.h"
#include "sp-flowregion.h"
#include "sp-flowtext.h"
#include "sp-string.h"

#include "libnr/nr-matrix.h"
#include "libnr/nr-translate.h"
#include "libnr/nr-scale.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-translate-ops.h"
#include "libnr/nr-scale-ops.h"

#include "libnrtype/FontFactory.h"
#include "libnrtype/font-instance.h"
#include "libnrtype/font-style-to-pos.h"

#include "libnrtype/FlowSrc.h"
#include "libnrtype/FlowDest.h"
#include "libnrtype/FlowRes.h"
#include "libnrtype/FlowEater.h"
#include "libnrtype/FlowStyle.h"

#include "livarot/Shape.h"

#include "display/nr-arena-item.h"
#include "display/nr-arena-group.h"
#include "display/nr-arena-glyphs.h"

#include <pango/pango.h>

static void sp_flowtext_class_init(SPFlowtextClass *klass);
static void sp_flowtext_init(SPFlowtext *group);
static void sp_flowtext_dispose(GObject *object);

static void sp_flowtext_child_added(SPObject *object, SPRepr *child, SPRepr *ref);
static void sp_flowtext_remove_child(SPObject *object, SPRepr *child);
static void sp_flowtext_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_flowtext_modified(SPObject *object, guint flags);
static SPRepr *sp_flowtext_write(SPObject *object, SPRepr *repr, guint flags);
static void sp_flowtext_build(SPObject *object, SPDocument *document, SPRepr *repr);
static void sp_flowtext_set(SPObject *object, unsigned key, gchar const *value);

static void sp_flowtext_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags);
static void sp_flowtext_print(SPItem *item, SPPrintContext *ctx);
static gchar *sp_flowtext_description(SPItem *item);
static NRArenaItem *sp_flowtext_show(SPItem *item, NRArena *arena, unsigned key, unsigned flags);
static void sp_flowtext_hide(SPItem *item, unsigned key);

static SPItemClass *parent_class;

GType
sp_flowtext_get_type(void)
{
    static GType group_type = 0;
    if (!group_type) {
        GTypeInfo group_info = {
            sizeof(SPFlowtextClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_flowtext_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPFlowtext),
            16,     /* n_preallocs */
            (GInstanceInitFunc) sp_flowtext_init,
            NULL,   /* value_table */
        };
        group_type = g_type_register_static(SP_TYPE_ITEM, "SPFlowtext", &group_info, (GTypeFlags)0);
    }
    return group_type;
}

static void
sp_flowtext_class_init(SPFlowtextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;

    parent_class = (SPItemClass *)g_type_class_ref(SP_TYPE_ITEM);

    object_class->dispose = sp_flowtext_dispose;

    sp_object_class->child_added = sp_flowtext_child_added;
    sp_object_class->remove_child = sp_flowtext_remove_child;
    sp_object_class->update = sp_flowtext_update;
    sp_object_class->modified = sp_flowtext_modified;
    sp_object_class->write = sp_flowtext_write;
    sp_object_class->build = sp_flowtext_build;
    sp_object_class->set = sp_flowtext_set;

    item_class->bbox = sp_flowtext_bbox;
    item_class->print = sp_flowtext_print;
    item_class->description = sp_flowtext_description;
    item_class->show = sp_flowtext_show;
    item_class->hide = sp_flowtext_hide;
}

static void
sp_flowtext_init(SPFlowtext *group)
{
    group->f_dst = group->f_excl = NULL;
    group->f_src = NULL;
    group->f_res = NULL;
    group->justify = false;
    group->par_indent = 0;
    group->algo = 0;

    new (&group->contents) div_flow_src(SP_OBJECT(group), flw_div);
}

static void
sp_flowtext_dispose(GObject *object)
{
    SPFlowtext *group = (SPFlowtext*)object;

    group->contents.~div_flow_src();

    //if ( group->f_dst ) delete group->f_dst;
    if ( group->f_excl ) delete group->f_excl;
    if ( group->f_res ) delete group->f_res;
    if ( group->f_src ) delete group->f_src;
}

static void
sp_flowtext_child_added(SPObject *object, SPRepr *child, SPRepr *ref)
{
    if (((SPObjectClass *) (parent_class))->child_added)
        (* ((SPObjectClass *) (parent_class))->child_added)(object, child, ref);

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: hide (Lauris) */

static void
sp_flowtext_remove_child(SPObject *object, SPRepr *child)
{
    if (((SPObjectClass *) (parent_class))->remove_child)
        (* ((SPObjectClass *) (parent_class))->remove_child)(object, child);

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_flowtext_update(SPObject *object, SPCtx *ctx, unsigned flags)
{
    SPFlowtext *group = SP_FLOWTEXT(object);
    SPItemCtx *ictx = (SPItemCtx *) ctx;
    SPItemCtx cctx = *ictx;

    if (((SPObjectClass *) (parent_class))->update)
        ((SPObjectClass *) (parent_class))->update(object, ctx, flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;
    for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        g_object_ref(G_OBJECT(child));
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse(l);
    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM(child)) {
                SPItem const &chi = *SP_ITEM(child);
                cctx.i2doc = chi.transform * ictx->i2doc;
                cctx.i2vp = chi.transform * ictx->i2vp;
                child->updateDisplay((SPCtx *)&cctx, flags);
            } else {
                child->updateDisplay(ctx, flags);
            }
        }
        g_object_unref(G_OBJECT(child));
    }

    group->UpdateFlowSource();
    group->UpdateFlowDest();
    group->ComputeFlowRes();
}

static void
sp_flowtext_modified(SPObject */*object*/, guint /*flags*/)
{
    /* SPFlowtext *group;

    group = SP_FLOWTEXT(object);*/
}

static void
sp_flowtext_build(SPObject *object, SPDocument *document, SPRepr *repr)
{
    sp_object_read_attr(object, "inkscape:layoutOptions");

    if (((SPObjectClass *) (parent_class))->build) {
        (* ((SPObjectClass *) (parent_class))->build)(object, document, repr);
    }
}

static void
sp_flowtext_set(SPObject *object, unsigned key, gchar const *value)
{
    SPFlowtext *group = (SPFlowtext *) object;

    switch (key) {
        case SP_ATTR_LAYOUT_OPTIONS: {
            SPCSSAttr *opts = sp_repr_css_attr((SP_OBJECT(group))->repr, "inkscape:layoutOptions");
            {
                gchar const *val = sp_repr_css_property(opts, "justification", NULL);
                if ( val == NULL ) {
                    group->justify = false;
                } else {
                    if ( strcmp(val, "0") == 0 || strcmp(val, "false") == 0 ) {
                        group->justify = false;
                    } else {
                        group->justify = true;
                    }
                }
            }
            {
                gchar const *val = sp_repr_css_property(opts, "layoutAlgo", NULL);
                if ( val == NULL ) {
                    group->algo = 0;
                } else {
                    if ( strcmp(val, "better") == 0 ) {
                        group->algo = 2;
                    } else if ( strcmp(val, "simple") == 0 ) {
                        group->algo = 1;
                    } else if ( strcmp(val, "default") == 0 ) {
                        group->algo = 0;
                    }
                }
            }
            {
                gchar const *val = sp_repr_css_property(opts, "par-indent", NULL);
                if ( val == NULL ) {
                    group->par_indent = 0.0;
                } else {
                    sp_repr_get_double((SPRepr*)opts, "par-indent", &group->par_indent);
                }
            }
            sp_repr_css_attr_unref(opts);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        default:
            if (((SPObjectClass *) (parent_class))->set) {
                (* ((SPObjectClass *) (parent_class))->set)(object, key, value);
            }
            break;
    }
}

static SPRepr *
sp_flowtext_write(SPObject *object, SPRepr *repr, guint flags)
{
    if ( flags & SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) repr = sp_repr_new("flowRoot");
        GSList *l = NULL;
        for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            SPRepr *c_repr = NULL;
            if ( SP_IS_FLOWDIV(child) ) {
                c_repr = child->updateRepr(NULL, flags);
            } else if ( SP_IS_FLOWREGION(child) ) {
                c_repr = child->updateRepr(NULL, flags);
            } else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
                c_repr = child->updateRepr(NULL, flags);
            }
            if ( c_repr ) l = g_slist_prepend(l, c_repr);
        }
        while ( l ) {
            sp_repr_add_child(repr, (SPRepr *) l->data, NULL);
            sp_repr_unref((SPRepr *) l->data);
            l = g_slist_remove(l, l->data);
        }
    } else {
        for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            if ( SP_IS_FLOWDIV(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_FLOWREGION(child) ) {
                child->updateRepr(flags);
            } else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
                child->updateRepr(flags);
            }
        }
    }

    if (((SPObjectClass *) (parent_class))->write)
        ((SPObjectClass *) (parent_class))->write(object, repr, flags);

    return repr;
}

static void
sp_flowtext_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const /*flags*/)
{
    SPFlowtext *group = SP_FLOWTEXT(item);
    if ( group->f_res ) group->f_res->BBox(bbox, transform);
}

static void
sp_flowtext_print(SPItem *item, SPPrintContext *ctx)
{
    SPFlowtext *group = SP_FLOWTEXT(item);

    NRRect pbox;
    sp_item_invoke_bbox(item, &pbox, NR::identity(), TRUE);
    NRRect bbox;
    sp_item_bbox_desktop(item, &bbox);
    NRRect dbox;
    dbox.x0 = 0.0;
    dbox.y0 = 0.0;
    dbox.x1 = sp_document_width(SP_OBJECT_DOCUMENT(item));
    dbox.y1 = sp_document_height(SP_OBJECT_DOCUMENT(item));
    NRMatrix ctm;
    sp_item_i2d_affine(item, &ctm);

    if ( group->f_res ) group->f_res->Print(ctx, &pbox, &dbox, &bbox, ctm);
}

static gchar *sp_flowtext_description(SPItem * /*item*/)
{
    //g_return_val_if_fail(SP_IS_FLOWTEXT(item), NULL);

    //return g_strdup_printf(_("Text flow"));
    return g_strdup_printf("<b>Flowed text</b>");
}

static NRArenaItem *
sp_flowtext_show(SPItem *item, NRArena *arena, unsigned/* key*/, unsigned /*flags*/)
{
    SPFlowtext *group = (SPFlowtext *) item;
    NRArenaGroup *flowed = NRArenaGroup::create(arena);
    nr_arena_group_set_transparent(flowed, FALSE);

    // pass the bbox of the flowtext object as paintbox (used for paintserver fills)	
    NRRect paintbox;
    sp_item_invoke_bbox(item, &paintbox, NR::identity(), TRUE);
    group->BuildFlow(flowed, &paintbox);

    return flowed;
}

static void
sp_flowtext_hide(SPItem *item, unsigned int key)
{
    if (((SPItemClass *) parent_class)->hide)
        ((SPItemClass *) parent_class)->hide(item, key);
}


/*
 *
 */

void SPFlowtext::ClearFlow(NRArenaGroup *in_arena)
{
    nr_arena_item_request_render(NR_ARENA_ITEM(in_arena));
    for (NRArenaItem *child = in_arena->children; child != NULL; ) {
        NRArenaItem *nchild = child->next;

        nr_arena_glyphs_group_clear(NR_ARENA_GLYPHS_GROUP(child));
        nr_arena_item_remove_child(NR_ARENA_ITEM(in_arena), child);

        child = nchild;
    }
}

void SPFlowtext::BuildFlow(NRArenaGroup *in_arena, NRRect *paintbox)
{
    if ( f_res ) f_res->Show(in_arena, paintbox);
}

static void FlowReLink(SPObject *object, one_flow_src *&after, one_flow_src *from)
{
    one_flow_src *mine = NULL;
    if ( SP_IS_FLOWTEXT(object) ) {
        mine = &(SP_FLOWTEXT(object)->contents);
        SP_FLOWTEXT(object)->contents.SetStyle(SP_OBJECT_STYLE(object));
    } else if ( SP_IS_FLOWDIV(object) ) {
        mine = &(SP_FLOWDIV(object)->contents);
        SP_FLOWDIV(object)->contents.SetStyle(SP_OBJECT_STYLE(object));
    } else if ( SP_IS_FLOWPARA(object) ) {
        mine = &(SP_FLOWPARA(object)->contents);
        SP_FLOWPARA(object)->contents.SetStyle(SP_OBJECT_STYLE(object));
    } else if ( SP_IS_FLOWTSPAN(object) ) {
        mine = &(SP_FLOWTSPAN(object)->contents);
        SP_FLOWTSPAN(object)->contents.SetStyle(SP_OBJECT_STYLE(object));
    } else if ( SP_IS_FLOWLINE(object) ) {
        mine = &(SP_FLOWLINE(object)->contents);
    } else if ( SP_IS_FLOWREGIONBREAK(object) ) {
        mine = &(SP_FLOWREGIONBREAK(object)->contents);
    } else if ( SP_IS_STRING(object) ) {
        mine = &(SP_STRING(object)->contents);
    } else {
        return;
    }

    mine->Link(after, from);
    after = mine;

    if ( SP_IS_FLOWTEXT(object) || SP_IS_FLOWTSPAN(object) || SP_IS_FLOWDIV(object) || SP_IS_FLOWPARA(object) ) {
        for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            FlowReLink(child, after, mine);
        }
    }

    // special cases
    if ( SP_IS_FLOWDIV(object) ) {
        mine = &(SP_FLOWDIV(object)->fin);
        mine->Link(after, from);
        after = mine;
    } else if ( SP_IS_FLOWPARA(object) ) {
        mine = &(SP_FLOWPARA(object)->fin);
        mine->Link(after, from);
        after = mine;
    }
}

void SPFlowtext::UpdateFlowSource()
{
    if ( f_src ) delete f_src;
    f_src = new flow_src;

    one_flow_src *last = NULL;
    FlowReLink(SP_OBJECT(this), last, NULL);
    contents.DoPositions(false);
    contents.DoFill(f_src);

    f_src->Prepare();
    //f_src->Affiche();
}

void SPFlowtext::UpdateFlowDest()
{
    SPItem *item = SP_ITEM((SPFlowtext*)this);
    SPObject *object = SP_OBJECT(item);

    if ( f_excl ) delete f_excl;
    f_excl = new flow_dest;

    //printf("update flow dest\n");

    for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if ( SP_IS_FLOWDIV(child) ) {
        } else if ( SP_IS_FLOWREGION(child) ) {
        } else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
            SPFlowregionExclude *c_child = SP_FLOWREGIONEXCLUDE(child);
            f_excl->AddShape(c_child->computed->rgn_dest);
        }
    }

    flow_dest *last_dest = NULL;
    f_dst = NULL;
    for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if ( SP_IS_FLOWDIV(child) ) {
        } else if ( SP_IS_FLOWREGION(child) ) {
            SPFlowregion *c_child = SP_FLOWREGION(child);
            for (int i = 0; i<c_child->nbComp; i++) {
                flow_dest *n_d = c_child->computed[i];
                n_d->rgn_flow->Reset();
                if ( f_excl->rgn_dest->hasEdges() ) {
                    n_d->rgn_flow->Booleen(n_d->rgn_dest, f_excl->rgn_dest, bool_op_diff);
                } else {
                    n_d->rgn_flow->Copy(n_d->rgn_dest);
                }
                n_d->Prepare();

                n_d->next_in_flow = NULL;
                if ( last_dest ) last_dest->next_in_flow = n_d;
                last_dest = n_d;
                if ( f_dst == NULL ) f_dst = n_d;
            }
        } else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
        }
    }
}

void SPFlowtext::ComputeFlowRes(void)
{
    SPItem *item = SP_ITEM((SPFlowtext*)this);
    SPObject *object = SP_OBJECT(item);

    if ( f_res ) {
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            ClearFlow(NR_ARENA_GROUP(v->arenaitem));
        }
        delete f_res;
        f_res = NULL;
    }
    if ( f_src->nbElem <= 0 ) return;
    if ( f_dst == NULL ) return;

    flow_maker *f_mak = new flow_maker(f_src, f_dst);
    f_mak->justify = justify;
    f_mak->par_indent = par_indent;
    if ( algo == 0 ) {
        f_mak->strictBefore = false;
        f_mak->strictAfter = true;
        f_mak->min_scale = 0.7;
        f_mak->max_scale = 1.0;
        f_mak->algo = 0;
    } else if ( algo == 1 ) {
        f_mak->strictBefore = false;
        f_mak->strictAfter = true;
        f_mak->min_scale = 0.8;
        f_mak->max_scale = 1.2;
        f_mak->algo = 0;
    } else {
        f_mak->strictBefore = false;
        f_mak->strictAfter = false;
        f_mak->min_scale = 0.8;
        f_mak->max_scale = 1.2;
        f_mak->algo = 1;
    }
    f_res = f_mak->Work();
    delete f_mak;

    if ( f_res ) {
        f_res->ComputeIntervals();
        //f_res->AfficheOutput();
    }

    // pass the bbox of the flowtext object as paintbox (used for paintserver fills)	
    NRRect paintbox;
    sp_item_invoke_bbox(item, &paintbox, NR::identity(), TRUE);
    for (SPItemView *v = item->display; v != NULL; v = v->next) {
        BuildFlow(NR_ARENA_GROUP(v->arenaitem), &paintbox);
    }

    for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if ( SP_IS_FLOWDIV(child) ) {
        } else if ( SP_IS_FLOWREGION(child) ) {
            SPFlowregion *c_child = SP_FLOWREGION(child);
            for (int i = 0; i<c_child->nbComp; i++) {
                flow_dest *n_d = c_child->computed[i];
                n_d->UnPrepare();
            }
        } else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
        }
    }
}

void convert_to_text(void)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!SP_IS_DESKTOP(desktop)) return;
    SPSelection *selection = SP_DT_SELECTION(desktop);
    SPItem *item = selection->singleItem();
    SPObject *object = SP_OBJECT(item);
    if ( SP_IS_FLOWTEXT(object) == false ) return;

    SPFlowtext *group = SP_FLOWTEXT(object);
    group->UpdateFlowSource();
    group->UpdateFlowDest();
    group->ComputeFlowRes();

    flow_res *comp = group->f_res;
    if ( comp == NULL || comp->nbGroup <= 0 || comp->nbGlyph <= 0 || comp->nbChunk <= 0) {
        // exmpty text, no need to produce anything...
        return;
    }

    selection->clear();

    SPRepr *parent = SP_OBJECT_REPR(object)->parent;
    SPRepr *repr = sp_repr_new("text");
    sp_repr_set_attr(repr, "style", sp_repr_attr(SP_OBJECT_REPR(object), "style"));
    sp_repr_append_child(parent, repr);
    // add a tspan for each chunk of the flow
    double last_kern_y = 0;
    for (int i = 0; i < comp->nbChunk; i++) {
        double chunk_spc = comp->chunks[i].spacing;
        if ( comp->chunks[i].rtl ) chunk_spc = -chunk_spc;
        for (int j = comp->chunks[i].s_st; j < comp->chunks[i].s_en; j++) {
            if ( comp->spans[j].l_st < comp->spans[j].l_en && comp->spans[j].c_style && comp->spans[j].c_style->with_style ) {
                SPRepr *srepr = sp_repr_new("tspan");
                // set the good font family (may differ if pango needed a different one)
                gchar *nstyle = NULL;
                text_style *curS = comp->spans[j].c_style;
                font_instance *curF = curS->theFont;
                SPStyle *curSPS = curS->with_style;
                char const *curFam = pango_font_description_get_family(curF->descr);
                {
                    char *savFam = curSPS->text->font_family.value;
                    curS->with_style->text->font_family.value = (gchar*)curFam;
                    SPILength sav_spc = curSPS->text->letterspacing;
                    SPIEnum sav_anc = curSPS->text_anchor;
                    curSPS->text->letterspacing.set = 1;
                    curSPS->text->letterspacing.inherit = 0;
                    curSPS->text->letterspacing.unit = SP_CSS_UNIT_PX;
                    if ( comp->spans[j].rtl == comp->chunks[i].rtl ) {
                        curSPS->text->letterspacing.computed = chunk_spc;
                    } else {
                        curSPS->text->letterspacing.computed = -chunk_spc;
                    }
                    curSPS->text->letterspacing.value = curSPS->text->letterspacing.computed;
                    curSPS->text_anchor.set = 1;
                    if ( comp->spans[j].rtl ) {
                        curSPS->text_anchor.value = curSPS->text_anchor.computed = SP_CSS_TEXT_ANCHOR_END;
                    } else {
                        curSPS->text_anchor.value = curSPS->text_anchor.computed = SP_CSS_TEXT_ANCHOR_START;
                    }
                    nstyle = sp_style_write_string(curSPS, SP_STYLE_FLAG_ALWAYS);
                    curSPS->text->letterspacing = sav_spc;
                    curSPS->text_anchor = sav_anc;
                    curS->with_style->text->font_family.value = savFam;
                }
                if ( comp->spans[j].rtl ) {
                    sp_repr_set_double(srepr, "x", comp->spans[j].x_en);
                } else {
                    sp_repr_set_double(srepr, "x", comp->spans[j].x_st);
                }
                sp_repr_set_double(srepr, "y", comp->chunks[i].y);
                sp_repr_set_attr(srepr, "style", nstyle);
                g_free(nstyle);

                {
                    bool zero = true;
                    for (int k = comp->spans[j].l_st; k<comp->spans[j].l_en; k++) {
                        if ( fabs(comp->letters[k].kern_x) > 0.1 ) {zero = false; break;}
                    }
                    if ( zero == false ) {
                        gchar c[32];
                        gchar *s = NULL;

                        for (int k = comp->spans[j].l_st; k<comp->spans[j].l_en; k++) {
                            int u4_l = 0, u8_l = comp->letters[k].t_en-comp->letters[k].t_st;
                            for (char *p = comp->chars+comp->letters[k].t_st; p && *p; p = g_utf8_next_char(p)) {
                                int d= ((int)p)-((int)comp->chars);
                                d-=comp->letters[k].t_st;
                                if ( d >= u8_l ) break;
                                u4_l++;
                            }
                            for (int n = 0; n<u4_l; n++) {
                                g_ascii_formatd(c, sizeof(c), "%.8g", comp->letters[k].kern_x);
                                if ( s == NULL ) {
                                    s = g_strdup(c);
                                } else {
                                    s = g_strjoin(" ", s, c, NULL);
                                }
                            }
                        }
                        sp_repr_set_attr(srepr, "dx", s);
                        g_free(s);
                    }
                }
                {
                    bool zero = true;
                    for (int k = comp->spans[j].l_st; k<comp->spans[j].l_en; k++) {
                        if ( fabs(comp->letters[k].kern_y-last_kern_y) > 0.1 ) {zero = false; break;}
                    }
                    if ( zero == false ) {
                        gchar c[32];
                        gchar *s = NULL;

                        for (int k = comp->spans[j].l_st; k<comp->spans[j].l_en; k++) {
                            int u4_l = 0, u8_l = comp->letters[k].t_en-comp->letters[k].t_st;
                            for (char *p = comp->chars+comp->letters[k].t_st; p && *p; p = g_utf8_next_char(p)) {
                                int d = ((int)p)-((int)comp->chars);
                                d -= comp->letters[k].t_st;
                                if ( d >= u8_l ) break;
                                u4_l++;
                            }
                            for (int n = 0; n<u4_l; n++) {
                                g_ascii_formatd(c, sizeof(c), "%.8g", comp->letters[k].kern_y-last_kern_y);
                                last_kern_y = comp->letters[k].kern_y;
                                if ( s == NULL ) {
                                    s = g_strdup(c);
                                } else {
                                    s = g_strjoin(" ", s, c, NULL);
                                }
                            }
                        }
                        sp_repr_set_attr(srepr, "dy", s);
                        g_free(s);
                    }
                }

                int t_st = comp->letters[comp->spans[j].l_st].t_st;
                int t_en = comp->letters[comp->spans[j].l_en-1].t_en;
                char savC = comp->chars[t_en];
                comp->chars[t_en] = 0;
                SPRepr *rstr = sp_xml_document_createTextNode(sp_repr_document(repr), comp->chars+t_st);
                comp->chars[t_en] = savC;
                sp_repr_append_child(srepr, rstr);
                sp_repr_unref(rstr);

                sp_repr_append_child(repr, srepr);
                sp_repr_unref(srepr);
            }
        }
    }
    SPItem *nitem = (SPItem *) SP_DT_DOCUMENT(desktop)->getObjectByRepr(repr);
    sp_item_write_transform(nitem, repr, item->transform);
    SP_OBJECT(nitem)->updateRepr();

    sp_repr_unref(repr);
    selection->setItem(nitem);
    object->deleteObject();

    sp_document_done(SP_DT_DOCUMENT(desktop));
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
