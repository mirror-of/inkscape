/*
 *  FlowSrcText.h
 */

#ifndef my_flow_srctext
#define my_flow_srctext

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FlowDefs.h"

class text_holder;
class one_flow_src;

/** /brief stores a UTF-8 string

Stores a UTF-8 string. Why it can't inherit from Glib::ustring I don't
know. There are a few added utility methods; read on...
*/
class partial_text {
public:
    /** A 0-terminated string, storage allocated by the class. */
	char*            utf8_text;
    /** Length of \a utf8_text in bytes */
	int              utf8_length;
    /** Length of \a utf8_text in characters, always kept synchronised,
    except for what I am almost certain are a couple of bugs */
    int              ucs4_length;

	// possible owners of this
    /** set to NULL by the constructor and never touched by this class
    after that. Set by the text_holder constructor to 'this' and never
    touched by anybody else. */
	text_holder*     t_owner;

    /** set to NULL by the constructor and never touched by this class
    after that. Set by the text_flow_src constructor to 'this' and
    used by flow_res::ComputeLetterOffsets(). */
	one_flow_src*    f_owner;

    /** As calculated by SetWhiteEnds() */
	bool             white_start,white_end;
	
	partial_text(void);
	~partial_text(void);

    /** Clear the stored string and reset it to zero length */
	void             ResetText(void);

    /** Append the given text to #utf8_text. iLen is in bytes.
    Given the name, you may expect that some XML processing might happen;
    it doesn't. */
	void             AddSVGInputText(char* iText,int iLen);

    /** Scan through the stored string. If it starts with whitespace set
    #white_start = true, likewise for the end. Otherwise they are set to
    false. It does not remove said whitespace from the stored string. */
	void             SetWhiteEnds(void);
	
    /** Delete characters from the stored string starting at the st'th
    byte and continuing up to, but not including, the en'th. If st or en
    are out of bounds they are clamped. If st > en the method does
    nothing. TODO: bug? #ucs4_length is not updated. */
	void             Delete(int st,int en);

    /** Inserts the given string (iLen in bytes) before the at'th byte
    of the stored string. TODO: bug? #ucs4_length is not updated.

    If at is negative something very strange happens: the absolute value
    of at is subtracted from iText and added to iLen so as to extend the
    source string into what should be unexplored territory. This longer
    string is then inserted before the first byte of the stored string. */
	void             Insert(int at,char* iText,int iLen);
	
    /** returns the character number of the utf8_pos'th byte. If that byte
    does not mark the beginning of a character then the index of the next
    character after it is returned. If the stored string is not that long
    then #ucs4_length is returned. */
	int              UTF8_2_UCS4(int utf8_pos);

    /** returns the byte location of the ucs4_pos'th character. If the
    stored string is not that long, #utf8_length is returned. */
	int              UCS4_2_UTF8(int ucs4_pos);
};

/** \brief internal libnrtype class

Internal libnrtype class. This is a generic class that takes a set of
partial_text's as input and creates a single partial_text as output,
while maintaining the ability to convert character or byte positions in
both directions, ie the ability to ask "given that I am looking at the n'th
character of the output string, where did that come from" and vice-versa.

Usage:
There are two usage scenarios, an automated one and a manual one.
-# construct
-# Call \a SetDestination
-# Call \a AddSource as many times as necessary
-#
  - Manual: Call one of PrepareForText(), PrepareForFlow() or
    PrepareForMerge()
  - Manual
    -# Call StartAdding()
    -# Call TreatChar() and FlushAdding() as required
    -# Call EndAdding()
-# Call SourceToDest() and/or DestToSource() as necessary
*/
class correspondance {
public:
	typedef struct corresp_src {
		partial_text*  txt;
		int            utf8_offset,ucs4_offset;
	} corresp_src;
	int              nbSrc,maxSrc;
    /** Yet another std::vector reimplementation. Describes the list of
    text sources used for conversion. New items should be added with
    AddSource(). The _offset members specify the number of bytes/characters
    before the beginning of each string in all the preceding sources. They
    are zero for the first source.
    Storage allocated by the class. */
	corresp_src*		 src;

	correspondance(void);
	~correspondance(void);
	
    /** Append the given iSrc to the end of the #src array. The offsets
    are zeroed. If a destination has been set, it is emptied by this call. */
	void             AddSource(partial_text* iSrc);

    /** Sets a new destination string for outputting text. The string is
    immediately emptied. */
	void             SetDestination(partial_text* iDst);

	// special cases of correspondance
	// 1 - SPString contents -> text for SPText
    /** Identical to PrepareForFlow() except that when it encouters a soft
    hyphen (unicode 0x00AD) it will be not be added. */
	void             PrepareForText(bool preserve,bool &after_white);

