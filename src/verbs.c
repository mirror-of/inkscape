#define __SP_VERBS_C__

/*
 * Actions for sodipodi
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <assert.h>

#include <gtk/gtkstock.h>

#include "helper/sp-intl.h"

#include "dialogs/text-edit.h"
#include "dialogs/export.h"
#include "dialogs/xml-tree.h"
#include "dialogs/align.h"
#include "dialogs/transformation.h"
#include "dialogs/object-properties.h"
#include "dialogs/desktop-properties.h"
#include "dialogs/document-properties.h"
#include "dialogs/display-settings.h"
#include "dialogs/tool-options.h"
#include "dialogs/tool-attributes.h"
#include "dialogs/item-properties.h"

#include "select-context.h"
#include "node-context.h"
#include "rect-context.h"
#include "arc-context.h"
#include "star-context.h"
#include "spiral-context.h"
#include "draw-context.h"
#include "dyna-draw-context.h"
#include "text-context.h"
#include "zoom-context.h"
#include "dropper-context.h"

#include "sodipodi-private.h"
#include "file.h"
#include "document.h"
#include "desktop.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "path-chemistry.h"
#include "shortcuts.h"

#include "verbs.h"

static void sp_verbs_init (void);

static SPAction *verb_actions = NULL;

SPAction *
sp_verb_get_action (unsigned int verb)
{
	if (!verb_actions) sp_verbs_init ();
	return &verb_actions[verb];
}

static void
sp_verb_action_set_shortcut (SPAction *action, unsigned int shortcut, void *data)
{
	unsigned int verb, ex;
	verb = (unsigned int) data;
	ex = sp_shortcut_get_verb (shortcut);
	if (verb != ex) sp_shortcut_set_verb (shortcut, verb, FALSE);
}

static void
sp_verb_action_file_perform (SPAction *action, void *data)
{

	switch ((int) data) {
	case SP_VERB_FILE_NEW:
		sp_file_new ();
		break;
	case SP_VERB_FILE_OPEN:
		sp_file_open_dialog (NULL, NULL);
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
	default:
		break;
	}
}

static void
sp_verb_action_edit_perform (SPAction *action, void *data)
{
	SPDesktop *dt;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;

	switch ((int) data) {
	case SP_VERB_EDIT_UNDO:
		sp_document_undo (SP_DT_DOCUMENT (dt));
		break;
	case SP_VERB_EDIT_REDO:
		sp_document_redo (SP_DT_DOCUMENT (dt));
		break;
	case SP_VERB_EDIT_CUT:
		sp_selection_cut (NULL);
		break;
	case SP_VERB_EDIT_COPY:
		sp_selection_copy (NULL);
		break;
	case SP_VERB_EDIT_PASTE:
		sp_selection_paste (NULL);
		break;
	case SP_VERB_EDIT_DELETE:
		sp_selection_delete (NULL, NULL);
		break;
	case SP_VERB_EDIT_DUPLICATE:
		sp_selection_duplicate (NULL, NULL);
		break;
	case SP_VERB_EDIT_CLEAR_ALL:
	  	sp_edit_clear_all (NULL, NULL);
		break;
	case SP_VERB_EDIT_SELECT_ALL:
	  	sp_edit_select_all(NULL, NULL);
		break;
	default:
		break;
	}
}

static void
sp_verb_action_selection_perform (SPAction *action, void *data)
{
	SPDesktop *dt;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;

	switch ((int) data) {
	case SP_VERB_SELECTION_TO_FRONT:
		sp_selection_raise_to_top (NULL);
		break;
	case SP_VERB_SELECTION_TO_BACK:
		sp_selection_lower_to_bottom (NULL);
		break;
	case SP_VERB_SELECTION_RAISE:
		sp_selection_raise (NULL);
		break;
	case SP_VERB_SELECTION_LOWER:
		sp_selection_lower (NULL);
		break;
	case SP_VERB_SELECTION_GROUP:
		sp_selection_group (NULL, NULL);
		break;
	case SP_VERB_SELECTION_UNGROUP:
		sp_selection_ungroup (NULL, NULL);
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
}

static void
sp_verb_action_object_perform (SPAction *action, void *data)
{
	SPDesktop *dt;
	SPSelection *sel;
	NRRectF bbox;
	NRPointF center;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;
	sel = SP_DT_SELECTION (dt);
	if (sp_selection_is_empty (sel)) return;
	sp_selection_bbox (sel, &bbox);
	center.x = 0.5 * (bbox.x0 + bbox.x1);
	center.y = 0.5 * (bbox.y0 + bbox.y1);

	switch ((int) data) {
	case SP_VERB_OBJECT_ROTATE_90:
		sp_selection_rotate_90 ();
		break;
	case SP_VERB_OBJECT_FLATTEN:
		sp_selection_remove_transform ();
		break;
	case SP_VERB_OBJECT_TO_CURVE:
		sp_selected_path_to_curves ();
		break;
	case SP_VERB_OBJECT_FLIP_HORIZONTAL:
		sp_selection_scale_relative (sel, &center, -1.0, 1.0);
		sp_document_done (SP_DT_DOCUMENT (dt));
		break;
	case SP_VERB_OBJECT_FLIP_VERTICAL:
		sp_selection_scale_relative (sel, &center, 1.0, -1.0);
		sp_document_done (SP_DT_DOCUMENT (dt));
		break;
	default:
		break;
	}
}

static void
sp_verb_action_ctx_perform (SPAction *action, void *data)
{
	SPDesktop *dt;
	unsigned int verb;
	int vidx;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;
	verb = (unsigned int) data;

	for (vidx = SP_VERB_CONTEXT_SELECT; vidx <= SP_VERB_CONTEXT_DROPPER; vidx++) {
		sp_action_set_active (&verb_actions[vidx], vidx == verb);
	}

	switch ((int) data) {
	case SP_VERB_CONTEXT_SELECT:
		sp_desktop_set_event_context (dt, SP_TYPE_SELECT_CONTEXT, "tools.select");
		/* fixme: This is really ugly hack. We should bind and unbind class methods */
		sp_desktop_activate_guides (dt, TRUE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_NODE:
		sp_desktop_set_event_context (dt, SP_TYPE_NODE_CONTEXT, "tools.nodes");
		sp_desktop_activate_guides (dt, TRUE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_RECT:
		sp_desktop_set_event_context (dt, SP_TYPE_RECT_CONTEXT, "tools.shapes.rect");
		sp_desktop_activate_guides (dt, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_ARC:
		sp_desktop_set_event_context (dt, SP_TYPE_ARC_CONTEXT, "tools.shapes.arc");
		sp_desktop_activate_guides (dt, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_STAR:
		sp_desktop_set_event_context (dt, SP_TYPE_STAR_CONTEXT, "tools.shapes.star");
		sp_desktop_activate_guides (dt, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_SPIRAL:
		sp_desktop_set_event_context (dt, SP_TYPE_SPIRAL_CONTEXT, "tools.shapes.spiral");
		sp_desktop_activate_guides (dt, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_PENCIL:
		sp_desktop_set_event_context (dt, SP_TYPE_PENCIL_CONTEXT, "tools.freehand.pencil");
		sp_desktop_activate_guides (dt, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_PEN:
		sp_desktop_set_event_context (dt, SP_TYPE_PEN_CONTEXT, "tools.freehand.pen");
		sp_desktop_activate_guides (dt, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_CALLIGRAPHIC:
		sp_desktop_set_event_context (dt, SP_TYPE_DYNA_DRAW_CONTEXT, "tools.calligraphic");
		sp_desktop_activate_guides (dt, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_TEXT:
		sp_desktop_set_event_context (dt, SP_TYPE_TEXT_CONTEXT, "tools.text");
		sp_desktop_activate_guides (dt, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_ZOOM:
		sp_desktop_set_event_context (dt, SP_TYPE_ZOOM_CONTEXT, "tools.zoom");
		sp_desktop_activate_guides (dt, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	case SP_VERB_CONTEXT_DROPPER:
		sp_desktop_set_event_context (dt, SP_TYPE_DROPPER_CONTEXT, "tools.dropper");
		sp_desktop_activate_guides (dt, FALSE);
		sodipodi_eventcontext_set (SP_DT_EVENTCONTEXT (dt));
		break;
	default:
		break;
	}
}

static void
sp_verb_action_zoom_perform (SPAction *action, void *data)
{
	SPDesktop *dt;
	NRRectF d;

	dt = SP_ACTIVE_DESKTOP;
	if (!dt) return;

	switch ((int) data) {
	case SP_VERB_ZOOM_IN:
		sp_desktop_get_display_area (dt, &d);
		sp_desktop_zoom_relative (dt, (d.x0 + d.x1) / 2, (d.y0 + d.y1) / 2, SP_DESKTOP_ZOOM_INC);
		break;
	case SP_VERB_ZOOM_OUT:
		sp_desktop_get_display_area (dt, &d);
		sp_desktop_zoom_relative (dt, (d.x0 + d.x1) / 2, (d.y0 + d.y1) / 2, 1 / SP_DESKTOP_ZOOM_INC);
		break;
	case SP_VERB_ZOOM_1_1:
		sp_desktop_get_display_area (dt, &d);
		sp_desktop_zoom_absolute (dt, (d.x0 + d.x1) / 2, (d.y0 + d.y1) / 2, 1.0);
		break;
	case SP_VERB_ZOOM_1_2:
		sp_desktop_get_display_area (dt, &d);
		sp_desktop_zoom_absolute (dt, (d.x0 + d.x1) / 2, (d.y0 + d.y1) / 2, 0.5);
		break;
	case SP_VERB_ZOOM_2_1:
		sp_desktop_get_display_area (dt, &d);
		sp_desktop_zoom_absolute (dt, (d.x0 + d.x1) / 2, (d.y0 + d.y1) / 2, 2.0);
		break;
	case SP_VERB_ZOOM_PAGE:
		sp_desktop_zoom_page (dt);
		break;
	case SP_VERB_ZOOM_DRAWING:
		sp_desktop_zoom_drawing (dt);
		break;
	case SP_VERB_ZOOM_SELECTION:
		sp_desktop_zoom_selection (dt);
		break;
	default:
		break;
	}
}

static void
sp_verb_action_dialog_perform (SPAction *action, void *data)
{
	switch ((int) data) {
	case SP_VERB_DIALOG_DISPLAY:
		sp_display_dialog ();
		break;
	case SP_VERB_DIALOG_DOCUMENT:
		sp_document_dialog ();
		break;
	case SP_VERB_DIALOG_NAMEDVIEW:
		sp_desktop_dialog ();
		break;
	case SP_VERB_DIALOG_TOOL_OPTIONS:
		sp_tool_options_dialog ();
		break;
	case SP_VERB_DIALOG_TOOL_ATTRIBUTES:
		sp_tool_attributes_dialog ();
		break;
	case SP_VERB_DIALOG_FILL_STROKE:
		sp_object_properties_dialog ();
		break;
	case SP_VERB_DIALOG_SIZE_POSITION:
		sp_object_properties_layout ();
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
	case SP_VERB_DIALOG_ITEM:
		sp_item_dialog ();
		break;
	default:
		break;
	}
}

static SPActionEventVector action_file_vector = {{NULL}, sp_verb_action_file_perform, NULL, NULL, sp_verb_action_set_shortcut};
static SPActionEventVector action_edit_vector = {{NULL}, sp_verb_action_edit_perform, NULL, NULL, sp_verb_action_set_shortcut};
static SPActionEventVector action_selection_vector = {{NULL}, sp_verb_action_selection_perform, NULL, NULL, sp_verb_action_set_shortcut};
static SPActionEventVector action_object_vector = {{NULL}, sp_verb_action_object_perform, NULL, NULL, sp_verb_action_set_shortcut};
static SPActionEventVector action_ctx_vector = {{NULL}, sp_verb_action_ctx_perform, NULL, NULL, sp_verb_action_set_shortcut};
static SPActionEventVector action_zoom_vector = {{NULL}, sp_verb_action_zoom_perform, NULL, NULL, sp_verb_action_set_shortcut};
static SPActionEventVector action_dialog_vector = {{NULL}, sp_verb_action_dialog_perform, NULL, NULL, sp_verb_action_set_shortcut};

#define SP_VERB_IS_FILE(v) ((v >= SP_VERB_FILE_NEW) && (v <= SP_VERB_FILE_EXPORT))
#define SP_VERB_IS_EDIT(v) ((v >= SP_VERB_EDIT_UNDO) && (v <= SP_VERB_EDIT_SELECT_ALL))
#define SP_VERB_IS_SELECTION(v) ((v >= SP_VERB_SELECTION_TO_FRONT) && (v <= SP_VERB_SELECTION_BREAK_APART))
#define SP_VERB_IS_OBJECT(v) ((v >= SP_VERB_OBJECT_ROTATE_90) && (v <= SP_VERB_OBJECT_FLIP_VERTICAL))
#define SP_VERB_IS_CONTEXT(v) ((v >= SP_VERB_CONTEXT_SELECT) && (v <= SP_VERB_CONTEXT_DROPPER))
#define SP_VERB_IS_ZOOM(v) ((v >= SP_VERB_ZOOM_IN) && (v <= SP_VERB_ZOOM_SELECTION))
#define SP_VERB_IS_DIALOG(v) ((v >= SP_VERB_DIALOG_DISPLAY) && (v <= SP_VERB_DIALOG_ITEM))

typedef struct {
	unsigned int code;
	const unsigned char *id;
	const unsigned char *name;
	const unsigned char *tip;
	const unsigned char *image;
} SPVerbActionDef;

static const SPVerbActionDef props[] = {
	/* Header */
	{SP_VERB_INVALID, NULL, NULL, NULL, NULL},
	{SP_VERB_NONE, "None", N_("None"), N_("Does nothing"), NULL},
	/* File */
	{SP_VERB_FILE_NEW, "FileNew", N_("New"), N_("Create new SVG document"), GTK_STOCK_NEW },
	{SP_VERB_FILE_OPEN, "FileOpen", N_("Open..."), N_("Open existing SVG document"), GTK_STOCK_OPEN },
	{SP_VERB_FILE_SAVE, "FileSave", N_("Save"), N_("Save document"), GTK_STOCK_SAVE },
	{SP_VERB_FILE_SAVE_AS, "FileSaveAs", N_("Save As..."), N_("Save document under new name"), GTK_STOCK_SAVE_AS },
	{SP_VERB_FILE_PRINT, "FilePrint", N_("Print..."), N_("Print document"), GTK_STOCK_PRINT },
	{SP_VERB_FILE_PRINT_DIRECT, "FilePrintDirect", N_("Print Direct..."), N_("Print directly to file or pipe"), "file_print_direct" },
	{SP_VERB_FILE_PRINT_PREVIEW, "FilePrintPreview", N_("Print Preview"), N_("Preview document printout"), GTK_STOCK_PRINT_PREVIEW },
	{SP_VERB_FILE_IMPORT, "FileImport", N_("Import"), N_("Import bitmap or SVG image into document"), "file_import"},
	{SP_VERB_FILE_EXPORT, "FileExport", N_("Export"), N_("Export document as PNG bitmap"), "file_export"},
	/* Edit */
	{SP_VERB_EDIT_UNDO, "EditUndo", N_("Undo"), N_("Revert last action"), GTK_STOCK_UNDO},
	{SP_VERB_EDIT_REDO, "EditRedo", N_("Redo"), N_("Do again undone action"), GTK_STOCK_REDO},
	{SP_VERB_EDIT_CUT, "EditCut", N_("Cut"), N_("Cut selected objects to clipboard"), GTK_STOCK_CUT},
	{SP_VERB_EDIT_COPY, "EditCopy", N_("Copy"), N_("Copy selected objects to clipboard"), GTK_STOCK_COPY},
	{SP_VERB_EDIT_PASTE, "EditPaste", N_("Paste"), N_("Paste objects from clipboard"), GTK_STOCK_PASTE},
	{SP_VERB_EDIT_DELETE, "EditDelete", N_("Delete"), N_("Delete selected objects"), GTK_STOCK_DELETE},
	{SP_VERB_EDIT_DUPLICATE, "EditDuplicate", N_("Duplicate"), N_("Duplicate selected objects"), "edit_duplicate"},
	{SP_VERB_EDIT_CLEAR_ALL, "EditClearAll", N_("Clear All"), N_("Delete all objects from document"), NULL},
	{SP_VERB_EDIT_SELECT_ALL, "EditSelectAll", N_("Select All"), N_("Select all objects in document"), NULL},
	/* Selection */
	{SP_VERB_SELECTION_TO_FRONT, "SelectionToFront", N_("Bring to Front"), N_("Raise selected objects to top"), "selection_top"},
	{SP_VERB_SELECTION_TO_BACK, "SelectionToBack", N_("Send to Back"), N_("Lower selected objects to bottom"), "selection_bot"},
	{SP_VERB_SELECTION_RAISE, "SelectionRaise", N_("Raise"), N_("Raise selected objects one position"), "selection_up"},
	{SP_VERB_SELECTION_LOWER, "SelectionLower", N_("Lower"), N_("Lower selected objects one position"), "selection_down"},
	{SP_VERB_SELECTION_GROUP, "SelectionGroup", N_("Group"), N_("Group selected objects"), "selection_group"},
	{SP_VERB_SELECTION_UNGROUP, "SelectionUnGroup", N_("Ungroup"), N_("Ungroup selected group"), "selection_ungroup"},
	{SP_VERB_SELECTION_COMBINE, "SelectionCombine", N_("Combine"), N_("Combine multiple paths"), "selection_combine"},
	{SP_VERB_SELECTION_BREAK_APART, "SelectionBreakApart", N_("Break Apart"), N_("Break selected path to subpaths"), "selection_break"},
	/* Object */
	{SP_VERB_OBJECT_ROTATE_90, "ObjectRotate90", N_("Rotate 90 degrees"), N_("Rotates object 90 degrees clockwise"), "object_rotate"},
	{SP_VERB_OBJECT_FLATTEN, "ObjectFlatten", N_("Flatten object"), N_("Remove transformations from object"), "object_reset"},
	{SP_VERB_OBJECT_TO_CURVE, "ObjectToCurve", N_("Convert to Curves"), N_("Convert selected object to path"), "object_tocurve"},
	{SP_VERB_OBJECT_FLIP_HORIZONTAL, "ObjectFlipHorizontally", N_("Flip Horizontally"),
	 N_("Flip selected objects horizontally"), "object_flip_hor"},
	{SP_VERB_OBJECT_FLIP_VERTICAL, "ObjectFlipVertically", N_("Flip Vertically"),
	 N_("Flip selected objects vertically"), "object_flip_ver"},
	/* Event contexts */
	{SP_VERB_CONTEXT_SELECT, "DrawSelect", N_("Select"), N_("Select and transform objects"), "draw_select"},
	{SP_VERB_CONTEXT_NODE, "DrawNode", N_("Node edit"), N_("Modify existing objects by control nodes"), "draw_node"},
	{SP_VERB_CONTEXT_RECT, "DrawRect", N_("Rectangle"), N_("Create rectangles and squares with optional rounded corners"), "draw_rect"},
	{SP_VERB_CONTEXT_ARC, "DrawArc", N_("Ellipse"), N_("Create circles, ellipses and arcs"), "draw_arc"},
	{SP_VERB_CONTEXT_STAR, "DrawStar", N_("Star"), N_("Create stars and polygons"), "draw_star"},
	{SP_VERB_CONTEXT_SPIRAL, "DrawSpiral", N_("Spiral"), N_("Create spirals"), "draw_spiral"},
	{SP_VERB_CONTEXT_PENCIL, "DrawPencil", N_("Pencil"), N_("Draw freehand curves and straight lines"), "draw_freehand"},
	{SP_VERB_CONTEXT_PEN, "DrawPen", N_("Pen"), N_("Draw precisely positioned curved and straight lines"), "draw_pen"},
	{SP_VERB_CONTEXT_CALLIGRAPHIC, "DrawCalligrphic", N_("Calligraphy"), N_("Draw calligraphic lines"), "draw_dynahand"},
	{SP_VERB_CONTEXT_TEXT, "DrawText", N_("Text"), N_("Create and edit text objects"), "draw_text"},
	{SP_VERB_CONTEXT_ZOOM, "DrawZoom", N_("Zoom"), N_("Zoom into precisely selected area"), "draw_zoom"},
	{SP_VERB_CONTEXT_DROPPER, "DrawDropper", N_("Dropper"), N_("Pick averaged colors from image"), "draw_dropper"},
	/* Zooming */
	{SP_VERB_ZOOM_IN, "ZoomIn", N_("In"), N_("Zoom in drawing"), "zoom_in"},
	{SP_VERB_ZOOM_OUT, "ZoomOut", N_("Out"), N_("Zoom out drawing"), "zoom_out"},
	{SP_VERB_ZOOM_1_1, "Zoom1:0", N_("1:1"), N_("Set zoom factor to 1:1"), "zoom_1_to_1"},
	{SP_VERB_ZOOM_1_2, "Zoom1:2", N_("1:2"), N_("Set zoom factor to 1:2"), "zoom_1_to_2"},
	{SP_VERB_ZOOM_2_1, "Zoom2:1", N_("2:1"), N_("Set zoom factor to 2:1"), "zoom_2_to_1"},
	{SP_VERB_ZOOM_PAGE, "ZoomPage", N_("Page"), N_("Fit the whole page into window"), "zoom_page"},
	{SP_VERB_ZOOM_DRAWING, "ZoomDrawing", N_("Drawing"), N_("Fit the whole drawing into window"), "zoom_draw"},
	{SP_VERB_ZOOM_SELECTION, "ZoomSelection", N_("Selection"), N_("Fit the whole selection into window"), "zoom_select"},
	/* Dialogs */
	{SP_VERB_DIALOG_DISPLAY, "DialogDisplay", N_("Display"), N_("Global display settings"), NULL},
	{SP_VERB_DIALOG_DOCUMENT, "DialogDocument", N_("Document"), N_("Page layout"), NULL},
	{SP_VERB_DIALOG_NAMEDVIEW, "DialogNamedview", N_("Editing Window"), N_("Editing window properties"), NULL},
	{SP_VERB_DIALOG_TOOL_OPTIONS, "DialogToolOptions", N_("Tool Options"), N_("Tool options"), NULL},
	{SP_VERB_DIALOG_TOOL_ATTRIBUTES, "DialogToolAttributes", N_("Tool Attributes"), N_("Tool attributes"), NULL},
	{SP_VERB_DIALOG_FILL_STROKE, "DialogFillStroke", N_("Fill and Stroke"), N_("Fill and stroke settings"), NULL},
	{SP_VERB_DIALOG_SIZE_POSITION, "DialogSizePosition", N_("Size and Position"), N_("Object size and position"), "object_layout"},
	{SP_VERB_DIALOG_TRANSFORM, "DialogTransform", N_("Transformations"), N_("Object transformations"), "object_trans"},
	{SP_VERB_DIALOG_ALIGN_DISTRIBUTE, "DialogAlignDistribute", N_("Align and Distribute"), N_("Align and distribute"), "object_align"},
	{SP_VERB_DIALOG_TEXT, "Dialogtext", N_("Text and Font"), N_("Text editing and font settings"), "object_font"},
	{SP_VERB_DIALOG_XML_EDITOR, "DialogXMLEditor", N_("XML Editor"), N_("XML Editor"), NULL},
	{SP_VERB_DIALOG_ITEM, "DialogItem", N_("Item Properties"), N_("Item properties"), NULL},
	/* Footer */
	{SP_VERB_LAST, NULL, NULL, NULL, NULL}
};

static void
sp_verbs_init (void)
{
	int v;
	verb_actions = nr_new (SPAction, SP_VERB_LAST);
	for (v = 0; v < SP_VERB_LAST; v++) {
		assert (props[v].code == v);
		sp_action_setup (&verb_actions[v], props[v].id, _(props[v].name), _(props[v].tip), props[v].image);
		/* fixme: Make more elegant (Lauris) */
		if (SP_VERB_IS_FILE (v)) {
			nr_active_object_add_listener ((NRActiveObject *) &verb_actions[v],
						       (NRObjectEventVector *) &action_file_vector,
						       sizeof (SPActionEventVector),
						       (void *) v);
		} else if (SP_VERB_IS_EDIT (v)) {
			nr_active_object_add_listener ((NRActiveObject *) &verb_actions[v],
						       (NRObjectEventVector *) &action_edit_vector,
						       sizeof (SPActionEventVector),
						       (void *) v);
		} else if (SP_VERB_IS_SELECTION (v)) {
			nr_active_object_add_listener ((NRActiveObject *) &verb_actions[v],
						       (NRObjectEventVector *) &action_selection_vector,
						       sizeof (SPActionEventVector),
						       (void *) v);
		} else if (SP_VERB_IS_OBJECT (v)) {
			nr_active_object_add_listener ((NRActiveObject *) &verb_actions[v],
						       (NRObjectEventVector *) &action_object_vector,
						       sizeof (SPActionEventVector),
						       (void *) v);
		} else if (SP_VERB_IS_CONTEXT (v)) {
			nr_active_object_add_listener ((NRActiveObject *) &verb_actions[v],
						       (NRObjectEventVector *) &action_ctx_vector,
						       sizeof (SPActionEventVector),
						       (void *) v);
		} else if (SP_VERB_IS_ZOOM (v)) {
			nr_active_object_add_listener ((NRActiveObject *) &verb_actions[v],
						       (NRObjectEventVector *) &action_zoom_vector,
						       sizeof (SPActionEventVector),
						       (void *) v);
		} else if (SP_VERB_IS_DIALOG (v)) {
			nr_active_object_add_listener ((NRActiveObject *) &verb_actions[v],
						       (NRObjectEventVector *) &action_dialog_vector,
						       sizeof (SPActionEventVector),
						       (void *) v);
		}
	}
}
