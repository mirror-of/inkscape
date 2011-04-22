/*
 * Gallery - A dialog that provides a method to open a folder containing
 * frequently used files import them quickly and easily using various methods.
 *
 * Authors:
 *   Andrew Higginson
 *
 * Copyright (C) 2011
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */
 
#include "verbs.h"
#include "ui/dialog/gallery.h"
#include "io/sys.h"
#include "file.h"
#include "interface.h"
#include "preferences.h"

#include "extension/db.h"
#include "extension/input.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/*
 * The columns for the TreeView
 */
ModelColumns::ModelColumns() {
    add(thumbnail);
    add(name);
    add(file);
}

/*
 * The main constructor
 */

Gallery::Gallery() : UI::Widget::Panel ("", "/dialogs/gallery", SP_VERB_DIALOG_GALLERY) {

    // Creation
    Gtk::Box* vbox = _getContents();
    filechooserbutton = Gtk::manage(new Gtk::FileChooserButton());
    button_import = Gtk::manage(new Gtk::Button(_("Import")));
    Gtk::HButtonBox* hbuttonbox = Gtk::manage(new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6));
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    treeview = Gtk::manage(new Gtk::TreeView());
    /// TreeView
    model = Gtk::ListStore::create(columns);
    treeview->append_column(_("Thumbnail"), columns.thumbnail);
    Gtk::TreeViewColumn* column_name = Gtk::manage(new Gtk::TreeViewColumn(_("Name")));
    Gtk::CellRendererText* cr_text = Gtk::manage(new Gtk::CellRendererText());
    cr_text->set_property("ellipsize", Pango::ELLIPSIZE_END);
    column_name->pack_start(*cr_text, true);
    column_name->add_attribute(*cr_text, "text", columns.name);
    treeview->append_column(*column_name);
    /// Drag and Drop
    std::list<Gtk::TargetEntry> drag_targets;
    drag_targets.push_back(Gtk::TargetEntry("text/uri-list", Gtk::TARGET_SAME_APP, URI_LIST));
    THUMBNAIL_SIZE = 32;
    has_fallback_icon = (Gtk::IconTheme::get_default()->lookup_icon("image-x-generic",
                        THUMBNAIL_SIZE, Gtk::ICON_LOOKUP_FORCE_SIZE) != 0);
    updating_model = false;

    Inkscape::Extension::db.get_input_map(extension_input_map);

    // Packing
    scrolledwindow->add(*treeview);
    hbuttonbox->pack_start(*button_import, false, false);
    vbox->pack_start(*filechooserbutton, false, false);
    vbox->pack_start(*scrolledwindow, true, true);
    vbox->pack_start(*hbuttonbox, false, false);

    // Properties
    vbox->set_spacing(12);
    vbox->set_border_width(12);
    filechooserbutton->set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    scrolledwindow->set_shadow_type(Gtk::SHADOW_IN);
    scrolledwindow->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    /// TreeView
    treeview->set_model(model);
    treeview->set_headers_visible(false);
    treeview->set_rules_hint(true);
    treeview->enable_model_drag_source();
    treeview->set_enable_search(true);
    treeview->set_search_column(columns.name);
    treeview->enable_model_drag_source(drag_targets);
    treeview->set_tooltip_column(COLUMN_NAME);

    // Signals
    filechooserbutton->signal_current_folder_changed().connect(
            sigc::mem_fun(*this, &Gallery::on_filechooserbutton_current_folder_changed));
    treeview->signal_row_activated().connect(
            sigc::mem_fun(*this, &Gallery::on_treeview_row_activated));
    treeview->get_selection()->signal_changed().connect(
            sigc::mem_fun(*this, &Gallery::on_treeview_selection_changed));
    button_import->signal_clicked().connect(
            sigc::mem_fun(*this, &Gallery::on_button_import_clicked));
    treeview->signal_drag_data_get().connect(sigc::mem_fun(*this,
              &Gallery::on_treeview_drag_data_get));

    on_treeview_selection_changed();
    vbox->show_all();

    // Go back to the last folder it opened
    Inkscape::Preferences *preferences = Inkscape::Preferences::get();
    Glib::ustring directory_path = preferences->getString("/dialogs/gallery/directory");
    
    bool directory_exists = Glib::file_test(directory_path, Glib::FILE_TEST_EXISTS);
    if (!directory_path.empty() && directory_exists) {
        filechooserbutton->set_current_folder(directory_path);
        update_treeview(directory_path);
    }
}

/*
 * Handle the event when the user changes the folder to be browsed.
 */

