/*
 *  FlowEater.cpp
 */

#include "FlowEater.h"

//#include <config.h>
//#include <libnrtype/font-instance.h>

#include "FlowSrc.h"
#include "text_holder.h"
#include "text_style.h"

#include "../sp-flowdiv.h"
#include "../sp-flowregion.h"
#include "../sp-flowtext.h"

#define nflow_maker_verbose

flow_eater::flow_eater(void)
{
	the_flow=NULL;
	first_letter=false;
}
flow_eater::~flow_eater(void)
{
}

void          flow_eater::StartLine(bool rtl,double x,double y,double l_spacing)
{
	//printf("start line rtl=%i at=%f %f spc=%f\n",(rtl)?1:0,x,y,l_spacing);
	line_rtl=rtl;
	cur_length=word_length=next_length=0;
	cur_letter=word_letter=next_letter=0;
	line_st_x=x;
	line_st_y=y;
	line_spc=l_spacing;
	first_letter=true;
	if ( the_flow ) {
		the_flow->cur_spacing=line_spc;
		the_flow->last_c_style=NULL;
	}
}
void          flow_eater::StartWord(bool rtl,int nb_letter,double length)
{
//	printf("start word rtl=%i nb_l=%i l=%f\n",(rtl)?1:0,nb_letter,length);
	cur_length=next_length;
	cur_letter=next_letter;
	next_length+=length+nb_letter*line_spc;
	next_letter+=nb_letter;
	if ( first_letter ) next_length-=line_spc;
	word_rtl=rtl;
	if ( word_rtl == line_rtl ) {
		word_letter=cur_letter;
		word_length=cur_length;
	} else {
		word_letter=next_letter;
		word_length=next_length;
	}
	//printf("start word rtl=%i nb_l=%i l=%f\n",(rtl)?1:0,nb_letter,length);
	//printf("  -> word=%f %i  next=%f %i\n",word_length,word_letter,next_length,next_letter);
}
void          flow_eater::StartLetter(void)
{
//	printf("start letter\n");
	if ( word_rtl == line_rtl ) {
		word_letter++;
		if ( first_letter == false ) word_length+=line_spc;
	} else {
		word_letter--;
		word_length-=line_spc; // reverse, so always linespacing
	}
	first_letter=false;
}
void          flow_eater::Eat(int g_id,text_style* g_s,double g_x,double g_y,double g_w)
{
	double   px=line_st_x;
	if ( line_rtl ) {
		px-=word_length;
	} else {
		px+=word_length;
	}
	if ( word_rtl ) px-=g_w;
	if ( the_flow ) the_flow->AddGlyph(g_id,px+g_x,line_st_y+g_y,g_s);
//	printf("glyph %i at %f %f  w=%f -> p=%f\n",g_id,g_x,g_y,g_w,px);
	if ( word_rtl == line_rtl ) {
		word_length+=g_w;
	} else {
		word_length-=g_w;
	}
}
void          flow_eater::StartBox(bool rtl,int nb_letter,double length)
{
	cur_length=next_length;
	cur_letter=next_letter;
	next_length+=length+nb_letter*line_spc;
	next_letter+=nb_letter;
	if ( first_letter ) next_length-=line_spc;
	word_rtl=rtl;
	if ( word_rtl == line_rtl ) {
		word_letter=cur_letter;
		word_length=cur_length;
	} else {
		if ( the_flow ) the_flow->last_c_style=NULL;
		word_letter=next_letter;
		word_length=next_length;
	}
	//printf("start box rtl=%i nb_l=%i l=%f\n",(rtl)?1:0,nb_letter,length);
	//printf("  -> word=%f %i  next=%f %i\n",word_length,word_letter,next_length,next_letter);
}
void          flow_eater::Eat(char* iText,int iLen,double i_w,int i_l,text_style* i_style,double* k_x,double* k_y)
{
	if ( i_l <= 0 ) return;
	double  n_w=i_w+((double)i_l)*line_spc;
	if ( first_letter ) n_w-=line_spc;
	first_letter=false;

	double   px=line_st_x;
	if ( line_rtl ) {
		px-=word_length;
	} else {
		px+=word_length;
	}
	if ( word_rtl ) { // because the tspan are always ltr
		px-=n_w;
	}

	//printf("eat box tLen=%i w=%f l=%i\n",iLen,i_w,i_l);
	if ( the_flow ) {
		the_flow->AddChunk(iText,iLen,i_style,px,line_st_y,word_rtl);
		if ( k_x ) the_flow->KernLastAddition(k_x,true);
		if ( k_y ) the_flow->KernLastAddition(k_y,false);
	}
	if ( word_rtl == line_rtl ) {
		word_letter+=i_l;
		word_length+=n_w;
	} else {
		word_letter-=i_l;
		word_length-=n_w;
	}
}

/*
 *
 */
	
