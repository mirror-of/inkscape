/*
 * Copyright (c) 2000 World Wide Web Consortium,
 * (Massachusetts Institute of Technology, Institut National de
 * Recherche en Informatique et en Automatique, Keio University). All
 * Rights Reserved. This program is distributed under the W3C's Software
 * Intellectual Property License. This program is distributed the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * See W3C License http://www.w3.org/Consortium/Legal/ for more details.
 */

// File: http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113/css.idl

#ifndef __CSS_H__
#define __CSS_H__

#include "dom.h"
#include "stylesheets.h"
#include "views.h"




namespace w3c
{
namespace org
{
namespace dom
{
namespace css
{




//Make local definitions
typedef dom::DOMString DOMString;
typedef dom::Element Element;
typedef dom::DOMImplementation DOMImplementation;

//forward declarations
class CSSRule;
class CSSStyleSheet;
class CSSStyleDeclaration;
class CSSValue;
class Counter;
class Rect;
class RGBColor;




/*#########################################################################
## CSSRuleList
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSRuleList 
{


    /**
     *
     */
    virtual unsigned long getLength() =0;

    /**
     *
     */
    virtual CSSRule item(unsigned long index) =0;
};



/*#########################################################################
## CSSRule
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSRule 
{


    /**
     * RuleType
     */
    enum
        {
        UNKNOWN_RULE    = 0,
        STYLE_RULE      = 1,
        CHARSET_RULE    = 2,
        IMPORT_RULE     = 3,
        MEDIA_RULE      = 4,
        FONT_FACE_RULE  = 5,
        PAGE_RULE       = 6
        };


    /**
     *
     */
    virtual unsigned short getType() =0;

    /**
     *
     */
    virtual DOMString getCssText() =0;

    /**
     *
     */
    virtual void setCssText(DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual CSSStyleSheet *getParentStyleSheet() =0;

    /**
     *
     */
    virtual CSSRule *getParentRule() =0;
};



/*#########################################################################
## CSSStyleRule
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSStyleRule : virtual public CSSRule 
{


    /**
     *
     */
    virtual DOMString getSelectorText() =0;

    /**
     *
     */
    virtual void setSelectorText(DOMString& val)
                throw (dom::DOMException) =0;


    /**
     *
     */
    virtual CSSStyleDeclaration *getStyle() =0;
};

/*#########################################################################
## CSSMediaRule
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSMediaRule : virtual public CSSRule 
{


    /**
     *
     */
    virtual stylesheets::MediaList *getMedia() =0;

    /**
     *
     */
    virtual CSSRuleList *getCssRules() =0;

    /**
     *
     */
    virtual unsigned long insertRule(DOMString& rule, 
                                     unsigned long index)
                                     throw (dom::DOMException) =0;

    /**
     *
     */
    virtual void deleteRule(unsigned long index)
                            throw(dom::DOMException) =0;
};




/*#########################################################################
## CSSFontFaceRule
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSFontFaceRule : virtual public CSSRule 
{

    /**
     *
     */
    virtual CSSStyleDeclaration *getStyle() =0;
};




/*#########################################################################
## CSSPageRule
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSPageRule : virtual public CSSRule 
{

    /**
     *
     */
    virtual DOMString getSelectorText() =0;

    /**
     *
     */
    virtual void setSelectorText(DOMString& val)
                         throw(dom::DOMException) =0;


    /**
     *
     */
    virtual CSSStyleDeclaration *getStyle() =0;
};





/*#########################################################################
## CSSImportRule
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSImportRule : virtual public CSSRule 
{


    /**
     *
     */
    virtual DOMString getHref() =0;

    /**
     *
     */
    virtual stylesheets::MediaList *getMedia() =0;

    /**
     *
     */
    virtual CSSStyleSheet *getStyleSheet() =0;
};






/*#########################################################################
## CSSCharsetRule
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSCharsetRule : virtual public CSSRule 
{

    /**
     *
     */
    virtual DOMString getEncoding() =0;

    /**
     *
     */
    virtual void setEncoding(DOMString& val) throw (dom::DOMException) =0;

};