void Gallery::on_filechooserbutton_current_folder_changed()
{   
    Inkscape::Preferences *preferences = Inkscape::Preferences::get();
    Glib::ustring new_directory_path = filechooserbutton->get_current_folder();
    Glib::ustring old_directory_path = preferences->getString("/dialogs/gallery/directory");

    // Only browse it the selected folder if the user is not re-selecting it
    if (old_directory_path.empty() || new_directory_path != old_directory_path) {
        update_treeview(new_directory_path);
    }
}

/*
 * Re-create the treeview by enumerating the children of the selected directory
 */

void Gallery::update_treeview(Glib::ustring directory_path, bool update_monitor)
{
    Inkscape::Preferences *preferences = Inkscape::Preferences::get();
    preferences->setString("/dialogs/gallery/directory", directory_path);
    
    Glib::RefPtr<Gio::File> directory = Gio::File::create_for_path(directory_path);

    if (update_monitor) {
        update_directory_monitor(directory);
    }

    filechooserbutton->set_tooltip_text(directory_path);

    // Only update the model when nothing else is updating it
    if (!updating_model) {
        updating_model = true;
        model->clear();
        directory->enumerate_children_async(
            sigc::bind< Glib::RefPtr<Gio::File> >(
                sigc::mem_fun(*this, &Gallery::on_directory_enumerated),
            directory));
    }
}

/*
 * Re-create the treeview by enumerating the children of the selected directory
 */

void Gallery::update_directory_monitor(Glib::RefPtr<Gio::File> directory)
{
    if (directory_monitor != 0) {
        directory_monitor->cancel();
    }
    
    directory_monitor = directory->monitor_directory();
    directory_monitor->signal_changed().connect(
        sigc::bind< Glib::RefPtr<Gio::File> >(
            sigc::mem_fun(*this, &Gallery::on_directory_monitor_changed),
        directory));
}

/*
 * Handle the event when the selected directory's children have all been enumerated
 */

void Gallery::on_directory_enumerated(const Glib::RefPtr<Gio::AsyncResult>& result,
    Glib::RefPtr<Gio::File> directory)
{
    Glib::RefPtr<Gio::FileEnumerator> enumerator = directory->enumerate_children_finish(result);

    // Get the first file yielded by the enumerator
    enumerator->next_files_async(
        sigc::bind< Glib::RefPtr<Gio::FileEnumerator>, Glib::RefPtr<Gio::File> >(
            sigc::mem_fun(*this, &Gallery::on_enumerator_file_ready),
        enumerator, directory));
}

/*
 * Handle the event when a new file is yielded by the directory's enumerator
 */

void Gallery::on_enumerator_file_ready(const Glib::RefPtr<Gio::AsyncResult>& result,
    Glib::RefPtr<Gio::FileEnumerator> enumerator, Glib::RefPtr<Gio::File> directory)
{
    std::vector< Glib::RefPtr<Gio::FileInfo> > file_info_list = enumerator->next_files_finish(result);
    Glib::RefPtr<Gio::FileInfo> file_info;
    
    // Get the first FileInfo in the list (the only one)
    try {
        file_info = file_info_list.at(0);
    // If there are no more FileInfo objects, all files have been yielded
    } catch(std::exception) {
        // Sort the TreeView by filename
        model->set_sort_column(columns.name, Gtk::SORT_ASCENDING);
        // Close the enumerator
        enumerator->close_async(Glib::PRIORITY_LOW,
            sigc::bind< Glib::RefPtr<Gio::FileEnumerator> >(
                sigc::mem_fun(*this, &Gallery::on_enumerator_closed),
            enumerator));
        updating_model = false;
        return;
    }
    
    Glib::ustring file_name = Glib::ustring::ustring(file_info->get_name());
    Glib::ustring file_extension = Inkscape::IO::get_file_extension(file_name);
    bool filetype_supported = (extension_input_map.find(file_extension) != extension_input_map.end());

    // If the filetype is supported, add the file, its thumbnail and its name to the TreeView
    if (filetype_supported) {
        Glib::RefPtr<Gio::File> file = directory->get_child(file_name);
        std::string file_path = file->get_path();
        Glib::RefPtr<Gdk::Pixbuf> pixbuf = create_thumbnail(file_path);

        Gtk::TreeModel::iterator iter = model->append();
        iter->set_value(columns.thumbnail, pixbuf);
        iter->set_value(columns.name, file_name);
        iter->set_value(columns.file, file);
    }

    // Yield the next file in the directory
    enumerator->next_files_async(
        sigc::bind< Glib::RefPtr<Gio::FileEnumerator>, Glib::RefPtr<Gio::File> >(
            sigc::mem_fun(*this, &Gallery::on_enumerator_file_ready),
        enumerator, directory));
}

