/*
 * Copyright (c) 2000 World Wide Web Consortium,
 * (Massachusetts Institute of Technology, Institut National de
 * Recherche en Informatique et en Automatique, Keio University). All
 * Rights Reserved. This program is distributed under the W3C's Software
 * Intellectual Property License. This program is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See W3C License http://www.w3.org/Consortium/Legal/ for more
 * details.
 */

// File: smil.idl
#ifndef __SMIL_H__
#define __SMIL_H__

#include "dom.h"
#include "views.h"
#include "events.h"





namespace w3c
{
namespace org
{
namespace dom
{
namespace smil
{




//local definitions
typedef dom::DOMString DOMString;
typedef dom::Element Element;
typedef dom::NodeList NodeList;
typedef dom::Document Document;


//forward declarations
class SMILRegionElement;



/*#########################################################################
## ElementLayout
#########################################################################*/

/**
 *
 */
class ElementLayout
{

    /**
     *
     */
    virtual DOMString getTitle();

    /**
     *
     */
    virtual void setTitle(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getBackgroundColor() =0;

    /**
     *
     */
    virtual void setBackgroundColor(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual long getHeight() =0;

    /**
     *
     */
    virtual void setHeight(long val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual long getWidth() =0;

    /**
     *
     */
    virtual void setWidth(long val) throw (dom::DOMException) =0;

};





/*#########################################################################
## SMILRegionInterface
#########################################################################*/

/**
 *
 */
class SMILRegionInterface
{


    /**
     *
     */
    virtual SMILRegionElement  *getRegion();

    /**
     *
     */
    virtual void setRegion(SMILRegionElement *val);
};






/*#########################################################################
## Time
#########################################################################*/

/**
 *
 */
class Time
{


    /**
     *
     */
    virtual bool getResolved() =0;

    /**
     *
     */
    virtual double resolvedOffset() =0;

    /**
     * TimeTypes
     */
    enum
        {
        SMIL_TIME_INDEFINITE     = 0,
        SMIL_TIME_OFFSET         = 1,
        SMIL_TIME_SYNC_BASED     = 2,
        SMIL_TIME_EVENT_BASED    = 3,
        SMIL_TIME_WALLCLOCK      = 4,
        SMIL_TIME_MEDIA_MARKER   = 5
        };


    /**
     *
     */
    virtual unsigned short getTimeType() =0;

    /**
     *
     */
    virtual double getOffset() =0;

    /**
     *
     */
    virtual void setOffset(double val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual Element *getBaseElement() =0;

    /**
     *
     */
    virtual void setBaseElement(Element *val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual bool getBaseBegin() =0;

    /**
     *
     */
    virtual void setBaseBegin(bool val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getEvent() =0;

    /**
     *
     */
    virtual void setEvent(DOMString& val)  throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getMarker() =0;

    /**
     *
     */
    virtual void setmarker(DOMString& val)  throw (dom::DOMException) =0;

};




/*#########################################################################
## TimeList
#########################################################################*/

/**
 *
 */
class TimeList
{

    /**
     *
     */
    virtual Time *item(unsigned long index) = 0;

    /**
     *
     */
    virtual unsigned long getLength() =0;

};




/*#########################################################################
## ElementTime
#########################################################################*/

/**
 *
 */
class ElementTime
{


    /**
     *
     */
    virtual TimeList *getBegin() =0;

    /**
     *
     */
    virtual void setBegin(TimeList *val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual TimeList *getEnd() =0;

    /**
     *
     */
    virtual void setEnd(TimeList *val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual float getDur() =0;

    /**
     *
     */
    virtual void setDur(float val) throw (dom::DOMException) = 0;

    /**
     * restartTypes
     */
    enum
        {
        RESTART_ALWAYS          = 0,
        RESTART_NEVER           = 1,
        RESTART_WHEN_NOT_ACTIVE = 2
        };


    /**
     *
     */
    virtual unsigned short getRestart() =0;

    /**
     *
     */
    virtual void setRestart(unsigned short val) throw (dom::DOMException) = 0;

    /**
     * fillTypes
     */
    enum
        {
        FILL_REMOVE             = 0,
        FILL_FREEZE             = 1
        };


    /**
     *
     */
    virtual unsigned short getFill() =0;

    /**
     *
     */
    virtual void setFill(unsigned short val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual float getRepeatCount() =0;

    /**
     *
     */
    virtual void setRepeatCount(float val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual float getRepeatDur() =0;

    /**
     *
     */
    virtual void setRepeatDur(float val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual bool beginElement();

    /**
     *
     */
    virtual bool endElement();

    /**
     *
     */
    virtual void pauseElement();

    /**
     *
     */
    virtual void resumeElement();

    /**
     *
     */
    virtual void seekElement(float *seekTo);

};




/*#########################################################################
## ElementTimeManipulation
#########################################################################*/

/**
 *
 */
class ElementTimeManipulation
{

    /**
     *
     */
    virtual float getSpeed() =0;

    /**
     *
     */
    virtual void set () throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual float getAccelerate() =0;

    /**
     *
     */
    virtual void setAccelerate(float val) throw (dom::DOMException) = 0;



    /**
     *
     */
    virtual float getDecelerate() =0;

    /**
     *
     */
    virtual void setDecelerate(float val) throw (dom::DOMException) = 0;

    /**
     *
     */
    virtual bool getAutoReverse() =0;

    /**
     *
     */
    virtual void setAutoReverse(bool val) throw (dom::DOMException) = 0;


};




/*#########################################################################
## ElementTimeContainer
#########################################################################*/

/**
 *
 */
class ElementTimeContainer : virtual public ElementTime
{


    /**
     *
     */
    virtual NodeList *getTimeChildren() =0;

    /**
     *
     */
    virtual NodeList *getActiveChildrenAt(float instant) =0;

};




/*#########################################################################
## ElementSyncBehavior
#########################################################################*/

/**
 *
 */
class ElementSyncBehavior
{


    /**
     *
     */
    virtual DOMString getSyncBehavior() =0;

    /**
     *
     */
    virtual float getSyncTolerance() =0;

    /**
     *
     */
    virtual DOMString getDefaultSyncBehavior() =0;

    /**
     *
     */
    virtual float getDefaultSyncTolerance() =0;

    /**
     *
     */
    virtual bool getSyncMaster() =0;



};




/*#########################################################################
## ElementParallelTimeContainer
#########################################################################*/

/**
 *
 */
class ElementParallelTimeContainer : virtual public ElementTimeContainer
{


    /**
     *
     */
    virtual DOMString getEndSync() =0;

    /**
     *
     */
    virtual void setEndSync(DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual float  getImplicitDuration() =0;

};




/*#########################################################################
## ElementSequentialTimeContainer
#########################################################################*/

/**
 *
 */
class ElementSequentialTimeContainer : virtual public ElementTimeContainer
{

};




/*#########################################################################
## ElementExclusiveTimeContainer
#########################################################################*/

/**
 *
 */
class ElementExclusiveTimeContainer : virtual public ElementTimeContainer
{

    /**
     *
     */
    virtual DOMString getEndSync() =0;

    /**
     *
     */
    virtual void setEndSync(DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual NodeList *getPausedElements() =0;
};




/*#########################################################################
## ElementTimeControl
#########################################################################*/

/**
 *
 */
class ElementTimeControl
{


    /**
     *
     */
    virtual bool beginElement() throw (dom::DOMException) =0;

    /**
     *
     */
    virtual bool beginElementAt(float offset) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual bool endElement() throw (dom::DOMException) =0;

    /**
     *
     */
    virtual bool endElementAt(float offset) throw (dom::DOMException) =0;

};




/*#########################################################################
## ElementTargetAttributes
#########################################################################*/

/**
 *
 */
class ElementTargetAttributes
{

    /**
     *
     */
    virtual DOMString getAttributeName() =0;

    /**
     *
     */
    virtual void setAttributeName(DOMString& val) =0;

    /**
     * attributeTypes
     */
    enum
        {
        ATTRIBUTE_TYPE_AUTO    = 0,
        ATTRIBUTE_TYPE_CSS     = 1,
        ATTRIBUTE_TYPE_XML     = 2
        };


    /**
     *
     */
    virtual unsigned short getAttributeType() =0;

    /**
     *
     */
    virtual void setAttributeType(unsigned short val) =0;
};




/*#########################################################################
## ElementTest
#########################################################################*/

/**
 *
 */
class ElementTest
{


    /**
     *
     */
    virtual long getSystemBitrate() =0;

    /**
     *
     */
    virtual void setSystemBitrate(long val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual bool getSystemCaptions() =0;

    /**
     *
     */
    virtual void setSystemCaptions(bool val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getSystemLanguage() =0;

    /**
     *
     */
    virtual void setSystemlanguage(long val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual bool getSystemRequired() =0;

    /**
     *
     */
    virtual bool getSystemScreenSize() =0;

    /**
     *
     */
    virtual bool getSystemScreenDepth() =0;

    /**
     *
     */
    virtual DOMString getSystemOverdubOrSubtitle() =0;

    /**
     *
     */
    virtual void setSystemOverdubOrSubtitle(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual bool getSystemAudioDesc() =0;

    /**
     *
     */
    virtual void setSystemAudioDesc(bool val) throw (dom::DOMException) =0;

};




/*#########################################################################
## SMILDocument
#########################################################################*/

/**
 *
 */
class SMILDocument : virtual public Document, 
                     virtual public ElementSequentialTimeContainer
{

};




/*#########################################################################
## SMILElement
#########################################################################*/

/**
 *
 */
class SMILElement : virtual public Element
{


    /**
     *
     */
    virtual DOMString getId() =0;

    /**
     *
     */
    virtual void setId(DOMString& val) throw (dom::DOMException) =0;

};




/*#########################################################################
## SMILLayoutElement
#########################################################################*/

/**
 *
 */
class SMILLayoutElement : virtual public SMILElement
{

    /**
     *
     */
    virtual DOMString getType() =0;

    /**
     *
     */
    virtual bool getResolved() =0;
};




/*#########################################################################
## SMILTopLayoutElement
#########################################################################*/

/**
 *
 */
class SMILTopLayoutElement : virtual public SMILElement, virtual public ElementLayout
{

};




/*#########################################################################
## SMILRootLayoutElement
#########################################################################*/

/**
 *
 */
class SMILRootLayoutElement : virtual public SMILElement, virtual public ElementLayout
{

};




/*#########################################################################
## SMILRegionElement
#########################################################################*/

/**
 *
 */
class SMILRegionElement : virtual public SMILElement, virtual public ElementLayout
{

    /**
     *
     */
    virtual DOMString getFit() =0;

    /**
     *
     */
    virtual void setFit(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getTop() =0;

    /**
     *
     */
    virtual void setTop(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual long getZIndex() =0;

    /**
     *
     */
    virtual void setZIndex(long val) throw (dom::DOMException) =0;
};





/*#########################################################################
## TimeEvent
#########################################################################*/

/**
 *
 */
class TimeEvent : virtual public events::Event
{


    /**
     *
     */
    virtual views::AbstractView  *getView() =0;

    /**
     *
     */
    virtual long getDetail() =0;

    /**
     *
     */
    virtual void initTimeEvent(DOMString& typeArg, 
                               views::AbstractView *viewArg, 
                               long detailArg) =0;
};




/*#########################################################################
## SMILMediaElement
#########################################################################*/

/**
 *
 */
class SMILMediaElement : virtual public ElementTime, virtual public SMILElement
{


    /**
     *
     */
    virtual DOMString getAbstractAttr() = 0;

    /**
     *
     */
    virtual void setAbstractAttr(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getAlt() =0;

    /**
     *
     */
    virtual void setAlt(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getAuthor() =0;

    /**
     *
     */
    virtual void setAuthor(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getClipBegin() =0;

    /**
     *
     */
    virtual void setClipBegin(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getClipEnd() =0;

    /**
     *
     */
    virtual void setClipEnd(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getCopyright() =0;

    /**
     *
     */
    virtual void setCopyright(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getLongdesc() =0;

    /**
     *
     */
    virtual void setLongdesc(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getPort() =0;

    /**
     *
     */
    virtual void setPort(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getReadIndex() =0;

    /**
     *
     */
    virtual void setReadIndex(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getRtpformat() =0;

    /**
     *
     */
    virtual void setRtpformat(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getSrc() =0;

    /**
     *
     */
    virtual void setSrc(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getStripRepeat() =0;

    /**
     *
     */
    virtual void setStripRepeat(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getTitle() =0;

    /**
     *
     */
    virtual void setTitle(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getTransport() =0;

    /**
     *
     */
    virtual void setTransport(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getType() =0;

    /**
     *
     */
    virtual void setType(DOMString& val) throw (dom::DOMException) =0;

};




/*#########################################################################
## SMILRefElement
#########################################################################*/

/**
 *
 */
class SMILRefElement : virtual public SMILMediaElement
{

};




/*#########################################################################
## SMILAnimation
#########################################################################*/

/**
 *
 */
class SMILAnimation : virtual public SMILElement, 
                      virtual public ElementTargetAttributes, 
                      virtual public ElementTime,
                      virtual public ElementTimeControl
{

    /**
     * additiveTypes
     */
    enum
        {
        ADDITIVE_REPLACE  = 0,
        ADDITIVE_SUM      = 1
        };

    /**
     *
     */
    virtual unsigned short getAdditive() =0;

    /**
     *
     */
    virtual void setAdditive(unsigned short val) throw (dom::DOMException) =0;


    /**
     *  accumulateTypes
     */
    enum
        {
        ACCUMULATE_NONE  = 0,
        ACCUMULATE_SUM   = 1
        };

    /**
     *
     */
    virtual unsigned short getAccumulate() =0;

    /**
     *
     */
    virtual void setAccumulate(unsigned short val) throw (dom::DOMException) =0;

    /**
     * calcModeTypes
     */
    enum
        {
        CALCMODE_DISCRETE   = 0,
        CALCMODE_LINEAR     = 1,
        CALCMODE_PACED      = 2,
        CALCMODE_SPLINE     = 3
        };

    /**
     *
     */
    virtual unsigned short getCalcMode() =0;

    /**
     *
     */
    virtual void setCalcMode(unsigned short val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getKeySplines() =0;

    /**
     *
     */
    virtual void setKeySplines(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual TimeList *getKeyTimes() =0;

    /**
     *
     */
    virtual void setKeyTimes(TimeList *val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getValues() =0;

    /**
     *
     */
    virtual void setValues(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getFrom() =0;

    /**
     *
     */
    virtual void setFrom(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getTo() =0;

    /**
     *
     */
    virtual void setTo(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getBy() =0;

    /**
     *
     */
    virtual void setBy(DOMString& val) throw (dom::DOMException) =0;

};

/*#########################################################################
## SMILAnimateElement
#########################################################################*/

/**
 *
 */
class SMILAnimateElement : virtual public SMILAnimation
{

};




/*#########################################################################
## SMILSetElement
#########################################################################*/

/**
 *
 */
class SMILSetElement : virtual public ElementTimeControl, virtual public ElementTime,
             virtual public  ElementTargetAttributes, virtual public SMILElement
{
    /**
     *
     */
    virtual DOMString getTo() =0;

    /**
     *
     */
    virtual void setTo(DOMString& val) throw (dom::DOMException) =0;

};




/*#########################################################################
## SMILAnimateMotionElement
#########################################################################*/

/**
 *
 */
class SMILAnimateMotionElement : virtual public SMILAnimateElement
{

    /**
     *
     */
    virtual DOMString getPath() =0;

    /**
     *
     */
    virtual void setPath(DOMString& val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getOrigin() =0;

    /**
     *
     */
    virtual void setOrigin(DOMString& val) throw (dom::DOMException) =0;



};




/*#########################################################################
## SMILAnimateColorElement
#########################################################################*/

/**
 *
 */
class SMILAnimateColorElement : virtual public SMILAnimation
{

};





/*#########################################################################
## SMILSwitchElement
#########################################################################*/

/**
 *
 */
class SMILSwitchElement : virtual public SMILElement
{
    /**
     *
     */
    virtual Element *getSelectedElement() =0;
};











};//namespace smil
};//namespace dom
};//namespace org
};//namespace w3c


#endif // __SMIL_H__

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/
