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
#include <regex>

#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "extension/db.h"
#include "extension/output.h"
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
#include "object/sp-page.h"
#include "object/sp-root.h"
#include "page-manager.h"
#include "preferences.h"
#include "selection-chemistry.h"
#include "ui/dialog-events.h"
#include "ui/dialog/export.h"
#include "ui/dialog/dialog-notebook.h"
#include "ui/dialog/filedialog.h"
#include "ui/interface.h"
#include "ui/widget/export-lists.h"
#include "ui/widget/export-preview.h"
#include "ui/widget/scrollprotected.h"
#include "ui/widget/unit-menu.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class BatchItem : public Gtk::FlowBoxChild
{
public:
    BatchItem(SPItem *item);
    BatchItem(SPPage *page);
    ~BatchItem() override = default;

    Glib::ustring getLabel() { return _label_str; }
    SPItem *getItem() { return _item; }
    SPPage *getPage() { return _page; }
    bool isActive() { return _selector.get_active(); }
    void refresh(bool hide = false);
    void refreshHide(const std::vector<SPItem *> *list) { _preview.refreshHide(list); }
    void setDocument(SPDocument *doc) { _preview.setDocument(doc); }

private:
    void init(SPDocument *doc, Glib::ustring label);

    Glib::ustring _label_str;
    Gtk::Grid _grid;
    Gtk::Label _label;
    Gtk::CheckButton _selector;
    ExportPreview _preview;
    SPItem *_item = nullptr;
    SPPage *_page = nullptr;
    bool is_hide = false;
};

BatchItem::BatchItem(SPItem *item)
{
    _item = item;

    Glib::ustring id = _item->defaultLabel();
    if (id.empty()) {
        if (auto _id = _item->getId()) {
            id = _id;
        } else {
            id = "no-id";
        }
    }
    init(_item->document, id);
}

BatchItem::BatchItem(SPPage *page)
{
    _page = page;

    Glib::ustring label = _page->getDefaultLabel();
    if (auto id = _page->label()) {
        label = id;
    }
    init(_page->document, label);
}

void BatchItem::init(SPDocument *doc, Glib::ustring label) {
    _label_str = label;

    _grid.set_row_spacing(5);
    _grid.set_column_spacing(5);
    _grid.set_valign(Gtk::Align::ALIGN_CENTER);

    _selector.set_active(true);
    _selector.set_can_focus(false);
    _selector.set_margin_start(2);
    _selector.set_margin_bottom(2);

    _preview.set_name("export_preview_batch");
    _preview.setItem(_item);
    _preview.setDocument(doc);
    _preview.setSize(64);
    _preview.set_halign(Gtk::ALIGN_CENTER);
    _preview.set_valign(Gtk::ALIGN_CENTER);

    _label.set_width_chars(10);
    _label.set_ellipsize(Pango::ELLIPSIZE_END);
    _label.set_halign(Gtk::Align::ALIGN_CENTER);
    _label.set_text(label);

    set_valign(Gtk::Align::ALIGN_START);
    set_halign(Gtk::Align::ALIGN_START);
    add(_grid);
    show();
    this->set_can_focus(false);
    this->set_tooltip_text(label);

    // This initially packs the widgets with a hidden preview.
    refresh(!is_hide);
}

void BatchItem::refresh(bool hide)
{
    if (_page) {
        auto b = _page->getDesktopRect();
        _preview.setDbox(b.left(), b.right(), b.top(), b.bottom());
    }

    // When hiding the preview, we show the items as a checklist
    // So all items must be packed differently on refresh.
    if (hide != is_hide) {
        is_hide = hide;
        _grid.remove(_selector);
        _grid.remove(_label);
        _grid.remove(_preview);

        if (hide) {
            _selector.set_valign(Gtk::Align::ALIGN_BASELINE);
            _label.set_xalign(0.0);
            _grid.attach(_selector, 0, 1, 1, 1);
            _grid.attach(_label, 1, 1, 1, 1);
        } else {
            _selector.set_valign(Gtk::Align::ALIGN_END);
            _label.set_xalign(0.5);
            _grid.attach(_selector, 0, 1, 1, 1);
            _grid.attach(_label, 0, 2, 2, 1);
            _grid.attach(_preview, 0, 0, 2, 2);
        }
        show_all_children();
    }

    if (!hide) {
        _preview.queueRefresh();
    }
}


