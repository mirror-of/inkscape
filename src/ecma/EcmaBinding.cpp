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

#include <jsapi.h>
#include <inkscape.h>

#include "EcmaBinding.h"


namespace Inkscape {




//#########################################################################
//# Private data containers
//#########################################################################

/**
 * Implementation-specific data for the EcmaScript class.  Notice that
 * this precludes having #include <jsapi.h> in EcmaBinding.h
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
    
    pdata = new EcmaBindingPrivate();
    if (!pdata)
        throw EcmaException("EcmaBinding cannot allocate private data");

    pdata->runtime = JS_NewRuntime(0x100000);
    if (!pdata->runtime)
        throw EcmaException("EcmaBinding unable to create Javascript runtime");
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








}//namespace Inkscape

//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
