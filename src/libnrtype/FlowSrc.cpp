/*
 *  FlowSrc.cpp
 */

#include "FlowSrc.h"

#include "text_style.h"
#include "text_holder.h"
#include "FlowEater.h"
	
#include <math.h>

#define NOTflow_src_verbose

flow_src::flow_src(void)
{
	nbElem=maxElem=0;
	elems=NULL;
	nbStack=maxStack=0;
	stacks=NULL;
	styles=new flow_styles;
}
flow_src::~flow_src(void)
{
	for (int i=0;i<nbElem;i++) {
		if ( elems[i].type == flw_text ) {
			delete elems[i].text;
		}
	}
	if ( elems ) free(elems);
	if ( stacks ) free(stacks);
	delete styles;
	nbElem=maxElem=0;
	elems=NULL;
	nbStack=maxStack=0;
	stacks=NULL;
}

void                flow_src::Push(text_style* i_style)
{
//	printf("push\n");
	styles->AddStyle(i_style);
	if ( nbStack >= maxStack ) {
		maxStack=2*nbStack+1;
		stacks=(stack_elem*)realloc(stacks,maxStack*sizeof(stack_elem));
	}
	stacks[nbStack].ucs4_st=( nbStack > 0 )?stacks[nbStack-1].ucs4_en:0; 
	stacks[nbStack].ucs4_en=stacks[nbStack].ucs4_st;
	stacks[nbStack].style=i_style;
	stacks[nbStack].elem=-1;
	stacks[nbStack].nb_kern_x=0;
	stacks[nbStack].kern_x=NULL;
	stacks[nbStack].nb_kern_y=0;
	stacks[nbStack].kern_y=NULL;
	if ( nbStack > 0 ) stacks[nbStack].elem=stacks[nbStack-1].elem;
	nbStack++;
}
void                flow_src::SetKern(double *i_kern,int i_nb,bool is_x)
{
	if ( nbStack <= 0 ) return;
	stack_elem* cur=stacks+(nbStack-1);
	if ( is_x ) {
		cur->nb_kern_x=i_nb;
		cur->kern_x=i_kern;
	} else {
		cur->nb_kern_y=i_nb;
		cur->kern_y=i_kern;
	}
}
void                flow_src::Pop(void)
{
//	printf("pop\n");
	if ( nbStack >= 2 ) stacks[nbStack-2].ucs4_en=stacks[nbStack-1].ucs4_en;
	nbStack--;
	if ( nbStack < 0 ) nbStack=0;
}
void                flow_src::AddUTF8(char* iText,int iLen,bool force)
{
	if ( nbStack <= 0 ) {
		printf("no style yet\n");
		return;
	}
	stack_elem* cur=stacks+(nbStack-1);
	if ( cur->elem < 0 || force ) {
		if ( nbElem >= maxElem ) {
			maxElem=2*nbElem+1;
			elems=(one_elem*)realloc(elems,maxElem*sizeof(one_elem));
		}
		for (int i=nbStack-1;i>=0;i--) stacks[i].elem=nbElem;
		elems[nbElem].type=flw_text;
		elems[nbElem].text=new text_holder;
		nbElem++;
	}
	text_holder* th=elems[cur->elem].text;
	int     o,l;
	int     old_ucs4_length=th->ucs4_length;
	int     old_ucs4_en=cur->ucs4_en;
	int     ucs4_offset=old_ucs4_en-old_ucs4_length;
	o=th->AppendUTF8(iText,iLen,l);
	int			added_ucs4=th->ucs4_length-old_ucs4_length;
	cur->ucs4_en+=added_ucs4;
	
	if ( l > 0 && cur->style ) th->AddStyleSpan(o,o+l,cur->style);
	for (int c_s=nbStack-1;c_s>=0;c_s--) {
		if ( stacks[c_s].kern_x ) {
			if ( stacks[c_s].ucs4_st+stacks[c_s].nb_kern_x > old_ucs4_en ) {
				int   k_st=old_ucs4_en;
				int   k_en=cur->ucs4_en;
				if ( stacks[c_s].ucs4_st+stacks[c_s].nb_kern_x < k_en ) k_en=stacks[c_s].ucs4_st+stacks[c_s].nb_kern_x;
				th->AddKerning(stacks[c_s].kern_x+k_st,k_st-ucs4_offset,k_en-ucs4_offset,true);
			}
		}
		if ( stacks[c_s].kern_y ) {
			if ( stacks[c_s].ucs4_st+stacks[c_s].nb_kern_y > old_ucs4_en ) {
				int   k_st=old_ucs4_en;
				int   k_en=cur->ucs4_en;
				if ( stacks[c_s].ucs4_st+stacks[c_s].nb_kern_y < k_en ) k_en=stacks[c_s].ucs4_st+stacks[c_s].nb_kern_y;
				th->AddKerning(stacks[c_s].kern_y+k_st,k_st-ucs4_offset,k_en-ucs4_offset,false);
			}
		}
	}
}
void                flow_src::AddControl(int type)
{
	if ( nbElem >= maxElem ) {
		maxElem=2*nbElem+1;
		elems=(one_elem*)realloc(elems,maxElem*sizeof(one_elem));
	}
	elems[nbElem].type=type;
	elems[nbElem].text=NULL;
	nbElem++;
	for (int i=nbStack-1;i>=0;i--) stacks[i].elem=-1;
}
void                flow_src::Prepare(void)
{
	for (int i=0;i<nbElem;i++) {
		if ( elems[i].type == flw_text ) {
			elems[i].text->DoChunking(styles);
			elems[i].text->SortCtrl();
//			elems[i].text->AfficheCtrl();
			elems[i].text->ComputeBoxes();
//			elems[i].text->AfficheBoxes();
		}
	}
}
void                flow_src::Affiche(void)
{
	printf("%i elements\n",nbElem);
	for (int i=0;i<nbElem;i++) {
		printf("%i: ",i);
		if ( elems[i].type == flw_text ) {
			elems[i].text->AfficheBoxes();
		} else if ( elems[i].type == flw_line_brk ) {
			printf("line break\n");
		} else if ( elems[i].type == flw_rgn_brk ) {
			printf("region break\n");
		} else {
			printf("\n");
		}
	}
}

