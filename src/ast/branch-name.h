/*
 * Inkscape::AST - Abstract Syntax Tree in a Database
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2003 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_AST_BRANCH_NAME_H
#define SEEN_INKSCAPE_AST_BRANCH_NAME_H

#include <cstdlib>
#include <glib/glib.h>

namespace Inkscape {
namespace AST {

class BranchName {
public:
    BranchName(GQuark axis=0, GQuark name=0) : _axis(axis), _name(name) {}

    BranchName(BranchName const &branch)
    : _axis(branch._axis), _name(branch._name) {}

    GQuark axis() const { return _axis; }
    GQuark name() const { return _name; }

    bool operator==(BranchName const &branch) const {
            return _axis == branch._axis && _name == branch._name;
    }

private:
    void operator=(BranchName const &);

    GQuark _axis;
    GQuark _name;
};

};
};

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