void BatchExport::initialise(const Glib::RefPtr<Gtk::Builder> &builder)
{
    builder->get_widget("b_s_selection", selection_buttons[SELECTION_SELECTION]);
    selection_names[SELECTION_SELECTION] = "selection";
    builder->get_widget("b_s_layers", selection_buttons[SELECTION_LAYER]);
    selection_names[SELECTION_LAYER] = "layer";
    builder->get_widget("b_s_pages", selection_buttons[SELECTION_PAGE]);
    selection_names[SELECTION_PAGE] = "page";

    builder->get_widget("b_preview_box", preview_container);
    builder->get_widget("b_show_preview", show_preview);
    builder->get_widget("b_num_elements", num_elements);
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
    selection_buttons[SELECTION_SELECTION]->set_sensitive(!selection->isEmpty());
    if (selection->isEmpty()) {
        if (current_key == SELECTION_SELECTION) {
            selection_buttons[SELECTION_LAYER]->set_active(true); // This causes refresh area
            // return otherwise refreshArea will be called again
            // even though we are at default key, selection is the one which was original key.
            prefs->setString("/dialogs/export/batchexportarea/value", selection_names[SELECTION_SELECTION]);
            return;
        }
    } else {
        Glib::ustring pref_key_name = prefs->getString("/dialogs/export/batchexportarea/value");
        if (selection_names[SELECTION_SELECTION] == pref_key_name && current_key != SELECTION_SELECTION) {
            selection_buttons[SELECTION_SELECTION]->set_active();
            return;
        }
    }
    refreshItems();
    loadExportHints();
}

void BatchExport::pagesChanged()
{
    if (!_desktop || !_document) return;

    bool has_pages = _document->getPageManager().hasPages();
    selection_buttons[SELECTION_PAGE]->set_sensitive(has_pages);

    if (current_key == SELECTION_PAGE && !has_pages) {
        current_key = SELECTION_LAYER;
        selection_buttons[SELECTION_LAYER]->set_active();
    }

    refreshItems();
    loadExportHints();
}

