#define __SP_FLOWTEXT_C__

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

#include "helper/sp-intl.h"

#include "libnr/nr-matrix.h"
#include "libnr/nr-point.h"
#include "xml/repr.h"
#include "xml/repr-private.h"

#include "sp-flowdiv.h"
#include "sp-flowregion.h"
#include "sp-flowtext.h"

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
#include "libnrtype/FlowEater.h"
#include "libnrtype/text_style.h"

#include "livarot/Shape.h"

#include "display/nr-arena-item.h"
#include "display/nr-arena-group.h"
#include "display/nr-arena-glyphs.h"

#include <pango/pango.h>

static void sp_flowtext_class_init (SPFlowtextClass *klass);
static void sp_flowtext_init (SPFlowtext *group);
static void sp_flowtext_dispose (GObject *object);

static void sp_flowtext_child_added (SPObject * object, SPRepr * child, SPRepr * ref);
static void sp_flowtext_remove_child (SPObject * object, SPRepr * child);
static void sp_flowtext_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_flowtext_modified (SPObject *object, guint flags);
static SPRepr *sp_flowtext_write (SPObject *object, SPRepr *repr, guint flags);
static void sp_flowtext_build(SPObject *object, SPDocument *document, SPRepr *repr);
static void sp_flowtext_set(SPObject *object, unsigned key, gchar const *value);

static void sp_flowtext_bbox(SPItem *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags);
static void sp_flowtext_print (SPItem * item, SPPrintContext *ctx);
static gchar * sp_flowtext_description (SPItem * item);
static NRArenaItem *sp_flowtext_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_flowtext_hide (SPItem * item, unsigned int key);

static SPItemClass * parent_class;

GType
sp_flowtext_get_type (void)
{
	static GType group_type = 0;
	if (!group_type) {
		GTypeInfo group_info = {
			sizeof (SPFlowtextClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_flowtext_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPFlowtext),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_flowtext_init,
			NULL,	/* value_table */
		};
		group_type = g_type_register_static (SP_TYPE_ITEM, "SPFlowtext", &group_info, (GTypeFlags)0);
	}
	return group_type;
}

