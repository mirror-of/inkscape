// File: svg.idl
#ifndef __SVG_H__
#define __SVG_H__


// For access to DOM2 core
#include "dom.h"

// For access to DOM2 events
#include "events.h"

// For access to those parts from DOM2 CSS OM used by SVG DOM.
#include "css.h"

// For access to those parts from DOM2 Views OM used by SVG DOM.
#include "views.h"

// For access to the SMIL OM used by SVG DOM.
#include "smil.h"








namespace w3c
{
namespace org
{
namespace dom
{
namespace svg
{




//local definitions
typedef dom::DOMString DOMString;
typedef dom::DOMException DOMException;
typedef dom::Element Element;
typedef dom::Document Document;
typedef dom::NodeList NodeList;



// Predeclarations
class SVGElement;
class SVGLangSpace;
class SVGExternalResourcesRequired;
class SVGTests;
class SVGFitToViewBox;
class SVGZoomAndPan;
class SVGViewSpec;
class SVGURIReference;
class SVGPoint;
class SVGMatrix;
class SVGPreserveAspectRatio;
class SVGAnimatedPreserveAspectRatio;
class SVGTransformList;
class SVGAnimatedTransformList;
class SVGTransform;
class SVGICCColor;
class SVGColor;
class SVGPaint;
class SVGTransformable;
class SVGDocument;
class SVGSVGElement;
class SVGElementInstance;
class SVGElementInstanceList;




/*#########################################################################
## SVGElement
#########################################################################*/

/**
 *
 */
class SVGException : public DOMException
{
    // unsigned short   code;  //inherited
};

    /**
     * SVGExceptionCode
     */
    enum
        {
        SVG_WRONG_TYPE_ERR           = 0,
        SVG_INVALID_VALUE_ERR        = 1,
        SVG_MATRIX_NOT_INVERTABLE    = 2
        };




/*#########################################################################
## SVGElement
#########################################################################*/

/**
 *
 */
class SVGElement : virtual public Element
{

    /**
     *
     */
    virtual DOMString getId() =0;

