#ifndef __ECMA_BINDING_H__
#define __ECMA_BINDING_H__

/*
 * Provide the binding of ECMAScript to the SVG Tree
 *
 * See the README file in this directory for more information.
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *   Others
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <inkscape.h>

#include <exception>

namespace Inkscape {


//Forward declarations
class  EcmaScript;
struct EcmaScriptPrivate;  //hide impl specifics
class  EcmaBinding;
struct EcmaBindingPrivate;  //hide impl specifics



/**
 * This Exception class can allow us to customize the messages generated
 * by anomalies in the binding and engine.
 */
class EcmaException : public std::exception
{
    public:
        EcmaException(const char *theReason) { reason=(char *)theReason; };
	const char *what() const throw() { return reason; }
    private:
    	char *reason;

};




/**
 * This is one <script> node or onClick="script" chunk from the SVG tree
 */
class EcmaScript
{



public:
    /**
     * Constructor.
     *
     * @param parent.  The EcmaBinding container that owns this script
     * chunk.
     *
     */
    EcmaScript(EcmaBinding *parent) throw (EcmaException);

    /**
     * Destructor.  Should perform any cleanup, esp the JSContext
     * library.
     */
    virtual ~EcmaScript();

    /**
     * Implementation-specific data
     */
    struct EcmaScriptPrivate *pdata;


private:
    // My owner
    EcmaBinding *parent;




};//class EcmaScript






/**
 *
 * This is the container of the JSENgine and the EcmaScript nodes
 * in an SVG tree.
 *
 */
class EcmaBinding
{


public:
    /**
     * Constructor.
     *
     * @param parent.  This is the Inkscape application that owns this
     * EcmaBinding.
     *
     */
    EcmaBinding(Inkscape::Application *parent) throw (EcmaException);

    /**
     * Destructor.  Should perform any cleanup, esp in the Spidermonkey
     * library.
     */
    virtual ~EcmaBinding();


    /**
     * Implementation-specific data
     */
    struct EcmaBindingPrivate *pdata;


private:

    // My owner
    Inkscape::Application *parent;




};//class EcmaBinding







}//namespace Inkscape

#endif /*__ECMA_BINDING_H__*/
