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

#include "../config.h"
#include "helper/sp-intl.h"
#include "selection.h"
#include "message-stack.h"
#include "selection-describer.h"

namespace Inkscape {

SelectionDescriber::SelectionDescriber(SPSelection *selection, MessageStack *stack)
: _context(stack)
{
    selection->connectChanged(sigc::mem_fun(*this, &SelectionDescriber::_updateMessageFromSelection));
    _updateMessageFromSelection(selection);
}

void SelectionDescriber::_updateMessageFromSelection(SPSelection *selection) {
    GSList const *items = selection->itemList();

    char const *when_selected = _("Click selection to toggle scale/rotation handles");
    if (!items) { // no items
        _context.set(Inkscape::NORMAL_MESSAGE, _("No objects selected. Click, Shift+click, or drag around objects to select."));
    } else if (!items->next) { // one item
        _context.setF(Inkscape::NORMAL_MESSAGE, "%s. %s.", sp_item_description(SP_ITEM(items->data)), when_selected);
    } else { // multiple items
        _context.setF(Inkscape::NORMAL_MESSAGE, _("%i objects selected. %s."), g_slist_length((GSList *)items), when_selected);
    }
}

}

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
