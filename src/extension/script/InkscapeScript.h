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



namespace Inkscape {
namespace Extension {
namespace Script {


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

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
