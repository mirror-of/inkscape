/*
 *  FlowSrc.cpp
 */

#include "FlowSrc.h"
#include "FlowSols.h"

#include "FlowStyle.h"
#include "FlowBoxes.h"
#include "FlowEater.h"
#include "FlowRes.h"
	
#include <math.h>

#include "../sp-object.h"
#include "../style.h"

flow_src::flow_src(void)
{
	nbElem=maxElem=0;
	elems=NULL;
	cur_holder=NULL;
	min_mode=true;
}
flow_src::~flow_src(void)
{
	for (int i=0;i<nbElem;i++) {
		if ( elems[i].type == flw_text ) {
			delete elems[i].text;
		}
	}
	if ( elems ) free(elems);
	nbElem=maxElem=0;
	elems=NULL;
}

char*               flow_src::Summary(void)
{
	int               nb=0,max=0;
	char*             res=(char*)malloc(sizeof(char));
	bool              follows_text=false;
	for (int i=0;i<nbElem;i++) {
		if ( elems[i].type == txt_text || elems[i].type == txt_tline || elems[i].type == txt_firstline || elems[i].type == txt_textpath ) {
			if ( follows_text ) {
				if ( nb+1 >= max ) {
					max=2*nb+1;
					res=(char*)realloc(res,(max+1)*sizeof(char));
				}
				res[nb]='\n';
				nb++;
			}
			follows_text=false;
		} else if ( elems[i].type == flw_line_brk ) {
			if ( nb+1 >= max ) {
				max=2*nb+1;
				res=(char*)realloc(res,(max+1)*sizeof(char));
			}
			res[nb]='\n';
			nb++;
		} else if ( elems[i].type == flw_text ) {
			text_holder* cur=elems[i].text;
			if ( nb+cur->utf8_length >= max ) {
				max=2*nb+cur->utf8_length;
				res=(char*)realloc(res,(max+1)*sizeof(char));
			}
			memcpy(res+nb,cur->utf8_text,cur->utf8_length*sizeof(char));
			nb+=cur->utf8_length;
			follows_text=true;
		}
	}
	res[nb]=0;
	return res;
}

