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

#include "export-single.h"

#include <glibmm/convert.h>
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <gtkmm.h>
#include <png.h>

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
#include "message-stack.h"
#include "object/object-set.h"
#include "object/sp-namedview.h"
#include "object/sp-root.h"
#include "object/sp-page.h"
#include "page-manager.h"
#include "preferences.h"
#include "selection-chemistry.h"
#include "ui/dialog-events.h"
#include "ui/dialog/dialog-notebook.h"
#include "ui/dialog/export.h"
#include "ui/dialog/filedialog.h"
#include "ui/icon-names.h"
#include "ui/interface.h"
#include "ui/widget/export-lists.h"
#include "ui/widget/export-preview.h"
#include "ui/widget/scrollprotected.h"
#include "ui/widget/unit-menu.h"
#ifdef _WIN32

#endif

using Inkscape::Util::unit_table;

namespace Inkscape {
namespace UI {
namespace Dialog {


/**
 * Initialise Builder Objects. Called in Export constructor.
 */
void SingleExport::initialise(const Glib::RefPtr<Gtk::Builder> &builder)
{
    builder->get_widget("si_s_document", selection_buttons[SELECTION_DRAWING]);
    selection_names[SELECTION_DRAWING] = "drawing";
    builder->get_widget("si_s_page", selection_buttons[SELECTION_PAGE]);
    selection_names[SELECTION_PAGE] = "page";
    builder->get_widget("si_s_selection", selection_buttons[SELECTION_SELECTION]);
    selection_names[SELECTION_SELECTION] = "selection";
    builder->get_widget("si_s_custom", selection_buttons[SELECTION_CUSTOM]);
    selection_names[SELECTION_CUSTOM] = "custom";

    builder->get_widget_derived("si_left_sb", spin_buttons[SPIN_X0]);
    builder->get_widget_derived("si_right_sb", spin_buttons[SPIN_X1]);
    builder->get_widget_derived("si_top_sb", spin_buttons[SPIN_Y0]);
    builder->get_widget_derived("si_bottom_sb", spin_buttons[SPIN_Y1]);
    builder->get_widget_derived("si_height_sb", spin_buttons[SPIN_HEIGHT]);
    builder->get_widget_derived("si_width_sb", spin_buttons[SPIN_WIDTH]);

    builder->get_widget("si_label_left", spin_labels[SPIN_X0]);
    builder->get_widget("si_label_right", spin_labels[SPIN_X1]);
    builder->get_widget("si_label_top", spin_labels[SPIN_Y0]);
    builder->get_widget("si_label_bottom", spin_labels[SPIN_Y1]);
    builder->get_widget("si_label_height", spin_labels[SPIN_HEIGHT]);
    builder->get_widget("si_label_width", spin_labels[SPIN_WIDTH]);

    builder->get_widget_derived("si_img_height_sb", spin_buttons[SPIN_BMHEIGHT]);
    builder->get_widget_derived("si_img_width_sb", spin_buttons[SPIN_BMWIDTH]);
    builder->get_widget_derived("si_dpi_sb", spin_buttons[SPIN_DPI]);

    builder->get_widget("page_prev", page_prev);
    page_prev->signal_clicked().connect([=]() {
        if (_document && _document->getPageManager().selectPrevPage()) {
            _document->getPageManager().zoomToSelectedPage(_desktop);
        }
    });

    builder->get_widget("page_next", page_next);
    page_next->signal_clicked().connect([=]() {
        if (_document && _document->getPageManager().selectNextPage()) {
            _document->getPageManager().zoomToSelectedPage(_desktop);
        }
    });

    // builder->get_widget("si_show_export_area", show_export_area);
    builder->get_widget_derived("si_units", units);
    builder->get_widget("si_units_row", si_units_row);
    builder->get_widget("si_area_name", si_name_label);

    builder->get_widget("si_hide_all", si_hide_all);
    builder->get_widget("si_show_preview", si_show_preview);
    builder->get_widget("si_default_opts", si_default_opts);
    builder->get_widget_derived("si_preview", preview);

    builder->get_widget_derived("si_extention", si_extension_cb);
    builder->get_widget("si_filename", si_filename_entry);
    builder->get_widget("si_export", si_export);

    builder->get_widget("si_progress", _prog);
}

// Inkscape Selection Modified CallBack
void SingleExport::selectionModified(Inkscape::Selection *selection, guint flags)
{
    if (!_desktop || _desktop->getSelection() != selection) {
        return;
    }
    if (!(flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
        return;
    }
    if (!_document) {
        refreshArea();
        loadExportHints();
    }
}

void SingleExport::selectionChanged(Inkscape::Selection *selection)
{
    if (!_desktop || _desktop->getSelection() != selection) {
        return;
    }
    Glib::ustring pref_key_name = prefs->getString("/dialogs/export/exportarea/value");
    for (auto [key, name] : selection_names) {
        if (name == pref_key_name && current_key != key && key != SELECTION_SELECTION) {
            selection_buttons[key]->set_active(true);
            current_key = key;
            break;
        }
    }
    if (selection->isEmpty()) {
        selection_buttons[SELECTION_SELECTION]->set_sensitive(false);
        if (current_key == SELECTION_SELECTION) {
            selection_buttons[(selection_mode)0]->set_active(true); // This causes refresh area
            // even though we are at default key, selection is the one which was original key.
            prefs->setString("/dialogs/export/exportarea/value", selection_names[SELECTION_SELECTION]);
            // return otherwise refreshArea will be called again
            return;
        }
    } else {
        selection_buttons[SELECTION_SELECTION]->set_sensitive(true);
        if (selection_names[SELECTION_SELECTION] == pref_key_name && current_key != SELECTION_SELECTION) {
            selection_buttons[SELECTION_SELECTION]->set_active();
            return;
        }
    }

    refreshArea();
    loadExportHints();
}

// Setup Single Export.Called by export on realize
void SingleExport::setup()
{
    if (setupDone) {
        // We need to setup only once
        return;
    }
    setupDone = true;
    prefs = Inkscape::Preferences::get();
    si_extension_cb->setup();

    setupUnits();
    setupSpinButtons();

    // set them before connecting to signals
    setDefaultSelectionMode();

    // Refresh values to sync them with defaults.
    refreshArea();
    refreshPage();
    loadExportHints();

    // Connect Signals Here
    for (auto [key, button] : selection_buttons) {
        button->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &SingleExport::onAreaTypeToggle), key));
    }
    units->signal_changed().connect(sigc::mem_fun(*this, &SingleExport::onUnitChanged));
    extensionConn = si_extension_cb->signal_changed().connect(sigc::mem_fun(*this, &SingleExport::onExtensionChanged));
    exportConn = si_export->signal_clicked().connect(sigc::mem_fun(*this, &SingleExport::onExport));
    filenameConn = si_filename_entry->signal_changed().connect(sigc::mem_fun(*this, &SingleExport::onFilenameModified));
    browseConn = si_filename_entry->signal_icon_press().connect(sigc::mem_fun(*this, &SingleExport::onBrowse));
    si_filename_entry->signal_activate().connect(sigc::mem_fun(*this, &SingleExport::onExport));
    si_show_preview->signal_toggled().connect(sigc::mem_fun(*this, &SingleExport::refreshPreview));
    si_hide_all->signal_toggled().connect(sigc::mem_fun(*this, &SingleExport::refreshPreview));

