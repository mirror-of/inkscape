#ifndef __INKSCAPE_INTERPRETER_H__
#define __INKSCAPE_INTERPRETER_H__

/**
 * Base class for interpreter implementations, (InkscapePython, etc)
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */



namespace Inkscape {
namespace Extension {
namespace Script {


class InkscapeInterpreter
{
public:

    /**
     *
     */
    InkscapeInterpreter();

    /**
     *
     */
    virtual ~InkscapeInterpreter();

    /**
     *
     */
    virtual bool interpretScript(char *script);

    /**
     *
     */
    virtual bool interpretUri(char *uri);



}; //class InkscapeScript




}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape



#endif  /* __INKSCAPE_INTERPRETER_H__ */
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################


