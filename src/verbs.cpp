#define __SP_VERBS_C__
/**
 * \file verbs.cpp
 *
 * \brief This file implements routines necessary to deal with verbs.  A verb
 * is a numeric identifier used to retrieve standard SPActions for particular
 * views.
 *
 * Actions for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *   MenTaLguY <mental@rydia.net>
 *
 * This code is in public domain.
 *
 */

#include <assert.h>

#include <gtk/gtkstock.h>

#include <config.h>

#include "helper/sp-intl.h"

#include "dialogs/text-edit.h"
#include "dialogs/export.h"
#include "dialogs/xml-tree.h"
#include "dialogs/align.h"
#include "dialogs/transformation.h"
#include "dialogs/object-properties.h"
#include "dialogs/desktop-properties.h"
#include "dialogs/display-settings.h"
#include "dialogs/item-properties.h"
#include "dialogs/find.h"

#include "tools-switch.h"
#include "inkscape-private.h"
#include "file.h"
#include "help.h"
#include "document.h"
#include "desktop.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "shortcuts.h"
#include "toolbox.h"
#include "view.h"
#include "interface.h"
#include "prefs-utils.h"
#include "splivarot.h"
#include "sp-namedview.h"

#include "select-context.h"
#include "node-context.h"
#include "nodepath.h"
#include "rect-context.h"
#include "arc-context.h"
#include "star-context.h"
#include "spiral-context.h"
#include "draw-context.h"
#include "dyna-draw-context.h"
#include "text-context.h"
#include "zoom-context.h"
#include "dropper-context.h"

#include "verbs.h"

static SPAction *make_action (sp_verb_t verb, SPView *view);

/* FIXME !!! we should probably go ahead and use GHashTables, actually -- more portable */

#if defined(__GNUG__) && (__GNUG__ >= 3)
# include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
# include <hash_map.h>
#endif

#if defined(__GNUG__) && (__GNUG__ >= 3)
namespace __gnu_cxx {
#endif

template <>
class hash<SPView *> {
    typedef SPView *T;
public:
    size_t operator()(const T& x) const {
        return (size_t)g_direct_hash((gpointer)x);
    }
};

#if defined(__GNUG__) && (__GNUG__ >= 3)
}; /* namespace __gnu_cxx */
#endif

typedef hash_map<sp_verb_t, SPAction *> ActionTable;
typedef hash_map<SPView *, ActionTable *> VerbTable;
typedef hash_map<sp_verb_t, SPVerbActionFactory *> FactoryTable;

static VerbTable verb_tables;
static FactoryTable factories;
static sp_verb_t next_verb=SP_VERB_LAST;

/**
 * \return  A pointer to SPAction
 * \brief   Retrieves an SPAction for a particular verb in a given view
 * \param   verb  The verb in question
 * \param   view  The SPView to request an SPAction for
 *
 */
SPAction *
sp_verb_get_action (sp_verb_t verb, SPView * view)
{
    VerbTable::iterator view_found=verb_tables.find(view);
    ActionTable *actions;
    if (view_found != verb_tables.end()) {
        actions = (*view_found).second;
    } else {
        actions = new ActionTable;
        verb_tables.insert(VerbTable::value_type(view, actions));
        /* TODO !!! add SPView::destroy callback to destroy actions
                    and free table when SPView is no more */
    }

    ActionTable::iterator action_found=actions->find(verb);
    if (action_found != actions->end()) {
        return (*action_found).second;
    } else {
        SPAction *action=NULL;
        if (verb < SP_VERB_LAST) {
            action = make_action(verb, view);
        } else {
            FactoryTable::iterator found;
            found = factories.find(verb);
            if (found != factories.end()) {
                action = (*found).second->make_action(verb, view);
            }
        }
        if (action) {
            actions->insert(ActionTable::value_type(verb, action));
            return action;
        } else {
            return NULL;
        }
    }

} // end of sp_verb_get_action()



/**
 * \brief Return the name without underscores and ellipsis, for use in dialog
 * titles, etc. Allocated memory must be freed by caller.
 */
gchar *
sp_action_get_title (const SPAction *action)
{
    char const *src = action->name;
    gchar *ret = g_new (gchar, strlen(src) + 1);
    unsigned ri = 0;

    for (unsigned si = 0 ; ; si++)  {
        int const c = src[si];
        if ( c != '_' && c != '.' ) {
            ret[ri] = c;
            ri++;
            if (c == '\0') {
                return ret;
            }
        }
    }

} // end of sp_action_get_title()



static void
sp_verb_action_file_perform (SPAction *action, void * data, void *pdata)
{
    switch ((int) data) {
        case SP_VERB_FILE_NEW:
            sp_file_new ();
            break;
        case SP_VERB_FILE_OPEN:
            sp_file_open_dialog (NULL, NULL);
            break;
        case SP_VERB_FILE_REVERT:
            sp_file_revert_dialog ();
            break;
        case SP_VERB_FILE_SAVE:
            sp_file_save (NULL, NULL);
            break;
        case SP_VERB_FILE_SAVE_AS:
            sp_file_save_as (NULL, NULL);
            break;
        case SP_VERB_FILE_PRINT:
            sp_file_print ();
            break;
        case SP_VERB_FILE_PRINT_DIRECT:
            sp_file_print_direct ();
            break;
        case SP_VERB_FILE_PRINT_PREVIEW:
            sp_file_print_preview (NULL, NULL);
            break;
        case SP_VERB_FILE_IMPORT:
            sp_file_import (NULL);
            break;
        case SP_VERB_FILE_EXPORT:
            sp_file_export_dialog (NULL);
            break;
        case SP_VERB_FILE_NEXT_DESKTOP:
            inkscape_switch_desktops_next();
            break;
        case SP_VERB_FILE_PREV_DESKTOP:
            inkscape_switch_desktops_prev();
            break;
        case SP_VERB_FILE_CLOSE_VIEW:
            sp_ui_close_view (NULL);
            break;
        case SP_VERB_FILE_QUIT:
            sp_file_exit ();
            break;
        default:
            break;
    }

} // end of sp_verb_action_file_perform()