    si_default_opts->set_active(prefs->getBool("/dialogs/export/defaultopts", true));
}

// Setup units combobox
void SingleExport::setupUnits()
{
    units->setUnitType(Inkscape::Util::UNIT_TYPE_LINEAR);
    if (_desktop) {
        units->setUnit(_desktop->getNamedView()->display_units->abbr);
    }
}

// Create all spin buttons
void SingleExport::setupSpinButtons()
{
    setupSpinButton<sb_type>(spin_buttons[SPIN_X0], 0.0, -1000000.0, 1000000.0, 0.1, 1.0, EXPORT_COORD_PRECISION, true,
                             &SingleExport::onAreaXChange, SPIN_X0);
    setupSpinButton<sb_type>(spin_buttons[SPIN_X1], 0.0, -1000000.0, 1000000.0, 0.1, 1.0, EXPORT_COORD_PRECISION, true,
                             &SingleExport::onAreaXChange, SPIN_X1);
    setupSpinButton<sb_type>(spin_buttons[SPIN_Y0], 0.0, -1000000.0, 1000000.0, 0.1, 1.0, EXPORT_COORD_PRECISION, true,
                             &SingleExport::onAreaYChange, SPIN_Y0);
    setupSpinButton<sb_type>(spin_buttons[SPIN_Y1], 0.0, -1000000.0, 1000000.0, 0.1, 1.0, EXPORT_COORD_PRECISION, true,
                             &SingleExport::onAreaYChange, SPIN_Y1);

    setupSpinButton<sb_type>(spin_buttons[SPIN_HEIGHT], 0.0, 0.0, PNG_UINT_31_MAX, 0.1, 1.0, EXPORT_COORD_PRECISION,
                             true, &SingleExport::onAreaYChange, SPIN_HEIGHT);
    setupSpinButton<sb_type>(spin_buttons[SPIN_WIDTH], 0.0, 0.0, PNG_UINT_31_MAX, 0.1, 1.0, EXPORT_COORD_PRECISION,
                             true, &SingleExport::onAreaXChange, SPIN_WIDTH);

    setupSpinButton<sb_type>(spin_buttons[SPIN_BMHEIGHT], 1.0, 1.0, 1000000.0, 1.0, 10.0, 0, true,
                             &SingleExport::onDpiChange, SPIN_BMHEIGHT);
    setupSpinButton<sb_type>(spin_buttons[SPIN_BMWIDTH], 1.0, 1.0, 1000000.0, 1.0, 10.0, 0, true,
                             &SingleExport::onDpiChange, SPIN_BMWIDTH);
    setupSpinButton<sb_type>(spin_buttons[SPIN_DPI], prefs->getDouble("/dialogs/export/defaultxdpi/value", DPI_BASE),
                             1.0, 100000.0, 0.1, 1.0, 2, true, &SingleExport::onDpiChange, SPIN_DPI);
}

template <typename T>
void SingleExport::setupSpinButton(Gtk::SpinButton *sb, double val, double min, double max, double step, double page,
                                   int digits, bool sensitive, void (SingleExport::*cb)(T), T param)
{
    if (sb) {
        sb->set_digits(digits);
        sb->set_increments(step, page);
        sb->set_range(min, max);
        sb->set_value(val);
        sb->set_sensitive(sensitive);
        sb->set_width_chars(0);
        sb->set_max_width_chars(0);
        if (cb) {
            auto signal = sb->signal_value_changed().connect(sigc::bind(sigc::mem_fun(*this, cb), param));
            // add signals to list to block all easily
            spinButtonConns.push_back(signal);
        }
    }
}

void SingleExport::refreshArea()
{
    if (_document) {
        Geom::OptRect bbox;
        _document->ensureUpToDate();

        switch (current_key) {
            case SELECTION_SELECTION:
                if ((_desktop->getSelection())->isEmpty() == false) {
                    bbox = _desktop->getSelection()->visualBounds();
                    break;
                }
            case SELECTION_DRAWING:
                bbox = _document->getRoot()->desktopVisualBounds();
                if (bbox) {
                    break;
                }
            case SELECTION_PAGE:
                bbox = _document->getPageManager().getSelectedPageRect();
                break;
            case SELECTION_CUSTOM:
                break;
            default:
                break;
        }
        if (current_key != SELECTION_CUSTOM && bbox) {
            setArea(bbox->min()[Geom::X], bbox->min()[Geom::Y], bbox->max()[Geom::X], bbox->max()[Geom::Y]);
        }
    }
    refreshPreview();
}

void SingleExport::refreshPage()
{
    bool pages = current_key == SELECTION_PAGE;
    si_name_label->set_visible(pages);
    page_prev->set_visible(pages);
    page_next->set_visible(pages);

    auto &page_manager = _document->getPageManager();
    page_prev->set_sensitive(page_manager.hasPrevPage());
    page_next->set_sensitive(page_manager.hasNextPage());

    if (auto page = page_manager.getSelected()) {
        if (auto label = page->label()) {
            si_name_label->set_text(label);
        } else {
            si_name_label->set_text(page->getDefaultLabel());
        }
    } else {
        si_name_label->set_text(_("First Page"));
    }
}

void SingleExport::loadExportHints()
{
    if (filename_modified) return;

    Glib::ustring old_filename = si_filename_entry->get_text();
    Glib::ustring filename;
    Geom::Point dpi;
    switch (current_key) {
        case SELECTION_PAGE:
            if (auto page = _document->getPageManager().getSelected()) {
                dpi = page->getExportDpi();
                filename = page->getExportFilename();
                if (filename.empty()) {
                    filename = Export::filePathFromId(_document, page->getLabel(), old_filename);
                }
                break;
            }
            // No page means page is drawing, continue.
        case SELECTION_CUSTOM:
        case SELECTION_DRAWING:
        {
            dpi = _document->getRoot()->getExportDpi();
            filename = _document->getRoot()->getExportFilename();
            break;
        }
        case SELECTION_SELECTION:
        {
            auto selection = _desktop->getSelection();
            if (selection->isEmpty()) break;

            // Get filename and dpi from selected items
            for (auto item : selection->items()) {
                if (!dpi.x()) {
                    dpi = item->getExportDpi();
                }
                if (filename.empty()) {
                    filename = item->getExportFilename();
                }
            }

            if (filename.empty()) {
                filename = Export::filePathFromObject(_document, selection->firstItem(), old_filename);
            }
            break;
        }
        default:
            break;
    }
    if (filename.empty()) {
        filename = Export::defaultFilename(_document, old_filename, ".png");
    }
    if (auto ext = si_extension_cb->getExtension()) {
        si_extension_cb->removeExtension(filename);
        ext->add_extension(filename);
    }

    original_name = filename;
    si_filename_entry->set_text(filename);
    si_filename_entry->set_position(filename.length());

    if (dpi.x() != 0.0) { // XXX Should this deal with dpi.y() ?
        spin_buttons[SPIN_DPI]->set_value(dpi.x());
    }
}

void SingleExport::saveExportHints(SPObject *target)
{
    if (target) {
        target->setExportFilename(si_filename_entry->get_text());
        target->setExportDpi(Geom::Point(
            spin_buttons[SPIN_DPI]->get_value(),
            spin_buttons[SPIN_DPI]->get_value()
        ));
    }
}

void SingleExport::setArea(double x0, double y0, double x1, double y1)
{
    blockSpinConns(true);

    Unit const *unit = units->getUnit();
    auto px = unit_table.getUnit("px");
    spin_buttons[SPIN_X0]->get_adjustment()->set_value(px->convert(x0, unit));
    spin_buttons[SPIN_X1]->get_adjustment()->set_value(px->convert(x1, unit));
    spin_buttons[SPIN_Y0]->get_adjustment()->set_value(px->convert(y0, unit));
    spin_buttons[SPIN_Y1]->get_adjustment()->set_value(px->convert(y1, unit));

    areaXChange(SPIN_X1);
    areaYChange(SPIN_Y1);

    blockSpinConns(false);
}

// Signals CallBack

void SingleExport::onUnitChanged()
{
    refreshArea();
}

void SingleExport::onAreaTypeToggle(selection_mode key)
{
    // Prevent executing function twice
    if (!selection_buttons[key]->get_active()) {
        return;
    }
    // If you have reached here means the current key is active one ( not sure if multiple transitions happen but
    // last call will change values)
    current_key = key;
    prefs->setString("/dialogs/export/exportarea/value", selection_names[current_key]);

    refreshPage();
    refreshArea();
    loadExportHints();
    toggleSpinButtonVisibility();
}

void SingleExport::toggleSpinButtonVisibility()
{
    bool show = current_key == SELECTION_CUSTOM;
    spin_buttons[SPIN_X0]->set_visible(show);
    spin_buttons[SPIN_X1]->set_visible(show);
    spin_buttons[SPIN_Y0]->set_visible(show);
    spin_buttons[SPIN_Y1]->set_visible(show);
    spin_buttons[SPIN_WIDTH]->set_visible(show);
    spin_buttons[SPIN_HEIGHT]->set_visible(show);

    spin_labels[SPIN_X0]->set_visible(show);
    spin_labels[SPIN_X1]->set_visible(show);
    spin_labels[SPIN_Y0]->set_visible(show);
    spin_labels[SPIN_Y1]->set_visible(show);
    spin_labels[SPIN_WIDTH]->set_visible(show);
    spin_labels[SPIN_HEIGHT]->set_visible(show);

    si_units_row->set_visible(show);
}

void SingleExport::onAreaXChange(sb_type type)
{
    blockSpinConns(true);
    areaXChange(type);
    selection_buttons[SELECTION_CUSTOM]->set_active(true);
    refreshPreview();
    blockSpinConns(false);
}
void SingleExport::onAreaYChange(sb_type type)
{
    blockSpinConns(true);
    areaYChange(type);
    selection_buttons[SELECTION_CUSTOM]->set_active(true);
    refreshPreview();
    blockSpinConns(false);
}
void SingleExport::onDpiChange(sb_type type)
{
    blockSpinConns(true);
    dpiChange(type);
    blockSpinConns(false);
}

void SingleExport::onFilenameModified()
{
    extensionConn.block();
    Glib::ustring filename = si_filename_entry->get_text();

    if (original_name == filename) {
        filename_modified = false;
    } else {
        filename_modified = true;
    }

    si_extension_cb->setExtensionFromFilename(filename);

    extensionConn.unblock();
}

void SingleExport::onExtensionChanged()
{
    filenameConn.block();
    Glib::ustring filename = si_filename_entry->get_text();
    if (auto ext = si_extension_cb->getExtension()) {
        si_extension_cb->removeExtension(filename);
        ext->add_extension(filename);
    }
    si_filename_entry->set_text(filename);
    si_filename_entry->set_position(filename.length());
    filenameConn.unblock();
}

void SingleExport::onExport()
{
    interrupted = false;
    if (!_desktop || !_document)
        return;

    auto &page_manager = _document->getPageManager();
    auto selection = _desktop->getSelection();
    si_export->set_sensitive(false);
    bool exportSuccessful = false;
    auto omod = si_extension_cb->getExtension();
    if (!omod) {
        si_export->set_sensitive(true);
        return;
    }

    bool selected_only = si_hide_all->get_active();
    Unit const *unit = units->getUnit();
    Glib::ustring filename = si_filename_entry->get_text();

    float x0 = unit->convert(spin_buttons[SPIN_X0]->get_value(), "px");
    float x1 = unit->convert(spin_buttons[SPIN_X1]->get_value(), "px");
    float y0 = unit->convert(spin_buttons[SPIN_Y0]->get_value(), "px");
    float y1 = unit->convert(spin_buttons[SPIN_Y1]->get_value(), "px");
    auto area = Geom::Rect(Geom::Point(x0, y0), Geom::Point(x1, y1));

    bool default_opts = si_default_opts->get_active();
    prefs->setBool("/dialogs/export/defaultopts", default_opts);
    if (!default_opts && !omod->prefs()) {
        si_export->set_sensitive(true);
        return; // cancel button
    }

    if (omod->is_raster()) {
        area *= _desktop->dt2doc();
        unsigned long int width = int(spin_buttons[SPIN_BMWIDTH]->get_value() + 0.5);
        unsigned long int height = int(spin_buttons[SPIN_BMHEIGHT]->get_value() + 0.5);

        float dpi = spin_buttons[SPIN_DPI]->get_value();

        /* TRANSLATORS: %1 will be the filename, %2 the width, and %3 the height of the image */
        prog_dlg = create_progress_dialog(Glib::ustring::compose(_("Exporting %1 (%2 x %3)"), filename, width, height));
        prog_dlg->set_export_panel(this);
        setExporting(true, Glib::ustring::compose(_("Exporting %1 (%2 x %3)"), filename, width, height));
        prog_dlg->set_current(0);
        prog_dlg->set_total(0);

        std::vector<SPItem *> selected(selection->items().begin(), selection->items().end());

        exportSuccessful = Export::exportRaster(
            area, width, height, dpi, filename, false, onProgressCallback, prog_dlg,
            omod, selected_only ? &selected : nullptr);

    } else {
        setExporting(true, Glib::ustring::compose(_("Exporting %1"), filename));

        auto copy_doc = _document->copy();

        std::vector<SPItem *> items;
        if (selected_only) {
            auto itemlist = selection->items();
            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                SPItem *item = *i;
                items.push_back(item);
            }
        }

        SPPage *page;
        if (current_key == SELECTION_PAGE && page_manager.hasPages()) {
            page = page_manager.getSelected();
        } else {
            // To get the right kind of export, we're going to make a page
            // This allows all the same raster options to work for vectors
            page = copy_doc->getPageManager().newDocumentPage(area);
        }

        exportSuccessful = Export::exportVector(omod, copy_doc.get(), filename, false, &items, page);
    }
    if (prog_dlg) {
        delete prog_dlg;
        prog_dlg = nullptr;
    }
    // Save the export hints back to the svg document
    if (exportSuccessful) {
        SPObject *target;
        switch (current_key) {
            case SELECTION_CUSTOM:
            case SELECTION_DRAWING:
                target = _document->getRoot();
                break;
            case SELECTION_PAGE:
                target = page_manager.getSelected();
                if (!target)
                    target = _document->getRoot();
                break;
            case SELECTION_SELECTION:
                target = _desktop->getSelection()->firstItem();
                break;
            default:
                break;
        }
        if (target) {
            saveExportHints(target);
            DocumentUndo::done(_document, _("Set Export Options"), INKSCAPE_ICON("export"));
        }
    }
    setExporting(false);
    si_export->set_sensitive(true);
    original_name = filename;
    filename_modified = false;
    interrupted = false;
}

