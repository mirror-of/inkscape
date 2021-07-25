// SPDX-License-Identifier: GPL-2.0-or-later

/** @file
 * @brief A widget that manages DialogNotebook's and other widgets inside a horizontal DialogMultipaned.
 *
 * Authors: see git history
 *   Tavmjong Bah
 *
 * Copyright (c) 2018 Tavmjong Bah, Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "dialog-container.h"

#include <glibmm/i18n.h>
#include <giomm/file.h>
#include <glibmm/keyfile.h>
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>

#include "enums.h"
#include "inkscape-application.h"
// #include "ui/dialog/align-and-distribute.h"
#include "ui/dialog/clonetiler.h"
#include "ui/dialog/dialog-data.h"
#include "ui/dialog/dialog-multipaned.h"
#include "ui/dialog/dialog-notebook.h"
#include "ui/dialog/dialog-window.h"
#include "ui/dialog/document-properties.h"
#include "ui/dialog/export.h"
#include "ui/dialog/fill-and-stroke.h"
#include "ui/dialog/filter-effects-dialog.h"
#include "ui/dialog/find.h"
#include "ui/dialog/glyphs.h"
#include "ui/dialog/icon-preview.h"
#include "ui/dialog/inkscape-preferences.h"
#include "ui/dialog/input.h"
#include "ui/dialog/layers.h"
#include "ui/dialog/livepatheffect-editor.h"
#include "ui/dialog/memory.h"
#include "ui/dialog/messages.h"
#include "ui/dialog/object-attributes.h"
#include "ui/dialog/object-properties.h"
#include "ui/dialog/objects.h"
#include "ui/dialog/paint-servers.h"
#include "ui/dialog/prototype.h"
#include "ui/dialog/selectorsdialog.h"
#include "ui/shortcuts.h"
#if WITH_GSPELL
#include "ui/dialog/spellcheck.h"
#endif
#include "ui/dialog/styledialog.h"
#include "ui/dialog/svg-fonts-dialog.h"
#include "ui/dialog/swatches.h"
#include "ui/dialog/symbols.h"
#include "ui/dialog/text-edit.h"
#include "ui/dialog/tile.h"
#include "ui/dialog/tracedialog.h"
#include "ui/dialog/transformation.h"
#include "ui/dialog/undo-history.h"
#include "ui/dialog/xml-tree.h"
#include "ui/icon-names.h"
#include "ui/widget/canvas-grid.h"
#include "verbs.h"

// TEMP TEMP TEMP Until dialog verbs are gone.
static std::map<int, Glib::ustring> verb_to_dialog_map =
{
    {SP_VERB_DIALOG_ALIGN_DISTRIBUTE, "AlignDistribute"   },
    // {SP_VERB_SELECTION_ARRANGE,       "Arrange"           },
    {SP_VERB_DIALOG_ATTR_XML,         "AttrDialog"        },
    {SP_VERB_DIALOG_CLONETILER,       "Clonetiler"        },
    {SP_VERB_DIALOG_DOCPROPERTIES,    "DocumentProperties"},
    {SP_VERB_DIALOG_EXPORT,           "Export"            },
    {SP_VERB_DIALOG_FILL_STROKE,      "FillStroke"        },
    {SP_VERB_DIALOG_FILTER_EFFECTS,   "FilterEffects"     },
    {SP_VERB_DIALOG_FIND,             "Find"              },
    {SP_VERB_DIALOG_GLYPHS,           "Glyphs"            },
    {SP_VERB_VIEW_ICON_PREVIEW,       "IconPreview"       },
    {SP_VERB_DIALOG_INPUT,            "Input"             },
    {SP_VERB_DIALOG_LAYERS,           "Layers"            },
    {SP_VERB_DIALOG_LIVE_PATH_EFFECT, "LivePathEffect"    },
    {SP_VERB_HELP_MEMORY,             "Memory"            },
    {SP_VERB_DIALOG_DEBUG,            "Messages"          },
    {SP_VERB_DIALOG_ATTR,             "ObjectAttributes"  },
    {SP_VERB_DIALOG_ITEM,             "ObjectProperties"  },
    {SP_VERB_DIALOG_OBJECTS,          "Objects"           },
    {SP_VERB_DIALOG_PAINT,            "PaintServers"      },
    {SP_VERB_DIALOG_PREFERENCES,      "Preferences"       },
    {SP_VERB_DIALOG_SELECTORS,        "Selectors"         },
    {SP_VERB_DIALOG_STYLE,            "Style"             },
    {SP_VERB_DIALOG_SVG_FONTS,        "SVGFonts"          },
    {SP_VERB_DIALOG_SWATCHES,         "Swatches"          },
    {SP_VERB_DIALOG_SYMBOLS,          "Symbols"           },
    {SP_VERB_DIALOG_TEXT,             "Text"              },
    {SP_VERB_SELECTION_TRACE,         "Trace"             },
    {SP_VERB_DIALOG_TRANSFORM,        "Transform"         },
    {SP_VERB_DIALOG_UNDO_HISTORY,     "UndoHistory"       },
    {SP_VERB_DIALOG_XML_EDITOR,       "XMLEditor"         },
#if WITH_GSPELL
    {SP_VERB_DIALOG_SPELLCHECK,       "Spellcheck"        },
#endif
#ifdef DEBUG
    {SP_VERB_DIALOG_PROTOTYPE,        "Prototype"         }
#endif
};

namespace Inkscape {
namespace UI {
namespace Dialog {

DialogContainer::~DialogContainer() {
    // delete columns; desktop widget deletes dialog container before it get "unrealized",
    // so it doesn't get a chance to remove them
    delete columns;
}

DialogContainer::DialogContainer()
{
    set_name("DialogContainer");

    // Setup main column
    columns = Gtk::manage(new DialogMultipaned(Gtk::ORIENTATION_HORIZONTAL));

    connections.emplace_back(columns->signal_prepend_drag_data().connect(
        sigc::bind<DialogMultipaned *>(sigc::mem_fun(*this, &DialogContainer::prepend_drop), columns)));

    connections.emplace_back(columns->signal_append_drag_data().connect(
        sigc::bind<DialogMultipaned *>(sigc::mem_fun(*this, &DialogContainer::append_drop), columns)));

    // Setup drop targets.
    target_entries.emplace_back(Gtk::TargetEntry("GTK_NOTEBOOK_TAB"));
    columns->set_target_entries(target_entries);

    add(*columns);

    // Should probably be moved to window.
    //  connections.emplace_back(signal_unmap().connect(sigc::mem_fun(*this, &DialogContainer::cb_on_unmap)));

    show_all_children();
}

DialogMultipaned *DialogContainer::create_column()
{
    DialogMultipaned *column = Gtk::manage(new DialogMultipaned(Gtk::ORIENTATION_VERTICAL));

    connections.emplace_back(column->signal_prepend_drag_data().connect(
        sigc::bind<DialogMultipaned *>(sigc::mem_fun(*this, &DialogContainer::prepend_drop), column)));

    connections.emplace_back(column->signal_append_drag_data().connect(
        sigc::bind<DialogMultipaned *>(sigc::mem_fun(*this, &DialogContainer::append_drop), column)));

    connections.emplace_back(column->signal_now_empty().connect(
        sigc::bind<DialogMultipaned *>(sigc::mem_fun(*this, &DialogContainer::column_empty), column)));

    column->set_target_entries(target_entries);

    return column;
}

/**
 * Get an instance of a DialogBase dialog using the associated dialog name.
 */
