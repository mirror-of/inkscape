/*
 *  FlowSrc.cpp
 */

#include "FlowSols.h"
#include "FlowSrc.h"

#include "FlowStyle.h"
#include "FlowBoxes.h"
#include "FlowEater.h"
	
#include <math.h>


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
	if ( s.descent > l_descent+0.001 || s.ascent+s.leading > l_ascent+l_leading+0.001 ) {
		// doesn't fit in the line
		//printf("oversized box: begin line enlarge  %i %i\n",end_no,end_pos);
		style_change=true;
		style_end_ascent=(s.ascent>l_ascent)?s.ascent:l_ascent;
		style_end_descent=(s.descent>l_descent)?s.descent:l_descent;
		style_end_leading=(s.ascent+s.leading>l_ascent+l_leading)?s.ascent+s.leading:l_ascent+l_leading;
		style_end_leading-=style_end_ascent;
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


