/*
 *  text_style.h
 */

#ifndef my_flow_style
#define my_flow_style

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <memory>

//#include <pango/pango.h>

#include "FlowDefs.h"

class font_instance;
class flow_eater;
//struct PangoAnalysis;

/** \brief stores a font specification and the data needed to render it

This class stores both the specification of a font style (including
the fill and outline colours) and references to the objects needed to
retrieve rendering information about that font.

This is probably supposed to be an internal libnrtype class but, as
usual, convert_to_text() fiddles with it. */
class text_style {
public:
	bool             vertical_layout;
	font_instance*   theFont;
	double           theSize;
    /** used solely for font substitution (see SetFont()) where the new
    font may have a slightly different baseline to the requested one */
	double           baseline_shift;
	struct SPStyle*  with_style;
	
	text_style(void);
	text_style(text_style const * modele);
	~text_style(void);
	
    /** resets all the stored information (except #vertical_layout) based
    on the style given. The reference count of i_style is incremented while
    it is needed. */
	void             SetStyle(SPStyle* i_style);

    /** changes the internal font information. This should be used only by
    libnrtype in the case when it needs to substitute a different font from
    the one specified because a required glyph is not available. */
	void             SetFont(font_instance* iFont,double iSize,double iShift);
	
    /** compute the box_sizes for a specified string of text rendered in
    the font represented by this class.

      \param iText   the text to measure (UTF-8)
      \param iLen    length of iText in BYTES, or -1 if it is 0-terminated
      \param sizes   output. The computed size of the text. On output, the
                     nb_letter element will contain a count of what \a flow_res
                     calls 'letters', not the number of input characters.
      \param hyphen  if true then the space necessary for a hyphen is added
                     to the resultant size
      \param pan     this is actually a PangoAnalysis*, which is the Pango
                     object used to store a font specification. I think it's
                     void* to avoid having to #include pango.h
      \param kern_x  an array of manual kerns in the x direction to add to
                     the returned width. This must contain one element for
                     every character in iText (plus 1 if hyphen is true), or
                     it can be NULL.
      \param kern_y  an array of manual kerns in the y direction. The
                     ascent and descent of the resultant box is adjusted to
                     accommodate the repositioned characters. This must
                     contain one element for every character in iText (plus
                     1 if hyphen is true), or it can be NULL.
    */
	void             Measure(char* iText,int iLen,box_sizes *sizes,bool hyphen,void *pan,double const * kern_x,double const * kern_y);

    /** convert a UTF-8 string into a sequence of glyphs in the font
    specified by this class. The glyphs are sent through the \a flow_eater
    and incorporated directly into its associated flow_res.
    flow_eater::StartWord() must have been called prior to calling this
    method in order to set the directionality.

      \param iText   the text to measure (UTF-8)
      \param iLen    length of iText in BYTES, or -1 if it is 0-terminated
      \param hyphen  if true then the glyph(s) for a hyphen are sent to the
                     flow_eater once all the other characters have been
                     sent.
      \param pan     this is actually a PangoAnalysis*, which is the Pango
                     object used to store a font specification. I think it's
                     void* to avoid having to #include pango.h
      \param baby    the place to send the glyphs
      \param kern_x  an array of manual kerns in the x direction to apply to
                     the glyphs being output. This must contain one element for
                     every character in iText (plus 1 if hyphen is true), or
                     it can be NULL.
      \param kern_y  an array of manual kerns in the y direction to apply to
                     the glyphs being output. This must contain one element for
                     every character in iText (plus 1 if hyphen is true), or
                     it can be NULL.
    */
	void             Feed(char* iText,int iLen,bool hyphen,void *pan,flow_eater* baby,double const *kern_x,double const *kern_y);
};
	
/** \brief internal libnrtype class
	
A simple inheritance from std::vector which auto-deletes all its contents on destruction.
*/
class flow_styles : public std::vector<text_style*> {
public:
    ~flow_styles() {for(std::vector<text_style*>::iterator it=begin();it!=end();it++) delete *it;}
};

#endif


