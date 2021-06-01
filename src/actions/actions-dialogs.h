// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for dialogs.
 *
 * Copyright (C) 2021 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#ifndef INK_ACTIONS_DIALOGS_H
#define INK_ACTIONS_DIALOGS_H

#include <glibmm.h>
#include <2geom/point.h>

enum DialogType {
    DIALOG_INVALID,
    DIALOG_ALIGN_DISTRIBUTE,
    DIALOG_ATTR,
    DIALOG_ATTR_XML,
    DIALOG_CLONETILER,
    DIALOG_DEBUG,
    DIALOG_DOCPROPERTIES,
    DIALOG_EXPORT,
    DIALOG_FILL_STROKE,
    DIALOG_FILTER_EFFECTS,
    DIALOG_FIND,
    DIALOG_GLYPHS,
    DIALOG_INPUT,
    DIALOG_ITEM,
    DIALOG_LAYERS,
    DIALOG_LIVE_PATH_EFFECT,
    DIALOG_OBJECTS,
    DIALOG_PAINT,
    DIALOG_PREFERENCES,
    DIALOG_SELECTORS,
    DIALOG_STYLE,
    DIALOG_SVG_FONTS,
    DIALOG_SWATCHES,
    DIALOG_SYMBOLS,
    DIALOG_TEXT,
    DIALOG_TOGGLE,
    DIALOG_TRANSFORM,
    DIALOG_UNDO_HISTORY,
    DIALOG_XML_EDITOR,

    // Put conditional dialogs last so they don't effect numbering of other dialogs.
#if WITH_GSPELL
    DIALOG_SPELLCHECK,
#endif

#ifdef DEBUG
    DIALOG_PROTOTYPE,
#endif
};

class InkscapeWindow;

// Standard function to add actions.
void add_actions_dialogs(InkscapeWindow* win);


#endif // INK_ACTIONS_DIALOGS_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
