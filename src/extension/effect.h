/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifndef INKSCAPE_EXTENSION_EFFECT_H__
#define INKSCAPE_EXTENSION_EFFECT_H__

#include "extension.h"

namespace Inkscape {
namespace Extension {

class Effect : public Extension {

public:
                 Effect  (SPRepr * in_repr,
                          Implementation::Implementation * in_imp);
    virtual     ~Effect  (void);
    virtual bool check                (void);
    GtkDialog *  prefs   (void);
    void         effect  (SPDocument * doc);
};

}; }; /* namespace Inkscape, Extension */
#endif /* INKSCAPE_EXTENSION_EFFECT_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