void                flow_src::AddElement(int i_type,text_holder* i_text,one_flow_src* i_obj)
{
	if ( nbElem >= maxElem ) {
		maxElem=2*nbElem+1;
		elems=(one_elem*)realloc(elems,maxElem*sizeof(one_elem));
	}
	elems[nbElem].type=i_type;
	elems[nbElem].text=i_text;
	elems[nbElem].obj=i_obj;
	nbElem++;
}
void                flow_src::Prepare(void)
{
	// set source_start/source_end in text_holders
	for (int i=0;i<nbElem;i++) {
		if ( elems[i].type == flw_text ) {
			if ( i > 0 ) {
				if ( elems[i-1].type == txt_text || elems[i-1].type == txt_tline || elems[i-1].type == txt_firstline 
						 || elems[i-1].type == txt_textpath || elems[i-1].type == txt_span ) {
					elems[i].text->source_start=elems[i-1].obj;
				}
				if ( elems[i-1].type == flw_div || elems[i-1].type == flw_span || elems[i-1].type == flw_para 
						 || elems[i-1].type == flw_line_brk || elems[i-1].type == flw_rgn_brk ) {
					elems[i].text->source_start=elems[i-1].obj;
				}
			}
			if ( i+1 < nbElem ) {
				if ( elems[i+1].type == txt_text || elems[i+1].type == txt_tline || elems[i+1].type == txt_firstline 
						 || elems[i+1].type == txt_textpath || elems[i+1].type == txt_span ) {
					elems[i].text->source_end=elems[i+1].obj;
				}
				if ( elems[i+1].type == flw_div || elems[i+1].type == flw_span || elems[i+1].type == flw_para 
						 || elems[i+1].type == flw_line_brk || elems[i+1].type == flw_rgn_brk ) {
					elems[i].text->source_end=elems[i+1].obj;
				}
			}
		}
	}
	for (int i=0;i<nbElem;i++) {
		if ( elems[i].type == flw_text ) {
			elems[i].text->DoChunking(this);
			elems[i].text->SortCtrl();
			elems[i].text->ComputeBoxes();
		}
	}
	//Affiche();
}
void                flow_src::Affiche(void)
{
	printf("%i elements\n",nbElem);
	for (int i=0;i<nbElem;i++) {
		printf("%i: ",i);
		if ( elems[i].type == flw_text ) {
			printf("texte %i boites (st=%p en=%p)\n",elems[i].text->nbBox,elems[i].text->source_start,elems[i].text->source_end);
			elems[i].text->AfficheBoxes();
		} else if ( elems[i].type == flw_line_brk ) {
			printf("line break\n");
		} else if ( elems[i].type == flw_rgn_brk ) {
			printf("region break\n");
		} else if ( elems[i].type == flw_div ) {
			printf("flow div\n");
		} else if ( elems[i].type == flw_span ) {
			printf("flow span\n");
		} else if ( elems[i].type == txt_span ) {
			printf("text span\n");
		} else if ( elems[i].type == flw_para ) {
			printf("flow para\n");
		} else if ( elems[i].type == txt_text ) {
			printf("text %p\n",elems[i].obj);
		} else if ( elems[i].type == txt_firstline ) {
			printf("tspan firstline %p\n",elems[i].obj);
		} else if ( elems[i].type == txt_tline ) {
			printf("tspan line %p\n",elems[i].obj);
		} else if ( elems[i].type == txt_textpath ) {
			printf("textpath %p\n",elems[i].obj);
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
		if ( elems[0].text->nbBox > 0 ) flow_rtl=elems[0].text->paragraph_rtl;
	}
	if ( elems[from_no].type == flw_line_brk || elems[from_no].type == flw_rgn_brk ) {
		if ( from_no+1 < nbElem && elems[from_no+1].type == flw_text ) {
			// recompute rtl for the next part of the flow, if possible
			if ( elems[from_no+1].text->nbBox > 0 ) flow_rtl=elems[from_no+1].text->paragraph_rtl;
		}
		if ( fabs(ascent) < 0.01 && fabs(descent) < 0.01 ) {
			MetricsAt(from_no+1,0,ascent,descent,leading,flow_rtl);
		} else {
			// keep current dimensions
		}
	} else if ( elems[from_no].type == flw_div || elems[from_no].type == flw_para || elems[from_no].type == flw_span ) {
		if ( from_no+1 < nbElem && elems[from_no+1].type == flw_text ) {
			// recompute rtl for the next part of the flow, if possible
			if ( elems[from_no+1].text->nbBox > 0 ) flow_rtl=elems[from_no+1].text->paragraph_rtl;
		}
		// new paragraph->get dimensions
		MetricsAt(from_no+1,0,ascent,descent,leading,flow_rtl);
	} else if ( elems[from_no].type == txt_text || elems[from_no].type == txt_tline || elems[from_no].type == txt_firstline 
							|| elems[from_no].type == txt_textpath || elems[from_no].type == txt_span ) {
		if ( from_no+1 < nbElem && elems[from_no+1].type == flw_text ) {
			// recompute rtl for the next part of the flow, if possible
			if ( elems[from_no+1].text->nbBox > 0 ) flow_rtl=elems[from_no+1].text->paragraph_rtl;
		}
		if ( fabs(ascent) < 0.01 && fabs(descent) < 0.01 ) {
			MetricsAt(from_no+1,0,ascent,descent,leading,flow_rtl);
		} else {
			// keep current dimensions
		}
	} else if ( elems[from_no].type == flw_text ) {
		if ( from_pos < elems[from_no].text->nbBox ) {
			elems[from_no].text->MetricsAt(from_pos,ascent,descent,leading,flow_rtl);
		} else {
			MetricsAt(from_no+1,0,ascent,descent,leading,flow_rtl);
		}
	}
}
void                flow_src::ComputeSol(int from_no,int from_pos,line_solutions *sols,bool &flow_rtl)
{
	if ( from_no >= nbElem ) return;
	if ( min_mode ) sols->StartLine(from_no,from_pos);
	// line breaks
	if ( elems[from_no].type == flw_line_brk ) {
		if ( from_no+1 < nbElem && elems[from_no+1].type == flw_text ) {
			if ( elems[from_no+1].text->nbBox > 0 ) flow_rtl=elems[from_no+1].text->paragraph_rtl;
		}
		sols->ForceSol(from_no+1,0,true,false);
		return;	
	}
	// region breaks
	if ( elems[from_no].type == flw_rgn_brk ) {
		if ( from_no+1 < nbElem && elems[from_no+1].type == flw_text ) {
			if ( elems[from_no+1].text->nbBox > 0 ) flow_rtl=elems[from_no+1].text->paragraph_rtl;
		}
		sols->ForceSol(from_no+1,0,false,true);
		return;	
	}
	// informative elements
	if ( elems[from_no].type == flw_div || elems[from_no].type == flw_para || elems[from_no].type == flw_span || elems[from_no].type == txt_text 
			 || elems[from_no].type == txt_tline || elems[from_no].type == txt_firstline || elems[from_no].type == txt_textpath || elems[from_no].type == txt_span ) {
		ComputeSol(from_no+1,0,sols,flow_rtl);
		return;
	}
	// text elements
	if ( elems[from_no].type != flw_text ) return;
	if ( from_no == 0 && from_pos == 0 && elems[0].type == flw_text ) {
		if ( elems[0].text->nbBox > 0 ) flow_rtl=elems[0].text->paragraph_rtl;
	}
	bool   cur_last_in_rgn=false;
	bool   cur_last_in_para=false;
	if ( from_no+1 < nbElem ) {
		if ( elems[from_no+1].type == flw_rgn_brk ) cur_last_in_rgn=true;
		if ( elems[from_no+1].type == flw_line_brk ) cur_last_in_para=true;
		if ( elems[from_no+1].type == txt_text ) cur_last_in_para=true;
		if ( elems[from_no+1].type == txt_tline ) cur_last_in_para=true;
		if ( elems[from_no+1].type == txt_firstline ) cur_last_in_para=true;
		if ( elems[from_no+1].type == txt_textpath ) cur_last_in_para=true;
		if ( elems[from_no+1].type == txt_span ) cur_last_in_para=true; // if it's here, it's a chunk start
	} else {
		cur_last_in_rgn=true;
		cur_last_in_para=true;
	}
	if ( elems[from_no].text->nbBox > 0 && from_pos < elems[from_no].text->nbBox ) {
		if ( sols->in_leading_white ) {
			one_flow_src* t_start=elems[from_no].text->source_start;
			if ( t_start->me->xml_space.value == SP_XML_SPACE_PRESERVE ) sols->in_leading_white=false; // otherwise it gets eaten
		}
		if ( elems[from_no].text->ComputeSols(from_pos,sols,from_no,cur_last_in_para,cur_last_in_rgn,flow_rtl) ) return;
	} else if ( elems[from_no].text->nbBox <= 0 ) {
		if ( cur_last_in_para || cur_last_in_rgn ) {
			sols->ForceSol(from_no,0,cur_last_in_para,cur_last_in_rgn);
			return;
		}
	}
	// at this point we have a text_holder and from_pos is after it, so go to the next element
	ComputeSol(from_no+1,0,sols,flow_rtl);
}
void                flow_src::Feed(int st_no,int st_pos,int en_no,int en_pos,bool flow_rtl,flow_eater* baby)
{
	// warning: text_holder with 0 boxes will be flown over by the computeSol function
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
text_holder*        flow_src::ParagraphBetween(int st_no,int /*st_pos*/,int en_no,int en_pos)
{
	text_holder*     feeder=NULL;
	// warning: text_holder with 0 boxes will be flown over by the computeSol function
	for (int i=st_no;i<en_no;i++) {
		if ( elems[i].type == flw_text ) {
			if ( feeder == NULL ) feeder=elems[i].text;
		}
	}
	if ( en_pos > 0 || ( en_pos == 0 && elems[en_no].type == flw_text && elems[en_no].text->nbBox == 0 ) ) { // 
		if ( elems[en_no].type == flw_text ) {
			if ( feeder == NULL ) feeder=elems[en_no].text;
		}
	}
	return feeder;
}

/*
 *
 */

one_flow_src::one_flow_src(SPObject* i_me)
{
	me=i_me;
	ucs4_st=ucs4_en=0;
	utf8_st=utf8_en=0;
	next=prev=NULL;
	dad=chunk=NULL;
}
one_flow_src::~one_flow_src(void)
{
}
void              one_flow_src::Link(one_flow_src* after,one_flow_src* inside)
{
	if ( prev && prev->next == this ) prev->next=NULL;
	if ( next && next->prev == this ) next->prev=NULL;
	
	prev=after;
	next=NULL;
	if ( prev ) prev->next=this;
	dad=inside;
}
void              one_flow_src::SetPositions(bool /*for_text*/,int &last_utf8,int &last_ucs4,bool &/*in_white*/)
{
	ucs4_st=ucs4_en=last_ucs4;
	utf8_st=utf8_en=last_utf8;
	chunk=prev; // this is not a chunk start
	// in_white is unchanged
}
text_style*          one_flow_src::GetStyle(void)
{
	return NULL;
}
void              one_flow_src::PushInfo(int /*st*/,int /*en*/,int /*offset*/,text_holder* /*into*/)
{
}
void              one_flow_src::Fill(flow_src* /*what*/)
{
}
one_flow_src*     one_flow_src::Locate(int utf8_pos,int &ucs4_pos,bool src_start,bool src_end,bool must_be_text)
{
	one_flow_src* res=NULL;
	one_flow_src* cur=this;
	while ( cur ) {
		if ( utf8_pos < cur->utf8_st ) break; // no need to look further
		bool at_start=(utf8_pos==cur->utf8_st);
		bool at_end=(utf8_pos==cur->utf8_en);
		bool inside=(utf8_pos>cur->utf8_st&&utf8_pos<cur->utf8_en);
		if ( inside || at_start || at_end ) {
			if ( must_be_text == false || cur->Type() == flw_text ) {
				if ( inside ) {
					res=cur;
					break;
				} else {
					if ( src_start && at_start ) {
						res=cur;
						//break;
					} else if ( src_end && at_end ) {
						res=cur;
						//break;
					}
				}
			}
		}
		cur=cur->next;
	}
	if ( res ) {
		if ( res->Type() == flw_text ) {
			text_flow_src* tres=dynamic_cast<text_flow_src*>(res);
			ucs4_pos=res->ucs4_st+tres->cleaned_up.UTF8_2_UCS4(utf8_pos-res->utf8_st);
		} else {
			ucs4_pos=res->ucs4_st;
			if ( utf8_pos == res->utf8_st ) ucs4_pos=res->ucs4_st;
			if ( utf8_pos == res->utf8_en ) ucs4_pos=res->ucs4_en;
		}
	}
	return res;
}
void              one_flow_src::DoFill(flow_src* what)
{
	one_flow_src* cur=this;
	what->cur_holder=NULL;
	while ( cur ) {
		cur->Fill(what);
		cur=cur->next;
	}
}
void              one_flow_src::DoPositions(bool for_text)
{
	one_flow_src* cur=this;
	int           cur_last_ucs4=0;
	int           cur_last_utf8=0;
	bool          cur_white=true;
	while ( cur ) {
		cur->SetPositions(for_text,cur_last_ucs4,cur_last_utf8,cur_white);
		cur=cur->next;
	}
}
void              one_flow_src::Insert(int /*utf8_pos*/,int /*ucs4_pos*/,const char* /*n_text*/,int /*n_len*/,int /*n_ucs4_len*/,bool &/*done*/)
{
}
void              one_flow_src::Delete(int /*i_utf8_st*/,int /*i_utf8_en*/)
{
}
void              one_flow_src::DeleteInfo(int /*i_utf8_st*/,int /*i_utf8_en*/,int /*i_ucs4_st*/,int /*i_ucs4_en*/)
{
}
void              one_flow_src::AddValue(int /*utf8_pos*/,SPSVGLength &/*val*/,int /*type*/,bool /*increment*/)
{
}

// text variant	
text_flow_src::text_flow_src(SPObject* i_me):one_flow_src(i_me),cleaned_up(),string_to_me()
{
	cleaned_up.f_owner=this;
	string_to_me.SetDestination(&cleaned_up);
}
text_flow_src::~text_flow_src(void)
{
}
void              text_flow_src::SetStringText(partial_text* iTxt)
{
	string_to_me.nbSrc=0;
	string_to_me.AddSource(iTxt);
}
void              text_flow_src::SetPositions(bool for_text,int &last_utf8,int &last_ucs4,bool &in_white)
{
	chunk=prev;
	bool preserve=(me->xml_space.value == SP_XML_SPACE_PRESERVE);
	if ( for_text ) {
		string_to_me.PrepareForText(preserve,in_white);
	} else {
		string_to_me.PrepareForFlow(preserve,in_white);
	}
	ucs4_st=last_ucs4;
	ucs4_en=last_ucs4+cleaned_up.ucs4_length;
	utf8_st=last_utf8;
	utf8_en=last_utf8+cleaned_up.utf8_length;
	last_ucs4=ucs4_en;
	last_utf8=utf8_en;
}
void              text_flow_src::Fill(flow_src* what)
{	
	if ( what->cur_holder == NULL ) {
		what->cur_holder=new text_holder;
		what->AddElement(flw_text,what->cur_holder,this);
	}
	// add text
	text_holder* th=what->cur_holder;
	th->AppendUTF8(&cleaned_up);
	int     added_utf8_st=0,added_utf8_en=0;
	int     added_ucs4_st=0,added_ucs4_en=0;
	th->LastAddition(added_utf8_st,added_ucs4_st,added_utf8_en,added_ucs4_en);
	if ( added_utf8_st < 0 || added_utf8_st >= added_utf8_en ) return; // no codepoint -> basta
	// percolate styles from parent elements
	if ( dad ) {
		// no need to go further than dad
		text_style*  n_sp_style=dad->GetStyle();
		if ( n_sp_style ) {
			// we could reuse the text_style, maybe
			what->AddStyle(n_sp_style);
			th->AddStyleSpan(added_utf8_st,added_utf8_en,n_sp_style);
		}
	}
	one_flow_src*  cur_dad=dad;
	while ( cur_dad ) {
		cur_dad->PushInfo(added_ucs4_st,added_ucs4_en,ucs4_st,th);
		cur_dad=cur_dad->dad;
	}
}
void              text_flow_src::Insert(int utf8_pos,int /*ucs4_pos*/,const char* n_text,int n_len,int /*n_ucs4_len*/,bool &done)
{
	//printf("txt_insert %i in of %i %i\n",utf8_pos,utf8_st,utf8_en);
	if ( utf8_pos >= utf8_st && utf8_pos <= utf8_en /*|| ( utf8_st == utf8_en && utf8_st == utf8_pos && done == false )*/ ) {
		// will append on the span before, maybe it's the unexpected behaviour
		cleaned_up.Insert(utf8_pos-utf8_st,(char*)n_text,n_len);
		partial_text* s_st_txt=NULL;
		int           s_st_utf8_pos=0,s_st_ucs4_pos=0;
		int i_ucs4_st=cleaned_up.UTF8_2_UCS4(utf8_pos-utf8_st);
		i_ucs4_st+=ucs4_st;
		string_to_me.DestToSource(utf8_pos-utf8_st,i_ucs4_st-ucs4_st,s_st_utf8_pos,s_st_ucs4_pos,s_st_txt,false);
		if ( s_st_txt ) {
			s_st_txt->Insert(s_st_utf8_pos,(char*)n_text,n_len);
		}
		done=true;
	}
}
void              text_flow_src::Delete(int i_utf8_st,int i_utf8_en)
{
	//printf("txt_delete %i %i out of %i %i\n",i_utf8_st,i_utf8_en,utf8_st,utf8_en);
	if ( i_utf8_st < utf8_st ) i_utf8_st=utf8_st;
	if ( i_utf8_en > utf8_en ) i_utf8_en=utf8_en;
	if ( i_utf8_st >= i_utf8_en ) return;
	if ( i_utf8_st >= utf8_en ) return;
	if ( i_utf8_en <= utf8_st ) return;
	//int utf8_offset=i_utf8_st-utf8_st;
	//int utf8_suppr=i_utf8_en-i_utf8_st;
	int i_ucs4_st=cleaned_up.UTF8_2_UCS4(i_utf8_st-utf8_st);
	int i_ucs4_en=cleaned_up.UTF8_2_UCS4(i_utf8_en-utf8_st);
	i_ucs4_st+=ucs4_st;
	i_ucs4_en+=ucs4_st;
	
	//printf(" -> %i %i out of %i %i\n",i_ucs4_st,i_ucs4_en,ucs4_st,ucs4_en);
	cleaned_up.Delete(i_utf8_st-utf8_st,i_utf8_en-utf8_st);
	partial_text* s_st_txt=NULL;
	int           s_st_utf8_pos=0,s_st_ucs4_pos=0;
	int           s_en_utf8_pos=0,s_en_ucs4_pos=0;
	string_to_me.DestToSource(i_utf8_st-utf8_st,i_ucs4_st-ucs4_st,s_st_utf8_pos,s_st_ucs4_pos,s_st_txt,false);
	string_to_me.DestToSource(i_utf8_en-utf8_st,i_ucs4_en-ucs4_st,s_en_utf8_pos,s_en_ucs4_pos,s_st_txt,true);
	if ( s_st_txt ) {
		s_st_txt->Delete(s_st_utf8_pos,s_en_utf8_pos);
	}
	
	one_flow_src*  cur_dad=dad;
	while ( cur_dad ) {
		cur_dad->DeleteInfo(i_utf8_st,i_utf8_en,i_ucs4_st,i_ucs4_en);
		cur_dad=cur_dad->dad;
	}
}
void              text_flow_src::AddValue(int utf8_pos,SPSVGLength &val,int v_type,bool increment)
{
	if ( utf8_pos >= utf8_st && utf8_pos < utf8_en ) {
		int ucs4_pos=ucs4_st+cleaned_up.UTF8_2_UCS4(utf8_pos-utf8_st);
		if ( dad ) (dynamic_cast<div_flow_src*>(dad))->DoAddValue(utf8_pos,ucs4_pos,val,v_type,increment);
	}
}
// control stuff in the flow, like line and region breaks
control_flow_src::control_flow_src(SPObject* i_me,int i_type):one_flow_src(i_me)
{
	type=i_type;
}
control_flow_src::~control_flow_src(void)
{
}
void              control_flow_src::SetPositions(bool /*for_text*/,int &last_utf8,int &last_ucs4,bool &in_white)
{
	ucs4_st=last_ucs4;
	ucs4_en=last_ucs4+1;
	last_ucs4=ucs4_en;
	utf8_st=last_utf8;
	utf8_en=last_utf8+1;
	last_utf8=utf8_en;
	chunk=prev;
	in_white=true;
}
void              control_flow_src::Fill(flow_src* what)
{
	if ( type == flw_line_brk || type == flw_rgn_brk ) {
		what->cur_holder=NULL;
	}
	what->AddElement(type,NULL,this);
}

// object variant, to hold placement info
div_flow_src::div_flow_src(SPObject* i_me,int i_type):one_flow_src(i_me)
{
	type=i_type;
	vertical_layout=false;
	is_chunk_start=is_chunk_end=false;
	style=NULL;
	nb_x=nb_y=nb_rot=nb_dx=nb_dy=0;
	x_s=y_s=rot_s=dx_s=dy_s=NULL;
}
div_flow_src::~div_flow_src(void)
{
	if ( x_s ) free(x_s);
	if ( y_s ) free(y_s);
	if ( dx_s ) free(dx_s);
	if ( dy_s ) free(dy_s);
	if ( rot_s ) free(rot_s);
	nb_x=nb_y=nb_rot=nb_dx=nb_dy=0;
	x_s=y_s=rot_s=dx_s=dy_s=NULL;
	if ( style ) sp_style_unref(style);
}

void              div_flow_src::ReadArray(int &nb,SPSVGLength* &array,const char* value)
{
	if ( array ) free(array);
	array=NULL;
	nb=0;
	if ( value == NULL ) {
	} else {
		GList* list=sp_svg_length_list_read (value);
		nb=g_list_length(list);
		array=(SPSVGLength*)malloc(nb*sizeof(SPSVGLength));
		for (int i=0;i<nb;i++) sp_svg_length_unset (array+i, SP_SVG_UNIT_NONE, 0.0, 0.0);
		int    cur=0;
		for (GList* l=list;l;l=l->next) {
			SPSVGLength* nl=(SPSVGLength*)l->data;
			if ( cur < nb ) array[cur++]=*nl; // overcautious
			g_free(l->data);
		}
		g_list_free(list);
	}
}
char*             div_flow_src::WriteArray(int nb,SPSVGLength* array)
{
	if ( nb <= 0 || array == NULL ) return NULL;
	gchar c[32];
	gchar *s = NULL;
	
	for (int i=0;i<nb;i++) {
		g_ascii_formatd (c, sizeof (c), "%.8g", array[i].computed);
		if (i == 0) {
			s = g_strdup (c);
		}  else {
			s = g_strjoin (" ", s, c, NULL);
		}
	}
	return s;
}
void              div_flow_src::InsertArray(int l,int at,int &nb,SPSVGLength* &array,bool is_delta)
{
	if ( at < 0 ) {
		l+=at;
		at=0;
	}
	if ( l <= 0 ) return;
	if ( at >= nb ) return;
	if ( at+l <= 0 ) return; 
	if ( at == nb-1 && is_delta == false ) return;
	SPSVGLength cur=array[at];
	array=(SPSVGLength*)realloc(array,(nb+l)*sizeof(SPSVGLength));
	memmove(array+(at+l),array+at,(nb-at)*sizeof(SPSVGLength));
	if ( is_delta ) {
		cur.set=1;
		cur.value=cur.computed=0;
	}
	for (int i=0;i<l;i++) {
		array[i+at].set=1;
		array[i+at]=cur;
	}
	nb+=l;
}
void              div_flow_src::SuppressArray(int l,int at,int &nb,SPSVGLength* &array)
{
	if ( at+l >= nb ) l=nb-at;
	if ( l <= 0 ) return;
	if ( l >= nb ) {
		nb=0;
		if ( array ) free(array);
		array=NULL;
		return;
	}
	if ( nb > at+l ) {
		memmove(array+at,array+(at+l),(nb-(at+l))*sizeof(SPSVGLength));
	}
	nb-=l;
}
void              div_flow_src::UpdateArray(double size,double scale,int &nb,SPSVGLength* &array)
{
	for (int i=0;i<nb;i++) {
		if (array[i].unit == SP_SVG_UNIT_EM)
			array[i].computed = array[i].value * size;
		else if (array[i].unit == SP_SVG_UNIT_EX)
			array[i].computed = array[i].value * 0.5 * size;
		else if (array[i].unit == SP_SVG_UNIT_PERCENT)
			array[i].computed = array[i].value * scale * size; // ?
	}
}
void              div_flow_src::UpdateLength(double size,double scale)
{
	UpdateArray(size,scale,nb_x,x_s);
	UpdateArray(size,scale,nb_y,y_s);
	UpdateArray(size,scale,nb_dx,dx_s);
	UpdateArray(size,scale,nb_dy,dy_s);
	UpdateArray(size,scale,nb_rot,rot_s);
}
void              div_flow_src::ForceVal(int at,SPSVGLength &val,int &nb,SPSVGLength* &array,bool increment)
{
	if ( at < 0 ) return;
	if ( at >= nb ) {
		array=(SPSVGLength*)realloc(array,(at+1)*sizeof(SPSVGLength));
		for (int i=nb;i<=at;i++) {
			array[i].set=1;
			array[i].value=array[i].computed=0;
		}
		nb=at+1;
	}
	if ( increment == false || array[at].set == 0 ) {
		array[at]=val;
	} else {
		array[at].value+=val.value;
		array[at].computed+=val.computed;
	}
}
void              div_flow_src::SetStyle(SPStyle* i_style)
{
	if ( style ) sp_style_unref(style);
	style=i_style;
	if ( style ) sp_style_ref(style);
}
void							div_flow_src::SetX(const char* val)
{
	ReadArray(nb_x,x_s,val);
}
void							div_flow_src::SetY(const char* val)
{
	ReadArray(nb_y,y_s,val);
}
void							div_flow_src::SetDX(const char* val)
{
	ReadArray(nb_dx,dx_s,val);
}
void							div_flow_src::SetDY(const char* val)
{
	ReadArray(nb_dy,dy_s,val);
}
void							div_flow_src::SetRot(const char* val)
{
	ReadArray(nb_rot,rot_s,val);
}
char*							div_flow_src::GetX(int st,int en)
{
	if ( st < 0 ) st=0;
	if ( en < 0 ) en=nb_x;
	if ( st > nb_x ) return NULL;
	if ( en > nb_x ) en=nb_x;
	return WriteArray(en-st,(x_s)?x_s+st:NULL);
}
char*							div_flow_src::GetY(int st,int en)
{
	if ( st < 0 ) st=0;
	if ( en < 0 ) en=nb_y;
	if ( st > nb_y ) return NULL;
	if ( en > nb_y ) en=nb_y;
	return WriteArray(en-st,(y_s)?y_s+st:NULL);
}
char*							div_flow_src::GetDX(int st,int en)
{
	if ( st < 0 ) st=0;
	if ( en < 0 ) en=nb_dx;
	if ( st > nb_dx ) return NULL;
	if ( en > nb_dx ) en=nb_dx;
	return WriteArray(en-st,(dx_s)?dx_s+st:NULL);
}
char*							div_flow_src::GetDY(int st,int en)
{
	if ( st < 0 ) st=0;
	if ( en < 0 ) en=nb_dy;
	if ( st > nb_dy ) return NULL;
	if ( en > nb_dy ) en=nb_dy;
	return WriteArray(en-st,(dy_s)?dy_s+st:NULL);
}
char*							div_flow_src::GetRot(int st,int en)
{
	if ( st < 0 ) st=0;
	if ( en < 0 ) en=nb_rot;
	if ( st > nb_rot ) return NULL;
	if ( en > nb_rot ) en=nb_rot;
	return WriteArray(en-st,(rot_s)?rot_s+st:NULL);
}
text_style*          div_flow_src::GetStyle(void)
{
	text_style* n_style=new text_style;
	n_style->vertical_layout=vertical_layout;
	n_style->SetStyle(style);
	return n_style;
}
void              div_flow_src::PushInfo(int st,int en,int offset,text_holder* into)
{
	// offset= global ucs4_st of the addition
	// st,en= addition in the text_holder
	if ( vertical_layout ) {
		// dx for text_holder should always be the progression direction, so we swap dx and dy
		for (int i=offset;i<offset+en-st;i++) {
			int th_i=i-offset+st;
			int me_i=i-ucs4_en; // use ucs4_en otherwise the newline will be accounted for
			if ( me_i >= 0 && me_i < nb_dx && dx_s[me_i].set ) {
				double val=dx_s[me_i].computed;
				into->AddKerning(&val,th_i,th_i+1,false);
			}
			if ( me_i >= 0 && me_i < nb_dy && dy_s[me_i].set ) {
				double val=dy_s[me_i].computed;
				into->AddKerning(&val,th_i,th_i+1,true);
			}
		}
	} else {
		for (int i=offset;i<offset+en-st;i++) {
			int th_i=i-offset+st;
			int me_i=i-ucs4_en; // use ucs4_en otherwise the newline will be accounted for
			if ( me_i >= 0 && me_i < nb_dx && dx_s[me_i].set ) {
				double val=dx_s[me_i].computed;
				into->AddKerning(&val,th_i,th_i+1,true);
			}
			if ( me_i >= 0 && me_i < nb_dy && dy_s[me_i].set ) {
				double val=dy_s[me_i].computed;
				into->AddKerning(&val,th_i,th_i+1,false);
			}
		}
	}
}
void              div_flow_src::SetPositions(bool /*for_text*/,int &last_utf8,int &last_ucs4,bool &in_white)
{
	ucs4_st=last_ucs4;
	ucs4_en=last_ucs4;
	utf8_st=last_utf8;
	utf8_en=last_utf8;
	is_chunk_start=is_chunk_end=false;
	if ( type == flw_para || type == flw_div ) is_chunk_start=is_chunk_end=true;
	if ( type == txt_text || type == txt_textpath ) is_chunk_start=is_chunk_end=true;
	if ( type == txt_tline || type == txt_firstline ) is_chunk_start=true; // sodipodi:role=line is just the beginning of a line
	if ( type == txt_span && ( nb_x > 0 || nb_y > 0 ) ) is_chunk_start=true;
	if ( is_chunk_start ) {
		in_white=true;
		chunk=NULL; // chunk start
	} else {
		chunk=prev;
	}
	// extra tweaking
	if ( type == txt_tline || ( type == txt_span && is_chunk_start ) || type == txt_textpath ) {
		// add a ghost newline for the on-canvas text edition
		if ( ucs4_st > 0 ) { // if it's the first line, nothing to do anyway
			ucs4_en++;
			utf8_en++;
		}
		last_ucs4=ucs4_en;
		last_utf8=utf8_en;
	}
}
void              div_flow_src::Fill(flow_src* what)
{
	if ( is_chunk_start ) {
		what->cur_holder=NULL;
		what->AddElement(type,NULL,this);
	}
}
void              div_flow_src::Insert(int /*utf8_pos*/,int ucs4_pos,const char* /*n_text*/,int /*n_utf8_len*/,int n_ucs4_len,bool &/*done*/)
{
	int ucs4_offset=ucs4_pos-ucs4_en; // use ucs4_en otherwise the newline will be accounted for
	InsertArray(n_ucs4_len,ucs4_offset,nb_x,x_s,false);
	InsertArray(n_ucs4_len,ucs4_offset,nb_y,y_s,false);
	InsertArray(n_ucs4_len,ucs4_offset,nb_dx,dx_s,true);
	InsertArray(n_ucs4_len,ucs4_offset,nb_dy,dy_s,true);
	InsertArray(n_ucs4_len,ucs4_offset,nb_rot,rot_s,false);
}
void              div_flow_src::Delete(int i_utf8_st,int i_utf8_en)
{
	if ( utf8_st < utf8_en && i_utf8_st <= utf8_st && i_utf8_en >= utf8_en ) {
		// delete the return, ie remove the sodipodi:role=line
		utf8_st=utf8_en; // we mark it as deleted
	}
}
void              div_flow_src::DoAddValue(int /*utf8_pos*/,int ucs4_pos,SPSVGLength &val,int v_type,bool increment)
{
	int ucs4_offset=ucs4_pos-ucs4_en;
	if ( ucs4_offset >= 0 ) {
		if ( v_type == 0 ) ForceVal(ucs4_offset,val,nb_x,x_s,increment);
		if ( v_type == 1 ) ForceVal(ucs4_offset,val,nb_y,y_s,increment);
		if ( v_type == 2 ) ForceVal(ucs4_offset,val,nb_dx,dx_s,increment);
		if ( v_type == 3 ) ForceVal(ucs4_offset,val,nb_dy,dy_s,increment);
		if ( v_type == 4 ) ForceVal(ucs4_offset,val,nb_rot,rot_s,increment);
	}
}
void              div_flow_src::DeleteInfo(int /*i_utf8_st*/,int /*i_utf8_en*/,int i_ucs4_st,int i_ucs4_en)
{
	//printf("delete_info %i %i out of %i %i\n",i_ucs4_st,i_ucs4_en,ucs4_st,ucs4_en);
	if ( i_ucs4_st < ucs4_st ) i_ucs4_st=ucs4_st;
	if ( i_ucs4_en > ucs4_en ) i_ucs4_en=ucs4_en;
	int ucs4_offset=i_ucs4_st-ucs4_en; // use ucs4_en otherwise the newline will be accounted for
	int ucs4_suppr=i_ucs4_en-i_ucs4_st;
	if ( ucs4_offset >= 0 && ucs4_suppr > 0 ) {
		SuppressArray(ucs4_offset,ucs4_suppr,nb_x,x_s);
		SuppressArray(ucs4_offset,ucs4_suppr,nb_y,y_s);
		SuppressArray(ucs4_offset,ucs4_suppr,nb_dx,dx_s);
		SuppressArray(ucs4_offset,ucs4_suppr,nb_dy,dy_s);
		SuppressArray(ucs4_offset,ucs4_suppr,nb_rot,rot_s);
	}
}




