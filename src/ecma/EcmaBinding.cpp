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

};


/**
 * Implementation-specific data for the EcmaScript class.
 */
struct EcmaScriptPrivate
{
    // JS variables
    JSContext *context;
    JSObject  *globalObject;

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
    if (!children)
      children = newNode;
    else
        {
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
//# EcmaScript    Methods
//#########################################################################


/**
 * Constructor.
 *
 * @param parent.  The EcmaBinding container that owns this script
 * chunk.
 *
 */
EcmaScript::EcmaScript(EcmaBinding *theParent) throw (EcmaException)
{
    // Set the owner
    if (!theParent)
        throw EcmaException("EcmaScript cannot have a NULL parent");
    parent = theParent;
    
    pdata = new EcmaScriptPrivate();
    if (!pdata)
        throw EcmaException("EcmaScript cannot allocate private data");

    // Make a context
    pdata->context = JS_NewContext(parent->pdata->runtime, 8192);
    if (!pdata->context)
        throw EcmaException("Cannot create JSContext for script");

    // Make a global object with the "standard" classes and methods
    JSClass myGlobalClass;
    pdata->globalObject = JS_NewObject(pdata->context, &myGlobalClass, 0, 0);
    if (!pdata->globalObject)
        throw EcmaException("Cannot create global object for JSContext");
    if (!JS_InitStandardClasses(pdata->context, pdata->globalObject))
        throw EcmaException("Cannot init standard classes for JSContext");
	
}



/**
 * Destructor.  Should perform any cleanup, esp the JSContext
 * library.
 */
EcmaScript::~EcmaScript()
{

    JS_DestroyContext(pdata->context);
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
}


/** Get ECMAScript nodes from document and compile scripts
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
        throw EcmaException("EcmaBinding cannot bind do SVG document with NULL repr root");
    



    return true;
}




/**
 * Destructor.  Should perform any cleanup, esp in the Spidermonkey
 * library.
 */
EcmaBinding::~EcmaBinding()
{

    JS_DestroyRuntime(pdata->runtime);
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
