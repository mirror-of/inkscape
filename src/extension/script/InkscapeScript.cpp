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

#include "InkscapeInterpreter.h"

#ifdef WITH_PERL
#include "InkscapePerl.h"
#endif

#ifdef WITH_PYTHON
#include "InkscapePython.h"
#endif

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
bool InkscapeScript::interpretScript(Glib::ustring &script,
                                 Glib::ustring &output,
                                 Glib::ustring &error,
                                 ScriptLanguage language)
{

    InkscapeInterpreter *interp = NULL;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::PERL)
        {
#ifdef WITH_PERL
        interp = new InkscapePerl();
#endif
        }
    else if (language == InkscapeScript::PYTHON)
        {
#ifdef WITH_PYTHON
        interp = new InkscapePython();
#endif
        }
    else
        {
        //replace with g_error
        fprintf(stderr, "Unknown Script Language type:%d\n", language);
        return false;
        }
        
    if (!interp)
        return false;

    if (!interp->interpretScript(script, output, error))
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
bool InkscapeScript::interpretUri(Glib::ustring &uri,
                                 Glib::ustring &output,
                                 Glib::ustring &error,
                                 ScriptLanguage language)
{

    InkscapeInterpreter *interp = NULL;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::PERL)
        {
#ifdef WITH_PERL
        interp = new InkscapePerl();
#endif
        }
    else if (language == InkscapeScript::PYTHON)
        {
#ifdef WITH_PYTHON
        interp = new InkscapePython();
#endif
        }
    else
        {
        //replace with g_error
        fprintf(stderr, "Unknown Script Language type:%d\n", language);
        return false;
        }
        
    if (!interp)
        return false;

    if (!interp->interpretUri(uri, output, error))
        {
        fprintf(stderr, "error in executing script '%s'\n", uri.raw().c_str());
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
