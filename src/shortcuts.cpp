#define __SP_SHORTCUTS_C__

/*
 * Keyboard shortcut processing
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * This code is in public domain
 */

#include <glib.h>
#include <gdk/gdkkeysyms.h>

#include "verbs.h"

#include "shortcuts.h"

/* Returns true if action was performed */

bool
sp_shortcut_invoke (unsigned int shortcut, SPView *view)
{
	sp_verb_t verb;
	verb = sp_shortcut_get_verb (shortcut);
	if (verb) {
		SPAction *action;
		action = sp_verb_get_action (verb, view);
		if (action) {
			sp_action_perform (action, NULL);
			return true;
		}
	}
	return false;
}

static GHashTable *verbs = NULL;
static GHashTable *primary_shortcuts = NULL;

static void
sp_shortcut_init ()
{
	verbs = g_hash_table_new (NULL, NULL);
	primary_shortcuts = g_hash_table_new (NULL, NULL);

	/* File */
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_n, SP_VERB_FILE_NEW, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_N, SP_VERB_FILE_NEW, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_o, SP_VERB_FILE_OPEN, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_O, SP_VERB_FILE_OPEN, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_s, SP_VERB_FILE_SAVE, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_S, SP_VERB_FILE_SAVE, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_s, SP_VERB_FILE_SAVE_AS, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_S, SP_VERB_FILE_SAVE_AS, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_p, SP_VERB_FILE_PRINT, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_P, SP_VERB_FILE_PRINT, false);

