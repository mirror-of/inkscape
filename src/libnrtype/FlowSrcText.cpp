/*
 *  FlowSrcText.cpp
 */

#include "FlowSrcText.h"
	
#include <math.h>

#include <glib.h>

	
partial_text::partial_text(void)
{
	utf8_text=NULL;
	utf8_length=ucs4_length=0;
	t_owner=NULL;
	f_owner=NULL;
	white_start=white_end=false;
}
partial_text::~partial_text(void)
{
	ResetText();
	t_owner=NULL;
	f_owner=NULL;
}

void             partial_text::ResetText(void)
{
	if ( utf8_text ) free(utf8_text);
	utf8_text=NULL;
	utf8_length=ucs4_length=0;
	white_start=white_end=false;
}
void             partial_text::AddSVGInputText(char* iText,int iLen)
{
	if ( iText == NULL ) return;
	if ( iLen < 0 ) iLen=strlen(iText);
	//if ( iLen <= 0 ) return; // add even though length is 0, because there may be empty strings
	utf8_text=(char*)realloc(utf8_text,(utf8_length+iLen+1)*sizeof(char));
	memcpy(utf8_text+utf8_length,iText,iLen*sizeof(char));
	utf8_length+=iLen;
	utf8_text[utf8_length]=0;
	ucs4_length=0;
	for (char* p=utf8_text;p&&*p;p=g_utf8_next_char(p)) {
		ucs4_length++;
	}
	//SetWhiteEnds();
}
void             partial_text::SetWhiteEnds(void)
{
	white_start=white_end=false;
	if ( utf8_length <= 0 || ucs4_length <= 0 ) return;
	for (char* p=utf8_text;p&&*p;p=g_utf8_next_char(p)) {
		gunichar c=g_utf8_get_char(p);
		if ( g_unichar_isspace(c) )  {
			if ( p == utf8_text ) white_start=true;
			white_end=true;
		} else {
			if ( p == utf8_text ) white_start=false;
			white_end=false;
		}
	}	
}
int              partial_text::UTF8_2_UCS4(int utf8_pos)
{
	int ucs4_pos=0;
	for (char* p=utf8_text;p&&*p;p=g_utf8_next_char(p)) {
		int d=((int)p)-((int)utf8_text);
		if ( d >= utf8_pos ) {
			return ucs4_pos;
		}
		ucs4_pos++;
	}
	return ucs4_length;
}
int              partial_text::UCS4_2_UTF8(int ucs4_pos)
{
	int cur_pos=0;
	for (char* p=utf8_text;p&&*p;p=g_utf8_next_char(p)) {
		int d=((int)p)-((int)utf8_text);
		if ( cur_pos >= ucs4_pos ) {
			return d;
		}
		cur_pos++;
	}
	return utf8_length;
}
void             partial_text::Delete(int st,int en)
{
	if ( st < 0 ) st=0;
	if ( en >= utf8_length ) en=utf8_length;
	if ( st >= en ) return;
	int rem=en-st;
	if ( en < utf8_length ) {
		memmove(utf8_text+st,utf8_text+(st+rem),(utf8_length-en)*sizeof(char));
	}
	utf8_length-=rem;
	utf8_text[utf8_length]=0;
}
void             partial_text::Insert(int at,char* iText,int iLen)
{
	if ( at < 0 ) {
		iText+=(-at);
		iLen+=at;
		at=0;
	}
	if ( iLen <= 0 ) return;
	utf8_text=(char*)realloc(utf8_text,(utf8_length+iLen+1)*sizeof(char));
	if ( at < utf8_length ) {
		memmove(utf8_text+at+iLen,utf8_text+at,(utf8_length-at)*sizeof(char));
	}
	memcpy(utf8_text+at,iText,iLen*sizeof(char));
	utf8_length+=iLen;
	utf8_text[utf8_length]=0;
}


/*
 *
 */