	// 2 - SPString contents -> text for SPFlowtext
    /** Fully creates the destination string by running through all the
    sources and copying all the non-control (tab, linebreak, etc)
    characters to #dst.
      \param preserve    If false, continuous runs of more than one
                         whitespace are collapsed into one whitespace.
      \param after_white On input, specifies that whitespace precedes all
                         these sources and hence, if preserve is false, any
                         leading whitespace should be stripped.
                         On output, specifies whether the last character in
                         the last source was whitespace.
    */
	void             PrepareForFlow(bool preserve,bool &after_white);
	
	// 3 - text_flow_src -> text_holder
    /** never called. TODO. */
	void             PrepareForMerge(void);
	
    /** prepare to manually start specifying runs of continuous characters
    and adding them to #dst. */
	void             StartAdding(void);

    /** specify whether the character at pos in source source_no should be
    added to #dst or not. TODO: what would happen if pos was not strictly
    incremented by 1? */
	void             TreatChar(int pos,bool add_it,int source_no);

    /** specify that there is going to be a gap in the input. This method
    must also be called after the end of a source and before calling
    TreatChar() on the next source. Also at the end of the last source. */
	void             FlushAdding(int pos,int source_no);

    /** Calls partial_text::SetWhiteEnds() on dst. */
	void             EndAdding(void);
	
    /** Translates a position in a source to its corresponding position in
    \a dst.
      \param i_8_pos  the byte offset within the source to convert from
      \param i_4_pos  the character offset within the source to convert from
      \param i_txt    the partial_text of the source to use. If you
                      haven't previously called AddSource() with this
                      pointer then -1 is returned
      \param o_8_pos  the output byte position in #dst or -1 if the input
                      was out of bounds
      \param o_4_pos  the output character position in #dst or -1 if the
                      input was out of bounds
      \param is_end   if true then positions on the boundary between two runs
                      will belong to the preceding run, otherwise they will
                      belong to the following run.
    */
	void             SourceToDest(int i_8_pos,int i_4_pos,partial_text const * i_txt,int &o_8_pos,int &o_4_pos,bool is_end) const;

    /** Translates a position in #dst to its corresponding position a
    source.
      \param i_8_pos  the byte offset within #dst to convert from
      \param i_4_pos  the character offset within #dst to convert from
      \param o_8_pos  the output byte position in the source or the last
                      byte of the last source if the input was out of bounds.
      \param o_4_pos  the output character position in the source or the
                      last byte of the last source if the input was out of
                      bounds.
      \param o_txt    the partial_text of the source that was found or
                      the last source if the input was out of bounds.
      \param is_end   if true then positions on the boundary between two runs
                      will belong to the preceding run, otherwise they will
                      belong to the following run.
    */
	void             DestToSource(int i_8_pos,int i_4_pos,int &o_8_pos,int &o_4_pos,partial_text* &o_txt,bool is_end) const;

private:
    /** set to maintain state between calls to TreatChar() so it can combine
    continuous runs into one block */
	int              last_add_start;

    /** set to maintain state between calls to TreatChar() so it can combine
    continuous runs into one block */
	bool             currently_adding;
	
	typedef struct corresp {
		int            s_utf8_st,s_utf8_en;
		int            d_utf8_st,d_utf8_en;
		int            s_ucs4_st,s_ucs4_en;
		int            d_ucs4_st,d_ucs4_en;
		int            s_no;
	} corresp;
	int              nbCorr,maxCorr;
    /** Yet another std::vector reimplementation. Used to map an range of
    characters in one string to a range of characters in another, using
    both byte indices and character indices. s_* are the indices in
    #src[s_no] and d_* are the indices in #dst.
    Storage allocated by the class. */
	corresp*         corrs;

    /** Stores the merged string. Storage allocated by the caller of
    SetDestination(). */
	partial_text*    dst;

    /** Clears the #corrs array by setting #nbCorr = 0 */
	void             ResetCorrespondances(void);

    /** Appends a new entry to the #corrs array with all the fields
    initialised using the given parameters. Called only by AppendDest(). */
	void             AddCorrespondance(int s_8_st,int s_8_en,int s_4_st,int s_4_en,int d_8_st,int d_8_en,int d_4_st,int d_4_en,int s_no);

    /** Appends the substring of #src[source_no] from source_st to
    source_en to #dst and adds a correspondence noting this fact for
    later use by SourceToDest() and DestToSource(). */
	void             AppendDest(int source_st,int source_en,int source_no);
};

#endif
