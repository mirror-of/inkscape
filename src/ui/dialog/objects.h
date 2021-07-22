// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * A simple dialog for objects UI.
 *
 * Authors:
 *   Theodore Janeczko
 *   Tavmjong Bah
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *               Tavmjong Bah 2017
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_OBJECTS_PANEL_H
#define SEEN_OBJECTS_PANEL_H

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>

#include "helper/auto-connection.h"
#include "xml/node-observer.h"

#include "ui/dialog/dialog-base.h"
#include "ui/widget/style-subject.h"

#include "selection.h"
#include "color-rgba.h"

using Inkscape::XML::Node;

class SPObject;
class SPGroup;
struct SPColorSelector;

namespace Inkscape {
namespace UI {
namespace Dialog {

class ObjectsPanel;
class ObjectWatcher;

enum {COL_LABEL, COL_VISIBLE, COL_LOCKED};

using SelectionState = int;
enum SelectionStates : SelectionState {
    SELECTED_NOT = 0,     // Object is NOT in desktop's selection
    SELECTED_OBJECT = 1,  // Object is in the desktop's selection
    LAYER_FOCUSED = 2,    // This layer is the desktop's focused layer
    LAYER_FOCUS_CHILD = 4 // This object is a child of the focused layer
};

/**
 * A panel that displays objects.
 */
class ObjectsPanel : public DialogBase
{
public:
    ObjectsPanel();
    ~ObjectsPanel() override;

    class ModelColumns;
    static ObjectsPanel& getInstance();

protected:

    void desktopReplaced() override;
    void documentReplaced() override;
    void layerChanged(SPObject *obj);
    void selectionChanged(Selection *selected) override;

    // Accessed by ObjectWatcher directly (friend class)
    SPObject* getObject(Node *node);
    ObjectWatcher* getWatcher(Node *node);
    ObjectWatcher *getRootWatcher() const { return root_watcher; };

    Node *getRepr(Gtk::TreeModel::Row const &row) const;
    SPItem *getItem(Gtk::TreeModel::Row const &row) const;
    Gtk::TreeModel::Row *getRow(SPItem *item) const;

    bool isDummy(Gtk::TreeModel::Row const &row) const { return getRepr(row) == nullptr; }
    bool hasDummyChildren(Gtk::TreeModel::Row const &row) const;
    bool removeDummyChildren(Gtk::TreeModel::Row const &row);
    bool cleanDummyChildren(Gtk::TreeModel::Row const &row);

    Glib::RefPtr<Gtk::TreeStore> _store;
    ModelColumns* _model;

private:
    void setRootWatcher();

    ObjectWatcher* root_watcher;
    SPItem *current_item;

    Inkscape::auto_connection layer_changed;
    SPObject *_layer;
    Gtk::TreeModel::RowReference _hovered_row_ref;

    //Show icons in the context menu
    bool _show_contextmenu_icons;
    bool _is_editing;

    std::vector<Gtk::Widget*> _watching;
    std::vector<Gtk::Widget*> _watchingNonTop;
    std::vector<Gtk::Widget*> _watchingNonBottom;

    Gtk::TreeView _tree;
    Gtk::CellRendererText *_text_renderer;
    Gtk::TreeView::Column *_name_column;
    Gtk::Box _buttonsRow;
    Gtk::Box _buttonsPrimary;
    Gtk::Box _buttonsSecondary;
    Gtk::ScrolledWindow _scroller;
    Gtk::Menu _popupMenu;
    Gtk::Box _page;
    Gtk::ToggleButton _object_mode;

    ObjectsPanel(ObjectsPanel const &) = delete; // no copy
    ObjectsPanel &operator=(ObjectsPanel const &) = delete; // no assign

    Gtk::Button *_addBarButton(char const* iconName, char const* tooltip, int verb_id);
    void _fireAction( unsigned int code );
    void _objects_toggle();
    
    void toggleVisible(const Glib::ustring& path);
    void toggleLocked(const Glib::ustring& path);
    
    bool _handleButtonEvent(GdkEventButton *event);
    bool _handleKeyEvent(GdkEventKey *event);
    bool _handleMotionEvent(GdkEventMotion* motion_event);
    
    void _handleEdited(const Glib::ustring& path, const Glib::ustring& new_text);

    void _takeAction(int val);
    
    bool select_row( Glib::RefPtr<Gtk::TreeModel> const & model, Gtk::TreeModel::Path const & path, bool b );

    bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext> &, int, int, guint) override;
    bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext> &, int, int, guint) override;
    void on_drag_start(const Glib::RefPtr<Gdk::DragContext> &);
    void on_drag_end(const Glib::RefPtr<Gdk::DragContext> &) override;

    friend class ObjectWatcher;
};



} //namespace Dialogs
} //namespace UI
} //namespace Inkscape



#endif // SEEN_OBJECTS_PANEL_H

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