DialogBase *DialogContainer::dialog_factory(const Glib::ustring& dialog_type)
{

    // clang-format off
    if(     dialog_type == "AlignDistribute")     return &Inkscape::UI::Dialog::ArrangeDialog::getInstance();
    // else if(dialog_type == "Arrange")             return &Inkscape::UI::Dialog::ArrangeDialog::getInstance();
    else if(dialog_type == "AttrDialog")          return &Inkscape::UI::Dialog::AttrDialog::getInstance();
    else if(dialog_type == "Clonetiler")          return &Inkscape::UI::Dialog::CloneTiler::getInstance();
    else if(dialog_type == "DocumentProperties")  return &Inkscape::UI::Dialog::DocumentProperties::getInstance();
    else if(dialog_type == "Export")              return &Inkscape::UI::Dialog::Export::getInstance();
    else if(dialog_type == "FillStroke")          return &Inkscape::UI::Dialog::FillAndStroke::getInstance();
    else if(dialog_type == "FilterEffects")       return &Inkscape::UI::Dialog::FilterEffectsDialog::getInstance();
    else if(dialog_type == "Find")                return &Inkscape::UI::Dialog::Find::getInstance();
    else if(dialog_type == "Glyphs")              return &Inkscape::UI::Dialog::GlyphsPanel::getInstance();
    else if(dialog_type == "IconPreview")         return &Inkscape::UI::Dialog::IconPreviewPanel::getInstance();
    else if(dialog_type == "Input")               return &Inkscape::UI::Dialog::InputDialog::getInstance();
    else if(dialog_type == "Layers")              return &Inkscape::UI::Dialog::LayersPanel::getInstance();
    else if(dialog_type == "LivePathEffect")      return &Inkscape::UI::Dialog::LivePathEffectEditor::getInstance();
    else if(dialog_type == "Memory")              return &Inkscape::UI::Dialog::Memory::getInstance();
    else if(dialog_type == "Messages")            return &Inkscape::UI::Dialog::Messages::getInstance();
    else if(dialog_type == "ObjectAttributes")    return &Inkscape::UI::Dialog::ObjectAttributes::getInstance();
    else if(dialog_type == "ObjectProperties")    return &Inkscape::UI::Dialog::ObjectProperties::getInstance();
    else if(dialog_type == "Objects")             return &Inkscape::UI::Dialog::ObjectsPanel::getInstance();
    else if(dialog_type == "PaintServers")        return &Inkscape::UI::Dialog::PaintServersDialog::getInstance();
    else if(dialog_type == "Preferences")         return &Inkscape::UI::Dialog::InkscapePreferences::getInstance();
    else if(dialog_type == "Selectors")           return &Inkscape::UI::Dialog::SelectorsDialog::getInstance();
    else if(dialog_type == "Style")               return &Inkscape::UI::Dialog::StyleDialog::getInstance();
    else if(dialog_type == "SVGFonts")            return &Inkscape::UI::Dialog::SvgFontsDialog::getInstance();
    else if(dialog_type == "Swatches")            return &Inkscape::UI::Dialog::SwatchesPanel::getInstance();
    else if(dialog_type == "Symbols")             return &Inkscape::UI::Dialog::SymbolsDialog::getInstance();
    else if(dialog_type == "Text")                return &Inkscape::UI::Dialog::TextEdit::getInstance();
    else if(dialog_type == "Trace")               return &Inkscape::UI::Dialog::TraceDialog::getInstance();
    else if(dialog_type == "Transform")           return &Inkscape::UI::Dialog::Transformation::getInstance();
    else if(dialog_type == "UndoHistory")         return &Inkscape::UI::Dialog::UndoHistory::getInstance();
    else if(dialog_type == "XMLEditor")           return &Inkscape::UI::Dialog::XmlTree::getInstance();
#if WITH_GSPELL
    else if(dialog_type == "SpellCheck")          return &Inkscape::UI::Dialog::SpellCheck::getInstance();
#endif
#ifdef DEBUG
    else if(dialog_type == "Prototype")           return &Inkscape::UI::Dialog::Prototype::getInstance();
#endif
    else {
        std::cerr << "DialogContainer::dialog_factory: Unhandled dialog: " << dialog_type << std::endl;
        return nullptr;
    }
    // clang-format on
}