static void
sp_flowtext_class_init (SPFlowtextClass *klass)
{
	GObjectClass * object_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	
	object_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;
	
	parent_class = (SPItemClass *)g_type_class_ref (SP_TYPE_ITEM);
	
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
sp_flowtext_init (SPFlowtext *group)
{
	group->f_dst=group->f_excl=NULL;
	group->f_src=NULL;
	group->f_res=NULL;
	group->justify=false;
	group->par_indent=0;
	group->algo=0;
}

static void
sp_flowtext_dispose(GObject *object)
{
	SPFlowtext* group=(SPFlowtext*)object;
	
	//	if ( group->f_dst ) delete group->f_dst;
	if ( group->f_excl ) delete group->f_excl;
	if ( group->f_res ) delete group->f_res;
	if ( group->f_src ) delete group->f_src;
}

static void
sp_flowtext_child_added (SPObject *object, SPRepr *child, SPRepr *ref)
{
	SPItem *item;
	
	item = SP_ITEM (object);
	
	if (((SPObjectClass *) (parent_class))->child_added)
		(* ((SPObjectClass *) (parent_class))->child_added) (object, child, ref);
	
	object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: hide (Lauris) */

static void
sp_flowtext_remove_child (SPObject * object, SPRepr * child)
{
	if (((SPObjectClass *) (parent_class))->remove_child)
		(* ((SPObjectClass *) (parent_class))->remove_child) (object, child);
	
	object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_flowtext_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPFlowtext *group;
	SPObject *child;
	SPItemCtx *ictx, cctx;
	GSList *l;
	
	group = SP_FLOWTEXT (object);
	ictx = (SPItemCtx *) ctx;
	cctx = *ictx;
	
	if (((SPObjectClass *) (parent_class))->update)
		((SPObjectClass *) (parent_class))->update (object, ctx, flags);
	
	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;
	
	l = NULL;
	for (child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			if (SP_IS_ITEM (child)) {
				SPItem const &chi = *SP_ITEM(child);
				cctx.i2doc = chi.transform * ictx->i2doc;
				cctx.i2vp = chi.transform * ictx->i2vp;
				child->updateDisplay((SPCtx *)&cctx, flags);
			} else {
				child->updateDisplay(ctx, flags);
			}
		}
		g_object_unref (G_OBJECT (child));
	}
	
	group->UpdateFlowSource();
	group->UpdateFlowDest();
	group->ComputeFlowRes();
}

static void
sp_flowtext_modified (SPObject */*object*/, guint /*flags*/)
{
	/*	SPFlowtext *group;
	
	group = SP_FLOWTEXT (object);*/
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
    case SP_ATTR_LAYOUT_OPTIONS:
    {
      SPCSSAttr * opts=sp_repr_css_attr ((SP_OBJECT(group))->repr, "inkscape:layoutOptions");
      {
        const gchar * val=sp_repr_css_property (opts,"justification", NULL);
        if ( val == NULL ) {
          group->justify=false;
        } else {
          if ( strcmp(val,"0") == 0 || strcmp(val,"false") == 0 ) {
            group->justify=false;
          } else {
            group->justify=true;
          }
        }
      }
      {
        const gchar * val=sp_repr_css_property (opts,"layoutAlgo", NULL);
        if ( val == NULL ) {
          group->algo=0;
        } else {
          if ( strcmp(val,"better") == 0 ) {
            group->algo=2;
          } else if ( strcmp(val,"simple") == 0 ) {
            group->algo=1;
          } else if ( strcmp(val,"default") == 0 ) {
            group->algo=0;
          }
        }
      }
      {
        const gchar * val=sp_repr_css_property (opts,"par-indent", NULL);
        if ( val == NULL ) {
          group->par_indent=0.0;
        } else {
					sp_repr_get_double((SPRepr*)opts,"par-indent",&group->par_indent);
        }
      }
      sp_repr_css_attr_unref (opts);
			object->requestModified(SP_OBJECT_MODIFIED_FLAG);
    }
      break;
		default:
			if (((SPObjectClass *) (parent_class))->set) {
				(* ((SPObjectClass *) (parent_class))->set)(object, key, value);
			}
			break;
	}
}
static SPRepr *
sp_flowtext_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPFlowtext *group;
	
	group = SP_FLOWTEXT (object);
	
	if ( flags&SP_OBJECT_WRITE_BUILD ) {
		if ( repr == NULL ) repr = sp_repr_new ("flowRoot");
		GSList *l = NULL;
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			SPRepr* c_repr=NULL;
			if ( SP_IS_FLOWDIV (child) ) {
				c_repr = child->updateRepr(NULL, flags);
			} else if ( SP_IS_FLOWREGION (child) ) {
				c_repr = child->updateRepr(NULL, flags);
			} else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
				c_repr = child->updateRepr(NULL, flags);
			}
			if ( c_repr ) l = g_slist_prepend (l, c_repr);
		}
		while ( l ) {
			sp_repr_add_child (repr, (SPRepr *) l->data, NULL);
			sp_repr_unref ((SPRepr *) l->data);
			l = g_slist_remove (l, l->data);
		}
	} else {
		for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
			if ( SP_IS_FLOWDIV (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_FLOWREGION (child) ) {
				child->updateRepr(flags);
			} else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
				child->updateRepr(flags);
			}
		}
	}
	
	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, repr, flags);
	
	return repr;
}

