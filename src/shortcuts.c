#define __SP_SHORTCUTS_C__

/*
 * Keyboard shortcut processing
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <glib.h>
#include <gdk/gdkkeysyms.h>

#include "verbs.h"

#include "shortcuts.h"

/* Returns TRUE if action was performed */

unsigned int
sp_shortcut_run (unsigned int shortcut)
{
	unsigned int verb;
	verb = sp_shortcut_get_verb (shortcut);
	if (verb) {
		SPAction *action;
		action = sp_verb_get_action (verb);
		if (action) {
			sp_action_perform (action);
			return TRUE;
		}
	}
	return FALSE;
}

void
sp_shortcut_table_load (const unsigned char *name)
{
	/* File */
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_n, SP_VERB_FILE_NEW, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_N, SP_VERB_FILE_NEW, FALSE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_o, SP_VERB_FILE_OPEN, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_O, SP_VERB_FILE_OPEN, FALSE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_s, SP_VERB_FILE_SAVE, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_S, SP_VERB_FILE_SAVE, FALSE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_p, SP_VERB_FILE_PRINT, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_P, SP_VERB_FILE_PRINT, FALSE);
	sp_shortcut_set_verb (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_p, SP_VERB_FILE_PRINT_PREVIEW, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_P, SP_VERB_FILE_PRINT_PREVIEW, FALSE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_e, SP_VERB_FILE_EXPORT, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_E, SP_VERB_FILE_EXPORT, FALSE);
	/* Event contexts */
	sp_shortcut_set_verb (GDK_F1, SP_VERB_CONTEXT_SELECT, TRUE);
	sp_shortcut_set_verb (GDK_F2, SP_VERB_CONTEXT_NODE, TRUE);
	sp_shortcut_set_verb (GDK_F3, SP_VERB_CONTEXT_ZOOM, TRUE);
	sp_shortcut_set_verb (GDK_F4, SP_VERB_CONTEXT_RECT, TRUE);
	sp_shortcut_set_verb (GDK_F5, SP_VERB_CONTEXT_ARC, TRUE);
	sp_shortcut_set_verb (GDK_F6, SP_VERB_CONTEXT_PENCIL, TRUE);
	sp_shortcut_set_verb (GDK_F7, SP_VERB_CONTEXT_TEXT, TRUE);
	sp_shortcut_set_verb (GDK_F8, SP_VERB_CONTEXT_DROPPER, TRUE);
	/* Zooming */
	sp_shortcut_set_verb (GDK_plus, SP_VERB_ZOOM_IN, TRUE);
	sp_shortcut_set_verb (GDK_equal, SP_VERB_ZOOM_IN, FALSE);
	sp_shortcut_set_verb (GDK_KP_Add, SP_VERB_ZOOM_IN, FALSE);
	sp_shortcut_set_verb (GDK_minus, SP_VERB_ZOOM_OUT, TRUE);
	sp_shortcut_set_verb (GDK_KP_Subtract, SP_VERB_ZOOM_OUT, FALSE);
	sp_shortcut_set_verb (GDK_0, SP_VERB_ZOOM_PAGE, TRUE);
	sp_shortcut_set_verb (GDK_KP_0, SP_VERB_ZOOM_PAGE, FALSE);
	sp_shortcut_set_verb (GDK_1, SP_VERB_ZOOM_1_1, TRUE);
	sp_shortcut_set_verb (GDK_KP_1, SP_VERB_ZOOM_1_1, FALSE);
	sp_shortcut_set_verb (GDK_KP_5, SP_VERB_ZOOM_DRAWING, TRUE);
	/* Edit */
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_z, SP_VERB_EDIT_UNDO, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_Z, SP_VERB_EDIT_UNDO, FALSE);
	sp_shortcut_set_verb (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_z, SP_VERB_EDIT_REDO, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_Z, SP_VERB_EDIT_REDO, FALSE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_x, SP_VERB_EDIT_CUT, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_X, SP_VERB_EDIT_CUT, FALSE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_c, SP_VERB_EDIT_COPY, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_C, SP_VERB_EDIT_COPY, FALSE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_v, SP_VERB_EDIT_PASTE, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_V, SP_VERB_EDIT_PASTE, FALSE);
	sp_shortcut_set_verb (GDK_Delete, SP_VERB_EDIT_DELETE, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_d, SP_VERB_EDIT_DUPLICATE, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_D, SP_VERB_EDIT_DUPLICATE, FALSE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_a, SP_VERB_EDIT_SELECT_ALL, TRUE);

	/* Selection */
	sp_shortcut_set_verb (GDK_Home, SP_VERB_SELECTION_TO_FRONT, TRUE);
	sp_shortcut_set_verb (GDK_End, SP_VERB_SELECTION_TO_BACK, TRUE);
	sp_shortcut_set_verb (GDK_Page_Up, SP_VERB_SELECTION_RAISE, TRUE);
	sp_shortcut_set_verb (GDK_Page_Down, SP_VERB_SELECTION_LOWER, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_g, SP_VERB_SELECTION_GROUP, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_G, SP_VERB_SELECTION_GROUP, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_g, SP_VERB_SELECTION_UNGROUP, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_G, SP_VERB_SELECTION_UNGROUP, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_k, SP_VERB_SELECTION_COMBINE, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_CONTROL_MASK | GDK_K, SP_VERB_SELECTION_COMBINE, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_k, SP_VERB_SELECTION_BREAK_APART, TRUE);
	sp_shortcut_set_verb (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_K, SP_VERB_SELECTION_BREAK_APART, TRUE);
}

static GHashTable *scdict = NULL;

void
sp_shortcut_set_verb (unsigned int shortcut, unsigned int verb, unsigned int primary)
{
	unsigned int ex;
	if (!scdict) scdict = g_hash_table_new (NULL, NULL);
	ex = (unsigned int) g_hash_table_lookup (scdict, (void *) shortcut);
	if (ex != verb) g_hash_table_insert (scdict, (void *) shortcut, (void *) verb);
	if (primary) {
		SPAction *action;
		action = sp_verb_get_action (verb);
		if (action && (shortcut != action->shortcut)) {
			sp_action_set_shortcut (action, shortcut);
		}
	}
}

void
sp_shortcut_remove_verb (unsigned int shortcut)
{
	unsigned int ex;
	if (!scdict) return;
	ex = (unsigned int) g_hash_table_lookup (scdict, (void *) shortcut);
	if (ex) {
		SPAction *action;
		g_hash_table_insert (scdict, (void *) shortcut, (void *) 0);
		action = sp_verb_get_action (ex);
		if (action && (action->shortcut)) {
			sp_action_set_shortcut (action, 0);
		}
	}
}

unsigned int
sp_shortcut_get_verb (unsigned int shortcut)
{
	if (!scdict) return 0;
	return (unsigned int) g_hash_table_lookup (scdict, (void *) shortcut);;
}

