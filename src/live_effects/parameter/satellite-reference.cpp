// SPDX-License-Identifier: GPL-2.0-or-later

#include "satellite-reference.h"

#include "document.h"
#include "live_effects/lpeobject.h"
#include "object/sp-item-group.h"
#include "object/sp-lpe-item.h"
#include "object/sp-shape.h"
#include "object/sp-text.h"
#include "object/uri-references.h"

// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * TODO: insert short description here
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
namespace Inkscape {

namespace LivePathEffect {

bool SatelliteReference::_acceptObject(SPObject *const obj) const
{
    if (SP_IS_SHAPE(obj) || SP_IS_TEXT(obj) || SP_IS_GROUP(obj)) {
        /* Refuse references to lpeobject */
        SPObject *owner = getOwner();
        if (obj == owner) {
            return false;
        }
        if (!dynamic_cast<LivePathEffectObject *>(owner)) {
            return false;
        }
        return URIReference::_acceptObject(obj);
    } else {
        return false;
    }
}

} /* namespace LivePathEffect */

} /* namespace Inkscape */

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