static void
sp_flowtext_bbox(SPItem *item, NRRect *bbox, NR::Matrix const &transform, unsigned const /*flags*/)
{
	SPFlowtext * group;
	
	group = SP_FLOWTEXT (item);
	
	flow_res*  comp=group->f_res;
	if ( comp && comp->nbGroup > 0 && comp->nbGlyph > 0 ) {
		for (int i=0;i<comp->nbGroup;i++) {
			NR::Matrix  f_tr(NR::scale(comp->groups[i].style->with_style->font_size.computed,-comp->groups[i].style->with_style->font_size.computed));
			font_instance*  curF=comp->groups[i].style->theFont;
			if ( curF ) {
				for (int j=comp->groups[i].st;j<comp->groups[i].en;j++) {
					NR::Matrix  g_tr(NR::translate(comp->glyphs[j].g_x,comp->glyphs[j].g_y));
					g_tr=f_tr*g_tr;
					NR::Matrix   tot_tr=g_tr*transform;
					NR::Rect  res=curF->BBox(comp->glyphs[j].g_id);
					NR::Point bmi=res.min(),bma=res.max();
					NR::Point tlp(bmi[0],bmi[1]),trp(bma[0],bmi[1]),blp(bmi[0],bma[1]),brp(bma[0],bma[1]);
					tlp=tlp*tot_tr;
					trp=trp*tot_tr;
					blp=blp*tot_tr;
					brp=brp*tot_tr;
					res=NR::Rect(tlp,trp);
					res.expandTo(blp);
					res.expandTo(brp);
					if ( (res.min())[0] < bbox->x0 ) bbox->x0=(res.min())[0];
					if ( (res.max())[0] > bbox->x1 ) bbox->x1=(res.max())[0];
					if ( (res.min())[1] < bbox->y0 ) bbox->y0=(res.min())[1];
					if ( (res.max())[1] > bbox->y1 ) bbox->y1=(res.max())[1];
				}
			}
		}
	}
}

static void
sp_flowtext_print (SPItem * item, SPPrintContext *ctx)
{
	NRRect     pbox, dbox, bbox;
	SPFlowtext *group = SP_FLOWTEXT (item);
	
	sp_item_invoke_bbox(item, &pbox, NR::identity(), TRUE);
	sp_item_bbox_desktop (item, &bbox);
	dbox.x0 = 0.0;
	dbox.y0 = 0.0;
	dbox.x1 = sp_document_width (SP_OBJECT_DOCUMENT (item));
	dbox.y1 = sp_document_height (SP_OBJECT_DOCUMENT (item));
	NRMatrix ctm;
	sp_item_i2d_affine (item, &ctm);
	
	flow_res*  comp=group->f_res;
	if ( comp && comp->nbGroup > 0 && comp->nbGlyph > 0 && comp->nbChar > 0 ) {
		bool text_to_path=ctx->module->textToPath();
		if ( text_to_path ) {
			for (int i=0;i<comp->nbGroup;i++) {
				text_style*     curS=comp->groups[i].style;
				font_instance*  curF=curS->theFont;
				const char*     curFam=pango_font_description_get_family(curF->descr);
				char*           savFam=curS->with_style->text->font_family.value;
				curS->with_style->text->font_family.value=(gchar*)curFam;
				NR::Matrix  f_tr(NR::scale(comp->groups[i].style->with_style->font_size.computed,-comp->groups[i].style->with_style->font_size.computed));
				for (int j=comp->groups[i].st;j<comp->groups[i].en;j++) {
					NR::Matrix  g_tr(NR::translate(comp->glyphs[j].g_x,comp->glyphs[j].g_y));
					g_tr=f_tr*g_tr;
					NRBPath     bpath;
					if ( curF ) bpath.path=(NArtBpath*)curF->ArtBPath(comp->glyphs[j].g_id); else bpath.path=NULL;
					if ( bpath.path ) {
						NRBPath abp;
						abp.path = nr_artpath_affine (bpath.path, g_tr);
						if (curS->with_style->fill.type != SP_PAINT_TYPE_NONE) {
							sp_print_fill (ctx, &abp, &ctm, curS->with_style, &pbox, &dbox, &bbox);
						}
						
						if (curS->with_style->stroke.type != SP_PAINT_TYPE_NONE) {
							sp_print_stroke (ctx, &abp, &ctm, curS->with_style, &pbox, &dbox, &bbox);
						}
						nr_free (abp.path);
					}
				}
				curS->with_style->text->font_family.value=savFam;
			}
		} else {
			for (int i=0;i<comp->nbGroup;i++) {
				text_style*     curS=comp->groups[i].style;
				font_instance*  curF=curS->theFont;
				const char*     curFam=pango_font_description_get_family(curF->descr);
				char*           savFam=curS->with_style->text->font_family.value;
				curS->with_style->text->font_family.value=(gchar*)curFam;
				for (int j=comp->groups[i].st;j<comp->groups[i].en;j++) {
					NR::Point   g_pos(comp->glyphs[j].g_x,comp->glyphs[j].g_y);
					char*       g_txt=comp->chars+comp->glyphs[j].g_st;
					int         g_len=comp->glyphs[j].g_en-comp->glyphs[j].g_st;
					char savC=g_txt[g_len];
					g_txt[g_len]=0;
					sp_print_text (ctx, g_txt, g_pos, curS->with_style);
					g_txt[g_len]=savC;
				}
				curS->with_style->text->font_family.value=savFam;
			}
		}
	}
	
}