static void
sp_verb_action_edit_perform (SPAction *action, void * data, void * pdata)
{
    SPDesktop *dt;
    SPEventContext *ec;

    dt = SP_DESKTOP (sp_action_get_view (action));
    if (!dt)
        return;

    ec = dt->event_context;

    switch ((int) data) {
        case SP_VERB_EDIT_UNDO:
            sp_undo (dt, SP_DT_DOCUMENT (dt));
            break;
        case SP_VERB_EDIT_REDO:
            sp_redo (dt, SP_DT_DOCUMENT (dt));
            break;
        case SP_VERB_EDIT_CUT:
            sp_selection_cut();
            break;
        case SP_VERB_EDIT_COPY:
            sp_selection_copy();
            break;
        case SP_VERB_EDIT_PASTE:
            sp_selection_paste(false);
            break;
        case SP_VERB_EDIT_PASTE_STYLE:
            sp_selection_paste_style();
            break;
        case SP_VERB_EDIT_PASTE_IN_PLACE:
            sp_selection_paste(true);
            break;
        case SP_VERB_EDIT_DELETE:
            sp_selection_delete();
            break;
        case SP_VERB_EDIT_DUPLICATE:
		sp_selection_duplicate();
            break;
        case SP_VERB_EDIT_CLONE:
            sp_selection_clone();
            break;
        case SP_VERB_EDIT_UNLINK_CLONE:
            sp_selection_unlink();
            break;
        case SP_VERB_EDIT_CLONE_ORIGINAL:
            sp_select_clone_original();
            break;
        case SP_VERB_EDIT_TILE:
            sp_selection_tile();
            break;
        case SP_VERB_EDIT_UNTILE:
		sp_selection_untile();
            break;
        case SP_VERB_EDIT_CLEAR_ALL:
            sp_edit_clear_all();
            break;
        case SP_VERB_EDIT_SELECT_ALL:
            if (tools_isactive (dt, TOOLS_NODES)) {
                sp_nodepath_select_all (SP_NODE_CONTEXT(ec)->nodepath);
            } else {
                sp_edit_select_all();
            }
            break;
        case SP_VERB_EDIT_DESELECT:
            if (tools_isactive (dt, TOOLS_NODES)) {
                sp_nodepath_deselect (SP_NODE_CONTEXT(ec)->nodepath);
            } else {
                SP_DT_SELECTION(dt)->clear();
            }
            break;
        default:
            break;
    }

} // end of sp_verb_action_edit_perform()



static void
sp_verb_action_selection_perform (SPAction *action, void * data, void * pdata)
{
    SPDesktop *dt;

    dt = SP_DESKTOP (sp_action_get_view (action));

    if (!dt)
        return;

    switch ((int) data) {
        case SP_VERB_SELECTION_TO_FRONT:
            sp_selection_raise_to_top();
            break;
        case SP_VERB_SELECTION_TO_BACK:
            sp_selection_lower_to_bottom();
            break;
        case SP_VERB_SELECTION_RAISE:
            sp_selection_raise();
            break;
        case SP_VERB_SELECTION_LOWER:
            sp_selection_lower();
            break;
        case SP_VERB_SELECTION_GROUP:
            sp_selection_group();
            break;
        case SP_VERB_SELECTION_UNGROUP:
            sp_selection_ungroup();
            break;

        case SP_VERB_SELECTION_UNION:
            sp_selected_path_union ();
            break;
        case SP_VERB_SELECTION_INTERSECT:
            sp_selected_path_intersect ();
            break;
        case SP_VERB_SELECTION_DIFF:
            sp_selected_path_diff ();
            break;
        case SP_VERB_SELECTION_SYMDIFF:
            sp_selected_path_symdiff ();
            break;

        case SP_VERB_SELECTION_CUT:
            sp_selected_path_cut ();
            break;
        case SP_VERB_SELECTION_SLICE:
            sp_selected_path_slice ();
            break;

        case SP_VERB_SELECTION_OFFSET:
            sp_selected_path_offset ();
            break;
        case SP_VERB_SELECTION_OFFSET_SCREEN:
            sp_selected_path_offset_screen (1);
            break;
        case SP_VERB_SELECTION_OFFSET_SCREEN_10:
            sp_selected_path_offset_screen (10);
            break;
        case SP_VERB_SELECTION_INSET:
            sp_selected_path_inset ();
            break;
        case SP_VERB_SELECTION_INSET_SCREEN:
            sp_selected_path_inset_screen (1);
            break;
        case SP_VERB_SELECTION_INSET_SCREEN_10:
            sp_selected_path_inset_screen (10);
            break;
        case SP_VERB_SELECTION_DYNAMIC_OFFSET:
            sp_selected_path_create_offset_object_zero ();
            break;
        case SP_VERB_SELECTION_LINKED_OFFSET:
            sp_selected_path_create_updating_offset_object_zero ();
            break;

        case SP_VERB_SELECTION_OUTLINE:
            sp_selected_path_outline ();
            break;
        case SP_VERB_SELECTION_SIMPLIFY:
            sp_selected_path_simplify ();
            break;
        case SP_VERB_SELECTION_CLEANUP:
            sp_selection_cleanup ();
            break;
        case SP_VERB_SELECTION_REVERSE:
            sp_selected_path_reverse ();
            break;

        case SP_VERB_SELECTION_COMBINE:
            sp_selected_path_combine ();
            break;
        case SP_VERB_SELECTION_BREAK_APART:
            sp_selected_path_break_apart ();
            break;
        default:
            break;
    }

} // end of sp_verb_action_selection_perform()



