/*
 *  FlowSrcText.h
 */

#ifndef my_flow_srctext
#define my_flow_srctext

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "FlowDefs.h"

class text_holder;
class one_flow_src;

class partial_text {
public:
	char*            utf8_text;
	int              utf8_length,ucs4_length;
	// possible owners of this
	text_holder*     t_owner;
	one_flow_src*    f_owner;
	// temp data
	bool             white_start,white_end;
	
	partial_text(void);
	~partial_text(void);

	void             ResetText(void);
	void             AddSVGInputText(char* iText,int iLen);
	void             SetWhiteEnds(void);
	
	void             Delete(int st,int en);
	void             Insert(int at,char* iText,int iLen);
	
	int              UTF8_2_UCS4(int utf8_pos);
	int              UCS4_2_UTF8(int ucs4_pos);
};

class correspondance {
public:
	typedef struct corresp_src {
		partial_text*  txt;
		int            utf8_offset,ucs4_offset;
	} corresp_src;
	int              nbSrc,maxSrc;
	corresp_src*		 src;
	partial_text*    dst;
	
	typedef struct corresp {
		int            s_utf8_st,s_utf8_en;
		int            d_utf8_st,d_utf8_en;
		int            s_ucs4_st,s_ucs4_en;
		int            d_ucs4_st,d_ucs4_en;
		int            s_no;
	} corresp;
	int              nbCorr,maxCorr;
	corresp*         corrs;

	int              last_add_start;
	bool             currently_adding;

	correspondance(void);
	~correspondance(void);
	
	void             AddSource(partial_text* iSrc);
	void             SetDestination(partial_text* iDst);

	// special cases of correspondance
	// 1 - SPString contents -> text for SPText
	void             PrepareForText(bool preserve,bool &after_white);
	// 2 - SPString contents -> text for SPFlowtext
	void             PrepareForFlow(bool preserve,bool &after_white);
	// 3 - text_flow_src -> text_holder
	void             PrepareForMerge(void);
	
	void             StartAdding(void);
	void             TreatChar(int pos,bool add_it,int source_no);
	void             FlushAdding(int pos,int source_no);
	void             EndAdding(void);
	
	void             SourceToDest(int i_8_pos,int i_4_pos,partial_text* i_txt,int &o_8_pos,int &o_4_pos,bool is_end);
	void             DestToSource(int i_8_pos,int i_4_pos,int &o_8_pos,int &o_4_pos,partial_text* &o_txt,bool is_end);
	
	void             ResetCorrespondances(void);
	void             AddCorrespondance(int s_8_st,int s_8_en,int s_4_st,int s_4_en,int d_8_st,int d_8_en,int d_4_st,int d_4_en,int s_no);
	void             AppendDest(int source_st,int source_en,int source_no);
};

#endif