static gchar * sp_flowtext_description (SPItem * /*item*/)
{
	return g_strdup_printf(_("Flowed text"));
}

static NRArenaItem *
sp_flowtext_show (SPItem *item, NRArena *arena, unsigned int/* key*/, unsigned int /*flags*/)
{
	SPFlowtext *group;
	group = (SPFlowtext *) item;
	
	NRArenaGroup* flowed=NRArenaGroup::create(arena);
	nr_arena_group_set_transparent (flowed, FALSE);
	
	group->BuildFlow(flowed);
	
	return flowed;
}

static void
sp_flowtext_hide (SPItem *item, unsigned int key)
{
	SPFlowtext * group;
	
	group = (SPFlowtext *) item;
	
	if (((SPItemClass *) parent_class)->hide)
		((SPItemClass *) parent_class)->hide (item, key);
}


/*
 *
 */

flow_res::flow_res(void)
{
	nbGlyph=maxGlyph=0;
	glyphs=NULL;
	nbGroup=maxGroup=0;
	groups=NULL;
	nbChar=maxChar=0;
	chars=NULL;
	nbChunk=maxChunk=0;
	chunks=NULL;
}
flow_res::~flow_res(void)
{
	for (int i=0;i<nbChunk;i++) {
		if ( chunks[i].c_txt ) free(chunks[i].c_txt);
		if ( chunks[i].kern_x ) free(chunks[i].kern_x);
		if ( chunks[i].kern_y ) free(chunks[i].kern_y);
	}
	if ( chunks ) free(chunks);
	if ( chars ) free(chars);
	if ( glyphs ) free(glyphs);
	if ( groups ) free(groups);
	nbGlyph=maxGlyph=0;
	glyphs=NULL;
	nbGroup=maxGroup=0;
	groups=NULL;
	nbChar=maxChar=0;
	chars=NULL;
	nbChunk=maxChunk=0;
	chunks=NULL;
}
void               flow_res::Reset(void)
{
	nbGlyph=nbGroup=0;
	nbChar=0;
	last_c_style=NULL;
	cur_spacing=0;
}
void               flow_res::AddGroup(text_style* g_s)
{
	if ( nbGroup >= maxGroup ) {
		maxGroup=2*nbGroup+1;
		groups=(flow_glyph_group*)realloc(groups,maxGroup*sizeof(flow_glyph_group));
	}
	groups[nbGroup].st=nbGlyph;
	groups[nbGroup].en=nbGlyph;
	groups[nbGroup].style=g_s;
	groups[nbGroup].g_gr=NULL;
	nbGroup++;
}
void               flow_res::AddGlyph(int g_id,double g_x,double g_y,text_style* g_s)
{
	if ( nbGlyph >= maxGlyph ) {
		maxGlyph=2*nbGlyph+1;
		glyphs=(flow_glyph*)realloc(glyphs,maxGlyph*sizeof(flow_glyph));
	}
	if ( nbGroup <= 0 || groups[nbGroup-1].style != g_s ) AddGroup(g_s);
	glyphs[nbGlyph].g_id=g_id;
	glyphs[nbGlyph].g_x=g_x;
	glyphs[nbGlyph].g_y=g_y;
	glyphs[nbGlyph].g_font=NULL;
	glyphs[nbGlyph].g_gl=NULL;
	glyphs[nbGlyph].g_st=glyphs[nbGlyph].g_en=nbChar;
	nbGlyph++;
	groups[nbGroup-1].en=nbGlyph;
}
void							 flow_res::SetLastText(char* iText,int iLen)
{
	if ( iLen <= 0 ) return;
	if ( nbChar+iLen >= maxChar ) {
		maxChar=2*nbChar+iLen;
		chars=(char*)realloc(chars,(maxChar+1)*sizeof(char));
	}
	memcpy(chars+nbChar,iText,iLen*sizeof(char));
	int  old_nb=nbChar;
	nbChar+=iLen;
	if ( nbGlyph > 0 ) {
		glyphs[nbGlyph-1].g_st=old_nb;
		glyphs[nbGlyph-1].g_en=nbChar;
	}
}
void							 flow_res::AddChunk(char* iText,int iLen,text_style* i_style,double x,double y,bool rtl)
{
	if ( i_style == NULL ) {
		last_c_style=NULL;
		return;
	}
	if ( iLen <= 0 ) return;
	gunichar  nc=g_utf8_get_char(iText);
	bool      is_white=g_unichar_isspace(nc);
	double    the_x=x;
	if ( last_c_style != i_style ) {
		if ( is_white ) {
			// will be eaten at the start of a tspan, so we skip it
			return;
		}
		if ( nbChunk >= maxChunk ) {
			maxChunk=2*nbChunk+1;
			chunks=(flow_styled_chunk*)realloc(chunks,maxChunk*sizeof(flow_styled_chunk));
		}
		chunks[nbChunk].c_txt=(char*)malloc(sizeof(char));
		chunks[nbChunk].c_len=0;
		chunks[nbChunk].c_txt[0]=0;
		chunks[nbChunk].c_style=i_style;
		chunks[nbChunk].x=x;
		chunks[nbChunk].y=y;
		chunks[nbChunk].last_add=0;
		chunks[nbChunk].c_ucs4_l=0;
		chunks[nbChunk].kern_x=chunks[nbChunk].kern_y=NULL;
		chunks[nbChunk].spc=cur_spacing;
		nbChunk++;
	} else {
		if ( nbChunk > 0 ) {
			if ( rtl == false || iText[0] == ' ' ) the_x=chunks[nbChunk-1].x;
		}
	}
	last_c_style=i_style;
	if ( nbChunk <= 0 ) return;
	flow_styled_chunk* cur=chunks+(nbChunk-1);
	cur->x=the_x;
	cur->c_txt=(char*)realloc(cur->c_txt,(cur->c_len+iLen+1)*sizeof(char));
	memcpy(cur->c_txt+cur->c_len,iText,iLen*sizeof(char));
	cur->c_len+=iLen;
	cur->c_txt[cur->c_len]=0;
	int  ucs4_add=0;
	for (char* p=iText;*p;p=g_utf8_next_char(p)) {
		int d=((int)p)-((int)iText);
		if ( d >= iLen ) break;
		ucs4_add++;
	}
	cur->last_add=cur->c_ucs4_l;
	cur->c_ucs4_l+=ucs4_add;
	if ( cur->kern_x ) {
		cur->kern_x=(double*)realloc(cur->kern_x,cur->c_ucs4_l*sizeof(double));
		for (int i=cur->last_add;i<cur->c_ucs4_l;i++) cur->kern_x[i]=0;
	}
	if ( cur->kern_y ) {
		cur->kern_y=(double*)realloc(cur->kern_y,cur->c_ucs4_l*sizeof(double));
		for (int i=cur->last_add;i<cur->c_ucs4_l;i++) cur->kern_y[i]=0;
	}
}
void              flow_res::KernLastAddition(double* with,bool is_x)
{
	if ( nbChunk <= 0 ) return;
	flow_styled_chunk* cur=chunks+(nbChunk-1);
	if ( is_x ) {
		if ( cur->kern_x == NULL ) {
			cur->kern_x=(double*)malloc(cur->c_ucs4_l*sizeof(double));
			for (int i=0;i<cur->c_ucs4_l;i++) cur->kern_x[i]=0;
		}
		for (int i=cur->last_add;i<cur->c_ucs4_l;i++) cur->kern_x[i]=with[i-cur->last_add];
	} else {
		if ( cur->kern_y == NULL ) {
			cur->kern_y=(double*)malloc(cur->c_ucs4_l*sizeof(double));
			for (int i=0;i<cur->c_ucs4_l;i++) cur->kern_y[i]=0;
		}
		for (int i=cur->last_add;i<cur->c_ucs4_l;i++) cur->kern_y[i]=with[i-cur->last_add];
	}
}
void              flow_res::AfficheChunks(void)
{
	printf("%i chunks\n",nbChunk);
	for (int i=0;i<nbChunk;i++) {
		printf("txt=%s pos=(%f %f) spc=%f\n",chunks[i].c_txt,chunks[i].x,chunks[i].y,chunks[i].spc);
	}
}
/*
 *
 */

