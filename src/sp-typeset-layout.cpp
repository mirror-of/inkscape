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

#include <pango/pango.h>
//#include <pango/pangoxft.h>
#include <gdk/gdk.h>

void   sp_typeset_relayout(SPTypeset *typeset);



/*
 *
 *
 *
 */

typedef struct typeset_step {
  box_solution      box;
  int               start_ind,end_ind;
  int               nbGlyph;
  bool              no_justification;
} typeset_step;

void   sp_typeset_relayout(SPTypeset *typeset)
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
      if ( theData->theShape ) {
        void   exclude_shape_from_dest(SPTypeset *typeset,Shape *canditate);
        nd->AppendShape(theData->theShape,typeset->excluded);
      }
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
  
  char*   combined_src=NULL;
  int     combined_type=has_no_src;
  {
    // gather source text
    if ( typeset->srcText ) {
      if ( typeset->srcType == has_std_txt ) {
        combined_src=strdup(typeset->srcText);
        combined_type=has_std_txt;
      } else if ( typeset->srcType == has_pango_txt ) {
        combined_src=strdup(typeset->srcText);
        combined_type=has_pango_txt;
      }
    }
    // kill children
    {
      for (	SPObject * child = sp_object_first_child(SP_OBJECT(typeset)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if ( SP_IS_TYPESET(child) ) {
          SPTypeset*  child_t=SP_TYPESET(child);
          if ( child_t->srcText ) {
            if ( child_t->srcType == has_std_txt ) {
              if ( combined_src ) {
                int old_len=strlen(combined_src);
                combined_src=(char*)realloc(combined_src,(strlen(combined_src)+strlen(child_t->srcText)+2)*sizeof(char));
                combined_src[old_len]='\n';
                memcpy(combined_src+(old_len+1),child_t->srcText,(strlen(child_t->srcText)+1)*sizeof(char));
              } else {
                combined_src=strdup(child_t->srcText);
              }
              if ( combined_type == has_no_src ) combined_type=has_std_txt;
            } else if ( child_t->srcType == has_pango_txt ) {
              if ( combined_src ) {
                int old_len=strlen(combined_src);
                combined_src=(char*)realloc(combined_src,(old_len+strlen(child_t->srcText)+2)*sizeof(char));
                combined_src[old_len]='\n';
                memcpy(combined_src+(old_len+1),child_t->srcText,(strlen(child_t->srcText)+1)*sizeof(char));
              } else {
                combined_src=strdup(child_t->srcText);
              }
              combined_type=has_pango_txt;
            }
          }
        }
      }
    }
    
  }
  
  if ( typeset->theSrc ) delete typeset->theSrc;
  typeset->theSrc=NULL;
  if ( combined_type == has_std_txt ) {
    SPCSSAttr *css;
    css = sp_repr_css_attr (SP_OBJECT_REPR (SP_OBJECT(typeset)), "inkscape:layoutOptions");
    const gchar *val_size = sp_repr_css_property (css, "font-size", NULL);
    double  fsize=12.0;
    if ( val_size ) fsize = sp_repr_css_double_property (css, "font-size", 12.0);
    const gchar *val_family = sp_repr_css_property (css, "font-family", NULL);
    if ( val_family ) {
      typeset->theSrc = new pango_text_chunker(combined_src, (gchar *) val_family, fsize, p_t_c_none,false);
    } else {
      typeset->theSrc = new pango_text_chunker(combined_src, "Luxi Sans", fsize, p_t_c_none,false);
    }
    if ( css ) sp_repr_css_attr_unref(css);
  } else if ( combined_type == has_pango_txt ) {
    SPCSSAttr *css;
    css = sp_repr_css_attr (SP_OBJECT_REPR (SP_OBJECT(typeset)), "inkscape:layoutOptions");
    const gchar *val_size = sp_repr_css_property (css, "font-size", NULL);
    double  fsize=12.0;
    if ( val_size ) fsize = sp_repr_css_double_property (css, "font-size", 12.0);
    const gchar *val_family = sp_repr_css_property (css, "font-family", NULL);
    if ( val_family ) {
      typeset->theSrc = new pango_text_chunker(combined_src, (gchar *) val_family, fsize, p_t_c_none,true);
    } else {
      typeset->theSrc = new pango_text_chunker(combined_src, "Luxi Sans", fsize, p_t_c_none,true);
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
  typeset_step  *steps=(typeset_step*)malloc(sizeof(typeset_step));
  int           nb_step=1;
  steps[0].box.x_start=steps[0].box.x_end=0;
  steps[0].box.y=steps[0].box.ascent=steps[0].box.descent=0;
  steps[0].start_ind=-1;
  steps[0].end_ind=-1;
  steps[0].nbGlyph=0;

  if ( typeset->theSrc && typeset->theDst ) {
    // dumb layout: stuff 'til it's too big    
    double nAscent=0.0,nDescent=0.0;
    typeset->theSrc->InitialMetricsAt(0,nAscent,nDescent);
    int           maxIndex=typeset->theSrc->MaxIndex();
    if ( nAscent < 0.0001 && nDescent < 0.0001 ) {
      // nothing to stuff?
    } else {
      int           last_step=0;
      int           prev_line_step=0;
      bool          jump_to_next_line=false;
      bool          start_need_reach=false;
      int           need_to_reach_ind=-1;
      typeset_step* sav_steps=NULL;
      int           nb_sav_step=0;
      
      steps[prev_line_step].box=typeset->theDst->VeryFirst();
      last_step=prev_line_step;
            
      do {
        int           cur_pos=steps[last_step].end_ind+1;
        bool          sameLine=false;
        box_solution  cur_box;
        if ( jump_to_next_line ) {
//          printf("it's just a jump to the left\n");
          cur_box=typeset->theDst->NextLine(steps[last_step].box,nAscent,nDescent,0.0);
          jump_to_next_line=false;
        } else {
          cur_box=typeset->theDst->NextBox(steps[last_step].box,nAscent,nDescent,0.0,sameLine);
        }
        if ( cur_box.finished ) break;
        double nLen=cur_box.x_end-cur_box.x_start;
        text_chunk_solution* sol=typeset->theSrc->StuffThatBox(cur_pos,0.8*nLen,nLen,1.2*nLen,false);
        if ( sol == NULL ) {
          break;
        }
        if ( sol[0].end_of_array ) {
          free(sol);
          break;
        }
//        for (int i=0;sol[i].end_of_array==false;i++) {
//          printf("sol %i : s=%i e=%i l=%f ep=%i\n",i,sol[i].start_ind,sol[i].end_ind,sol[i].length,(sol[i].endOfParagraph)?1:0);
//        }
        
        int     best=0;
        if ( sol[best].endOfParagraph ) {
        } else {
          for (int i=1;sol[i].end_of_array==false;i++) {
            if ( sol[i].length < nLen ) {
              best=i;
              if ( sol[best].endOfParagraph ) {
                break;
              }
            }
          }
        }
        if ( sameLine == false ) {
          if ( start_need_reach ) {
            start_need_reach=false;
          } else {
            if ( need_to_reach_ind >= 0 ) {
              if ( sol[best].end_ind <= need_to_reach_ind ) {
                // didn't improve, revert to saved steps
                last_step=prev_line_step+nb_sav_step;
                memcpy(steps+(prev_line_step+1),sav_steps,nb_sav_step*sizeof(typeset_step));
                need_to_reach_ind=-1;
                if ( sav_steps ) free(sav_steps);
                sav_steps=NULL;
                nb_sav_step=0;
                prev_line_step=last_step;
                jump_to_next_line=true;
                continue;
              } else {
                need_to_reach_ind=-1;
                if ( sav_steps ) free(sav_steps);
                sav_steps=NULL;
                nb_sav_step=0;
              }
            }
          }
        }

        if ( sol[best].length > nLen ) {
          // ouchie: doesn't fit in line
          steps=(typeset_step*)realloc(steps,(nb_step+1)*sizeof(typeset_step));
          steps[nb_step].box=cur_box;
          steps[nb_step].start_ind=sol[best].start_ind;
          steps[nb_step].end_ind=sol[best].start_ind-1;
          steps[nb_step].nbGlyph=0;
          nb_step++;
          
          last_step=nb_step-1;
          if ( sameLine ) {
          } else {
            prev_line_step=last_step;
          }
          typeset->theSrc->InitialMetricsAt(steps[last_step].end_ind+1,nAscent,nDescent);
          
          free(sol);
          if ( steps[last_step].end_ind+1 >= maxIndex ) break;
          continue;
        }
        if ( sol[best].ascent > nAscent || sol[best].descent > nDescent ) {
          nAscent=sol[best].ascent;
          nDescent=sol[best].descent;
          free(sol);
          start_need_reach=true;
          need_to_reach_ind=steps[last_step].end_ind;
          nb_sav_step=last_step-prev_line_step;
          sav_steps=(typeset_step*)malloc(nb_sav_step*sizeof(typeset_step));
          memcpy(sav_steps,steps+(prev_line_step+1),nb_sav_step*sizeof(typeset_step));
          if ( last_step > prev_line_step ) {
            nb_step=prev_line_step+1;
            last_step=prev_line_step;
          }
          if ( steps[last_step].end_ind+1 >= maxIndex ) {
            free(sav_steps);
            sav_steps=NULL;
            nb_sav_step=0;
            break;
          }
          continue;
        }
        
//        printf("best=%i\n",best);
        steps=(typeset_step*)realloc(steps,(nb_step+1)*sizeof(typeset_step));
        steps[nb_step].box=cur_box;
        steps[nb_step].start_ind=sol[best].start_ind;
        steps[nb_step].end_ind=sol[best].end_ind;
        steps[nb_step].nbGlyph=0;
        steps[nb_step].no_justification=false;
        jump_to_next_line=false;
        if ( sol[best].endOfParagraph ) {
          steps[nb_step].no_justification=true;
          jump_to_next_line=true;
        }
        nb_step++;
        
        last_step=nb_step-1;
        if ( sameLine ) {
        } else {
          prev_line_step=last_step;
        }
        typeset->theSrc->InitialMetricsAt(steps[last_step].end_ind+1,nAscent,nDescent);
        
        free(sol);
      } while ( steps[last_step].end_ind+1 < maxIndex );
      if ( sav_steps ) free(sav_steps);
    }
  }
  // create offspring
  {
    int           maxIndex=(typeset->theSrc)?typeset->theSrc->MaxIndex():0;
    
    SPRepr *parent = SP_OBJECT_REPR(SP_OBJECT(typeset));
    char const *style = sp_repr_attr (parent, "style");
    if (!style) {
	    style = "font-family:Sans;font-size:12;";
    }
    
    SPRepr* text_repr = sp_repr_new ("text");
    sp_repr_set_attr (text_repr, "style", style);
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
  if ( combined_src ) free(combined_src);
}
