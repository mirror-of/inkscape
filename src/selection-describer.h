/*
 * Inkscape::SelectionDescriber - shows messages describing selection
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_SELECTION_DESCRIPTION_HANDLER_H
#define SEEN_INKSCAPE_SELECTION_DESCRIPTION_HANDLER_H

#include <sigc++/sigc++.h>
#include "message-context.h"

class SPSelection;

namespace Inkscape {

class MessageStack;

class SelectionDescriber : public SigC::Object {
public:
    SelectionDescriber(SPSelection *selection, MessageStack *stack);

private:
    void _updateMessageFromSelection(SPSelection *selection);

    MessageContext _context;
};

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