void SingleExport::onBrowse(Gtk::EntryIconPosition pos, const GdkEventButton *ev)
{
    if (!_app || !_document) {
        return;
    }
    Gtk::Window *window = _app->get_active_window();
    browseConn.block();
    Glib::ustring filename = Glib::filename_from_utf8(si_filename_entry->get_text());

    if (filename.empty()) {
        filename = Export::defaultFilename(_document, filename, ".png");
    }

    Inkscape::UI::Dialog::FileSaveDialog *dialog = Inkscape::UI::Dialog::FileSaveDialog::create(
        *window, filename, Inkscape::UI::Dialog::RASTER_TYPES, _("Select a filename for exporting"), "", "",
        Inkscape::Extension::FILE_SAVE_METHOD_EXPORT);

    if (dialog->show()) {
        filename = dialog->getFilename();
        if (auto ext = si_extension_cb->getExtension()) {
            si_extension_cb->removeExtension(filename);
            ext->add_extension(filename);
        }
        si_filename_entry->set_text(filename);
        si_filename_entry->set_position(filename.length());
        // deleting dialog before exporting is important
        delete dialog;
        onExport();
    } else {
        delete dialog;
    }
    browseConn.unblock();
}

// Utils Functions

void SingleExport::blockSpinConns(bool status = true)
{
    for (auto signal : spinButtonConns) {
        if (status) {
            signal.block();
        } else {
            signal.unblock();
        }
    }
}