    /**
     *
     */
    virtual void setId(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getXmlBase() =0;

    /**
     *
     */
    virtual void setXmlBase(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual SVGSVGElement *getOwnerSVGElement() = 0;

    /**
     *
     */
    virtual SVGElement *getViewportElement() =0;

};//



/*#########################################################################
## SVGAnimatedBoolean
#########################################################################*/

/**
 *
 */
class SVGAnimatedBoolean 
{

    /**
     *
     */
    virtual bool getBaseVal() =0;

    /**
     *
     */
    virtual void setBaseval(bool val) throw (DOMException) =0;

    /**
     *
     */
    virtual bool getAnimVal() =0;

};//




/*#########################################################################
## SVGAnimatedString
#########################################################################*/

/**
 *
 */
class SVGAnimatedString 
{

    /**
     *
     */
    virtual DOMString getBaseVal() =0;

    /**
     *
     */
    virtual void setBaseVal(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getAnimVal() =0;


};//




/*#########################################################################
## SVGStringList
#########################################################################*/

/**
 *
 */
class SVGStringList 
{




    /**
     *
     */
    virtual unsigned long getNumberOfItems() =0;


    /**
     *
     */
    virtual void   clear (  )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual DOMString initialize ( DOMString& newItem )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual DOMString getItem ( unsigned long index )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual DOMString insertItemBefore ( DOMString& newItem, unsigned long index )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual DOMString replaceItem ( DOMString& newItem, unsigned long index )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual DOMString removeItem ( unsigned long index )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual DOMString appendItem ( DOMString& newItem )
                    throw( DOMException, SVGException ) =0;



};//




/*#########################################################################
## SVGAnimatedEnumeration
#########################################################################*/

/**
 *
 */
class SVGAnimatedEnumeration 
{




    /**
     *
     */
    virtual unsigned short getBaseVal() =0;

    /**
     *
     */
    virtual void setBaseVal(unsigned short val) throw (DOMException) =0;

    /**
     *
     */
    virtual unsigned short getAnimVal() =0;



};//




/*#########################################################################
## SVGAnimatedInteger
#########################################################################*/

/**
 *
 */
class SVGAnimatedInteger 
{


    /**
     *
     */
    virtual long getBaseVal() =0;

    /**
     *
     */
    virtual void setBaseVal(long val) throw (DOMException) =0;

    /**
     *
     */
    virtual long getAnimVal() =0;



};//




/*#########################################################################
## SVGNumber
#########################################################################*/

/**
 *
 */
class SVGNumber 
{


    /**
     *
     */
    virtual float getValue() =0;

    /**
     *
     */
    virtual void setValue(float val) throw (DOMException) =0;


{


};//




/*#########################################################################
## SVGAnimatedNumber
#########################################################################*/

/**
 *
 */
class SVGAnimatedNumber 
{



    /**
     *
     */
    virtual float getBaseVal() =0;

    /**
     *
     */
    virtual void setBaseVal(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getAnimVal() =0;



};//




/*#########################################################################
## SVGNumberList
#########################################################################*/

/**
 *
 */
class SVGNumberList 
{




    /**
     *
     */
    virtual unsigned long getNumberOfItems() =0;


    /**
     *
     */
    virtual void clear (  ) throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGNumber *initialize ( SVGNumber *newItem )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGNumber *getItem ( unsigned long index )
                                  throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGNumber *insertItemBefore ( SVGNumber *newItem,
                                         unsigned long index )
                                         throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGNumber *replaceItem ( SVGNumber *newItem, 
                                    unsigned long index )
                                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGNumber *removeItem ( unsigned long index )
                                  throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGNumber *appendItem ( SVGNumber *newItem )
                                   throw( DOMException, SVGException ) =0;


};//




/*#########################################################################
## SVGAnimatedNumberList
#########################################################################*/

/**
 *
 */
class SVGAnimatedNumberList 
{


    /**
     *
     */
    virtual SVGNumberList *getBaseVal() =0;

    /**
     *
     */
    virtual SVGNumberList *getAnimVal() =0;



};//





/*#########################################################################
## SVGLength
#########################################################################*/

/**
 *
 */
class SVGLength 
{



    /**
     * Length Unit Types
     */
    enum
        {
        SVG_LENGTHTYPE_UNKNOWN    = 0,
        SVG_LENGTHTYPE_NUMBER     = 1,
        SVG_LENGTHTYPE_PERCENTAGE = 2,
        SVG_LENGTHTYPE_EMS        = 3,
        SVG_LENGTHTYPE_EXS        = 4,
        SVG_LENGTHTYPE_PX         = 5,
        SVG_LENGTHTYPE_CM         = 6,
        SVG_LENGTHTYPE_MM         = 7,
        SVG_LENGTHTYPE_IN         = 8,
        SVG_LENGTHTYPE_PT         = 9,
        SVG_LENGTHTYPE_PC         = 10
        };

    /**
     *
     */
    virtual unsigned short getUnitType( ) =0;

    /**
     *
     */
    virtual float getValue( ) =0;

    /**
     *
     */
    virtual void setValue( float value )  throw (DOMException) =0;

    /**
     *
     */
    virtual float getValueInSpecifiedUnits( ) =0;
 
    /**
     *
     */
    virtual void setValueInSpecifiedUnits( float val )
                                           throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getValueAsString( ) =0;

    /**
     *
     */
    virtual void setValueAsString( DOMString& val )
                                   throw (DOMException) =0;


    /**
     *
     */
    virtual void newValueSpecifiedUnits ( unsigned short unitType, float val ) =0;

    /**
     *
     */
    virtual void convertToSpecifiedUnits ( unsigned short unitType ) =0;



};//





/*#########################################################################
## SVGAnimatedLength
#########################################################################*/

/**
 *
 */
class SVGAnimatedLength 
{

    /**
     *
     */
    virtual SVGLength *getBaseVal() =0;

    /**
     *
     */
    virtual SVGLength *getAnimVal() =0;



};//





/*#########################################################################
## SVGLengthList
#########################################################################*/

/**
 *
 */
class SVGLengthList 
{




    /**
     *
     */
    virtual unsigned long getNumberOfItems() =0;


    /**
     *
     */
    virtual void   clear (  )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGLength *initialize (SVGLength *newItem )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGLength *getItem (unsigned long index )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGLength *insertItemBefore (SVGLength *newItem, unsigned long index )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGLength *replaceItem (SVGLength *newItem, unsigned long index )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGLength *removeItem (unsigned long index )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGLength *appendItem (SVGLength *newItem )
                    throw( DOMException, SVGException ) =0;



};//





/*#########################################################################
## SVGAnimatedLengthList
#########################################################################*/

/**
 *
 */
class SVGAnimatedLengthList 
{

    /**
     *
     */
    virtual SVGLengthList *getBaseVal() =0;

    /**
     *
     */
    virtual SVGLengthList *getAnimVal() =0;



};//





/*#########################################################################
## SVGAngle
#########################################################################*/

/**
 *
 */
class SVGAngle 
{



    /**
     *  Angle Unit Types
     */
    enum
        {
        SVG_ANGLETYPE_UNKNOWN     = 0,
        SVG_ANGLETYPE_UNSPECIFIED = 1,
        SVG_ANGLETYPE_DEG         = 2,
        SVG_ANGLETYPE_RAD         = 3,
        SVG_ANGLETYPE_GRAD        = 4
        };



    /**
     *
     */
    virtual unsigned short getUnitType() =0;

    /**
     *
     */
    virtual float getValue() =0;

    /**
     *
     */
    virtual void setValue(float val) throw (DOMException) = 0;

    /**
     *
     */
    virtual float getValueInSpecifiedUnits() =0;

    /**
     *
     */
    virtual void setValueInSpecifiedUnits(float val) throw (DOMException) = 0;

    /**
     *
     */
    virtual DOMString getValueAsString() =0;

    /**
     *
     */
    virtual void setValueAsString(DOMString& val) throw (DOMException) = 0;


    /**
     *
     */
    virtual void newValueSpecifiedUnits (unsigned short unitType,
                                 float valueInSpecifiedUnits ) =0;

    /**
     *
     */
    virtual void convertToSpecifiedUnits (unsigned short unitType ) =0;



};//





/*#########################################################################
## SVGAnimatedAngle
#########################################################################*/

/**
 *
 */
class SVGAnimatedAngle 
{



    /**
     *
     */
    virtual SVGAngle *getBaseVal() =0;

    /**
     *
     */
    virtual SVGAngle *getAnimVal() =0;




};//





/*#########################################################################
## SVGColor
#########################################################################*/

/**
 *
 */
class SVGColor : virtual public css::CSSValue 
{


    /**
     * Color Types
     */
    enum
        {
        SVG_COLORTYPE_UNKNOWN           = 0,
        SVG_COLORTYPE_RGBCOLOR          = 1,
        SVG_COLORTYPE_RGBCOLOR_ICCCOLOR = 2,
        SVG_COLORTYPE_CURRENTCOLOR      = 3
        };


    /**
     *
     */
    virtual unsigned short getColorType() =0;

    /**
     *
     */
    virtual css::RGBColor *getRgbColor() =0;

    /**
     *
     */
    virtual SVGICCColor *getIccColor() =0;


    /**
     *
     */
    virtual void setRGBColor (DOMString& rgbColor )
                              throw( SVGException ) =0;

    /**
     *
     */
    virtual void setRGBColorICCColor (DOMString& rgbColor,
                                      DOMString& iccColor )
                                      throw( SVGException ) =0;

    /**
     *
     */
    virtual void setColor (unsigned short colorType, 
                           DOMString& rgbColor,
                           DOMString& iccColor )
                           throw( SVGException ) =0;



};//





/*#########################################################################
## SVGICCColor
#########################################################################*/

/**
 *
 */
class SVGICCColor 
{

    /**
     *
     */
    virtual DOMString getColorProfile() =0;

    /**
     *
     */
    virtual void setColorProfile() throw (DOMException) =0;

    /**
     *
     */
    virtual SVGNumberList *getColors() =0;



};//





/*#########################################################################
## SVGRect
#########################################################################*/

/**
 *
 */
class SVGRect 
{

    /**
     *
     */
    virtual float getX() =0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() =0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getWidth() =0;

    /**
     *
     */
    virtual void setWidth(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getHeight() =0;

    /**
     *
     */
    virtual void setHeight(float val) throw (DOMException) =0;


};//





/*#########################################################################
## SVGAnimatedRect
#########################################################################*/

/**
 *
 */
class SVGAnimatedRect 
{

    /**
     *
     */
    virtual SVGRect *getBaseVal() =0;

    /**
     *
     */
    virtual SVGRect *getAnimVal() =0;



};//





/*#########################################################################
## SVGUnitTypes
#########################################################################*/

/**
 *
 */
class SVGUnitTypes 
{

    /**
     * Unit Types
     */
    enum
        {
        SVG_UNIT_TYPE_UNKNOWN           = 0,
        SVG_UNIT_TYPE_USERSPACEONUSE    = 1,
        SVG_UNIT_TYPE_OBJECTBOUNDINGBOX = 2
        };



};//





/*#########################################################################
## SVGStylable
#########################################################################*/

/**
 *
 */
class SVGStylable 
{

    /**
     *
     */
    virtual SVGAnimatedString *getClassName() =0;

    /**
     *
     */
    virtual css::CSSStyleDeclaration *getStyle() =0;


    /**
     *
     */
    css::CSSValue *getPresentationAttribute (DOMString& name ) =0;


};//





/*#########################################################################
## SVGLocatable
#########################################################################*/

/**
 *
 */
class SVGLocatable 
{

    /**
     *
     */
    virtual SVGElement *getNearestViewportElement() =0;

    /**
     *
     */
    virtual SVGElement *getFarthestViewportElement() =0;

    /**
     *
     */
    virtual SVGRect   *getBBox (  ) =0;

    /**
     *
     */
    virtual SVGMatrix *getCTM (  ) =0;

    /**
     *
     */
    virtual SVGMatrix *getScreenCTM (  ) =0;

    /**
     *
     */
    virtual SVGMatrix *getTransformToElement (SVGElement element )
                    throw( SVGException ) =0;



};//





/*#########################################################################
## SVGTransformable
#########################################################################*/

/**
 *
 */
class SVGTransformable : virtual public SVGLocatable 
{


    /**
     *
     */
    virtual SVGAnimatedTransformList *getTransform() =0;



};//





/*#########################################################################
## SVGTests
#########################################################################*/

/**
 *
 */
class SVGTests 
{



    /**
     *
     */
    virtual SVGStringList *getRequiredFeatures() =0;

    /**
     *
     */
    virtual SVGStringList *getRequiredExtensions() =0;

    /**
     *
     */
    virtual SVGStringList *getSystemLanguage() =0;

    
    /**
     *
     */
    virtual bool hasExtension (DOMString& extension ) =0;



};//





/*#########################################################################
## SVGLangSpace
#########################################################################*/

/**
 *
 */
class SVGLangSpace 
{



    /**
     *
     */
    virtual DOMString getXmllang() =0;

    /**
     *
     */
    virtual void setXmllang(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getXmlspace() =0;

    /**
     *
     */
    virtual void setXmlspace(DOMString& val) throw (DOMException) =0;



};//





/*#########################################################################
## SVGExternalResourcesRequired
#########################################################################*/

/**
 *
 */
class SVGExternalResourcesRequired 
{


    /**
     *
     */
    virtual SVGAnimatedBoolean *getExternalResourcesRequired() =0;



};//





/*#########################################################################
## SVGFitToViewBox
#########################################################################*/

/**
 *
 */
class SVGFitToViewBox 
{

    /**
     *
     */
    virtual SVGAnimatedRect getViewBox() =0;

    /**
     *
     */
    virtual SVGAnimatedPreserveAspectRatio getPreserveAspectRatio() =0;



};//





/*#########################################################################
## SVGZoomAndPan
#########################################################################*/

/**
 *
 */
class SVGZoomAndPan 
{


    /**
     * Zoom and Pan Types
     */
    enum
        {
        SVG_ZOOMANDPAN_UNKNOWN = 0,
        SVG_ZOOMANDPAN_DISABLE = 1,
        SVG_ZOOMANDPAN_MAGNIFY = 2
        };


    /**
     *
     */
    virtual unsigned short getZoomAndPan() =0;

    /**
     *
     */
    virtual void setZoomAndPan(unsigned short val) throw (DOMException) =0;


};//





/*#########################################################################
## SVGViewSpec
#########################################################################*/

/**
 *
 */
class SVGViewSpec : 
                virtual public SVGZoomAndPan,
                virtual public SVGFitToViewBox 
{




    /**
     *
     */
    virtual SVGTransformList *getTransform() =0;

    /**
     *
     */
    virtual SVGElement *getViewTarget() =0;

    /**
     *
     */
    virtual DOMString getViewBoxString() =0;

    /**
     *
     */
    virtual DOMString getPreserveAspectRatioString() =0;

    /**
     *
     */
    virtual DOMString getTransformString() =0;

    /**
     *
     */
    virtual DOMString getViewTargetString() =0;



};//





/*#########################################################################
## SVGURIReference
#########################################################################*/

/**
 *
 */
class SVGURIReference 
{

    /**
     *
     */
    virtual SVGAnimatedString *getHref() =0;



};//





/*#########################################################################
## SVGCSSRule
#########################################################################*/

/**
 *
 */
class SVGCSSRule : virtual public css::CSSRule 
{


    /**
     * Additional CSS RuleType to support ICC color specifications
     */
    enum
        {
        COLOR_PROFILE_RULE = 7
        };

};//





/*#########################################################################
## SVGRenderingIntent
#########################################################################*/

/**
 *
 */
class SVGRenderingIntent 
{

    /**
     * Rendering Intent Types
     */
    enum
        {
        RENDERING_INTENT_UNKNOWN               = 0,
        RENDERING_INTENT_AUTO                  = 1,
        RENDERING_INTENT_PERCEPTUAL            = 2,
        RENDERING_INTENT_RELATIVE_COLORIMETRIC = 3,
        RENDERING_INTENT_SATURATION            = 4,
        RENDERING_INTENT_ABSOLUTE_COLORIMETRIC = 5
        };



};//





/*#########################################################################
## SVGDocument
#########################################################################*/

/**
 *
 */
class SVGDocument : virtual public Document,
                    virtual public events::DocumentEvent 
{




    /**
     *
     */
    virtual DOMString getTitle() =0;

    /**
     *
     */
    virtual DOMString getReferrer() =0;

    /**
     *
     */
    virtual DOMString getDomain() =0;

    /**
     *
     */
    virtual DOMString getURL() =0;

    /**
     *
     */
    virtual SVGSVGElement *getRootElement() =0;


};//





/*#########################################################################
## SVGSVGElement
#########################################################################*/

/**
 *
 */
class SVGSVGElement : virtual public SVGElement,
                      virtual public SVGTests,
                      virtual public SVGLangSpace,
                      virtual public SVGExternalResourcesRequired,
                      virtual public SVGStylable,
                      virtual public SVGLocatable,
                      virtual public SVGFitToViewBox,
                      virtual public SVGZoomAndPan,
                      virtual public events::EventTarget,
                      virtual public events::DocumentEvent,
                      virtual public css::ViewCSS,
                      virtual public css::DocumentCSS 
{



    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getHeight() =0;

    /**
     *
     */
    virtual DOMString getContentScriptType() =0;

    /**
     *
     */
    virtual void setContentScriptType(DOMString& val) throw (DOMException) =0;


    /**
     *
     */
    virtual DOMString getContentStyleType() =0;

    /**
     *
     */
    virtual void setContentStyleType(DOMString& val) throw (DOMException) =0;


    /**
     *
     */
    virtual SVGRect *getViewport() =0;

    /**
     *
     */
    virtual float getPixelUnitToMillimeterX() =0;

    /**
     *
     */
    virtual float getPixelUnitToMillimeterY() =0;

    /**
     *
     */
    virtual float getScreenPixelToMillimeterX() =0;

    /**
     *
     */
    virtual float getScreenPixelToMillimeterY() =0;


    /**
     *
     */
    virtual bool getUseCurrentView() =0;

    /**
     *
     */
    virtual void setUseCurrentView(bool val) throw (DOMException) =0;

    /**
     *
     */
    virtual SVGViewSpec *getCurrentView() =0;


    /**
     *
     */
    virtual float getCurrentScale() =0;

    /**
     *
     */
    virtual void setCurrentScale(float val) throw (DOMException) =0;


    /**
     *
     */
    virtual SVGPoint *getCurrentTranslate() =0;

    
    /**
     *
     */
    virtual unsigned long suspendRedraw (unsigned long max_wait_milliseconds ) =0;
    
    /**
     *
     */
    virtual void unsuspendRedraw (unsigned long suspend_handle_id )
                                  throw( DOMException ) =0;
    
    /**
     *
     */
    virtual void unsuspendRedrawAll (  ) =0;
    
    /**
     *
     */
    virtual void forceRedraw (  ) =0;
    
    /**
     *
     */
    virtual void pauseAnimations (  ) =0;
    
    /**
     *
     */
    virtual void unpauseAnimations (  ) =0;
    
    /**
     *
     */
    virtual boolean animationsPaused (  ) =0;
    
    /**
     *
     */
    virtual float getCurrentTime (  ) =0;
    
    /**
     *
     */
    virtual void setCurrentTime (float seconds ) =0;
    
    /**
     *
     */
    virtual NodeList *getIntersectionList (SVGRect *rect, 
                                           SVGElement *referenceElement ) =0;
    
    /**
     *
     */
    virtual NodeList *getEnclosureList (SVGRect *rect,
                                        SVGElement *referenceElement ) =0;
    
    /**
     *
     */
    virtual bool checkIntersection (SVGElement *element, SVGRect *rect ) =0;
    
    /**
     *
     */
    virtual bool checkEnclosure (SVGElement *element, SVGRect *rect ) =0;
    
    /**
     *
     */
    virtual void deselectAll (  ) =0;
    
    /**
     *
     */
    virtual SVGNumber *createSVGNumber (  ) =0;
    
    /**
     *
     */
    virtual SVGLength *createSVGLength (  ) =0;
    
    /**
     *
     */
    virtual SVGAngle *createSVGAngle (  ) =0;
    
    /**
     *
     */
    virtual SVGPoint *createSVGPoint (  ) =0;
    
    /**
     *
     */
    virtual SVGMatrix *createSVGMatrix (  ) =0;
    
    /**
     *
     */
    virtual SVGRect *createSVGRect (  ) =0;
    
    /**
     *
     */
    virtual SVGTransform *createSVGTransform (  ) =0;
    
    /**
     *
     */
    virtual SVGTransform *createSVGTransformFromMatrix (SVGMatrix *matrix ) =0;
    
    /**
     *
     */
    virtual Element *getElementById (DOMString& elementId ) =0;



};//





/*#########################################################################
## SVGGElement
#########################################################################*/

/**
 *
 */
class SVGGElement :
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget
{

};



/*#########################################################################
## SVGDefsElement
#########################################################################*/

/**
 *
 */
class SVGDefsElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget
{

};



/*#########################################################################
## SVGDescElement
#########################################################################*/

/**
 *
 */
class SVGDescElement : 
                    virtual public SVGElement,
                    virtual public SVGLangSpace,
                    virtual public SVGStylable
{

};



/*#########################################################################
## SVGTitleElement
#########################################################################*/

/**
 *
 */
class SVGTitleElement : 
                    virtual public SVGElement,
                    virtual public SVGLangSpace,
                    virtual public SVGStylable
{

};



/*#########################################################################
## SVGSymbolElement
#########################################################################*/

/**
 *
 */
class SVGSymbolElement : 
                    virtual public SVGElement,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGFitToViewBox,
                    virtual public events::EventTarget
{

};



/*#########################################################################
## SVGUseElement
#########################################################################*/

/**
 *
 */
class SVGUseElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget 
{




    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getHeight() =0;

    /**
     *
     */
    virtual SVGElementInstance *getInstanceRoot() =0;

    /**
     *
     */
    virtual SVGElementInstance *getAnimatedInstanceRoot() =0;



};//





/*#########################################################################
## SVGElementInstance
#########################################################################*/

/**
 *
 */
class SVGElementInstance : virtual public events::EventTarget 
{

    /**
     *
     */
    virtual SVGElement *getCorrespondingElement() =0;

    /**
     *
     */
    virtual SVGUseElement *getCorrespondingUseElement() =0;

    /**
     *
     */
    virtual SVGElementInstance *getParentNode() =0;

    /**
     *
     */
    virtual SVGElementInstanceList *getChildNodes() =0;

    /**
     *
     */
    virtual SVGElementInstance *getFirstChild() =0;

    /**
     *
     */
    virtual SVGElementInstance *getLastChild() =0;

    /**
     *
     */
    virtual SVGElementInstance *getPreviousSibling() =0;

    /**
     *
     */
    virtual SVGElementInstance *getNextSibling() =0;



};//





/*#########################################################################
## SVGElementInstanceList
#########################################################################*/

/**
 *
 */
class SVGElementInstanceList 
{


    /**
     *
     */
    virtual unsigned long getLength() =0;

    /**
     *
     */
    virtual SVGElementInstance *item (unsigned long index ) =0;



};//





/*#########################################################################
## SVGImageElement
#########################################################################*/

/**
 *
 */
class SVGImageElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget 
{


    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getHeight() =0;


    /**
     *
     */
    virtual SVGAnimatedPreserveAspectRatio *getPreserveAspectRatio() =0;



};//





/*#########################################################################
## SVGSwitchElement
#########################################################################*/

/**
 *
 */
class SVGSwitchElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget
{

};



/*#########################################################################
## GetSVGDocument
#########################################################################*/

/**
 *
 */
class GetSVGDocument 
{

    /**
     *
     */
    virtual SVGDocument *getSVGDocument (  )
                    throw( DOMException ) =0;

};//





/*#########################################################################
## SVGStyleElement
#########################################################################*/

/**
 *
 */
class SVGStyleElement : virtual public SVGElement 
{

    /**
     *
     */
    virtual DOMString getXmlspace() = 0;

    /**
     *
     */
    virtual void setXmlspace(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getType() = 0;

    /**
     *
     */
    virtual void setType(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getMedia() = 0;

    /**
     *
     */
    virtual void setMedia(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getTitle() = 0;

    /**
     *
     */
    virtual void setTitle(DOMString& val) throw (DOMException) =0;



};//





/*#########################################################################
## SVGPoint
#########################################################################*/

/**
 *
 */
class SVGPoint 
{



    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual SVGPoint *matrixTransform (SVGMatrix *matrix ) =0;



};//





/*#########################################################################
## SVGPointList
#########################################################################*/

/**
 *
 */
class SVGPointList 
{

    /**
     *
     */
    virtual unsigned long getNumberOfItems() =0;

    /**
     *
     */
    virtual void clear (  ) throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGPoint *initialize (SVGPoint *newItem )
                                 throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGPoint *getItem (unsigned long index )
                               throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGPoint *insertItemBefore (SVGPoint *newItem, unsigned long index )
                                        throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGPoint *replaceItem (SVGPoint *newItem, unsigned long index )
                                   throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGPoint *removeItem (unsigned long index )
                                  throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGPoint *appendItem (SVGPoint *newItem )
                                  throw( DOMException, SVGException ) =0;



};//





/*#########################################################################
## SVGMatrix
#########################################################################*/

/**
 *
 */
class SVGMatrix 
{


    /**
     *
     */
    virtual float getA() = 0;

    /**
     *
     */
    virtual void setA(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getB() = 0;

    /**
     *
     */
    virtual void setB(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getC() = 0;

    /**
     *
     */
    virtual void setC(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getD() = 0;

    /**
     *
     */
    virtual void setD(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getE() = 0;

    /**
     *
     */
    virtual void setE(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getF() = 0;

    /**
     *
     */
    virtual void setF(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual SVGMatrix *multiply (SVGMatrix *secondMatrix ) =0;

    /**
     *
     */
    virtual SVGMatrix *inverse (  )
                    throw( SVGException ) =0;

    /**
     *
     */
    virtual SVGMatrix *translate (float x, float y ) =0;

    /**
     *
     */
    virtual SVGMatrix *scale (float scaleFactor ) =0;

    /**
     *
     */
    virtual SVGMatrix *scaleNonUniform (float scaleFactorX, float scaleFactorY ) =0;

    /**
     *
     */
    virtual SVGMatrix *rotate (float angle ) =0;

    /**
     *
     */
    virtual SVGMatrix *rotateFromVector (float x, float y )
                    throw( SVGException ) =0;

    /**
     *
     */
    virtual SVGMatrix *flipX (  ) =0;

    /**
     *
     */
    virtual SVGMatrix *flipY (  ) =0;

    /**
     *
     */
    virtual SVGMatrix *skewX (float angle ) =0;

    /**
     *
     */
    virtual SVGMatrix *skewY (float angle ) =0;



};//





/*#########################################################################
## SVGTransform
#########################################################################*/

/**
 *
 */
class SVGTransform 
{



    /**
     * Transform Types
     */
    enum
        {
        SVG_TRANSFORM_UNKNOWN   = 0,
        SVG_TRANSFORM_MATRIX    = 1,
        SVG_TRANSFORM_TRANSLATE = 2,
        SVG_TRANSFORM_SCALE     = 3,
        SVG_TRANSFORM_ROTATE    = 4,
        SVG_TRANSFORM_SKEWX     = 5,
        SVG_TRANSFORM_SKEWY     = 6,
        };

    /**
     *
     */
    virtual unsigned short getType() =0;

    /**
     *
     */
    virtual SVGMatrix *matrix() =0;

    /**
     *
     */
    virtual float angle() =0;


    /**
     *
     */
    virtual void setMatrix (SVGMatrix *matrix )=0;

    /**
     *
     */
    virtual void setTranslate (float tx, float ty )=0;

    /**
     *
     */
    virtual void setScale (float sx, float sy )=0;

    /**
     *
     */
    virtual void setRotate (float angle, float cx, float cy )=0;

    /**
     *
     */
    virtual void setSkewX (float angle ) =0;

    /**
     *
     */
    virtual void setSkewY (float angle ) =0;



};//





/*#########################################################################
## SVGTransformList
#########################################################################*/

/**
 *
 */
class SVGTransformList 
{


    /**
     *
     */
    virtual unsigned long numberOfItems;


    /**
     *
     */
    virtual void   clear (  )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGTransform *initialize (SVGTransform *newItem )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGTransform *getItem (unsigned long index )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGTransform *insertItemBefore (SVGTransform *newItem, unsigned long index )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGTransform *replaceItem (SVGTransform *newItem, unsigned long index )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGTransform *removeItem (unsigned long index )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGTransform *appendItem (SVGTransform *newItem )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGTransform *createSVGTransformFromMatrix (SVGMatrix *matrix ) =0;

    /**
     *
     */
    virtual SVGTransform consolidate (  ) =0;



};//





/*#########################################################################
## SVGAnimatedTransformList
#########################################################################*/

/**
 *
 */
class SVGAnimatedTransformList 
{

    /**
     *
     */
    virtual SVGTransformList *getBaseVal() =0;

    /**
     *
     */
    virtual SVGTransformList *getAnimVal() =0;



};//





/*#########################################################################
## SVGPreserveAspectRatio
#########################################################################*/

/**
 *
 */
class SVGPreserveAspectRatio 
{



    /**
     * Alignment Types
     */
    enum
        {
        SVG_PRESERVEASPECTRATIO_UNKNOWN  = 0,
        SVG_PRESERVEASPECTRATIO_NONE     = 1,
        SVG_PRESERVEASPECTRATIO_XMINYMIN = 2,
        SVG_PRESERVEASPECTRATIO_XMIDYMIN = 3,
        SVG_PRESERVEASPECTRATIO_XMAXYMIN = 4,
        SVG_PRESERVEASPECTRATIO_XMINYMID = 5,
        SVG_PRESERVEASPECTRATIO_XMIDYMID = 6,
        SVG_PRESERVEASPECTRATIO_XMAXYMID = 7,
        SVG_PRESERVEASPECTRATIO_XMINYMAX = 8,
        SVG_PRESERVEASPECTRATIO_XMIDYMAX = 9,
        SVG_PRESERVEASPECTRATIO_XMAXYMAX = 10
        };


    /**
     * Meet-or-slice Types
     */
    enum
        {
        SVG_MEETORSLICE_UNKNOWN  = 0,
        SVG_MEETORSLICE_MEET     = 1,
        SVG_MEETORSLICE_SLICE    = 2
        };


    /**
     *
     */
    virtual unsigned short getALign() = 0;

    /**
     *
     */
    virtual void setAlign(unsigned short val) throw (DOMException) =0;

    /**
     *
     */
    virtual unsigned short getMeetOrSlice() = 0;

    /**
     *
     */
    virtual void setMeetOrSlice(unsigned short val) throw (DOMException) =0;



};//





/*#########################################################################
## SVGAnimatedPreserveAspectRatio
#########################################################################*/

/**
 *
 */
class SVGAnimatedPreserveAspectRatio 
{


    /**
     *
     */
    virtual SVGPreserveAspectRatio *getBaseVal() =0;

    /**
     *
     */
    virtual SVGPreserveAspectRatio *getAnimVal() =0;



};//





/*#########################################################################
## SVGPathSeg
#########################################################################*/

/**
 *
 */
class SVGPathSeg 
{



    /**
     *  Path Segment Types
     */
    enum
        {
        PATHSEG_UNKNOWN                      = 0,
        PATHSEG_CLOSEPATH                    = 1,
        PATHSEG_MOVETO_ABS                   = 2,
        PATHSEG_MOVETO_REL                   = 3,
        PATHSEG_LINETO_ABS                   = 4,
        PATHSEG_LINETO_REL                   = 5,
        PATHSEG_CURVETO_CUBIC_ABS            = 6,
        PATHSEG_CURVETO_CUBIC_REL            = 7,
        PATHSEG_CURVETO_QUADRATIC_ABS        = 8,
        PATHSEG_CURVETO_QUADRATIC_REL        = 9,
        PATHSEG_ARC_ABS                      = 10,
        PATHSEG_ARC_REL                      = 11,
        PATHSEG_LINETO_HORIZONTAL_ABS        = 12,
        PATHSEG_LINETO_HORIZONTAL_REL        = 13,
        PATHSEG_LINETO_VERTICAL_ABS          = 14,
        PATHSEG_LINETO_VERTICAL_REL          = 15,
        PATHSEG_CURVETO_CUBIC_SMOOTH_ABS     = 16,
        PATHSEG_CURVETO_CUBIC_SMOOTH_REL     = 17,
        PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS = 18,
        PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL = 19
        };

    /**
     *
     */
    virtual unsigned short getPathSegType() =0;

    /**
     *
     */
    virtual DOMString getPathSegTypeAsLetter() =0;



};//





/*#########################################################################
## SVGPathSegClosePath
#########################################################################*/

/**
 *
 */
class SVGPathSegClosePath : virtual public SVGPathSeg
{

};



/*#########################################################################
## SVGPathSegMovetoAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegMovetoAbs : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegMovetoRel
#########################################################################*/

/**
 *
 */
class SVGPathSegMovetoRel : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegLinetoAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoAbs : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegLinetoRel
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoRel : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegCurvetoCubicAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicAbs : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getX1() = 0;

    /**
     *
     */
    virtual void setX1(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY1() = 0;

    /**
     *
     */
    virtual void setY1(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getX2() = 0;

    /**
     *
     */
    virtual void setX2(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY2() = 0;

    /**
     *
     */
    virtual void setY2(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegCurvetoCubicRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicRel : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getX1() = 0;

    /**
     *
     */
    virtual void setX1(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY1() = 0;

    /**
     *
     */
    virtual void setY1(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getX2() = 0;

    /**
     *
     */
    virtual void setX2(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY2() = 0;

    /**
     *
     */
    virtual void setY2(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegCurvetoQuadraticAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticAbs : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getX1() = 0;

    /**
     *
     */
    virtual void setX1(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY1() = 0;

    /**
     *
     */
    virtual void setY1(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegCurvetoQuadraticRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticRel : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getX1() = 0;

    /**
     *
     */
    virtual void setX1(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY1() = 0;

    /**
     *
     */
    virtual void setY1(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegArcAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegArcAbs : virtual public SVGPathSeg 
{


    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getR1() = 0;

    /**
     *
     */
    virtual void setR1(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getR2() = 0;

    /**
     *
     */
    virtual void setR2(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getAngle() = 0;

    /**
     *
     */
    virtual void setAngle(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual bool getLargeArcFlag() = 0;

    /**
     *
     */
    virtual void setLargeArcFlag(bool val) throw (DOMException) =0;

    /**
     *
     */
    virtual bool getSweepFlag() = 0;

    /**
     *
     */
    virtual void setSweepFlag(bool val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegArcRel
#########################################################################*/

/**
 *
 */
class SVGPathSegArcRel : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getR1() = 0;

    /**
     *
     */
    virtual void setR1(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getR2() = 0;

    /**
     *
     */
    virtual void setR2(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getAngle() = 0;

    /**
     *
     */
    virtual void setAngle(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual bool getLargeArcFlag() = 0;

    /**
     *
     */
    virtual void setLargeArcFlag(bool val) throw (DOMException) =0;

    /**
     *
     */
    virtual bool getSweepFlag() = 0;

    /**
     *
     */
    virtual void setSweepFlag(bool val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegLinetoHorizontalAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoHorizontalAbs : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegLinetoHorizontalRel
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoHorizontalRel : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegLinetoVerticalAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoVerticalAbs : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegLinetoVerticalRel
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoVerticalRel : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegCurvetoCubicSmoothAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicSmoothAbs : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getX2() = 0;

    /**
     *
     */
    virtual void setX2(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY2() = 0;

    /**
     *
     */
    virtual void setY2(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegCurvetoCubicSmoothRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicSmoothRel : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getX2() = 0;

    /**
     *
     */
    virtual void setX2(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY2() = 0;

    /**
     *
     */
    virtual void setY2(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegCurvetoQuadraticSmoothAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticSmoothAbs : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegCurvetoQuadraticSmoothRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticSmoothRel : virtual public SVGPathSeg 
{
    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

};//





/*#########################################################################
## SVGPathSegList
#########################################################################*/

/**
 *
 */
class SVGPathSegList 
{




    /**
     *
     */
    virtual unsigned long getNumberOfItems() =0;


    /**
     *
     */
    virtual void   clear (  )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGPathSeg *initialize (SVGPathSeg *newItem )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGPathSeg *getItem (unsigned long index )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGPathSeg *insertItemBefore (SVGPathSeg *newItem, unsigned long index )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGPathSeg *replaceItem (SVGPathSeg *newItem, unsigned long index )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGPathSeg *removeItem (unsigned long index )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGPathSeg *appendItem (SVGPathSeg *newItem )
                    throw( DOMException, SVGException ) =0;



};//





/*#########################################################################
## SVGAnimatedPathData
#########################################################################*/

/**
 *
 */
class SVGAnimatedPathData 
{

    /**
     *
     */
    virtual SVGPathSegList *getPathSegList() =0;

    /**
     *
     */
    virtual SVGPathSegList *getNormalizedPathSegList() =0;

    /**
     *
     */
    virtual SVGPathSegList *getAnimatedPathSegList() =0;

    /**
     *
     */
    virtual SVGPathSegList *getAnimatedNormalizedPathSegList() =0;



};//





/*#########################################################################
## SVGPathElement
#########################################################################*/

/**
 *
 */
class SVGPathElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget,
                    virtual public SVGAnimatedPathData 
{




    /**
     *
     */
    virtual SVGAnimatedNumber *getPathLength() =0;

    /**
     *
     */
    virtual float getTotalLength (  ) =0;

    /**
     *
     */
    virtual SVGPoint *getPointAtLength (float distance ) =0;

    /**
     *
     */
    virtual unsigned long getPathSegAtLength (float distance ) =0;

    /**
     *
     */
    virtual SVGPathSegClosePath 
              *createSVGPathSegClosePath (  ) =0;

    /**
     *
     */
    virtual SVGPathSegMovetoAbs 
              *createSVGPathSegMovetoAbs (float x, float y ) =0;

    /**
     *
     */
    virtual SVGPathSegMovetoRel 
              *createSVGPathSegMovetoRel (float x, float y ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoAbs 
              *createSVGPathSegLinetoAbs (float x, float y ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoRel 
              *createSVGPathSegLinetoRel (float x, float y ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicAbs 
              *createSVGPathSegCurvetoCubicAbs (float x, float y,
                        float x1, float y1, float x2, float y2 ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicRel 
              *createSVGPathSegCurvetoCubicRel (float x, float y,
                        float x1, float y1, float x2, float y2 ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticAbs 
              *createSVGPathSegCurvetoQuadraticAbs (float x, float y,
                         float x1, float y1 ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticRel 
              *createSVGPathSegCurvetoQuadraticRel (float x, float y,
                         float x1, float y1 ) =0;

    /**
     *
     */
    virtual SVGPathSegArcAbs 
              *createSVGPathSegArcAbs (float x, float y,
                         float r1, float r2, float angle,
                         boolean largeArcFlag, boolean sweepFlag ) =0;

    /**
     *
     */
    virtual SVGPathSegArcRel 
              *createSVGPathSegArcRel (float x, float y, float r1, 
                         float r2, float angle, boolean largeArcFlag,
                         boolean sweepFlag ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoHorizontalAbs 
              *createSVGPathSegLinetoHorizontalAbs (float x ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoHorizontalRel 
              *createSVGPathSegLinetoHorizontalRel (float x ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoVerticalAbs 
              *createSVGPathSegLinetoVerticalAbs (float y ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoVerticalRel 
              *createSVGPathSegLinetoVerticalRel (float y ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicSmoothAbs 
              *createSVGPathSegCurvetoCubicSmoothAbs (float x, float y,
                                             float x2, float y2 ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicSmoothRel 
              *createSVGPathSegCurvetoCubicSmoothRel (float x, float y,
                                                      float x2, float y2 ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticSmoothAbs 
              *createSVGPathSegCurvetoQuadraticSmoothAbs (float x, float y ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticSmoothRel 
              *createSVGPathSegCurvetoQuadraticSmoothRel (float x, float y ) =0;



};//





/*#########################################################################
## SVGRectElement
#########################################################################*/

/**
 *
 */
class SVGRectElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget 
{




    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getHeight() =0;


    /**
     *
     */
    virtual SVGAnimatedLength *getRx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getRy() =0;



};//





/*#########################################################################
## SVGCircleElement
#########################################################################*/

/**
 *
 */
class SVGCircleElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget 
{


    /**
     *
     */
    virtual SVGAnimatedLength *getCx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getCy() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getR() =0;



};//





/*#########################################################################
## SVGLineElement
#########################################################################*/

/**
 *
 */
class SVGEllipseElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget 
{
    /**
     *
     */
    virtual SVGAnimatedLength *getCx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getCy() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getRx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getRy() =0;
};//





/*#########################################################################
## SVGLineElement
#########################################################################*/

/**
 *
 */
class SVGLineElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget 
{
    /**
     *
     */
    virtual SVGAnimatedLength *getX1() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY1() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getX2() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY2() =0;

};//





/*#########################################################################
## SVGAnimatedPoints
#########################################################################*/

/**
 *
 */
class SVGAnimatedPoints 
{

    /**
     *
     */
    virtual SVGPointList *getPoints() =0;

    /**
     *
     */
    virtual SVGPointList *getAnimatedPoints() =0;



};//





/*#########################################################################
## SVGPolylineElement
#########################################################################*/

/**
 *
 */
class SVGPolylineElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget,
                    virtual public SVGAnimatedPoints
{

};



/*#########################################################################
## SVGPolygonElement
#########################################################################*/

/**
 *
 */
class SVGPolygonElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget,
                    virtual public SVGAnimatedPoints
{

};



/*#########################################################################
## SVGTextContentElement
#########################################################################*/

/**
 *
 */
class SVGTextContentElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public events::EventTarget 
{



    /**
     * lengthAdjust Types
     */
    enum
        {
        LENGTHADJUST_UNKNOWN          = 0,
        LENGTHADJUST_SPACING          = 1,
        LENGTHADJUST_SPACINGANDGLYPHS = 2
        };


    /**
     *
     */
    virtual SVGAnimatedLength *getTextLength() =0;


    /**
     *
     */
    virtual SVGAnimatedEnumeration *getLengthAdjust() =0;


    /**
     *
     */
    virtual long getNumberOfChars (  ) =0;

    /**
     *
     */
    virtual float getComputedTextLength (  ) =0;

    /**
     *
     */
    virtual float getSubStringLength (unsigned long charnum, unsigned long nchars )
                                     throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGPoint *getStartPositionOfChar (unsigned long charnum )
                                              throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGPoint *getEndPositionOfChar (unsigned long charnum )
                                           throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGRect *getExtentOfChar (unsigned long charnum )
                                      throw( DOMException ) =0;

    /**
     *
     */
    virtual float getRotationOfChar (unsigned long charnum )
                                     throw( DOMException ) =0;

    /**
     *
     */
    virtual long getCharNumAtPosition (SVGPoint point ) =0;

    /**
     *
     */
    virtual void selectSubString (unsigned long charnum, unsigned long nchars )
                                  throw( DOMException ) =0;



};//





/*#########################################################################
## SVGTextPositioningElement
#########################################################################*/

/**
 *
 */
class SVGTextPositioningElement : virtual public SVGTextContentElement 
{



    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getDx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getDy() =0;


    /**
     *
     */
    virtual SVGAnimatedNumberList *getRotate() =0;



};//





/*#########################################################################
## SVGTextElement
#########################################################################*/

/**
 *
 */
class SVGTextElement : 
                SVGTextPositioningElement,
                    virtual public SVGTransformable
{

};



/*#########################################################################
## SVGTSpanElement
#########################################################################*/

/**
 *
 */
class SVGTSpanElement : virtual public SVGTextPositioningElement
{

};



/*#########################################################################
## SVGTRefElement
#########################################################################*/

/**
 *
 */
class SVGTRefElement : 
                    virtual public SVGTextPositioningElement,
                    virtual public SVGURIReference
{

};



/*#########################################################################
## SVGTextPathElement
#########################################################################*/

/**
 *
 */
class SVGTextPathElement : 
                    virtual public SVGTextContentElement,
                    virtual public SVGURIReference 
{



    /**
     * textPath Method Types
     */
    enum
        {
        TEXTPATH_METHODTYPE_UNKNOWN   = 0,
        TEXTPATH_METHODTYPE_ALIGN     = 1,
        TEXTPATH_METHODTYPE_STRETCH   = 2
        };

    /**
     * textPath Spacing Types
     */
    enum
        {
        TEXTPATH_SPACINGTYPE_UNKNOWN  = 0,
        TEXTPATH_SPACINGTYPE_AUTO     = 1,
        TEXTPATH_SPACINGTYPE_EXACT    = 2
        };


    /**
     *
     */
    virtual SVGAnimatedLength *getStartOffset() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getMethod() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getSpacing() =0;



};//





/*#########################################################################
## SVGAltGlyphElement
#########################################################################*/

/**
 *
 */
class SVGAltGlyphElement : 
                    virtual public SVGTextPositioningElement,
                    virtual public SVGURIReference 
{

    /**
     *
     */
    virtual DOMString getGlyphRef() =0;

    /**
     *
     */
    virtual void setGlyphRef(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getFormat() =0;

    /**
     *
     */
    virtual void setFormat(DOMString& val) throw (DOMException) =0;




};//





/*#########################################################################
## SVGAltGlyphDefElement
#########################################################################*/

/**
 *
 */
class SVGAltGlyphDefElement : virtual public SVGElement
{

};



/*#########################################################################
## SVGAltGlyphItemElement
#########################################################################*/

/**
 *
 */
class SVGAltGlyphItemElement : virtual public SVGElement
{

};



/*#########################################################################
## SVGGlyphRefElement
#########################################################################*/

/**
 *
 */
class SVGGlyphRefElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGStylable 
{
    /**
     *
     */
    virtual DOMString getGlyphRef() =0;

    /**
     *
     */
    virtual void setGlyphRef(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getFormat() =0;

    /**
     *
     */
    virtual void setFormat(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getX() = 0;

    /**
     *
     */
    virtual void setX(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getY() = 0;

    /**
     *
     */
    virtual void setY(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getDx() = 0;

    /**
     *
     */
    virtual void setDx(float val) throw (DOMException) =0;

    /**
     *
     */
    virtual float getDy() = 0;

    /**
     *
     */
    virtual void setDy(float val) throw (DOMException) =0;




};//





/*#########################################################################
## SVGPaint
#########################################################################*/

/**
 *
 */
class SVGPaint : virtual public SVGColor 
{


    /**
     * Paint Types
     */
    enum
        {
        SVG_PAINTTYPE_UNKNOWN               = 0,
        SVG_PAINTTYPE_RGBCOLOR              = 1,
        SVG_PAINTTYPE_RGBCOLOR_ICCCOLOR     = 2,
        SVG_PAINTTYPE_NONE                  = 101,
        SVG_PAINTTYPE_CURRENTCOLOR          = 102,
        SVG_PAINTTYPE_URI_NONE              = 103,
        SVG_PAINTTYPE_URI_CURRENTCOLOR      = 104,
        SVG_PAINTTYPE_URI_RGBCOLOR          = 105,
        SVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR = 106,
        SVG_PAINTTYPE_URI                   = 107
        };


    /**
     *
     */
    virtual unsigned short paintType() =0;

    /**
     *
     */
    virtual DOMString getUri() =0;

    /**
     *
     */
    virtual void setUri (DOMString& uri ) =0;

    /**
     *
     */
    virtual void setPaint (unsigned short paintType, DOMString& uri,
                           DOMString& rgbColor, DOMString& iccColor )
                           throw( SVGException ) =0;



};//





/*#########################################################################
## SVGMarkerElement
#########################################################################*/

/**
 *
 */
class SVGMarkerElement : 
                    virtual public SVGElement,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGFitToViewBox 
{



    /**
     * Marker Unit Types
     */
    enum
        {
        SVG_MARKERUNITS_UNKNOWN        = 0,
        SVG_MARKERUNITS_USERSPACEONUSE = 1,
        SVG_MARKERUNITS_STROKEWIDTH    = 2
        };

    /**
     * Marker Orientation Types
     */
    enum
        {
        SVG_MARKER_ORIENT_UNKNOWN      = 0,
        SVG_MARKER_ORIENT_AUTO         = 1,
        SVG_MARKER_ORIENT_ANGLE        = 2
        };


    /**
     *
     */
    virtual SVGAnimatedLength *getRefX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getRefY() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getMarkerUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getMarkerWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getMarkerHeight() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getOrientType() =0;

    /**
     *
     */
    virtual SVGAnimatedAngle *getOrientAngle() =0;


    /**
     *
     */
    virtual void setOrientToAuto (  ) =0;

    /**
     *
     */
    virtual void setOrientToAngle (SVGAngle *angle ) =0;



};//





/*#########################################################################
## SVGColorProfileElement
#########################################################################*/

/**
 *
 */
class SVGColorProfileElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGRenderingIntent 
{
    /**
     *
     */
    virtual DOMString getLocal() =0;

    /**
     *
     */
    virtual void setLocal(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getName() =0;

    /**
     *
     */
    virtual void setName(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual unsigned short getRenderingIntent() =0;

    /**
     *
     */
    virtual void setRenderingIntent(unsigned short val) throw (DOMException) =0;



};//





/*#########################################################################
## SVGColorProfileRule
#########################################################################*/

/**
 *
 */
class SVGColorProfileRule : 
                    virtual public SVGCSSRule,
                    virtual public SVGRenderingIntent 
{
   /**
     *
     */
    virtual DOMString getSrc() =0;

    /**
     *
     */
    virtual void setSrc(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getName() =0;

    /**
     *
     */
    virtual void setName(DOMString& val) throw (DOMException) =0;

    /**
     *
     */
    virtual unsigned short getRenderingIntent() =0;

    /**
     *
     */
    virtual void setRenderingIntent(unsigned short val) throw (DOMException) =0;



};//





/*#########################################################################
## SVGGradientElement
#########################################################################*/

/**
 *
 */
class SVGGradientElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGUnitTypes 
{



    /**
     * Spread Method Types
     */
    enum
        {
        SVG_SPREADMETHOD_UNKNOWN = 0,
        SVG_SPREADMETHOD_PAD     = 1,
        SVG_SPREADMETHOD_REFLECT = 2,
        SVG_SPREADMETHOD_REPEAT  = 3
        };


    /**
     *
     */
    virtual SVGAnimatedEnumeration *getGradientUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedTransformList *getGradientTransform() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getSpreadMethod() =0;



};//





/*#########################################################################
## SVGLinearGradientElement
#########################################################################*/

/**
 *
 */
class SVGLinearGradientElement : virtual public SVGGradientElement 
{


    /**
     *
     */
    virtual SVGAnimatedLength *getX1() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY1() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getX2() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY2() =0;


};//





/*#########################################################################
## SVGRadialGradientElement
#########################################################################*/

/**
 *
 */
class SVGRadialGradientElement : virtual public SVGGradientElement 
{


    /**
     *
     */
    virtual SVGAnimatedLength *getCx() =0;


    /**
     *
     */
    virtual SVGAnimatedLength *getCy() =0;


    /**
     *
     */
    virtual SVGAnimatedLength *getR() =0;


    /**
     *
     */
    virtual SVGAnimatedLength *getFx() =0;


    /**
     *
     */
    virtual SVGAnimatedLength *getFy() =0;




};//





/*#########################################################################
## SVGStopElement
#########################################################################*/

/**
 *
 */
class SVGStopElement : 
                    virtual public SVGElement,
                    virtual public SVGStylable 
{

    /**
     *
     */
    virtual SVGAnimatedNumber *getOffset() =0;



};//





/*#########################################################################
## SVGPatternElement
#########################################################################*/

/**
 *
 */
class SVGPatternElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGFitToViewBox,
                    virtual public SVGUnitTypes 
{




    /**
     *
     */
    virtual SVGAnimatedEnumeration *getPatternUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getPatternContentUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedTransformList *getPatternTransform() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getHeight() =0;



};//





/*#########################################################################
## SVGClipPathElement
#########################################################################*/

/**
 *
 */
class SVGClipPathElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public SVGUnitTypes 
{
    /**
     *
     */
    virtual SVGAnimatedEnumeration *getClipPathUnits() =0;




};//





/*#########################################################################
## SVGMaskElement
#########################################################################*/

/**
 *
 */
class SVGMaskElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGUnitTypes 
{



    /**
     *
     */
    virtual SVGAnimatedEnumeration *getMaskUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getMaskContentUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getHeight() =0;

};//





/*#########################################################################
## SVGFilterElement
#########################################################################*/

/**
 *
 */
class SVGFilterElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGUnitTypes 
{



    /**
     *
     */
    virtual SVGAnimatedEnumeration *getFilterUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getPrimitiveUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getHeight() =0;


    /**
     *
     */
    virtual SVGAnimatedInteger *getFilterResX() =0;

    /**
     *
     */
    virtual SVGAnimatedInteger *getFilterResY() =0;

    /**
     *
     */
    virtual void setFilterRes (unsigned long filterResX, 
                               unsigned long filterResY ) =0;



};//





/*#########################################################################
## SVGFilterPrimitiveStandardAttributes
#########################################################################*/

/**
 *
 */
class SVGFilterPrimitiveStandardAttributes : virtual public SVGStylable 
{



    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getHeight() =0;

    /**
     *
     */
    virtual SVGAnimatedString *getResult() =0;



};//





/*#########################################################################
## SVGFEBlendElement
#########################################################################*/

/**
 *
 */
class SVGFEBlendElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{


    /**
     * Blend Mode Types
     */
    enum
        {
        SVG_FEBLEND_MODE_UNKNOWN  = 0,
        SVG_FEBLEND_MODE_NORMAL   = 1,
        SVG_FEBLEND_MODE_MULTIPLY = 2,
        SVG_FEBLEND_MODE_SCREEN   = 3,
        SVG_FEBLEND_MODE_DARKEN   = 4,
        SVG_FEBLEND_MODE_LIGHTEN  = 5
        );

    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedString *getIn2() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getMode() =0;


};//





/*#########################################################################
## SVGFEColorMatrixElement
#########################################################################*/

/**
 *
 */
class SVGFEColorMatrixElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{



    /**
     * Color Matrix Types
     */
    enum
        {
        SVG_FECOLORMATRIX_TYPE_UNKNOWN          = 0,
        SVG_FECOLORMATRIX_TYPE_MATRIX           = 1,
        SVG_FECOLORMATRIX_TYPE_SATURATE         = 2,
        SVG_FECOLORMATRIX_TYPE_HUEROTATE        = 3,
        SVG_FECOLORMATRIX_TYPE_LUMINANCETOALPHA = 4
        };


    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getType() =0;

    /**
     *
     */
    virtual SVGAnimatedNumberList *getValues() =0;



};//





/*#########################################################################
## SVGFEComponentTransferElement
#########################################################################*/

/**
 *
 */
class SVGFEComponentTransferElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{
    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;

};//





/*#########################################################################
## SVGComponentTransferFunctionElement
#########################################################################*/

/**
 *
 */
class SVGComponentTransferFunctionElement : virtual public SVGElement 
{


    /**
     * Component Transfer Types
     */
    enum
        {
        SVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN  = 0,
        SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY = 1,
        SVG_FECOMPONENTTRANSFER_TYPE_TABLE    = 2,
        SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE = 3,
        SVG_FECOMPONENTTRANSFER_TYPE_LINEAR   = 4,
        SVG_FECOMPONENTTRANSFER_TYPE_GAMMA    = 5
        };


    /**
     *
     */
    virtual SVGAnimatedEnumeration *getType() =0;

    /**
     *
     */
    virtual SVGAnimatedNumberList *getTableValues() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getSlope() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getIntercept() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getAmplitude() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getExponent() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getOffset() =0;


};//





/*#########################################################################
## SVGFEFuncRElement
#########################################################################*/

/**
 *
 */
class SVGFEFuncRElement : virtual public SVGComponentTransferFunctionElement
{

};



/*#########################################################################
## SVGFEFuncGElement
#########################################################################*/

/**
 *
 */
class SVGFEFuncGElement : virtual public SVGComponentTransferFunctionElement
{

};



/*#########################################################################
## SVGFEFuncBElement
#########################################################################*/

/**
 *
 */
class SVGFEFuncBElement : virtual public SVGComponentTransferFunctionElement
{

};



/*#########################################################################
## SVGFEFuncAElement
#########################################################################*/

/**
 *
 */
class SVGFEFuncAElement : virtual public SVGComponentTransferFunctionElement
{

};



/*#########################################################################
## SVGFECompositeElement
#########################################################################*/

/**
 *
 */
class SVGFECompositeElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{



    /**
     *  Composite Operators
     */
    enum
        {
        SVG_FECOMPOSITE_OPERATOR_UNKNOWN    = 0,
        SVG_FECOMPOSITE_OPERATOR_OVER       = 1,
        SVG_FECOMPOSITE_OPERATOR_IN         = 2,
        SVG_FECOMPOSITE_OPERATOR_OUT        = 3,
        SVG_FECOMPOSITE_OPERATOR_ATOP       = 4,
        SVG_FECOMPOSITE_OPERATOR_XOR        = 5,
        SVG_FECOMPOSITE_OPERATOR_ARITHMETIC = 6
        };

    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedString *getIn2() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getOperator() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getK1() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getK2() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getK3() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getK4() =0;



};//





/*#########################################################################
## SVGFEConvolveMatrixElement
#########################################################################*/

/**
 *
 */
class SVGFEConvolveMatrixElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{



    /**
     * Edge Mode Values
     */
    enum
        {
        SVG_EDGEMODE_UNKNOWN   = 0,
        SVG_EDGEMODE_DUPLICATE = 1,
        SVG_EDGEMODE_WRAP      = 2,
        SVG_EDGEMODE_NONE      = 3
        };


    /**
     *
     */
    virtual SVGAnimatedInteger *getOrderX() =0;

    /**
     *
     */
    virtual SVGAnimatedInteger *getOrderY() =0;

    /**
     *
     */
    virtual SVGAnimatedNumberList *getKernelMatrix() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getDivisor() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getBias() =0;

    /**
     *
     */
    virtual SVGAnimatedInteger *getTargetX() =0;

    /**
     *
     */
    virtual SVGAnimatedInteger *getTargetY() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getEdgeMode() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getKernelUnitLengthX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getKernelUnitLengthY() =0;

    /**
     *
     */
    virtual SVGAnimatedBoolean *getPreserveAlpha() =0;



};//





/*#########################################################################
## SVGFEDiffuseLightingElement
#########################################################################*/

/**
 *
 */
class SVGFEDiffuseLightingElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{

    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getSurfaceScale() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getDiffuseConstant() =0;



};//





/*#########################################################################
## SVGFEDistantLightElement
#########################################################################*/

/**
 *
 */
class SVGFEDistantLightElement : virtual public SVGElement 
{

    /**
     *
     */
    virtual SVGAnimatedNumber *getAzimuth() =0;


    /**
     *
     */
    virtual SVGAnimatedNumber *getElevation() =0;



};//





/*#########################################################################
## SVGFEPointLightElement
#########################################################################*/

/**
 *
 */
class SVGFEPointLightElement : virtual public SVGElement 
{
    /**
     *
     */
    virtual SVGAnimatedNumber *getX() =0;


    /**
     *
     */
    virtual SVGAnimatedNumber *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getZ() =0;

};//





/*#########################################################################
## SVGFESpotLightElement
#########################################################################*/

/**
 *
 */
class SVGFESpotLightElement : virtual public SVGElement 
{

    /**
     *
     */
    virtual SVGAnimatedNumber *getX() =0;


    /**
     *
     */
    virtual SVGAnimatedNumber *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getZ() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getPointsAtX() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getPointsAtY() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getPointsAtZ() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getSpecularExponent() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getLimitingConeAngle() =0;



};//





/*#########################################################################
## SVGFEDisplacementMapElement
#########################################################################*/

/**
 *
 */
class SVGFEDisplacementMapElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{



    /**
     *  Channel Selectors
     */
    enum
        {
        SVG_CHANNEL_UNKNOWN = 0,
        SVG_CHANNEL_R       = 1,
        SVG_CHANNEL_G       = 2,
        SVG_CHANNEL_B       = 3,
        SVG_CHANNEL_A       = 4
        };

    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedString *getIn2() =0;


    /**
     *
     */
    virtual SVGAnimatedNumber *getScale() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getXChannelSelector() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getYChannelSelector() =0;



};//





/*#########################################################################
## SVGFEFloodElement
#########################################################################*/

/**
 *
 */
class SVGFEFloodElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{
    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;


};//





/*#########################################################################
## SVGFEGaussianBlurElement
#########################################################################*/

/**
 *
 */
class SVGFEGaussianBlurElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{
    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;


    /**
     *
     */
    virtual SVGAnimatedNumber *getStdDeviationX() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getStdDeviationY() =0;


    /**
     *
     */
    virtual void setStdDeviation (float stdDeviationX, float stdDeviationY ) =0;



};//





/*#########################################################################
## SVGFEImageElement
#########################################################################*/

/**
 *
 */
class SVGFEImageElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGFilterPrimitiveStandardAttributes
{

};



/*#########################################################################
## SVGFEMergeElement
#########################################################################*/

/**
 *
 */
class SVGFEMergeElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes
{

};



/*#########################################################################
## SVGFEMergeNodeElement
#########################################################################*/

/**
 *
 */
class SVGFEMergeNodeElement : virtual public SVGElement 
{
    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;


};//





/*#########################################################################
## SVGFEMorphologyElement
#########################################################################*/

/**
 *
 */
class SVGFEMorphologyElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{



    /**
     *  Morphology Operators
     */
    enum
        {
        SVG_MORPHOLOGY_OPERATOR_UNKNOWN = 0,
        SVG_MORPHOLOGY_OPERATOR_ERODE   = 1,
        SVG_MORPHOLOGY_OPERATOR_DILATE  = 2
        };


    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;


    /**
     *
     */
    virtual SVGAnimatedEnumeration *getOperator() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getRadiusX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getRadiusY() =0;



};//





/*#########################################################################
## SVGFEOffsetElement
#########################################################################*/

/**
 *
 */
class SVGFEOffsetElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{



    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getDx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getDy() =0;




};//





/*#########################################################################
## SVGFESpecularLightingElement
#########################################################################*/

/**
 *
 */
class SVGFESpecularLightingElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{

    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getSurfaceScale() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getSpecularConstant() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getSpecularExponent() =0;


};//





/*#########################################################################
## SVGFETileElement
#########################################################################*/

/**
 *
 */
class SVGFETileElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{


    /**
     *
     */
    virtual SVGAnimatedString *getIn1() =0;



};//





/*#########################################################################
## SVGFETurbulenceElement
#########################################################################*/

/**
 *
 */
class SVGFETurbulenceElement : 
                    virtual public SVGElement,
                    virtual public SVGFilterPrimitiveStandardAttributes 
{



    /**
     *  Turbulence Types
     */
    enum
        {
        SVG_TURBULENCE_TYPE_UNKNOWN      = 0,
        SVG_TURBULENCE_TYPE_FRACTALNOISE = 1,
        SVG_TURBULENCE_TYPE_TURBULENCE   = 2
        };

    /**
     *  Stitch Options
     */
    enum
        {
        SVG_STITCHTYPE_UNKNOWN  = 0,
        SVG_STITCHTYPE_STITCH   = 1,
        SVG_STITCHTYPE_NOSTITCH = 2
        };



    /**
     *
     */
    virtual SVGAnimatedNumber *getBaseFrequencyX() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getBaseFrequencyY() =0;

    /**
     *
     */
    virtual SVGAnimatedInteger  *getNumOctaves() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber *getSeed() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getStitchTiles() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration *getType() =0;



};//





/*#########################################################################
## SVGCursorElement
#########################################################################*/

/**
 *
 */
class SVGCursorElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGTests,
                    virtual public SVGExternalResourcesRequired 
{
    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

};//





/*#########################################################################
## SVGAElement
#########################################################################*/

/**
 *
 */
class SVGAElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget 
{

    /**
     *
     */
    virtual SVGAnimatedString *getTarget() =0;



};//





/*#########################################################################
## SVGViewElement
#########################################################################*/

/**
 *
 */
class SVGViewElement : 
                    virtual public SVGElement,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGFitToViewBox,
                    virtual public SVGZoomAndPan 
{

    /**
     *
     */
    virtual SVGStringList *getViewTarget() =0;



};//





/*#########################################################################
## SVGScriptElement
#########################################################################*/

/**
 *
 */
class SVGScriptElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGExternalResourcesRequired 
{

    /**
     *
     */
    virtual DOMString getType() =0;

    /**
     *
     */
    virtual void setType(DOMString& val) throw (DOMException) =0;


};//





/*#########################################################################
## SVGEvent
#########################################################################*/

/**
 *
 */
class SVGEvent : events::Event
{

};



/*#########################################################################
## SVGZoomEvent
#########################################################################*/

/**
 *
 */
class SVGZoomEvent : events::UIEvent 
{

    /**
     *
     */
    virtual SVGRect *getZoomRectScreen() =0;

    /**
     *
     */
    virtual float getPreviousScale() =0;

    /**
     *
     */
    virtual SVGPoint *getPreviousTranslate() =0;

    /**
     *
     */
    virtual float getNewScale() =0;
 
   /**
     *
     */
    virtual SVGPoint *getNewTranslate() =0;



};//





/*#########################################################################
## SVGAnimationElement
#########################################################################*/

/**
 *
 */
class SVGAnimationElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGExternalResourcesRequired,
                    virtual public smil::ElementTimeControl,
                    virtual public events::EventTarget 
{


    /**
     *
     */
    virtual SVGElement getTargetElement() =0;


    /**
     *
     */
    virtual float getStartTime (  ) =0;

    /**
     *
     */
    virtual float getCurrentTime (  ) =0;

    /**
     *
     */
    virtual float getSimpleDuration (  )
                    throw( DOMException ) =0;
;



};//





/*#########################################################################
## SVGAnimateElement
#########################################################################*/

/**
 *
 */
class SVGAnimateElement : virtual public SVGAnimationElement
{

};



/*#########################################################################
## SVGSetElement
#########################################################################*/

/**
 *
 */
class SVGSetElement : virtual public SVGAnimationElement
{

};



/*#########################################################################
## SVGAnimateMotionElement
#########################################################################*/

/**
 *
 */
class SVGAnimateMotionElement : virtual public SVGAnimationElement
{

};



/*#########################################################################
## SVGMPathElement
#########################################################################*/

/**
 *
 */
class SVGMPathElement : 
                    virtual public SVGElement,
                    virtual public SVGURIReference,
                    virtual public SVGExternalResourcesRequired
{

};



/*#########################################################################
## SVGAnimateColorElement
#########################################################################*/

/**
 *
 */
class SVGAnimateColorElement : virtual public SVGAnimationElement
{

};



/*#########################################################################
## SVGAnimateTransformElement
#########################################################################*/

/**
 *
 */
class SVGAnimateTransformElement : virtual public SVGAnimationElement
{

};



/*#########################################################################
## SVGFontElement
#########################################################################*/

/**
 *
 */
class SVGFontElement : 
                    virtual public SVGElement,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable
{

};



/*#########################################################################
## SVGGlyphElement
#########################################################################*/

/**
 *
 */
class SVGGlyphElement : 
                    virtual public SVGElement,
                    virtual public SVGStylable
{

};



/*#########################################################################
## SVGMissingGlyphElement
#########################################################################*/

/**
 *
 */
class SVGMissingGlyphElement : 
                    virtual public SVGElement,
                    virtual public SVGStylable
{

};



/*#########################################################################
## SVGHKernElement
#########################################################################*/

/**
 *
 */
class SVGHKernElement : virtual public SVGElement
{

};



/*#########################################################################
## SVGVKernElement
#########################################################################*/

/**
 *
 */
class SVGVKernElement : virtual public SVGElement
{

};



/*#########################################################################
## SVGFontFaceElement
#########################################################################*/

/**
 *
 */
class SVGFontFaceElement : virtual public SVGElement
{

};



/*#########################################################################
## SVGFontFaceSrcElement
#########################################################################*/

/**
 *
 */
class SVGFontFaceSrcElement : virtual public SVGElement
{

};



/*#########################################################################
## SVGFontFaceUriElement
#########################################################################*/

/**
 *
 */
class SVGFontFaceUriElement : virtual public SVGElement
{

};



/*#########################################################################
## SVGFontFaceFormatElement
#########################################################################*/

/**
 *
 */
class SVGFontFaceFormatElement : virtual public SVGElement
{

};



/*#########################################################################
## SVGFontFaceNameElement
#########################################################################*/

/**
 *
 */
class SVGFontFaceNameElement : virtual public SVGElement
{

};



/*#########################################################################
## SVGDefinitionSrcElement
#########################################################################*/

/**
 *
 */
class SVGDefinitionSrcElement : virtual public SVGElement
{

};



/*#########################################################################
## SVGMetadataElement
#########################################################################*/

/**
 *
 */
class SVGMetadataElement : virtual public SVGElement
{

};


/*#########################################################################
## SVGForeignObjectElement
#########################################################################*/

/**
 *
 */
class SVGForeignObjectElement : 
                    virtual public SVGElement,
                    virtual public SVGTests,
                    virtual public SVGLangSpace,
                    virtual public SVGExternalResourcesRequired,
                    virtual public SVGStylable,
                    virtual public SVGTransformable,
                    virtual public events::EventTarget 
{


    /**
     *
     */
    virtual SVGAnimatedLength *getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength *getHeight() =0;



};//

















};//namespace svg
};//namespace dom
};//namespace org
};//namespace w3c

#endif // __SVG_H__
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