static void sp_verb_action_object_perform ( SPAction *action, void *data,
                                            void *pdata )
{
    SPDesktop *dt = SP_DESKTOP(sp_action_get_view(action));

    if (!dt)
        return;

    SPSelection *sel = SP_DT_SELECTION(dt);

    if (sel->isEmpty())
        return;

    NR::Point const center(sel->bounds().midpoint());

    switch ((int) data) {
        case SP_VERB_OBJECT_ROTATE_90_CW:
            sp_selection_rotate_90_cw ();
            break;
        case SP_VERB_OBJECT_ROTATE_90_CCW:
            sp_selection_rotate_90_ccw ();
            break;
        case SP_VERB_OBJECT_FLATTEN:
            sp_selection_remove_transform ();
            break;
        case SP_VERB_OBJECT_TO_CURVE:
            sp_selected_path_to_curves ();
            break;
        case SP_VERB_OBJECT_FLIP_HORIZONTAL:
            // TODO: make tool-sensitive, in node edit flip selected node(s)
            sp_selection_scale_relative(sel, center, NR::scale(-1.0, 1.0));
            sp_document_done (SP_DT_DOCUMENT (dt));
            break;
        case SP_VERB_OBJECT_FLIP_VERTICAL:
            // TODO: make tool-sensitive, in node edit flip selected node(s)
            sp_selection_scale_relative(sel, center, NR::scale(1.0, -1.0));
            sp_document_done (SP_DT_DOCUMENT (dt));
            break;
        default:
            break;
    }

} // end of sp_verb_action_object_perform()



static void
sp_verb_action_ctx_perform (SPAction *action, void * data, void * pdata)
{
    SPDesktop *dt;
    sp_verb_t verb;
    int vidx;

    dt = SP_DESKTOP (sp_action_get_view (action));

    if (!dt)
        return;

    verb = (sp_verb_t)GPOINTER_TO_INT((gpointer)data);

    /* TODO !!! hopefully this can go away soon and actions can look after
     * themselves
     */
    for (vidx = SP_VERB_CONTEXT_SELECT; vidx <= SP_VERB_CONTEXT_DROPPER; vidx++)
    {
        SPAction *tool_action=sp_verb_get_action((sp_verb_t)vidx, SP_VIEW (dt));
        if (tool_action) {
            sp_action_set_active (tool_action, vidx == (int)verb);
        }
    }

    switch (verb) {
        case SP_VERB_CONTEXT_SELECT:
            tools_switch_current (TOOLS_SELECT);
            break;
        case SP_VERB_CONTEXT_NODE:
            tools_switch_current (TOOLS_NODES);
            break;
        case SP_VERB_CONTEXT_RECT:
            tools_switch_current (TOOLS_SHAPES_RECT);
            break;
        case SP_VERB_CONTEXT_ARC:
            tools_switch_current (TOOLS_SHAPES_ARC);
            break;
        case SP_VERB_CONTEXT_STAR:
            tools_switch_current (TOOLS_SHAPES_STAR);
            break;
        case SP_VERB_CONTEXT_SPIRAL:
            tools_switch_current (TOOLS_SHAPES_SPIRAL);
            break;
        case SP_VERB_CONTEXT_PENCIL:
            tools_switch_current (TOOLS_FREEHAND_PENCIL);
            break;
        case SP_VERB_CONTEXT_PEN:
            tools_switch_current (TOOLS_FREEHAND_PEN);
            break;
        case SP_VERB_CONTEXT_CALLIGRAPHIC:
            tools_switch_current (TOOLS_CALLIGRAPHIC);
            break;
        case SP_VERB_CONTEXT_TEXT:
            tools_switch_current (TOOLS_TEXT);
            break;
        case SP_VERB_CONTEXT_ZOOM:
            tools_switch_current (TOOLS_ZOOM);
            break;
        case SP_VERB_CONTEXT_DROPPER:
            tools_switch_current (TOOLS_DROPPER);
            break;
        default:
            break;
    }

} // end of sp_verb_action_ctx_perform()



static void
sp_verb_action_zoom_perform (SPAction *action, void * data, void * pdata)
{
    SPDesktop *dt;
    NRRect d;
    SPRepr *repr;

    dt = SP_DESKTOP (sp_action_get_view (action));

    if (!dt)
        return;

    repr = SP_OBJECT_REPR (dt->namedview);

    gdouble zoom_inc =
        prefs_get_double_attribute_limited ( "options.zoomincrement",
                                             "value", 1.414213562, 1.01, 10 );

    switch ((int) data) {
        case SP_VERB_ZOOM_IN:
            sp_desktop_get_display_area (dt, &d);
            sp_desktop_zoom_relative ( dt, (d.x0 + d.x1) / 2,
                                       (d.y0 + d.y1) / 2, zoom_inc );
            break;
        case SP_VERB_ZOOM_OUT:
            sp_desktop_get_display_area (dt, &d);
            sp_desktop_zoom_relative ( dt, (d.x0 + d.x1) / 2,
                                       (d.y0 + d.y1) / 2, 1 / zoom_inc );
            break;
        case SP_VERB_ZOOM_1_1:
            sp_desktop_get_display_area (dt, &d);
            sp_desktop_zoom_absolute ( dt, (d.x0 + d.x1) / 2,
                                       (d.y0 + d.y1) / 2, 1.0 );
            break;
        case SP_VERB_ZOOM_1_2:
            sp_desktop_get_display_area (dt, &d);
            sp_desktop_zoom_absolute ( dt, (d.x0 + d.x1) / 2,
                                       (d.y0 + d.y1) / 2, 0.5);
            break;
        case SP_VERB_ZOOM_2_1:
            sp_desktop_get_display_area (dt, &d);
            sp_desktop_zoom_absolute ( dt, (d.x0 + d.x1) / 2,
                                       (d.y0 + d.y1) / 2, 2.0 );
            break;
        case SP_VERB_ZOOM_PAGE:
            sp_desktop_zoom_page (dt);
            break;
        case SP_VERB_ZOOM_PAGE_WIDTH:
            sp_desktop_zoom_page_width (dt);
            break;
        case SP_VERB_ZOOM_DRAWING:
            sp_desktop_zoom_drawing (dt);
            break;
        case SP_VERB_ZOOM_SELECTION:
            sp_desktop_zoom_selection (dt);
            break;
        case SP_VERB_ZOOM_NEXT:
            sp_desktop_next_zoom (dt);
            break;
        case SP_VERB_ZOOM_PREV:
            sp_desktop_prev_zoom (dt);
            break;
        case SP_VERB_TOGGLE_RULERS:
            sp_desktop_toggle_rulers (dt);
            break;
        case SP_VERB_TOGGLE_SCROLLBARS:
            sp_desktop_toggle_scrollbars (dt);
            break;
        case SP_VERB_TOGGLE_GUIDES:
            sp_namedview_toggle_guides (repr);
            break;
        case SP_VERB_TOGGLE_GRID:
            sp_namedview_toggle_grid (repr);
            break;
#ifdef HAVE_GTK_WINDOW_FULLSCREEN
        case SP_VERB_FULLSCREEN:
            fullscreen (dt);
            break;
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */
        case SP_VERB_VIEW_NEW:
            sp_ui_new_view ();
            break;
        case SP_VERB_VIEW_NEW_PREVIEW:
            sp_ui_new_view_preview ();
            break;
        default:
            break;
    }

} // end of sp_verb_action_zoom_perform()