correspondance::correspondance(void)
{
	nbSrc=maxSrc=0;
	src=NULL;
	dst=NULL;
	nbCorr=maxCorr=0;
	corrs=NULL;
}
correspondance::~correspondance(void)
{
	if ( src ) free(src);
	if ( corrs ) free(corrs);
	nbSrc=maxSrc=0;
	src=NULL;
	dst=NULL;
	nbCorr=maxCorr=0;
	corrs=NULL;
}

void             correspondance::AddSource(partial_text* iSrc)
{
	if ( nbSrc >= maxSrc ) {
		maxSrc=2*nbSrc+1;
		src=(corresp_src*)realloc(src,maxSrc*sizeof(corresp_src));
	}
	src[nbSrc].txt=iSrc;
	src[nbSrc].utf8_offset=0;
	src[nbSrc].ucs4_offset=0;
	nbSrc++;
	ResetCorrespondances();
	if ( dst ) dst->ResetText();
}
void             correspondance::SetDestination(partial_text* iDst)
{
	dst=iDst;
	ResetCorrespondances();
	if ( dst ) dst->ResetText();
}

void             correspondance::StartAdding(void)
{
	last_add_start=0;
	currently_adding=false;
	ResetCorrespondances();
	dst->ResetText();
	int cur8=0,cur4=0;
	for (int i=0;i<nbSrc;i++) {
		src[i].utf8_offset=cur8;
		src[i].ucs4_offset=cur4;
		cur8+=src[i].txt->utf8_length;
		cur4+=src[i].txt->ucs4_length;
	}
}
void             correspondance::TreatChar(int pos,bool add_it,int s_no)
{
	if ( add_it ) {
		if ( currently_adding == false ) last_add_start=pos;
	} else {
		if ( currently_adding == true ) AppendDest(last_add_start,pos,s_no);
	}
	currently_adding=add_it;
}
void             correspondance::FlushAdding(int pos,int s_no)
{
	if ( currently_adding == true ) AppendDest(last_add_start,pos,s_no);
	currently_adding=false;
}
void             correspondance::EndAdding(void)
{
	dst->SetWhiteEnds();
}
void             correspondance::PrepareForText(bool preserve,bool &after_white)
{
	StartAdding();
	bool  in_white=after_white;
	for (int i=0;i<nbSrc;i++) {
		char* src_text=src[i].txt->utf8_text;
		int   src_length=src[i].txt->utf8_length;
		for (char* p=src_text;p&&*p;p=g_utf8_next_char(p)) {
			gunichar  c=g_utf8_get_char(p);
			int       d=((int)p)-((int)src_text);
			bool      add_it=true;
			if ( g_unichar_iscntrl(c) ) {
				add_it=false;
			} else if ( g_unichar_isspace(c) ) {
				if ( in_white && preserve == false ) add_it=false;
				in_white=true;
			} else {
				if ( c == 0x00AD ) add_it=false;
				in_white=false;
			}
			TreatChar(src[i].utf8_offset+d,add_it,i);
		}
		FlushAdding(src[i].utf8_offset+src_length,i);
	}
	EndAdding();

	if ( dst->utf8_length > 0 ) after_white=dst->white_end;
}
void             correspondance::PrepareForFlow(bool preserve,bool &after_white)
{
	StartAdding();
	bool  in_white=after_white;
	for (int i=0;i<nbSrc;i++) {
		char* src_text=src[i].txt->utf8_text;
		int   src_length=src[i].txt->utf8_length;
		for (char* p=src_text;p&&*p;p=g_utf8_next_char(p)) {
			gunichar  c=g_utf8_get_char(p);
			int       d=((int)p)-((int)src_text);
			bool      add_it=true;
			if ( g_unichar_iscntrl(c) ) {
				add_it=false;
			} else if ( g_unichar_isspace(c) ) {
				if ( in_white && preserve == false ) add_it=false;
				in_white=true;
			} else {
				in_white=false;
			}
			TreatChar(src[i].utf8_offset+d,add_it,i);
		}
		FlushAdding(src[i].utf8_offset+src_length,i);
	}
	EndAdding();

	if ( dst->utf8_length > 0 ) after_white=dst->white_end;
}
void             correspondance::PrepareForMerge(void)
{
	StartAdding();
	for (int i=0;i<nbSrc;i++) {
		int   src_length=src[i].txt->utf8_length;
		if ( src_length > 0 ) TreatChar(src[i].utf8_offset,true,i);
		FlushAdding(src[i].utf8_offset+src_length,i);
	}
	EndAdding();
}

