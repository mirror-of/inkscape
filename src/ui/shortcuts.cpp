// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Shortcuts
 *
 * Copyright (C) 2020 Tavmjong Bah
 * Rewrite of code (C) MenTalguY and others.
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */


/* Much of the complexity of this code is in dealing with both Inkscape verbs and Gio::Actions at
 * the same time. When we remove verbs we can avoid using 'unsigned long long int shortcut' to
 * track keys and rely directly on Glib::ustring as used by
 * Gtk::Application::get_accels_for_action(). This will then automatically handle the '<Primary>'
 * modifier value (which takes care of the differences between Linux and OSX) as well as allowing
 * us to set multiple accelerators for actions in InkscapePreferences. */

#include "shortcuts.h"

#include <iostream>
#include <iomanip>

#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>

#include "preferences.h"
#include "inkscape-application.h"
#include "inkscape-window.h"

#include "helper/action-context.h"

#include "io/resource.h"
#include "io/dir-util.h"

#include "ui/modifiers.h"
#include "ui/tools/tool-base.h"    // For latin keyval
#include "ui/dialog/filedialog.h"  // Importing/exporting files.

#include "xml/simple-document.h"
#include "xml/node.h"
#include "xml/node-iterators.h"

using namespace Inkscape::IO::Resource;
using namespace Inkscape::Modifiers;

