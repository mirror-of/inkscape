#define __sp_typeset_layout_C__

/*
 * layout routines for the typeset element
 *
 * public domain
 *
 */

#include <config.h>
#include <string.h>

#include "display/nr-arena-group.h"
#include "xml/repr-private.h"
#include "xml/repr.h"
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-point.h>
#include <libnr/nr-point-fns.h>
#include <libnr/nr-point-ops.h>
#include "sp-object-repr.h"
#include "svg/svg.h"
#include "inkscape.h"
#include "desktop-handles.h"
#include "document.h"
#include "style.h"
#include "attributes.h"

#include "sp-root.h"
#include "sp-use.h"
#include "sp-typeset.h"
#include "sp-typeset-utils.h"
#include "helper/sp-intl.h"

#include "sp-text.h"
#include "sp-shape.h"

#include "display/curve.h"
#include "livarot/Path.h"
#include "livarot/Ligne.h"
#include "livarot/Shape.h"
#include "livarot/LivarotDefs.h"

#include "libnrtype/TextWrapper.h"

#include <pango/pango.h>
//#include <pango/pangoxft.h>

void   sp_typeset_rekplayout(SPTypeset *typeset);


// breakpoints for the algo
typedef struct one_break {
  // this breakpoints comes after [start_ind .. end_ind] in the source text
  int          start_ind,end_ind;
  // the box this breakpoint ends
  box_solution pos;
  // whether the text in [start_ind .. end_ind] needs to be justified
  bool         no_justification;

  // previous breakpoint
  int          prev;
  // delta of score to the previous breakpoint
  double       score_to_prev;

  // next breakpoint for index end_ind (linked list)
  int          next_for_ind;
} one_break;


// class to hold the breakpoints during the algorithm
// breakpoints are not freed until the end of the algo
// thus the linked lists of breakpoints for a given index don't need maintainance
class break_holder {
public:
  // array of breakpoints
  int              nb_brk,max_brk;
  one_break*       brks;

  // pot_first[i] is the first breakpoint for index i
  int              nb_pot;
  int*             pot_first;

  // nb_potential is the max index (+1)
  break_holder(int nb_potential) {
    nb_brk=max_brk=0;
    brks=NULL;
    nb_pot=nb_potential;
    pot_first=(int*)malloc((nb_pot+1)*sizeof(int));
    for (int i=0;i<=nb_pot;i++) pot_first[i]=-1;
  };
  ~break_holder(void) {
    if ( brks ) free(brks);
    if ( pot_first ) free(pot_first);
  };

  // score of breakpoint of
  double           Score(int of);
  // brekpoint ending the line before of
  int              PrevLine(int of);
  // adds a breakpoint
  int              AddBrk(int st,int en,const box_solution &pos,int after,double delta,bool noJust);
};


double           break_holder::Score(int of)
{
  double res=0;
  for (int cur=of;cur > 0;cur=brks[cur].prev) res+=brks[cur].score_to_prev;
  return res;
}
int              break_holder::PrevLine(int of)
{
  if ( of < 0 ) return -1;
  double theY=brks[of].pos.y;
  for (int cur=of;cur >= 0;cur=brks[cur].prev) {
    if ( fabs(brks[cur].pos.y-theY) > 0.001 ) return cur;
  }
  return -1;
}
int              break_holder::AddBrk(int st,int en,const box_solution &pos,int after,double delta,bool noJust)
{
  double nScore=Score(after)+delta;
  if ( en < 0 ) {
    if ( nb_brk >= max_brk ) {
      max_brk=2*nb_brk+1;
      brks=(one_break*)realloc(brks,max_brk*sizeof(one_break));
    }
    int n=nb_brk;
    brks[n].start_ind=st;
    brks[n].end_ind=en;
    brks[n].pos=pos;
    brks[n].score_to_prev=delta;
    brks[n].prev=-1;
    brks[n].next_for_ind=-1;
    brks[n].no_justification=noJust;
    nb_brk++;
    return n;
  }
  for (int cur=pot_first[en];cur >= 0;cur=brks[cur].next_for_ind) {
    if ( brks[cur].end_ind == en && fabs(pos.y-brks[cur].pos.y) < 0.001 && fabs(pos.x_end-brks[cur].pos.x_end) < 0.001 ) {
      if ( nScore < Score(cur) ) {
        brks[cur].prev=after;
        brks[cur].pos=pos;
        brks[cur].start_ind=st;
        brks[cur].end_ind=en;
        brks[cur].score_to_prev=delta;
        brks[cur].no_justification=noJust;
 //       printf(" replaced %i\n",cur);
        return -1;
      } else {
//        printf("%i -> %i  dumped\n",st,en);
        return -1;
      }
    }
  }
  {
    if ( nb_brk >= max_brk ) {
      max_brk=2*nb_brk+1;
      brks=(one_break*)realloc(brks,max_brk*sizeof(one_break));
    }
    int n=nb_brk;
//    printf("add %i  s=%i e=%i\n",n,st,en);
    brks[n].start_ind=st;
    brks[n].end_ind=en;
    brks[n].pos=pos;
    brks[n].score_to_prev=delta;
    brks[n].prev=after;
    brks[n].next_for_ind=pot_first[en];
    brks[n].no_justification=noJust;
    pot_first[en]=n;
    nb_brk++;
    return n;
  }
}


