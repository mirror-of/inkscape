/*
 * Inkscape::XML::NodeObserver - interface implemented by observers of XML nodes
 *
 * Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_NODE_OBSERVER_H
#define SEEN_INKSCAPE_XML_NODE_OBSERVER_H

#include <glib/gquark.h>
#include "util/shared-c-string-ptr.h"

class SPRepr;

namespace Inkscape {

namespace XML {

class NodeObserver {
public:
    virtual void notifyChildAdded(SPRepr &node, SPRepr &child,
                                  SPRepr *prev)=0;

    virtual void notifyChildRemoved(SPRepr &node, SPRepr &child,
                                    SPRepr *prev)=0;

    virtual void notifyChildOrderChanged(SPRepr &node, SPRepr &child,
                                         SPRepr *old_prev, SPRepr *new_prev)=0;

    virtual void notifyContentChanged(SPRepr &node,
                                      Util::SharedCStringPtr old_content,
                                      Util::SharedCStringPtr new_content)=0;

    virtual void notifyAttributeChanged(SPRepr &node, GQuark name,
                                        Util::SharedCStringPtr old_value,
                                        Util::SharedCStringPtr new_value)=0;
};

}

}

#endif
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
