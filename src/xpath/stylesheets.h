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

// File: http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113/stylesheets.idl

#ifndef __STYLESHEETS_H__
#define __STYLESHEETS_H__

#include "dom.h"

namespace w3c
{
namespace org
{
namespace dom
{
namespace stylesheets
{



//Make local definitions
typedef dom::DOMString DOMString;
typedef dom::Node Node;

//Forward declaration
class MediaList;




/*#########################################################################
## StyleSheet
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class StyleSheet {

    /**
     *
     */
    virtual DOMString getType() =0;

    /**
     *
     */
    virtual bool getDisabled() =0;

    /**
     *
     */
    virtual void setDisabled(bool val) =0;

    /**
     *
     */
    virtual Node *getOwnerNode() =0;

    /**
     *
     */
    virtual StyleSheet *getParentStyleSheet() =0;

    /**
     *
     */
    virtual DOMString getHref() =0;

    /**
     *
     */
    virtual DOMString getTitle() =0;

    /**
     *
     */
    virtual MediaList getMedia() =0;
};




/*#########################################################################
## StyleSheetList
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class StyleSheetList
{

    /**
     *
     */
    virtual unsigned long getLength() =0;

    /**
     *
     */
    virtual StyleSheet *item(in unsigned long index) =0;
};




/*#########################################################################
## MediaList
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class MediaList
{

    /**
     *
     */
    virtual DOMString getMediaText() =0;

    /**
     *
     */
    virtual void setMediaText(DOMString &val) throw (dom::DOMException) =0;

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
    virtual void deleteMedium(DOMString& oldMedium)
                              throw (dom::DOMException) =0;

    /**
     *
     */
    virtual void appendMedium(DOMString& newMedium)
                               throw (dom::DOMException) =0;
};




/*#########################################################################
## LinkStyle
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class LinkStyle
{

    /**
     *
     */
    virtual StyleSheet *getSheet() =0;
};




/*#########################################################################
## DocumentStyle
#########################################################################*/

/**
 *  Introduced in DOM Level 2:
 */
class DocumentStyle
{

    /**
     *
     */
    virtual StyleSheetList *getStyleSheets() =0;
};





};//namespace stylesheets
};//namespace dom
};//namespace org
};//namespace w3c


#endif // __STYLESHEETS_H__
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