#ifdef HAVE_GTK_WINDOW_FULLSCREEN
	sp_shortcut_set (GDK_F11, SP_VERB_FULLSCREEN, true);
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */
/* commented out until implemented */
/*	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_p, SP_VERB_FILE_PRINT_PREVIEW, true);*/
/*	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_P, SP_VERB_FILE_PRINT_PREVIEW, false);*/

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_e, SP_VERB_FILE_EXPORT, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_E, SP_VERB_FILE_EXPORT, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_i, SP_VERB_FILE_IMPORT, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_I, SP_VERB_FILE_IMPORT, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_Tab, SP_VERB_FILE_NEXT_DESKTOP, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_Tab, SP_VERB_FILE_PREV_DESKTOP, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_ISO_Left_Tab, SP_VERB_FILE_PREV_DESKTOP, false);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_ISO_Left_Tab, SP_VERB_FILE_PREV_DESKTOP, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_q, SP_VERB_FILE_QUIT, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_Q, SP_VERB_FILE_QUIT, true);

	/* Tools (event contexts) */
	sp_shortcut_set (GDK_F1, SP_VERB_CONTEXT_SELECT, false);
	sp_shortcut_set (GDK_s, SP_VERB_CONTEXT_SELECT, true);
	sp_shortcut_set (GDK_S, SP_VERB_CONTEXT_SELECT, false);

	sp_shortcut_set (GDK_F2, SP_VERB_CONTEXT_NODE, false);
	sp_shortcut_set (GDK_n, SP_VERB_CONTEXT_NODE, true);
	sp_shortcut_set (GDK_N, SP_VERB_CONTEXT_NODE, false);

	sp_shortcut_set (GDK_F3, SP_VERB_CONTEXT_ZOOM, false);
	sp_shortcut_set (GDK_z, SP_VERB_CONTEXT_ZOOM, true);
	sp_shortcut_set (GDK_Z, SP_VERB_CONTEXT_ZOOM, false);

	sp_shortcut_set (GDK_F4, SP_VERB_CONTEXT_RECT, false);
	sp_shortcut_set (GDK_r, SP_VERB_CONTEXT_RECT, true);
	sp_shortcut_set (GDK_R, SP_VERB_CONTEXT_RECT, false);

	sp_shortcut_set (GDK_F5, SP_VERB_CONTEXT_ARC, false);
	sp_shortcut_set (GDK_e, SP_VERB_CONTEXT_ARC, true);
	sp_shortcut_set (GDK_E, SP_VERB_CONTEXT_ARC, false);

	sp_shortcut_set (GDK_F6, SP_VERB_CONTEXT_PENCIL, false);
	sp_shortcut_set (GDK_p, SP_VERB_CONTEXT_PENCIL, true);
	sp_shortcut_set (GDK_P, SP_VERB_CONTEXT_PENCIL, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | GDK_F6, SP_VERB_CONTEXT_PEN, false);
	sp_shortcut_set (GDK_b, SP_VERB_CONTEXT_PEN, true);
	sp_shortcut_set (GDK_B, SP_VERB_CONTEXT_PEN, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_F6, SP_VERB_CONTEXT_CALLIGRAPHIC, true);

	sp_shortcut_set (GDK_F7, SP_VERB_CONTEXT_DROPPER, false);
	sp_shortcut_set (GDK_d, SP_VERB_CONTEXT_DROPPER, true);
	sp_shortcut_set (GDK_D, SP_VERB_CONTEXT_DROPPER, false);

	sp_shortcut_set (GDK_F8, SP_VERB_CONTEXT_TEXT, false);
	sp_shortcut_set (GDK_t, SP_VERB_CONTEXT_TEXT, true);
	sp_shortcut_set (GDK_T, SP_VERB_CONTEXT_TEXT, false);

	sp_shortcut_set (GDK_F9, SP_VERB_CONTEXT_SPIRAL, false);
	sp_shortcut_set (GDK_i, SP_VERB_CONTEXT_SPIRAL, true);
	sp_shortcut_set (GDK_I, SP_VERB_CONTEXT_SPIRAL, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | GDK_F9, SP_VERB_CONTEXT_STAR, false);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | GDK_asterisk, SP_VERB_CONTEXT_STAR, false);
	sp_shortcut_set (GDK_asterisk, SP_VERB_CONTEXT_STAR, false);
	sp_shortcut_set (GDK_KP_Multiply, SP_VERB_CONTEXT_STAR, true);

	/* Zooming and desktop */
	sp_shortcut_set (GDK_plus, SP_VERB_ZOOM_IN, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | GDK_plus, SP_VERB_ZOOM_IN, false);
	sp_shortcut_set (GDK_equal, SP_VERB_ZOOM_IN, false);
	sp_shortcut_set (GDK_KP_Add, SP_VERB_ZOOM_IN, false);

	sp_shortcut_set (GDK_minus, SP_VERB_ZOOM_OUT, true);
	sp_shortcut_set (GDK_KP_Subtract, SP_VERB_ZOOM_OUT, false);

	sp_shortcut_set (GDK_1, SP_VERB_ZOOM_1_1, true);

	sp_shortcut_set (GDK_grave, SP_VERB_ZOOM_PREV, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | GDK_grave, SP_VERB_ZOOM_NEXT, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | GDK_asciitilde, SP_VERB_ZOOM_NEXT, false);

	// zooming via kp conflicts with shift-kp-arrows
	//	sp_shortcut_set (GDK_KP_1, SP_VERB_ZOOM_1_1, false);
	sp_shortcut_set (GDK_2, SP_VERB_ZOOM_1_2, true);
	//	sp_shortcut_set (GDK_KP_2, SP_VERB_ZOOM_1_2, false);
	sp_shortcut_set (GDK_3, SP_VERB_ZOOM_SELECTION, true);
	//	sp_shortcut_set (GDK_KP_3, SP_VERB_ZOOM_SELECTION, true);
	sp_shortcut_set (GDK_4, SP_VERB_ZOOM_DRAWING, true);
	//	sp_shortcut_set (GDK_KP_4, SP_VERB_ZOOM_DRAWING, true);

	sp_shortcut_set (GDK_5, SP_VERB_ZOOM_PAGE, true);
	sp_shortcut_set (GDK_KP_5, SP_VERB_ZOOM_PAGE, false);

	sp_shortcut_set (GDK_6, SP_VERB_ZOOM_PAGE_WIDTH, true);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_r, SP_VERB_TOGGLE_RULERS, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_R, SP_VERB_TOGGLE_RULERS, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_b, SP_VERB_TOGGLE_SCROLLBARS, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_B, SP_VERB_TOGGLE_SCROLLBARS, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | GDK_bar, SP_VERB_TOGGLE_GUIDES, false);
	sp_shortcut_set (GDK_bar, SP_VERB_TOGGLE_GUIDES, true);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | GDK_numbersign, SP_VERB_TOGGLE_GRID, false);
	sp_shortcut_set (GDK_numbersign, SP_VERB_TOGGLE_GRID, true);

	/* Edit */
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_z, SP_VERB_EDIT_UNDO, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_Z, SP_VERB_EDIT_UNDO, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_y, SP_VERB_EDIT_REDO, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_Y, SP_VERB_EDIT_REDO, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_z, SP_VERB_EDIT_REDO, false);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_Z, SP_VERB_EDIT_REDO, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_y, SP_VERB_EDIT_UNDO, false);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_Y, SP_VERB_EDIT_UNDO, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_x, SP_VERB_EDIT_CUT, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_X, SP_VERB_EDIT_CUT, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_c, SP_VERB_EDIT_COPY, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_C, SP_VERB_EDIT_COPY, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_v, SP_VERB_EDIT_PASTE, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_V, SP_VERB_EDIT_PASTE, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_v, SP_VERB_EDIT_PASTE_STYLE, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_V, SP_VERB_EDIT_PASTE_STYLE, false);

	sp_shortcut_set (GDK_Delete, SP_VERB_EDIT_DELETE, true);
	sp_shortcut_set (GDK_KP_Delete, SP_VERB_EDIT_DELETE, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_d, SP_VERB_EDIT_DUPLICATE, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_D, SP_VERB_EDIT_DUPLICATE, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_c, SP_VERB_OBJECT_TO_CURVE, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_C, SP_VERB_OBJECT_TO_CURVE, false);

	sp_shortcut_set (GDK_h, SP_VERB_OBJECT_FLIP_HORIZONTAL, true);
	sp_shortcut_set (GDK_H, SP_VERB_OBJECT_FLIP_HORIZONTAL, false);

	sp_shortcut_set (GDK_v, SP_VERB_OBJECT_FLIP_VERTICAL, true);
	sp_shortcut_set (GDK_V, SP_VERB_OBJECT_FLIP_VERTICAL, false);

	/* Selection */
	sp_shortcut_set (GDK_Home, SP_VERB_SELECTION_TO_FRONT, true);
	sp_shortcut_set (GDK_KP_Home, SP_VERB_SELECTION_TO_FRONT, false);

	sp_shortcut_set (GDK_End, SP_VERB_SELECTION_TO_BACK, true);
	sp_shortcut_set (GDK_KP_End, SP_VERB_SELECTION_TO_BACK, false);

	sp_shortcut_set (GDK_Page_Up, SP_VERB_SELECTION_RAISE, true);
	sp_shortcut_set (GDK_KP_Page_Up, SP_VERB_SELECTION_RAISE, false);

	sp_shortcut_set (GDK_Page_Down, SP_VERB_SELECTION_LOWER, true);
	sp_shortcut_set (GDK_KP_Page_Down, SP_VERB_SELECTION_LOWER, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_a, SP_VERB_EDIT_SELECT_ALL, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_A, SP_VERB_EDIT_SELECT_ALL, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_g, SP_VERB_SELECTION_GROUP, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_G, SP_VERB_SELECTION_GROUP, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_u, SP_VERB_SELECTION_UNGROUP, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_U, SP_VERB_SELECTION_UNGROUP, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_g, SP_VERB_SELECTION_UNGROUP, false);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_G, SP_VERB_SELECTION_UNGROUP, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_u, SP_VERB_SELECTION_GROUP, false);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_U, SP_VERB_SELECTION_GROUP, false);

	/* Path ops */
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_plus, SP_VERB_SELECTION_UNION, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_KP_Add, SP_VERB_SELECTION_UNION, false);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_plus, SP_VERB_SELECTION_UNION, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_asterisk, SP_VERB_SELECTION_INTERSECT, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_KP_Multiply, SP_VERB_SELECTION_INTERSECT, false);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_asterisk, SP_VERB_SELECTION_INTERSECT, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_minus, SP_VERB_SELECTION_DIFF, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_underscore, SP_VERB_SELECTION_DIFF, false);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_underscore, SP_VERB_SELECTION_DIFF, false);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_KP_Subtract, SP_VERB_SELECTION_DIFF, false);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_minus, SP_VERB_SELECTION_DIFF, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_asciicircum, SP_VERB_SELECTION_SYMDIFF, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_asciicircum, SP_VERB_SELECTION_SYMDIFF, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_parenright, SP_VERB_SELECTION_OFFSET, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_parenright, SP_VERB_SELECTION_OFFSET, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_parenleft, SP_VERB_SELECTION_INSET, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_parenleft, SP_VERB_SELECTION_INSET, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_j, SP_VERB_SELECTION_DYNAMIC_OFFSET, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_J, SP_VERB_SELECTION_DYNAMIC_OFFSET, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_j, SP_VERB_SELECTION_LINKED_OFFSET, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_J, SP_VERB_SELECTION_LINKED_OFFSET, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_c, SP_VERB_SELECTION_OUTLINE, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_C, SP_VERB_SELECTION_OUTLINE, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_l, SP_VERB_SELECTION_SIMPLIFY, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_L, SP_VERB_SELECTION_SIMPLIFY, false);

	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_k, SP_VERB_SELECTION_COMBINE, true);
	sp_shortcut_set (SP_SHORTCUT_CONTROL_MASK | GDK_K, SP_VERB_SELECTION_COMBINE, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_k, SP_VERB_SELECTION_BREAK_APART, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_K, SP_VERB_SELECTION_BREAK_APART, false);

	/* Dialogs */
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_x, SP_VERB_DIALOG_XML_EDITOR, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_X, SP_VERB_DIALOG_XML_EDITOR, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_d, SP_VERB_DIALOG_NAMEDVIEW, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_D, SP_VERB_DIALOG_NAMEDVIEW, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_a, SP_VERB_DIALOG_ALIGN_DISTRIBUTE, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_A, SP_VERB_DIALOG_ALIGN_DISTRIBUTE, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_t, SP_VERB_DIALOG_TEXT, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_T, SP_VERB_DIALOG_TEXT, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_o, SP_VERB_DIALOG_DISPLAY, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_O, SP_VERB_DIALOG_DISPLAY, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_f, SP_VERB_DIALOG_FILL_STROKE, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_F, SP_VERB_DIALOG_FILL_STROKE, false);

	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_m, SP_VERB_DIALOG_TRANSFORM, true);
	sp_shortcut_set (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_M, SP_VERB_DIALOG_TRANSFORM, false);

	sp_shortcut_set (GDK_F12, SP_VERB_DIALOG_TOGGLE, true);
}