void SingleExport::areaXChange(sb_type type)
{
    auto x0_adj = spin_buttons[SPIN_X0]->get_adjustment();
    auto x1_adj = spin_buttons[SPIN_X1]->get_adjustment();
    auto width_adj = spin_buttons[SPIN_WIDTH]->get_adjustment();

    float x0, x1, dpi, width, bmwidth;

    // Get all values in px
    Unit const *unit = units->getUnit();
    x0 = unit->convert(x0_adj->get_value(), "px");
    x1 = unit->convert(x1_adj->get_value(), "px");
    width = unit->convert(width_adj->get_value(), "px");
    bmwidth = spin_buttons[SPIN_BMWIDTH]->get_value();
    dpi = spin_buttons[SPIN_DPI]->get_value();

    switch (type) {
        case SPIN_X0:
            bmwidth = (x1 - x0) * dpi / DPI_BASE;
            if (bmwidth < SP_EXPORT_MIN_SIZE) {
                x0 = x1 - (SP_EXPORT_MIN_SIZE * DPI_BASE) / dpi;
            }
            break;
        case SPIN_X1:
            bmwidth = (x1 - x0) * dpi / DPI_BASE;
            if (bmwidth < SP_EXPORT_MIN_SIZE) {
                x1 = x0 + (SP_EXPORT_MIN_SIZE * DPI_BASE) / dpi;
            }
            break;
        case SPIN_WIDTH:
            bmwidth = width * dpi / DPI_BASE;
            if (bmwidth < SP_EXPORT_MIN_SIZE) {
                width = (SP_EXPORT_MIN_SIZE * DPI_BASE) / dpi;
            }
            x1 = x0 + width;
            break;
        default:
            break;
    }

    width = x1 - x0;
    bmwidth = floor(width * dpi / DPI_BASE + 0.5);

    auto px = unit_table.getUnit("px");
    x0_adj->set_value(px->convert(x0, unit));
    x1_adj->set_value(px->convert(x1, unit));
    width_adj->set_value(px->convert(width, unit));
    spin_buttons[SPIN_BMWIDTH]->set_value(bmwidth);
}