// Create the notebook tab
Gtk::Widget *DialogContainer::create_notebook_tab(Glib::ustring label_str, Glib::ustring image_str, const Glib::ustring shortcut)
{
    Gtk::Label *label = Gtk::manage(new Gtk::Label(label_str));
    Gtk::Image *image = Gtk::manage(new Gtk::Image());
    Gtk::Button *close = Gtk::manage(new Gtk::Button());
    image->set_from_icon_name(image_str, Gtk::ICON_SIZE_MENU);
    Gtk::Box *tab = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 2));
    close->set_image_from_icon_name("window-close");
    close->set_halign(Gtk::ALIGN_END);
    close->set_tooltip_text(_("Close Tab"));
    close->get_style_context()->add_class("close-button");

    tab->set_name(label_str);
    tab->pack_start(*image);
    tab->pack_end(*close);
    tab->pack_end(*label);
    tab->show_all();

    // Workaround to the fact that Gtk::Box doesn't receive on_button_press event
    Gtk::EventBox *cover = Gtk::manage(new Gtk::EventBox());
    cover->add(*tab);

    // Add shortcut tooltip
    if (shortcut.size() > 0) {
        auto tlabel = shortcut;
        int pos = tlabel.find("&", 0);
        if (pos >= 0 && pos < tlabel.length()) {
            tlabel.replace(pos, 1, "&amp;");
        }
        tab->set_tooltip_markup(label_str + " (<b>" + tlabel + "</b>)");
    } else {
        tab->set_tooltip_text(label_str);
    }

    return cover;
}

// find dialog's multipaned parent; is there a better way?
DialogMultipaned* get_dialog_parent(DialogBase* dialog) {
    if (!dialog) return nullptr;

    // dialogs are nested inside Gtk::Notebook
    if (auto notebook = dynamic_cast<Gtk::Notebook*>(dialog->get_parent())) {
        // notebooks are inside viewport, inside scrolled window
        if (auto viewport = dynamic_cast<Gtk::Viewport*>(notebook->get_parent())) {
            if (auto scroll = dynamic_cast<Gtk::ScrolledWindow*>(viewport->get_parent())) {
                // finally get the panel
                if (auto panel = dynamic_cast<DialogMultipaned*>(scroll->get_parent())) {
                    return panel;
                }
            }
        }
    }

    return nullptr;
}

/**
 * Add new dialog to the current container or in a floating window, based on preferences. TEMP TEMP TEMP
 */
void DialogContainer::new_dialog(unsigned int code)
{
    auto it = verb_to_dialog_map.find(code);
    if (it != verb_to_dialog_map.end()) {
        return new_dialog(it->second);
    } else {
        std::cerr << "DialogContainer::new_dialog: Could not find dialog for verb " << code << std::endl;
    }
}


/**
 * Add new dialog to the current container or in a floating window, based on preferences.
 */
void DialogContainer::new_dialog(const Glib::ustring& dialog_type )
{
    // Open all dialogs as floating, if set in preferences
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs == nullptr) {
        return;
    }

    int dockable = prefs->getInt("/options/dialogtype/value", PREFS_DIALOGS_BEHAVIOR_DOCKABLE);
    bool floating = DialogManager::singleton().should_open_floating(dialog_type);
    if (dockable == PREFS_DIALOGS_BEHAVIOR_FLOATING || floating) {
        new_floating_dialog(dialog_type);
    } else {
        new_dialog(dialog_type, nullptr);
    }

    if (DialogBase* dialog = find_existing_dialog(dialog_type)) {
        dialog->focus_dialog();
    }
}


DialogBase* DialogContainer::find_existing_dialog(const Glib::ustring& dialog_type) {
    DialogBase *existing_dialog = get_dialog(dialog_type);
    if (!existing_dialog) {
        existing_dialog = DialogManager::singleton().find_floating_dialog(dialog_type);
    }
    return existing_dialog;
}

/**
 * Overloaded new_dialog
 */