void              SPFlowtext::ClearFlow(NRArenaGroup* in_arena)
{
	for (int i=0;i<f_res->nbGroup;i++) if ( f_res->groups[i].g_gr ) nr_arena_glyphs_group_clear(f_res->groups[i].g_gr);
	
	nr_arena_item_request_render (NR_ARENA_ITEM (in_arena));
	while (in_arena->children) {
		nr_arena_item_remove_child (NR_ARENA_ITEM (in_arena), in_arena->children);
	}
	
	for (int i=0;i<f_res->nbGroup;i++) f_res->groups[i].g_gr=NULL;
	for (int i=0;i<f_res->nbGlyph;i++) f_res->glyphs[i].g_gl=NULL;
}
void              SPFlowtext::BuildFlow(NRArenaGroup* in_arena)
{
	flow_res*  comp=f_res;
	if ( comp && comp->nbGroup > 0 && comp->nbGlyph > 0 ) {
		for (int i=0;i<comp->nbGroup;i++) {
			comp->groups[i].g_gr = NRArenaGlyphsGroup::create(in_arena->arena);
			nr_arena_item_add_child (in_arena, comp->groups[i].g_gr, NULL);
			nr_arena_item_unref (comp->groups[i].g_gr);
			
			nr_arena_glyphs_group_set_style (comp->groups[i].g_gr, comp->groups[i].style->with_style);
			font_instance* curF=comp->groups[i].style->theFont;
			NR::Matrix f_tr(NR::scale(comp->groups[i].style->with_style->font_size.computed,-comp->groups[i].style->with_style->font_size.computed));
			for (int j=comp->groups[i].st;j<comp->groups[i].en;j++) {
				NR::Matrix  g_tr(NR::translate(comp->glyphs[j].g_x,comp->glyphs[j].g_y));
				const NRMatrix  g_mat=f_tr*g_tr;
				nr_arena_glyphs_group_add_component (comp->groups[i].g_gr, curF, comp->glyphs[j].g_id, &g_mat);
			}
		}
		nr_arena_item_request_update (NR_ARENA_ITEM (in_arena), NR_ARENA_ITEM_STATE_ALL, FALSE);
		//comp->AfficheChunks();
	}
}

