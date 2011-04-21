/** @file
 * @brief Gallery dialog
 */
/* Authors:
 *     Andrew Higginson
 * 
 * Copyright (C) 2011
 * 
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_GALLERY_H
#define INKSCAPE_UI_DIALOG_GALLERY_H

#include <glibmm.h>
#include <gtkmm.h>

//Gtk includes
#include <glibmm/i18n.h>
#include <glib/gstdio.h>

#include "ui/widget/panel.h"
#include "extension/db.h"
#include "extension/input.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns();
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > thumbnail;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn< Glib::RefPtr<Gio::File> > file;
};

class Gallery : public UI::Widget::Panel {
    public:
        Gallery();
        ~Gallery();
    
        static Gallery &getInstance() { return *new Gallery(); }
    
    private:
        int THUMBNAIL_SIZE;
        bool has_fallback_icon;
        bool updating_model;
    
        Gtk::FileChooserButton *filechooserbutton;
        Gtk::TreeView* treeview;
        Glib::RefPtr<Gtk::ListStore> model;
        ModelColumns columns;
        Gtk::Button *button_import;

        Glib::RefPtr<Gio::FileMonitor> directory_monitor;
    
        std::map<Glib::ustring, Inkscape::Extension::Input*> extension_input_map;
    
        void on_filechooserbutton_current_folder_changed();
        void on_directory_enumerated(const Glib::RefPtr<Gio::AsyncResult>& result,
            Glib::RefPtr<Gio::File> directory);
        void on_enumerator_file_ready(const Glib::RefPtr<Gio::AsyncResult>& result,
            Glib::RefPtr<Gio::FileEnumerator> enumerator, Glib::RefPtr<Gio::File> directory);
        void on_enumerator_closed(const Glib::RefPtr<Gio::AsyncResult>& result,
            Glib::RefPtr<Gio::FileEnumerator> enumerator);
        void update_treeview(Glib::ustring directory_path, bool update_monitor = true);
        void update_directory_monitor(Glib::RefPtr<Gio::File> directory);
        void on_treeview_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
        void on_button_import_clicked();
        void on_treeview_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&,
            Gtk::SelectionData& selection_data, guint target_id, guint etime);
        Glib::RefPtr<Gdk::Pixbuf> create_thumbnail(std::string file_path);
        void on_treeview_selection_changed();
        void on_directory_monitor_changed(const Glib::RefPtr<Gio::File>& file1,
            const Glib::RefPtr<Gio::File>& file2, Gio::FileMonitorEvent event_type,
            Glib::RefPtr<Gio::File> directory);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape
#endif