// Setup Single Export.Called by export on realize
void BatchExport::setup()
{
    if (setupDone) {
        return;
    }
    setupDone = true;
    prefs = Inkscape::Preferences::get();

    export_list->setup();

    // set them before connecting to signals
    setDefaultSelectionMode();
    loadExportHints();

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
    if (!_desktop || !_document) return;

    _document->ensureUpToDate();

    // Create New List of Items
    std::set<SPItem *> itemsList;
    std::set<SPPage *> pageList;

    char *num_str = nullptr;
    switch (current_key) {
        case SELECTION_SELECTION: {
            auto items = _desktop->getSelection()->items();
            for (auto i = items.begin(); i != items.end(); ++i) {
                if (SPItem *item = *i) {
                    // Ignore empty items (empty groups, other bad items)
                    if (item->visualBounds()) {
                        itemsList.insert(item);
                    }
                }
            }
            num_str = g_strdup_printf(ngettext("%d Item", "%d Items", itemsList.size()), (int)itemsList.size());
            break;
        }
        case SELECTION_LAYER: {
            for (auto layer : _desktop->layerManager().getAllLayers()) {
                // Ignore empty layers, they have no size.
                if (layer->geometricBounds()) {
                    itemsList.insert(layer);
                }
            }
            num_str = g_strdup_printf(ngettext("%d Layer", "%d Layers", itemsList.size()), (int)itemsList.size());
            break;
        }
        case SELECTION_PAGE: {
            for (auto page : _desktop->getDocument()->getPageManager().getPages()) {
                pageList.insert(page);
            }
            num_str = g_strdup_printf(ngettext("%d Page", "%d Pages", pageList.size()), (int)pageList.size());
            break;
        }
        default:
            break;
    }
    if (num_str) {
        num_elements->set_text(num_str);
        g_free(num_str);
    }

    // Create a list of items which are already present but will be removed as they are not present anymore
    std::vector<std::string> toRemove;
    for (auto &[key, val] : current_items) {
        if (SPItem *item = val->getItem()) {
            // if item is not present in itemList add it to remove list so that we can remove it
            auto itemItr = itemsList.find(item);
            if (itemItr == itemsList.end() || !(*itemItr)->getId() || (*itemItr)->getId() != key) {
                toRemove.push_back(key);
            }
        }
        if (SPPage *page = val->getPage()) {
            auto pageItr = pageList.find(page);
            if (pageItr == pageList.end() || !(*pageItr)->getId() || (*pageItr)->getId() != key) {
                toRemove.push_back(key);
            }
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
        if (auto id = item->getId()) {
            // If an Item with same Id is already present, Skip
            if (current_items[id] && current_items[id]->getItem() == item) {
                continue;
            }
            // Add new item to the end of list
            current_items[id] = Gtk::manage(new BatchItem(item));
            preview_container->insert(*current_items[id], -1);
        }
    }
    for (auto &page : pageList) {
        if (auto id = page->getId()) {
            if (current_items[id] && current_items[id]->getPage() == page) {
                continue;
            }
            current_items[id] = Gtk::manage(new BatchItem(page));
            preview_container->insert(*current_items[id], -1);
        }
    }

    refreshPreview();
}

void BatchExport::refreshPreview()
{
    if (!_desktop) return;

    // For Batch Export we are now hiding all object except current object
    bool hide = hide_all->get_active();
    bool preview = show_preview->get_active();
    preview_container->set_orientation(preview ? Gtk::ORIENTATION_HORIZONTAL : Gtk::ORIENTATION_VERTICAL);

    for (auto &[key, val] : current_items) {
        if (preview) {
            if (!hide) {
                val->refreshHide(nullptr);
            } else if (auto item = val->getItem()) {
                std::vector<SPItem *> selected = {item};
                val->refreshHide(&selected);
            } else if (val->getPage()) {
                auto sels = _desktop->getSelection()->items();
                std::vector<SPItem *> selected(sels.begin(), sels.end());
                val->refreshHide(&selected);
            }
        }
        val->refresh(!preview);
    }
}

void BatchExport::loadExportHints()
{
    SPDocument *doc = _desktop->getDocument();
    auto old_filename = filename_entry->get_text();
    if (old_filename.empty()) {
        Glib::ustring filename = doc->getRoot()->getExportFilename();
        if (filename.empty()) {
            Glib::ustring filename_entry_text = filename_entry->get_text();
            Glib::ustring extension = ".png";
            filename = Export::defaultFilename(doc, original_name, extension);
        }
        filename_entry->set_text(filename);
        filename_entry->set_position(filename.length());
        doc_export_name = filename;
    }
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
    loadExportHints();
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

    // If there are no selected button, simply flash message in status bar
    int num = current_items.size();
    if (current_items.size() == 0) {
        _desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No items selected."));
        export_btn->set_sensitive(true);
        return;
    }

    // Find and remove any extension from filename so that we can add suffix to it.
    Glib::ustring filename = filename_entry->get_text();
    export_list->removeExtension(filename);

    // create vector of exports
    int num_rows = export_list->get_rows();
    std::vector<Glib::ustring> suffixs;
    std::vector<Inkscape::Extension::Output *> extensions;
    std::vector<double> dpis;
    for (int i = 0; i < num_rows; i++) {
        suffixs.push_back(export_list->get_suffix(i));
        extensions.push_back(export_list->getExtension(i));
        dpis.push_back(export_list->get_dpi(i));
    }

    // We are exporting standalone items only for now
    // std::vector<SPItem *> selected(_desktop->getSelection()->items().begin(),
    // _desktop->getSelection()->items().end());
    bool hide = hide_all->get_active();

    auto sels = _desktop->getSelection()->items();
    std::vector<SPItem *> selected_items(sels.begin(), sels.end());

    // Start Exporting Each Item
    for (int i = 0; i < num_rows; i++) {
        auto suffix = suffixs[i];
        auto omod = extensions[i];
        float dpi = dpis[i];

        if (!omod || omod->deactivated() || !omod->prefs()) {
            continue;
        }

        int count = 0;
        for (auto i = current_items.begin(); i != current_items.end() && !interrupted; ++i) {
            count++;

            BatchItem *batchItem = i->second;
            if (!batchItem->isActive()) {
                continue;
            }

            SPItem *item = batchItem->getItem();
            SPPage *page = batchItem->getPage();

            std::vector<SPItem *> show_only;
            Geom::Rect area;
            if (item) {
                if (auto bounds = item->documentVisualBounds()) {
                    area = *bounds;
                } else {
                    continue;
                }
                show_only.emplace_back(item);
            } else if (page) {
                area = page->getDesktopRect();
                show_only = selected_items; // Maybe stuff here
            } else {
                continue;
            }

            Glib::ustring id = batchItem->getLabel();
            if (id.empty()) {
                continue;
            }

            Glib::ustring item_filename = filename + "_" + id;
            if (!suffix.empty()) {
                if (omod->is_raster()) {
                    // Put the dpi in at the user's requested location.
                    suffix = std::regex_replace(suffix.c_str(), std::regex("\\{dpi\\}"), std::to_string((int)dpi));
                }
                item_filename = item_filename + "_" + suffix;
            }

            bool found = Export::unConflictFilename(_document, item_filename, omod->get_extension());
            if (!found) {
                continue;
            }

            delete prog_dlg;
            prog_dlg = create_progress_dialog(Glib::ustring::compose(_("Exporting %1 files"), num));
            prog_dlg->set_export_panel(this);
            setExporting(true, Glib::ustring::compose(_("Exporting %1 files"), num));
            prog_dlg->set_current(count);
            prog_dlg->set_total(num);

            onProgressCallback(0.0, prog_dlg);

            if (omod->is_raster()) {
                unsigned long int width = (int)(area.width() * dpi / DPI_BASE + 0.5);
                unsigned long int height = (int)(area.height() * dpi / DPI_BASE + 0.5);

                Export::exportRaster(
                    area, width, height, dpi, item_filename, true, onProgressCallback,
                    prog_dlg, omod, hide ? &show_only : nullptr);
            } else {
                setExporting(true, Glib::ustring::compose(_("Exporting %1"), filename));
                auto copy_doc = _document->copy();
                Export::exportVector(omod, copy_doc.get(), item_filename, true, &show_only, page);
            }
            setExporting(false);

            if (prog_dlg) {
                delete prog_dlg;
                prog_dlg = nullptr;
            }
        }
    }
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
        filename = Export::defaultFilename(_document, filename, ".png");
    }

    Inkscape::UI::Dialog::FileSaveDialog *dialog = Inkscape::UI::Dialog::FileSaveDialog::create(
        *window, filename, Inkscape::UI::Dialog::RASTER_TYPES, _("Select a filename for exporting"), "", "",
        Inkscape::Extension::FILE_SAVE_METHOD_EXPORT);

    if (dialog->show()) {
        filename = dialog->getFilename();
        // Remove extension and don't add a new one, for obvious reasons.
        export_list->removeExtension(filename);

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
        if (auto _sel = _desktop->getSelection()) {
            selection_buttons[SELECTION_SELECTION]->set_sensitive(!_sel->isEmpty());
        }
        selection_buttons[SELECTION_PAGE]->set_sensitive(_document->getPageManager().hasPages());
    }
    if (!selection_buttons[current_key]->get_sensitive()) {
        current_key = SELECTION_LAYER;
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

void BatchExport::setDesktop(SPDesktop *desktop)
{
    if (desktop != _desktop) {
        _pages_changed_connection.disconnect();
        _desktop = desktop;
    }
}

void BatchExport::setDocument(SPDocument *document)
{
    if (!_desktop) {
        document = nullptr;
    }

    _document = document;
    _pages_changed_connection.disconnect();
    if (document) {
        // when the page selected is changes, update the export area
        _pages_changed_connection = document->getPageManager().connectPagesChanged([=]() { pagesChanged(); });
    }
    for (auto &[key, val] : current_items) {
        val->setDocument(document);
    }
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
