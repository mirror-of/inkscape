// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * TODO: insert short description here
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "util/longest-common-suffix.h"
#include "xml/repr.h"
#include "xml/node-iterators.h"
#include "repr-sorting.h"

Inkscape::XML::Node const *LCA(Inkscape::XML::Node const *a, Inkscape::XML::Node const *b)
{
    using Inkscape::Algorithms::nearest_common_ancestor;
    Inkscape::XML::Node const *ancestor =
        nearest_common_ancestor<Inkscape::XML::NodeConstParentIterator>(a, b, nullptr);
    bool OK = false;
    if (ancestor) {
        if (ancestor->type() != Inkscape::XML::NodeType::DOCUMENT_NODE) {
            OK = true;
        }
    }
    if ( OK ) {
        return ancestor;
    } else {
        return nullptr;
    }
}

Inkscape::XML::Node *LCA(Inkscape::XML::Node *a, Inkscape::XML::Node *b)
{
    Inkscape::XML::Node const *tmp = LCA(const_cast<Inkscape::XML::Node const *>(a), const_cast<Inkscape::XML::Node const *>(b));
    return const_cast<Inkscape::XML::Node *>(tmp);
}

Inkscape::XML::Node const *AncetreFils(Inkscape::XML::Node const *descendent, Inkscape::XML::Node const *ancestor)
{
    Inkscape::XML::Node const *result = nullptr;
    if ( descendent && ancestor ) {
        if (descendent->parent() == ancestor) {
            result = descendent;
        } else {
            result = AncetreFils(descendent->parent(), ancestor);
        }
    }
    return result;
}

Inkscape::XML::Node *AncetreFils(Inkscape::XML::Node *descendent, Inkscape::XML::Node *ancestor)
{
    Inkscape::XML::Node const * tmp = AncetreFils(const_cast<Inkscape::XML::Node const*>(descendent), const_cast<Inkscape::XML::Node const*>(ancestor));
    return const_cast<Inkscape::XML::Node *>(tmp);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
