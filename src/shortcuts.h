#ifndef __SP_SHORTCUTS_H__
#define __SP_SHORTCUTS_H__

/*
 * Keyboard shortcut processing
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <verbs.h>

/* We define high-bit mask for packing into single int */

#define SP_SHORTCUT_SHIFT_MASK (1 << 24)
#define SP_SHORTCUT_CONTROL_MASK (1 << 25)
#define SP_SHORTCUT_ALT_MASK (1 << 26)

/* Returns true if action was performed */
bool sp_shortcut_invoke (unsigned int shortcut, SPView *view);

void sp_shortcut_set (unsigned int shortcut, sp_verb_t verb, bool is_primary);
void sp_shortcut_clear (unsigned int shortcut);
sp_verb_t sp_shortcut_get_verb (unsigned int shortcut);
unsigned int sp_shortcut_get_primary (sp_verb_t verb);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