// utility class to hold the breakpoints in need of consideration
class pending_holder {
public:
  typedef struct pooled {
    int          brk;
    double       nAsc,nDesc;
    bool         jump;
  } pooled;

  int            nb_pending,max_pending;
  pooled*        pending;

  pending_holder(void);
  ~pending_holder(void);

  void           AddPending(int after,double nA,double nD,bool jump);
  bool           Pop(int &after,double &nA,double &nD,bool &jump);
};
pending_holder::pending_holder(void)
{
  nb_pending=max_pending=0;
  pending=NULL;
}
pending_holder::~pending_holder(void)
{
  if ( pending ) free(pending);
  nb_pending=max_pending=0;
  pending=NULL;
}

void           pending_holder::AddPending(int after,double nA,double nD,bool jump)
{
  if ( after < 0 ) return;
  if ( nb_pending >= max_pending ) {
    max_pending=2*nb_pending+1;
    pending=(pooled*)realloc(pending,max_pending*sizeof(pooled));
  }
  pending[nb_pending].brk=after;
  pending[nb_pending].nAsc=nA;
  pending[nb_pending].nDesc=nD;
  pending[nb_pending].jump=jump;
  nb_pending++;
}
bool           pending_holder::Pop(int &after,double &nA,double &nD,bool &jump)
{
  if ( nb_pending <= 0 ) return false;
  // rand() is more portable than random()
  int i=rand()%nb_pending;
  if ( i < 0 ) i=-i;
  after=pending[i].brk;
  nA=pending[i].nAsc;
  nD=pending[i].nDesc;
  jump=pending[i].jump;
  pending[i]=pending[--nb_pending];
  return true;
}


typedef struct typeset_step {
  box_solution      box;
  int               start_ind,end_ind;
  bool              no_justification;
} typeset_step;