void DialogContainer::new_dialog(const Glib::ustring& dialog_type, DialogNotebook *notebook)
{
    columns->ensure_multipaned_children();

    // Limit each container to containing one of any type of dialog.
    if (DialogBase* existing_dialog = find_existing_dialog(dialog_type)) {
        // make sure parent window is not hidden/collapsed
        if (auto panel = get_dialog_parent(existing_dialog)) {
            panel->show();
        }
        // found existing dialog; blink & exit
        existing_dialog->blink();
        return;
    }

    // Create the dialog widget
    DialogBase *dialog = dialog_factory(dialog_type);

    if (!dialog) {
        std::cerr << "DialogContainer::new_dialog(): couldn't find dialog for: " << dialog_type << std::endl;
        return;
    }

    // manage the dialog instance
    dialog = Gtk::manage(dialog);

    // Create the notebook tab
    Glib::ustring image("inkscape-logo");
    auto it = dialog_data.find(dialog_type);
    if (it != dialog_data.end()) {
        image = it->second.icon_name;
    }

    Glib::ustring label;
    Glib::ustring action_name = "win.dialog-open('" + dialog_type + "')";
    auto app = InkscapeApplication::instance();
    std::vector<Glib::ustring> accels = app->gtk_app()->get_accels_for_action(action_name);
    if (accels.size() > 0) {
        guint key = 0;
        Gdk::ModifierType mods;
        Gtk::AccelGroup::parse(accels[0], key, mods);
        label = Gtk::AccelGroup::get_label(key, mods);
    } else {
        // TEMP TEMP TEMP
        for (auto it : verb_to_dialog_map) {
            if (it.second == dialog_type) {
                auto verb = Inkscape::Verb::get(it.first);
                if (verb) {
                    auto key = Inkscape::Shortcuts::getInstance().get_shortcut_from_verb(verb);
                    if (!key.is_null()) {
                        label = Inkscape::Shortcuts::getInstance().get_label(key);
                    }
                }
                break;
            }
        }
    }

    Gtk::Widget *tab = create_notebook_tab(dialog->get_name(), image, label);

    // If not from notebook menu add at top of last column.
    if (!notebook) {
        // Look to see if last column contains a multipane. If not, add one.
        DialogMultipaned *last_column = dynamic_cast<DialogMultipaned *>(columns->get_last_widget());
        if (!last_column) {
            last_column = create_column();
            columns->append(last_column);
        }

        // Look to see if first widget in column is notebook, if not add one.
        notebook = dynamic_cast<DialogNotebook *>(last_column->get_first_widget());
        if (!notebook) {
            notebook = Gtk::manage(new DialogNotebook(this));
            last_column->prepend(notebook);
        }
    }

    // Add dialog
    notebook->add_page(*dialog, *tab, dialog->get_name());

    if (auto panel = dynamic_cast<DialogMultipaned*>(notebook->get_parent())) {
        // if panel is collapsed, show it now, or else new dialog will be mysteriously missing
        panel->show();
    }
}

// recreate dialogs hosted (docked) in a floating DialogWindow; window will be created
bool DialogContainer::recreate_dialogs_from_state(const Glib::KeyFile* keyfile)
{
    bool restored = false;
    // Step 1: check if we want to load the state
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int save_state = prefs->getInt("/options/savedialogposition/value", PREFS_DIALOGS_STATE_SAVE);
    if (save_state == PREFS_DIALOGS_STATE_NONE) {
        return restored; // User has turned off this feature in Preferences
    }

    // if it isn't dockable, all saved docked dialogs are made floating
    bool is_dockable =
        prefs->getInt("/options/dialogtype/value", PREFS_DIALOGS_BEHAVIOR_DOCKABLE) != PREFS_DIALOGS_BEHAVIOR_FLOATING;

    if (!is_dockable)
        return false; // not applicable if docking is off

    // Step 2: get the number of windows; should be 1
    int windows_count = 0;
    try {
        windows_count = keyfile->get_integer("Windows", "Count");
    } catch (Glib::Error &error) {
        std::cerr << G_STRFUNC << ": " << error.what() << std::endl;
    }

    // Step 3: for each window, load its state.
    for (int window_idx = 0; window_idx < windows_count; ++window_idx) {
        Glib::ustring group_name = "Window" + std::to_string(window_idx);

        bool has_position = keyfile->has_key(group_name, "Position") && keyfile->get_boolean(group_name, "Position");
        window_position_t pos;
        if (has_position) { // floating window position recorded?
            pos.x = keyfile->get_integer(group_name, "x");
            pos.y = keyfile->get_integer(group_name, "y");
            pos.width = keyfile->get_integer(group_name, "width");
            pos.height = keyfile->get_integer(group_name, "height");
        }
        // Step 3.0: read the window parameters
        int column_count = 0;
        try {
            column_count = keyfile->get_integer(group_name, "ColumnCount");
        } catch (Glib::Error &error) {
            std::cerr << G_STRFUNC << ": " << error.what() << std::endl;
        }

        // Step 3.1: get the window's container columns where we want to create the dialogs
        DialogWindow *dialog_window = new DialogWindow(nullptr);
        DialogContainer *active_container = dialog_window->get_container();
        DialogMultipaned *active_columns = active_container ? active_container->get_columns() : nullptr;

        if (!active_container || !active_columns) {
            continue;
        }

        // Step 3.2: for each column, load its state
        for (int column_idx = 0; column_idx < column_count; ++column_idx) {
            Glib::ustring column_group_name = group_name + "Column" + std::to_string(column_idx);

            // Step 3.2.0: read the column parameters
            int notebook_count = 0;
            bool before_canvas = false;
            try {
                notebook_count = keyfile->get_integer(column_group_name, "NotebookCount");
                if (keyfile->has_key(column_group_name, "BeforeCanvas")) {
                    before_canvas = keyfile->get_boolean(column_group_name, "BeforeCanvas");
                }
            } catch (Glib::Error &error) {
                std::cerr << G_STRFUNC << ": " << error.what() << std::endl;
            }

            // Step 3.2.1: create the column
            DialogMultipaned *column = active_container->create_column();

            before_canvas ? active_columns->prepend(column) : active_columns->append(column);

            // Step 3.2.2: for each noteboook, load its dialogs
            for (int notebook_idx = 0; notebook_idx < notebook_count; ++notebook_idx) {
                Glib::ustring key = "Notebook" + std::to_string(notebook_idx) + "Dialogs";

                // Step 3.2.2.0 read the list of dialogs in the current notebook
                std::vector<Glib::ustring> dialogs;
                try {
                    dialogs = keyfile->get_string_list(column_group_name, key);
                } catch (Glib::Error &error) {
                    std::cerr << G_STRFUNC << ": " << error.what() << std::endl;
                }

                if (!dialogs.size()) {
                    continue;
                }

                DialogNotebook *notebook = nullptr;

                // Step 3.2.2.1 create each dialog in the current notebook
                for (auto type : dialogs) {
                    if (DialogManager::singleton().find_floating_dialog(type)) {
                        // avoid duplicates
                        continue;
                    }

                    if (dialog_data.find(type) != dialog_data.end()) {
                        if (!notebook) {
                            notebook = Gtk::manage(new DialogNotebook(active_container));
                            column->append(notebook);
                        }
                        active_container->new_dialog(type, notebook);
                    } else {
                        std::cerr << "recreate_dialogs_from_state: invalid dialog type: " << type << std::endl;
                    }
                }
            }
        }

        if (has_position) {
            dm_restore_window_position(*dialog_window, pos);
        }
        else {
            dialog_window->update_window_size_to_fit_children();
        }
        dialog_window->show_all();
        restored = true;
    }

    return restored;
}

