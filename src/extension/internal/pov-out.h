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

#ifndef EXTENSION_INTERNAL_POV_OUT_H
#define EXTENSION_INTERNAL_POV_OUT_H

#include <extension/implementation/implementation.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

class PovOutput : Inkscape::Extension::Implementation::Implementation {

public:
	bool check (Inkscape::Extension::Extension * module);
	void          save  (Inkscape::Extension::Output *mod,
	                     SPDocument *doc,
	                     const gchar *uri);
	static void   init  (void);

};

};};}; /* namespace Inkscape, Extension, Implementation */

#endif /* EXTENSION_INTERNAL_POV_OUT_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

