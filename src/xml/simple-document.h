/*
 * Inkscape::XML::SimpleDocument - generic XML document implementation
 *
 * Copyright 2004-2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_SIMPLE_DOCUMENT_H
#define SEEN_INKSCAPE_XML_SIMPLE_DOCUMENT_H

#include "xml/sp-repr-doc.h"
#include "xml/simple-node.h"

namespace Inkscape {

namespace XML {

struct SimpleDocument : public SimpleNode, public SPReprDoc {
    explicit SimpleDocument(int code) : SimpleNode(code) {
        _initBindings();
    }

    SPReprType type() const { return SP_XML_DOCUMENT_NODE; }

protected:
    SimpleDocument(SimpleDocument const &doc) : SPRepr(), SimpleNode(doc), SPReprDoc() {
        _initBindings();
    }

    SimpleNode *_duplicate() const { return new SimpleDocument(*this); }

private:
    void _initBindings();
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