flow_maker::flow_maker(flow_src* i_src,flow_dest* i_dst)
{
	f_src=i_src;
	f_dst=i_dst;
	pending=new flow_tasks(this);
	nbBrk=maxBrk=0;
	brks=NULL;
	justify=false;
	par_indent=0;
	strictBefore=strictAfter=false;
	min_scale=0.8;
	max_scale=1.2;
	algo=0;
	elem_st_st=0;
	nbElmSt=maxElmSt=0;
	elem_start=NULL;
	nbKing=maxKing=0;
	kings=NULL;
	nbKgSt=maxKgSt=0;
	king_start=NULL;
}
flow_maker::~flow_maker(void)
{
	if ( kings ) free(kings);
	if ( king_start ) free(king_start);
	if ( elem_start ) free(elem_start);
	delete pending;
	if ( brks ) free(brks);
	nbBrk=maxBrk=0;
	brks=NULL;
	nbElmSt=maxElmSt=0;
	elem_start=NULL;
	nbKing=maxKing=0;
	kings=NULL;
	nbKgSt=maxKgSt=0;
	king_start=NULL;
}

int                flow_maker::AddBrk(box_sol& i_box,int i_no,int i_pos)
{
	if ( nbBrk >= maxBrk ) {
		maxBrk=2*nbBrk+1;
		brks=(flow_brk*)realloc(brks,maxBrk*sizeof(flow_brk));
	}
	new (brks+nbBrk) flow_brk(this,nbBrk,i_box);
	brks[nbBrk].SetEnd(i_no,i_pos);
	nbBrk++;
	return nbBrk-1;
}

