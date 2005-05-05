/*
 * Inkscape::Debug::Event - event for debug tracing
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DEBUG_EVENT_H
#define SEEN_INKSCAPE_DEBUG_EVENT_H

#include <utility>
#include "util/shared-c-string-ptr.h"

namespace Inkscape {

namespace Debug {

class Event {
public:
    virtual ~Event() {}

    struct PropertyPair {
    public:
        PropertyPair() {}
        PropertyPair(Util::SharedCStringPtr n, Util::SharedCStringPtr v)
        : name(n), value(v) {}
        PropertyPair(char const *n, Util::SharedCStringPtr v)
        : name(Util::SharedCStringPtr::copy(n)), value(v) {}
        PropertyPair(Util::SharedCStringPtr n, char const *v)
        : name(n), value(Util::SharedCStringPtr::copy(v)) {}
        PropertyPair(char const *n, char const *v)
        : name(Util::SharedCStringPtr::copy(n)),
          value(Util::SharedCStringPtr::copy(v)) {}

        Util::SharedCStringPtr name;
        Util::SharedCStringPtr value;
    };

    virtual Util::SharedCStringPtr name() const=0;
    virtual unsigned propertyCount() const=0;
    virtual PropertyPair property(unsigned property) const=0;
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
