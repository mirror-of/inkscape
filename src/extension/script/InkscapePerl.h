#ifndef __INKSCAPE_PERL_H__
#define __INKSCAPE_PERL_H__

/**
 * Perl Interpreter wrapper for Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
#include "InkscapeInterpreter.h"

namespace Inkscape
{
namespace Extension
{
namespace Script
{


class InkscapePerl : public InkscapeInterpreter
{
public:

    /*
     *
     */
    InkscapePerl();
    

    /*
     *
     */
    virtual ~InkscapePerl();
    
    

    /*
     *
     */
    bool interpretScript(char *str);
    
    



private:


};

}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape



#endif /*__INKSCAPE_PERL_H__ */
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################