void
sp_shortcut_set (unsigned int shortcut, sp_verb_t verb, bool is_primary)
{
	sp_verb_t old_verb;

	if (!verbs) sp_shortcut_init();

	old_verb = (sp_verb_t)GPOINTER_TO_INT (g_hash_table_lookup (verbs, GINT_TO_POINTER (shortcut)));
	g_hash_table_insert (verbs, GINT_TO_POINTER (shortcut), GINT_TO_POINTER (verb));

	if (old_verb && old_verb != verb) {
		unsigned int old_primary;

		old_primary = (unsigned int)GPOINTER_TO_INT (g_hash_table_lookup (primary_shortcuts, GINT_TO_POINTER (old_verb)));

		if (old_primary == shortcut) {
			g_hash_table_insert (primary_shortcuts, GINT_TO_POINTER (old_verb), GINT_TO_POINTER (SP_VERB_INVALID));
		}
	}

	if (is_primary) {
		g_hash_table_insert (primary_shortcuts, GINT_TO_POINTER (verb), GINT_TO_POINTER (shortcut));
	}
}

void
sp_shortcut_clear (unsigned int shortcut)
{
	sp_verb_t verb;

	if (!verbs) return;

	verb = (sp_verb_t)GPOINTER_TO_INT (g_hash_table_lookup (verbs, GINT_TO_POINTER (shortcut)));

	if (verb) {
		unsigned int old_primary;
		g_hash_table_remove (verbs, GINT_TO_POINTER (shortcut));
		old_primary = (unsigned int)GPOINTER_TO_INT (g_hash_table_lookup (primary_shortcuts, GINT_TO_POINTER (verb)));
		if (old_primary == shortcut) {
			g_hash_table_remove (primary_shortcuts, GINT_TO_POINTER (verb));
		}
	}
}

sp_verb_t
sp_shortcut_get_verb (unsigned int shortcut)
{
	if (!verbs) sp_shortcut_init();
	return (sp_verb_t)GPOINTER_TO_INT (g_hash_table_lookup (verbs, GINT_TO_POINTER (shortcut)));
}

unsigned int
sp_shortcut_get_primary (sp_verb_t verb)
{
	if (!primary_shortcuts) sp_shortcut_init();
	return (unsigned int)GPOINTER_TO_INT (g_hash_table_lookup (primary_shortcuts, GINT_TO_POINTER (verb)));
}