void   sp_typeset_rekplayout(SPTypeset *typeset)
{
  if ( typeset == NULL || typeset->layoutDirty == false ) return;
  typeset->layoutDirty=false;

  if ( typeset->theDst ) delete typeset->theDst;
  typeset->theDst=NULL;
  if ( typeset->dstType == has_shape_dest ) {
    GSList* l=typeset->dstElems;
    dest_shape_chunker* nd=new dest_shape_chunker();
    typeset->theDst=nd;
    while ( l && l->data ) {
      shape_dest* theData=(shape_dest*)l->data;
      if ( theData->theShape ) nd->AppendShape(theData->theShape,typeset->excluded);
      l=l->next;
    }
  } else if ( typeset->dstType == has_path_dest ) {
    GSList* l=typeset->dstElems;
    dest_path_chunker* nd=new dest_path_chunker();
    typeset->theDst=nd;
    while ( l && l->data ) {
      path_dest* theData=(path_dest*)l->data;
      if ( theData->thePath ) nd->AppendPath(theData->thePath);
      l=l->next;
    }
  } else if ( typeset->dstType == has_box_dest ) {
    GSList* l=typeset->dstElems;
    dest_box_chunker* nd=new dest_box_chunker();
    typeset->theDst=nd;
    while ( l && l->data ) {
      box_dest* theData=(box_dest*)l->data;
      nd->AppendBox(theData->box);
      l=l->next;
    }
  } else if ( typeset->dstType == has_col_dest ) {
    if ( typeset->dstElems && typeset->dstElems->data ) {
      column_dest* theData=(column_dest*)typeset->dstElems->data;
      typeset->theDst=new dest_col_chunker(theData->width);
    }
  }
  text_wrapper*   combined_src=new text_wrapper();
  {
    // gather source text
    if ( typeset->srcText ) {
      if ( typeset->srcType == has_std_txt ) {
        combined_src->AppendUTF8(typeset->srcText,-1);
      } else if ( typeset->srcType == has_pango_txt ) {
        combined_src->AppendUTF8(typeset->srcText,-1);
      }
      const char* val_delta_x=sp_repr_attr((SP_OBJECT(typeset))->repr,"dx");
      if ( val_delta_x ) {
        GList *  the_delta_x=sp_svg_length_list_read (val_delta_x);
        combined_src->KernXForLastAddition(the_delta_x,1.0);
        g_list_free(the_delta_x);
      }
      const char* val_delta_y=sp_repr_attr((SP_OBJECT(typeset))->repr,"dy");
      if ( val_delta_y ) {
        GList *  the_delta_y=sp_svg_length_list_read (val_delta_y);
        combined_src->KernYForLastAddition(the_delta_y,1.0);
        g_list_free(the_delta_y);
      }
    }
    for (	SPObject * child = sp_object_first_child(SP_OBJECT(typeset)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
      if ( SP_IS_TYPESET(child) ) {
        SPTypeset*  child_t=SP_TYPESET(child);
        if ( child_t->srcText ) {
          if ( child_t->srcType == has_std_txt ) {
            if ( combined_src->utf8_length > 0 ) combined_src->AppendUTF8("\n",-1);
            combined_src->AppendUTF8(child_t->srcText,-1);
          } else if ( child_t->srcType == has_pango_txt ) {
            if ( combined_src->utf8_length > 0 ) combined_src->AppendUTF8("\n",-1);
            combined_src->AppendUTF8(child_t->srcText,-1);
          }
          const char* val_delta_x=sp_repr_attr((SP_OBJECT(child_t))->repr,"dx");
          if ( val_delta_x ) {
						GList *  the_delta_x=sp_svg_length_list_read (val_delta_x);
						combined_src->KernXForLastAddition(the_delta_x,1.0);
						g_list_free(the_delta_x);
          }
          const char* val_delta_y=sp_repr_attr((SP_OBJECT(child_t))->repr,"dy");
          if ( val_delta_y ) {
						GList *  the_delta_y=sp_svg_length_list_read (val_delta_y);
						combined_src->KernYForLastAddition(the_delta_y,1.0);
						g_list_free(the_delta_y);
          }
        }
      }
    }
  }
  
  if ( typeset->theSrc ) delete typeset->theSrc;
  typeset->theSrc=NULL;
  {
    SPCSSAttr *css;
    css = sp_repr_css_attr ((SP_OBJECT(typeset))->repr, "inkscape:layoutOptions");
    const gchar *val_size = sp_repr_css_property (css, "font-size", NULL);
    double  fsize=12.0;
    if ( val_size ) fsize = sp_repr_css_double_property (css, "font-size", 12.0);
    const gchar *val_family = sp_repr_css_property (css, "font-family", NULL);
    if ( val_family ) {
      typeset->theSrc = new pango_text_chunker(combined_src, (gchar *) val_family, fsize, p_t_c_none);
    } else {
      typeset->theSrc = new pango_text_chunker(combined_src, "Luxi Sans", fsize, p_t_c_none);
    }
    if ( css ) sp_repr_css_attr_unref(css);
  }
  
  
  // kill children
  {
    GSList *l=NULL;
    for (	SPObject * child = sp_object_first_child(SP_OBJECT(typeset)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
      if ( SP_IS_TYPESET(child) ) {
      } else {
        l=g_slist_prepend(l,child);
      }
    }
    while ( l ) {
      SPObject *child=(SPObject*)l->data;
      //      sp_object_unref(child, SP_OBJECT(typeset));
      child->deleteObject();
      l=g_slist_remove(l,child);
    }
  }
  // do layout
  typeset_step  *steps=NULL;
  int           nb_step=0;

  if ( typeset->theSrc && typeset->theDst ) {
    // dumb layout: stuff 'til it's too big
    double nAscent=0.0,nDescent=0.0;
    typeset->theSrc->InitialMetricsAt(0,nAscent,nDescent);
    int           maxIndex=typeset->theSrc->MaxIndex();
    if ( nAscent < 0.0001 && nDescent < 0.0001 ) {
      // nothing to stuff?
    } else {
      box_solution    cur_box;
      break_holder*   brk_list=new break_holder(maxIndex);
      pending_holder* pen_list=new pending_holder();

      cur_box=typeset->theDst->VeryFirst();
      int           cur_brk=brk_list->AddBrk(-1,-1,cur_box,-1,0,true);
      pen_list->AddPending(cur_brk,nAscent,nDescent,false);

      int      best_brk=-1;
      double   best_score=0;

      do {
        bool          sameLine=false;
        bool          jump_to_next_line=false;
        if ( pen_list->Pop(cur_brk,nAscent,nDescent,jump_to_next_line) == false ) {
          break;
        }

        int    cur_pos=brk_list->brks[cur_brk].end_ind+1;
//        printf("traite: %i: %i %f %f %i: s=%f\n",cur_brk,cur_pos,nAscent,nDescent,(jump_to_next_line)?1:0,brk_list->Score(cur_brk));

        if ( cur_pos >= maxIndex ) {
          if ( brk_list->brks[cur_brk].end_ind >= 0 ) {
            double  ns=brk_list->Score(cur_brk)/(1+brk_list->brks[cur_brk].end_ind);
            if ( best_brk < 0 || ns < best_score ) {
              best_brk=cur_brk;
              best_score=ns;
            }
          }
          continue;
        }

        if ( jump_to_next_line ) {
//          printf("it's just a jump to the left\n");
          cur_box=typeset->theDst->NextLine(brk_list->brks[cur_brk].pos,nAscent,nDescent,0.0);
        } else {
          cur_box=typeset->theDst->NextBox(brk_list->brks[cur_brk].pos,nAscent,nDescent,0.0,sameLine);
        }
        if ( cur_box.finished ) {
          if ( brk_list->brks[cur_brk].end_ind >= 0 ) {
            double  ns=brk_list->Score(cur_brk)/(1+brk_list->brks[cur_brk].end_ind);
            if ( best_brk < 0 || ns < best_score ) {
              best_brk=cur_brk;
              best_score=ns;
            }
          }
          continue;
        }

        double nLen=cur_box.x_end-cur_box.x_start;
        text_chunk_solution* sol=typeset->theSrc->StuffThatBox(cur_pos,0.8*nLen,nLen,1.2*nLen,false);

        if ( sol == NULL ) {
          if ( brk_list->brks[cur_brk].end_ind >= 0 ) {
            double  ns=brk_list->Score(cur_brk)/(1+brk_list->brks[cur_brk].end_ind);
            if ( best_brk < 0 || ns < best_score ) {
              best_brk=cur_brk;
              best_score=ns;
            }
          }
          continue;
        }
        if ( sol[0].end_of_array ) {
          free(sol);
          if ( brk_list->brks[cur_brk].end_ind >= 0 ) {
            double  ns=brk_list->Score(cur_brk)/(1+brk_list->brks[cur_brk].end_ind);
            if ( best_brk < 0 || ns < best_score ) {
              best_brk=cur_brk;
              best_score=ns;
            }
          }
          continue;
        }

        for (int i=0;sol[i].end_of_array==false;i++) {
          if ( sol[i].end_ind >= sol[i].start_ind ) {
//            printf("void %i %i\n",sol[i].start_ind,sol[i].end_ind);
            if ( sol[i].ascent > nAscent || sol[i].descent > nDescent ) {
              int p_line=(sameLine)?brk_list->PrevLine(cur_brk):cur_brk;
              if ( p_line >= 0 ) {
                pen_list->AddPending(p_line,sol[i].ascent,sol[i].descent,true);
              }
            } else {
              if ( sol[i].length < 0.001 ) {
//                printf("oversmall %i %i\n",sol[i].start_ind,sol[i].end_ind);
               if ( sol[i].endOfParagraph ) {
                  int n_brk=brk_list->AddBrk(sol[i].start_ind,sol[i].end_ind,cur_box,cur_brk,0,true);
                  if ( n_brk >= 0 ) {
                    double    a,d;
                    typeset->theSrc->InitialMetricsAt(sol[i].end_ind+1,a,d);
                    pen_list->AddPending(n_brk,a,d,true);
                  }
                } else {
//                  printf("qu'est ce que c'est que cette longueur nulle: %i %i \n",sol[i].start_ind,sol[i].end_ind);
                }
              } else if ( sol[i].length > 1.5*1.2*nLen ) {
//                printf("overlarge %i %i\n",sol[i].start_ind,sol[i].end_ind);
                int n_brk=brk_list->AddBrk(brk_list->brks[cur_brk].end_ind+1,brk_list->brks[cur_brk].end_ind,cur_box,cur_brk,0,true);
                if ( n_brk >= 0 ) {
                  double    a,d;
                  typeset->theSrc->InitialMetricsAt(brk_list->brks[cur_brk].end_ind+1,a,d);
                  pen_list->AddPending(n_brk,a,d,sol[i].endOfParagraph);
                }
              } else {
                double   delta=0;
                if ( sol[i].endOfParagraph == false || sol[i].length > nLen) {
                  if ( sol[i].length > nLen ) {
                    delta=(sol[i].length/nLen)-1;
                  } else {
                    delta=(nLen/sol[i].length)-1;
                  }
//                  printf("%i %i -> delta=%f\n",sol[i].start_ind,sol[i].end_ind,delta);
                } else {
//                  printf("%i %i  eo\n",sol[i].start_ind,sol[i].end_ind);
                }
                int n_brk=brk_list->AddBrk(sol[i].start_ind,sol[i].end_ind,cur_box,cur_brk,delta,sol[i].endOfParagraph);
                if ( n_brk >= 0 ) {
                  if ( sol[i].end_ind >= maxIndex-1 ) {
                    double  ns=brk_list->Score(n_brk)/(1+brk_list->brks[n_brk].end_ind);
                    if ( best_brk < 0 || ns < best_score ) {
                      best_brk=n_brk;
                      best_score=ns;
                    }
                  } else {
                    double    a,d;
                    typeset->theSrc->InitialMetricsAt(sol[i].end_ind+1,a,d);
                    pen_list->AddPending(n_brk,a,d,sol[i].endOfParagraph);
                  }
                }
              }
            }
          }
        }

        free(sol);
      } while ( 1 );

      if ( best_brk >= 0 ) {
        for (cur_brk=best_brk;cur_brk>=0;cur_brk=brk_list->brks[cur_brk].prev) {
          if ( brk_list->brks[cur_brk].start_ind <= brk_list->brks[cur_brk].end_ind ) {
//            printf("bk=%i (->p=%i): s=%i e=%i l=%f\n",cur_brk,brk_list->brks[cur_brk].prev,brk_list->brks[cur_brk].start_ind,brk_list->brks[cur_brk].end_ind
//                   ,brk_list->brks[cur_brk].pos.x_end-brk_list->brks[cur_brk].pos.x_start);
            steps=(typeset_step*)realloc(steps,(nb_step+1)*sizeof(typeset_step));
            if ( nb_step > 0 ) memmove(steps+1,steps,nb_step*sizeof(typeset_step));
            steps[0].box=brk_list->brks[cur_brk].pos;
            steps[0].start_ind=brk_list->brks[cur_brk].start_ind;
            steps[0].end_ind=brk_list->brks[cur_brk].end_ind;
            steps[0].no_justification=brk_list->brks[cur_brk].no_justification;
            if ( steps[0].end_ind >= maxIndex-1 ) steps[0].no_justification=true;
            nb_step++;
          }
        }
      }

      delete brk_list;
      delete pen_list;
    }
  }
  // create offspring
  {
    int           maxIndex=(typeset->theSrc)?typeset->theSrc->MaxIndex():0;

    SPRepr *parent = SP_OBJECT_REPR(SP_OBJECT(typeset));
    
    SPRepr* text_repr = sp_repr_new ("text");
    {
      SPCSSAttr *css;
      SPCSSAttr *style_repr=sp_repr_css_attr_new();
      css = sp_repr_css_attr ((SP_OBJECT(typeset))->repr, "inkscape:layoutOptions");
      const gchar *val_size = sp_repr_css_property (css, "font-size", NULL);
      if ( val_size ) {
        sp_repr_set_attr ((SPRepr*)style_repr,"font-size", val_size);
      } else {
        sp_repr_set_attr ((SPRepr*)style_repr,"font-size", "12");
      }
      const gchar *val_family = sp_repr_css_property (css, "font-family", NULL);
      if ( val_family ) {
        sp_repr_set_attr ((SPRepr*)style_repr,"font-family", val_family);
      } else {
        sp_repr_set_attr ((SPRepr*)style_repr,"font-family", "Sans");
      }
      if ( css ) sp_repr_css_attr_unref(css);
      
      if ( style_repr ) {
        sp_repr_css_set (text_repr,style_repr, "style");
        sp_repr_css_attr_unref(style_repr);
      }
    }
    sp_repr_append_child (parent, text_repr);
    sp_repr_unref (text_repr);

    for (int i=0;i<nb_step;i++) {
      if ( steps[i].start_ind >= 0 && steps[i].end_ind >= steps[i].start_ind ) {
        double spacing=steps[i].box.x_end-steps[i].box.x_start;
        double used=0;
        int    nbSrcChar=0;
        typeset->theSrc->GlyphsInfo(steps[i].start_ind,steps[i].end_ind,nbSrcChar,used);
        spacing-=used;
        if ( nbSrcChar > 1 ) {
          if ( ( steps[i].end_ind < maxIndex-1 && steps[i].no_justification == false ) || spacing < 0 ) {
            spacing/=(nbSrcChar-1);
          } else {
            spacing=0;
          }
        } else {
          spacing=0;
        }
        
        if ( typeset->justify == false ) spacing=0;
        double   delta_start=0;
        if ( steps[i].no_justification || typeset->justify == false ) {
          if ( typeset->centering == 1 ) {
            delta_start=0.5*(steps[i].box.x_end-steps[i].box.x_start-used);
          } else if ( typeset->centering == 2 ) {
            delta_start=steps[i].box.x_end-steps[i].box.x_start-used;
          }
        }
        
        if ( typeset->dstType == has_path_dest ) {
          dest_path_chunker* dpc=(dest_path_chunker*)typeset->theDst;
          if ( steps[i].box.frame_no >= 0 && steps[i].box.frame_no < dpc->nbPath ) {
            path_to_SVG_context*  nCtx=new path_to_SVG_context(text_repr,dpc->paths[steps[i].box.frame_no].theP,dpc->paths[steps[i].box.frame_no].length,delta_start);
            nCtx->SetLetterSpacing(spacing);
            typeset->theSrc->GlyphsAndPositions(steps[i].start_ind,steps[i].end_ind,nCtx);
            delete nCtx;
          }
        } else {
          box_to_SVG_context*  nCtx=new box_to_SVG_context(text_repr,steps[i].box.y,steps[i].box.x_start+delta_start,steps[i].box.x_end-delta_start);
          nCtx->SetLetterSpacing(spacing);
          typeset->theSrc->GlyphsAndPositions(steps[i].start_ind,steps[i].end_ind,nCtx);
          delete nCtx;
        }
      }
    }
    free(steps);

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    sp_document_done (SP_DT_DOCUMENT (desktop));
  }
  if ( typeset->theSrc ) delete typeset->theSrc;
  typeset->theSrc=NULL;
  if ( typeset->theDst ) delete typeset->theDst;
  typeset->theDst=NULL;
  if ( combined_src ) delete combined_src;
}
