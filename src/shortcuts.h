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

/* We define high-bit mask for packing into single int */

#define SP_SHORTCUT_SHIFT_MASK (1 << 24)
#define SP_SHORTCUT_CONTROL_MASK (1 << 25)
#define SP_SHORTCUT_ALT_MASK (1 << 26)

/* Returns TRUE is action was performed */

unsigned int sp_shortcut_run (unsigned int shortcut);

/* Loads named shortcut table */

void sp_shortcut_table_load (const unsigned char *name);

void sp_shortcut_set_verb (unsigned int shortcut, unsigned int verb, unsigned int primary);
void sp_shortcut_remove_verb (unsigned int shortcut);
unsigned int sp_shortcut_get_verb (unsigned int shortcut);

#endif