void SingleExport::areaYChange(sb_type type)
{
    auto y0_adj = spin_buttons[SPIN_Y0]->get_adjustment();
    auto y1_adj = spin_buttons[SPIN_Y1]->get_adjustment();
    auto height_adj = spin_buttons[SPIN_HEIGHT]->get_adjustment();

    float y0, y1, dpi, height, bmheight;

    // Get all values in px
    Unit const *unit = units->getUnit();
    y0 = unit->convert(y0_adj->get_value(), "px");
    y1 = unit->convert(y1_adj->get_value(), "px");
    height = unit->convert(height_adj->get_value(), "px");
    bmheight = spin_buttons[SPIN_BMHEIGHT]->get_value();
    dpi = spin_buttons[SPIN_DPI]->get_value();

    switch (type) {
        case SPIN_Y0:
            bmheight = (y1 - y0) * dpi / DPI_BASE;
            if (bmheight < SP_EXPORT_MIN_SIZE) {
                y0 = y1 - (SP_EXPORT_MIN_SIZE * DPI_BASE) / dpi;
            }
            break;
        case SPIN_Y1:
            bmheight = (y1 - y0) * dpi / DPI_BASE;
            if (bmheight < SP_EXPORT_MIN_SIZE) {
                y1 = y0 + (SP_EXPORT_MIN_SIZE * DPI_BASE) / dpi;
            }
            break;
        case SPIN_HEIGHT:
            bmheight = height * dpi / DPI_BASE;
            if (bmheight < SP_EXPORT_MIN_SIZE) {
                height = (SP_EXPORT_MIN_SIZE * DPI_BASE) / dpi;
            }
            y1 = y0 + height;
            break;
        default:
            break;
    }

    height = y1 - y0;
    bmheight = floor(height * dpi / DPI_BASE + 0.5);

    auto px = unit_table.getUnit("px");
    y0_adj->set_value(px->convert(y0, unit));
    y1_adj->set_value(px->convert(y1, unit));
    height_adj->set_value(px->convert(height, unit));
    spin_buttons[SPIN_BMHEIGHT]->set_value(bmheight);
}