void                flow_src::Clean(int &from_no,int &from_pos)
{
	if ( from_pos < 0 ) {
		from_no--;
		from_pos=0;
	}
	if ( from_no < 0 ) from_no=0;
	if ( from_no >= nbElem ) return;
	if ( elems[from_no].type == flw_text ) {
		if ( from_pos >= elems[from_no].text->nbBox ) {
			from_no++;
			from_pos=0;
		}
	}
}
void                flow_src::MetricsAt(int from_no,int from_pos,double &ascent,double &descent,double &leading,bool &flow_rtl)
{
//	ascent=descent=leading=0;
	if ( from_no < 0 || from_no >= nbElem ) return;
	if ( from_no == 0 && from_pos == 0 && elems[0].type == flw_text ) {
		if ( elems[0].text->nbBox > 0 ) flow_rtl=elems[0].text->boxes[0].rtl;
	}
	if ( elems[from_no].type == flw_line_brk ) {
		if ( from_no+1 < nbElem && elems[from_no+1].type == flw_text ) {
			// recompute rtl for the next part of the flow, if possible
			if ( elems[from_no+1].text->nbBox > 0 ) flow_rtl=elems[from_no+1].text->boxes[0].rtl;
		}
		if ( fabs(ascent) < 0.01 && fabs(descent) < 0.01 ) {
			MetricsAt(from_no+1,0,ascent,descent,leading,flow_rtl);
		} else {
			// keep current dimensions
		}
	} else if ( elems[from_no].type == flw_rgn_brk ) {
		if ( from_no+1 < nbElem && elems[from_no+1].type == flw_text ) {
			// recompute rtl for the next part of the flow, if possible
			if ( elems[from_no+1].text->nbBox > 0 ) flow_rtl=elems[from_no+1].text->boxes[0].rtl;
		}
		if ( fabs(ascent) < 0.01 && fabs(descent) < 0.01 ) {
			MetricsAt(from_no+1,0,ascent,descent,leading,flow_rtl);
		} else {
			// keep current dimensions
		}
	} else if ( elems[from_no].type == flw_text ) {
		elems[from_no].text->MetricsAt(from_pos,ascent,descent,leading,flow_rtl);
	} 
}
void                flow_src::ComputeSol(int from_no,int from_pos,line_solutions *sols,bool &flow_rtl)
{
	if ( from_no >= nbElem ) return;
	sols->StartLine(from_no,from_pos);
	if ( elems[from_no].type == flw_line_brk ) {
		if ( from_no+1 < nbElem && elems[from_no+1].type == flw_text ) {
			// recompute rtl for the next part of the flow, if possible
			if ( elems[from_no+1].text->nbBox > 0 ) flow_rtl=elems[from_no+1].text->boxes[0].rtl;
		}
		sols->ForceSol(from_no+1,0,true,false);
		return;	
	}
	if ( elems[from_no].type == flw_rgn_brk ) {
		if ( from_no+1 < nbElem && elems[from_no+1].type == flw_text ) {
			if ( elems[from_no+1].text->nbBox > 0 ) flow_rtl=elems[from_no+1].text->boxes[0].rtl;
		}
		sols->ForceSol(from_no+1,0,false,true);
		return;	
	}
	if ( elems[from_no].type != flw_text ) return;
	if ( elems[from_no].text->nbBox <= 0 || elems[from_no].text->nbBox <= from_pos ) {
		ComputeSol(from_no+1,0,sols,flow_rtl);
		return;
	}
	if ( from_no == 0 && from_pos == 0 && elems[0].type == flw_text ) {
		if ( elems[0].text->nbBox > 0 ) flow_rtl=elems[0].text->boxes[0].rtl;
	}
	bool   cur_last_in_rgn=false;
	bool   cur_last_in_para=false;
	if ( from_no+1 < nbElem ) {
		if ( elems[from_no+1].type == flw_rgn_brk ) cur_last_in_rgn=true;
		if ( elems[from_no+1].type == flw_line_brk ) cur_last_in_para=true;
	} else {
		cur_last_in_rgn=true;
		cur_last_in_para=true;
	}
	if ( elems[from_no].text->ComputeSols(from_pos,sols,from_no,cur_last_in_para,cur_last_in_rgn,flow_rtl) ) return;
	if ( cur_last_in_rgn == false && cur_last_in_para == false ) {
		ComputeSol(from_no+1,0,sols,flow_rtl);
	}
}
void                flow_src::Feed(int st_no,int st_pos,int en_no,int en_pos,bool flow_rtl,flow_eater* baby)
{
	for (int i=st_no;i<en_no;i++) {
		if ( elems[i].type == flw_text ) {
			elems[i].text->Feed((i==st_no)?st_pos:0,elems[i].text->nbBox+1,flow_rtl,baby);
		}
	}
	if ( en_pos > 0 ) {
		if ( elems[en_no].type == flw_text ) {
			elems[en_no].text->Feed((en_no<=st_no)?st_pos:0,en_pos,flow_rtl,baby);
		}
	}
}