/**
 * Add a new floating dialog (or reuse existing one if it's already up)
 */
DialogWindow *DialogContainer::new_floating_dialog(unsigned int code)
{
    auto it = verb_to_dialog_map.find(code);
    if (it != verb_to_dialog_map.end()) {
        return create_new_floating_dialog(it->second, true);
    } else {
        std::cerr << "DialogContainer::new_floating_dialog: Could not find dialog for verb " << code << std::endl;
        return nullptr;
    }
}

/**
 * Add a new floating dialog (or reuse existing one if it's already up)
 */
DialogWindow *DialogContainer::new_floating_dialog(const Glib::ustring& dialog_type)
{
    return create_new_floating_dialog(dialog_type, true);
}

DialogWindow *DialogContainer::create_new_floating_dialog(const Glib::ustring& dialog_type, bool blink)
{
    // check if this dialog is already open
    if (DialogBase* existing_dialog = find_existing_dialog(dialog_type)) {
        // found existing dialog; blink & exit
        if (blink) {
            existing_dialog->blink();
            // show its window if it is hidden
            if (auto window = DialogManager::singleton().find_floating_dialog_window(dialog_type)) {
                DialogManager::singleton().set_floating_dialog_visibility(window, true);
            }
        }
        return nullptr;
    }

    // check if this dialog *was* open and floating; if so recreate its window
    if (auto state = DialogManager::singleton().find_dialog_state(dialog_type)) {
        if (recreate_dialogs_from_state(state.get())) {
            return nullptr;
        }
    }

    // Create the dialog widget
    DialogBase *dialog = dialog_factory(dialog_type);

    if (!dialog) {
        std::cerr << "DialogContainer::new_dialog(): couldn't find dialog for: " << dialog_type << std::endl;
        return nullptr;
    }

    // manage the dialog instance
    dialog = Gtk::manage(dialog);

    // Create the notebook tab
    gchar* image = nullptr;

    Glib::ustring label;
    Glib::ustring action_name = "win.dialog-open('" + dialog_type + "')";
    auto app = InkscapeApplication::instance();
    std::vector<Glib::ustring> accels = app->gtk_app()->get_accels_for_action(action_name);
    if (accels.size() > 0) {
        guint key = 0;
        Gdk::ModifierType mods;
        Gtk::AccelGroup::parse(accels[0], key, mods);
        label = Gtk::AccelGroup::get_label(key, mods);
    }

    Gtk::Widget *tab =
        create_notebook_tab(dialog->get_name(), image ? Glib::ustring(image) : INKSCAPE_ICON("inkscape-logo"), label);

    // New temporary noteboook
    DialogNotebook *notebook = Gtk::manage(new DialogNotebook(this));
    notebook->add_page(*dialog, *tab, dialog->get_name());

    return notebook->pop_tab_callback();
}

// toggle dialogs (visibility) is invoked on a top container embedded in Inkscape window
void DialogContainer::toggle_dialogs()
{
    // check how many dialog panels are visible and how many are hidden
    // we use this info to decide what it means to toggle visibility
    int visible = 0;
    int hidden = 0;
    for (auto child : columns->get_children()) {
        // only examine panels, skip drop zones and handles
        if (auto panel = dynamic_cast<DialogMultipaned*>(child)) {
            if (panel->is_visible()) {
                ++visible;
            }
            else {
                ++hidden;
            }
        }
    }

    // next examine floating dialogs
    auto windows = DialogManager::singleton().get_all_floating_dialog_windows();
    for (auto wnd : windows) {
        if (wnd->is_visible()) {
            ++visible;
        }
        else {
            ++hidden;
        }
    }

    bool show_dialogs = true;
    // if some dialogs are hidden, toggle will first show them;
    // another option could be to hide all if some dialogs are visible
    if (hidden > 0) {
        show_dialogs = true;
    }
    else {
        // if everything's visible, hide them
        show_dialogs = false;
    }

    // set visibility of floating dialogs
    for (auto wnd : windows) {
        DialogManager::singleton().set_floating_dialog_visibility(wnd, show_dialogs);
    }

    // set visibility of docked dialogs
    columns->toggle_multipaned_children(show_dialogs);
}

// Update dialogs
void DialogContainer::update_dialogs()
{
    for_each(dialogs.begin(), dialogs.end(), [&](auto dialog) { dialog.second->update(); });
}

void DialogContainer::set_desktop(SPDesktop *desktop)
{
    for_each(dialogs.begin(), dialogs.end(), [&](auto dialog) { dialog.second->setDesktop(desktop); });
}

bool DialogContainer::has_dialog_of_type(DialogBase *dialog)
{
    return (dialogs.find(dialog->get_type()) != dialogs.end());
}

DialogBase *DialogContainer::get_dialog(const Glib::ustring& dialog_type)
{
    auto found = dialogs.find(dialog_type);
    if (found != dialogs.end()) {
        return found->second;
    }
    return nullptr;
}