void              SPFlowtext::UpdateFlowSource(void)
{
	SPItem*   item=SP_ITEM((SPFlowtext*)this);
	SPObject* object=SP_OBJECT(item);
	
	if ( f_src ) delete f_src;
	f_src=new flow_src;
	
	text_style*      n_style=new text_style;
	const SPStyle*   c_style = SP_OBJECT_STYLE (object);
	const double     c_size = c_style->font_size.computed;
	font_instance*   c_face = (font_factory::Default())->Face(c_style->text->font_family.value, font_style_to_pos(*c_style));
	if ( c_face == NULL ) {
		// bail out
		delete n_style;
		return;
	}
	n_style->SetFont(c_face,c_size,0.0);
	c_face->Unref();
	
	f_src->Push(n_style);
	
	for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		if ( SP_IS_FLOWDIV (child) ) {
			SPFlowdiv*   c_child=SP_FLOWDIV(child);
			c_child->CollectFlow(f_src);
		} else if ( SP_IS_FLOWREGION (child) ) {
		} else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
		}
	}
	
	f_src->Pop();
	
	f_src->Prepare();
	//	f_src->Affiche();
}
void              SPFlowtext::UpdateFlowDest(void)
{
	SPItem*   item=SP_ITEM((SPFlowtext*)this);
	SPObject* object=SP_OBJECT(item);
	
	if ( f_excl ) delete f_excl;
	f_excl=new flow_dest;
	
	//	printf("update flow dest\n");
	
	for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		if ( SP_IS_FLOWDIV (child) ) {
		} else if ( SP_IS_FLOWREGION (child) ) {
		} else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
			SPFlowregionExclude*   c_child=SP_FLOWREGIONEXCLUDE(child);
			f_excl->AddShape(c_child->computed->rgn_dest);
		}
	}
	
	flow_dest*  last_dest=NULL;
	f_dst=NULL;
	for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		if ( SP_IS_FLOWDIV (child) ) {
		} else if ( SP_IS_FLOWREGION (child) ) {
			SPFlowregion*   c_child=SP_FLOWREGION(child);
			for (int i=0;i<c_child->nbComp;i++) {
				flow_dest* n_d=c_child->computed[i];
				n_d->rgn_flow->Reset();
				if ( f_excl->rgn_dest->aretes.size() > 0 ) {
					n_d->rgn_flow->Booleen(n_d->rgn_dest,f_excl->rgn_dest,bool_op_diff);
				} else {
					n_d->rgn_flow->Copy(n_d->rgn_dest);
				}
				n_d->Prepare();
				
				n_d->next_in_flow=NULL;
				if ( last_dest ) last_dest->next_in_flow=n_d;
				last_dest=n_d;
				if ( f_dst == NULL ) f_dst=n_d;
			}
		} else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
		}
	}
}
void              SPFlowtext::ComputeFlowRes(void)
{
	SPItem*   item=SP_ITEM((SPFlowtext*)this);
	SPObject* object=SP_OBJECT(item);
	
	if ( f_res ) {
		for (SPItemView* v = item->display; v != NULL; v = v->next) {
			ClearFlow(NR_ARENA_GROUP(v->arenaitem));
		}
		delete f_res;
		f_res=NULL;
	}
	if ( f_src->nbElem <= 0 ) return;
	if ( f_dst == NULL ) return;
	
	flow_maker* f_mak=new flow_maker(f_src,f_dst);
	f_mak->justify=justify;
	f_mak->par_indent=par_indent;
	if ( algo == 0 ) {
		f_mak->strictBefore=false;
		f_mak->strictAfter=true;
		f_mak->min_scale=0.7;
		f_mak->max_scale=1.0;
		f_mak->algo=0;
	} else if ( algo == 1 ) {
		f_mak->strictBefore=false;
		f_mak->strictAfter=true;
		f_mak->min_scale=0.8;
		f_mak->max_scale=1.2;
		f_mak->algo=0;
	} else {
		f_mak->strictBefore=false;
		f_mak->strictAfter=false;
		f_mak->min_scale=0.8;
		f_mak->max_scale=1.2;
		f_mak->algo=1;
	}
	f_res=f_mak->Work();
	delete f_mak;
	
	for (SPItemView* v = item->display; v != NULL; v = v->next) {
		BuildFlow(NR_ARENA_GROUP(v->arenaitem));
	}
	
	for (SPObject* child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		if ( SP_IS_FLOWDIV (child) ) {
		} else if ( SP_IS_FLOWREGION (child) ) {
			SPFlowregion*   c_child=SP_FLOWREGION(child);
			for (int i=0;i<c_child->nbComp;i++) {
				flow_dest* n_d=c_child->computed[i];
				n_d->UnPrepare();
			}
		} else if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
		}
	}
}