void SingleExport::dpiChange(sb_type type)
{
    float dpi, height, width, bmheight, bmwidth;

    // Get all values in px
    Unit const *unit = units->getUnit();
    height = unit->convert(spin_buttons[SPIN_HEIGHT]->get_value(), "px");
    width = unit->convert(spin_buttons[SPIN_WIDTH]->get_value(), "px");
    bmheight = spin_buttons[SPIN_BMHEIGHT]->get_value();
    bmwidth = spin_buttons[SPIN_BMWIDTH]->get_value();
    dpi = spin_buttons[SPIN_DPI]->get_value();

    switch (type) {
        case SPIN_BMHEIGHT:
            if (bmheight < SP_EXPORT_MIN_SIZE) {
                bmheight = SP_EXPORT_MIN_SIZE;
            }
            dpi = bmheight * DPI_BASE / height;
            break;
        case SPIN_BMWIDTH:
            if (bmwidth < SP_EXPORT_MIN_SIZE) {
                bmwidth = SP_EXPORT_MIN_SIZE;
            }
            dpi = bmwidth * DPI_BASE / width;
            break;
        case SPIN_DPI:
            prefs->setDouble("/dialogs/export/defaultdpi/value", dpi);
            break;
        default:
            break;
    }

    bmwidth = floor(width * dpi / DPI_BASE + 0.5);
    bmheight = floor(height * dpi / DPI_BASE + 0.5);

    spin_buttons[SPIN_BMHEIGHT]->set_value(bmheight);
    spin_buttons[SPIN_BMWIDTH]->set_value(bmwidth);
    spin_buttons[SPIN_DPI]->set_value(dpi);
}

