/*
 * Copyright (c) 2000 World Wide Web Consortium,
 * (Massachusetts Institute of Technology, Institut National de
 * Recherche en Informatique et en Automatique, Keio University). All
 * Rights Reserved. This program is distributed under the W3C's Software
 * Intellectual Property License. This program is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * See W3C License http://www.w3.org/Consortium/Legal/ for more details.
 */

// File: http://www.w3.org/TR/2000/REC-DOM-Level-2-Events-20001113/events.idl

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "dom.h"
#include "views.h"

namespace w3c
{
namespace org
{
namespace dom
{
namespace events
{

//Make local definitions
typedef dom::DOMString DOMString;
typedef dom::DOMTimeStamp DOMTimeStamp;
typedef dom::Node Node;

//forward declarations
class EventListener;
class Event;



/*#########################################################################
## EventException
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class EventException : public dom::DOMException
{
    public:

    EventException(short theCode)
        {
        code = theCode;
        }

    ~EventException() throw()
       {}
};

    /**
     * EventExceptionCode
     */
    enum
        {
        UNSPECIFIED_EVENT_TYPE_ERR = 0
        };





/*#########################################################################
## EventTarget
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class EventTarget 
{

    /**
     *
     */
    virtual void addEventListener(DOMString& type, 
                                  EventListener *listener, 
                                  bool useCapture) =0;

    /**
     *
     */
    virtual void removeEventListener(DOMString& type, 
                                     EventListener *listener, 
                                     bool useCapture) =0;

    /**
     *
     */
    virtual bool dispatchEvent(Event *evt) throw(EventException) =0;
};




/*#########################################################################
## EventListener
#########################################################################*/

/**
 * Introduced in DOM Level 2:
 */
class EventListener
{

    /**
     *
     */
    virtual void handleEvent(Event *evt) =0;
};




/*#########################################################################
## Event
#########################################################################*/

/**
 * Introduced in DOM Level 2:
 */
class Event
{

    /**
     * PhaseType
     */
    enum
        {
        CAPTURING_PHASE = 1,
        AT_TARGET       = 2,
        BUBBLING_PHASE  = 3
        };

    /**
     *
     */
    virtual DOMString getType() =0;

    /**
     *
     */
    virtual EventTarget *getTarget() =0;

    /**
     *
     */
    virtual EventTarget *getCurrentTarget() =0;

    /**
     *
     */
    virtual unsigned short getEventPhase() =0;

    /**
     *
     */
    virtual bool getBubbles() =0;

    /**
     *
     */
    virtual bool getCancelable() =0;

    /**
     *
     */
    virtual DOMTimeStamp getTimeStamp() =0;

    /**
     *
     */
    virtual void stopPropagation();

    /**
     *
     */
    virtual void preventDefault();

    /**
     *
     */
    virtual void initEvent(DOMString& eventTypeArg, 
                           bool canBubbleArg, 
                           bool cancelableArg) =0;
};





/*#########################################################################
## DocumentEvent
#########################################################################*/

/*
 *Introduced in DOM Level 2:
 */
class DocumentEvent
{

    /**
     *
     */
    virtual Event *createEvent(DOMString& eventType) 
                               throw (dom::DOMException) =0;
};




/*#########################################################################
## UIEvent
#########################################################################*/

/**
 * Introduced in DOM Level 2:
 */
class UIEvent : virtual public Event
{

    /**
     *
     */
    virtual views::AbstractView *getView() =0;

    /**
     *
     */
    virtual long getDetail() =0;

    /**
     *
     */
    virtual void initUIEvent(DOMString& typeArg, 
                             bool canBubbleArg, 
                             bool cancelableArg, 
                             views::AbstractView *viewArg, 
                             long detailArg) =0;
};








/*#########################################################################
## MouseEvent
#########################################################################*/

/**
 * Introduced in DOM Level 2:
 */
class MouseEvent : virtual public UIEvent
{

    /**
     *
     */
    virtual long getScreenX() =0;

    /**
     *
     */
    virtual long getScreenY() =0;

    /**
     *
     */
    virtual long getClientX() =0;

    /**
     *
     */
    virtual long getClientY() =0;

    /**
     *
     */
    virtual bool getCtrlKey() =0;

    /**
     *
     */
    virtual bool getShiftKey() =0;

    /**
     *
     */
    virtual bool getAltKey() =0;

    /**
     *
     */
    virtual bool getMetaKey() =0;

    /**
     *
     */
    virtual unsigned short getButton() =0;

    /**
     *
     */
    virtual EventTarget *getRelatedTarget() =0;

    /**
     *
     */
    virtual void initMouseEvent(DOMString& typeArg, 
                                bool canBubbleArg, 
                                bool cancelableArg, 
                                views::AbstractView *viewArg, 
                                long detailArg, 
                                long screenXArg, 
                                long screenYArg, 
                                long clientXArg, 
                                long clientYArg, 
                                bool ctrlKeyArg, 
                                bool altKeyArg, 
                                bool shiftKeyArg, 
                                bool metaKeyArg, 
                                unsigned short buttonArg, 
                                EventTarget *relatedTargetArg) =0;
  };









/*#########################################################################
## MouseEvent
#########################################################################*/

/**
 * Introduced in DOM Level 2:
 */
class MutationEvent : virtual public Event
{

    /**
     * attrChangeType
     */
    enum
        {
        MODIFICATION = 1,
        ADDITION     = 2,
        REMOVAL      = 3
        };

    /**
     *
     */
    virtual Node *getRelatedNode() =0;

    /**
     *
     */
    virtual DOMString getPrevValue() =0;

    /**
     *
     */
    virtual DOMString getNewValue() =0;

    /**
     *
     */
    virtual DOMString getAttrName() =0;

    /**
     *
     */
    virtual unsigned short getAttrChange() =0;

    /**
     *
     */
    virtual void initMutationEvent(DOMString& typeArg, 
                           bool canBubbleArg, 
                           bool cancelableArg, 
                           Node *relatedNodeArg, 
                           DOMString& prevValueArg, 
                           DOMString& newValueArg, 
                           DOMString& attrNameArg, 
                           unsigned short attrChangeArg) =0;

};//class MutationEvent






};//namespace events
};//namespace dom
};//namespace org
};//namespace w3c

#endif // __EVENTS_H__
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