/*
 * Handle event where the enumerator is closed
 */

void Gallery::on_enumerator_closed(const Glib::RefPtr<Gio::AsyncResult>& result,
    Glib::RefPtr<Gio::FileEnumerator> enumerator)
{
    enumerator->close_finish(result);
}

/*
 * Handle the event where a user activates (double clicks) a row in the Treeview
 */

void Gallery::on_treeview_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
{
    // Import the file designated by the activated row
    on_button_import_clicked();
}

/*
 * Handle the event where a user clicks the import button
 */

void Gallery::on_button_import_clicked()
{
    // Get the file designated by the selected row
    Gtk::TreeModel::iterator selected_iter = treeview->get_selection()->get_selected();
    Glib::RefPtr<Gio::File> file = selected_iter->get_value(columns.file);

    // Import the file into the current document
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    file_import(doc, file->get_path(), NULL);
}

/*
 * Creates a thumbnail with size THUMBNAIL_SIZE x THUMBNAIL_SIZE px
 */

Glib::RefPtr<Gdk::Pixbuf> Gallery::create_thumbnail(std::string file_path)
{
    // Try to get a thumbnail of the file
    Glib::RefPtr<Gdk::Pixbuf> initial_pixbuf;
    try {
        initial_pixbuf = Gdk::Pixbuf::create_from_file(file_path, THUMBNAIL_SIZE,
            THUMBNAIL_SIZE, true);
    // If there is an error, try to use the fallback image icon
    } catch(Glib::Error) {
        if (has_fallback_icon) {
            initial_pixbuf = Gtk::IconTheme::get_default()->load_icon("image-x-generic",
                THUMBNAIL_SIZE);
    // Otherwise just return a blank pixbuf at the correct size
        } else {
            initial_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8,
                THUMBNAIL_SIZE, THUMBNAIL_SIZE);
            initial_pixbuf->fill(0);
            return initial_pixbuf;
        }
    }

    int pixbuf_width = initial_pixbuf->get_width();
    int pixbuf_height = initial_pixbuf->get_height();

    if ((pixbuf_width == THUMBNAIL_SIZE) && (pixbuf_height == THUMBNAIL_SIZE)) {
        return initial_pixbuf;
    }

    // Make the pixbuf exactly THUMBNAIL_SIZE x THUMBNAIL_SIZE px by adding transparency
    int x_offset = (THUMBNAIL_SIZE - pixbuf_width) / 2;
    int y_offset = (THUMBNAIL_SIZE - pixbuf_height) / 2;

    Glib::RefPtr<Gdk::Pixbuf> final_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB,
        true, 8, THUMBNAIL_SIZE, THUMBNAIL_SIZE);
    final_pixbuf->fill(0);

    initial_pixbuf->copy_area(0, 0, pixbuf_width, pixbuf_height, final_pixbuf,
        x_offset, y_offset);

    return final_pixbuf;
}

/*
 * Handle the event where a file is dragged and dropped and data is requested on it
 */

void Gallery::on_treeview_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&,
    Gtk::SelectionData& selection_data, guint target_id, guint etime)
{
    // Get the file designated by the dragged row
    Gtk::TreeModel::iterator selected_iter = treeview->get_selection()->get_selected();
    Glib::RefPtr<Gio::File> file = selected_iter->get_value(columns.file);

    // Add the file's uri to a URI list, given to the requestor of the data
    std::vector<Glib::ustring> uri_list;
    uri_list.push_back(file->get_uri());
    selection_data.set_uris(uri_list);
}

/*
 * Handle the event when the user changes their selection on the TreeView
 */

void Gallery::on_treeview_selection_changed()
{
    // Get selected row
    Gtk::TreeModel::iterator selected_iter = treeview->get_selection()->get_selected();
    bool row_is_selected = (selected_iter != 0);
    button_import->set_sensitive(row_is_selected);
}

/*
 * Handle the event when something has changed about the current directory
 */

void Gallery::on_directory_monitor_changed(const Glib::RefPtr<Gio::File>& file1,
    const Glib::RefPtr<Gio::File>& file2, Gio::FileMonitorEvent event_type,
    Glib::RefPtr<Gio::File> directory)
{
    if (event_type == Gio::FILE_MONITOR_EVENT_DELETED ||
        event_type == Gio::FILE_MONITOR_EVENT_CREATED ||
        event_type == Gio::FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED)
    {
        update_treeview(directory->get_path(), false);
    }
}


Gallery::~Gallery() { }


} // namespace Dialog
} // namespace UI
} // namespace Inkscape
