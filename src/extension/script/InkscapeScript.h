#ifndef __INKSCAPE_SCRIPT_H__
#define __INKSCAPE_SCRIPT_H__

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



namespace Inkscape
{
namespace Extension
{
namespace Script
{


class InkscapeScript
{
public:

    /**
     * Which type of language?
     */
    typedef enum
        {
        PYTHON,
        PERL
        } ScriptLanguage;

    /**
     *
     */
    InkscapeScript();

    /**
     *
     */
    ~InkscapeScript();

    /**
     *
     */
    bool interpretScript(char *script, ScriptLanguage language);

    /**
     *
     */
    bool interpretUri(char *uri, ScriptLanguage language);



}; //class InkscapeScript




}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape



#endif  /* __INKSCAPE_SCRIPT_H__ */
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################


