#define __SP_SHORTCUTS_C__

/** \file
 * Keyboard shortcut processing.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *
 * Copyright (C) 2005  Monash University
 *
 * You may redistribute and/or modify this file under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <gdk/gdkkeysyms.h>

#include "helper/action.h"
#include "shortcuts.h"
#include "verbs.h"


/* Returns true if action was performed */

bool
sp_shortcut_invoke(unsigned int shortcut, Inkscape::UI::View::View *view)
{
    Inkscape::Verb *verb = sp_shortcut_get_verb(shortcut);
    if (verb) {
        SPAction *action = verb->get_action(view);
        if (action) {
            sp_action_perform(action, NULL);
            return true;
        }
    }
    return false;
}

static GHashTable *verbs = NULL;
static GHashTable *primary_shortcuts = NULL;

static void
sp_shortcut_init()
{
    verbs = g_hash_table_new(NULL, NULL);
    primary_shortcuts = g_hash_table_new(NULL, NULL);

    struct TblEntry {
        unsigned int shortcut;
        int verb_num;
        bool is_primary;
    } const entries[] = {
        /* File */
        {SP_SHORTCUT_CONTROL_MASK | GDK_n, SP_VERB_FILE_NEW, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_N, SP_VERB_FILE_NEW, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_o, SP_VERB_FILE_OPEN, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_O, SP_VERB_FILE_OPEN, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_s, SP_VERB_FILE_SAVE, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_S, SP_VERB_FILE_SAVE, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_s, SP_VERB_FILE_SAVE_AS, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_S, SP_VERB_FILE_SAVE_AS, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_p, SP_VERB_FILE_PRINT, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_P, SP_VERB_FILE_PRINT, false},

#ifdef HAVE_GTK_WINDOW_FULLSCREEN
        {GDK_F11, SP_VERB_FULLSCREEN, true},
#endif
        /* commented out until implemented */
        /*{SP_SHORTCUT_ALT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_p, SP_VERB_FILE_PRINT_PREVIEW, true},*/
        /*{SP_SHORTCUT_ALT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_P, SP_VERB_FILE_PRINT_PREVIEW, false},*/

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_e, SP_VERB_FILE_EXPORT, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_E, SP_VERB_FILE_EXPORT, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_i, SP_VERB_FILE_IMPORT, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_I, SP_VERB_FILE_IMPORT, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_Tab, SP_VERB_FILE_NEXT_DESKTOP, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_Tab, SP_VERB_FILE_PREV_DESKTOP, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_ISO_Left_Tab, SP_VERB_FILE_PREV_DESKTOP, false},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_ISO_Left_Tab, SP_VERB_FILE_PREV_DESKTOP, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_w, SP_VERB_FILE_CLOSE_VIEW, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_W, SP_VERB_FILE_CLOSE_VIEW, true},

        {SP_SHORTCUT_CONTROL_MASK | GDK_q, SP_VERB_FILE_QUIT, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_Q, SP_VERB_FILE_QUIT, true},

        /* Tools (event contexts) */
        {GDK_F1, SP_VERB_CONTEXT_SELECT, true},
        {GDK_s, SP_VERB_CONTEXT_SELECT, false},
        {GDK_S, SP_VERB_CONTEXT_SELECT, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_F1, SP_VERB_CONTEXT_GRADIENT, true},
        {GDK_g, SP_VERB_CONTEXT_GRADIENT, false},
        {GDK_G, SP_VERB_CONTEXT_GRADIENT, false},

        {GDK_F2, SP_VERB_CONTEXT_NODE, true},
        {GDK_n, SP_VERB_CONTEXT_NODE, false},
        {GDK_N, SP_VERB_CONTEXT_NODE, false},

        {GDK_F3, SP_VERB_CONTEXT_ZOOM, true},
        {GDK_z, SP_VERB_CONTEXT_ZOOM, false},
        {GDK_Z, SP_VERB_CONTEXT_ZOOM, false},

        {GDK_F4, SP_VERB_CONTEXT_RECT, true},
        {GDK_r, SP_VERB_CONTEXT_RECT, false},
        {GDK_R, SP_VERB_CONTEXT_RECT, false},

        {GDK_F5, SP_VERB_CONTEXT_ARC, true},
        {GDK_e, SP_VERB_CONTEXT_ARC, false},
        {GDK_E, SP_VERB_CONTEXT_ARC, false},

        {GDK_F6, SP_VERB_CONTEXT_PENCIL, true},
        {GDK_p, SP_VERB_CONTEXT_PENCIL, false},
        {GDK_P, SP_VERB_CONTEXT_PENCIL, false},

        {SP_SHORTCUT_SHIFT_MASK | GDK_F6, SP_VERB_CONTEXT_PEN, true},
        {GDK_b, SP_VERB_CONTEXT_PEN, false},
        {GDK_B, SP_VERB_CONTEXT_PEN, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_F6, SP_VERB_CONTEXT_CALLIGRAPHIC, true},
        {GDK_c, SP_VERB_CONTEXT_CALLIGRAPHIC, false},
        {GDK_C, SP_VERB_CONTEXT_CALLIGRAPHIC, false},

        {GDK_F7, SP_VERB_CONTEXT_DROPPER, true},
        {GDK_d, SP_VERB_CONTEXT_DROPPER, false},
        {GDK_D, SP_VERB_CONTEXT_DROPPER, false},

        {GDK_F8, SP_VERB_CONTEXT_TEXT, true},
        {GDK_t, SP_VERB_CONTEXT_TEXT, false},
        {GDK_T, SP_VERB_CONTEXT_TEXT, false},

        {GDK_F9, SP_VERB_CONTEXT_SPIRAL, true},
        {GDK_i, SP_VERB_CONTEXT_SPIRAL, false},
        {GDK_I, SP_VERB_CONTEXT_SPIRAL, false},

        {SP_SHORTCUT_SHIFT_MASK | GDK_F9, SP_VERB_CONTEXT_STAR, false},
        {SP_SHORTCUT_SHIFT_MASK | GDK_asterisk, SP_VERB_CONTEXT_STAR, false},
        {GDK_asterisk, SP_VERB_CONTEXT_STAR, false},
        {GDK_KP_Multiply, SP_VERB_CONTEXT_STAR, true},

        {SP_SHORTCUT_CONTROL_MASK | GDK_F2, SP_VERB_CONTEXT_CONNECTOR, true},
        {GDK_o, SP_VERB_CONTEXT_CONNECTOR, false},
        {GDK_O, SP_VERB_CONTEXT_CONNECTOR, false},

        /* Zooming and desktop */
        {GDK_plus, SP_VERB_ZOOM_IN, true},
        {SP_SHORTCUT_SHIFT_MASK | GDK_plus, SP_VERB_ZOOM_IN, false},
        {GDK_equal, SP_VERB_ZOOM_IN, false},
        {GDK_KP_Add, SP_VERB_ZOOM_IN, false},

        {GDK_minus, SP_VERB_ZOOM_OUT, true},
        {GDK_KP_Subtract, SP_VERB_ZOOM_OUT, false},

        {GDK_1, SP_VERB_ZOOM_1_1, true},

        {GDK_grave, SP_VERB_ZOOM_PREV, true},
        {SP_SHORTCUT_SHIFT_MASK | GDK_grave, SP_VERB_ZOOM_NEXT, true},
        {SP_SHORTCUT_SHIFT_MASK | GDK_asciitilde, SP_VERB_ZOOM_NEXT, false},

        // zooming via kp conflicts with shift-kp-arrows
        //{GDK_KP_1, SP_VERB_ZOOM_1_1, false},
        {GDK_2, SP_VERB_ZOOM_1_2, true},
        //{GDK_KP_2, SP_VERB_ZOOM_1_2, false},
        {GDK_3, SP_VERB_ZOOM_SELECTION, true},
        //{GDK_KP_3, SP_VERB_ZOOM_SELECTION, true},
        {GDK_4, SP_VERB_ZOOM_DRAWING, true},
        //{GDK_KP_4, SP_VERB_ZOOM_DRAWING, true},

        {GDK_5, SP_VERB_ZOOM_PAGE, true},
        {GDK_KP_5, SP_VERB_ZOOM_PAGE, false},

        {GDK_6, SP_VERB_ZOOM_PAGE_WIDTH, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_e, SP_VERB_ZOOM_PAGE_WIDTH, false},
        {SP_SHORTCUT_CONTROL_MASK | GDK_E, SP_VERB_ZOOM_PAGE_WIDTH, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_r, SP_VERB_TOGGLE_RULERS, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_R, SP_VERB_TOGGLE_RULERS, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_b, SP_VERB_TOGGLE_SCROLLBARS, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_B, SP_VERB_TOGGLE_SCROLLBARS, false},

        {SP_SHORTCUT_SHIFT_MASK | GDK_bar, SP_VERB_TOGGLE_GUIDES, false},
        {GDK_bar, SP_VERB_TOGGLE_GUIDES, true},

        {SP_SHORTCUT_SHIFT_MASK | GDK_numbersign, SP_VERB_TOGGLE_GRID, false},
        {GDK_numbersign, SP_VERB_TOGGLE_GRID, true},

        /* Edit */
        {SP_SHORTCUT_CONTROL_MASK | GDK_z, SP_VERB_EDIT_UNDO, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_Z, SP_VERB_EDIT_UNDO, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_y, SP_VERB_EDIT_REDO, false},
        {SP_SHORTCUT_CONTROL_MASK | GDK_Y, SP_VERB_EDIT_REDO, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_z, SP_VERB_EDIT_REDO, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_Z, SP_VERB_EDIT_REDO, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_y, SP_VERB_EDIT_UNDO, false},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_Y, SP_VERB_EDIT_UNDO, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_x, SP_VERB_EDIT_CUT, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_X, SP_VERB_EDIT_CUT, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_c, SP_VERB_EDIT_COPY, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_C, SP_VERB_EDIT_COPY, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_v, SP_VERB_EDIT_PASTE, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_V, SP_VERB_EDIT_PASTE, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_v, SP_VERB_EDIT_PASTE_STYLE, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_V, SP_VERB_EDIT_PASTE_STYLE, false},

        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_v, SP_VERB_EDIT_PASTE_IN_PLACE, true},
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_V, SP_VERB_EDIT_PASTE_IN_PLACE, false},

        {GDK_Delete, SP_VERB_EDIT_DELETE, true},
        {GDK_KP_Delete, SP_VERB_EDIT_DELETE, false},
        {GDK_BackSpace, SP_VERB_EDIT_DELETE, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_d, SP_VERB_EDIT_DUPLICATE, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_D, SP_VERB_EDIT_DUPLICATE, false},

        {SP_SHORTCUT_ALT_MASK | GDK_d, SP_VERB_EDIT_CLONE, true},
        {SP_SHORTCUT_ALT_MASK | GDK_D, SP_VERB_EDIT_CLONE, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_ALT_MASK | GDK_d, SP_VERB_EDIT_UNLINK_CLONE, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_ALT_MASK | GDK_D, SP_VERB_EDIT_UNLINK_CLONE, false},

        {SP_SHORTCUT_SHIFT_MASK | GDK_d, SP_VERB_EDIT_CLONE_ORIGINAL, true},
        {SP_SHORTCUT_SHIFT_MASK | GDK_D, SP_VERB_EDIT_CLONE_ORIGINAL, false},

        {SP_SHORTCUT_ALT_MASK | GDK_i, SP_VERB_EDIT_TILE, true},
        {SP_SHORTCUT_ALT_MASK | GDK_I, SP_VERB_EDIT_TILE, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_ALT_MASK | GDK_i, SP_VERB_EDIT_UNTILE, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_ALT_MASK | GDK_I, SP_VERB_EDIT_UNTILE, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_c, SP_VERB_OBJECT_TO_CURVE, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_C, SP_VERB_OBJECT_TO_CURVE, false},

        {SP_SHORTCUT_ALT_MASK | GDK_w, SP_VERB_OBJECT_FLOW_TEXT, true},
        {SP_SHORTCUT_ALT_MASK | GDK_W, SP_VERB_OBJECT_FLOW_TEXT, false},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_ALT_MASK | GDK_w, SP_VERB_OBJECT_UNFLOW_TEXT, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_ALT_MASK | GDK_W, SP_VERB_OBJECT_UNFLOW_TEXT, false},

        {GDK_h, SP_VERB_OBJECT_FLIP_HORIZONTAL, true},
        {GDK_H, SP_VERB_OBJECT_FLIP_HORIZONTAL, false},

        {GDK_v, SP_VERB_OBJECT_FLIP_VERTICAL, true},
        {GDK_V, SP_VERB_OBJECT_FLIP_VERTICAL, false},

        /* Layer */
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_SHIFT_MASK | GDK_Page_Up, SP_VERB_LAYER_RAISE, true},
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_SHIFT_MASK | GDK_KP_Page_Up, SP_VERB_LAYER_RAISE, false},
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_SHIFT_MASK | GDK_Page_Down, SP_VERB_LAYER_LOWER, true},
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_SHIFT_MASK | GDK_KP_Page_Down, SP_VERB_LAYER_LOWER, false},
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_SHIFT_MASK | GDK_Home, SP_VERB_LAYER_TO_TOP, true},
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_SHIFT_MASK | GDK_KP_Home, SP_VERB_LAYER_TO_TOP, false},
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_SHIFT_MASK | GDK_End, SP_VERB_LAYER_TO_BOTTOM, true},
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_SHIFT_MASK | GDK_KP_End, SP_VERB_LAYER_TO_BOTTOM, false},

        {SP_SHORTCUT_SHIFT_MASK | GDK_Page_Up, SP_VERB_LAYER_MOVE_TO_NEXT, true},
        {SP_SHORTCUT_SHIFT_MASK | GDK_KP_Page_Up, SP_VERB_LAYER_MOVE_TO_NEXT, false},
        {SP_SHORTCUT_SHIFT_MASK | GDK_Page_Down, SP_VERB_LAYER_MOVE_TO_PREV, true},
        {SP_SHORTCUT_SHIFT_MASK | GDK_KP_Page_Down, SP_VERB_LAYER_MOVE_TO_PREV, false},

        /* Selection */
        {GDK_Home, SP_VERB_SELECTION_TO_FRONT, true},
        {GDK_KP_Home, SP_VERB_SELECTION_TO_FRONT, false},

        {GDK_End, SP_VERB_SELECTION_TO_BACK, true},
        {GDK_KP_End, SP_VERB_SELECTION_TO_BACK, false},

        {GDK_Page_Up, SP_VERB_SELECTION_RAISE, true},
        {GDK_KP_Page_Up, SP_VERB_SELECTION_RAISE, false},

        {GDK_Page_Down, SP_VERB_SELECTION_LOWER, true},
        {GDK_KP_Page_Down, SP_VERB_SELECTION_LOWER, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_a, SP_VERB_EDIT_SELECT_ALL, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_A, SP_VERB_EDIT_SELECT_ALL, false},

        {SP_SHORTCUT_ALT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_a, SP_VERB_EDIT_SELECT_ALL_IN_ALL_LAYERS, true},
        {SP_SHORTCUT_ALT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_A, SP_VERB_EDIT_SELECT_ALL_IN_ALL_LAYERS, false},

        {GDK_exclam, SP_VERB_EDIT_INVERT, true},
        {SP_SHORTCUT_SHIFT_MASK | GDK_exclam, SP_VERB_EDIT_INVERT, false},

        {SP_SHORTCUT_ALT_MASK | GDK_exclam, SP_VERB_EDIT_INVERT_IN_ALL_LAYERS, true},
        {SP_SHORTCUT_ALT_MASK | SP_SHORTCUT_SHIFT_MASK | GDK_exclam, SP_VERB_EDIT_INVERT_IN_ALL_LAYERS, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_g, SP_VERB_SELECTION_GROUP, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_G, SP_VERB_SELECTION_GROUP, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_u, SP_VERB_SELECTION_UNGROUP, false},
        {SP_SHORTCUT_CONTROL_MASK | GDK_U, SP_VERB_SELECTION_UNGROUP, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_g, SP_VERB_SELECTION_UNGROUP, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_G, SP_VERB_SELECTION_UNGROUP, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_u, SP_VERB_SELECTION_GROUP, false},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_U, SP_VERB_SELECTION_GROUP, false},

        /* Path ops */
        {SP_SHORTCUT_CONTROL_MASK | GDK_plus, SP_VERB_SELECTION_UNION, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_KP_Add, SP_VERB_SELECTION_UNION, false},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_plus, SP_VERB_SELECTION_UNION, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_asterisk, SP_VERB_SELECTION_INTERSECT, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_KP_Multiply, SP_VERB_SELECTION_INTERSECT, false},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_asterisk, SP_VERB_SELECTION_INTERSECT, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_minus, SP_VERB_SELECTION_DIFF, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_underscore, SP_VERB_SELECTION_DIFF, false},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_underscore, SP_VERB_SELECTION_DIFF, false},
        {SP_SHORTCUT_CONTROL_MASK | GDK_KP_Subtract, SP_VERB_SELECTION_DIFF, false},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_minus, SP_VERB_SELECTION_DIFF, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_asciicircum, SP_VERB_SELECTION_SYMDIFF, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_asciicircum, SP_VERB_SELECTION_SYMDIFF, false},

        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_slash, SP_VERB_SELECTION_SLICE, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_slash, SP_VERB_SELECTION_SLICE, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_slash, SP_VERB_SELECTION_CUT, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_slash, SP_VERB_SELECTION_CUT, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_parenright, SP_VERB_SELECTION_OFFSET, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_parenright, SP_VERB_SELECTION_OFFSET, false},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_0, SP_VERB_SELECTION_OFFSET, false},

        {SP_SHORTCUT_ALT_MASK | GDK_parenright, SP_VERB_SELECTION_OFFSET_SCREEN, true},
        {SP_SHORTCUT_ALT_MASK | GDK_0, SP_VERB_SELECTION_OFFSET_SCREEN, false},

        {SP_SHORTCUT_ALT_MASK | SP_SHORTCUT_SHIFT_MASK  | GDK_parenright, SP_VERB_SELECTION_OFFSET_SCREEN_10, true},
        {SP_SHORTCUT_ALT_MASK | SP_SHORTCUT_SHIFT_MASK  | GDK_0, SP_VERB_SELECTION_OFFSET_SCREEN_10, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_parenleft, SP_VERB_SELECTION_INSET, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_parenleft, SP_VERB_SELECTION_INSET, false},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_9, SP_VERB_SELECTION_INSET, false},

        {SP_SHORTCUT_ALT_MASK | GDK_parenleft, SP_VERB_SELECTION_INSET_SCREEN, true},
        {SP_SHORTCUT_ALT_MASK | GDK_9, SP_VERB_SELECTION_INSET_SCREEN, false},

        {SP_SHORTCUT_ALT_MASK | SP_SHORTCUT_SHIFT_MASK  | GDK_parenleft, SP_VERB_SELECTION_INSET_SCREEN_10, true},
        {SP_SHORTCUT_ALT_MASK | SP_SHORTCUT_SHIFT_MASK  | GDK_9, SP_VERB_SELECTION_INSET_SCREEN_10, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_j, SP_VERB_SELECTION_DYNAMIC_OFFSET, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_J, SP_VERB_SELECTION_DYNAMIC_OFFSET, false},

        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_j, SP_VERB_SELECTION_LINKED_OFFSET, true},
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_J, SP_VERB_SELECTION_LINKED_OFFSET, false},

        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_c, SP_VERB_SELECTION_OUTLINE, true},
        {SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK | GDK_C, SP_VERB_SELECTION_OUTLINE, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_l, SP_VERB_SELECTION_SIMPLIFY, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_L, SP_VERB_SELECTION_SIMPLIFY, false},

        {SP_SHORTCUT_ALT_MASK | GDK_b, SP_VERB_SELECTION_CREATE_BITMAP, true},
        {SP_SHORTCUT_ALT_MASK | GDK_B, SP_VERB_SELECTION_CREATE_BITMAP, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_ALT_MASK | GDK_b, SP_VERB_SELECTION_TRACE, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_ALT_MASK | GDK_B, SP_VERB_SELECTION_TRACE, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_k, SP_VERB_SELECTION_COMBINE, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_K, SP_VERB_SELECTION_COMBINE, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_k, SP_VERB_SELECTION_BREAK_APART, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_K, SP_VERB_SELECTION_BREAK_APART, false},

        /* Dialogs */
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_x, SP_VERB_DIALOG_XML_EDITOR, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_X, SP_VERB_DIALOG_XML_EDITOR, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_d, SP_VERB_DIALOG_NAMEDVIEW, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_D, SP_VERB_DIALOG_NAMEDVIEW, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_a, SP_VERB_DIALOG_ALIGN_DISTRIBUTE, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_A, SP_VERB_DIALOG_ALIGN_DISTRIBUTE, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_t, SP_VERB_DIALOG_TEXT, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_T, SP_VERB_DIALOG_TEXT, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_p, SP_VERB_DIALOG_DISPLAY, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_P, SP_VERB_DIALOG_DISPLAY, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_f, SP_VERB_DIALOG_FILL_STROKE, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_F, SP_VERB_DIALOG_FILL_STROKE, false},

        {SP_SHORTCUT_CONTROL_MASK | GDK_f, SP_VERB_DIALOG_FIND, true},
        {SP_SHORTCUT_CONTROL_MASK | GDK_F, SP_VERB_DIALOG_FIND, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_m, SP_VERB_DIALOG_TRANSFORM, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_M, SP_VERB_DIALOG_TRANSFORM, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_o, SP_VERB_DIALOG_ITEM, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_O, SP_VERB_DIALOG_ITEM, false},

        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_w, SP_VERB_DIALOG_SWATCHES, true},
        {SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | GDK_W, SP_VERB_DIALOG_SWATCHES, false},

        {GDK_F12, SP_VERB_DIALOG_TOGGLE, true}
    };

    for (unsigned i = 0; i < G_N_ELEMENTS(entries); ++i) {
        TblEntry const &entry = entries[i];
        sp_shortcut_set(entry.shortcut,
                        Inkscape::Verb::get(entry.verb_num),
                        entry.is_primary);
    }
}