// Add dialog to list.
void DialogContainer::link_dialog(DialogBase *dialog)
{
    dialogs.insert(std::pair<Glib::ustring, DialogBase *>(dialog->get_type(), dialog));

    DialogWindow *window = dynamic_cast<DialogWindow *>(get_toplevel());
    if (window) {
        window->update_dialogs();
    }
    else {
        // dialog without DialogWindow has been docked; remove it's floating state
        // so if user closes and reopens it, it shows up docked again, not floating
        DialogManager::singleton().remove_dialog_floating_state(dialog->get_type());
    }
}

// Remove dialog from list.
void DialogContainer::unlink_dialog(DialogBase *dialog)
{
    if (!dialog) {
        return;
    }

    auto found = dialogs.find(dialog->get_type());
    if (found != dialogs.end()) {
        dialogs.erase(found);
    }

    DialogWindow *window = dynamic_cast<DialogWindow *>(get_toplevel());
    if (window) {
        window->update_dialogs();
    }
}

/**
 * Load last open window's dialog configuration state.
 *
 * For the keyfile format, check `save_container_state()`.
 */
void DialogContainer::load_container_state(Glib::KeyFile *keyfile, bool include_floating)
{
    // Step 1: check if we want to load the state
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // if it isn't dockable, all saved docked dialogs are made floating
    bool is_dockable =
        prefs->getInt("/options/dialogtype/value", PREFS_DIALOGS_BEHAVIOR_DOCKABLE) != PREFS_DIALOGS_BEHAVIOR_FLOATING;

    // Step 2: get the number of windows
    int windows_count = keyfile->get_integer("Windows", "Count");

    // Step 3: for each window, load its state. Only the first window is not floating (the others are DialogWindow)
    for (int window_idx = 0; window_idx < windows_count; ++window_idx) {
        if (window_idx > 0 && !include_floating)
            break;

        Glib::ustring group_name = "Window" + std::to_string(window_idx);

        // Step 3.0: read the window parameters
        int column_count = 0;
        bool floating = window_idx != 0;
        window_position_t pos;
        bool has_position = false;
        try {
            column_count = keyfile->get_integer(group_name, "ColumnCount");
            floating = keyfile->get_boolean(group_name, "Floating");
            if (keyfile->has_key(group_name, "Position") && keyfile->get_boolean(group_name, "Position")) {
                pos.x = keyfile->get_integer(group_name, "x");
                pos.y = keyfile->get_integer(group_name, "y");
                pos.width = keyfile->get_integer(group_name, "width");
                pos.height = keyfile->get_integer(group_name, "height");
                has_position = true;
            }
        } catch (Glib::Error &error) {
            std::cerr << "DialogContainer::load_container_state: " << error.what() << std::endl;
        }

        // Step 3.1: get the window's container columns where we want to create the dialogs
        DialogContainer *active_container = nullptr;
        DialogMultipaned *active_columns = nullptr;
        DialogWindow *dialog_window = nullptr;

        if (is_dockable) {
            if (floating) {
                dialog_window = new DialogWindow(nullptr);
                if (dialog_window) {
                    active_container = dialog_window->get_container();
                    active_columns = dialog_window->get_container()->get_columns();
                }
            } else {
                active_container = this;
                active_columns = columns;
            }

            if (!active_container || !active_columns) {
                continue;
            }
        }

        // Step 3.2: for each column, load its state
        for (int column_idx = 0; column_idx < column_count; ++column_idx) {
            Glib::ustring column_group_name = group_name + "Column" + std::to_string(column_idx);

            // Step 3.2.0: read the column parameters
            int notebook_count = 0;
            bool before_canvas = false;
            try {
                notebook_count = keyfile->get_integer(column_group_name, "NotebookCount");
                before_canvas = keyfile->get_boolean(column_group_name, "BeforeCanvas");
            } catch (Glib::Error &error) {
                std::cerr << "DialogContainer::load_container_state: " << error.what() << std::endl;
            }

            // Step 3.2.1: create the column
            DialogMultipaned *column = nullptr;
            if (is_dockable) {
                column = active_container->create_column();
                if (!column) {
                    continue;
                }

                before_canvas ? active_columns->prepend(column) : active_columns->append(column);
            }

            // Step 3.2.2: for each noteboook, load its dialogs
            for (int notebook_idx = 0; notebook_idx < notebook_count; ++notebook_idx) {
                Glib::ustring key = "Notebook" + std::to_string(notebook_idx) + "Dialogs";

                // Step 3.2.2.0 read the list of dialogs in the current notebook
                std::vector<Glib::ustring> dialogs;
                try {
                    dialogs = keyfile->get_string_list(column_group_name, key);
                } catch (Glib::Error &error) {
                    std::cerr << "DialogContainer::load_container_state: " << error.what() << std::endl;
                }

                if (!dialogs.size()) {
                    continue;
                }

                DialogNotebook *notebook = nullptr;
                if (is_dockable) {
                    notebook = Gtk::manage(new DialogNotebook(active_container));
                    column->append(notebook);
                }

                // Step 3.2.2.1 create each dialog in the current notebook
                for (auto type : dialogs) {

                    if (dialog_data.find(type) != dialog_data.end()) {
                        if (is_dockable) {
                            active_container->new_dialog(type, notebook);
                        } else {
                            dialog_window = create_new_floating_dialog(type, false);
                        }
                    } else {
                        std::cerr << "load_container_state: invalid dialog type: " << type << std::endl;
                    }
                }
            }
        }

        if (dialog_window) {
            if (has_position) {
                dm_restore_window_position(*dialog_window, pos);
            }
            else {
                dialog_window->update_window_size_to_fit_children();
            }
            dialog_window->show_all();
        }
    }
}