flow_res*          flow_maker::Work(void)
{
	if ( algo == 0 ) return StdAlgo();
	if ( algo == 1 ) return KPAlgo();
	return NULL;
}
flow_res*          flow_maker::StdAlgo(void)
{
	flow_res*        f_res=new flow_res;
	line_solutions*  sols=new line_solutions;

	{
		// initialization
		box_sol          st_box;
		st_box.frame=f_dst;
		st_box.before_rgn=true;
		st_box.ascent=st_box.descent=st_box.leading=0;
		st_box.y=st_box.x_start=st_box.x_end=0;
		flow_requirement st_req;
		st_req.next_line=true;
		st_req.min_elem_no=st_req.min_elem_pos=-1;
		f_src->MetricsAt(0,0,st_req.ascent,st_req.descent,st_req.leading,st_req.rtl);
		if ( fabs(st_req.ascent+st_req.descent) < 0.01 ) {
			printf("null height line\n");
			delete sols;
			delete f_res;
			return NULL;
		}
		int   st_brk=AddBrk(st_box,0,0);
		brks[st_brk].rtl=st_req.rtl;
		brks[st_brk].para_end=true;
		pending->Push(st_brk,st_req);
	}
	
	flow_requirement   cur_req;
	int                cur_brk=-1;
	int                final_brk=-1;
	int                cur_id=-1;
	while ( pending->Pop(cur_brk,cur_req,cur_id) ) {
		box_sol  cur_end,cur_box;
		brks[cur_brk].FillBox(cur_end,cur_req.rtl);
		int      cur_elem_no=brks[cur_brk].elem_no;
		int      cur_elem_pos=brks[cur_brk].elem_pos;
		f_src->Clean(cur_elem_no,cur_elem_pos);
		if ( cur_elem_no >= f_src->nbElem ) {
			final_brk=cur_brk;
			break;
		}
#ifdef flow_maker_verbose
		printf("curbrk=%i (%i %i)\n",cur_brk,cur_elem_no,cur_elem_pos);
#endif
		bool     stillSameLine=true;
		double   bare_min_length=0;
		if ( brks[cur_brk].para_end ) {
			if ( f_src->elems[cur_elem_no].type == flw_text ) bare_min_length+=par_indent;
		}
		if ( f_src->elems[cur_elem_no].type == flw_text ) bare_min_length+=(cur_req.ascent+cur_req.descent)*3;
		if ( cur_req.rtl ) {
			cur_box=f_dst->TxenBox(cur_end,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.next_line,stillSameLine,bare_min_length);
			if ( brks[cur_brk].para_end ) {
				if ( f_src->elems[cur_elem_no].type == flw_text ) cur_box.x_end-=par_indent;
			}
		} else {
			cur_box=f_dst->NextBox(cur_end,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.next_line,stillSameLine,bare_min_length);
			if ( brks[cur_brk].para_end ) {
				if ( f_src->elems[cur_elem_no].type == flw_text ) cur_box.x_start+=par_indent;
			}
		}
#ifdef flow_maker_verbose
		printf("after %f %f %f comes %f %f %f\n",cur_end.x_start,cur_end.x_end,cur_end.y,cur_box.x_start,cur_box.x_end,cur_box.y);
#endif
		if ( cur_box.frame == NULL || cur_box.after_rgn == true ) {
			final_brk=cur_brk;
			break;
		}
		cur_req.next_line=false;
		if ( stillSameLine == false ) {
			if ( cur_req.min_elem_no >= 0 ) {
				if ( cur_req.min_brk != cur_brk ) {
					if ( brks[cur_brk].elem_no < cur_req.min_elem_no || ( brks[cur_brk].elem_no == cur_req.min_elem_no && brks[cur_brk].elem_pos < cur_req.min_elem_pos ) ) {
						// pas suffisant, on a pas reussi a caser la 1ere boite qui posait probleme
						if ( cur_req.min_fallback >= 0 ) {
#ifdef flow_maker_verbose
							printf("unfreeze %i\n",cur_req.min_fallback);
#endif
							pending->Thaw(cur_req.min_fallback);
						} else {
							printf("pas de fallback?\n");
						}
						continue;
					} else {
						cur_req.min_elem_no=-1;
					}
				}
			}
		}
		
		double   typ_length=cur_box.x_end-cur_box.x_start;
		sols->StartLine(brks[cur_brk].elem_no,brks[cur_brk].elem_pos);
		sols->SetLineSizes(cur_box.ascent,cur_box.descent,cur_box.leading);
		sols->NewLine(min_scale*typ_length,max_scale*typ_length,typ_length,strictBefore,strictAfter);
		f_src->ComputeSol(brks[cur_brk].elem_no,brks[cur_brk].elem_pos,sols,cur_req.rtl);
		sols->EndLine();
		
		//sols->Affiche();
	
		if ( sols->style_ending ) {
			bool old_rtl=cur_req.rtl;
			cur_req.ascent=sols->style_end_ascent;
			cur_req.descent=sols->style_end_descent;
			cur_req.leading=sols->style_end_leading;
			if ( fabs(cur_req.ascent+cur_req.descent) < 0.01 ) {
				// shouldn't happen
			} else {
				if ( sols->style_end_set == false ) {
					if ( stillSameLine ) {
						cur_req.next_line=true;
						cur_req.min_elem_no=cur_req.min_elem_pos=-1;
						cur_req.min_fallback=-1;
						int f_id=pending->Push(cur_brk,cur_req,true);
						cur_req.min_elem_no=sols->style_end_no;
						cur_req.min_elem_pos=sols->style_end_pos;
						cur_req.min_fallback=f_id;
						cur_req.min_brk=brks[cur_brk].prev_line_brk;
#ifdef flow_maker_verbose
						printf("new req a (%i %i) fall=%i \n",cur_req.min_elem_no,cur_req.min_elem_pos,cur_req.min_fallback);
#endif
						pending->Push(cur_req.min_brk,cur_req);
					} else {
						cur_req.next_line=true;
						cur_req.min_elem_no=cur_req.min_elem_pos=-1;
#ifdef flow_maker_verbose
						printf("no req\n");
#endif
						pending->Push(cur_brk,cur_req);
					}
				} else {
					line_solutions::one_sol  the_sol=sols->style_end_sol;
					int n_brk=AddBrk(cur_box,the_sol.elem_no,the_sol.pos);
					brks[n_brk].sol_box=the_sol.meas;
					brks[n_brk].rtl=old_rtl;
					brks[n_brk].SetContent(sols->min_line_no,sols->min_line_pos,the_sol.elem_no,the_sol.pos);
					brks[n_brk].LinkAfter(cur_brk,stillSameLine);
					cur_req.next_line=true;
					cur_req.min_elem_no=cur_req.min_elem_pos=-1;
					cur_req.min_fallback=-1;
					int f_id=pending->Push(n_brk,cur_req,true);
					cur_req.min_elem_no=sols->style_end_no;
					cur_req.min_elem_pos=sols->style_end_pos;
					cur_req.min_fallback=f_id;
					cur_req.min_brk=(stillSameLine)?brks[cur_brk].prev_line_brk:cur_brk;
#ifdef flow_maker_verbose
					printf("new req b (%i %i) fall=%i \n",cur_req.min_elem_no,cur_req.min_elem_pos,cur_req.min_fallback);
#endif
					pending->Push(cur_req.min_brk,cur_req);
				}						
			}
			continue;
		}

		if ( sols->nbSol <= 0 ) {
			// no sols-> ouch. skip to next box (there should be a penalty)
			int n_brk=AddBrk(cur_box,cur_elem_no,cur_elem_pos);
			brks[n_brk].rtl=cur_req.rtl;
			brks[n_brk].LinkAfter(cur_brk,stillSameLine);
			pending->Push(n_brk,cur_req);
			continue;				
		}
		
		int      best_sol=-1;
		double   best_score=0;
		for (int i=0;i<sols->nbSol;i++) {
			double sol_length=sols->sols[i].meas.width;
			if ( fabs(sol_length) < 0.01 ) {
				// special case: line break or rgn break
				// do not create chunks for these
				if ( sols->sols[i].para_end ) {
					best_sol=-1;
					cur_req.next_line=(stillSameLine)?true:false;
					cur_req.min_elem_no=cur_req.min_elem_pos=-1;
					bool   old_rtl=cur_req.rtl;
					f_src->MetricsAt(cur_elem_no+1,0,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.rtl);
					if ( cur_req.rtl ) {
						cur_box.x_start=cur_box.x_end;
					} else {
						cur_box.x_end=cur_box.x_start;
					}
					int n_brk=AddBrk(cur_box,cur_elem_no+1,0);
					brks[n_brk].rtl=old_rtl;
					brks[n_brk].LinkAfter(cur_brk,stillSameLine);
					brks[n_brk].para_end=true;
					brks[n_brk].SetContent(cur_elem_no,cur_elem_pos,cur_elem_no,cur_elem_pos);
					if ( fabs(cur_req.ascent+cur_req.descent) < 0.01 ) {
						// fini
						final_brk=n_brk;
					} else {
						pending->Push(n_brk,cur_req);
					}
					break;
				} else if ( sols->sols[i].rgn_end ) {
					best_sol=-1;
					cur_req.next_line=(stillSameLine)?true:false;
					cur_req.min_elem_no=cur_req.min_elem_pos=-1;
					bool   old_rtl=cur_req.rtl;
					f_src->MetricsAt(cur_elem_no+1,0,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.rtl);
					if ( cur_req.rtl ) {
						cur_box.x_start=cur_box.x_end;
					} else {
						cur_box.x_end=cur_box.x_start;
					}
					cur_box.frame=cur_box.frame->next_in_flow;
					cur_box.before_rgn=true;
					cur_box.after_rgn=false;
					int n_brk=AddBrk(cur_box,cur_elem_no+1,0);
					brks[n_brk].rtl=old_rtl;
					brks[n_brk].LinkAfter(cur_brk,stillSameLine);
					brks[n_brk].para_end=true;
					brks[n_brk].SetContent(cur_elem_no,cur_elem_pos,cur_elem_no,cur_elem_pos);
					if ( fabs(cur_req.ascent+cur_req.descent) < 0.01 ) {
						// fini
						final_brk=n_brk;
					} else {
						pending->Push(n_brk,cur_req);
					}
					break;
				} else {
					printf("oversmall solution?\n");
					break;
				}
			} else {
				double n_score=(sol_length>typ_length)?sol_length/typ_length:typ_length/sol_length;
				n_score-=1.0;
				if ( ( sols->sols[i].para_end || sols->sols[i].rgn_end ) && sol_length < typ_length ) n_score=0;
				if ( best_sol < 0 || n_score < best_score ) {
					best_sol=i;
					best_score=n_score;
				}
			}
		}
		if ( best_sol >= 0 ) {
			line_solutions::one_sol  the_sol=sols->sols[best_sol];
			double sol_length=the_sol.meas.width;
			double used_length=typ_length;
			if ( the_sol.para_end || the_sol.rgn_end ) {
				if ( sol_length < typ_length ) {
					used_length=sol_length;
					if ( cur_req.rtl ) {
						cur_box.x_start=cur_box.x_end-sol_length;
					} else {
						cur_box.x_end=cur_box.x_start+sol_length;
					}
				}
			}
#ifdef flow_maker_verbose
			printf("best= %i %i -> %i %i\n",sols->min_line_no,sols->min_line_pos,the_sol.elem_no,the_sol.pos);
#endif
			int n_brk=AddBrk(cur_box,the_sol.elem_no,the_sol.pos);
			brks[n_brk].sol_box=the_sol.meas;
			brks[n_brk].rtl=cur_req.rtl;
			brks[n_brk].SetContent(sols->min_line_no,sols->min_line_pos,the_sol.elem_no,the_sol.pos);
			brks[n_brk].LinkAfter(cur_brk,stillSameLine);
			f_src->MetricsAt(the_sol.elem_no,the_sol.pos,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.rtl);
			pending->Push(n_brk,cur_req);
			
		}
	}
		
	flow_eater*      baby=new flow_eater;
	baby->the_flow=f_res;
	int     brk_start=-1;
	for (int c_b=final_brk;c_b>=0;c_b=brks[c_b].prev_box_brk) {
		brk_start=c_b;
		if ( brks[c_b].prev_box_brk >= 0 ) brks[brks[c_b].prev_box_brk].next=c_b;
	}
#ifdef flow_maker_verbose
	printf("final:\n");
#endif
	for (int i=brk_start;i >= 0 && i < nbBrk;i=brks[i].next) {
		bool    line_rtl=brks[i].rtl;
		double  line_y=brks[i].used_box.y;
		double  used_length=brks[i].used_box.x_end-brks[i].used_box.x_start;
		double  sol_length=brks[i].sol_box.width;
		double  delta=used_length-sol_length;
#ifdef flow_maker_verbose
		printf("b=%i e_no=%i e_pos=%i (%i %i %i %i) x=%f y=%f\n",i,brks[i].elem_no,brks[i].elem_pos,
					 brks[i].u_st_no,brks[i].u_st_pos,brks[i].u_en_no,brks[i].u_en_pos,brks[i].used_box.x_end,brks[i].used_box.y);
#endif
		if ( brks[i].sol_box.nb_letter > 0 ) delta/=(brks[i].sol_box.nb_letter-1); else delta=0;
		if ( justify == false ) delta=0;
		if ( line_rtl ) {
			baby->StartLine(line_rtl,brks[i].used_box.x_end,line_y,delta);
		} else {
			baby->StartLine(line_rtl,brks[i].used_box.x_start,line_y,delta);
		}
		f_src->Feed(brks[i].u_st_no,brks[i].u_st_pos,brks[i].u_en_no,brks[i].u_en_pos,line_rtl,baby);
	}
	for (int i=brk_start;i >= 0 && i < nbBrk;i=brks[i].next) {
		bool    line_rtl=brks[i].rtl;
		double  line_y=brks[i].used_box.y;
		double  used_length=brks[i].used_box.x_end-brks[i].used_box.x_start;
		double  sol_length=brks[i].sol_box.width;
		double  delta=used_length-sol_length;

		if ( brks[i].sol_box.nb_letter > 0 ) delta/=(brks[i].sol_box.nb_letter-1); else delta=0;
		if ( justify == false ) delta=0;
		if ( line_rtl ) {
			baby->StartLine(line_rtl,brks[i].used_box.x_end,line_y,delta);
		} else {
			baby->StartLine(line_rtl,brks[i].used_box.x_start,line_y,delta);
		}
		f_src->Construct(brks[i].u_st_no,brks[i].u_st_pos,brks[i].u_en_no,brks[i].u_en_pos,line_rtl,baby);
	}
	
	delete baby;
	delete sols;
	return f_res;
}

