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
#include "style.h"
#include "version.h"
#include "inkscape.h"
#include "view.h"
#include "print.h"

#include "xml/repr.h"

#include "sp-shape.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "sp-tspan.h"
#include "sp-string.h"

#include "text-editing.h"

div_flow_src *
te_get_contents (SPItem *item)
{
    if (SP_IS_TEXT(item)) {
        return &(SP_TEXT(item)->contents);
    } else if (SP_IS_FLOWTEXT (item)) {
        return &(SP_FLOWTEXT(item)->contents);
    }
    return NULL;
}

flow_res*
te_get_f_res (SPItem *item)
{
    if (SP_IS_TEXT(item)) {
        return SP_TEXT(item)->f_res;
    } else if (SP_IS_FLOWTEXT (item)) {
        return SP_FLOWTEXT(item)->f_res;
    }
    return NULL;
}

flow_src*
te_get_f_src (SPItem *item)
{
    if (SP_IS_TEXT(item)) {
        return SP_TEXT(item)->f_src;
    } else if (SP_IS_FLOWTEXT (item)) {
        return SP_FLOWTEXT(item)->f_src;
    }
    return NULL;
}

NR::Point
te_getOrigin (SPItem *item)
{
    if (SP_IS_TEXT(item)) {
        SPText *text = SP_TEXT(item);
        return  NR::Point(text->x.computed, text->y.computed);
    } else if (SP_IS_FLOWTEXT (item)) {
       //FIXME!!!!!!!!
        return NR::Point(0,0);
    }
    return NR::Point(0,0);
}