/**
 * Adds a keyboard shortcut for the given verb.
 * (Removes any existing binding for the given shortcut, including appropriately
 * adjusting sp_shortcut_get_primary if necessary.)
 *
 * \param is_primary True iff this is the shortcut to be written in menu items or buttons.
 *
 * \post sp_shortcut_get_verb(shortcut) == verb.
 * \post !is_primary or sp_shortcut_get_primary(verb) == shortcut.
 */
void
sp_shortcut_set(unsigned int const shortcut, Inkscape::Verb *const verb, bool const is_primary)
{
    if (!verbs) sp_shortcut_init();

    Inkscape::Verb *old_verb = (Inkscape::Verb *)(g_hash_table_lookup(verbs, GINT_TO_POINTER(shortcut)));
    g_hash_table_insert(verbs, GINT_TO_POINTER(shortcut), (gpointer)(verb));

    /* Maintain the invariant that sp_shortcut_get_primary(v) returns either 0 or a valid shortcut for v. */
    if (old_verb && old_verb != verb) {
        unsigned int const old_primary = (unsigned int)GPOINTER_TO_INT(g_hash_table_lookup(primary_shortcuts, (gpointer)old_verb));

        if (old_primary == shortcut) {
            g_hash_table_insert(primary_shortcuts, (gpointer)old_verb, GINT_TO_POINTER(0));
        }
    }

    if (is_primary) {
        g_hash_table_insert(primary_shortcuts, (gpointer)(verb), GINT_TO_POINTER(shortcut));
    }
}

void
sp_shortcut_clear(unsigned int shortcut)
{
    if (!verbs) return;

    Inkscape::Verb *verb = (Inkscape::Verb *)(g_hash_table_lookup(verbs, GINT_TO_POINTER(shortcut)));

    if (verb) {
        g_hash_table_remove(verbs, GINT_TO_POINTER(shortcut));
        unsigned int const old_primary
            = (unsigned int)GPOINTER_TO_INT(g_hash_table_lookup(primary_shortcuts,
                                                                (gpointer)(verb)));
        if (old_primary == shortcut) {
            g_hash_table_remove(primary_shortcuts, (gpointer)(verb));
        }
    }
}

Inkscape::Verb *
sp_shortcut_get_verb(unsigned int shortcut)
{
    if (!verbs) sp_shortcut_init();
    return (Inkscape::Verb *)(g_hash_table_lookup(verbs, GINT_TO_POINTER(shortcut)));
}

unsigned int
sp_shortcut_get_primary(Inkscape::Verb *verb)
{
    if (!primary_shortcuts) sp_shortcut_init();
    return (unsigned int)GPOINTER_TO_INT(g_hash_table_lookup(primary_shortcuts,
                                                             (gpointer)(verb)));
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
