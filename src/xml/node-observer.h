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

namespace Inkscape {
namespace XML {
class Node;
}
}


namespace Inkscape {

namespace XML {

class NodeObserver {
public:
    virtual void notifyChildAdded(Inkscape::XML::Node &node, Inkscape::XML::Node &child,
                                  Inkscape::XML::Node *prev)=0;

    virtual void notifyChildRemoved(Inkscape::XML::Node &node, Inkscape::XML::Node &child,
                                    Inkscape::XML::Node *prev)=0;

    virtual void notifyChildOrderChanged(Inkscape::XML::Node &node, Inkscape::XML::Node &child,
                                         Inkscape::XML::Node *old_prev, Inkscape::XML::Node *new_prev)=0;

    virtual void notifyContentChanged(Inkscape::XML::Node &node,
                                      Util::SharedCStringPtr old_content,
                                      Util::SharedCStringPtr new_content)=0;

    virtual void notifyAttributeChanged(Inkscape::XML::Node &node, GQuark name,
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
