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
#include "export-helper.h"
#include "extension/db.h"
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

SingleExport::~SingleExport()
{
    ;
}

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

    // builder->get_widget("si_show_export_area", show_export_area);
    builder->get_widget_derived("si_units", units);
    builder->get_widget("si_units_row", si_units_row);

    builder->get_widget("si_hide_all", si_hide_all);
    builder->get_widget("si_preview_box", si_preview_box);
    builder->get_widget("si_show_preview", si_show_preview);

    builder->get_widget_derived("si_extention", si_extension_cb);
    builder->get_widget("si_filename", si_filename_entry);
    builder->get_widget("si_export", si_export);

    builder->get_widget("si_progress", _prog);

    builder->get_widget("si_advance_box", adv_box);

    Inkscape::UI::Widget::ScrollTransfer<Gtk::ScrolledWindow> *temp = nullptr;
    builder->get_widget_derived("s_scroll", temp);
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
    Geom::OptRect bbox;
    SPDocument *doc = _desktop->getDocument();

    if (!doc) {
        return;
    }

    // Will Remove this code after testing

    // switch (current_key) {
    //     case SELECTION_DRAWING:
    //         bbox = doc->getRoot()->desktopVisualBounds();
    //         if (bbox) {
    //             setArea(bbox->left(), bbox->top(), bbox->right(), bbox->bottom());
    //         }
    //         break;
    //     case SELECTION_SELECTION:
    //         if (selection->isEmpty() == false) {
    //             bbox = selection->visualBounds();
    //             if (bbox) {
    //                 setArea(bbox->left(), bbox->top(), bbox->right(), bbox->bottom());
    //             }
    //         }
    //         break;
    //     default:
    //         /* Do nothing for page or for custom */
    //         break;
    // }

    refreshArea();
    refreshExportHints();
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
    refreshExportHints();
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

    // Add advance options to adv box
    adv_box->pack_start(advance_options, true, true, 0);
    adv_box->show_all_children();

    setupUnits();
    setupSpinButtons();

    // set them before connecting to signals
    setDefaultFilename();
    setDefaultSelectionMode();

    // Refresh values to sync them with defaults.
    refreshArea();
    refreshExportHints();

    // Connect Signals Here
    for (auto [key, button] : selection_buttons) {
        button->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &SingleExport::onAreaTypeToggle), key));
    }
    units->signal_changed().connect(sigc::mem_fun(*this, &SingleExport::onUnitChanged));
    filenameConn = si_filename_entry->signal_changed().connect(sigc::mem_fun(*this, &SingleExport::onFilenameModified));
    extensionConn = si_extension_cb->signal_changed().connect(sigc::mem_fun(*this, &SingleExport::onExtensionChanged));
    exportConn = si_export->signal_clicked().connect(sigc::mem_fun(*this, &SingleExport::onExport));
    browseConn = si_filename_entry->signal_icon_press().connect(sigc::mem_fun(*this, &SingleExport::onBrowse));
    si_show_preview->signal_toggled().connect(sigc::mem_fun(*this, &SingleExport::refreshPreview));
    si_hide_all->signal_toggled().connect(sigc::mem_fun(*this, &SingleExport::refreshPreview));
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
    if (_desktop) {
        SPDocument *doc;
        Geom::OptRect bbox;
        doc = _desktop->getDocument();
        doc->ensureUpToDate();

        switch (current_key) {
            case SELECTION_SELECTION:
                if ((_desktop->getSelection())->isEmpty() == false) {
                    bbox = _desktop->getSelection()->visualBounds();
                    break;
                }
            case SELECTION_DRAWING:
                bbox = doc->getRoot()->desktopVisualBounds();
                if (bbox) {
                    break;
                }
            case SELECTION_PAGE:
                bbox = Geom::Rect(Geom::Point(0.0, 0.0),
                                  Geom::Point(doc->getWidth().value("px"), doc->getHeight().value("px")));
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

void SingleExport::refreshExportHints()
{
    if (_desktop && !filename_modified) {
        SPDocument *doc = _desktop->getDocument();
        Glib::ustring filename;
        float xdpi = 0.0, ydpi = 0.0;
        switch (current_key) {
            case SELECTION_CUSTOM:
            case SELECTION_PAGE:
            case SELECTION_DRAWING:
                sp_document_get_export_hints(doc, filename, &xdpi, &ydpi);
                if (filename.empty()) {
                    Glib::ustring filename_entry_text = si_filename_entry->get_text();
                    Glib::ustring extension_entry_text = si_extension_cb->get_active_text();
                    filename = get_default_filename(filename_entry_text, extension_entry_text, doc);
                }
                doc_export_name = filename;
                break;
            case SELECTION_SELECTION:
                if ((_desktop->getSelection())->isEmpty()) {
                    break;
                }
                _desktop->getSelection()->getExportHints(filename, &xdpi, &ydpi);

                /* If we still don't have a filename -- let's build
                   one that's nice */
                if (filename.empty()) {
                    const gchar *id = "object";
                    auto reprlst = _desktop->getSelection()->xmlNodes();
                    for (auto i = reprlst.begin(); reprlst.end() != i; ++i) {
                        Inkscape::XML::Node *repr = *i;
                        if (repr->attribute("id")) {
                            id = repr->attribute("id");
                            break;
                        }
                    }
                    filename = create_filepath_from_id(id, si_filename_entry->get_text());
                    filename = filename + si_extension_cb->get_active_text();
                }
                break;
            default:
                break;
        }
        if (!filename.empty()) {
            original_name = filename;
            si_filename_entry->set_text(filename);
            si_filename_entry->set_position(filename.length());
        } else {
            Glib::ustring newName = !doc_export_name.empty() ? doc_export_name : original_name;
            if (!newName.empty()) {
                si_filename_entry->set_text(filename);
                si_filename_entry->set_position(filename.length());
            }
        }

        if (xdpi != 0.0) {
            spin_buttons[SPIN_DPI]->set_value(xdpi);
        }
    }
}

void SingleExport::setArea(double x0, double y0, double x1, double y1)
{
    blockSpinConns(true);

    auto x0_adj = spin_buttons[SPIN_X0]->get_adjustment();
    auto x1_adj = spin_buttons[SPIN_X1]->get_adjustment();
    auto y0_adj = spin_buttons[SPIN_Y0]->get_adjustment();
    auto y1_adj = spin_buttons[SPIN_Y1]->get_adjustment();

    Unit const *unit = units->getUnit();
    setValuePx(x1_adj, x1, unit);
    setValuePx(y1_adj, y1, unit);
    setValuePx(x0_adj, x0, unit);
    setValuePx(y0_adj, y0, unit);

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

    refreshArea();
    refreshExportHints();
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
    si_extension_cb->appendExtensionToFilename(filename);
    si_filename_entry->set_text(filename);
    si_filename_entry->set_position(filename.length());
    filenameConn.unblock();
}

void SingleExport::onExport()
{
    interrupted = false;
    if (!_desktop)
        return;
    si_export->set_sensitive(false);
    bool exportSuccessful = false;
    auto extension = si_extension_cb->get_active_text();
    auto omod = ExtensionList::valid_extensions[extension];
    if (!omod) {
        si_export->set_sensitive(true);
        return;
    }

    Unit const *unit = units->getUnit();

    Glib::ustring filename = si_filename_entry->get_text();

    if (omod->is_raster()) {
        float x0 = getValuePx(spin_buttons[SPIN_X0]->get_value(), unit);
        float x1 = getValuePx(spin_buttons[SPIN_X1]->get_value(), unit);
        float y0 = getValuePx(spin_buttons[SPIN_Y0]->get_value(), unit);
        float y1 = getValuePx(spin_buttons[SPIN_Y1]->get_value(), unit);
        auto area = Geom::Rect(Geom::Point(x0, y0), Geom::Point(x1, y1)) * _desktop->dt2doc();

        unsigned long int width = int(spin_buttons[SPIN_BMWIDTH]->get_value() + 0.5);
        unsigned long int height = int(spin_buttons[SPIN_BMHEIGHT]->get_value() + 0.5);

        float dpi = spin_buttons[SPIN_DPI]->get_value();

        /* TRANSLATORS: %1 will be the filename, %2 the width, and %3 the height of the image */
        prog_dlg = create_progress_dialog(Glib::ustring::compose(_("Exporting %1 (%2 x %3)"), filename, width, height));
        prog_dlg->set_export_panel(this);
        setExporting(true, Glib::ustring::compose(_("Exporting %1 (%2 x %3)"), filename, width, height));
        prog_dlg->set_current(0);
        prog_dlg->set_total(0);

        std::vector<SPItem *> selected(_desktop->getSelection()->items().begin(),
                                       _desktop->getSelection()->items().end());
        bool hide = si_hide_all->get_active();

        exportSuccessful = _export_raster(area, width, height, dpi, filename, false, onProgressCallback, prog_dlg, omod,
                                          hide ? &selected : nullptr, &advance_options);

    } else {
        setExporting(true, Glib::ustring::compose(_("Exporting %1"), filename));
        SPDocument *doc = _desktop->getDocument();
        SPDocument *copy_doc = (doc->copy()).get();
        if (current_key == SELECTION_DRAWING) {
            fit_canvas_to_drawing(copy_doc, true);
        }
        std::vector<SPItem *> items;
        if (current_key == SELECTION_SELECTION) {
            auto itemlist = _desktop->getSelection()->items();
            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                SPItem *item = *i;
                items.push_back(item);
            }
        }
        exportSuccessful = _export_vector(omod, copy_doc, filename, false, &items);
    }
    if (prog_dlg) {
        delete prog_dlg;
        prog_dlg = nullptr;
    }
    setExporting(false);
    si_export->set_sensitive(true);
    original_name = filename;
    filename_modified = false;
    interrupted = false;
}

void SingleExport::onBrowse(Gtk::EntryIconPosition pos, const GdkEventButton *ev)
{
    if (!_app) {
        return;
    }
    Gtk::Window *window = _app->get_active_window();
    browseConn.block();
    Glib::ustring filename = Glib::filename_from_utf8(si_filename_entry->get_text());

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
        si_filename_entry->set_text(filename);
        si_filename_entry->set_position(filename.length());
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
    x0 = getValuePx(x0_adj->get_value(), unit);
    x1 = getValuePx(x1_adj->get_value(), unit);
    width = getValuePx(width_adj->get_value(), unit);
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

    setValuePx(x0_adj, x0, unit);
    setValuePx(x1_adj, x1, unit);
    setValuePx(width_adj, width, unit);
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
    y0 = getValuePx(y0_adj->get_value(), unit);
    y1 = getValuePx(y1_adj->get_value(), unit);
    height = getValuePx(height_adj->get_value(), unit);
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

    setValuePx(y0_adj, y0, unit);
    setValuePx(y1_adj, y1, unit);
    setValuePx(height_adj, height, unit);
    spin_buttons[SPIN_BMHEIGHT]->set_value(bmheight);
}

void SingleExport::dpiChange(sb_type type)
{
    float dpi, height, width, bmheight, bmwidth;

    // Get all values in px
    Unit const *unit = units->getUnit();
    height = getValuePx(spin_buttons[SPIN_HEIGHT]->get_value(), unit);
    width = getValuePx(spin_buttons[SPIN_WIDTH]->get_value(), unit);
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

// We first check any export hints related to document. If there is none we create a default name using document
// name. doc_export_name is set here and will only be changed when exporting.
void SingleExport::setDefaultFilename()
{
    if (!_desktop) {
        return;
    }
    Glib::ustring filename;
    float xdpi = 0.0, ydpi = 0.0;
    SPDocument *doc = _desktop->getDocument();
    sp_document_get_export_hints(doc, filename, &xdpi, &ydpi);
    if (filename.empty()) {
        Glib::ustring filename_entry_text = si_filename_entry->get_text();
        Glib::ustring extention_entry_text = si_extension_cb->get_active_text();
        filename = get_default_filename(filename_entry_text, extention_entry_text, doc);
    }
    doc_export_name = filename;
    original_name = filename;
    si_filename_entry->set_text(filename);
    si_filename_entry->set_position(filename.length());

    si_extension_cb->setExtensionFromFilename(filename);

    // We only need to check xdpi
    if (xdpi != 0.0) {
        spin_buttons[SPIN_DPI]->set_value(xdpi);
    }
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
            SPDocument *doc;
            Geom::OptRect bbox;
            doc = _desktop->getDocument();
            bbox = Geom::Rect(Geom::Point(0.0, 0.0),
                              Geom::Point(doc->getWidth().value("px"), doc->getHeight().value("px")));
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
    if (!preview) {
        preview = Gtk::manage(new ExportPreview());
        si_preview_box->pack_start(*preview, true, true, 0);
        si_preview_box->show_all_children();
    }
    if (!si_show_preview->get_active()) {
        preview->resetPixels();
        return;
    }

    std::vector<SPItem *> selected(_desktop->getSelection()->items().begin(), _desktop->getSelection()->items().end());
    bool hide = si_hide_all->get_active();

    Unit const *unit = units->getUnit();
    float x0 = getValuePx(spin_buttons[SPIN_X0]->get_value(), unit);
    float x1 = getValuePx(spin_buttons[SPIN_X1]->get_value(), unit);
    float y0 = getValuePx(spin_buttons[SPIN_Y0]->get_value(), unit);
    float y1 = getValuePx(spin_buttons[SPIN_Y1]->get_value(), unit);
    preview->setItem(nullptr);
    preview->setDbox(x0, x1, y0, y1);
    preview->refreshHide(hide ? &selected : nullptr);
    preview->queueRefresh();
}

void SingleExport::setDocument(SPDocument *document)
{
    if (!preview) {
        preview = Gtk::manage(new ExportPreview());
        si_preview_box->pack_start(*preview, true, true, 0);
        si_preview_box->show_all_children();
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