void
te_UpdateFlowSource (SPItem *item)
{
    if (SP_IS_TEXT(item)) {
        SPText *text = SP_TEXT(item);
        if ( text->f_src == NULL ) text->UpdateFlowSource();
    } else if (SP_IS_FLOWTEXT (item)) {
        SPFlowtext *text = SP_FLOWTEXT(item);
        if ( text->f_src == NULL ) text->UpdateFlowSource();
    }
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
{
    te_UpdateFlowSource (item);
    one_flow_src *cur = te_get_contents (item);

    gint length = 0;
    while (cur) {
			length += cur->ucs4_en-cur->ucs4_st;
			cur = cur->next;
    }
    return length;
}

gint
sp_te_up (SPItem *item, gint i_position)
{
    flow_res* f_res = te_get_f_res (item);
    div_flow_src *contents = te_get_contents (item);

    int position=contents->Do_UCS4_2_UTF8(i_position);

   if ( f_res == NULL ) return i_position;
    int    c_p=-1,s_p=-1,l_p=-1;
    bool   l_start=false,l_end=false;
    f_res->OffsetToLetter(position,c_p,s_p,l_p,l_start,l_end);

    if ( c_p >= 0 ) {
        int l_o = l_p - f_res->chunks[c_p].l_st;
        if ( l_end ) {
            l_end=false;
            l_o++;
        }
        c_p--;
        if ( c_p < 0 ) {
            c_p=0;
            if ( f_res->chunks[c_p].l_st < f_res->chunks[c_p].l_en ) l_p = f_res->chunks[c_p].l_st; else l_p=0;
        } else {
            if ( f_res->chunks[c_p].l_st < f_res->chunks[c_p].l_en ) {
                l_p = f_res->chunks[c_p].l_st+l_o;
                if ( l_p >= f_res->chunks[c_p].l_en ) l_p = f_res->chunks[c_p].l_en-1;
            } else 
                l_p = f_res->chunks[c_p].l_st;
        }
        int   res=position;
        f_res->LetterToOffset(c_p,s_p,l_p,l_start,l_end,res);
        return contents->Do_UTF8_2_UCS4(res);
    }
    return i_position;
}

gint
sp_te_down (SPItem *item, gint i_position)
{
    flow_res* f_res = te_get_f_res (item);
    div_flow_src *contents = te_get_contents (item);

    int position=contents->Do_UCS4_2_UTF8(i_position);
    if ( f_res == NULL ) return i_position;
    int    c_p=-1, s_p=-1, l_p=-1;
    bool   l_start=false, l_end=false;
    f_res->OffsetToLetter(position,c_p,s_p,l_p,l_start,l_end);
    if ( c_p >= 0 ) {
        int c_o=l_p - f_res->chunks[c_p].l_st;
        if ( l_end ) {l_end=false;c_o++;}
        c_p++;
        if ( c_p >= f_res->nbChunk ) {
            c_p=f_res->nbChunk-1;
            if ( f_res->chunks[c_p].l_st < f_res->chunks[c_p].l_en ) l_p = f_res->chunks[c_p].l_en; else l_p = f_res->nbLetter;
        } else {
            if ( f_res->chunks[c_p].l_st < f_res->chunks[c_p].l_en ) {
                l_p = f_res->chunks[c_p].l_st+c_o;
                if ( l_p >= f_res->chunks[c_p].l_en ) l_p = f_res->chunks[c_p].l_en-1;
            } else l_p = f_res->nbLetter;
        }
        int   res=position;
        f_res->LetterToOffset(c_p,s_p,l_p,l_start,l_end,res);
        return contents->Do_UTF8_2_UCS4(res);
    }
    return i_position;
}

gint
sp_te_start_of_line (SPItem *item, gint i_position)
{
    flow_res* f_res = te_get_f_res (item);
    div_flow_src *contents = te_get_contents (item);

    int position=contents->Do_UCS4_2_UTF8(i_position);
    if ( f_res == NULL ) return i_position;
    int    c_p=-1,s_p=-1,l_p=-1;
    bool   l_start=false,l_end=false;
    f_res->OffsetToLetter(position,c_p,s_p,l_p,l_start,l_end);
    if ( c_p >= 0 ) {
        if ( l_p >= 0 ) {
            l_p=f_res->chunks[c_p].l_st;
        }
        l_start=true;
        l_end=false;
        int   res=position;
        f_res->LetterToOffset(c_p,s_p,l_p,l_start,l_end,res);
        return contents->Do_UTF8_2_UCS4(res);
    }
    return i_position;
}

gint
sp_te_end_of_line (SPItem *item, gint i_position)
{
    flow_res* f_res = te_get_f_res (item);
    div_flow_src *contents = te_get_contents (item);

    int position=contents->Do_UCS4_2_UTF8(i_position);
   if ( f_res == NULL ) return i_position;
    int    c_p=-1,s_p=-1,l_p=-1;
    bool   l_start=false,l_end=false;
    f_res->OffsetToLetter(position,c_p,s_p,l_p,l_start,l_end);
    if ( c_p >= 0 ) {
        if ( l_p >= 0 ) {
            l_p = f_res->chunks[c_p].l_en-1;
        }
        l_start=true; // otherwise ends up at beginning of next line
        l_end=false;
				if ( c_p >= f_res->nbChunk-1 ) {l_start=false;l_end=true;}
        int   res=position;
        f_res->LetterToOffset(c_p,s_p,l_p,l_start,l_end,res);
        return contents->Do_UTF8_2_UCS4(res);
    }
    return i_position;
}

guint
sp_te_get_position_by_coords (SPItem *item, NR::Point &i_p)
{
    flow_res* f_res = te_get_f_res (item);
    div_flow_src *contents = te_get_contents (item);

    if ( f_res == NULL ) return 0;

    NR::Matrix  im=sp_item_i2d_affine (item);
    im = im.inverse();

    NR::Point p = i_p * im;
    int    c_p=-1, s_p=-1, l_p=-1;
    bool   l_start=false, l_end=false;
    //printf("letter at position %f %f : ",p[0],p[1]);
    f_res->PositionToLetter(p[0],p[1],c_p,s_p,l_p,l_start,l_end);
    if ( l_p >= 0 || c_p >= 0 ) {
        //printf(" c=%i s=%i l=%i st=%i en=%i ",c_p,s_p,l_p,(l_start)?1:0,(l_end)?1:0);
        int position=0;
        f_res->LetterToOffset(c_p,s_p,l_p,l_start,l_end,position);
        //printf(" -> offset %i \n",position);
        return contents->Do_UTF8_2_UCS4(position);
    } else {
        //printf("none\n");
    }
    return 0;
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

    flow_res* f_res = te_get_f_res (item);
    flow_src* f_src = te_get_f_src (item);
    div_flow_src *contents = te_get_contents (item);

 	int utf8_pos=contents->Do_UCS4_2_UTF8(i_ucs4_pos);
    int  utf8_len=strlen(utf8);
    int  ucs4_len=0;
    for (gchar const *p = utf8; *p; p = g_utf8_next_char(p)) {
        ucs4_len++;
    }
    //g_print("insert '%s'(utf8:%d ucs4:%d) at %i\n",dump_hexy(utf8),utf8_len,ucs4_len,utf8_pos);
    if ( f_src == NULL ) { // no source text?
        return i_ucs4_pos;
    }
    if ( f_res == NULL ) {
        // no output but some input means: totally empty text
        int  ucs4_pos=0;
        one_flow_src* into=contents->Locate(0,ucs4_pos,true,false,true);
        if ( into && into->Type() == flw_text ) {
            // found our guy
            bool done=false;
            into->Insert(0,ucs4_pos,utf8,utf8_len,ucs4_len,done);
            SP_OBJECT(item)->updateRepr(SP_OBJECT_REPR(SP_OBJECT(item)),SP_OBJECT_WRITE_EXT);
            SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            return ucs4_len;
        }
        return i_ucs4_pos;
    }

    // round to the letter granularity
    int    c_st=-1,s_st=-1,l_st=-1;
    bool   l_start_st=false,l_end_st=false;
    f_res->OffsetToLetter(utf8_pos,c_st,s_st,l_st,l_start_st,l_end_st);
    if ( l_st < 0 ) return i_ucs4_pos;
    f_res->LetterToOffset(c_st,s_st,l_st,l_start_st,l_end_st,utf8_pos);
    //utf8_pos=f_res->letters[l_st].utf8_offset;
    int  ucs4_pos=f_res->letters[l_st].ucs4_offset;

    one_flow_src* cur = contents;
    bool  done=false;
    while ( cur && done == false ) {
        cur->Insert(utf8_pos,ucs4_pos,utf8,utf8_len,ucs4_len,done);
        cur=cur->next;
    }
    SP_OBJECT(item)->updateRepr(SP_OBJECT_REPR(SP_OBJECT(item)),SP_OBJECT_WRITE_EXT);
    SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    return i_ucs4_pos+ucs4_len;
}

/* Returns start position */
gint
sp_te_delete (SPItem *item, gint i_start, gint i_end)
{
    flow_res* f_res = te_get_f_res (item);
    flow_src* f_src = te_get_f_src (item);
    div_flow_src *contents = te_get_contents (item);

    int start = contents->Do_UCS4_2_UTF8(i_start);
    int end = contents->Do_UCS4_2_UTF8(i_end);
 	//printf("delete %i -> %i\n",start,end);

    if ( f_src == NULL || f_res == NULL ) 
        return i_start;

    // round to the letter granularity
    int    c_st=-1, s_st=-1, l_st=-1;
    int    c_en=-1, s_en=-1, l_en=-1;
    bool   l_start_st=false, l_end_st=false;
    bool   l_start_en=false, l_end_en=false;
    f_res->OffsetToLetter(start, c_st, s_st, l_st, l_start_st, l_end_st);
    f_res->OffsetToLetter(end, c_en, s_en, l_en, l_start_en, l_end_en);
    if ( l_start_st == false && l_end_st == false ) l_start_st=true;
    if ( l_start_en == false && l_end_en == false ) l_end_en=true;
    if ( l_st < 0 || l_en < 0 || l_st > l_en ) return i_start;
    if ( l_st == l_en && ( l_start_st == l_start_en || l_end_st == l_end_en ) ) return i_start;
    f_res->LetterToOffset(c_st, s_st, l_st, l_start_st, l_end_st, start);
    f_res->LetterToOffset(c_en, s_en, l_en, l_start_en, l_end_en, end);

    // Find the last ofc
    one_flow_src* last=NULL;
    for (one_flow_src* cur = contents; cur; cur=cur->next) {
        last=cur;
    }

    // list of line tspans that are to be merged
    GSList *lines_to_merge = NULL;

    for (one_flow_src* cur = last; cur; cur=cur->prev) {
        if (start <= cur->utf8_st && end >= cur->utf8_en && 
            SP_IS_TSPAN(cur->me) && SP_TSPAN(cur->me)->role != SP_TSPAN_ROLE_UNSPECIFIED) {
            // If the delete range fully covers this ofc and it comes from a line tspan, remember to merge it
            lines_to_merge = g_slist_prepend (lines_to_merge, cur->me);
        }
        // only those ofc's that overlap the delete range will be affected:
        cur->Delete(start, end);
    }

    SP_OBJECT(item)->updateRepr(SP_OBJECT_REPR(SP_OBJECT(item)),SP_OBJECT_WRITE_EXT);

    for (GSList *i = lines_to_merge; i; i = i->next) {
        SPObject *prev = SP_OBJECT_PREV (i->data);
        if (prev && SP_IS_TSPAN(prev) && SP_TSPAN(prev)->role != SP_TSPAN_ROLE_UNSPECIFIED) {
            // If the line to be merged has a prev sibling and it's also a line,
            for (SPObject *child = sp_object_first_child(SP_OBJECT(i->data)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
                // copy all children to it
                Inkscape::XML::Node *copy = SP_OBJECT_REPR (child)->duplicate();
                SP_OBJECT_REPR(prev)->appendChild(copy);
                sp_repr_unref (copy);
            }
            // delete line to be merged
            SP_OBJECT(i->data)->deleteObject();
        }
    }
    g_slist_free (lines_to_merge);

    SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    return i_start;
}

void
sp_te_get_cursor_coords (SPItem *item, gint i_position, NR::Point &p0, NR::Point &p1)
{
    flow_res* f_res = te_get_f_res (item);
    div_flow_src *contents = te_get_contents (item);

    int position = contents->Do_UCS4_2_UTF8(i_position);
    p0 = p1 = te_getOrigin (item);
    if ( f_res == NULL ) return;
    int c_p = -1, s_p = -1, l_p = -1;
    bool l_start = false, l_end = false;
    //printf("letter at offset %i : ",position);
    f_res->OffsetToLetter(position,c_p,s_p,l_p,l_start,l_end);
    //printf(" c=%i s=%i l=%i st=%i en=%i ",c_p,s_p,l_p,(l_start)?1:0,(l_end)?1:0);
    if ( l_p >= 0 ) {
        double npx,npy,npa,nps;
        f_res->LetterToPosition(c_p,s_p,l_p,l_start,l_end,npx,npy,nps,npa);
        p0=NR::Point(npx,npy);
        p1=NR::Point(-sin(npa),cos(npa));
        p1=p0 - nps*p1;
        //printf(" -> coord %f %f \n",npx,npy);
    } else {
        //printf("none\n");
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
