//#########################################################################
//# $Id$
//#########################################################################

/**
 * Provide the binding of ECMAScript to the SVG Tree
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *   Others
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <jsapi.h>
#include <string.h>

#include <inkscape.h>
#include <document.h>

#include "EcmaBinding.h"


namespace Inkscape {




//#########################################################################
//# Private data containers
//#########################################################################

/**
 * Implementation-specific data for the EcmaObject class.  Notice that
 * this precludes having #include <jsapi.h> in EcmaBinding.h
 */
struct EcmaObjectPrivate
{
    // JS variables
    JSContext *context;
    JSObject  *globalObject;

    // Script object, if any

    // Onclick="script" ,  if any

};



/**
 * Implementation-specific data for the EcmaBinding class.
 */
struct EcmaBindingPrivate
{

    // JS variables
    JSRuntime *runtime;

};







//#########################################################################
//# EcmaObject    Methods
//#########################################################################


/**
 * Constructor.
 *
 * @param parent.  The EcmaBinding container that owns this script
 * chunk.
 *
 */
EcmaObject::EcmaObject(EcmaBinding *theOwner, EcmaObject *theParent)
                                       throw (EcmaException)
{
    // Set the owner
    if (!theOwner)
        throw EcmaException("EcmaObject cannot have a NULL EcmaBinding engine owner");
    owner = theOwner;
    
    // Set the parent
    parent = theParent;
    if (parent)
        parent->addChild(this);
    
    pdata = new EcmaObjectPrivate();
    if (!pdata)
        throw EcmaException("EcmaObject cannot allocate private data");

    // Init
    children = NULL;
    next     = NULL;

}


/**
 * Add a child object to this object
 * @param newNode node to add to this object
 */
void EcmaObject::addChild(EcmaObject *newNode)
{
    if (!newNode)
        return;

    newNode->parent = this;
    //add to the singly-linked list
    //first child?
    if (!children)
      children = newNode;
    else 
        {
        //else find the tail, add to it
        EcmaObject *node = children;
        for ( ; node->next ; node=node->next )
            {
            }
        node->next = newNode;
        }
}




/**
 * Destructor.  Should perform any cleanup, esp the JSContext
 * library.
 */
EcmaObject::~EcmaObject()
{

    EcmaObject *next = NULL;
    for (EcmaObject *obj=children ; obj ; obj=next )
        {
        next = obj->next;
	delete obj;
	}
    delete pdata;

}






//#########################################################################
//# EcmaBinding    Methods
//#########################################################################

/**
 * Constructor.
 *
 * @param theParent.  The Inkscape application who owns this EcmaBinding
 * engine.
 *
 */
EcmaBinding::EcmaBinding(Inkscape::Application *theParent) throw (EcmaException)
{
    parent = theParent;
    if (!parent)
        throw EcmaException("EcmaBinding cannot have a NULL parent app");

    pdata = new EcmaBindingPrivate();
    if (!pdata)
        throw EcmaException("EcmaBinding cannot allocate private data");

    pdata->runtime = JS_NewRuntime(0x100000);
    if (!pdata->runtime)
        throw EcmaException("EcmaBinding unable to create Javascript runtime");

    //No EcmaObjects yet
    rootObject = NULL;
}


/** 
 * Recursive processing of the internal SVG tree.  Called by
 * processDocument() and itself.
 *
 * @param node.  The current node to process, with its children
 * @return  EcmaObject for root of tree at this point.  NULL for
 * a soft error.  Throw an exception for a 'hard' error.
 *
 */
EcmaObject *EcmaBinding::processNode(SPRepr *node, EcmaObject *parent) 
                                            throw (EcmaException)
{

    char *name = (char *) sp_repr_name(node);
    if (!name)
        throw EcmaException("processNode: unnamed node");

    g_message("node:<%s>\n", name);

    //Create an EcmaObject
    EcmaObject *obj = new EcmaObject(this, parent);

    if (strcmp("script", name)==0)
        {
        //we are a <script> node
        //#1 check for 'javascript' or 'ecmascript'
        char *val = (char *)sp_repr_attr(node, "type");
        if (val && strcmp("text/ecmascript", val)==0)
            {
            //#2 get text
            char *text = (char *)sp_repr_content(node);
            if (text)
                {
                //#3 Process the script chunk
                obj->pdata->context = JS_NewContext(parent->owner->pdata->runtime, 8192);
                if (!obj->pdata->context)
                    throw("processNode: could not create context for script chunk");
                JSClass classrec;
                obj->pdata->globalObject = JS_NewObject(obj->pdata->context, &classrec, NULL, NULL);
                if (!obj->pdata->globalObject)
                    throw("processNode: could not create global object for script chunk");
                if (!JS_InitStandardClasses(obj->pdata->context, obj->pdata->globalObject))
                    throw("processNode: could not initialize standard classes for script chunk");
                }
            }

        }
    else
        {
        //we are a normal node
        //#1 check for onClick=""
        char *val = (char *)sp_repr_attr(node, "onclick");
        if (val)
            {
            //#2 Process the script chunk
            obj->pdata->context = JS_NewContext(parent->owner->pdata->runtime, 8192);
            if (!obj->pdata->context)
                throw("processNode: could not create context for script chunk");
            JSClass classrec;
            obj->pdata->globalObject = JS_NewObject(obj->pdata->context, &classrec, NULL, NULL);
            if (!obj->pdata->globalObject)
                throw("processNode: could not create global object for script chunk");
            if (!JS_InitStandardClasses(obj->pdata->context, obj->pdata->globalObject))
                throw("processNode: could not initialize standard classes for script chunk");
            }
        }


    //### descend down the tree
    for (SPRepr *child = sp_repr_children(node) ; child ; child=sp_repr_next(child))
        {
        if (!processNode(child, obj))
            return NULL;
        }

    return obj;
}


/**
 * Get ECMAScript nodes from document and compile scripts
 * This is before running anything.
 *
 * @param document.  The SVG document to process.
 *
 */
bool EcmaBinding::processDocument(SPDocument *theDocument) throw (EcmaException)
{

    if (!parent)
        throw EcmaException("bindToReprTree: NULL app");

    document = theDocument;
    if (!document)
        throw EcmaException("EcmaBinding cannot have a NULL SVG document");
	
    root = sp_document_repr_root(document);
    if (!root)
        throw EcmaException("EcmaBinding cannot bind to SVG document with NULL repr root");

    //Make our tree, mapping 1-to-1 with the SVG tree
    rootObject = processNode(root, NULL);
    if (!rootObject)
        return false;

    return true;
}




/**
 * Destructor.  Should perform any cleanup, esp in the Spidermonkey
 * library.
 */
EcmaBinding::~EcmaBinding()
{

    JS_DestroyRuntime(pdata->runtime);

    //Delete our EcmaObject tree
    delete rootObject;

    //Delete our private data
    delete pdata;

}


/**
 * Test binding from the application
 *
 */
int EcmaBinding::testMe()
{

    Inkscape::Application *mainApp  = INKSCAPE;
    
    SPDocument            *document = SP_ACTIVE_DOCUMENT;
    
    try
        {
        EcmaBinding *engine = new EcmaBinding(mainApp);
        engine->processDocument(document);
        delete engine;
        }
    catch (EcmaException &exc)
        {
	g_error("EcmaBinding test failed:%s\n", exc.what());
	return 0;
	}
    
    g_message("EcmaBinding test succeeded\n");

    return 1;

}








}//namespace Inkscape

//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