static void
sp_verb_action_dialog_perform (SPAction *action, void * data, void * pdata)
{
    if ((int) data != SP_VERB_DIALOG_TOGGLE) {
        // unhide all when opening a new dialog
        inkscape_dialogs_unhide ();
    }

    switch ((int) data) {
        case SP_VERB_DIALOG_DISPLAY:
            sp_display_dialog ();
            break;
        case SP_VERB_DIALOG_NAMEDVIEW:
            sp_desktop_dialog ();
            break;
        case SP_VERB_DIALOG_FILL_STROKE:
            sp_object_properties_dialog ();
            break;
        case SP_VERB_DIALOG_TRANSFORM:
            sp_transformation_dialog_move ();
            break;
        case SP_VERB_DIALOG_ALIGN_DISTRIBUTE:
            sp_quick_align_dialog ();
            break;
        case SP_VERB_DIALOG_TEXT:
            sp_text_edit_dialog ();
            break;
        case SP_VERB_DIALOG_XML_EDITOR:
            sp_xml_tree_dialog ();
            break;
        case SP_VERB_DIALOG_FIND:
            sp_find_dialog ();
            break;
        case SP_VERB_DIALOG_TOGGLE:
            inkscape_dialogs_toggle ();
            break;
        case SP_VERB_DIALOG_ITEM:
            sp_item_dialog ();
            break;
        default:
            break;
    }

} // end of sp_verb_action_dialog_perform()


static void
sp_verb_action_help_perform (SPAction *action, void * data, void * pdata)
{
    switch ((int) data) {
        case SP_VERB_HELP_KEYS:
            sp_help_open_screen (_("keys.svg"));
            break;
        case SP_VERB_HELP_ABOUT:
            sp_help_about ();
            break;
	default:
	    break;
    }
} // end of sp_verb_action_help_perform()

static void
sp_verb_action_tutorial_perform (SPAction *action, void * data, void * pdata)
{
    switch ((int) data) {
        case SP_VERB_TUTORIAL_BASIC:
            sp_help_open_tutorial (NULL, (gpointer)_("tutorial-basic.svg"));
            break;
        case SP_VERB_TUTORIAL_ADVANCED:
            sp_help_open_tutorial (NULL, (gpointer)_("tutorial-advanced.svg"));
            break;
        case SP_VERB_TUTORIAL_DESIGN:
            sp_help_open_tutorial (NULL, (gpointer)_("elementsofdesign.svg"));
            break;
        case SP_VERB_TUTORIAL_TIPS:
            sp_help_open_tutorial (NULL, (gpointer)_("tipsandtricks.svg"));
            break;
	default:
	    break;
    }
} // end of sp_verb_action_tutorial_perform()


/**
 * Action vector to define functions called if a staticly defined file verb
 * is called.
 */
static SPActionEventVector action_file_vector =
            {{NULL},sp_verb_action_file_perform, NULL, NULL, NULL};
/**
 * Action vector to define functions called if a staticly defined edit verb is
 * called.
 */
