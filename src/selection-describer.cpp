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
#include "desktop.h"
#include "sp-tspan.h"
#include "sp-offset.h"
#include "sp-use.h"

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
    } else {
        SPItem *item = SP_ITEM(items->data);
        SPObject *layer = selection->desktop()->layerForObject (SP_OBJECT (item));
        SPObject *root = selection->desktop()->currentRoot();
        const gchar *layer_name = NULL;
        if (layer != root) {
            if (layer && layer->label()) {
                layer_name = g_strdup_printf (_(" in layer <b>%s</b>"), layer->label());
            } else {
                layer_name = g_strdup_printf (_(" in layer <b><i>%s</i></b>"), layer->defaultLabel());
            }
        }

        if (!items->next) { // one item
            if (SP_IS_USE(item) || (SP_IS_OFFSET(item) && SP_OFFSET (item)->sourceHref) || SP_IS_TEXT_TEXTPATH(item)) {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s. %s.", 
                              sp_item_description(item), layer_name? layer_name : "", _("Use <b>Shift+D</b> to look up original"), when_selected);
            } else {
                _context.setF(Inkscape::NORMAL_MESSAGE, "%s%s. %s.", 
                              sp_item_description(item), layer_name? layer_name : "", when_selected);
            }
        } else { // multiple items
            if (selection->numberOfLayers() == 1) {
                _context.setF(Inkscape::NORMAL_MESSAGE, _("<b>%i</b> objects selected%s. %s."), 
                              g_slist_length((GSList *)items), layer_name? layer_name : "", when_selected);
            } else {
                _context.setF(Inkscape::NORMAL_MESSAGE, _("<b>%i</b> objects selected in <b>%i</b> layers. %s."), 
                              g_slist_length((GSList *)items), selection->numberOfLayers(), when_selected);
            }
        }

        if (layer_name) 
            g_free ((gchar *) layer_name);
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
