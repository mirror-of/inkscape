/**
 * Inkscape Scripting container
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "InkscapeScript.h"
#include "InkscapePerl.h"
#include "InkscapePython.h"

#include <iostream>
#include <stdio.h>

namespace Inkscape {
namespace Extension {
namespace Script {


/**
 *
 */
InkscapeScript::InkscapeScript()
{



}




/**
 *
 */
InkscapeScript::~InkscapeScript()
{


}




/**
 *
 */
bool InkscapeScript::interpretScript(char *script, ScriptLanguage language)
{

    InkscapeInterpreter *interp;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::PERL)
        {
        interp = new InkscapePerl();
        }
    else if (language == InkscapeScript::PYTHON)
        {
        interp = new InkscapePython();
        }
    else
        {
        //replace with g_error
        fprintf(stderr, "Unknown Script Language type:%d\n", language);
        return false;
        }
        
    if (!interp)
        return false;

    if (!interp->interpretScript(script))
        {
        fprintf(stderr, "error in executing script\n");
        return false;
        }
        
    delete interp;
    
    return true;
}

/**
 *
 */
bool InkscapeScript::interpretUri(char *uri, ScriptLanguage language)
{

    InkscapeInterpreter *interp;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::PERL)
        {
        interp = new InkscapePerl();
        }
    else if (language == InkscapeScript::PYTHON)
        {
        interp = new InkscapePython();
        }
    else
        {
        //replace with g_error
        fprintf(stderr, "Unknown Script Language type:%d\n", language);
        return false;
        }
        
    if (!interp)
        return false;

    if (!interp->interpretUri(uri))
        {
        fprintf(stderr, "error in executing script '%s'\n", uri);
        return false;
        }
        
    return true;
}









}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape

//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
