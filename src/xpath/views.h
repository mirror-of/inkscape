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

// File: http://www.w3.org/TR/2000/REC-DOM-Level-2-Views-20001113/views.idl

#ifndef __VIEWS_H__
#define __VIEWS_H__

#include "dom.h"

namespace w3c
{
namespace org
{
namespace dom
{
namespace views
{


//Forward declaration
class DocumentView;

/*#########################################################################
## AbstractView
#########################################################################*/

/**
 * Introduced in DOM Level 2:
 */
class AbstractView
{

    /**
     *
     */
    virtual DocumentView *getDocument() =0;
};

/*#########################################################################
## DocumentView
#########################################################################*/

/**
 * Introduced in DOM Level 2:
 */
class DocumentView
{

    /**
     *
     */
    virtual AbstractView *getDefaultView() =0;
};





};//namespace views
};//namespace dom
};//namespace org
};//namespace w3c


#endif // __VIEWS_H__
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