/*#########################################################################
## CSSUnknownRule
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSUnknownRule : virtual public CSSRule 
{

};





/*#########################################################################
## CSSStyleDeclaration
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSStyleDeclaration 
{

    /**
     *
     */
    virtual DOMString getCssText() =0;

    /**
     *
     */
    virtual void setCssText(DOMString& val)
                            throw (dom::DOMException) =0;


    /**
     *
     */
    virtual DOMString getPropertyValue(DOMString& propertyName) =0;

    /**
     *
     */
    virtual CSSValue *getPropertyCSSValue(DOMString& propertyName) =0;

    /**
     *
     */
    virtual DOMString removeProperty(DOMString& propertyName)
                                     throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getPropertyPriority(DOMString& propertyName) =0;

    /**
     *
     */
    virtual void setProperty(DOMString& propertyName, 
                             DOMString& value, 
                             DOMString& priority)
                             throw (dom::DOMException) =0;

    /**
     *
     */
    virtual unsigned long getLength() =0;

    /**
     *
     */
    virtual DOMString item(unsigned long index) =0;

    /**
     *
     */
    virtual CSSRule *getParentRule() =0;
};





/*#########################################################################
## CSSValue
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSValue 
{


    /**
     * UnitTypes
     */
    enum
        {
        CSS_INHERIT         = 0,
        CSS_PRIMITIVE_VALUE = 1,
        CSS_VALUE_LIST      = 2,
        CSS_CUSTOM          = 3
        }

    /**
     *
     */
    virtual DOMString getCssText() =0;

    /**
     *
     */
    virtual void setCssText(DOMString& val)
                            throw (dom::DOMException) =0;

    /**
     *
     */
    virtual unsigned short getCssValueType() =0;
};





/*#########################################################################
## CSSPrimitiveValue
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSPrimitiveValue : virtual public CSSValue 
{


    /**
     * UnitTypes
     */
    enum
        {
        CSS_UNKNOWN    = 0,
        CSS_NUMBER     = 1,
        CSS_PERCENTAGE = 2,
        CSS_EMS        = 3,
        CSS_EXS        = 4,
        CSS_PX         = 5,
        CSS_CM         = 6,
        CSS_MM         = 7,
        CSS_IN         = 8,
        CSS_PT         = 9,
        CSS_PC         = 10,
        CSS_DEG        = 11,
        CSS_RAD        = 12,
        CSS_GRAD       = 13,
        CSS_MS         = 14,
        CSS_S          = 15,
        CSS_HZ         = 16,
        CSS_KHZ        = 17,
        CSS_DIMENSION  = 18,
        CSS_STRING     = 19,
        CSS_URI        = 20,
        CSS_IDENT      = 21,
        CSS_ATTR       = 22,
        CSS_COUNTER    = 23,
        CSS_RECT       = 24,
        CSS_RGBCOLOR   = 25
        };


    /**
     *
     */
    virtual unsigned short getPrimitiveType() =0;

    /**
     *
     */
    virtual void setFloatValue(unsigned short unitType, 
                               float floatValue)
                               throw (dom::DOMException) =0;

    /**
     *
     */
    virtual float getFloatValue(unsigned short unitType)
                                throw (dom::DOMException) =0;

    /**
     *
     */
    virtual void setStringValue(unsigned short stringType, 
                                DOMString& stringValue)
                                throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getStringValue() throw (dom::DOMException) =0;

    /**
     *
     */
    virtual Counter *getCounterValue() throw (dom::DOMException) =0;

    /**
     *
     */
    virtual Rect *getRectValue() throw (dom::DOMException) =0;

    /**
     *
     */
    virtual RGBColor *getRGBColorValue() throw (dom::DOMException) =0;

};




/*#########################################################################
## CSSValueList
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSValueList : virtual public CSSValue 
{

    /**
     *
     */
    virtual unsigned long getLength() =0;

    /**
     *
     */
    virtual CSSValue *item(unsigned long index) =0;

};




/*#########################################################################
## RGBColor
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class RGBColor 
{


    /**
     *
     */
    virtual CSSPrimitiveValue *getRed() =0;

    /**
     *
     */
    virtual CSSPrimitiveValue *getGreen() =0;

    /**
     *
     */
    virtual CSSPrimitiveValue *getBlue() =0;

};




