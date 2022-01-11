// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 1999-2007, 2021 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "export-batch.h"

#include <glibmm/convert.h>
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <gtkmm.h>
#include <png.h>

#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "export-helper.h"
#include "export-preview.h"
#include "extension/db.h"
#include "file.h"
#include "helper/png-write.h"
#include "inkscape-window.h"
#include "inkscape.h"
#include "io/resource.h"
#include "io/sys.h"
#include "layer-manager.h"
#include "message-stack.h"
#include "object/object-set.h"
#include "object/sp-namedview.h"
#include "object/sp-root.h"
#include "preferences.h"
#include "selection-chemistry.h"
#include "ui/dialog-events.h"
#include "ui/dialog/dialog-notebook.h"
#include "ui/dialog/filedialog.h"
#include "ui/interface.h"
#include "ui/widget/scrollprotected.h"
#include "ui/widget/unit-menu.h"

#ifdef _WIN32

#endif

using Inkscape::Util::unit_table;

namespace Inkscape {
namespace UI {
namespace Dialog {

// START OF BATCH ITEM

class BatchItem : public Gtk::FlowBoxChild
{
public:
    BatchItem(SPItem *item);
    ~BatchItem() override;

public:
    Gtk::CheckButton selector;
    SPItem *getItem() { return _item; }
    bool isActive() { return selector.get_active(); }
    void refresh(bool hide = false);
    void refreshHide(const std::vector<SPItem *> *list) { preview->refreshHide(list); }

private:
    Gtk::Grid grid;
    Gtk::Label label;
    ExportPreview *preview = nullptr;
    SPItem *_item;
};

BatchItem::~BatchItem()
{
    if (preview) {
        delete preview;
        preview = nullptr;
    }
}

BatchItem::BatchItem(SPItem *item)
    : grid()
    , selector()
{
    if (!item) {
        return;
    }
    _item = item;
    grid.attach(selector, 0, 1, 1, 1);
    grid.set_row_spacing(5);
    grid.set_column_spacing(5);

    Glib::ustring id = _item->defaultLabel();
    if (id.empty()) {
        id = _item->getId();
    }
    Glib::ustring compactId = id.substr(0, 7);
    if (id.length() > 7) {
        compactId = compactId + "...";
    }

    selector.set_active(true);
    selector.set_can_focus(false);
    selector.set_valign(Gtk::Align::ALIGN_END);
    selector.set_margin_start(2);
    selector.set_margin_bottom(2);

    if (!preview) {
        preview = Gtk::manage(new ExportPreview());
        grid.attach(*preview, 0, 0, 2, 2);
    }
    preview->setItem(_item);
    preview->setDocument(_item->document);
    preview->setSize(64);

    label.set_text(compactId);
    label.set_halign(Gtk::Align::ALIGN_CENTER);
    grid.attach(label, 0, 2, 2, 1);

    add(grid);
    show_all_children();
    show();
    this->set_can_focus(false);
    this->set_tooltip_text(id);
}

void BatchItem::refresh(bool hide)
{
    if (!_item) {
        return;
    }
    if (hide) {
        preview->resetPixels();
    } else {
        preview->queueRefresh();
    }
}

// END OF BATCH ITEM

// START OF BATCH EXPORT

BatchExport::~BatchExport()
{
    ;
}

void BatchExport::initialise(const Glib::RefPtr<Gtk::Builder> &builder)
{
    builder->get_widget("b_s_selection", selection_buttons[SELECTION_SELECTION]);
    selection_names[SELECTION_SELECTION] = "selection";
    builder->get_widget("b_s_layers", selection_buttons[SELECTION_LAYER]);
    selection_names[SELECTION_LAYER] = "layer";

    builder->get_widget("b_preview_box", preview_container);
    builder->get_widget("b_show_preview", show_preview);
    builder->get_widget("b_num_elements", num_elements);
    builder->get_widget("b_advance_box", adv_box);
    builder->get_widget("b_hide_all", hide_all);
    builder->get_widget("b_filename", filename_entry);
    builder->get_widget("b_export", export_btn);
    builder->get_widget("b_progress_bar", _prog);
    builder->get_widget_derived("b_export_list", export_list);

    Inkscape::UI::Widget::ScrollTransfer<Gtk::ScrolledWindow> *temp = nullptr;
    builder->get_widget_derived("b_pbox_scroll", temp);
    builder->get_widget_derived("b_scroll", temp);
}

void BatchExport::selectionModified(Inkscape::Selection *selection, guint flags)
{
    if (!_desktop || _desktop->getSelection() != selection) {
        return;
    }
    if (!(flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
        return;
    }
    refreshItems();
}

void BatchExport::selectionChanged(Inkscape::Selection *selection)
{
    if (!_desktop || _desktop->getSelection() != selection) {
        return;
    }
    if (selection->isEmpty()) {
        selection_buttons[SELECTION_SELECTION]->set_sensitive(false);
        if (current_key == SELECTION_SELECTION) {
            selection_buttons[(selection_mode)0]->set_active(true); // This causes refresh area
            // return otherwise refreshArea will be called again
            // even though we are at default key, selection is the one which was original key.
            prefs->setString("/dialogs/export/batchexportarea/value", selection_names[SELECTION_SELECTION]);
            return;
        }
    } else {
        selection_buttons[SELECTION_SELECTION]->set_sensitive(true);
        Glib::ustring pref_key_name = prefs->getString("/dialogs/export/batchexportarea/value");
        if (selection_names[SELECTION_SELECTION] == pref_key_name && current_key != SELECTION_SELECTION) {
            selection_buttons[SELECTION_SELECTION]->set_active();
            return;
        }
    }
    refreshItems();
    refreshExportHints();
}

// Setup Single Export.Called by export on realize
void BatchExport::setup()
{
    if (setupDone) {
        return;
    }
    setupDone = true;
    prefs = Inkscape::Preferences::get();

    // Setup Advance Options
    adv_box->pack_start(advance_options, true, true, 0);
    adv_box->show_all_children();

    export_list->setup();

    // set them before connecting to signals
    setDefaultFilename();
    setDefaultSelectionMode();

    refreshExportHints();

    refreshItems();

    // Connect Signals
    for (auto [key, button] : selection_buttons) {
        button->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &BatchExport::onAreaTypeToggle), key));
    }
    show_preview->signal_toggled().connect(sigc::mem_fun(*this, &BatchExport::refreshPreview));
    filenameConn = filename_entry->signal_changed().connect(sigc::mem_fun(*this, &BatchExport::onFilenameModified));
    exportConn = export_btn->signal_clicked().connect(sigc::mem_fun(*this, &BatchExport::onExport));
    browseConn = filename_entry->signal_icon_press().connect(sigc::mem_fun(*this, &BatchExport::onBrowse));
    hide_all->signal_toggled().connect(sigc::mem_fun(*this, &BatchExport::refreshPreview));
}

void BatchExport::refreshItems()
{
    if (!_desktop) {
        return;
    }
    SPDocument *doc = _desktop->getDocument();
    if (!doc) {
        return;
    }
    doc->ensureUpToDate();

    // Create New List of Items
    std::set<SPItem *> itemsList;
    switch (current_key) {
        case SELECTION_SELECTION: {
            auto items = _desktop->getSelection()->items();
            for (auto i = items.begin(); i != items.end(); ++i) {
                SPItem *item = *i;
                if (item) {
                    itemsList.insert(item);
                }
            }
            break;
        }
        case SELECTION_LAYER: {
            auto layersList = _desktop->layerManager().getAllLayers();
            for (auto item : layersList) {
                itemsList.insert(item);
            }

            break;
        }
        default:
            break;
    }

    // Number of Items
    int num = itemsList.size();
    Glib::ustring label_text = std::to_string(num) + " Items";
    num_elements->set_text(label_text);

    // Create a list of items which are already present but will be removed as they are not present anymore
    std::vector<std::string> toRemove;
    for (auto &[key, val] : current_items) {
        SPItem *item = val->getItem();
        // if item is not present in itemList add it to remove list so that we can remove it
        auto itemItr = itemsList.find(item);
        if (itemItr == itemsList.end() || (*itemItr)->getId() != key) {
            toRemove.push_back(key);
        }
    }

    // now remove all the items
    for (auto key : toRemove) {
        if (current_items[key]) {
            // Preview Boxes are GTK managed so simply removing from container will handle delete
            preview_container->remove(*current_items[key]);
            current_items.erase(key);
        }
    }

    // now add which were are new
    for (auto &item : itemsList) {
        auto id = item->getId();
        // If an Item with same Id is already present, Skip
        if (current_items[id] && current_items[id]->getItem() == item) {
            continue;
        }
        // Add new item to the end of list
        current_items[id] = Gtk::manage(new BatchItem(item));
        preview_container->insert(*current_items[id], -1);
    }

    refreshPreview();
}

void BatchExport::refreshPreview()
{
    if (_desktop) {
        // For Batch Export we are now hiding all object except current object
        // std::vector<SPItem *> selected(_desktop->getSelection()->items().begin(),
        //                                _desktop->getSelection()->items().end());
        bool hide = hide_all->get_active();
        for (auto &[key, val] : current_items) {
            if (show_preview->get_active()) {
                std::vector<SPItem *> selected = {val->getItem()};
                val->refreshHide(hide ? &selected : nullptr);
                val->refresh();
            } else {
                val->refresh(true);
            }
        }
    }
}

void BatchExport::refreshExportHints()
{
    ;
}

// Signals CallBack

void BatchExport::onAreaTypeToggle(selection_mode key)
{
    // Prevent executing function twice
    if (!selection_buttons[key]->get_active()) {
        return;
    }
    // If you have reached here means the current key is active one ( not sure if multiple transitions happen but
    // last call will change values)
    current_key = key;
    prefs->setString("/dialogs/export/batchexportarea/value", selection_names[current_key]);

    refreshItems();
    refreshExportHints();
}

void BatchExport::onFilenameModified()
{
    ;
}

void BatchExport::onExport()
{
    interrupted = false;
    if (!_desktop)
        return;
    export_btn->set_sensitive(false);
    bool exportSuccessful = true;

    // If there are no selected button, simply flash message in status bar
    int num = current_items.size();
    if (current_items.size() == 0) {
        _desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No items selected."));
        export_btn->set_sensitive(true);
        return;
    }

    // Find and remove any extension from filename so that we can add suffix to it.
    Glib::ustring filename = filename_entry->get_text();
    Glib::ustring filename_extension = get_ext_from_filename(filename);
    if (ExtensionList::all_extensions[filename_extension]) {
        auto extension_point = filename.rfind(filename_extension);
        filename.erase(extension_point);
    }

    int n = 0;

    // create vector of exports
    int num_rows = export_list->get_rows();
    std::vector<Glib::ustring> suffixs;
    std::vector<Glib::ustring> extensions;
    std::vector<double> dpis;
    for (int i = 0; i < num_rows; i++) {
        suffixs.push_back(export_list->get_suffix(i));
        extensions.push_back(export_list->get_extension(i));
        dpis.push_back(export_list->get_dpi(i));
    }

    // We are exporting standalone items only for now
    // std::vector<SPItem *> selected(_desktop->getSelection()->items().begin(),
    // _desktop->getSelection()->items().end());
    bool hide = hide_all->get_active();

    // Start Exporting Each Item
    for (auto i = current_items.begin(); i != current_items.end() && !interrupted; ++i) {
        BatchItem *batchItem = i->second;
        if (!batchItem->isActive()) {
            n++;
            continue;
        }
        SPItem *item = batchItem->getItem();
        if (!item) {
            n++;
            continue;
        }

        Geom::OptRect area = item->documentVisualBounds();
        if (!area) {
            n++;
            continue;
        }
        Glib::ustring id = item->defaultLabel();
        if (id.empty()) {
            id = item->getId();
        }
        if (id.empty()) {
            n++;
            continue;
        }
        for (int i = 0; i < num_rows; i++) {
            auto omod = ExtensionList::valid_extensions[extensions[i]];
            float dpi = dpis[i];

            Glib::ustring item_filename = filename + "_" + id;
            if (!suffixs[i].empty()) {
                item_filename = item_filename + "_" + suffixs[i];
            }
            item_filename = item_filename + "_" + std::to_string((int)dpi);

            if (!omod) {
                continue;
            }
            if (prog_dlg) {
                delete prog_dlg;
                prog_dlg = nullptr;
            }
            prog_dlg = create_progress_dialog(Glib::ustring::compose(_("Exporting %1 files"), num));
            prog_dlg->set_export_panel(this);
            setExporting(true, Glib::ustring::compose(_("Exporting %1 files"), num));
            prog_dlg->set_current(n);
            prog_dlg->set_total(num);

            onProgressCallback(0.0, prog_dlg);
            bool found = getNonConflictingFilename(item_filename, extensions[i]);
            if (!found) {
                n++;
                continue;
            }

            if (omod->is_raster()) {
                unsigned long int width = (int)(area->width() * dpi / DPI_BASE + 0.5);
                unsigned long int height = (int)(area->height() * dpi / DPI_BASE + 0.5);

                std::vector<SPItem *> show_only = {item};
                exportSuccessful = _export_raster(*area, width, height, dpi, item_filename, true, onProgressCallback,
                                                  prog_dlg, omod, hide ? &show_only : nullptr, &advance_options);
            } else {
                setExporting(true, Glib::ustring::compose(_("Exporting %1"), filename));
                SPDocument *doc = _desktop->getDocument();
                SPDocument *copy_doc = (doc->copy()).get();
                std::vector<SPItem *> items;
                items.push_back(item);
                exportSuccessful = _export_vector(omod, copy_doc, item_filename, true, &items);
            }
            setExporting(false);
        }
        if (prog_dlg) {
            delete prog_dlg;
            prog_dlg = nullptr;
        }
    }
}

bool BatchExport::getNonConflictingFilename(Glib::ustring &filename, Glib::ustring const extension)
{
    if (!_desktop) {
        return false;
    }
    SPDocument *doc = _desktop->getDocument();
    std::string path = absolutize_path_from_document_location(doc, Glib::filename_from_utf8(filename));
    Glib::ustring test_filename = path + extension;
    if (!Inkscape::IO::file_test(test_filename.c_str(), G_FILE_TEST_EXISTS)) {
        filename = test_filename;
        return true;
    }
    for (int i = 1; i <= 100; i++) {
        test_filename = path + "_copy_" + std::to_string(i) + extension;
        if (!Inkscape::IO::file_test(test_filename.c_str(), G_FILE_TEST_EXISTS)) {
            filename = test_filename;
            return true;
        }
    }
    return false;
}

void BatchExport::onBrowse(Gtk::EntryIconPosition pos, const GdkEventButton *ev)
{
    if (!_app) {
        return;
    }
    Gtk::Window *window = _app->get_active_window();
    browseConn.block();
    Glib::ustring filename = Glib::filename_from_utf8(filename_entry->get_text());

    if (filename.empty()) {
        Glib::ustring tmp;
        filename = create_filepath_from_id(tmp, tmp);
    }

    Inkscape::UI::Dialog::FileSaveDialog *dialog = Inkscape::UI::Dialog::FileSaveDialog::create(
        *window, filename, Inkscape::UI::Dialog::RASTER_TYPES, _("Select a filename for exporting"), "", "",
        Inkscape::Extension::FILE_SAVE_METHOD_EXPORT);

    if (dialog->show()) {
        filename = dialog->getFilename();
        Inkscape::Extension::Output *selection_type =
            dynamic_cast<Inkscape::Extension::Output *>(dialog->getSelectionType());
        Glib::ustring extension = selection_type->get_extension();
        ExtensionList::appendExtensionToFilename(filename, extension);
        filename_entry->set_text(filename);
        filename_entry->set_position(filename.length());
        // deleting dialog before exporting is important
        // proper delete function should be made for dialog IMO
        delete dialog;
        onExport();
    } else {
        delete dialog;
    }
    browseConn.unblock();
}

// Utils Functions

// We first check any export hints related to document. If there is none we create a default name using document
// name. doc_export_name is set here and will only be changed when exporting.
void BatchExport::setDefaultFilename()
{
    if (!_desktop) {
        return;
    }
    Glib::ustring filename;
    float xdpi = 0.0, ydpi = 0.0;
    SPDocument *doc = _desktop->getDocument();
    sp_document_get_export_hints(doc, filename, &xdpi, &ydpi);
    if (filename.empty()) {
        Glib::ustring filename_entry_text = filename_entry->get_text();
        Glib::ustring extension = ".png";
        filename = get_default_filename(filename_entry_text, extension, doc);
    }
    doc_export_name = filename;
    original_name = filename;
    filename_entry->set_text(filename);
    filename_entry->set_position(filename.length());
}

void BatchExport::setDefaultSelectionMode()
{
    current_key = (selection_mode)0; // default key
    bool found = false;
    Glib::ustring pref_key_name = prefs->getString("/dialogs/export/batchexportarea/value");
    for (auto [key, name] : selection_names) {
        if (pref_key_name == name) {
            current_key = key;
            found = true;
            break;
        }
    }
    if (!found) {
        pref_key_name = selection_names[current_key];
    }
    if (_desktop) {
        if (current_key == SELECTION_SELECTION && (_desktop->getSelection())->isEmpty()) {
            current_key = (selection_mode)0;
        }
        if ((_desktop->getSelection())->isEmpty()) {
            selection_buttons[SELECTION_SELECTION]->set_sensitive(false);
        }
    }
    selection_buttons[current_key]->set_active(true);

    // we need to set pref key because signals above will set set pref == current key but we sometimes change
    // current key like selection key
    prefs->setString("/dialogs/export/batchexportarea/value", pref_key_name);
}

void BatchExport::setExporting(bool exporting, Glib::ustring const &text)
{
    if (exporting) {
        _prog->set_text(text);
        _prog->set_fraction(0.0);
        _prog->set_sensitive(true);
        export_btn->set_sensitive(false);
    } else {
        _prog->set_text("");
        _prog->set_fraction(0.0);
        _prog->set_sensitive(false);
        export_btn->set_sensitive(true);
    }
}

ExportProgressDialog *BatchExport::create_progress_dialog(Glib::ustring progress_text)
{
    // dont forget to delete it later
    auto dlg = new ExportProgressDialog(_("Export in progress"), true);
    dlg->set_transient_for(*(INKSCAPE.active_desktop()->getToplevel()));

    Gtk::ProgressBar *prg = Gtk::manage(new Gtk::ProgressBar());
    prg->set_text(progress_text);
    dlg->set_progress(prg);
    auto CA = dlg->get_content_area();
    CA->pack_start(*prg, FALSE, FALSE, 4);

    Gtk::Button *btn = dlg->add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);