void             correspondance::SourceToDest(int i_8_pos,int i_4_pos,partial_text* i_txt,int &o_8_pos,int &o_4_pos,bool is_end)
{
	//printf("source_to_dest %i %i %x ->",i_8_pos,i_4_pos,i_txt);
	int  s_no=-1;
	for (int i=0;i<nbSrc;i++) {
		if ( src[i].txt == i_txt ) {
			s_no=i;
			break;
		}
	}
	if ( s_no >= 0 ) {
		int  s_8_pos=i_8_pos+src[s_no].utf8_offset;
		int  s_4_pos=i_4_pos+src[s_no].ucs4_offset;
		o_8_pos=s_8_pos;
		o_4_pos=s_4_pos;
		for (int i=0;i<nbCorr;i++) {
			if ( is_end ) {
				if ( s_8_pos > corrs[i].s_utf8_st && s_8_pos <= corrs[i].s_utf8_en ) {
					o_8_pos=s_8_pos-corrs[i].s_utf8_st+corrs[i].d_utf8_st;
					o_4_pos=s_4_pos-corrs[i].s_ucs4_st+corrs[i].d_ucs4_st;
					//printf(" %i %i (s=%i)\n",o_8_pos,o_4_pos,s_no);
					return;
				}
			} else {
				if ( s_8_pos >= corrs[i].s_utf8_st && s_8_pos < corrs[i].s_utf8_en ) {
					o_8_pos=s_8_pos-corrs[i].s_utf8_st+corrs[i].d_utf8_st;
					o_4_pos=s_4_pos-corrs[i].s_ucs4_st+corrs[i].d_ucs4_st;
					//printf(" %i %i (s=%i)\n",o_8_pos,o_4_pos,s_no);
					return;
				}
			}
		}
		o_8_pos=i_txt->utf8_length+src[s_no].utf8_offset;
		o_4_pos=i_txt->ucs4_length+src[s_no].utf8_offset;
		//printf(" default %i %i (s=%i)\n",o_8_pos,o_4_pos,s_no);
		return;
	}
	o_8_pos=o_4_pos=-1;
	//printf(" none\n");
}
void             correspondance::DestToSource(int i_8_pos,int i_4_pos,int &o_8_pos,int &o_4_pos,partial_text* &o_txt,bool is_end)
{
	//printf("dest_to_source %i %i ->",i_8_pos,i_4_pos);
	o_8_pos=i_8_pos;
	o_4_pos=i_4_pos;
	o_txt=NULL;
	for (int i=0;i<nbCorr;i++) {
		if ( is_end ) {
			if ( i_8_pos > corrs[i].d_utf8_st && i_8_pos <= corrs[i].d_utf8_en ) {
				o_8_pos=i_8_pos-corrs[i].d_utf8_st+corrs[i].s_utf8_st;
				o_4_pos=i_4_pos-corrs[i].d_ucs4_st+corrs[i].s_ucs4_st;
				o_8_pos-=src[corrs[i].s_no].utf8_offset;
				o_4_pos-=src[corrs[i].s_no].ucs4_offset;
				o_txt=src[corrs[i].s_no].txt;
				//printf(" %i %i %x (s=%i o8=%i o4=%i)\n",o_8_pos,o_4_pos,o_txt,corrs[i].s_no,src[corrs[i].s_no].utf8_offset,src[corrs[i].s_no].ucs4_offset);
				return;
			}
		} else {
			if ( i_8_pos >= corrs[i].d_utf8_st && i_8_pos < corrs[i].d_utf8_en ) {
				o_8_pos=i_8_pos-corrs[i].d_utf8_st+corrs[i].s_utf8_st;
				o_4_pos=i_4_pos-corrs[i].d_ucs4_st+corrs[i].s_ucs4_st;
				o_8_pos-=src[corrs[i].s_no].utf8_offset;
				o_4_pos-=src[corrs[i].s_no].ucs4_offset;
				o_txt=src[corrs[i].s_no].txt;
				//printf(" %i %i %x (s=%i o8=%i o4=%i)\n",o_8_pos,o_4_pos,o_txt,corrs[i].s_no,src[corrs[i].s_no].utf8_offset,src[corrs[i].s_no].ucs4_offset);
				return;
			}
		}
	}
	if ( nbSrc > 0 ) {
		o_txt=src[nbSrc-1].txt;
		o_8_pos=o_txt->utf8_length;
		o_4_pos=o_txt->ucs4_length;
		//printf(" default: %i %i %x\n",o_8_pos,o_4_pos,o_txt);
	} else {
		//printf(" none\n");
	}
}