/*#########################################################################
## Rect
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class Rect 
{


    /**
     *
     */
    virtual CSSPrimitiveValue *getTop() =0;

    /**
     *
     */
    virtual CSSPrimitiveValue *getRight() =0;

    /**
     *
     */
    virtual CSSPrimitiveValue *getBottom() =0;

    /**
     *
     */
    virtual CSSPrimitiveValue *getLeft() =0;

};






/*#########################################################################
## Counter
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class Counter 
{


    /**
     *
     */
    virtual DOMString getIdentifier() =0;

    /**
     *
     */
    virtual DOMString getListStyle() =0;

    /**
     *
     */
    virtual DOMString getSeparator() =0;

};





/*#########################################################################
## ElementCSSInlineStyle
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class ElementCSSInlineStyle 
{


    /**
     *
     */
    virtual CSSStyleDeclaration getStyle() =0;
};






/*#########################################################################
## CSS2Properties
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSS2Properties 
{

    /**
     *
     */
    virtual DOMString getAzimuth() =0;

    /**
     *
     */
    virtual void setAzimuth(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBackground() throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual void setBackground(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBackgroundAttachment() =0;

    /**
     *
     */
    virtual void setBackgroundAttachment(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBackgroundColor() =0;

    /**
     *
     */
    virtual void setBackgroundColor(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBackgroundImage() =0;

    /**
     *
     */
    virtual void setBackgroundImage(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBackgroundPosition() =0;

    /**
     *
     */
    virtual void setBackgroundPosition(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBackgroundRepeat() =0;

    /**
     *
     */
    virtual void setBackgroundRepeat(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorder() =0;

    /**
     *
     */
    virtual void setBorder(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderCollapse() =0;

    /**
     *
     */
    virtual void setBorderCollapse(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderColor() =0;

    /**
     *
     */
    virtual void setBorderColor(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderSpacing() =0;

    /**
     *
     */
    virtual void setBorderSpacing(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderStyle() =0;

    /**
     *
     */
    virtual void setBorderStyle(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderTop() =0;

    /**
     *
     */
    virtual void setBorderTop(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderRight() =0;

    /**
     *
     */
    virtual void setBorderRight(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderBottom() =0;

    /**
     *
     */
    virtual void setBorderBottom(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderLeft() =0;

    /**
     *
     */
    virtual void setBorderLeft(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderTopColor() =0;

    /**
     *
     */
    virtual void setBorderTopColor(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderRightColor() =0;

    /**
     *
     */
    virtual void setBorderRightColor(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderBottomColor() =0;

    /**
     *
     */
    virtual void setBorderBottomColor(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderLeftColor() =0;

    /**
     *
     */
    virtual void setBorderLeftColor(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderTopStyle() =0;

    /**
     *
     */
    virtual void setBorderTopStyle(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderRightStyle() =0;

    /**
     *
     */
    virtual void setBorderRightStyle(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderBottomStyle() =0;

    /**
     *
     */
    virtual void setBorderBottomStyle(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderLeftStyle() =0;

    /**
     *
     */
    virtual void setBorderLeftStyle(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderTopWidth() =0;

    /**
     *
     */
    virtual void setBorderTopWidth(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderRightWidth() =0;

    /**
     *
     */
    virtual void setBorderRightWidth(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderBottomWidth() =0;

    /**
     *
     */
    virtual void setBorderBottomWidth(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderLeftWidth() =0;

    /**
     *
     */
    virtual void setBorderLeftWidth(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBorderWidth() =0;

    /**
     *
     */
    virtual void setBorderWidth(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getBottom() =0;

    /**
     *
     */
    virtual void setBottom(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getCaptionSide() =0;

    /**
     *
     */
    virtual void setCaptionSide(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getClear() =0;

    /**
     *
     */
    virtual void setClear(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getClip() =0;

    /**
     *
     */
    virtual void setClip(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getColor() =0;

    /**
     *
     */
    virtual void setColor(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getContent() =0;

    /**
     *
     */
    virtual void setContent(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getCounterIncrement() =0;

    /**
     *
     */
    virtual void setCounterIncrement(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getCounterReset() =0;

    /**
     *
     */
    virtual void setCounterReset(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getCssFloat() =0;

    /**
     *
     */
    virtual void setCssFloat(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getCue() =0;

    /**
     *
     */
    virtual void setCue(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getCueAfter() =0;

    /**
     *
     */
    virtual void setCueAfter(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getCueBefore() =0;

    /**
     *
     */
    virtual void setCueBefore(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getCursor() =0;

    /**
     *
     */
    virtual void setCursor(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getDirection() =0;

    /**
     *
     */
    virtual void setDirection(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getDisplay() =0;

    /**
     *
     */
    virtual void setDisplay(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getDlevation() =0;

    /**
     *
     */
    virtual void setDlevation(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getEmptyCells() =0;

    /**
     *
     */
    virtual void setEmptyCells(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getFont() =0;

    /**
     *
     */
    virtual void setFont(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getFontFamily() =0;

    /**
     *
     */
    virtual void setFontFamily(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getFontSize() =0;

    /**
     *
     */
    virtual void setFontSize(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getFontSizeAdjust() =0;

    /**
     *
     */
    virtual void setFontSizeAdjust(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getFontStretch() =0;

    /**
     *
     */
    virtual void setFontStretch(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getFontStyle() =0;

    /**
     *
     */
    virtual void setFontStyle(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getFontVariant() =0;

    /**
     *
     */
    virtual void setFontVariant(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getFontWeight() =0;

    /**
     *
     */
    virtual void setFontWeight(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getHeight() =0;

    /**
     *
     */
    virtual void setHeight(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getLeft() =0;

    /**
     *
     */
    virtual void setLeft(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getLetterSpacing() =0;

    /**
     *
     */
    virtual void setLetterSpacing(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getLineHeight() =0;

    /**
     *
     */
    virtual void setLineHeight(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getListStyle() =0;

    /**
     *
     */
    virtual void setListStyle(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getListStyleImage() =0;

    /**
     *
     */
    virtual void setListStyleImage(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getListStylePosition() =0;

    /**
     *
     */
    virtual void setListStylePosition(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getListStyleType() =0;

    /**
     *
     */
    virtual void setListStyleType(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMargin() =0;

    /**
     *
     */
    virtual void setMargin(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMarginTop() =0;

    /**
     *
     */
    virtual void setMarginTop(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMarginRight() =0;

    /**
     *
     */
    virtual void setMarginRight(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMarginBottom() =0;

    /**
     *
     */
    virtual void setMarginBottom(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMarginLeft() =0;

    /**
     *
     */
    virtual void setMarginLeft(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMarkerOffset() =0;

    /**
     *
     */
    virtual void setMarkerOffset(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMarks() =0;

    /**
     *
     */
    virtual void setMarks(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMaxHeight() =0;

    /**
     *
     */
    virtual void setMaxHeight(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMaxWidth() =0;

    /**
     *
     */
    virtual void setMaxWidth(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMinHeight() =0;

    /**
     *
     */
    virtual void setMinHeight(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getMinWidth() =0;

    /**
     *
     */
    virtual void setMinWidth(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getOrphans() =0;

    /**
     *
     */
    virtual void setOrphans(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getOutline() =0;

    /**
     *
     */
    virtual void setOutline(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getOutlineColor() =0;

    /**
     *
     */
    virtual void setOutlineColor(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getOutlineStyle() =0;

    /**
     *
     */
    virtual void setOutlineStyle(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getOutlineWidth() =0;

    /**
     *
     */
    virtual void setOutlineWidth(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getOverflow() =0;

    /**
     *
     */
    virtual void setOverflow(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPadding() =0;

    /**
     *
     */
    virtual void setPadding(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPaddingTop() =0;

    /**
     *
     */
    virtual void setPaddingTop(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPaddingRight() =0;

    /**
     *
     */
    virtual void setPaddingRight(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPaddingBottom() =0;

    /**
     *
     */
    virtual void setPaddingBottom(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPaddingLeft() =0;

    /**
     *
     */
    virtual void setPaddingLeft(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPage() =0;

    /**
     *
     */
    virtual void setPage(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPageBreakAfter() =0;

    /**
     *
     */
    virtual void setPageBreakAfter(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPageBreakBefore() =0;

    /**
     *
     */
    virtual void setPageBreakBefore(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPageBreakInside() =0;

    /**
     *
     */
    virtual void setPageBreakInside(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPause() =0;

    /**
     *
     */
    virtual void setPause(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPauseAfter() =0;

    /**
     *
     */
    virtual void setPauseAfter(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPauseBefore() =0;

    /**
     *
     */
    virtual void setPauseBefore(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPitch() =0;

    /**
     *
     */
    virtual void setPitch(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPitchRange() =0;

    /**
     *
     */
    virtual void setPitchRange(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPlayDuring() =0;

    /**
     *
     */
    virtual void setPlayDuring(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getPosition() =0;

    /**
     *
     */
    virtual void setPosition(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getQuotes() =0;

    /**
     *
     */
    virtual void setQuotes(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getRichness() =0;

    /**
     *
     */
    virtual void setRichness(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getRight() =0;

    /**
     *
     */
    virtual void setRight(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getSize() =0;

    /**
     *
     */
    virtual void setSize(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getSpeak() =0;

    /**
     *
     */
    virtual void setSpeak(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getSpeakHeader() =0;

    /**
     *
     */
    virtual void setSpeakHeader(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getSpeakNumeral() =0;

    /**
     *
     */
    virtual void setSpeakNumeral(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getSpeakPunctuation() =0;

    /**
     *
     */
    virtual void setSpeakPunctuation(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getSpeechRate() =0;

    /**
     *
     */
    virtual void setSpeechRate(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getStress() =0;

    /**
     *
     */
    virtual void setStress(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getTableLayout() =0;

    /**
     *
     */
    virtual void setTableLayout(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getTextAlign() =0;

    /**
     *
     */
    virtual void setTextAlign(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getTextDecoration() =0;

    /**
     *
     */
    virtual void setTextDecoration(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getTextIndent() =0;

    /**
     *
     */
    virtual void setTextIndent(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getTextShadow() =0;

    /**
     *
     */
    virtual void setTextShadow(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getTextTransform() =0;

    /**
     *
     */
    virtual void setTextTransform(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getTop() =0;

    /**
     *
     */
    virtual void setTop(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getUnicodeBidi() =0;

    /**
     *
     */
    virtual void setUnicodeBidi(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getVerticalAlign() =0;

    /**
     *
     */
    virtual void setVerticalAlign(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getVisibility() =0;

    /**
     *
     */
    virtual void setVisibility(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getVoiceFamily() =0;

    /**
     *
     */
    virtual void setVoiceFamily(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getVolume() =0;

    /**
     *
     */
    virtual void setVolume(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getWhiteSpace() =0;

    /**
     *
     */
    virtual void setWhiteSpace(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getWidows() =0;

    /**
     *
     */
    virtual void setWidows(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getWidth() =0;

    /**
     *
     */
    virtual void setWidth(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getWordSpacing() =0;

    /**
     *
     */
    virtual void setWordSpacing(DOMString& val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual DOMString getZIndex() =0;

    /**
     *
     */
    virtual void setZIndex(DOMString& val) throw (dom::DOMException) = 0;

};








/*#########################################################################
## CSSStyleSheet
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class CSSStyleSheet : virtual public stylesheets::StyleSheet 
{

    /**
     *
     */
    virtual CSSRule *getOwnerRule() =0;

    /**
     *
     */
    virtual CSSRuleList *getCssRules() =0;
    
    /**
     *
     */
    virtual unsigned long insertRule(DOMString& rule, 
                                     unsigned long index)
                                     throw (dom::DOMException) =0;
    
    /**
     *
     */
    virtual void deleteRule(unsigned long index)
                            throw (dom::DOMException) =0;
};





/*#########################################################################
## ViewCSS
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class ViewCSS : virtual public views::AbstractView 
{

    /**
     *
     */
    virtual CSSStyleDeclaration *getComputedStyle(Element *elt, 
                                                  DOMString& pseudoElt);
};





/*#########################################################################
## DocumentCSS
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class DocumentCSS : virtual public stylesheets::DocumentStyle 
{

    /**
     *
     */
    virtual CSSStyleDeclaration *getOverrideStyle(Element *elt, 
                                                  DOMString& pseudoElt);
};






/*#########################################################################
## DOMImplementationCSS
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class DOMImplementationCSS : virtual public DOMImplementation
{


    /**
     *
     */
    virtual CSSStyleSheet *createCSSStyleSheet(DOMString& title, 
                                               DOMString& media)
                                               throw (dom::DOMException) =0;
};








};//namespace css
};//namespace dom
};//namespace org
};//namespace w3c


#endif // __CSS_H__

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/