void save_wnd_position(Glib::KeyFile *keyfile, const Glib::ustring &group_name, const window_position_t *position)
{
    keyfile->set_boolean(group_name, "Position", position != nullptr);
    if (position) { // floating window position?
        keyfile->set_integer(group_name, "x", position->x);
        keyfile->set_integer(group_name, "y", position->y);
        keyfile->set_integer(group_name, "width", position->width);
        keyfile->set_integer(group_name, "height", position->height);
    }
}

// get *this* container's state only; store window 'position' in the state if given
std::shared_ptr<Glib::KeyFile> DialogContainer::get_container_state(const window_position_t *position) const
{
    std::shared_ptr<Glib::KeyFile> keyfile = std::make_shared<Glib::KeyFile>();

    DialogMultipaned *window = columns;
    const int window_idx = 0;

    // Step 2: save the number of windows
    keyfile->set_integer("Windows", "Count", 1);

    // Step 3.0: get all the multipanes of the window
    std::vector<DialogMultipaned *> multipanes;

    for (auto const &column : window->get_children()) {
        if (auto paned = dynamic_cast<DialogMultipaned *>(column)) {
            multipanes.push_back(paned);
        }
    }

    // Step 3.1: for each non-empty column, save its data.
    int column_count = 0; // non-empty columns count
    for (size_t column_idx = 0; column_idx < multipanes.size(); ++column_idx) {
        Glib::ustring group_name = "Window" + std::to_string(window_idx) + "Column" + std::to_string(column_idx);
        int notebook_count = 0; // non-empty notebooks count

        // Step 3.1.0: for each notebook, get its dialogs
        for (auto const &columns_widget : multipanes[column_idx]->get_children()) {
            if (auto dialog_notebook = dynamic_cast<DialogNotebook *>(columns_widget)) {
                std::vector<Glib::ustring> dialogs;

                for (auto const &widget : dialog_notebook->get_notebook()->get_children()) {
                    if (DialogBase *dialog = dynamic_cast<DialogBase *>(widget)) {
                        dialogs.push_back(dialog->get_type());
                    }
                }

                // save the dialogs type
                Glib::ustring key = "Notebook" + std::to_string(notebook_count) + "Dialogs";
                keyfile->set_string_list(group_name, key, dialogs);

                // increase the notebook count
                notebook_count++;
            }
        }

        // Step 3.1.1: increase the column count
        if (notebook_count != 0) {
            column_count++;
        }

        // Step 3.1.2: Save the column's data
        keyfile->set_integer(group_name, "NotebookCount", notebook_count);
    }

    // Step 3.2: save the window group
    Glib::ustring group_name = "Window" + std::to_string(window_idx);
    keyfile->set_integer(group_name, "ColumnCount", column_count);
    save_wnd_position(keyfile.get(), group_name, position);

    return keyfile;
}

/**
 * Save container state. The configuration of open dialogs and the relative positions of the notebooks are saved.
 *
 * The structure of such a KeyFile is:
 *
 * There is a "Windows" group that records the number of the windows:
 * [Windows]
 * Count=1
 *
 * A "WindowX" group saves the number of columns the window's container has and whether the window is floating:
 *
 * [Window0]
 * ColumnCount=1
 * Floating=false
 *
 * For each column, we have a "WindowWColumnX" group, where X is the index of the column. "BeforeCanvas" checks
 * if the column is before the canvas or not. "NotebookCount" records how many notebooks are in each column and
 * "NotebookXDialogs" records a list of the types for the dialogs in notebook X.
 *
 * [Window0Column0]
 * Notebook0Dialogs=Layers;Text;
 * NotebookCount=2
 * BeforeCanvas=false
 *
 */