void             correspondance::ResetCorrespondances(void)
{
	nbCorr=0;
}
void             correspondance::AddCorrespondance(int s_8_st,int s_8_en,int s_4_st,int s_4_en,int d_8_st,int d_8_en,int d_4_st,int d_4_en,int s_no)
{
	if ( nbCorr >= maxCorr ) {
		maxCorr=2*nbCorr+1;
		corrs=(corresp*)realloc(corrs,maxCorr*sizeof(corresp));
	}
	corrs[nbCorr].s_utf8_st=s_8_st;
	corrs[nbCorr].s_utf8_en=s_8_en;
	corrs[nbCorr].s_ucs4_st=s_4_st;
	corrs[nbCorr].s_ucs4_en=s_4_en;
	corrs[nbCorr].d_utf8_st=d_8_st;
	corrs[nbCorr].d_utf8_en=d_8_en;
	corrs[nbCorr].d_ucs4_st=d_4_st;
	corrs[nbCorr].d_ucs4_en=d_4_en;
	corrs[nbCorr].s_no=s_no;
	nbCorr++;
}
void             correspondance::AppendDest(int is_8_st,int is_8_en,int s_no)
{
	if ( s_no < 0 || s_no >= nbSrc ) return;
	if ( is_8_st >= is_8_en ) return;
	char*     src_text=src[s_no].txt->utf8_text;
	//int       src_length=src[s_no].txt->utf8_length;
	int       s_8_st=is_8_st-src[s_no].utf8_offset,s_8_en=is_8_en-src[s_no].utf8_offset;
	int       s_4_st=0,s_4_en=0;
	for (char* p=src_text;p&&*p;p=g_utf8_next_char(p)) {
		int   d=((int)p)-((int)src_text);
		if ( d >= s_8_st ) break;
		s_4_st++;
	}
	for (char* p=src_text;p&&*p;p=g_utf8_next_char(p)) {
		int   d=((int)p)-((int)src_text);
		if ( d >= s_8_en ) break;
		s_4_en++;
	}
	int   is_4_st=s_4_st+src[s_no].ucs4_offset,is_4_en=s_4_en+src[s_no].ucs4_offset;
	int   n_8_len=s_8_en-s_8_st;
	int   n_4_len=s_4_en-s_4_st;
	AddCorrespondance(is_8_st,is_8_en,is_4_st,is_4_en,dst->utf8_length,dst->utf8_length+n_8_len,dst->ucs4_length,dst->ucs4_length+n_4_len,s_no);
	dst->AddSVGInputText(src[s_no].txt->utf8_text+s_8_st,n_8_len);
}