flow_res*          flow_maker::KPAlgo(void)
{
	flow_res*        f_res=new flow_res;
	line_solutions*  sols=new line_solutions;

	{
		// initialization
		box_sol          st_box;
		st_box.frame=f_dst;
		st_box.before_rgn=true;
		st_box.ascent=st_box.descent=st_box.leading=0;
		st_box.y=st_box.x_start=st_box.x_end=0;
		flow_requirement st_req;
		st_req.next_line=true;
		st_req.min_elem_no=st_req.min_elem_pos=-1;
		f_src->MetricsAt(0,0,st_req.ascent,st_req.descent,st_req.leading,st_req.rtl);
		if ( fabs(st_req.ascent+st_req.descent) < 0.01 ) {
			printf("null height line\n");
			delete sols;
			delete f_res;
			return NULL;
		}
		int   st_brk=AddBrk(st_box,0,0);
		brks[st_brk].rtl=st_req.rtl;
		brks[st_brk].para_end=true;
		pending->Push(st_brk,st_req);
	}
	
	flow_requirement   cur_req;
	int                cur_brk=-1;
	int                final_brk=-1;
	int                cur_id=-1;
	while ( pending->Pop(cur_brk,cur_req,cur_id) ) {
		int      cur_elem_no=brks[cur_brk].elem_no;
		int      cur_elem_pos=brks[cur_brk].elem_pos;
		f_src->Clean(cur_elem_no,cur_elem_pos);
		if ( cur_elem_no >= f_src->nbElem ) {
			final_brk=cur_brk;
			break;
		}
		if ( f_src->elems[cur_elem_no].type != flw_text ) {
			box_sol  cur_end,cur_box;
			brks[cur_brk].FillBox(cur_end,cur_req.rtl);
			bool     stillSameLine=true;
			double   bare_min_length=0;
			if ( f_src->elems[cur_elem_no].type == flw_text ) bare_min_length+=par_indent;
			if ( cur_req.rtl ) {
				cur_box=f_dst->TxenBox(cur_end,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.next_line,stillSameLine,bare_min_length);
				if ( brks[cur_brk].para_end ) {
					if ( f_src->elems[cur_elem_no].type == flw_text ) cur_box.x_end-=par_indent;
				}
			} else {
				cur_box=f_dst->NextBox(cur_end,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.next_line,stillSameLine,bare_min_length);
				if ( brks[cur_brk].para_end ) {
					if ( f_src->elems[cur_elem_no].type == flw_text ) cur_box.x_start+=par_indent;
				}
			}
			if ( cur_box.frame == NULL || cur_box.after_rgn == true ) {
				final_brk=cur_brk;
				break;
			}
			
			if ( f_src->elems[cur_elem_no].type == flw_line_brk ) {
				cur_req.next_line=(stillSameLine)?true:false;
				cur_req.min_elem_no=cur_req.min_elem_pos=-1;
				bool   old_rtl=cur_req.rtl;
				f_src->MetricsAt(cur_elem_no+1,0,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.rtl);
				if ( cur_req.rtl ) {
					cur_box.x_start=cur_box.x_end;
				} else {
					cur_box.x_end=cur_box.x_start;
				}
				int n_brk=AddBrk(cur_box,cur_elem_no+1,0);
				brks[n_brk].rtl=old_rtl;
				brks[n_brk].LinkAfter(cur_brk,stillSameLine);
				brks[n_brk].para_end=true;
				if ( fabs(cur_req.ascent+cur_req.descent) < 0.01 ) {
					// fini
					final_brk=n_brk;
					break;
				} else {
					pending->Push(n_brk,cur_req);
				}
			} else if ( f_src->elems[cur_elem_no].type == flw_rgn_brk ) {
				cur_req.next_line=(stillSameLine)?true:false;
				cur_req.min_elem_no=cur_req.min_elem_pos=-1;
				bool   old_rtl=cur_req.rtl;
				f_src->MetricsAt(cur_elem_no+1,0,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.rtl);
				if ( cur_req.rtl ) {
					cur_box.x_start=cur_box.x_end;
				} else {
					cur_box.x_end=cur_box.x_start;
				}
				cur_box.frame=cur_box.frame->next_in_flow;
				cur_box.before_rgn=true;
				cur_box.after_rgn=false;
				int n_brk=AddBrk(cur_box,cur_elem_no+1,0);
				brks[n_brk].rtl=old_rtl;
				brks[n_brk].LinkAfter(cur_brk,stillSameLine);
				brks[n_brk].para_end=true;
				if ( fabs(cur_req.ascent+cur_req.descent) < 0.01 ) {
					// fini
					final_brk=n_brk;
					break;
				} else {
					pending->Push(n_brk,cur_req);
				}
			} else {
				printf("what's that elem type?!\n");
				break;
			}
		} else {
			int old_brk=cur_brk;
			KPDoPara(cur_brk,cur_req,sols);
			if ( cur_brk < 0 ) {
				final_brk=old_brk;
				break;
			} else {
				pending->Push(cur_brk,cur_req);
			}
		}
	}
		
	flow_eater*      baby=new flow_eater;
	baby->the_flow=f_res;
	int     brk_start=-1;
	for (int c_b=final_brk;c_b>=0;c_b=brks[c_b].prev_box_brk) {
		brk_start=c_b;
		if ( brks[c_b].prev_box_brk >= 0 ) brks[brks[c_b].prev_box_brk].next=c_b;
	}
#ifdef flow_maker_verbose
	printf("final:\n");
#endif
	for (int i=brk_start;i >= 0 && i < nbBrk;i=brks[i].next) {
		bool    line_rtl=brks[i].rtl;
		double  line_y=brks[i].used_box.y;
		double  used_length=brks[i].used_box.x_end-brks[i].used_box.x_start;
		double  sol_length=brks[i].sol_box.width;
		double  delta=used_length-sol_length;
#ifdef flow_maker_verbose
		printf("b=%i e_no=%i e_pos=%i (%i %i %i %i) d=%f\n",i,brks[i].elem_no,brks[i].elem_pos,
					 brks[i].u_st_no,brks[i].u_st_pos,brks[i].u_en_no,brks[i].u_en_pos,brks[i].delta_score);
#endif
		if ( brks[i].sol_box.nb_letter > 0 ) delta/=(brks[i].sol_box.nb_letter-1); else delta=0;
		if ( justify == false ) delta=0;
		if ( line_rtl ) {
			baby->StartLine(line_rtl,brks[i].used_box.x_end,line_y,delta);
		} else {
			baby->StartLine(line_rtl,brks[i].used_box.x_start,line_y,delta);
		}
		f_src->Feed(brks[i].u_st_no,brks[i].u_st_pos,brks[i].u_en_no,brks[i].u_en_pos,line_rtl,baby);
	}
	for (int i=brk_start;i >= 0 && i < nbBrk;i=brks[i].next) {
		bool    line_rtl=brks[i].rtl;
		double  line_y=brks[i].used_box.y;
		double  used_length=brks[i].used_box.x_end-brks[i].used_box.x_start;
		double  sol_length=brks[i].sol_box.width;
		double  delta=used_length-sol_length;
		if ( brks[i].sol_box.nb_letter > 0 ) delta/=(brks[i].sol_box.nb_letter-1); else delta=0;
		if ( justify == false ) delta=0;
		if ( line_rtl ) {
			baby->StartLine(line_rtl,brks[i].used_box.x_end,line_y,delta);
		} else {
			baby->StartLine(line_rtl,brks[i].used_box.x_start,line_y,delta);
		}
		f_src->Construct(brks[i].u_st_no,brks[i].u_st_pos,brks[i].u_en_no,brks[i].u_en_pos,line_rtl,baby);
	}
	
	delete baby;
	delete sols;
	return f_res;
}
double             flow_maker::KPBrkScore(int root_brk,int to_brk)
{
	double   score=brks[to_brk].Score(root_brk);
	double   covered=brks[to_brk].Length(root_brk);
	if ( covered > 0 ) return score/covered;
	return -1.0;
}
void               flow_maker::ResetElemStarts(int st)
{
	elem_st_st=st;
	nbElmSt=0;
	nbKing=0;
	nbKgSt=0;
	for (int i=0;i<maxKgSt;i++) king_start[i]=-1;
}
void               flow_maker::AddElems(int up_to)
{
	if ( up_to < elem_st_st ) return;
	if ( up_to < elem_st_st+nbElmSt ) return;
	int  len=up_to+1-elem_st_st;
	if ( len > maxElmSt ) {
		maxElmSt=len;
		elem_start=(int*)realloc(elem_start,maxElmSt*sizeof(int));
	}
	nbElmSt=len;
	elem_start[0]=0;
	for (int i=elem_st_st;i<up_to;i++) {
		int nl=0;
		if ( f_src->elems[i].type == flw_text ) {
			nl=f_src->elems[i].text->nbBox;
		} else {
			nl=1;
		}
		elem_start[i+1]=elem_start[i]+nl;
	}
	int max=(up_to>elem_st_st)?elem_start[up_to-elem_st_st-1]:0;
	if ( f_src->elems[up_to].type == flw_text ) {
		max+=f_src->elems[up_to].text->nbBox;
	} else {
		max+=1;
	}
	if ( max >= maxKgSt ) {
		int   oKgSt=maxKgSt;
		maxKgSt=max+1;
		king_start=(int*)realloc(king_start,maxKgSt*sizeof(int));
		for (int i=oKgSt;i<maxKgSt;i++) king_start[i]=-1;
	}
}
int                flow_maker::BrkIndex(int elem_no,int elem_pos)
{
	if ( elem_no < elem_st_st || elem_no >= elem_st_st+nbElmSt ) return -1;
	return elem_start[elem_no-elem_st_st]+elem_pos;
}
int                flow_maker::KingOf(int elem_no,int elem_pos,double x,double y)
{
	int index=BrkIndex(elem_no,elem_pos);
	if ( index < 0 ) return -1;
	int   cur=king_start[index];
	while ( cur >= 0 ) {
		if ( fabs(x-kings[cur].x) < 0.01 && fabs(y-kings[cur].y) < 0.01 ) {
			return cur;
		}
		cur=kings[cur].next;
	}
	return -1;
}
void							 flow_maker::AddKing(int brk,int elem_no,int elem_pos,double x,double y,double score)
{
	int index=BrkIndex(elem_no,elem_pos);
	if ( index < 0 ) return;
	if ( nbKing >= maxKing ) {
		maxKing=2*nbKing+1;
		kings=(brk_king*)realloc(kings,maxKing*sizeof(brk_king));
	}
	kings[nbKing].index=index;
	kings[nbKing].elem_no=elem_no;
	kings[nbKing].elem_pos=elem_pos;
	kings[nbKing].score=score;
	kings[nbKing].x=x;
	kings[nbKing].y=y;
	kings[nbKing].brk_no=brk;
	kings[nbKing].next=king_start[index];
	king_start[index]=nbKing;
	nbKing++;
}
void               flow_maker::KPDoPara(int &st_brk,flow_requirement &st_req,line_solutions *sols)
{
	int               cur_brk=st_brk;
	flow_requirement  cur_req=st_req;
	bool              very_first=true;
	flow_requirement  final_req=st_req;
	
	int               best_para_brk=-1;
	flow_requirement  best_para_req=st_req;
	double            best_para_score=0;
	int               cur_id=-1;
	cur_req.score_malus=0;
	ResetElemStarts(brks[cur_brk].elem_no);
	while ( very_first || pending->Pop(cur_brk,cur_req,cur_id) ) {
		if ( very_first ) {
			cur_brk=st_brk;
			cur_req=st_req;
			cur_id=-1;
			very_first=false;
		}
		int               cur_elem_no=brks[cur_brk].elem_no;
		int               cur_elem_pos=brks[cur_brk].elem_pos;
		f_src->Clean(cur_elem_no,cur_elem_pos);
		AddElems(cur_elem_no);
		double						cur_score=brks[cur_brk].Score(st_brk);
		bool              cur_rtl=cur_req.rtl;
		double            pending_malus=cur_req.score_malus;
		cur_req.score_malus=0;
		
		box_sol           cur_end,cur_box;
		brks[cur_brk].FillBox(cur_end,cur_rtl);
		bool     stillSameLine=true;
		double   bare_min_length=(cur_req.ascent+cur_req.descent)*3;
		if ( brks[cur_brk].para_end ) {
			bare_min_length+=par_indent;
		}
		if ( cur_req.rtl ) {
			cur_box=f_dst->TxenBox(cur_end,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.next_line,stillSameLine,bare_min_length);
			if ( brks[cur_brk].para_end ) cur_box.x_end-=par_indent;
		} else {
			cur_box=f_dst->NextBox(cur_end,cur_req.ascent,cur_req.descent,cur_req.leading,cur_req.next_line,stillSameLine,bare_min_length);
			if ( brks[cur_brk].para_end ) cur_box.x_start+=par_indent;
		}
		//printf("after %f %f %f comes %f %f %f\n",cur_end.x_start,cur_end.x_end,cur_end.y,cur_box.x_start,cur_box.x_end,cur_box.y);
		if ( cur_box.frame == NULL || cur_box.after_rgn == true ) {
			// pas d'ajout de malus puisqu'on finit
			double  n_score=KPBrkScore(st_brk,cur_brk);
			if ( n_score >= 0 ) {
				if ( best_para_brk < 0 || n_score < best_para_score ) {
					best_para_brk=cur_brk;
					best_para_req=cur_req;
					best_para_score=n_score;
 				}
			}
			continue;
		}
		cur_req.next_line=false;
		if ( stillSameLine == false ) {
			if ( cur_req.min_elem_no >= 0 ) {
				if ( cur_req.min_brk != cur_brk ) {
					if ( cur_elem_no < cur_req.min_elem_no || ( cur_elem_no == cur_req.min_elem_no && cur_elem_pos < cur_req.min_elem_pos ) ) {
						// pas suffisant, on a pas reussi a caser la 1ere boite qui posait probleme
						continue;
					} else {
						cur_req.min_elem_no=-1;
					}
				}
			}
		}
		
		double   typ_length=cur_box.x_end-cur_box.x_start;
		sols->StartLine(brks[cur_brk].elem_no,brks[cur_brk].elem_pos);
		sols->SetLineSizes(cur_box.ascent,cur_box.descent,cur_box.leading);
		sols->NewLine(min_scale*typ_length,max_scale*typ_length,typ_length,strictBefore,strictAfter);
		f_src->ComputeSol(brks[cur_brk].elem_no,brks[cur_brk].elem_pos,sols,cur_req.rtl);
		sols->EndLine();
		
		//sols->Affiche();
		
		if ( sols->style_ending ) {
			flow_requirement n_req=cur_req;
			n_req.ascent=sols->style_end_ascent;
			n_req.descent=sols->style_end_descent;
			n_req.leading=sols->style_end_leading;
			if ( fabs(n_req.ascent+n_req.descent) < 0.01 ) {
				// shouldn't happen
			} else {
				double skipped=0;
				if ( cur_req.rtl ) {
					skipped=f_dst->RemainingOnEnil(cur_end);
				} else {
					skipped=f_dst->RemainingOnLine(cur_end);
				}
				n_req.next_line=true;
				n_req.score_malus=pending_malus+2.5*skipped;
				if ( stillSameLine ) {
					n_req.min_elem_no=n_req.min_elem_pos=-1;
					n_req.min_fallback=-1;
					/*int f_id=*/pending->Push(cur_brk,n_req,false,false);
				}
				if ( brks[cur_brk].prev_line_brk >= st_brk ) {
					n_req.min_elem_no=sols->style_end_no;
					n_req.min_elem_pos=sols->style_end_pos;
					n_req.min_brk=brks[cur_brk].prev_line_brk;
					/*int f_id=*/pending->Push(n_req.min_brk,n_req,false,false);
				}
			}
		}
		
		if ( sols->nbSol <= 0 && sols->style_ending == false ) {
			flow_requirement n_req=cur_req;
			double   skipped=cur_box.x_end-cur_box.x_start;;
			double   n_score=pending_malus+2.5*skipped;
			// no sols-> ouch. skip to next box (there should be a penalty)
			f_src->MetricsAt(cur_elem_no,cur_elem_pos,n_req.ascent,n_req.descent,n_req.leading,cur_rtl);
			if ( fabs(n_req.ascent+n_req.descent) < 0.01 ) {
			} else {
				int  n_king=KingOf(cur_elem_no,cur_elem_pos,(cur_rtl)?cur_box.x_end:cur_box.x_start,cur_box.y);
				if ( n_king >= 0 ) {
					if ( cur_score+n_score < kings[n_king].score ) {
						int  o_brk=kings[n_king].brk_no;
						kings[n_king].score=cur_score+n_score;
						brks[o_brk].used_box=cur_box;
						brks[o_brk].delta_score=n_score;
						brks[o_brk].LinkAfter(cur_brk,stillSameLine);
						pending->Push(o_brk,n_req);
					}
				} else {
					int n_brk=AddBrk(cur_box,cur_elem_no,cur_elem_pos);
					brks[n_brk].delta_score=n_score;
					brks[n_brk].rtl=cur_rtl;
					brks[n_brk].LinkAfter(cur_brk,stillSameLine);
					AddKing(n_brk,cur_elem_no,cur_elem_pos,(cur_rtl)?cur_box.x_end:cur_box.x_start,cur_box.y,cur_score+n_score);
					pending->Push(n_brk,n_req);
				}
			}
		}
		
		for (int i=0;i<sols->nbSol;i++) {
			double sol_length=sols->sols[i].meas.width;
			if ( fabs(sol_length) < 0.01 ) {
				// special case: line break or rgn break
				// do not create chunks for these
				if ( sols->sols[i].para_end || sols->sols[i].rgn_end ) {
					// pas d'ajout de malus
					double  n_score=KPBrkScore(st_brk,cur_brk);
					if ( n_score >= 0 ) {
						if ( best_para_brk < 0 || n_score < best_para_score ) {
							best_para_brk=cur_brk;
							best_para_req=cur_req;
							best_para_score=n_score;
						}
					}
				} else {
					printf("oversmall solution?\n");
					break;
				}
			} else {
				line_solutions::one_sol  the_sol=sols->sols[i];
				box_sol                  n_box=cur_box;
				
				flow_requirement         n_req=cur_req;
				f_src->MetricsAt(the_sol.elem_no,the_sol.pos,n_req.ascent,n_req.descent,n_req.leading,cur_rtl);

				double sol_length=the_sol.meas.width;
				double used_length=typ_length;
				if ( the_sol.para_end || the_sol.rgn_end ) {
					if ( sol_length < typ_length ) {
						used_length=sol_length;
						if ( cur_rtl ) {
							n_box.x_start=n_box.x_end-sol_length;
						} else {
							n_box.x_end=n_box.x_start+sol_length;
						}
					}
				}

				double n_score=(sol_length>typ_length)?sol_length/typ_length:typ_length/sol_length;
				n_score-=1.0;
				n_score*=used_length;
				n_score+=pending_malus;
//				n_score*=n_score;

				int  n_king=KingOf(the_sol.elem_no,the_sol.pos,(cur_rtl)?cur_box.x_end:cur_box.x_start,cur_box.y);
				if ( n_king >= 0 ) {
					if ( cur_score+n_score < kings[n_king].score ) {
						kings[n_king].score=cur_score+n_score;
						int  o_brk=kings[n_king].brk_no;
						brks[o_brk].used_box=n_box;
						brks[o_brk].sol_box=the_sol.meas;
						brks[o_brk].delta_score=n_score;
						brks[o_brk].rtl=cur_rtl;
						brks[o_brk].SetContent(sols->min_line_no,sols->min_line_pos,the_sol.elem_no,the_sol.pos);
						brks[o_brk].LinkAfter(cur_brk,stillSameLine);
						pending->Push(o_brk,n_req);
					}
				} else {
					int n_brk=AddBrk(n_box,the_sol.elem_no,the_sol.pos);
					brks[n_brk].sol_box=the_sol.meas;
					brks[n_brk].delta_score=n_score;
					brks[n_brk].rtl=cur_rtl;
					brks[n_brk].SetContent(sols->min_line_no,sols->min_line_pos,the_sol.elem_no,the_sol.pos);
					brks[n_brk].LinkAfter(cur_brk,stillSameLine);
					AddKing(n_brk,the_sol.elem_no,the_sol.pos,(cur_rtl)?cur_box.x_end:cur_box.x_start,cur_box.y,cur_score+n_score);
					if ( the_sol.para_end || the_sol.rgn_end ) {
						n_score=KPBrkScore(st_brk,n_brk);
						if ( n_score >= 0 ) {
							if ( best_para_brk < 0 || n_score < best_para_score ) {
								best_para_brk=n_brk;
								best_para_req=n_req;
								best_para_score=n_score;
							}
						}
					} else {
						pending->Push(n_brk,n_req);
					}
				}
			}
		}
	}
	best_para_req.min_elem_no=-1;
	st_brk=best_para_brk;
	st_req=best_para_req;
}

