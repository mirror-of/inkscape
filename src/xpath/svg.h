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
class SVGElement : Element
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
## 
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
## 
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
## 
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
## 
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
## 
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
## 
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
    virtual void   clear (  )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGNumber initialize ( SVGNumber *newItem )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGNumber getItem ( unsigned long index )
                    throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGNumber insertItemBefore ( SVGNumber *newItem,
                                         unsigned long index )
                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGNumber replaceItem ( SVGNumber *newItem, 
                                    unsigned long index )
                                    throw( DOMException, SVGException ) =0;

    /**
     *
     */
    virtual SVGNumber removeItem ( unsigned long index )
                                  throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGNumber appendItem ( SVGNumber *newItem )
                                   throw( DOMException, SVGException ) =0;


};//




/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimatedNumberList 
{


    /**
     *
     */
    virtual SVGNumberList getBaseVal() =0;

    /**
     *
     */
    virtual SVGNumberList getAnimVal() =0;



};//





/*#########################################################################
## 
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

   virtual unsigned short getUnitType() =0;

   virtual float getValue() =0;
   virtual void setValue(float val) throw (DOMException) =0;

             attribute float          valueInSpecifiedUnits;
                         // throw DOMException on setting

    virtual DOMString getvalueAsString() =0;
    virtual void setvalueAsString(DOMString& val) throw (DOMException) =0;


    void newValueSpecifiedUnits ( in unsigned short unitType, in float valueInSpecifiedUnits );
    void convertToSpecifiedUnits ( in unsigned short unitType );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimatedLength 
{



    readonly attribute SVGLength baseVal;
    readonly attribute SVGLength animVal;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGLengthList 
{



    readonly attribute unsigned long numberOfItems;

    void   clear (  )
                    throw( DOMException );
    SVGLength initialize ( in SVGLength newItem )
                    throw( DOMException, SVGException );
    SVGLength getItem ( in unsigned long index )
                    throw( DOMException );
    SVGLength insertItemBefore ( in SVGLength newItem, in unsigned long index )
                    throw( DOMException, SVGException );
    SVGLength replaceItem ( in SVGLength newItem, in unsigned long index )
                    throw( DOMException, SVGException );
    SVGLength removeItem ( in unsigned long index )
                    throw( DOMException );
    SVGLength appendItem ( in SVGLength newItem )
                    throw( DOMException, SVGException );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimatedLengthList 
{



    readonly attribute SVGLengthList baseVal;
    readonly attribute SVGLengthList animVal;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAngle 
{



    // Angle Unit Types
    const unsigned short SVG_ANGLETYPE_UNKNOWN     = 0;
    const unsigned short SVG_ANGLETYPE_UNSPECIFIED = 1;
    const unsigned short SVG_ANGLETYPE_DEG         = 2;
    const unsigned short SVG_ANGLETYPE_RAD         = 3;
    const unsigned short SVG_ANGLETYPE_GRAD        = 4;

    readonly attribute unsigned short unitType;
             attribute float          value;
                         // throw DOMException on setting
             attribute float          valueInSpecifiedUnits;
                         // throw DOMException on setting
             attribute DOMString      valueAsString;
                         // throw DOMException on setting

    void newValueSpecifiedUnits ( in unsigned short unitType, in float valueInSpecifiedUnits );
    void convertToSpecifiedUnits ( in unsigned short unitType );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimatedAngle 
{



    readonly attribute SVGAngle baseVal;
    readonly attribute SVGAngle animVal;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGColor : css::CSSValue 
{


    // Color Types
    const unsigned short SVG_COLORTYPE_UNKNOWN           = 0;
    const unsigned short SVG_COLORTYPE_RGBCOLOR          = 1;
    const unsigned short SVG_COLORTYPE_RGBCOLOR_ICCCOLOR = 2;
    const unsigned short SVG_COLORTYPE_CURRENTCOLOR      = 3;

    readonly attribute unsigned short colorType;
    readonly attribute css::RGBColor  rgbColor;
    readonly attribute SVGICCColor    iccColor;

    void        setRGBColor ( in DOMString rgbColor )
                    throw( SVGException );
    void        setRGBColorICCColor ( in DOMString rgbColor, in DOMString iccColor )
                    throw( SVGException );
    void        setColor ( in unsigned short colorType, in DOMString rgbColor, in DOMString iccColor )
                    throw( SVGException );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGICCColor 
{



             attribute DOMString      colorProfile;
                         // throw DOMException on setting
    readonly attribute SVGNumberList colors;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGRect 
{



             attribute float x;
                         // throw DOMException on setting
             attribute float y;
                         // throw DOMException on setting
             attribute float width;
                         // throw DOMException on setting
             attribute float height;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimatedRect 
{



    readonly attribute SVGRect baseVal;
    readonly attribute SVGRect animVal;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGUnitTypes 
{



    // Unit Types
    const unsigned short SVG_UNIT_TYPE_UNKNOWN           = 0;
    const unsigned short SVG_UNIT_TYPE_USERSPACEONUSE    = 1;
    const unsigned short SVG_UNIT_TYPE_OBJECTBOUNDINGBOX = 2;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGStylable 
{



    readonly attribute SVGAnimatedString className;
    readonly attribute css::CSSStyleDeclaration style;

    css::CSSValue getPresentationAttribute ( in DOMString name );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGLocatable 
{



    readonly attribute SVGElement              nearestViewportElement;
    readonly attribute SVGElement              farthestViewportElement;

    SVGRect   getBBox (  );
    SVGMatrix getCTM (  );
    SVGMatrix getScreenCTM (  );
    SVGMatrix getTransformToElement ( in SVGElement element )
                    throw( SVGException );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTransformable : SVGLocatable 
{


    readonly attribute SVGAnimatedTransformList transform;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTests 
{



    readonly attribute SVGStringList requiredFeatures;
    readonly attribute SVGStringList requiredExtensions;
    readonly attribute SVGStringList systemLanguage;

    boolean hasExtension ( in DOMString extension );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGLangSpace 
{



             attribute DOMString xmllang;
                         // throw DOMException on setting
             attribute DOMString xmlspace;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGExternalResourcesRequired 
{



    readonly attribute SVGAnimatedBoolean externalResourcesRequired;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFitToViewBox 
{



    readonly attribute SVGAnimatedRect                viewBox;
    readonly attribute SVGAnimatedPreserveAspectRatio preserveAspectRatio;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGZoomAndPan 
{



    // Zoom and Pan Types
    const unsigned short SVG_ZOOMANDPAN_UNKNOWN   = 0;
    const unsigned short SVG_ZOOMANDPAN_DISABLE = 1;
    const unsigned short SVG_ZOOMANDPAN_MAGNIFY = 2;

             attribute unsigned short zoomAndPan;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGViewSpec : 
                SVGZoomAndPan,
                SVGFitToViewBox 
{



    readonly attribute SVGTransformList transform;
    readonly attribute SVGElement       viewTarget;
    readonly attribute DOMString        viewBoxString;
    readonly attribute DOMString        preserveAspectRatioString;
    readonly attribute DOMString        transformString;
    readonly attribute DOMString        viewTargetString;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGURIReference 
{



    readonly attribute SVGAnimatedString href;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGCSSRule : css::CSSRule 
{


    // Additional CSS RuleType to support ICC color specifications
    const unsigned short COLOR_PROFILE_RULE = 7;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGRenderingIntent 
{



    // Rendering Intent Types
    const unsigned short RENDERING_INTENT_UNKNOWN               = 0;
    const unsigned short RENDERING_INTENT_AUTO                  = 1;
    const unsigned short RENDERING_INTENT_PERCEPTUAL            = 2;
    const unsigned short RENDERING_INTENT_RELATIVE_COLORIMETRIC = 3;
    const unsigned short RENDERING_INTENT_SATURATION            = 4;
    const unsigned short RENDERING_INTENT_ABSOLUTE_COLORIMETRIC = 5;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGDocument : 
                Document,
                events::DocumentEvent 
{



    readonly attribute DOMString    title;
    readonly attribute DOMString     referrer;
    readonly attribute DOMString      domain;
    readonly attribute DOMString      URL;
    readonly attribute SVGSVGElement rootElement;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGSVGElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGLocatable,
                SVGFitToViewBox,
                SVGZoomAndPan,
                events::EventTarget,
                events::DocumentEvent,
                css::ViewCSS,
                css::DocumentCSS 
{



    readonly attribute SVGAnimatedLength x;
    readonly attribute SVGAnimatedLength y;
    readonly attribute SVGAnimatedLength width;
    readonly attribute SVGAnimatedLength height;
             attribute DOMString         contentScriptType;
                         // throw DOMException on setting
             attribute DOMString         contentStyleType;
                         // throw DOMException on setting
    readonly attribute SVGRect           viewport;
    readonly attribute float pixelUnitToMillimeterX;
    readonly attribute float pixelUnitToMillimeterY;
    readonly attribute float screenPixelToMillimeterX;
    readonly attribute float screenPixelToMillimeterY;
             attribute boolean useCurrentView;
                         // throw DOMException on setting
    readonly attribute SVGViewSpec currentView;
             attribute float currentScale;
                         // throw DOMException on setting
    readonly attribute SVGPoint currentTranslate;

    unsigned long suspendRedraw ( in unsigned long max_wait_milliseconds );
    void          unsuspendRedraw ( in unsigned long suspend_handle_id )
                    throw( DOMException );
    void          unsuspendRedrawAll (  );
    void          forceRedraw (  );
    void          pauseAnimations (  );
    void          unpauseAnimations (  );
    boolean       animationsPaused (  );
    float         getCurrentTime (  );
    void          setCurrentTime ( in float seconds );
    NodeList      getIntersectionList ( in SVGRect rect, in SVGElement referenceElement );
    NodeList      getEnclosureList ( in SVGRect rect, in SVGElement referenceElement );
    boolean       checkIntersection ( in SVGElement element, in SVGRect rect );
    boolean       checkEnclosure ( in SVGElement element, in SVGRect rect );
    void          deselectAll (  );
    SVGNumber              createSVGNumber (  );
    SVGLength              createSVGLength (  );
    SVGAngle               createSVGAngle (  );
    SVGPoint               createSVGPoint (  );
    SVGMatrix              createSVGMatrix (  );
    SVGRect                createSVGRect (  );
    SVGTransform           createSVGTransform (  );
    SVGTransform     createSVGTransformFromMatrix ( in SVGMatrix matrix );
    Element         getElementById ( in DOMString elementId );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGGElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGDefsElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGDescElement : 
                SVGElement,
                SVGLangSpace,
                SVGStylable
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTitleElement : 
                SVGElement,
                SVGLangSpace,
                SVGStylable
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGSymbolElement : 
                SVGElement,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGFitToViewBox,
                events::EventTarget
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGUseElement : 
                SVGElement,
                SVGURIReference,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget 
{



    readonly attribute SVGAnimatedLength   x;
    readonly attribute SVGAnimatedLength   y;
    readonly attribute SVGAnimatedLength   width;
    readonly attribute SVGAnimatedLength   height;
    readonly attribute SVGElementInstance instanceRoot;
    readonly attribute SVGElementInstance animatedInstanceRoot;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGElementInstance : events::EventTarget 
{


    readonly attribute SVGElement correspondingElement;
    readonly attribute SVGUseElement correspondingUseElement;
    readonly attribute SVGElementInstance parentNode;
    readonly attribute SVGElementInstanceList childNodes;
    readonly attribute SVGElementInstance firstChild;
    readonly attribute SVGElementInstance lastChild;
    readonly attribute SVGElementInstance previousSibling;
    readonly attribute SVGElementInstance nextSibling;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGElementInstanceList 
{



    readonly attribute unsigned long length;

    SVGElementInstance item ( in unsigned long index );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGImageElement : 
                SVGElement,
                SVGURIReference,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget 
{



    readonly attribute SVGAnimatedLength x;
    readonly attribute SVGAnimatedLength y;
    readonly attribute SVGAnimatedLength width;
    readonly attribute SVGAnimatedLength height;
    readonly attribute SVGAnimatedPreserveAspectRatio preserveAspectRatio;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGSwitchElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class GetSVGDocument 
{



    SVGDocument getSVGDocument (  )
                    throw( DOMException );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGStyleElement : SVGElement 
{


             attribute DOMString xmlspace;
                         // throw DOMException on setting
             attribute DOMString type;
                         // throw DOMException on setting
             attribute DOMString media;
                         // throw DOMException on setting
             attribute DOMString title;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPoint 
{



             attribute float x;
                         // throw DOMException on setting
             attribute float y;
                         // throw DOMException on setting

    SVGPoint matrixTransform ( in SVGMatrix matrix );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPointList 
{



    readonly attribute unsigned long numberOfItems;

    void   clear (  )
                    throw( DOMException );
    SVGPoint initialize ( in SVGPoint newItem )
                    throw( DOMException, SVGException );
    SVGPoint getItem ( in unsigned long index )
                    throw( DOMException );
    SVGPoint insertItemBefore ( in SVGPoint newItem, in unsigned long index )
                    throw( DOMException, SVGException );
    SVGPoint replaceItem ( in SVGPoint newItem, in unsigned long index )
                    throw( DOMException, SVGException );
    SVGPoint removeItem ( in unsigned long index )
                    throw( DOMException );
    SVGPoint appendItem ( in SVGPoint newItem )
                    throw( DOMException, SVGException );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGMatrix 
{



             attribute float a;
                         // throw DOMException on setting
             attribute float b;
                         // throw DOMException on setting
             attribute float c;
                         // throw DOMException on setting
             attribute float d;
                         // throw DOMException on setting
             attribute float e;
                         // throw DOMException on setting
             attribute float f;
                         // throw DOMException on setting

    SVGMatrix multiply ( in SVGMatrix secondMatrix );
    SVGMatrix inverse (  )
                    throw( SVGException );
    SVGMatrix translate ( in float x, in float y );
    SVGMatrix scale ( in float scaleFactor );
    SVGMatrix scaleNonUniform ( in float scaleFactorX, in float scaleFactorY );
    SVGMatrix rotate ( in float angle );
    SVGMatrix rotateFromVector ( in float x, in float y )
                    throw( SVGException );
    SVGMatrix flipX (  );
    SVGMatrix flipY (  );
    SVGMatrix skewX ( in float angle );
    SVGMatrix skewY ( in float angle );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTransform 
{



    // Transform Types
    const unsigned short SVG_TRANSFORM_UNKNOWN   = 0;
    const unsigned short SVG_TRANSFORM_MATRIX    = 1;
    const unsigned short SVG_TRANSFORM_TRANSLATE = 2;
    const unsigned short SVG_TRANSFORM_SCALE     = 3;
    const unsigned short SVG_TRANSFORM_ROTATE    = 4;
    const unsigned short SVG_TRANSFORM_SKEWX     = 5;
    const unsigned short SVG_TRANSFORM_SKEWY     = 6;

    readonly attribute unsigned short type;
    readonly attribute SVGMatrix matrix;
    readonly attribute float angle;

    void setMatrix ( in SVGMatrix matrix );
    void setTranslate ( in float tx, in float ty );
    void setScale ( in float sx, in float sy );
    void setRotate ( in float angle, in float cx, in float cy );
    void setSkewX ( in float angle );
    void setSkewY ( in float angle );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTransformList 
{



    readonly attribute unsigned long numberOfItems;

    void   clear (  )
                    throw( DOMException );
    SVGTransform initialize ( in SVGTransform newItem )
                    throw( DOMException, SVGException );
    SVGTransform getItem ( in unsigned long index )
                    throw( DOMException );
    SVGTransform insertItemBefore ( in SVGTransform newItem, in unsigned long index )
                    throw( DOMException, SVGException );
    SVGTransform replaceItem ( in SVGTransform newItem, in unsigned long index )
                    throw( DOMException, SVGException );
    SVGTransform removeItem ( in unsigned long index )
                    throw( DOMException );
    SVGTransform appendItem ( in SVGTransform newItem )
                    throw( DOMException, SVGException );
    SVGTransform createSVGTransformFromMatrix ( in SVGMatrix matrix );
    SVGTransform consolidate (  );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimatedTransformList 
{



    readonly attribute SVGTransformList baseVal;
    readonly attribute SVGTransformList animVal;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPreserveAspectRatio 
{



    // Alignment Types
    const unsigned short SVG_PRESERVEASPECTRATIO_UNKNOWN   = 0;
    const unsigned short SVG_PRESERVEASPECTRATIO_NONE     = 1;
    const unsigned short SVG_PRESERVEASPECTRATIO_XMINYMIN = 2;
    const unsigned short SVG_PRESERVEASPECTRATIO_XMIDYMIN = 3;
    const unsigned short SVG_PRESERVEASPECTRATIO_XMAXYMIN = 4;
    const unsigned short SVG_PRESERVEASPECTRATIO_XMINYMID = 5;
    const unsigned short SVG_PRESERVEASPECTRATIO_XMIDYMID = 6;
    const unsigned short SVG_PRESERVEASPECTRATIO_XMAXYMID = 7;
    const unsigned short SVG_PRESERVEASPECTRATIO_XMINYMAX = 8;
    const unsigned short SVG_PRESERVEASPECTRATIO_XMIDYMAX = 9;
    const unsigned short SVG_PRESERVEASPECTRATIO_XMAXYMAX = 10;
    // Meet-or-slice Types
    const unsigned short SVG_MEETORSLICE_UNKNOWN   = 0;
    const unsigned short SVG_MEETORSLICE_MEET  = 1;
    const unsigned short SVG_MEETORSLICE_SLICE = 2;

             attribute unsigned short align;
                         // throw DOMException on setting
             attribute unsigned short meetOrSlice;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimatedPreserveAspectRatio 
{



    readonly attribute SVGPreserveAspectRatio baseVal;
    readonly attribute SVGPreserveAspectRatio animVal;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSeg 
{



    // Path Segment Types
    const unsigned short PATHSEG_UNKNOWN                      = 0;
    const unsigned short PATHSEG_CLOSEPATH                    = 1;
    const unsigned short PATHSEG_MOVETO_ABS                   = 2;
    const unsigned short PATHSEG_MOVETO_REL                   = 3;
    const unsigned short PATHSEG_LINETO_ABS                   = 4;
    const unsigned short PATHSEG_LINETO_REL                   = 5;
    const unsigned short PATHSEG_CURVETO_CUBIC_ABS            = 6;
    const unsigned short PATHSEG_CURVETO_CUBIC_REL            = 7;
    const unsigned short PATHSEG_CURVETO_QUADRATIC_ABS        = 8;
    const unsigned short PATHSEG_CURVETO_QUADRATIC_REL        = 9;
    const unsigned short PATHSEG_ARC_ABS                      = 10;
    const unsigned short PATHSEG_ARC_REL                      = 11;
    const unsigned short PATHSEG_LINETO_HORIZONTAL_ABS        = 12;
    const unsigned short PATHSEG_LINETO_HORIZONTAL_REL        = 13;
    const unsigned short PATHSEG_LINETO_VERTICAL_ABS          = 14;
    const unsigned short PATHSEG_LINETO_VERTICAL_REL          = 15;
    const unsigned short PATHSEG_CURVETO_CUBIC_SMOOTH_ABS     = 16;
    const unsigned short PATHSEG_CURVETO_CUBIC_SMOOTH_REL     = 17;
    const unsigned short PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS = 18;
    const unsigned short PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL = 19;

    readonly attribute unsigned short pathSegType;
    readonly attribute DOMString      pathSegTypeAsLetter;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegClosePath : SVGPathSeg
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegMovetoAbs : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegMovetoRel : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoAbs : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoRel : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicAbs : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting
             attribute float   x1;
                         // throw DOMException on setting
             attribute float   y1;
                         // throw DOMException on setting
             attribute float   x2;
                         // throw DOMException on setting
             attribute float   y2;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicRel : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting
             attribute float   x1;
                         // throw DOMException on setting
             attribute float   y1;
                         // throw DOMException on setting
             attribute float   x2;
                         // throw DOMException on setting
             attribute float   y2;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticAbs : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting
             attribute float   x1;
                         // throw DOMException on setting
             attribute float   y1;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticRel : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting
             attribute float   x1;
                         // throw DOMException on setting
             attribute float   y1;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegArcAbs : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting
             attribute float   r1;
                         // throw DOMException on setting
             attribute float   r2;
                         // throw DOMException on setting
             attribute float   angle;
                         // throw DOMException on setting
             attribute boolean largeArcFlag;
                         // throw DOMException on setting
             attribute boolean sweepFlag;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegArcRel : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting
             attribute float   r1;
                         // throw DOMException on setting
             attribute float   r2;
                         // throw DOMException on setting
             attribute float   angle;
                         // throw DOMException on setting
             attribute boolean largeArcFlag;
                         // throw DOMException on setting
             attribute boolean sweepFlag;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoHorizontalAbs : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoHorizontalRel : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoVerticalAbs : SVGPathSeg 
{


             attribute float   y;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoVerticalRel : SVGPathSeg 
{


             attribute float   y;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicSmoothAbs : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting
             attribute float   x2;
                         // throw DOMException on setting
             attribute float   y2;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicSmoothRel : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting
             attribute float   x2;
                         // throw DOMException on setting
             attribute float   y2;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticSmoothAbs : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticSmoothRel : SVGPathSeg 
{


             attribute float   x;
                         // throw DOMException on setting
             attribute float   y;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathSegList 
{



    readonly attribute unsigned long numberOfItems;

    void   clear (  )
                    throw( DOMException );
    SVGPathSeg initialize ( in SVGPathSeg newItem )
                    throw( DOMException, SVGException );
    SVGPathSeg getItem ( in unsigned long index )
                    throw( DOMException );
    SVGPathSeg insertItemBefore ( in SVGPathSeg newItem, in unsigned long index )
                    throw( DOMException, SVGException );
    SVGPathSeg replaceItem ( in SVGPathSeg newItem, in unsigned long index )
                    throw( DOMException, SVGException );
    SVGPathSeg removeItem ( in unsigned long index )
                    throw( DOMException );
    SVGPathSeg appendItem ( in SVGPathSeg newItem )
                    throw( DOMException, SVGException );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimatedPathData 
{



    readonly attribute SVGPathSegList   pathSegList;
    readonly attribute SVGPathSegList   normalizedPathSegList;
    readonly attribute SVGPathSegList   animatedPathSegList;
    readonly attribute SVGPathSegList   animatedNormalizedPathSegList;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPathElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget,
                SVGAnimatedPathData 
{



    readonly attribute SVGAnimatedNumber pathLength;

    float         getTotalLength (  );
    SVGPoint      getPointAtLength ( in float distance );
    unsigned long getPathSegAtLength ( in float distance );
    SVGPathSegClosePath    createSVGPathSegClosePath (  );
    SVGPathSegMovetoAbs    createSVGPathSegMovetoAbs ( in float x, in float y );
    SVGPathSegMovetoRel    createSVGPathSegMovetoRel ( in float x, in float y );
    SVGPathSegLinetoAbs    createSVGPathSegLinetoAbs ( in float x, in float y );
    SVGPathSegLinetoRel    createSVGPathSegLinetoRel ( in float x, in float y );
    SVGPathSegCurvetoCubicAbs    createSVGPathSegCurvetoCubicAbs ( in float x, in float y, in float x1, in float y1, in float x2, in float y2 );
    SVGPathSegCurvetoCubicRel    createSVGPathSegCurvetoCubicRel ( in float x, in float y, in float x1, in float y1, in float x2, in float y2 );
    SVGPathSegCurvetoQuadraticAbs    createSVGPathSegCurvetoQuadraticAbs ( in float x, in float y, in float x1, in float y1 );
    SVGPathSegCurvetoQuadraticRel    createSVGPathSegCurvetoQuadraticRel ( in float x, in float y, in float x1, in float y1 );
    SVGPathSegArcAbs    createSVGPathSegArcAbs ( in float x, in float y, in float r1, in float r2, in float angle, in boolean largeArcFlag, in boolean sweepFlag );
    SVGPathSegArcRel    createSVGPathSegArcRel ( in float x, in float y, in float r1, in float r2, in float angle, in boolean largeArcFlag, in boolean sweepFlag );
    SVGPathSegLinetoHorizontalAbs    createSVGPathSegLinetoHorizontalAbs ( in float x );
    SVGPathSegLinetoHorizontalRel    createSVGPathSegLinetoHorizontalRel ( in float x );
    SVGPathSegLinetoVerticalAbs    createSVGPathSegLinetoVerticalAbs ( in float y );
    SVGPathSegLinetoVerticalRel    createSVGPathSegLinetoVerticalRel ( in float y );
    SVGPathSegCurvetoCubicSmoothAbs    createSVGPathSegCurvetoCubicSmoothAbs ( in float x, in float y, in float x2, in float y2 );
    SVGPathSegCurvetoCubicSmoothRel    createSVGPathSegCurvetoCubicSmoothRel ( in float x, in float y, in float x2, in float y2 );
    SVGPathSegCurvetoQuadraticSmoothAbs    createSVGPathSegCurvetoQuadraticSmoothAbs ( in float x, in float y );
    SVGPathSegCurvetoQuadraticSmoothRel    createSVGPathSegCurvetoQuadraticSmoothRel ( in float x, in float y );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGRectElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget 
{



    readonly attribute SVGAnimatedLength x;
    readonly attribute SVGAnimatedLength y;
    readonly attribute SVGAnimatedLength width;
    readonly attribute SVGAnimatedLength height;
    readonly attribute SVGAnimatedLength rx;
    readonly attribute SVGAnimatedLength ry;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGCircleElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget 
{



    readonly attribute SVGAnimatedLength cx;
    readonly attribute SVGAnimatedLength cy;
    readonly attribute SVGAnimatedLength r;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGEllipseElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget 
{



    readonly attribute SVGAnimatedLength cx;
    readonly attribute SVGAnimatedLength cy;
    readonly attribute SVGAnimatedLength rx;
    readonly attribute SVGAnimatedLength ry;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGLineElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget 
{



    readonly attribute SVGAnimatedLength x1;
    readonly attribute SVGAnimatedLength y1;
    readonly attribute SVGAnimatedLength x2;
    readonly attribute SVGAnimatedLength y2;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimatedPoints 
{



    readonly attribute SVGPointList   points;
    readonly attribute SVGPointList   animatedPoints;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPolylineElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget,
                SVGAnimatedPoints
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPolygonElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget,
                SVGAnimatedPoints
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTextContentElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                events::EventTarget 
{



    // lengthAdjust Types
    const unsigned short LENGTHADJUST_UNKNOWN   = 0;
    const unsigned short LENGTHADJUST_SPACING     = 1;
    const unsigned short LENGTHADJUST_SPACINGANDGLYPHS     = 2;

    readonly attribute SVGAnimatedLength      textLength;
    readonly attribute SVGAnimatedEnumeration lengthAdjust;

    long     getNumberOfChars (  );
    float    getComputedTextLength (  );
    float    getSubStringLength ( in unsigned long charnum, in unsigned long nchars )
                    throw( DOMException );
    SVGPoint getStartPositionOfChar ( in unsigned long charnum )
                    throw( DOMException );
    SVGPoint getEndPositionOfChar ( in unsigned long charnum )
                    throw( DOMException );
    SVGRect  getExtentOfChar ( in unsigned long charnum )
                    throw( DOMException );
    float    getRotationOfChar ( in unsigned long charnum )
                    throw( DOMException );
    long     getCharNumAtPosition ( in SVGPoint point );
    void     selectSubString ( in unsigned long charnum, in unsigned long nchars )
                    throw( DOMException );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTextPositioningElement : SVGTextContentElement 
{


    readonly attribute SVGAnimatedLengthList x;
    readonly attribute SVGAnimatedLengthList y;
    readonly attribute SVGAnimatedLengthList dx;
    readonly attribute SVGAnimatedLengthList dy;
    readonly attribute SVGAnimatedNumberList rotate;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTextElement : 
                SVGTextPositioningElement,
                SVGTransformable
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTSpanElement : SVGTextPositioningElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTRefElement : 
                SVGTextPositioningElement,
                SVGURIReference
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGTextPathElement : 
                SVGTextContentElement,
                SVGURIReference 
{



    // textPath Method Types
    const unsigned short TEXTPATH_METHODTYPE_UNKNOWN   = 0;
    const unsigned short TEXTPATH_METHODTYPE_ALIGN     = 1;
    const unsigned short TEXTPATH_METHODTYPE_STRETCH     = 2;
    // textPath Spacing Types
    const unsigned short TEXTPATH_SPACINGTYPE_UNKNOWN   = 0;
    const unsigned short TEXTPATH_SPACINGTYPE_AUTO     = 1;
    const unsigned short TEXTPATH_SPACINGTYPE_EXACT     = 2;

    readonly attribute SVGAnimatedLength              startOffset;
    readonly attribute SVGAnimatedEnumeration method;
    readonly attribute SVGAnimatedEnumeration spacing;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAltGlyphElement : 
                SVGTextPositioningElement,
                SVGURIReference 
{



             attribute DOMString glyphRef;
                         // throw DOMException on setting
             attribute DOMString format;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAltGlyphDefElement : SVGElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAltGlyphItemElement : SVGElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGGlyphRefElement : 
                SVGElement,
                SVGURIReference,
                SVGStylable 
{



             attribute DOMString glyphRef;
                         // throw DOMException on setting
             attribute DOMString format;
                         // throw DOMException on setting
             attribute float    x;
                         // throw DOMException on setting
             attribute float    y;
                         // throw DOMException on setting
             attribute float    dx;
                         // throw DOMException on setting
             attribute float    dy;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPaint : SVGColor 
{


    // Paint Types
    const unsigned short SVG_PAINTTYPE_UNKNOWN               = 0;
    const unsigned short SVG_PAINTTYPE_RGBCOLOR              = 1;
    const unsigned short SVG_PAINTTYPE_RGBCOLOR_ICCCOLOR     = 2;
    const unsigned short SVG_PAINTTYPE_NONE                  = 101;
    const unsigned short SVG_PAINTTYPE_CURRENTCOLOR          = 102;
    const unsigned short SVG_PAINTTYPE_URI_NONE              = 103;
    const unsigned short SVG_PAINTTYPE_URI_CURRENTCOLOR      = 104;
    const unsigned short SVG_PAINTTYPE_URI_RGBCOLOR          = 105;
    const unsigned short SVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR = 106;
    const unsigned short SVG_PAINTTYPE_URI                   = 107;

    readonly attribute unsigned short paintType;
    readonly attribute DOMString      uri;

    void setUri ( in DOMString uri );
    void setPaint ( in unsigned short paintType, in DOMString uri, in DOMString rgbColor, in DOMString iccColor )
                    throw( SVGException );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGMarkerElement : 
                SVGElement,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGFitToViewBox 
{



    // Marker Unit Types
    const unsigned short SVG_MARKERUNITS_UNKNOWN        = 0;
    const unsigned short SVG_MARKERUNITS_USERSPACEONUSE = 1;
    const unsigned short SVG_MARKERUNITS_STROKEWIDTH    = 2;
    // Marker Orientation Types
    const unsigned short SVG_MARKER_ORIENT_UNKNOWN      = 0;
    const unsigned short SVG_MARKER_ORIENT_AUTO         = 1;
    const unsigned short SVG_MARKER_ORIENT_ANGLE        = 2;

    readonly attribute SVGAnimatedLength      refX;
    readonly attribute SVGAnimatedLength      refY;
    readonly attribute SVGAnimatedEnumeration markerUnits;
    readonly attribute SVGAnimatedLength      markerWidth;
    readonly attribute SVGAnimatedLength      markerHeight;
    readonly attribute SVGAnimatedEnumeration orientType;
    readonly attribute SVGAnimatedAngle      orientAngle;

    void setOrientToAuto (  );
    void setOrientToAngle ( in SVGAngle angle );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGColorProfileElement : 
                SVGElement,
                SVGURIReference,
                SVGRenderingIntent 
{



             attribute DOMString      local;
                         // throw DOMException on setting
             attribute DOMString      name;
                         // throw DOMException on setting
             attribute unsigned short renderingIntent;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGColorProfileRule : 
                SVGCSSRule,
                SVGRenderingIntent 
{



             attribute DOMString      src;
                         // throw DOMException on setting
             attribute DOMString      name;
                         // throw DOMException on setting
             attribute unsigned short renderingIntent;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGGradientElement : 
                SVGElement,
                SVGURIReference,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGUnitTypes 
{



    // Spread Method Types
    const unsigned short SVG_SPREADMETHOD_UNKNOWN = 0;
    const unsigned short SVG_SPREADMETHOD_PAD     = 1;
    const unsigned short SVG_SPREADMETHOD_REFLECT = 2;
    const unsigned short SVG_SPREADMETHOD_REPEAT  = 3;

    readonly attribute SVGAnimatedEnumeration   gradientUnits;
    readonly attribute SVGAnimatedTransformList gradientTransform;
    readonly attribute SVGAnimatedEnumeration   spreadMethod;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGLinearGradientElement : SVGGradientElement 
{


    readonly attribute SVGAnimatedLength x1;
    readonly attribute SVGAnimatedLength y1;
    readonly attribute SVGAnimatedLength x2;
    readonly attribute SVGAnimatedLength y2;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGRadialGradientElement : SVGGradientElement 
{


    readonly attribute SVGAnimatedLength cx;
    readonly attribute SVGAnimatedLength cy;
    readonly attribute SVGAnimatedLength r;
    readonly attribute SVGAnimatedLength fx;
    readonly attribute SVGAnimatedLength fy;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGStopElement : 
                SVGElement,
                SVGStylable 
{



    readonly attribute SVGAnimatedNumber offset;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGPatternElement : 
                SVGElement,
                SVGURIReference,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGFitToViewBox,
                SVGUnitTypes 
{



    readonly attribute SVGAnimatedEnumeration   patternUnits;
    readonly attribute SVGAnimatedEnumeration   patternContentUnits;
    readonly attribute SVGAnimatedTransformList patternTransform;
    readonly attribute SVGAnimatedLength        x;
    readonly attribute SVGAnimatedLength        y;
    readonly attribute SVGAnimatedLength        width;
    readonly attribute SVGAnimatedLength        height;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGClipPathElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                SVGUnitTypes 
{



    readonly attribute SVGAnimatedEnumeration clipPathUnits;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGMaskElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGUnitTypes 
{



    readonly attribute SVGAnimatedEnumeration maskUnits;
    readonly attribute SVGAnimatedEnumeration maskContentUnits;
    readonly attribute SVGAnimatedLength      x;
    readonly attribute SVGAnimatedLength      y;
    readonly attribute SVGAnimatedLength      width;
    readonly attribute SVGAnimatedLength      height;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFilterElement : 
                SVGElement,
                SVGURIReference,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGUnitTypes 
{



    readonly attribute SVGAnimatedEnumeration filterUnits;
    readonly attribute SVGAnimatedEnumeration primitiveUnits;
    readonly attribute SVGAnimatedLength      x;
    readonly attribute SVGAnimatedLength      y;
    readonly attribute SVGAnimatedLength      width;
    readonly attribute SVGAnimatedLength      height;
    readonly attribute SVGAnimatedInteger    filterResX;
    readonly attribute SVGAnimatedInteger    filterResY;

    void setFilterRes ( in unsigned long filterResX, in unsigned long filterResY );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFilterPrimitiveStandardAttributes : SVGStylable 
{


    readonly attribute SVGAnimatedLength x;
    readonly attribute SVGAnimatedLength y;
    readonly attribute SVGAnimatedLength width;
    readonly attribute SVGAnimatedLength height;
    readonly attribute SVGAnimatedString result;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEBlendElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    // Blend Mode Types
    const unsigned short SVG_FEBLEND_MODE_UNKNOWN  = 0;
    const unsigned short SVG_FEBLEND_MODE_NORMAL   = 1;
    const unsigned short SVG_FEBLEND_MODE_MULTIPLY = 2;
    const unsigned short SVG_FEBLEND_MODE_SCREEN   = 3;
    const unsigned short SVG_FEBLEND_MODE_DARKEN   = 4;
    const unsigned short SVG_FEBLEND_MODE_LIGHTEN  = 5;

    readonly attribute SVGAnimatedString      in1;
    readonly attribute SVGAnimatedString      in2;
    readonly attribute SVGAnimatedEnumeration mode;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEColorMatrixElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    // Color Matrix Types
    const unsigned short SVG_FECOLORMATRIX_TYPE_UNKNOWN          = 0;
    const unsigned short SVG_FECOLORMATRIX_TYPE_MATRIX           = 1;
    const unsigned short SVG_FECOLORMATRIX_TYPE_SATURATE         = 2;
    const unsigned short SVG_FECOLORMATRIX_TYPE_HUEROTATE        = 3;
    const unsigned short SVG_FECOLORMATRIX_TYPE_LUMINANCETOALPHA = 4;

    readonly attribute SVGAnimatedString      in1;
    readonly attribute SVGAnimatedEnumeration type;
    readonly attribute SVGAnimatedNumberList  values;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEComponentTransferElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    readonly attribute SVGAnimatedString in1;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGComponentTransferFunctionElement : SVGElement 
{


    // Component Transfer Types
    const unsigned short SVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN  = 0;
    const unsigned short SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY = 1;
    const unsigned short SVG_FECOMPONENTTRANSFER_TYPE_TABLE    = 2;
    const unsigned short SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE    = 3;
    const unsigned short SVG_FECOMPONENTTRANSFER_TYPE_LINEAR   = 4;
    const unsigned short SVG_FECOMPONENTTRANSFER_TYPE_GAMMA    = 5;

    readonly attribute SVGAnimatedEnumeration type;
    readonly attribute SVGAnimatedNumberList  tableValues;
    readonly attribute SVGAnimatedNumber      slope;
    readonly attribute SVGAnimatedNumber      intercept;
    readonly attribute SVGAnimatedNumber      amplitude;
    readonly attribute SVGAnimatedNumber      exponent;
    readonly attribute SVGAnimatedNumber      offset;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEFuncRElement : SVGComponentTransferFunctionElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEFuncGElement : SVGComponentTransferFunctionElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEFuncBElement : SVGComponentTransferFunctionElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEFuncAElement : SVGComponentTransferFunctionElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFECompositeElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    // Composite Operators
    const unsigned short SVG_FECOMPOSITE_OPERATOR_UNKNOWN    = 0;
    const unsigned short SVG_FECOMPOSITE_OPERATOR_OVER       = 1;
    const unsigned short SVG_FECOMPOSITE_OPERATOR_IN         = 2;
    const unsigned short SVG_FECOMPOSITE_OPERATOR_OUT        = 3;
    const unsigned short SVG_FECOMPOSITE_OPERATOR_ATOP       = 4;
    const unsigned short SVG_FECOMPOSITE_OPERATOR_XOR        = 5;
    const unsigned short SVG_FECOMPOSITE_OPERATOR_ARITHMETIC = 6;

    readonly attribute SVGAnimatedString      in1;
    readonly attribute SVGAnimatedString      in2;
    readonly attribute SVGAnimatedEnumeration operator;
    readonly attribute SVGAnimatedNumber      k1;
    readonly attribute SVGAnimatedNumber      k2;
    readonly attribute SVGAnimatedNumber      k3;
    readonly attribute SVGAnimatedNumber      k4;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEConvolveMatrixElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    // Edge Mode Values
    const unsigned short SVG_EDGEMODE_UNKNOWN   = 0;
    const unsigned short SVG_EDGEMODE_DUPLICATE = 1;
    const unsigned short SVG_EDGEMODE_WRAP      = 2;
    const unsigned short SVG_EDGEMODE_NONE      = 3;

    readonly attribute SVGAnimatedInteger     orderX;
    readonly attribute SVGAnimatedInteger     orderY;
    readonly attribute SVGAnimatedNumberList  kernelMatrix;
    readonly attribute SVGAnimatedNumber      divisor;
    readonly attribute SVGAnimatedNumber      bias;
    readonly attribute SVGAnimatedInteger     targetX;
    readonly attribute SVGAnimatedInteger     targetY;
    readonly attribute SVGAnimatedEnumeration edgeMode;
    readonly attribute SVGAnimatedLength      kernelUnitLengthX;
    readonly attribute SVGAnimatedLength      kernelUnitLengthY;
    readonly attribute SVGAnimatedBoolean     preserveAlpha;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEDiffuseLightingElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    readonly attribute SVGAnimatedString in1;
    readonly attribute SVGAnimatedNumber surfaceScale;
    readonly attribute SVGAnimatedNumber diffuseConstant;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEDistantLightElement : SVGElement 
{


    readonly attribute SVGAnimatedNumber azimuth;
    readonly attribute SVGAnimatedNumber elevation;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEPointLightElement : SVGElement 
{


    readonly attribute SVGAnimatedNumber x;
    readonly attribute SVGAnimatedNumber y;
    readonly attribute SVGAnimatedNumber z;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFESpotLightElement : SVGElement 
{


    readonly attribute SVGAnimatedNumber x;
    readonly attribute SVGAnimatedNumber y;
    readonly attribute SVGAnimatedNumber z;
    readonly attribute SVGAnimatedNumber pointsAtX;
    readonly attribute SVGAnimatedNumber pointsAtY;
    readonly attribute SVGAnimatedNumber pointsAtZ;
    readonly attribute SVGAnimatedNumber specularExponent;
    readonly attribute SVGAnimatedNumber limitingConeAngle;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEDisplacementMapElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    // Channel Selectors
    const unsigned short SVG_CHANNEL_UNKNOWN = 0;
    const unsigned short SVG_CHANNEL_R       = 1;
    const unsigned short SVG_CHANNEL_G       = 2;
    const unsigned short SVG_CHANNEL_B       = 3;
    const unsigned short SVG_CHANNEL_A       = 4;

    readonly attribute SVGAnimatedString      in1;
    readonly attribute SVGAnimatedString      in2;
    readonly attribute SVGAnimatedNumber      scale;
    readonly attribute SVGAnimatedEnumeration xChannelSelector;
    readonly attribute SVGAnimatedEnumeration yChannelSelector;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEFloodElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    readonly attribute SVGAnimatedString      in1;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEGaussianBlurElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    readonly attribute SVGAnimatedString in1;
    readonly attribute SVGAnimatedNumber stdDeviationX;
    readonly attribute SVGAnimatedNumber stdDeviationY;

    void setStdDeviation ( in float stdDeviationX, in float stdDeviationY );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEImageElement : 
                SVGElement,
                SVGURIReference,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGFilterPrimitiveStandardAttributes
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEMergeElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEMergeNodeElement : SVGElement 
{


    readonly attribute SVGAnimatedString in1;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEMorphologyElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    // Morphology Operators
    const unsigned short SVG_MORPHOLOGY_OPERATOR_UNKNOWN = 0;
    const unsigned short SVG_MORPHOLOGY_OPERATOR_ERODE   = 1;
    const unsigned short SVG_MORPHOLOGY_OPERATOR_DILATE  = 2;

    readonly attribute SVGAnimatedString      in1;
    readonly attribute SVGAnimatedEnumeration operator;
    readonly attribute SVGAnimatedLength      radiusX;
    readonly attribute SVGAnimatedLength      radiusY;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFEOffsetElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    readonly attribute SVGAnimatedString in1;
    readonly attribute SVGAnimatedNumber dx;
    readonly attribute SVGAnimatedNumber dy;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFESpecularLightingElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    readonly attribute SVGAnimatedString in1;
    readonly attribute SVGAnimatedNumber surfaceScale;
    readonly attribute SVGAnimatedNumber specularConstant;
    readonly attribute SVGAnimatedNumber specularExponent;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFETileElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    readonly attribute SVGAnimatedString in1;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFETurbulenceElement : 
                SVGElement,
                SVGFilterPrimitiveStandardAttributes 
{



    // Turbulence Types
    const unsigned short SVG_TURBULENCE_TYPE_UNKNOWN      = 0;
    const unsigned short SVG_TURBULENCE_TYPE_FRACTALNOISE = 1;
    const unsigned short SVG_TURBULENCE_TYPE_TURBULENCE   = 2;
    // Stitch Options
    const unsigned short SVG_STITCHTYPE_UNKNOWN  = 0;
    const unsigned short SVG_STITCHTYPE_STITCH   = 1;
    const unsigned short SVG_STITCHTYPE_NOSTITCH = 2;

    readonly attribute SVGAnimatedNumber      baseFrequencyX;
    readonly attribute SVGAnimatedNumber      baseFrequencyY;
    readonly attribute SVGAnimatedInteger     numOctaves;
    readonly attribute SVGAnimatedNumber      seed;
    readonly attribute SVGAnimatedEnumeration stitchTiles;
    readonly attribute SVGAnimatedEnumeration type;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGCursorElement : 
                SVGElement,
                SVGURIReference,
                SVGTests,
                SVGExternalResourcesRequired 
{



    readonly attribute SVGAnimatedLength x;
    readonly attribute SVGAnimatedLength y;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAElement : 
                SVGElement,
                SVGURIReference,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget 
{



    readonly attribute SVGAnimatedString target;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGViewElement : 
                SVGElement,
                SVGExternalResourcesRequired,
                SVGFitToViewBox,
                SVGZoomAndPan 
{



    readonly attribute SVGStringList viewTarget;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGScriptElement : 
                SVGElement,
                SVGURIReference,
                SVGExternalResourcesRequired 
{



             attribute DOMString type;
                         // throw DOMException on setting



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGEvent : events::Event
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGZoomEvent : events::UIEvent 
{


    readonly attribute SVGRect zoomRectScreen;
    readonly attribute float previousScale;
    readonly attribute SVGPoint previousTranslate;
    readonly attribute float newScale;
    readonly attribute SVGPoint newTranslate;



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimationElement : 
                SVGElement,
                SVGTests,
                SVGExternalResourcesRequired,
                smil::ElementTimeControl,
                events::EventTarget 
{



    readonly attribute SVGElement targetElement;

    float getStartTime (  );
    float getCurrentTime (  );
    float getSimpleDuration (  )
                    throw( DOMException );



};//





/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimateElement : SVGAnimationElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGSetElement : SVGAnimationElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimateMotionElement : SVGAnimationElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGMPathElement : 
                SVGElement,
                SVGURIReference,
                SVGExternalResourcesRequired
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimateColorElement : SVGAnimationElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGAnimateTransformElement : SVGAnimationElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFontElement : 
                SVGElement,
                SVGExternalResourcesRequired,
                SVGStylable
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGGlyphElement : 
                SVGElement,
                SVGStylable
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGMissingGlyphElement : 
                SVGElement,
                SVGStylable
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGHKernElement : SVGElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGVKernElement : SVGElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFontFaceElement : SVGElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFontFaceSrcElement : SVGElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFontFaceUriElement : SVGElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFontFaceFormatElement : SVGElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGFontFaceNameElement : SVGElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGDefinitionSrcElement : SVGElement
{

};



/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGMetadataElement : SVGElement
{

};


/*#########################################################################
## 
#########################################################################*/

/**
 *
 */
class SVGForeignObjectElement : 
                SVGElement,
                SVGTests,
                SVGLangSpace,
                SVGExternalResourcesRequired,
                SVGStylable,
                SVGTransformable,
                events::EventTarget 
{



    readonly attribute SVGAnimatedLength x;
    readonly attribute SVGAnimatedLength y;
    readonly attribute SVGAnimatedLength width;
    readonly attribute SVGAnimatedLength height;



};//

















};//namespace svg
};//namespace dom
};//namespace org
};//namespace w3c

#endif // __SVG_H__
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