std::unique_ptr<Glib::KeyFile> DialogContainer::save_container_state()
{
    std::unique_ptr<Glib::KeyFile> keyfile = std::make_unique<Glib::KeyFile>();
    auto app = InkscapeApplication::instance();

    // Step 1: get all the container columns (in order, from the current container and all DialogWindow containers)
    std::vector<DialogMultipaned *> windows(1, columns);
    std::vector<DialogWindow *> dialog_windows(1, nullptr);

    for (auto const &window : app->gtk_app()->get_windows()) {
        DialogWindow *dialog_window = dynamic_cast<DialogWindow *>(window);
        if (dialog_window) {
            windows.push_back(dialog_window->get_container()->get_columns());
            dialog_windows.push_back(dialog_window);
        }
    }

    // Step 2: save the number of windows
    keyfile->set_integer("Windows", "Count", windows.size());

    // Step 3: for each window, save its data. Only the first window is not floating (the others are DialogWindow)
    for (int window_idx = 0; window_idx < (int)windows.size(); ++window_idx) {
        // Step 3.0: get all the multipanes of the window
        std::vector<DialogMultipaned *> multipanes;

        // used to check if the column is before or after canvas
        std::vector<DialogMultipaned *>::iterator multipanes_it = multipanes.begin();
        bool canvas_seen = window_idx != 0; // no floating windows (window_idx > 0) have a canvas
        int before_canvas_columns_count = 0;

        for (auto const &column : windows[window_idx]->get_children()) {
            if (!canvas_seen) {
                UI::Widget::CanvasGrid *canvas = dynamic_cast<UI::Widget::CanvasGrid *>(column);
                if (canvas) {
                    canvas_seen = true;
                } else {
                    DialogMultipaned *paned = dynamic_cast<DialogMultipaned *>(column);
                    if (paned) {
                        multipanes_it = multipanes.insert(multipanes_it, paned);
                        before_canvas_columns_count++;
                    }
                }
            } else {
                DialogMultipaned *paned = dynamic_cast<DialogMultipaned *>(column);
                if (paned) {
                    multipanes.push_back(paned);
                }
            }
        }

        // Step 3.1: for each non-empty column, save its data.
        int column_count = 0; // non-empty columns count
        for (int column_idx = 0; column_idx < (int)multipanes.size(); ++column_idx) {
            Glib::ustring group_name = "Window" + std::to_string(window_idx) + "Column" + std::to_string(column_idx);
            int notebook_count = 0; // non-empty notebooks count

            // Step 3.1.0: for each notebook, get its dialogs' types
            for (auto const &columns_widget : multipanes[column_idx]->get_children()) {
                DialogNotebook *dialog_notebook = dynamic_cast<DialogNotebook *>(columns_widget);

                if (dialog_notebook) {
                    std::vector<Glib::ustring> dialogs;

                    for (auto const &widget : dialog_notebook->get_notebook()->get_children()) {
                        DialogBase *dialog = dynamic_cast<DialogBase *>(widget);
                        if (dialog) {
                            dialogs.push_back(dialog->get_type());
                        }
                    }

                    // save the dialogs type
                    Glib::ustring key = "Notebook" + std::to_string(notebook_count) + "Dialogs";
                    keyfile->set_string_list(group_name, key, dialogs);

                    // increase the notebook count
                    notebook_count++;
                }
            }

            // Step 3.1.1: increase the column count
            if (notebook_count != 0) {
                column_count++;
            }

            // Step 3.1.2: Save the column's data
            keyfile->set_integer(group_name, "NotebookCount", notebook_count);
            keyfile->set_boolean(group_name, "BeforeCanvas", (column_idx < before_canvas_columns_count));
        }

        // Step 3.2: save the window group
        Glib::ustring group_name = "Window" + std::to_string(window_idx);
        keyfile->set_integer(group_name, "ColumnCount", column_count);
        keyfile->set_boolean(group_name, "Floating", window_idx != 0);
        if (window_idx != 0) { // floating?
            if (auto wnd = dynamic_cast<DialogWindow *>(dialog_windows.at(window_idx))) {
                // store window position
                auto pos = dm_get_window_position(*wnd);
                save_wnd_position(keyfile.get(), group_name, pos ? &*pos : nullptr);
            }
        }
    }

    return keyfile;
}

// Signals -----------------------------------------------------

/**
 * No zombie windows. TODO: Need to work on this as it still leaves Gtk::Window! (?)
 */
void DialogContainer::on_unrealize() {
    // Disconnect all signals
    for_each(connections.begin(), connections.end(), [&](auto c) { c.disconnect(); });

    delete columns;
    columns = nullptr;

    parent_type::on_unrealize();
}

// Create a new notebook and move page.
DialogNotebook *DialogContainer::prepare_drop(const Glib::RefPtr<Gdk::DragContext> context)
{
    Gtk::Widget *source = Gtk::Widget::drag_get_source_widget(context);

    // Find source notebook and page
    Gtk::Notebook *old_notebook = dynamic_cast<Gtk::Notebook *>(source);
    if (!old_notebook) {
        std::cerr << "DialogContainer::prepare_drop: notebook not found!" << std::endl;
        return nullptr;
    }

    // Find page
    Gtk::Widget *page = old_notebook->get_nth_page(old_notebook->get_current_page());
    if (!page) {
        std::cerr << "DialogContainer::prepare_drop: page not found!" << std::endl;
        return nullptr;
    }

    // Create new notebook and move page.
    DialogNotebook *new_notebook = Gtk::manage(new DialogNotebook(this));
    new_notebook->move_page(*page);

    // move_page() takes care of updating dialog lists.
    return new_notebook;
}

// Notebook page dropped on prepend target. Call function to create new notebook and then insert.
void DialogContainer::prepend_drop(const Glib::RefPtr<Gdk::DragContext> context, DialogMultipaned *multipane)
{
    DialogNotebook *new_notebook = prepare_drop(context); // Creates notebook, moves page.
    if (!new_notebook) {
        std::cerr << "DialogContainer::prepend_drop: no new notebook!" << std::endl;
        return;
    }

    if (multipane->get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        // Columns
        // Create column
        DialogMultipaned *column = create_column();
        column->prepend(new_notebook);
        columns->prepend(column);
    } else {
        // Column
        multipane->prepend(new_notebook);
    }

    update_dialogs(); // Always update dialogs on Notebook change
}

// Notebook page dropped on append target. Call function to create new notebook and then insert.
void DialogContainer::append_drop(const Glib::RefPtr<Gdk::DragContext> context, DialogMultipaned *multipane)
{
    DialogNotebook *new_notebook = prepare_drop(context); // Creates notebook, moves page.
    if (!new_notebook) {
        std::cerr << "DialogContainer::append_drop: no new notebook!" << std::endl;
        return;
    }

    if (multipane->get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        // Columns
        // Create column
        DialogMultipaned *column = create_column();
        column->append(new_notebook);
        columns->append(column);
    } else {
        // Column
        multipane->append(new_notebook);
    }

    update_dialogs(); // Always update dialogs on Notebook change
}

/**
 * If a DialogMultipaned column is empty and it can be removed, remove it
 */
void DialogContainer::column_empty(DialogMultipaned *column)
{
    DialogMultipaned *parent = dynamic_cast<DialogMultipaned *>(column->get_parent());
    if (parent) {
        parent->remove(*column);
    }

    DialogWindow *window = dynamic_cast<DialogWindow *>(get_toplevel());
    if (window && parent) {
        auto children = parent->get_children();
        // Close the DialogWindow if you're in an empty one
        if (children.size() == 3 && parent->has_empty_widget()) {
            window->close();
        }
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