static SPActionEventVector action_edit_vector =
            {{NULL}, sp_verb_action_edit_perform, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined selection
 * verb is called
 */
static SPActionEventVector action_selection_vector =
            {{NULL}, sp_verb_action_selection_perform, NULL, NULL, NULL};
/**
 * Action vector to define functions called if a staticly defined object
 * editing verb is called
 */
static SPActionEventVector action_object_vector =
            {{NULL}, sp_verb_action_object_perform, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined context
 * verb is called
 */
static SPActionEventVector action_ctx_vector =
            {{NULL}, sp_verb_action_ctx_perform, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined zoom verb
 * is called
 */
static SPActionEventVector action_zoom_vector =
            {{NULL}, sp_verb_action_zoom_perform, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined dialog verb
 * is called
 */
static SPActionEventVector action_dialog_vector =
            {{NULL}, sp_verb_action_dialog_perform, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined help verb
 * is called
 */
static SPActionEventVector action_help_vector =
            {{NULL}, sp_verb_action_help_perform, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined tutorial verb
 * is called
 */
static SPActionEventVector action_tutorial_vector =
            {{NULL}, sp_verb_action_tutorial_perform, NULL, NULL, NULL};

#define SP_VERB_IS_FILE(v) ((v >= SP_VERB_FILE_NEW) && (v <= SP_VERB_FILE_QUIT))
#define SP_VERB_IS_EDIT(v) ((v >= SP_VERB_EDIT_UNDO) && (v <= SP_VERB_EDIT_DESELECT))
#define SP_VERB_IS_SELECTION(v) ((v >= SP_VERB_SELECTION_TO_FRONT) && (v <= SP_VERB_SELECTION_BREAK_APART))
#define SP_VERB_IS_OBJECT(v) ((v >= SP_VERB_OBJECT_ROTATE_90_CW) && (v <= SP_VERB_OBJECT_FLIP_VERTICAL))
#define SP_VERB_IS_CONTEXT(v) ((v >= SP_VERB_CONTEXT_SELECT) && (v <= SP_VERB_CONTEXT_DROPPER))
#define SP_VERB_IS_ZOOM(v) ((v >= SP_VERB_ZOOM_IN) && (v <= SP_VERB_ZOOM_SELECTION))

#define SP_VERB_IS_DIALOG(v) ((v >= SP_VERB_DIALOG_DISPLAY) && (v <= SP_VERB_DIALOG_ITEM))
#define SP_VERB_IS_HELP(v) ((v >= SP_VERB_HELP_KEYS) && (v <= SP_VERB_HELP_ABOUT))
#define SP_VERB_IS_TUTORIAL(v) ((v >= SP_VERB_TUTORIAL_BASIC) && (v <= SP_VERB_TUTORIAL_TIPS))


/**
 * A structure to hold information about a verb
 */
typedef struct {
    sp_verb_t code;      /**< Verb number (staticly from enum) */
    const gchar *id;     /**< Textual identifier for the verb */
    const gchar *name;   /**< Name of the verb */
    const gchar *tip;    /**< Tooltip to print on hover */
    const gchar *image;  /**< Image to describe the verb */
} SPVerbActionDef;


/* these must be in the same order as the SP_VERB_* enum in "verbs.h" */
static const SPVerbActionDef props[] = {
    /* Header */
    {SP_VERB_INVALID, NULL, NULL, NULL, NULL},
    {SP_VERB_NONE, "None", N_("None"), N_("Does nothing"), NULL},

    /* File */
    {SP_VERB_FILE_NEW, "FileNew", N_("_New"), N_("Create new document"),
        GTK_STOCK_NEW },
    {SP_VERB_FILE_OPEN, "FileOpen", N_("_Open..."),
	N_("Open existing document"), GTK_STOCK_OPEN },
    {SP_VERB_FILE_REVERT, "FileRevert", N_("Re_vert"),
	N_("Revert to the last saved version of document (changes will be lost)"), "file_revert" },
    {SP_VERB_FILE_SAVE, "FileSave", N_("_Save"), N_("Save document"),
        GTK_STOCK_SAVE },
    {SP_VERB_FILE_SAVE_AS, "FileSaveAs", N_("Save _As..."),
        N_("Save document under new name"), GTK_STOCK_SAVE_AS },
    {SP_VERB_FILE_PRINT, "FilePrint", N_("_Print..."), N_("Print document"),
        GTK_STOCK_PRINT },
    {SP_VERB_FILE_PRINT_DIRECT, "FilePrintDirect", N_("Print _Direct"),
        N_("Print directly to file or pipe"), NULL },
    {SP_VERB_FILE_PRINT_PREVIEW, "FilePrintPreview", N_("Print Previe_w"),
        N_("Preview document printout"), GTK_STOCK_PRINT_PREVIEW },
    {SP_VERB_FILE_IMPORT, "FileImport", N_("_Import..."),
        N_("Import bitmap or SVG image into document"), "file_import"},
    {SP_VERB_FILE_EXPORT, "FileExport", N_("_Export Bitmap..."),
        N_("Export document or selection as PNG bitmap"), "file_export"},
    {SP_VERB_FILE_NEXT_DESKTOP, "FileNextDesktop", N_("N_ext Window"),
        N_("Switch to the next document window"), "window_next"},
    {SP_VERB_FILE_PREV_DESKTOP, "FilePrevDesktop", N_("P_revious Window"),
        N_("Switch to the previous document window"), "window_previous"},
    {SP_VERB_FILE_CLOSE_VIEW, "FileCloseView", N_("_Close"),
	N_("Close window"), GTK_STOCK_CLOSE},
    {SP_VERB_FILE_QUIT, "FileQuit", N_("_Quit"), N_("Quit Inkscape"), GTK_STOCK_QUIT},

    /* Edit */
    {SP_VERB_EDIT_UNDO, "EditUndo", N_("_Undo"), N_("Undo last action"),
        GTK_STOCK_UNDO},
    {SP_VERB_EDIT_REDO, "EditRedo", N_("_Redo"),
        N_("Do again last undone action"), GTK_STOCK_REDO},
    {SP_VERB_EDIT_CUT, "EditCut", N_("Cu_t"),
        N_("Cut selection to clipboard"), GTK_STOCK_CUT},
    {SP_VERB_EDIT_COPY, "EditCopy", N_("_Copy"),
        N_("Copy selection to clipboard"), GTK_STOCK_COPY},
    {SP_VERB_EDIT_PASTE, "EditPaste", N_("_Paste"),
        N_("Paste object(s) from clipboard to mouse point"), GTK_STOCK_PASTE},
    {SP_VERB_EDIT_PASTE_STYLE, "EditPasteStyle", N_("Paste _Style"),
        N_("Apply style of the copied object to selection"), "selection_paste_style"},
    {SP_VERB_EDIT_PASTE_IN_PLACE, "EditPasteInPlace", N_("Paste _In Place"),
        N_("Paste object(s) from clipboard to the original location"), "selection_paste_in_place"},
    {SP_VERB_EDIT_DELETE, "EditDelete", N_("_Delete"),
        N_("Delete selection"), GTK_STOCK_DELETE},
    {SP_VERB_EDIT_DUPLICATE, "EditDuplicate", N_("Duplic_ate"),
        N_("Duplicate selected object(s)"), "edit_duplicate"},
    {SP_VERB_EDIT_CLONE, "EditClone", N_("Clo_ne"),
        N_("Create a clone of selected object (a copy linked to the original)"), NULL},
    {SP_VERB_EDIT_UNLINK_CLONE, "EditUnlinkClone", N_("Unlin_k Clone"),
        N_("Cut the clone's link to its original"), NULL},
    {SP_VERB_EDIT_CLONE_ORIGINAL, "EditCloneOriginal", N_("Select _Original"),
        N_("Select the object to which the clone is linked"), NULL},
    {SP_VERB_EDIT_TILE, "EditTile", N_("_Tile"),
        N_("Convert selection to a rectangle with tiled pattern fill"), NULL},
    {SP_VERB_EDIT_UNTILE, "EditUnTile", N_("_Untile"),
        N_("Extract objects from a tiled pattern fill"), NULL},
    {SP_VERB_EDIT_CLEAR_ALL, "EditClearAll", N_("Clea_r All"),
        N_("Delete all objects from document"), NULL},
    {SP_VERB_EDIT_SELECT_ALL, "EditSelectAll", N_("Select Al_l"),
        N_("Select all objects or all nodes"), "selection_select_all"},
    {SP_VERB_EDIT_DESELECT, "EditDeselect", N_("D_eselect"),
        N_("Deselect any selected objects or nodes"), "selection_deselect"},

    /* Selection */
    {SP_VERB_SELECTION_TO_FRONT, "SelectionToFront", N_("Raise to _Top"),
        N_("Raise selection to top"), "selection_top"},
    {SP_VERB_SELECTION_TO_BACK, "SelectionToBack", N_("Lower to _Bottom"),
        N_("Lower selection to bottom"), "selection_bot"},
    {SP_VERB_SELECTION_RAISE, "SelectionRaise", N_("_Raise"),
        N_("Raise selection one step"), "selection_up"},
    {SP_VERB_SELECTION_LOWER, "SelectionLower", N_("_Lower"),
        N_("Lower selection one step"), "selection_down"},
    {SP_VERB_SELECTION_GROUP, "SelectionGroup", N_("_Group"),
        N_("Group selected objects"), "selection_group"},
    {SP_VERB_SELECTION_UNGROUP, "SelectionUnGroup", N_("_Ungroup"),
        N_("Ungroup selected group(s)"), "selection_ungroup"},

    {SP_VERB_SELECTION_UNION, "SelectionUnion", N_("_Union"),
        N_("Union of selected objects"), "union"},
    {SP_VERB_SELECTION_INTERSECT, "SelectionIntersect", N_("_Intersection"),
        N_("Intersection of selected objects"), "intersection"},
    {SP_VERB_SELECTION_DIFF, "SelectionDiff", N_("_Difference"),
        N_("Difference of selected objects (bottom minus top)"), "difference"},
    {SP_VERB_SELECTION_SYMDIFF, "SelectionSymDiff", N_("E_xclusion"),
        N_("Exclusive OR of selected objects"), "exclusion"},
    {SP_VERB_SELECTION_CUT, "SelectionDivide", N_("Di_vision"),
        N_("Cut the bottom object into pieces"), "division"},
    {SP_VERB_SELECTION_SLICE, "SelectionCutPath", N_("Cut _Path"),
        N_("Cut the bottom object's stroke into pieces, removing fill"), "cut_path"},
    // TRANSLATORS: "outset": expand a shape by offsetting the object's path,
    // i.e. by displacing it perpendicular to the path in each point.
    // See also the Advanced Tutorial for explanation.
    {SP_VERB_SELECTION_OFFSET, "SelectionOffset", N_("Ou_tset"),
        N_("Outset selected path(s)"), "outset_path"},
    {SP_VERB_SELECTION_OFFSET_SCREEN, "SelectionOffsetScreen", 
        N_("O_utset Path by 1px"),
        N_("Outset selected path(s) by 1px"), NULL},
    {SP_VERB_SELECTION_OFFSET_SCREEN_10, "SelectionOffsetScreen10", 
        N_("O_utset Path by 10px"),
        N_("Outset selected path(s) by 10px"), NULL},
    // TRANSLATORS: "inset": contract a shape by offsetting the object's path,
    // i.e. by displacing it perpendicular to the path in each point.
    // See also the Advanced Tutorial for explanation.
    {SP_VERB_SELECTION_INSET, "SelectionInset", N_("I_nset"),
        N_("Inset selected path(s)"), "inset_path"},
    {SP_VERB_SELECTION_INSET_SCREEN, "SelectionInsetScreen", 
        N_("I_nset Path by 1px"),
        N_("Inset selected path(s) by 1px"), NULL},
    {SP_VERB_SELECTION_INSET_SCREEN_10, "SelectionInsetScreen", 
        N_("I_nset Path by 10px"),
        N_("Inset selected path(s) by 10px"), NULL},
    {SP_VERB_SELECTION_DYNAMIC_OFFSET, "SelectionDynOffset",
        N_("D_ynamic Offset"), N_("Create a dynamic offset object"), "dynamic_offset"},
    {SP_VERB_SELECTION_LINKED_OFFSET, "SelectionLinkedOffset",
        N_("_Linked Offset"),
        N_("Create a dynamic offset object linked to the original path"),
        "linked_offset"},
    {SP_VERB_SELECTION_OUTLINE, "SelectionOutline", N_("_Stroke to Path"),
        N_("Convert selected stroke(s) to path(s)"), "stroke_tocurve"},
    {SP_VERB_SELECTION_SIMPLIFY, "SelectionSimplify", N_("Si_mplify"),
        N_("Simplify selected path(s) by removing extra nodes"), "simplify"},
    {SP_VERB_SELECTION_CLEANUP, "SelectionCleanup", N_("Cl_eanup"),
        N_("Clean up selected path(s)"), "selection_cleanup"},
    {SP_VERB_SELECTION_REVERSE, "SelectionReverse", N_("_Reverse"),
        N_("Reverses the direction of selected path(s); useful for flipping markers"), NULL},
    {SP_VERB_SELECTION_COMBINE, "SelectionCombine", N_("_Combine"),
        N_("Combine several paths into one"), "selection_combine"},
    {SP_VERB_SELECTION_BREAK_APART, "SelectionBreakApart", N_("Break _Apart"),
        N_("Break selected path(s) into subpaths"), "selection_break"},

    /* Object */
    {SP_VERB_OBJECT_ROTATE_90_CW, "ObjectRotate90", N_("Rotate _90 deg CW"),
        N_("Rotate selection 90 degrees clockwise"), "object_rotate_90_CW"},
    {SP_VERB_OBJECT_ROTATE_90_CCW, "ObjectRotate90CCW", N_("Rotate 9_0 deg CCW"),
        N_("Rotate selection 90 degrees counter-clockwise"), "object_rotate_90_CCW"},
    {SP_VERB_OBJECT_FLATTEN, "ObjectFlatten", N_("Remove _Transformations"),
        N_("Remove transformations from object"), "object_reset"},
    {SP_VERB_OBJECT_TO_CURVE, "ObjectToCurve", N_("_Object to Path"),
        N_("Convert selected object(s) to path(s)"), "object_tocurve"},
    {SP_VERB_OBJECT_FLIP_HORIZONTAL, "ObjectFlipHorizontally",
        N_("Flip _Horizontally"), N_("Flip selection horizontally"),
        "object_flip_hor"},
    {SP_VERB_OBJECT_FLIP_VERTICAL, "ObjectFlipVertically",
        N_("Flip _Vertically"), N_("Flip selection vertically"),
        "object_flip_ver"},

    /* Event contexts */
    // TODO: add shortcuts to tooltips automatically!
    {SP_VERB_CONTEXT_SELECT, "DrawSelect", N_("Select"),
        N_("Select and transform objects (F1)"), "draw_select"},
    {SP_VERB_CONTEXT_NODE, "DrawNode", N_("Node Edit"),
        N_("Edit path nodes or control handles (F2)"), "draw_node"},
    {SP_VERB_CONTEXT_RECT, "DrawRect", N_("Rectangle"),
        N_("Create rectangles and squares (F4)"), "draw_rect"},
    {SP_VERB_CONTEXT_ARC, "DrawArc", N_("Ellipse"),
        N_("Create circles, ellipses, and arcs (F5)"), "draw_arc"},
    {SP_VERB_CONTEXT_STAR, "DrawStar", N_("Star"),
        N_("Create stars and polygons (*)"), "draw_star"},
    {SP_VERB_CONTEXT_SPIRAL, "DrawSpiral", N_("Spiral"),
        N_("Create spirals (F9)"), "draw_spiral"},
    {SP_VERB_CONTEXT_PENCIL, "DrawPencil", N_("Pencil"),
        N_("Draw freehand lines (F6)"), "draw_freehand"},
    {SP_VERB_CONTEXT_PEN, "DrawPen", N_("Pen"),
        N_("Draw Bezier curves and straight lines (Shift+F6)"), "draw_pen"},
    {SP_VERB_CONTEXT_CALLIGRAPHIC, "DrawCalligrphic", N_("Calligraphy"),
        N_("Draw calligraphic lines (Ctrl+F6)"), "draw_dynahand"},
    {SP_VERB_CONTEXT_TEXT, "DrawText", N_("Text"),
        N_("Create and edit text objects (F8)"), "draw_text"},
    {SP_VERB_CONTEXT_ZOOM, "DrawZoom", N_("Zoom"),
        N_("Zoom in or out (F3)"), "draw_zoom"},
    {SP_VERB_CONTEXT_DROPPER, "DrawDropper", N_("Dropper"),
        N_("Pick averaged colors from image (F7)"), "draw_dropper"},

    /* Zooming */
    {SP_VERB_ZOOM_IN, "ZoomIn", N_("Zoom In"), N_("Zoom in"), "zoom_in"},
    {SP_VERB_ZOOM_OUT, "ZoomOut", N_("Zoom Out"), N_("Zoom out"), "zoom_out"},
    {SP_VERB_TOGGLE_RULERS, "ToggleRulers", N_("_Rulers"), N_("Show or hide the canvas rulers"), "rulers"},
    {SP_VERB_TOGGLE_SCROLLBARS, "ToggleScrollbars", N_("Scroll_bars"), N_("Show or hide the canvas scrollbars"), "scrollbars"},
    {SP_VERB_TOGGLE_GRID, "ToggleGrid", N_("_Grid"), N_("Show or hide grid"), "grid"},
    {SP_VERB_TOGGLE_GUIDES, "ToggleGuides", N_("G_uides"), N_("Show or hide guides"), "guides"},
    {SP_VERB_ZOOM_NEXT, "ZoomNext", N_("Nex_t Zoom"), N_("Next zoom (from the history of zooms)"),
        "zoom_next"},
    {SP_VERB_ZOOM_PREV, "ZoomPrev", N_("Pre_vious Zoom"), N_("Previous zoom (from the history of zooms)"),
        "zoom_previous"},
    {SP_VERB_ZOOM_1_1, "Zoom1:0", N_("Zoom 1:_1"), N_("Zoom to 1:1"),
        "zoom_1_to_1"},
    {SP_VERB_ZOOM_1_2, "Zoom1:2", N_("Zoom 1:_2"), N_("Zoom to 1:2"),
        "zoom_1_to_2"},
    {SP_VERB_ZOOM_2_1, "Zoom2:1", N_("_Zoom 2:1"), N_("Zoom to 2:1"),
        "zoom_2_to_1"},
#ifdef HAVE_GTK_WINDOW_FULLSCREEN
    {SP_VERB_FULLSCREEN, "FullScreen", N_("_Fullscreen"), N_("Stretch this document window to full screen"),
        "fullscreen"},
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */
    {SP_VERB_VIEW_NEW, "ViewNew", N_("D_uplicate Window"), N_("Open a new window with the same document"),
        "view_new"},
    {SP_VERB_VIEW_NEW_PREVIEW, "ViewNewPreview", N_("_New View Preview"),
	N_("New View Preview"), NULL/*"view_new_preview"*/},
    {SP_VERB_ZOOM_PAGE, "ZoomPage", N_("_Page"), 
       N_("Zoom to fit page in window"), "zoom_page"},
    {SP_VERB_ZOOM_PAGE_WIDTH, "ZoomPageWidth", N_("Page _Width"),
        N_("Zoom to fit page width in window"), "zoom_pagewidth"},
    {SP_VERB_ZOOM_DRAWING, "ZoomDrawing", N_("_Drawing"),
        N_("Zoom to fit drawing in window"), "zoom_draw"},
    {SP_VERB_ZOOM_SELECTION, "ZoomSelection", N_("_Selection"),
        N_("Zoom to fit selection in window"), "zoom_select"},
    /* Dialogs */
    {SP_VERB_DIALOG_DISPLAY, "DialogDisplay", N_("In_kscape Preferences..."),
        N_("Global Inkscape preferences"), "inkscape_options"},
    {SP_VERB_DIALOG_NAMEDVIEW, "DialogNamedview", N_("_Document Preferences..."),
        N_("Preferences saved with the document"), "document_options"},
    {SP_VERB_DIALOG_FILL_STROKE, "DialogFillStroke", N_("_Fill and Stroke..."),
        N_("Fill and Stroke dialog"), "fill_and_stroke"},
    {SP_VERB_DIALOG_TRANSFORM, "DialogTransform", N_("Transfor_m..."),
        N_("Transform dialog"), "object_trans"},
    {SP_VERB_DIALOG_ALIGN_DISTRIBUTE, "DialogAlignDistribute", N_("_Align and Distribute..."), 
        N_("Align and Distribute dialog"), "object_align"},
    {SP_VERB_DIALOG_TEXT, "Dialogtext", N_("Text and _Font..."),
        N_("Text and Font dialog"), "object_font"},
    {SP_VERB_DIALOG_XML_EDITOR, "DialogXMLEditor", N_("_XML Editor..."),
        N_("XML Editor"), "xml_editor"},
    {SP_VERB_DIALOG_FIND, "DialogFind", N_("_Find..."),
        N_("Find objects in document"), NULL},
    {SP_VERB_DIALOG_TOGGLE, "DialogsToggle", N_("Show/_Hide Dialogs"),
        N_("Show or hide all active dialogs"), "dialog_toggle"},
    {SP_VERB_DIALOG_ITEM, "DialogItem", N_("_Object Properties..."),
        N_("Object Properties dialog"), "dialog_item_properties"},

    /* Help */
    {SP_VERB_HELP_KEYS, "HelpKeys", N_("_Keys and Mouse"),
        N_("Key and mouse shortcuts reference"), "help_keys"},
    {SP_VERB_HELP_ABOUT, "HelpAbout", N_("_About Inkscape"),
        N_("About Inkscape"), /*"help_about"*/"inkscape_options"},

    /* Tutorials */
    {SP_VERB_TUTORIAL_BASIC, "TutorialsBasic", N_("Inkscape: _Basic"),
        N_("Basic Inkscape tutorial"), NULL/*"tutorial_basic"*/},
    {SP_VERB_TUTORIAL_ADVANCED, "TutorialsAdvanced", N_("Inkscape: _Advanced"),
        N_("Advanced Inkscape tutorial"), NULL/*"tutorial_advanced"*/},
    {SP_VERB_TUTORIAL_DESIGN, "TutorialsDesign", N_("_Elements of Design"),
        N_("Elements of Design tutorial"), NULL/*"tutorial_design"*/},
    {SP_VERB_TUTORIAL_TIPS, "TutorialsTips", N_("_Tips and Tricks"),
        N_("Miscellaneous Tips and Tricks"), NULL/*"tutorial_tips"*/},

    /* Footer */
    {SP_VERB_LAST, NULL, NULL, NULL, NULL}
};


/*
 * \return  numerical verb id.  SP_VERB_INVALID if no such verb exists
 * \brief   Looks up the numerical ID of the given verb name for future
 *          external extension system that won't exactly know what the
 *          verb enums are.
 * \param   name   Name of the verb to look up ("FileSave" etc)
 */
sp_verb_t
sp_verb_find (gchar const * name) {
    SPVerbActionDef const * list=NULL;
    for (list = props; list->code != SP_VERB_LAST; list++) {
        if (!list->name) continue;
        if (!strcmp(name,list->name)) break;
    }
    if (list->code == SP_VERB_LAST) return SP_VERB_INVALID;
    return list->code;
}

static SPAction *
make_action (sp_verb_t verb, SPView *view)
{
    assert (props[verb].code == verb);
    SPAction *action=sp_action_new(view, props[verb].id, _(props[verb].name),
                                   _(props[verb].tip), props[verb].image);

    /* TODO: Make more elegant (Lauris) */
    if (SP_VERB_IS_FILE (verb)) {
        nr_active_object_add_listener ((NRActiveObject *) action,
                        (NRObjectEventVector *) &action_file_vector,
                        sizeof (SPActionEventVector),
                        (void *) verb);
    } else if (SP_VERB_IS_EDIT (verb)) {
        nr_active_object_add_listener ((NRActiveObject *) action,
                        (NRObjectEventVector *) &action_edit_vector,
                        sizeof (SPActionEventVector),
                        (void *) verb);
    } else if (SP_VERB_IS_SELECTION (verb)) {
        nr_active_object_add_listener ((NRActiveObject *) action,
                        (NRObjectEventVector *) &action_selection_vector,
                        sizeof (SPActionEventVector),
                        (void *) verb);
    } else if (SP_VERB_IS_OBJECT (verb)) {
        nr_active_object_add_listener ((NRActiveObject *) action,
                        (NRObjectEventVector *) &action_object_vector,
                        sizeof (SPActionEventVector),
                        (void *) verb);
    } else if (SP_VERB_IS_CONTEXT (verb)) {
        nr_active_object_add_listener ((NRActiveObject *) action,
                        (NRObjectEventVector *) &action_ctx_vector,
                        sizeof (SPActionEventVector),
                        (void *) verb);
    } else if (SP_VERB_IS_ZOOM (verb)) {
        nr_active_object_add_listener ((NRActiveObject *) action,
                        (NRObjectEventVector *) &action_zoom_vector,
                        sizeof (SPActionEventVector),
                        (void *) verb);
    } else if (SP_VERB_IS_DIALOG (verb)) {
        nr_active_object_add_listener ((NRActiveObject *) action,
                        (NRObjectEventVector *) &action_dialog_vector,
                        sizeof (SPActionEventVector),
                        (void *) verb);
    } else if (SP_VERB_IS_HELP (verb)) {
        nr_active_object_add_listener ((NRActiveObject *) action,
                        (NRObjectEventVector *) &action_help_vector,
                        sizeof (SPActionEventVector),
                        (void *) verb);
    } else if (SP_VERB_IS_TUTORIAL (verb)) {
        nr_active_object_add_listener ((NRActiveObject *) action,
                        (NRObjectEventVector *) &action_tutorial_vector,
                        sizeof (SPActionEventVector),
                        (void *) verb);
    }
    return action;

} // end of make_action()



sp_verb_t
sp_verb_register (SPVerbActionFactory *factory)
{
    sp_verb_t verb=next_verb++;
    factories.insert(FactoryTable::value_type(verb, factory));
    return verb;

}