    btn->signal_clicked().connect(sigc::mem_fun(*this, &BatchExport::onProgressCancel));
    dlg->signal_delete_event().connect(sigc::mem_fun(*this, &BatchExport::onProgressDelete));

    dlg->show_all();
    return dlg;
}

/// Called when dialog is deleted
bool BatchExport::onProgressDelete(GdkEventAny * /*event*/)
{
    interrupted = true;
    prog_dlg->set_stopped();
    return TRUE;
}

/// Called when progress is cancelled
void BatchExport::onProgressCancel()
{
    interrupted = true;
    prog_dlg->set_stopped();
}

/// Called for every progress iteration
unsigned int BatchExport::onProgressCallback(float value, void *dlg)
{
    auto dlg2 = reinterpret_cast<ExportProgressDialog *>(dlg);

    auto self = dynamic_cast<BatchExport *>(dlg2->get_export_panel());

    if (!self || self->interrupted)
        return FALSE;

    auto current = dlg2->get_current();
    auto total = dlg2->get_total();
    if (total > 0) {
        double completed = current;
        completed /= static_cast<double>(total);

        value = completed + (value / static_cast<double>(total));
    }

    auto prg = dlg2->get_progress();
    prg->set_fraction(value);

    if (self) {
        self->_prog->set_fraction(value);
    }

    int evtcount = 0;
    while ((evtcount < 16) && gdk_events_pending()) {
        Gtk::Main::iteration(false);
        evtcount += 1;
    }

    Gtk::Main::iteration(false);
    return TRUE;
}

void BatchExport::setDocument(SPDocument *document)
{
    ;
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