void SingleExport::setDefaultSelectionMode()
{
    current_key = (selection_mode)0; // default key
    bool found = false;
    Glib::ustring pref_key_name = prefs->getString("/dialogs/export/exportarea/value");
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
        if (current_key == SELECTION_CUSTOM &&
            (spin_buttons[SPIN_HEIGHT]->get_value() == 0 || spin_buttons[SPIN_WIDTH]->get_value() == 0)) {
            Geom::OptRect bbox = _document->preferredBounds();
            setArea(bbox->min()[Geom::X], bbox->min()[Geom::Y], bbox->max()[Geom::X], bbox->max()[Geom::Y]);
        }
    } else {
        current_key = (selection_mode)0;
    }
    selection_buttons[current_key]->set_active(true);
    prefs->setString("/dialogs/export/exportarea/value", pref_key_name);

    toggleSpinButtonVisibility();
}

void SingleExport::setExporting(bool exporting, Glib::ustring const &text)
{
    if (exporting) {
        _prog->set_text(text);
        _prog->set_fraction(0.0);
        _prog->set_sensitive(true);
        si_export->set_sensitive(false);
    } else {
        _prog->set_text("");
        _prog->set_fraction(0.0);
        _prog->set_sensitive(false);
        si_export->set_sensitive(true);
    }
}

