#define __sp_typeset_layout_C__

/*
 * layout routines for the typeset element
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
      if ( theData->theShape ) nd->AppendShape(theData->theShape);
      l=l->next;
    }
  } else if ( typeset->dstType == has_path_dest ) {
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
  if ( typeset->theSrc ) delete typeset->theSrc;
  typeset->theSrc=NULL;
  if ( typeset->srcType == has_std_txt ) {
    SPCSSAttr *css;
    css = sp_repr_css_attr (SP_OBJECT_REPR (SP_OBJECT(typeset)), "style");
    const gchar *val_size = sp_repr_css_property (css, "font-size", NULL);
    double  fsize=12.0;
    if ( val_size ) fsize = sp_repr_css_double_property (css, "font-size", 12.0);
    const gchar *val_family = sp_repr_css_property (css, "font-family", NULL);
    if ( val_family ) {
      typeset->theSrc = new pango_text_chunker(typeset->srcText, (gchar *) val_family, fsize, p_t_c_none,false);
    } else {
      typeset->theSrc = new pango_text_chunker(typeset->srcText, "Luxi Sans", fsize, p_t_c_none,false);
    }
  } else if ( typeset->srcType == has_pango_txt ) {
    SPCSSAttr *css;
    css = sp_repr_css_attr (SP_OBJECT_REPR (SP_OBJECT(typeset)), "style");
    const gchar *val_size = sp_repr_css_property (css, "font-size", NULL);
    double  fsize=12.0;
    if ( val_size ) fsize = sp_repr_css_double_property (css, "font-size", 12.0);
    const gchar *val_family = sp_repr_css_property (css, "font-family", NULL);
    if ( val_family ) {
      typeset->theSrc = new pango_text_chunker(typeset->srcText, (gchar *) val_family, fsize, p_t_c_none,true);
    } else {
      typeset->theSrc = new pango_text_chunker(typeset->srcText, "Luxi Sans", fsize, p_t_c_none,true);
    }
  }
  
  
  // kill children
  {
    GSList *l=NULL;
    for (	SPObject * child = sp_object_first_child(SP_OBJECT(typeset)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
      l=g_slist_prepend(l,child);
    }
    while ( l ) {
      SPObject *child=(SPObject*)l->data;
      child->deleteObject();
//      sp_object_unref(child, SP_OBJECT(typeset));
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

    for (int i=1;i<nb_step;i++) {
      if ( steps[i].end_ind >= steps[i].start_ind ) {
        int              nbS=0;
        glyphs_for_span  *span_info=NULL;
        typeset->theSrc->GlyphsAndPositions(steps[i].start_ind,steps[i].end_ind,nbS,span_info);
        double spacing=steps[i].box.x_end-steps[i].box.x_start;
        int    nbSrcChar=0;
        for (int k=0;k<nbS;k++) {
          if ( span_info[k].nbG > 0 ) {
            spacing-=span_info[k].g_pos[span_info[k].nbG][0]-span_info[k].g_pos[0][0];
            for (int j=0;j<span_info[k].nbG;j++) nbSrcChar+=span_info[k].g_end[j]-span_info[k].g_start[j]+1;
          }
        }
        if ( nbSrcChar > 1 ) {
          if ( ( steps[i].end_ind < maxIndex-1 && steps[i].no_justification == false ) || spacing < 0 ) {
            spacing/=(nbSrcChar-1);
          } else {
            spacing=0;
          }
        } else {
          spacing=0;
        }
        
        for (int k=0;k<nbS;k++) {
          SPRepr* span_repr = sp_repr_new ("tspan");
          
          if ( span_info[k].style[0] != 0 ) {
            sp_repr_set_attr (span_repr, "style", span_info[k].style);
          }
          
          if ( span_info[k].g_pos && span_info[k].g_start && span_info[k].g_end && span_info[k].nbG > 0 ) {
            NR::Point   textPos(steps[i].box.x_start,steps[i].box.y);
            textPos+=span_info[k].g_pos[0];
            int nbPrevChar=span_info[k].g_start[0]-span_info[0].g_start[0];
            sp_repr_set_double (span_repr, "x", textPos[0]+spacing*((double)(nbPrevChar)));
            sp_repr_set_double (span_repr, "y", textPos[1]);
            
            {
              SPCSSAttr *ocss;
              ocss = sp_repr_css_attr (span_repr, "style");              
              sp_repr_set_double ((SPRepr*)ocss, "letter-spacing", spacing);
              sp_repr_css_change (span_repr, ocss, "style");
              sp_repr_css_attr_unref (ocss);
            }
            bool  do_dy=false;
            for (int j = 0; j < span_info[k].nbG ; j ++) {
              if ( fabs(span_info[k].g_pos[j][1]) > 0.001 ) {
                do_dy=true;
                break;
              }
            }
            if ( do_dy ) {
              gchar c[32];
              gchar *s = NULL;
              double    lastY=span_info[k].g_pos[0][1];
              for (int j = 0; j < span_info[k].nbG ; j ++) {
                int     t_st=span_info[k].g_start[j];
                int     t_en=span_info[k].g_end[j];
                for (int g=t_st;g<=t_en;g++) {
                  g_ascii_formatd (c, sizeof (c), "%.8g", span_info[k].g_pos[j][1]-lastY);
                  lastY=span_info[k].g_pos[j][1];
                  if (s == NULL) {
                    s = g_strdup (c);
                  }  else {
                    s = g_strjoin (" ", s, c, NULL);
                  }
                }
              }
              sp_repr_set_attr (span_repr, "dy", s);
              g_free(s);
            }
          } else {
            sp_repr_set_double (span_repr, "x", steps[i].box.x_start);
            sp_repr_set_double (span_repr, "y", steps[i].box.y);
          }
          
          int   content_length=0;
          char* temp_content=NULL;
          for (int j = 0; j < span_info[k].nbG ; j ++) {
            int     t_st=span_info[k].g_start[j];
            int     t_en=span_info[k].g_end[j];
            if ( t_en >= t_st ) {
              temp_content=(char*)realloc(temp_content,(content_length+t_en-t_st+1)*sizeof(char));
              memcpy(temp_content+content_length,span_info[k].g_text+t_st,(t_en-t_st+1)*sizeof(char));
              content_length+=t_en-t_st+1;
            }
          }
          temp_content=(char*)realloc(temp_content,(content_length+1)*sizeof(char));
          temp_content[content_length]=0;
          SPRepr* rstr = sp_xml_document_createTextNode (sp_repr_document (parent), temp_content);
          sp_repr_append_child (span_repr, rstr);
          sp_repr_unref (rstr);
          free(temp_content);
          
          sp_repr_append_child (text_repr, span_repr);
          sp_repr_unref (span_repr);
        }
        for (int k=0;k<nbS;k++) {
          if ( span_info[k].g_start ) free(span_info[k].g_start);
          if ( span_info[k].g_end ) free(span_info[k].g_end);
          if ( span_info[k].g_pos ) free(span_info[k].g_pos);
        }
        if ( span_info ) free(span_info);
      }
    }
    free(steps);
    
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    sp_document_done (SP_DT_DOCUMENT (desktop));
  }
}
