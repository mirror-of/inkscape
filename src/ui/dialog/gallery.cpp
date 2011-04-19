/** @file
 * @brief Gallery dialog
 */
/* Authors:
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

ModelColumns::ModelColumns() {
    add(thumbnail);
    add(name);
    add(file);
}

Gallery::Gallery() : UI::Widget::Panel ("", "/dialogs/gallery", SP_VERB_DIALOG_GALLERY) {

    // Creation
    Gtk::Box* vbox = _getContents();
    filechooserbutton = new Gtk::FileChooserButton();
    button_import = new Gtk::Button(_("Import"));
    Gtk::HButtonBox* hbuttonbox = new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6);
    Gtk::ScrolledWindow* scrolledwindow = new Gtk::ScrolledWindow();
    treeview = new Gtk::TreeView();
    /// TreeView
    model = Gtk::ListStore::create(columns);
    treeview->append_column(_("Thumbnail"), columns.thumbnail);
    Gtk::TreeViewColumn* column_name = new Gtk::TreeViewColumn(_("Name"));
    Gtk::CellRendererText* cr_text = new Gtk::CellRendererText();
    cr_text->set_property("ellipsize", Pango::ELLIPSIZE_END);
    column_name->pack_start(*cr_text, true);
    column_name->add_attribute(*cr_text, "text", columns.name);
    treeview->append_column(*column_name);
    /// Drag and Drop
    std::list<Gtk::TargetEntry> drag_targets;
    drag_targets.push_back(Gtk::TargetEntry("text/uri-list", Gtk::TARGET_SAME_APP, URI_LIST));
    THUMBNAIL_SIZE = 32;

    create_input_extension_map();

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
    treeview->signal_drag_data_get().connect(sigc::mem_fun(*this,
              &Gallery::on_treeview_drag_data_get));

    // Signals
    filechooserbutton->signal_current_folder_changed().connect(
            sigc::mem_fun(*this, &Gallery::on_filechooserbutton_current_folder_changed));
    treeview->signal_row_activated().connect(
            sigc::mem_fun(*this, &Gallery::on_treeview_row_activated));
    button_import->signal_clicked().connect(
            sigc::mem_fun(*this, &Gallery::on_button_import_clicked));

    // Go back to the last folder it opened
    Inkscape::Preferences *preferences = Inkscape::Preferences::get();
    Glib::ustring directory_path = preferences->getString("/dialogs/gallery/directory");
    bool directory_exists = Glib::file_test(directory_path, Glib::FILE_TEST_EXISTS);
    if (!directory_path.empty() && directory_exists) {
        Glib::RefPtr<Gio::File> directory = Gio::File::create_for_path(directory_path);
        filechooserbutton->set_file(directory);
        update_treeview(directory);
    }
    
    vbox->show_all();
}

void Gallery::on_filechooserbutton_current_folder_changed()
{
    Inkscape::Preferences *preferences = Inkscape::Preferences::get();
    
    Glib::RefPtr<Gio::File> new_directory = filechooserbutton->get_file();
    Glib::ustring new_directory_path = Glib::ustring::ustring(new_directory->get_path());
    Glib::ustring old_directory_path = preferences->getString("/dialogs/gallery/directory");
    
    if (old_directory_path.empty() || new_directory_path != old_directory_path) {
        update_treeview(new_directory);
        preferences->setString("/dialogs/gallery/directory", new_directory_path);
    }
}

void Gallery::update_treeview(Glib::RefPtr<Gio::File> directory)
{
    model->clear();
    directory->enumerate_children_async(
        sigc::bind< Glib::RefPtr<Gio::File> >(
            sigc::mem_fun(*this, &Gallery::on_directory_enumerated),
        directory));
}

void Gallery::on_directory_enumerated(const Glib::RefPtr<Gio::AsyncResult>& result,
    Glib::RefPtr<Gio::File> directory)
{
    Glib::RefPtr<Gio::FileEnumerator> enumerator = directory->enumerate_children_finish(result);

    enumerator->next_files_async(
        sigc::bind< Glib::RefPtr<Gio::FileEnumerator>, Glib::RefPtr<Gio::File> >(
            sigc::mem_fun(*this, &Gallery::on_enumerator_file_ready),
        enumerator, directory));
}

void Gallery::on_enumerator_file_ready(const Glib::RefPtr<Gio::AsyncResult>& result,
    Glib::RefPtr<Gio::FileEnumerator> enumerator, Glib::RefPtr<Gio::File> directory)
{
    std::vector< Glib::RefPtr<Gio::FileInfo> > file_info_list = enumerator->next_files_finish(result);
    Glib::RefPtr<Gio::FileInfo> file_info;

    try {
        file_info = file_info_list.at(0);
    } catch(std::exception) {
        model->set_sort_column(columns.name, Gtk::SORT_ASCENDING);
        return;
    }
        
    Glib::ustring file_name = Glib::ustring::ustring(file_info->get_name());
    
    Glib::ustring file_extension = Inkscape::IO::get_file_extension(file_name);

    bool filetype_supported = (extension_input_map.find(file_extension) != extension_input_map.end());

    if (filetype_supported) {
        Glib::RefPtr<Gio::File> file = directory->get_child(file_name);
        std::string file_path = file->get_path();
        Glib::RefPtr<Gdk::Pixbuf> pixbuf = create_thumbnail(file_path);

        Gtk::TreeModel::iterator iter = model->append();
        iter->set_value(columns.thumbnail, pixbuf);
        iter->set_value(columns.name, file_name);
        iter->set_value(columns.file, file);
    }

    // Move onto the next file in the directory
    enumerator->next_files_async(
        sigc::bind< Glib::RefPtr<Gio::FileEnumerator>, Glib::RefPtr<Gio::File> >(
            sigc::mem_fun(*this, &Gallery::on_enumerator_file_ready),
        enumerator, directory));
}

void Gallery::create_input_extension_map()
{
    Inkscape::Extension::DB::InputList extension_list;
    Inkscape::Extension::db.get_input_list(extension_list);

    for (Inkscape::Extension::DB::InputList::iterator current_item = extension_list.begin();
        current_item != extension_list.end(); current_item++)
    {
        Inkscape::Extension::Input* input = *current_item;
        Glib::ustring extension = Glib::ustring::ustring(input->get_extension());
        extension_input_map[extension] = input;
    }
}

void Gallery::on_treeview_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
{
    on_button_import_clicked();
}

void Gallery::on_button_import_clicked()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    Gtk::TreeModel::iterator selected_iter = treeview->get_selection()->get_selected();
    Glib::RefPtr<Gio::File> file = selected_iter->get_value(columns.file);
    
    file_import(doc, file->get_path(), NULL);
}

/*
 * Creates a thumbnail with size THUMBNAIL_SIZE x THUMBNAIL_SIZE px
 */

Glib::RefPtr<Gdk::Pixbuf> Gallery::create_thumbnail(std::string file_path)
{
    Glib::RefPtr<Gdk::Pixbuf> initial_pixbuf;
    try {
        initial_pixbuf = Gdk::Pixbuf::create_from_file(file_path, THUMBNAIL_SIZE,
            THUMBNAIL_SIZE, true);
    } catch(Glib::Error) {
        initial_pixbuf = Gtk::IconTheme::get_default()->load_icon("image-x-generic",
            THUMBNAIL_SIZE);
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

void Gallery::on_treeview_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&,
    Gtk::SelectionData& selection_data, guint target_id, guint etime)
{
    printf("%s\n", "GET!!!");
    
    Gtk::TreeModel::iterator selected_iter = treeview->get_selection()->get_selected();
    Glib::RefPtr<Gio::File> file = selected_iter->get_value(columns.file);

    std::vector<Glib::ustring> uri_list;
    uri_list.push_back(file->get_uri());
    
    selection_data.set_uris(uri_list);
}


Gallery::~Gallery() { }


} // namespace Dialog
} // namespace UI
} // namespace Inkscape