ExportProgressDialog *SingleExport::create_progress_dialog(Glib::ustring progress_text)
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

    btn->signal_clicked().connect(sigc::mem_fun(*this, &SingleExport::onProgressCancel));
    dlg->signal_delete_event().connect(sigc::mem_fun(*this, &SingleExport::onProgressDelete));

    dlg->show_all();
    return dlg;
}

/// Called when dialog is deleted
bool SingleExport::onProgressDelete(GdkEventAny * /*event*/)
{
    interrupted = true;
    prog_dlg->set_stopped();
    return TRUE;
}
/// Called when progress is cancelled
void SingleExport::onProgressCancel()
{
    interrupted = true;
    prog_dlg->set_stopped();
}

// Called for every progress iteration
unsigned int SingleExport::onProgressCallback(float value, void *dlg)
{
    auto dlg2 = reinterpret_cast<ExportProgressDialog *>(dlg);

    auto self = dynamic_cast<SingleExport *>(dlg2->get_export_panel());

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

void SingleExport::refreshPreview()
{
    if (!_desktop) {
        return;
    }
    if (!si_show_preview->get_active()) {
        preview->resetPixels();
        return;
    }

    std::vector<SPItem *> selected(_desktop->getSelection()->items().begin(), _desktop->getSelection()->items().end());
    bool hide = si_hide_all->get_active();

    Unit const *unit = units->getUnit();
    float x0 = unit->convert(spin_buttons[SPIN_X0]->get_value(), "px");
    float x1 = unit->convert(spin_buttons[SPIN_X1]->get_value(), "px");
    float y0 = unit->convert(spin_buttons[SPIN_Y0]->get_value(), "px");
    float y1 = unit->convert(spin_buttons[SPIN_Y1]->get_value(), "px");
    preview->setDbox(x0, x1, y0, y1);
    preview->refreshHide(hide ? &selected : nullptr);
    preview->queueRefresh();
}

void SingleExport::setDesktop(SPDesktop *desktop)
{
    if (desktop != _desktop) {
        _page_selected_connection.disconnect();
        _desktop = desktop;
    }
}

void SingleExport::setDocument(SPDocument *document)
{
    _document = document;
    _page_selected_connection.disconnect();
    if (document) {
        // when the page selected is changes, update the export area
        _page_selected_connection = document->getPageManager().connectPageSelected([=](SPPage *page) {
            refreshPage();
            refresh();
        });
    }
    preview->setDocument(document);
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
