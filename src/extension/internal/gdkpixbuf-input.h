
#ifndef INKSCAPE_EXTENSION_INTERNAL_GDKPIXBUF_INPUT_H__
#define INKSCAPE_EXTENSION_INTERNAL_GDKPIXBUF_INPUT_H__


#include "../implementation/implementation.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class GdkpixbufInput : Inkscape::Extension::Implementation::Implementation {

public:
	SPDocument *  open  (Inkscape::Extension::Input *mod,
	                     const gchar *uri);
	static void   init  (void);

};

};};}; /* namespace Inkscape, Extension, Implementation */

#endif /* INKSCAPE_EXTENSION_INTERNAL_GDKPIXBUF_INPUT_H__ */

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