void convert_to_text(void)
{
	SPDesktop*   desktop = SP_ACTIVE_DESKTOP;
	if (!SP_IS_DESKTOP (desktop)) return;
	SPSelection* selection = SP_DT_SELECTION (desktop);
	SPItem*      item = selection->singleItem();
	SPObject*    object=SP_OBJECT(item);
	if ( SP_IS_FLOWTEXT(object) == false ) return;
	SPFlowtext*  group=SP_FLOWTEXT(object);
	
	group->UpdateFlowSource();
	group->UpdateFlowDest();
	group->ComputeFlowRes();
	
	flow_res*  comp=group->f_res;
	if ( comp == NULL || comp->nbGroup <= 0 || comp->nbGlyph <= 0 || comp->nbChunk <= 0) {
		// exmpty text, no need to produce anything...
		return;
	}
	
	selection->clear();

	SPRepr  *parent = SP_OBJECT_REPR (object)->parent;
	SPRepr  *repr = sp_repr_new ("text");

	sp_repr_set_attr (repr, "style", sp_repr_attr (SP_OBJECT_REPR (object), "style"));

	sp_repr_set_attr (repr, "xml:space", "preserve"); // we preserve spaces in the text objects we create

	sp_repr_append_child (parent, repr);
	// add a tspan for each chunk of the flow
	for (int i=0;i<comp->nbChunk;i++) {
		if ( comp->chunks[i].c_len > 0 && comp->chunks[i].c_style && comp->chunks[i].c_style->with_style ) {
			SPRepr  *srepr = sp_repr_new ("tspan");
			// set the good font family (may differ if pango needed a different one)
			text_style*     curS=comp->chunks[i].c_style;
			font_instance*  curF=curS->theFont;
			SPStyle*        curSPS=curS->with_style;
			const char*     curFam=pango_font_description_get_family(curF->descr);
			char*           savFam=curSPS->text->font_family.value;
			curS->with_style->text->font_family.value=(gchar*)curFam;
			SPILength				sav_spc=curSPS->text->letterspacing;
			curSPS->text->letterspacing.set=1;
			curSPS->text->letterspacing.inherit=0;
			curSPS->text->letterspacing.unit=SP_CSS_UNIT_PX;
			curSPS->text->letterspacing.computed=comp->chunks[i].spc;
			curSPS->text->letterspacing.value=comp->chunks[i].spc;
			gchar   *nstyle=sp_style_write_string (comp->chunks[i].c_style->with_style ,SP_STYLE_FLAG_ALWAYS);
			curSPS->text->letterspacing=sav_spc;
			curS->with_style->text->font_family.value=savFam;
			
			sp_repr_set_double (srepr, "x", comp->chunks[i].x);
			sp_repr_set_double (srepr, "y", comp->chunks[i].y);
			sp_repr_set_attr (srepr, "style", nstyle);
			g_free(nstyle);
			
			if ( comp->chunks[i].kern_x ) {
				bool zero=true;
				for (int j=0;j<comp->chunks[i].c_ucs4_l;j++) {
					if ( fabs(comp->chunks[i].kern_x[j]) > 0.1 ) {zero=false;break;}
				}
				if ( zero == false ) {
					gchar c[32];
					gchar *s = NULL;
					
					for (int j=0;j<comp->chunks[i].c_ucs4_l;j++) {
					g_ascii_formatd (c, sizeof (c), "%.8g", comp->chunks[i].kern_x[j]);
						if ( s == NULL ) {
							s = g_strdup (c);
						}  else {
							s = g_strjoin (" ", s, c, NULL);
						}
					}
					sp_repr_set_attr (srepr, "dx", s);
					g_free(s);
				}
			}
			if ( comp->chunks[i].kern_y ) {
				bool zero=true;
				for (int j=0;j<comp->chunks[i].c_ucs4_l;j++) {
					if ( fabs(comp->chunks[i].kern_y[j]) > 0.1 ) {zero=false;break;}
				}
				if ( zero == false ) {
					gchar c[32];
					gchar *s = NULL;
					
					for (int j=0;j<comp->chunks[i].c_ucs4_l;j++) {
						g_ascii_formatd (c, sizeof (c), "%.8g", comp->chunks[i].kern_y[j]-((j>0)?comp->chunks[i].kern_y[j-1]:0));
						if ( s == NULL ) {
							s = g_strdup (c);
						}  else {
							s = g_strjoin (" ", s, c, NULL);
						}
					}
					sp_repr_set_attr (srepr, "dy", s);
					g_free(s);
				}
			}
			
			SPRepr* rstr = sp_xml_document_createTextNode (sp_repr_document (repr),comp->chunks[i].c_txt);
			sp_repr_append_child (srepr, rstr);
			sp_repr_unref (rstr);

			sp_repr_append_child (repr, srepr);
			sp_repr_unref (srepr);
			
		}
	}
	SPItem  *nitem = (SPItem *) SP_DT_DOCUMENT (desktop)->getObjectByRepr(repr);
	sp_item_write_transform(nitem, repr, item->transform);
	SP_OBJECT (nitem)->updateRepr();
		
	sp_repr_unref (repr);
	selection->setItem (nitem);
	object->deleteObject();

	sp_document_done (SP_DT_DOCUMENT (desktop));
}

