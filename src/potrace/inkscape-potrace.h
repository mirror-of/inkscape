/*
 * A quick hack to use the print output to write out a file.  This
 * then makes 'save as...' Postscript.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_POTRACE_H__
#define __INKSCAPE_POTRACE_H__

#include <glib.h>

namespace Inkscape
{
namespace Potrace
{


class Potrace
{

    public:

    Potrace()
        {}

    virtual ~Potrace()
        {}

    
    static gboolean convertImageToPath();


};//class Potrace



};//namespace Potrace
};//namespace Inkscape


#endif  //__INKSCAPE_POTRACE_H__