namespace Inkscape {

Shortcuts::Shortcuts()
{
    Glib::RefPtr<Gio::Application> gapp = Gio::Application::get_default();
    app = Glib::RefPtr<Gtk::Application>::cast_dynamic(gapp); // Save as we constantly use it.
    if (!app) {
        std::cerr << "Shortcuts::Shortcuts: No app! Shortcuts cannot be used without a Gtk::Application!" << std::endl;
    }
}


void
Shortcuts::init() {

    initialized = true;

    // Clear arrays (we may be re-reading).
    clear();

    bool success = false; // We've read a shortcut file!
    std::string path;
  
    // ------------ Open Inkscape shortcut file ------------

    // Try filename from preferences first.
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    path = prefs->getString("/options/kbshortcuts/shortcutfile");
    if (!path.empty()) {
        bool absolute = true;
        if (!Glib::path_is_absolute(path)) {
            path = get_path_string(SYSTEM, KEYS, path.c_str());
            absolute = false;
        }

        Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(path);
        success = read(file);
        if (!success) {
            std::cerr << "Shortcut::Shortcut: Unable to read shortcut file listed in preferences: " + path << std::endl;;
        }

        // Save relative path to "share/keys" if possible to handle parallel installations of
        // Inskcape gracefully.
        if (success && absolute) {
            std::string relative_path = sp_relative_path_from_path(path, std::string(get_path(SYSTEM, KEYS)));
            prefs->setString("/options/kbshortcuts/shortcutfile", relative_path.c_str());
        }
    }

    if (!success) {
        // std::cerr << "Shortcut::Shortcut: " << reason << ", trying default.xml" << std::endl;

        Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(get_path_string(SYSTEM, KEYS, "default.xml"));
        success = read(file);
    }
  
    if (!success) {
        std::cerr << "Shortcut::Shortcut: Failed to read file default.xml, trying inkscape.xml" << std::endl;

        Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(get_path_string(SYSTEM, KEYS, "inkscape.xml"));
        success = read(file);
    }

    if (!success) {
        std::cerr << "Shortcut::Shortcut: Failed to read file inkscape.xml; giving up!" << std::endl;
    }

 
    // ------------ Open User shortcut file -------------
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(get_path_string(USER, KEYS, "default.xml"));
    // Test if file exists before attempting to read to avoid generating warning message.
    if (file->query_exists()) {
        read(file, true);
    }

    // dump();
}

// Clear all shortcuts
void
Shortcuts::clear()
{
    // Actions: We rely on Gtk for everything except user/system setting.
    for (auto action_description : app->list_action_descriptions()) {
        app->unset_accels_for_action(action_description);
    }
    action_user_set.clear();
}

// This is only used in ToolBase (for F1, Tab, and Left-Tab).
// If this is ever used for more things, evaluate "get_from_event()".
bool
Shortcuts::invoke_action(GdkEventKey const *event)
{
    // Gtk::AccelKey shortcut = get_from_event(event);

    bool return_value = false;

    // This can be simplified in GTK4.
    Glib::ustring accel = Gtk::AccelGroup::name(event->keyval, Gdk::ModifierType(event->state));
    std::vector<Glib::ustring> actions = app->get_actions_for_accel(accel);
    if (!actions.empty()) {
        Glib::ustring action = actions[0];
        Glib::ustring action_name;
        Glib::VariantBase value;
        Gio::SimpleAction::parse_detailed_name_variant(action.substr(4), action_name, value);
        if (action.compare(0, 4, "app.") == 0) {
            app->activate_action(action_name, value);
            return_value = true;
        } else if (action.compare(0, 4, "win.") == 0) {
            auto window = dynamic_cast<InkscapeWindow *>(app->get_active_window());
            if (window) {
                window->activate_action(action_name, value);
                return_value = true;
            }
        }
    }
    return return_value;
}

Gdk::ModifierType
parse_modifier_string(gchar const *modifiers_string)
{
    Gdk::ModifierType modifiers(Gdk::ModifierType(0));
    if (modifiers_string) {
  
        Glib::ustring str(modifiers_string);
        std::vector<Glib::ustring> mod_vector = Glib::Regex::split_simple("\\s*,\\s*", modifiers_string);
  
        for (auto mod : mod_vector) {
            if (mod == "Control" || mod == "Ctrl") {
                modifiers |= Gdk::CONTROL_MASK;
            } else if (mod == "Shift") {
                modifiers |= Gdk::SHIFT_MASK;
            } else if (mod == "Alt") {
                modifiers |= Gdk::MOD1_MASK;
            } else if (mod == "Super") {
                modifiers |= Gdk::SUPER_MASK; // Not used
            } else if (mod == "Hyper") {
                modifiers |= Gdk::HYPER_MASK; // Not used
            } else if (mod == "Meta") {
                modifiers |= Gdk::META_MASK;
            } else if (mod == "Primary") {
  
                // System dependent key to invoke menus. (Needed for OSX in particular.)
                // We only read "Primary" and never write it.
                auto display = Gdk::Display::get_default();
                if (display) {
                    GdkKeymap* keymap = display->get_keymap();
                    GdkModifierType type = 
                        gdk_keymap_get_modifier_mask (keymap, GDK_MODIFIER_INTENT_PRIMARY_ACCELERATOR);
                    gdk_keymap_add_virtual_modifiers(keymap, &type);
                    if (type & Gdk::CONTROL_MASK)
                        modifiers |= Gdk::CONTROL_MASK;
                    else if (type & Gdk::META_MASK)
                        modifiers |= Gdk::META_MASK;
                    else {
                        std::cerr << "Shortcut::read: Unknown primary accelerator!" << std::endl;
                        modifiers |= Gdk::CONTROL_MASK;
                    }
                } else {
                    modifiers |= Gdk::CONTROL_MASK;
                }
            } else {
                std::cerr << "Shortcut::read: Unknown GDK modifier: " << mod.c_str() << std::endl;
            }
        }
    }
    return modifiers;
}


// Read a shortcut file.
bool
Shortcuts::read(Glib::RefPtr<Gio::File> file, bool user_set)
{
    if (!file->query_exists()) {
        std::cerr << "Shortcut::read: file does not exist: " << file->get_path() << std::endl;
        return false;
    }

    XML::Document *document = sp_repr_read_file(file->get_path().c_str(), nullptr);
    if (!document) {
        std::cerr << "Shortcut::read: could not parse file: " << file->get_path() << std::endl;
        return false;
    }

    XML::NodeConstSiblingIterator iter = document->firstChild();
    for ( ; iter ; ++iter ) { // We iterate in case of comments.
        if (strcmp(iter->name(), "keys") == 0) {
            break;
        }
    }

    if (!iter) {
        std::cerr << "Shortcuts::read: File in wrong format: " << file->get_path() << std::endl;
        return false;
    }

    // Loop through the children in <keys> (may have nested keys)
    _read(*iter, user_set);

    return true;
}

/**
 * Recursively reads shortcuts from shortcut file.
 *
 * @param keysnode The <keys> element. Its child nodes will be processed.
 * @param user_set true if reading from user shortcut file
 */
void
Shortcuts::_read(XML::Node const &keysnode, bool user_set)
{
    XML::NodeConstSiblingIterator iter {keysnode.firstChild()};
    for ( ; iter ; ++iter ) {

        if (strcmp(iter->name(), "modifier") == 0) {

            gchar const *mod_name = iter->attribute("action");
            if (!mod_name) {
                std::cerr << "Shortcuts::read: Missing modifier for action!" << std::endl;;
                continue;
            }

            Modifier *mod = Modifier::get(mod_name);
            if (mod == nullptr) {
                std::cerr << "Shortcuts::read: Can't find modifier: " << mod_name << std::endl; 
                continue;
            }
 
            // If mods isn't specified then it should use default, if it's an empty string
            // then the modifier is None (i.e. happens all the time without a modifier)
            KeyMask and_modifier = NOT_SET;
            gchar const *mod_attr = iter->attribute("modifiers");
            if (mod_attr) {
                and_modifier = (KeyMask) parse_modifier_string(mod_attr);
            }

            // Parse not (cold key) modifier
            KeyMask not_modifier = NOT_SET;
            gchar const *not_attr = iter->attribute("not_modifiers");
            if (not_attr) {
                not_modifier = (KeyMask) parse_modifier_string(not_attr);
            }

            gchar const *disabled_attr = iter->attribute("disabled");
            if (disabled_attr && strcmp(disabled_attr, "true") == 0) {
                and_modifier = NEVER;
            }

            if (and_modifier != NOT_SET) {
                if(user_set) {
                    mod->set_user(and_modifier, not_modifier);
                } else {
                    mod->set_keys(and_modifier, not_modifier);
                }
            }
            continue;
        } else if (strcmp(iter->name(), "keys") == 0) {
            _read(*iter, user_set);
            continue;
        } else if (strcmp(iter->name(), "bind") != 0) {
            // Unknown element, do not complain.
            continue;
        }

        // Gio::Action's
        gchar const *gaction = iter->attribute("gaction");
        gchar const *keys    = iter->attribute("keys");
        if (gaction && keys) {

            // Trim leading spaces
            Glib::ustring Keys = keys;
            auto p = Keys.find_first_not_of(" ");
            Keys = Keys.erase(0, p);

            std::vector<Glib::ustring> key_vector = Glib::Regex::split_simple("\\s*,\\s*", Keys);
            // Set one shortcut at a time so we can check if it has been previously used.
            for (auto key : key_vector) {
                add_shortcut(gaction, key, user_set);
            }

            // Uncomment to see what the cat dragged in.
            // if (!key_vector.empty()) {
            //     std::cout << "Shortcut::read: gaction: "<< gaction
            //               << ", user set: " << std::boolalpha << user_set << ", ";
            //     for (auto key : key_vector) {
            //         std::cout << key << ", ";
            //     }
            //     std::cout << std::endl;
            // }

            continue;
        }
    }
}

bool
Shortcuts::write_user() {
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(get_path_string(USER, KEYS, "default.xml"));
    return write(file, User);
}

// In principle, we only write User shortcuts. But for debugging, we might want to write something else.
bool
Shortcuts::write(Glib::RefPtr<Gio::File> file, What what) {

    auto *document = new XML::SimpleDocument();
    XML::Node * node = document->createElement("keys");
    switch (what) {
        case User:
            node->setAttribute("name", "User Shortcuts");
            break;
        case System:
            node->setAttribute("name", "System Shortcuts");
            break;
        default:
            node->setAttribute("name", "Inkscape Shortcuts");
    }

    document->appendChild(node);

    // Actions: write out all actions with accelerators.
    for (auto action_description : app->list_action_descriptions()) {
        if ( what == All                                 ||
            (what == System && !action_user_set[action_description]) ||
            (what == User   &&  action_user_set[action_description]) )
        {
            std::vector<Glib::ustring> accels = app->get_accels_for_action(action_description);
            if (!accels.empty()) {

                XML::Node * node = document->createElement("bind");

                node->setAttribute("gaction", action_description);

                Glib::ustring keys;
                for (auto accel : accels) {
                    keys += accel;
                    keys += ",";
                }
                keys.resize(keys.size() - 1);
                node->setAttribute("keys", keys);

                document->root()->appendChild(node);
            }
        }
    }

    for(auto modifier: Inkscape::Modifiers::Modifier::getList()) {
        if (what == User && modifier->is_set_user()) {
            XML::Node * node = document->createElement("modifier");
            node->setAttribute("action", modifier->get_id());

            if (modifier->get_config_user_disabled()) {
                node->setAttribute("disabled", "true");
            } else {
                node->setAttribute("modifiers", modifier->get_config_user_and());
                auto not_mask = modifier->get_config_user_not();
                if (!not_mask.empty() and not_mask != "-") {
                    node->setAttribute("not_modifiers", not_mask);
                }
            }

            document->root()->appendChild(node);
        }
    }

    sp_repr_save_file(document, file->get_path().c_str(), nullptr);
    GC::release(document);

    return true;
};

// Return if user set shortcut for Gio::Action.
bool
Shortcuts::is_user_set(Glib::ustring& action)
{
    auto it = action_user_set.find(action);
    if (it != action_user_set.end()) {
        return action_user_set[action];
    } else {
        return false;
    }
}

// Get a list of detailed action names (as defined in action extra data).
// This is more useful for shortcuts than a list of all actions.
std::vector<Glib::ustring>
Shortcuts::list_all_detailed_action_names()
{
    auto *iapp = InkscapeApplication::instance();
    InkActionExtraData& action_data = iapp->get_action_extra_data();
    return action_data.get_actions();
}

// Get a list of all actions (application, window, and document), properly prefixed.
// We need to do this ourselves as Gtk::Application does not have a function for this.
std::vector<Glib::ustring>
Shortcuts::list_all_actions()
{
    std::vector<Glib::ustring> all_actions;

    std::vector<Glib::ustring> actions = app->list_actions();
    std::sort(actions.begin(), actions.end());
    for (auto action : actions) {
        all_actions.emplace_back("app." + action);
    }

    auto gwindow = app->get_active_window();
    auto window = dynamic_cast<InkscapeWindow *>(gwindow);
    if (window) {
        std::vector<Glib::ustring> actions = window->list_actions();
        std::sort(actions.begin(), actions.end());
        for (auto action : actions) {
            all_actions.emplace_back("win." + action);
        }

        auto document = window->get_document();
        if (document) {
            auto map = document->getActionGroup();
            if (map) {
                std::vector<Glib::ustring> actions = map->list_actions();
                for (auto action : actions) {
                    all_actions.emplace_back("doc." + action);
                }
            } else {
                std::cerr << "Shortcuts::list_all_actions: No document map!" << std::endl;
            }
        }
    }

    return all_actions;
}


// Add a shortcut, removing any previous use of shortcut.
bool
Shortcuts::add_shortcut(Glib::ustring name, const Gtk::AccelKey& shortcut, bool user)
{
    // Remove previous use of shortcut (already removed if new user shortcut).
    if (Glib::ustring old_name = remove_shortcut(shortcut); old_name != "") {
        std::cerr << "Shortcut::add_shortcut: duplicate shortcut found for: " << shortcut.get_abbrev()
                  << "  Old: " << old_name << "  New: " << name << " !" << std::endl;
    }

    // Add shortcut

    // To see if action exists, We need to compare action names without values...
    Glib::ustring action_name_new;
    Glib::VariantBase value_new;
    Gio::SimpleAction::parse_detailed_name_variant(name, action_name_new, value_new);

    for (auto action : list_all_detailed_action_names()) {
        Glib::ustring action_name_old;
        Glib::VariantBase value_old;
        Gio::SimpleAction::parse_detailed_name_variant(action, action_name_old, value_old);

        if (action_name_new == action_name_old) {
            // Action exists, add shortcut to list of shortcuts.
            std::vector<Glib::ustring> accels = app->get_accels_for_action(name);
            accels.push_back(shortcut.get_abbrev());
            app->set_accels_for_action(name, accels);
            action_user_set[name] = user;
            return true;
        }
    }

    // Oops, not an action!
    std::cerr << "Shortcuts::add_shortcut: No Action for " << name << std::endl;
    return false;
}


// Add a user shortcut, updating user's shortcut file if successful.
bool
Shortcuts::add_user_shortcut(Glib::ustring name, const Gtk::AccelKey& shortcut)
{
    // Remove previous shortcut(s) for action.
    remove_shortcut(name);

    // Remove previous use of shortcut from other actions.
    remove_shortcut(shortcut);

    // Add shortcut, if successful, save to file.
    if (add_shortcut(name, shortcut, true)) {  // Always user.
        // Save
        return write_user();
    }

    std::cerr << "Shortcut::add_user_shortcut: Failed to add: " << name << " with shortcut " << shortcut.get_abbrev() << std::endl;
    return false;
};


// Remove a shortcut via key. Return name of removed action.
Glib::ustring
Shortcuts::remove_shortcut(const Gtk::AccelKey& shortcut)
{
    std::vector<Glib::ustring> actions = app->get_actions_for_accel(shortcut.get_abbrev());
    if (actions.empty()) {
        return Glib::ustring(); // No action, no pie.
    }

    Glib::ustring action_name;
    for (auto action : actions) {
        // Remove just the one shortcut, leaving the others intact.
        std::vector<Glib::ustring> accels = app->get_accels_for_action(action);
        auto it = std::find(accels.begin(), accels.end(), shortcut.get_abbrev());
        if (it != accels.end()) {
            action_name = action;
            accels.erase(it);
        }
        app->set_accels_for_action(action, accels);
    }

    return action_name;
}


// Remove a shortcut via action name.
bool
Shortcuts::remove_shortcut(Glib::ustring name)
{
    for (auto action : list_all_detailed_action_names()) {
        if (action == name) {
            // Action exists
            app->unset_accels_for_action(action);
            action_user_set.erase(action);
            return true;
        }
    }

    return false;
}

// Remove a user shortcut, updating user's shortcut file.
bool
Shortcuts::remove_user_shortcut(Glib::ustring name)
{
    // Check if really user shortcut.
    bool user_shortcut = is_user_set(name);

    if (!user_shortcut) {
        // We don't allow removing non-user shortcuts.
        return false;
    }

    if (remove_shortcut(name)) {
        // Save
        write_user();

        // Reread to get original shortcut (if any).
        init();
        return true;
    }

    std::cerr << "Shortcuts::remove_user_shortcut: Failed to remove shortcut for: " << name << std::endl;
    return false;
}


// Remove all user's shortcuts (simply overwrites existing file).
bool
Shortcuts::clear_user_shortcuts()
{
    // Create new empty document and save
    auto *document = new XML::SimpleDocument();
    XML::Node * node = document->createElement("keys");
    node->setAttribute("name", "User Shortcuts");
    document->appendChild(node);
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(get_path_string(USER, KEYS, "default.xml"));
    sp_repr_save_file(document, file->get_path().c_str(), nullptr);
    GC::release(document);
    
    // Re-read everything!
    init();
    return true;
}

Glib::ustring
Shortcuts::get_label(const Gtk::AccelKey& shortcut)
{
    Glib::ustring label;

    if (!shortcut.is_null()) {
        // ::get_label shows key pad and numeric keys identically.
        // TODO: Results in labels like "Numpad Alt+5"
        if (shortcut.get_abbrev().find("KP") != Glib::ustring::npos) {
            label += _("Numpad");
            label += " ";
        }

        label += Gtk::AccelGroup::get_label(shortcut.get_key(), shortcut.get_mod());
    }

    return label;
}

/*
 * Return: keyval translated to group 0 in lower 32 bits, modifier encoded in upper 32 bits.
 *
 * Usage of group 0 (i.e. the main, typically English layout) instead of simply event->keyval
 * ensures that shortcuts work regardless of the active keyboard layout (e.g. Cyrillic).
 *
 * The returned modifiers are the modifiers that were not "consumed" by the translation and
 * can be used by the application to define a shortcut, e.g.
 *  - when pressing "Shift+9" the resulting character is "(";
 *    the shift key was "consumed" to make this character and should not be part of the shortcut
 *  - when pressing "Ctrl+9" the resulting character is "9";
 *    the ctrl key was *not* consumed to make this character and must be included in the shortcut
 *  - Exception: letter keys like [A-Z] always need the shift modifier,
 *               otherwise lower case and uper case keys are treated as equivalent.
 */
Gtk::AccelKey
Shortcuts::get_from_event(GdkEventKey const *event, bool fix)
{
    // MOD2 corresponds to the NumLock key. Masking it out allows
    // shortcuts to work regardless of its state.
    Gdk::ModifierType initial_modifiers  = Gdk::ModifierType(event->state & ~Gdk::MOD2_MASK);
    unsigned int consumed_modifiers = 0;
    //Gdk::ModifierType consumed_modifiers = Gdk::ModifierType(0);

    unsigned int keyval = Inkscape::UI::Tools::get_latin_keyval(event, &consumed_modifiers);

    // If a key value is "convertible", i.e. it has different lower case and upper case versions,
    // convert to lower case and don't consume the "shift" modifier.
    bool is_case_convertible = !(gdk_keyval_is_upper(keyval) && gdk_keyval_is_lower(keyval));
    if (is_case_convertible) {
        keyval = gdk_keyval_to_lower(keyval);
        consumed_modifiers &= ~ Gdk::SHIFT_MASK;
    }

    // The InkscapePreferences dialog returns an event structure where the Shift modifier is not
    // set for keys like '('. This causes '(' to be converted to '9' by get_latin_keyval. It also
    // returns 'Shift-k' for 'K' (instead of 'Shift-K') but this is not a problem.
    // We fix this by restoring keyval to its original value.
    if (fix) {
        keyval = event->keyval;
    }

    auto unused_modifiers = Gdk::ModifierType((initial_modifiers &~ consumed_modifiers)
                                                                 & GDK_MODIFIER_MASK
                                                                 &~ GDK_LOCK_MASK);

    // std::cout << "Shortcuts::get_from_event: End:   "
    //           << " Key: " << std::hex << keyval << " (" << (char)keyval << ")"
    //           << " Mod: " << std::hex << unused_modifiers << std::endl;
    return (Gtk::AccelKey(keyval, unused_modifiers));
}


// Get a list of filenames to populate menu
std::vector<std::pair<Glib::ustring, Glib::ustring>>
Shortcuts::get_file_names()
{
    // TODO  Filenames should be std::string but that means changing the whole stack.
    using namespace Inkscape::IO::Resource;

    // Make a list of all key files from System and User.  Glib::ustring should be std::string!
    std::vector<Glib::ustring> filenames = get_filenames(SYSTEM, KEYS, {".xml"});
    // Exclude default.xml as it only contains user modifications.
    std::vector<Glib::ustring> filenames_user = get_filenames(USER, KEYS, {".xml"}, {"default.xml"});
    filenames.insert(filenames.end(), filenames_user.begin(), filenames_user.end());

    // Check file exists and extract out label if it does.
    std::vector<std::pair<Glib::ustring, Glib::ustring>> names_and_paths;
    for (auto &filename : filenames) {
        std::string label = Glib::path_get_basename(filename);
        Glib::ustring filename_relative = sp_relative_path_from_path(filename, std::string(get_path(SYSTEM, KEYS)));

        XML::Document *document = sp_repr_read_file(filename.c_str(), nullptr);
        if (!document) {
            std::cerr << "Shortcut::get_file_names: could not parse file: " << filename << std::endl;
            continue;
        }

        XML::NodeConstSiblingIterator iter = document->firstChild();
        for ( ; iter ; ++iter ) { // We iterate in case of comments.
            if (strcmp(iter->name(), "keys") == 0) {
                gchar const *name = iter->attribute("name");
                if (name) {
                    label = Glib::ustring(name) + " (" + label + ")";
                }
                std::pair<Glib::ustring, Glib::ustring> name_and_path = std::make_pair(label, filename_relative);
                names_and_paths.emplace_back(name_and_path);
                break;
            }
        }
        if (!iter) {
            std::cerr << "Shortcuts::get_File_names: not a shortcut keys file: " << filename << std::endl;
        }

        Inkscape::GC::release(document);
    }

    // Sort by name
    std::sort(names_and_paths.begin(), names_and_paths.end(),
            [](std::pair<Glib::ustring, Glib::ustring> pair1, std::pair<Glib::ustring, Glib::ustring> pair2) {
                return Glib::path_get_basename(pair1.first).compare(Glib::path_get_basename(pair2.first)) < 0;
            });

    // But default.xml at top
    auto it_default = std::find_if(names_and_paths.begin(), names_and_paths.end(),
            [](std::pair<Glib::ustring, Glib::ustring>& pair) {
                return !Glib::path_get_basename(pair.second).compare("default.xml");
            });
    if (it_default != names_and_paths.end()) {
        std::rotate(names_and_paths.begin(), it_default, it_default+1);
    }

    return names_and_paths;
}

// void on_foreach(Gtk::Widget& widget) {
//     std::cout <<  "on_foreach: " << widget.get_name() << std::endl;;
// }

/*
 * Update text with shortcuts.
 * Inkscape includes shortcuts in tooltips and in dialog titles. They need to be updated
 * anytime a tooltip is changed.
 */
void
Shortcuts::update_gui_text_recursive(Gtk::Widget* widget)
{

    // NOT what we want
    // auto activatable = dynamic_cast<Gtk::Activatable *>(widget);

    // We must do this until GTK4
    GtkWidget* gwidget = widget->gobj();
    bool is_actionable = GTK_IS_ACTIONABLE(gwidget);

    if (is_actionable) {
        const gchar* gaction = gtk_actionable_get_action_name(GTK_ACTIONABLE(gwidget));
        if (gaction) {
            Glib::ustring action = gaction;

            Glib::ustring variant;
            GVariant* gvariant = gtk_actionable_get_action_target_value(GTK_ACTIONABLE(gwidget));
            if (gvariant) {
                Glib::ustring type = g_variant_get_type_string(gvariant);
                if (type == "s") {
                    variant = g_variant_get_string(gvariant, nullptr);
                    action += "('" + variant + "')";
                } else if (type == "i") {
                    variant = std::to_string(g_variant_get_int32(gvariant));
                    action += "(" + variant + ")";
                } else {
                    std::cerr << "Shortcuts::update_gui_text_recursive: unhandled variant type: " << type << std::endl;
                }
            }

            std::vector<Glib::ustring> accels = app->get_accels_for_action(action);

            Glib::ustring tooltip;
            auto *iapp = InkscapeApplication::instance();
            if (iapp) {
                tooltip = iapp->get_action_extra_data().get_tooltip_for_action(action);
            }

            // Add new primary accelerator.
            if (accels.size() > 0) {

                // Add space between tooltip and accel if there is a tooltip
                if (!tooltip.empty()) {
                    tooltip += " ";
                }

                // Convert to more user friendly notation.
                unsigned int key = 0;
                Gdk::ModifierType mod = Gdk::ModifierType(0);
                Gtk::AccelGroup::parse(accels[0], key, mod);
                tooltip += "(" + Gtk::AccelGroup::get_label(key, mod) + ")";
            }

            // Update tooltip.
            widget->set_tooltip_text(tooltip);
        }
    }

    auto container = dynamic_cast<Gtk::Container *>(widget);
    if (container) {
        auto children = container->get_children();
        for (auto child : children) {
            update_gui_text_recursive(child);
        }
    }

}

// Dialogs

// Import user shortcuts from a file.
bool
Shortcuts::import_shortcuts() {

    // Users key directory.
    Glib::ustring directory = get_path_string(USER, KEYS, "");

    // Create and show the dialog
    Gtk::Window* window = app->get_active_window();
    Inkscape::UI::Dialog::FileOpenDialog *importFileDialog =
        Inkscape::UI::Dialog::FileOpenDialog::create(*window, directory, Inkscape::UI::Dialog::CUSTOM_TYPE, _("Select a file to import"));
    importFileDialog->addFilterMenu(_("Inkscape shortcuts (*.xml)"), "*.xml");
    bool const success = importFileDialog->show();

    if (!success) {
        delete importFileDialog;
        return false;
    }

    // Get file name and read.
    Glib::ustring path = importFileDialog->getFilename(); // It's a full path, not just a filename!
    delete importFileDialog;

    Glib::RefPtr<Gio::File> file_read = Gio::File::create_for_path(path);
    if (!read(file_read, true)) {
        std::cerr << "Shortcuts::import_shortcuts: Failed to read file!" << std::endl;
        return false;
    }

    // Save
    return write_user();
};

bool
Shortcuts::export_shortcuts() {

    // Users key directory.
    Glib::ustring directory = get_path_string(USER, KEYS, "");

    // Create and show the dialog
    Gtk::Window* window = app->get_active_window();
    Inkscape::UI::Dialog::FileSaveDialog *saveFileDialog =
        Inkscape::UI::Dialog::FileSaveDialog::create(*window, directory, Inkscape::UI::Dialog::CUSTOM_TYPE, _("Select a filename for export"),
                                                     "", "", Inkscape::Extension::FILE_SAVE_METHOD_SAVE_AS);
    saveFileDialog->addFileType(_("Inkscape shortcuts (*.xml)"), "*.xml");
    bool success = saveFileDialog->show();

    // Get file name and write.
    if (success) {
        Glib::ustring path = saveFileDialog->getFilename(); // It's a full path, not just a filename!
        if (path.size() > 0) {
            Glib::ustring newFileName = Glib::filename_to_utf8(path);  // Is this really correct? (Paths should be std::string.)
            Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(path);
            success = write(file, User);
        } else {
            // Can this ever happen?
            success = false;
        }
    }

    delete saveFileDialog;

    return success;
};


// For debugging.
void
Shortcuts::dump() {

    // What shortcuts are being used?
    std::vector<Gdk::ModifierType> modifiers {
        Gdk::ModifierType(0),
        Gdk::SHIFT_MASK,
        Gdk::CONTROL_MASK,
        Gdk::MOD1_MASK,
        Gdk::SHIFT_MASK   |  Gdk::CONTROL_MASK,
        Gdk::SHIFT_MASK   |  Gdk::MOD1_MASK,
        Gdk::CONTROL_MASK |  Gdk::MOD1_MASK,
        Gdk::SHIFT_MASK   |  Gdk::CONTROL_MASK   | Gdk::MOD1_MASK
    };
    for (auto mod : modifiers) {
        for (gchar key = '!'; key <= '~'; ++key) {

            Glib::ustring action;
            Glib::ustring accel = Gtk::AccelGroup::name(key, mod);
            std::vector<Glib::ustring> actions = app->get_actions_for_accel(accel);
            if (!actions.empty()) {
                action = actions[0];
            }

            std::cout << "  shortcut:"
                      << "  " << std::setw(8) << std::hex << mod
                      << "  " << std::setw(8) << std::hex << key
                      << "  " << std::setw(30) << std::left << accel
                      << "  " << action
                      << std::endl;
        }
    }
}

void
Shortcuts::dump_all_recursive(Gtk::Widget* widget)
{
    static unsigned int indent = 0;
    ++indent;
    for (int i = 0; i < indent; ++i) std::cout << "  ";

    // NOT what we want
    // auto activatable = dynamic_cast<Gtk::Activatable *>(widget);

    // We must do this until GTK4
    GtkWidget* gwidget = widget->gobj();
    bool is_actionable = GTK_IS_ACTIONABLE(gwidget);
    Glib::ustring action;
    if (is_actionable) {
        const gchar* gaction = gtk_actionable_get_action_name( GTK_ACTIONABLE(gwidget) );
        if (gaction) {
            action = gaction;
        }
    }

    std::cout << widget->get_name()
              << ":   actionable: " << std::boolalpha << is_actionable
              << ":   " << widget->get_tooltip_text()
              << ":   " << action
              << std::endl;
    auto container = dynamic_cast<Gtk::Container *>(widget);
    if (container) {
        auto children = container->get_children();
        for (auto child : children) {
            dump_all_recursive(child);
        }
    }
    --indent;
}


} // Namespace

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
