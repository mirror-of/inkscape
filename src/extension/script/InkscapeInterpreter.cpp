/**
 * Python Interpreter wrapper for Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "InkscapeInterpreter.h"

#include "svg/stringstream.h"

#include <iostream>
#include <fstream>

namespace Inkscape
{
namespace Extension
{
namespace Script
{

/*
 *
 */
InkscapeInterpreter::InkscapeInterpreter()
{
}

    

/*
 *
 */
InkscapeInterpreter::~InkscapeInterpreter()
{

}

    
    

/*
 *  Interpret an in-memory string
 */
bool InkscapeInterpreter::interpretScript(char *codeStr)
{
    //do nothing.  let the subclasses implement this
    return true;
}

    
    

/*
 *  Interpret a named file
 */
bool InkscapeInterpreter::interpretUri(char *uri)
{
    std::ifstream ins(uri);
    if (!ins.good())
        {
        printf("interpretUri: Could not open %s for reading\n", uri);
        return false;
        }
        
    Inkscape::SVGOStringStream os;
    
    while (!ins.eof())
        {
        char ch = ins.get();
        os << ch;
        }

    ins.close();
    
    char *buf = (char *) os.str().c_str();

    bool ret = interpretScript(buf);

    return ret;

}



}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape

//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