/*
 *
 */

line_solutions::line_solutions(void)
{
	elem_st=-1;
	pos_st=-1;
	nbSol=maxSol=0;
	sols=NULL;
	min_length=max_length=0;
	noBefore=noAfter=false;
}
line_solutions::~line_solutions(void)
{
	if ( sols ) free(sols);
	nbSol=maxSol=0;
	sols=NULL;
}

void            line_solutions::NewLine(double min,double max,double typ,bool strict_bef,bool strict_aft)
{
	min_length=min;
	max_length=max;
	typ_length=typ;
	noBefore=strict_bef;
	noAfter=strict_aft;
	nbSol=0;
	cur_line.ascent=cur_line.descent=cur_line.leading=0;
	cur_line.width=0;
	cur_line.nb_letter=0;
	last_line=last_word=cur_line;
	before_state=after_state=0;
	//printf("sols newline min %f max %g\n",min,max);
}
void						line_solutions::SetLineSizes(double ascent,double descent,double leading)
{
	l_ascent=ascent;
	l_descent=descent;
	l_leading=leading;
}
void            line_solutions::StartLine(int at_elem,int at_pos)
{
	cur_line.ascent=cur_line.descent=cur_line.leading=0;
	cur_line.width=0;
	cur_line.nb_letter=0;
	last_line=last_word=cur_line;
	min_line_no=elem_st=at_elem;
	min_line_pos=pos_st=at_pos;
	nbSol=0;
	before_state=after_state=0;
	in_leading_white=true;
	style_ending=false;
	style_end_set=false;
	no_style_ending=false;
	//printf("sols startline at (%i %i)\n",at_elem,at_pos);
}
void            line_solutions::EndLine(void)
{
	if ( before_state == 1 ) {
		nbSol++;
		before_state=2;
	}
}
void            line_solutions::StartWord(void)
{
	cur_line=last_word;
	//printf("sols startword %f\n",cur_line.width);
}
bool						line_solutions::PushBox(box_sizes &s,int end_no,int end_pos,bool is_white,bool last_in_para,bool last_in_rgn,bool is_word)
{
	if ( nbSol+1 >= maxSol ) { // may add 2 boxes in this function
		maxSol=2*nbSol+2;
		sols=(one_sol*)realloc(sols,maxSol*sizeof(one_sol));
	}
	
//	printf("sols pushbox (%i %i) l=%f w=%i iw=%i\n",end_no,end_pos,s.width,(is_white)?1:0,(is_word)?1:0);
	bool   style_change=false;
	if ( s.descent > l_descent || s.ascent+s.leading > l_ascent+l_leading) {
		// doesn't fit in the line

#ifdef flow_src_verbose
		printf("oversized box: begin line enlarge  %i %i\n",end_no,end_pos);
#endif

		style_change=true;

		style_end_ascent=(s.ascent>l_ascent)?s.ascent:l_ascent;
		style_end_descent=(s.descent>l_descent)?s.descent:l_descent;
		style_end_leading=(s.leading+s.ascent>l_ascent+l_leading)?s.leading+s.ascent-style_end_ascent:l_leading+l_ascent-style_end_ascent;

		style_end_no=end_no;
		style_end_pos=end_pos;
		
		if ( before_state == 1 ) {
			nbSol++;
			before_state=2;
		}
		if ( no_style_ending == false ) {
			style_ending=true;
		}
		return true;
	}

	last_line=cur_line;
	if ( in_leading_white ) {
		if ( is_white ) {
			min_line_no=end_no;
			min_line_pos=end_pos;
		} else {
			last_line.Add(s);
			in_leading_white=false;
		}
	} else {
		last_line.Add(s);
	}
	if ( is_word ) last_word=last_line;
	one_sol  n_s;
	n_s.meas=last_line;
	n_s.elem_no=end_no;
	n_s.pos=end_pos;
	n_s.para_end=last_in_para;
	n_s.rgn_end=last_in_rgn;
	style_ending=false;
	
	if ( is_white ) {
		// only add a sol if necessary
		if ( n_s.para_end || n_s.rgn_end ) {
			if ( before_state == 1 ) {
				nbSol++;
				before_state=2;
			}
			sols[nbSol]=n_s;
			nbSol++;
		} else {
			if ( nbSol > 0 ) {
				sols[nbSol-1].elem_no=end_no;
				sols[nbSol-1].pos=end_pos;
			}
		}
	} else {
		if ( last_line.width < min_length ) {
			style_end_sol=n_s;
			style_end_set=true;
			if ( noBefore == false ) {
				if ( before_state != 1 || sols[nbSol].meas.width < n_s.meas.width ) {
					sols[nbSol]=n_s;
					before_state=1;
				}
			}
		} else if ( last_line.width > max_length ) {
			style_end_sol=n_s;
			style_end_set=true;
			if ( before_state == 1 ) {
				nbSol++;
				before_state=2;
			}
			if ( noAfter == false && after_state == 0 ) {
				sols[nbSol]=n_s;
				nbSol++;
				after_state=1;
			}
		} else {
			style_end_sol=n_s;
			style_end_set=true;
			if ( before_state == 1 ) {
				nbSol++;
				before_state=2;
			}
			sols[nbSol]=n_s;
			nbSol++;
			if ( last_line.width >= typ_length ) no_style_ending=true;
		}
	}
	if ( n_s.para_end || n_s.rgn_end ) return true;
	if ( last_line.width > max_length ) return true;
	return false;
}
void            line_solutions::ForceSol(int end_no,int end_pos,bool last_in_para,bool last_in_rgn)
{
	if ( nbSol >= maxSol ) {
		maxSol=2*nbSol+1;
		sols=(one_sol*)realloc(sols,maxSol*sizeof(one_sol));
	}
	one_sol  n_s;
	n_s.meas=cur_line;
	n_s.elem_no=end_no;
	n_s.pos=end_pos;
	n_s.para_end=last_in_para;
	n_s.rgn_end=last_in_rgn;
	sols[nbSol++]=n_s;
}
void            line_solutions::Affiche(void)
{
	printf("%i solutions\n",nbSol);
	for (int i=0;i<nbSol;i++) {
		printf(" (%i %i) -> (%i %i) = %f %f %f / %f %i ep=%i er=%i\n",elem_st,pos_st,sols[i].elem_no,sols[i].pos
					 ,sols[i].meas.ascent,sols[i].meas.descent,sols[i].meas.leading,sols[i].meas.width,sols[i].meas.nb_letter
					 ,(sols[i].para_end)?1:0,(sols[i].rgn_end)?1:0);
	}
}


