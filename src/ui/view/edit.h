/**
 * \brief This class implements the functionality of the window layout, menus,
 *        and signals.
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_VIEW_EDIT_H
#define INKSCAPE_UI_VIEW_EDIT_H

#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/entry.h>
#include <gtkmm/ruler.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/drawingarea.h>  // TODO: remove this when SVG Canvas is hooked in

#include "libnr/nr-matrix.h"
#include "ui/view/view.h"

#include "ui/dialog/dialog-manager.h"
#include "ui/widget/toolbox.h"

struct SPCanvas;
struct SPCanvasItem;
struct SPCanvasGroup;
struct SPCanvasArena;
struct SPCSSAttr;
struct SPDesktopWidget;
struct SPEventContext;
struct SPNamedView;
struct NRArenaItem;


namespace Inkscape {
class Selection;

namespace UI {
namespace View {

class Edit : public Gtk::Window, public View {
public:
    Edit();
    ~Edit();

    // Initialization
    void initActions();
    void initUIManager();
    void initLayout();

    // Actions
    void onActionFileNew();
    void onActionFileOpen();
    void onActionFilePrint();
    void onActionFileQuit();
    void onToolbarItem();
    void onSelectTool();
    void onNodeTool();

    // Menus
    void onMenuItem();

    void onDialogAbout();
    void onDialogAlignAndDistribute();
    void onDialogInkscapePreferences();
    void onDialogDialog();
    void onDialogDocumentProperties();
    void onDialogExport();
    void onDialogExtensionEditor();
    void onDialogFillAndStroke();
    void onDialogFind();
    void onDialogLayerEditor();
    void onDialogMessages();
    void onDialogObjectProperties();
    void onDialogTextProperties();
    void onDialogTransform();
    void onDialogTransformation();
    void onDialogTrace();
    void onDialogXmlEditor();

	// Whiteboard (Inkboard)
#ifdef WITH_INKBOARD
	void onDialogWhiteboardConnect();
	void onDialogWhiteboardShareWithUser();
	void onDialogWhiteboardShareWithChat();
	void onDialogOpenSessionFile();
	void onDumpXMLTracker();
#endif

    void onUriChanged();

    // from View
    virtual void mouseover() {}
    virtual void mouseout() {}
    virtual bool shutdown() { return true; }
    virtual void setDoc(SPDocument*) {}

    virtual void onPositionSet (double, double) {}
    virtual void onResized (double, double) {}
    virtual void onRedrawRequested() {}
    virtual void onStatusMessage (Inkscape::MessageType type, gchar const *message) {}
    virtual void onDocumentURISet (gchar const* uri) {}
    virtual void onDocumentResized (double, double) {}


    SPEventContext       *event_context;

protected:
    SPDesktopWidget      *_owner;
    SPNamedView          *_namedview;
    SPDocument           *_document;
    Inkscape::Selection  *_selection;
    sigc::connection     _sel_modified_connection;
    sigc::connection     _sel_changed_connection;

    unsigned int         _dkey;

    SPCanvasItem         *_acetate;
    SPCanvasGroup        *_main;
    SPCanvasGroup        *_grid;
    SPCanvasGroup        *_guides;
    SPCanvasItem         *_drawing;
    SPCanvasGroup        *_sketch;
    SPCanvasGroup        *_controls;

    SPCanvasItem         *_table; // outside-of-page background
    SPCanvasItem         *_page; // page background
    SPCanvasItem         *_page_border; // page border

    NR::Matrix           _d2w, _w2d, _doc2dt;
    gint                 _number;
    bool             _is_fullscreen;

    // current style
    SPCSSAttr            *_current_style;

    // Child widgets:
    Gtk::Table           _main_window_table;
    Gtk::VBox            _toolbars_vbox;
    Gtk::HBox            _sub_window_hbox;
    Gtk::Table           _viewport_table;

    UI::Widget::Toolbox  *_tool_ctrl;
    Gtk::Toolbar         *_select_ctrl;
    Gtk::Toolbar         *_uri_ctrl;
    Gtk::Label           _uri_label;
    Gtk::Entry           _uri_entry;
    Gtk::Toolbar         *_node_ctrl;

    Gtk::HRuler          _top_ruler;
    Gtk::VRuler          _left_ruler;
    Gtk::HScrollbar      _bottom_scrollbar;
    Gtk::VScrollbar      _right_scrollbar;
    Gtk::Widget*         _svg_canvas;
    Gtk::Statusbar       _statusbar;

    Glib::RefPtr<Gtk::ActionGroup>  _act_grp;
    Glib::RefPtr<Gtk::UIManager>    _ui_mgr;
    UI::Dialog::DialogManager       _dlg_mgr;

    void initMenuActions();
    void initToolbarActions();
    void initAccelMap();
    void initMenuBar();
    void initCommandsBar();
    void initToolControlsBar();
    void initUriBar();
    void initToolsBar();
    void initBottomScrollbar();
    void initRightScrollbar();
    void initLeftRuler();
    void initTopRuler();
    void initSvgCanvas();
    void initStatusbar();
};
/** Edit Handlers
 *
 *  These routines implement a C-style interface for signal handlers.
 *  This is mostly for backwards compatibility with the Gtkm code;
 *  ideally, these would all be implemented in Inkscape::UI::View::Edit
 *  directly.
 *
 */

gint editor_enter_notify(GtkWidget *widget, GdkEventCrossing *event);
gint editor_canvas_enter_notify(GtkWidget *widget, GdkEventCrossing *event);
gint editor_canvas_leave_notify(GtkWidget *widget, GdkEventCrossing *event);
gint editor_canvas_motion_notify(GtkWidget *widget, GdkEventCrossing *event);

void editor_namedview_modified(SPNamedView *nv, guint flags, Edit *editor);

} // namespace View
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_VIEW_EDIT_H

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
