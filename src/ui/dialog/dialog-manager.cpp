// SPDX-License-Identifier: GPL-2.0-or-later

#include "dialog-manager.h"

#include <gdkmm/monitor.h>
#include <limits>
#include <boost/filesystem.hpp>
namespace filesystem = boost::filesystem;
// #include <filesystem> - waiting for compiler on MacOS to catch up to C++x17

#include "io/resource.h"
#include "inkscape-application.h"
#include "dialog-base.h"
#include "dialog-container.h"
#include "dialog-window.h"
#include "enums.h"
#include "preferences.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

std::optional<window_position_t> dm_get_window_position(Gtk::Window &window)
{
    std::optional<window_position_t> position = std::nullopt;

    const int max = std::numeric_limits<int>::max();
    int x = max;
    int y = max;
    int width = 0;
    int height = 0;
    // gravity NW to include window decorations
    window.property_gravity() = Gdk::GRAVITY_NORTH_WEST;
    window.get_position(x, y);
    window.get_size(width, height);

    if (x != max && y != max && width > 0 && height > 0) {
        position = window_position_t{x, y, width, height};
    }

    return position;
}

void dm_restore_window_position(Gtk::Window &window, const window_position_t &position)
{
    // note: Gtk window methods are recommended over low-level Gdk ones to resize and position window
    window.property_gravity() = Gdk::GRAVITY_NORTH_WEST;
    window.set_default_size(position.width, position.height);
    // move & resize positions window on the screen making sure it is not clipped
    // (meaning it is visible; this works with two monitors too)
    window.move(position.x, position.y);
    window.resize(position.width, position.height);
}

DialogManager &DialogManager::singleton()
{
    static DialogManager dm;
    return dm;
}

// store complete dialog window state (including its container state)
void DialogManager::store_state(DialogWindow &wnd)
{
    // get window's size and position
    if (auto pos = dm_get_window_position(wnd)) {
        if (auto container = wnd.get_container()) {
            // get container's state
            auto state = container->get_container_state(&*pos);

            // find dialogs hosted in this window
            for (auto dlg : *container->get_dialogs()) {
                _floating_dialogs[dlg.first] = state;
            }
        }
    }
}

//
bool DialogManager::should_open_floating(const Glib::ustring& dialog_type)
{
    return _floating_dialogs.count(dialog_type) > 0;
}

void DialogManager::set_floating_dialog_visibility(DialogWindow* wnd, bool show) {
    if (!wnd) return;

    if (show) {
        if (wnd->is_visible()) return;

        // wnd->present(); - not sure which one is better, show or present...
        wnd->show();
        _hidden_dlg_windows.erase(wnd);
        // re-add it to application; hiding removed it
        if (auto app = InkscapeApplication::instance()) {
            app->gtk_app()->add_window(*wnd);
        }
    }
    else {
        if (!wnd->is_visible()) return;

        _hidden_dlg_windows.insert(wnd);
        wnd->hide();
    }
}

std::vector<DialogWindow*> DialogManager::get_all_floating_dialog_windows() {
    std::vector<Gtk::Window*> windows = InkscapeApplication::instance()->gtk_app()->get_windows();

    std::vector<DialogWindow*> result(_hidden_dlg_windows.begin(), _hidden_dlg_windows.end());
    for (auto wnd : windows) {
        if (auto dlg_wnd = dynamic_cast<DialogWindow*>(wnd)) {
            result.push_back(dlg_wnd);
        }
    }

    return result;
}

DialogWindow* DialogManager::find_floating_dialog_window(const Glib::ustring& dialog_type) {
    auto windows = get_all_floating_dialog_windows();

    for (auto dlg_wnd : windows) {
        if (auto container = dlg_wnd->get_container()) {
            if (auto dlg = container->get_dialog(dialog_type)) {
                return dlg_wnd;
            }
        }
    }

    return nullptr;
}

DialogBase *DialogManager::find_floating_dialog(const Glib::ustring& dialog_type)
{
    auto windows = get_all_floating_dialog_windows();

    for (auto dlg_wnd : windows) {
        if (auto container = dlg_wnd->get_container()) {
            if (auto dlg = container->get_dialog(dialog_type)) {
                return dlg;
            }
        }
    }

    return nullptr;
}

std::shared_ptr<Glib::KeyFile> DialogManager::find_dialog_state(const Glib::ustring& dialog_type)
{
    auto it = _floating_dialogs.find(dialog_type);
    if (it != _floating_dialogs.end()) {
        return it->second;
    }
    return nullptr;
}

const char dialogs_state[] = "dialogs-state-ex.ini";
const char save_dialog_position[] = "/options/savedialogposition/value";
const char transient_group[] = "transient";
const char floating_group[] = "floating";

// list of dialogs sharing the same state
std::vector<Glib::ustring> DialogManager::count_dialogs(const Glib::KeyFile *state) const
{
    std::vector<Glib::ustring> dialogs;
    if (!state) return dialogs;

    for (auto dlg : _floating_dialogs) {
        if (dlg.second.get() == state) {
            dialogs.push_back(dlg.first);
        }
    }
    return dialogs;
}

void DialogManager::save_dialogs_state(DialogContainer *docking_container)
{
    if (!docking_container) return;

    // check if we want to save the state
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int save_state = prefs->getInt(save_dialog_position, PREFS_DIALOGS_STATE_SAVE);
    if (save_state == PREFS_DIALOGS_STATE_NONE) return;

    // save state of docked dialogs and currently open floating ones
    auto keyfile = docking_container->save_container_state();

    // save transient state of floating dialogs that user might have opened interacting with the app
    int idx = 1;
    for (auto dlg : _floating_dialogs) {
        auto state = dlg.second.get();
        auto&& type = dlg.first;
        auto index = std::to_string(idx++);
        // state may be empty; all that means it that dialog hasn't been opened yet,
        // but when it is, then it should be open in a floating state
        keyfile->set_string(transient_group, "state" + index, state ? state->to_data() : "");
        auto dialogs = count_dialogs(state);
        if (!state) {
            dialogs.push_back(type);
        }
        keyfile->set_string_list(transient_group, "dialogs" + index, dialogs);
    }
    keyfile->set_integer(transient_group, "count", _floating_dialogs.size());

    std::string filename = Glib::build_filename(Inkscape::IO::Resource::profile_path(), dialogs_state);
    try {
        keyfile->save_to_file(filename);
    } catch (Glib::FileError &error) {
        std::cerr << G_STRFUNC << ": " << error.what() << std::endl;
    }
}

// load transient dialog state - it includes state of floating dialogs that may or may not be open
void DialogManager::load_transient_state(Glib::KeyFile *file)
{
    int count = file->get_integer(transient_group, "count");
    for (int i = 0; i < count; ++i) {
        auto index = std::to_string(i + 1);
        auto dialogs = file->get_string_list(transient_group, "dialogs" + index);
        auto state = file->get_string(transient_group, "state" + index);

        auto keyfile = std::make_shared<Glib::KeyFile>();
        if (!state.empty()) {
            keyfile->load_from_data(state);
        }
        for (auto type : dialogs) {
            _floating_dialogs[type] = keyfile;
        }
    }
}

// restore state of dialogs; populate docking container and open visible floating dialogs
void DialogManager::restore_dialogs_state(DialogContainer *docking_container, bool include_floating)
{
    if (!docking_container) return;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int save_state = prefs->getInt(save_dialog_position, PREFS_DIALOGS_STATE_SAVE);
    if (save_state == PREFS_DIALOGS_STATE_NONE) return;

    try {
        auto keyfile = std::make_unique<Glib::KeyFile>();
        std::string filename = Glib::build_filename(Inkscape::IO::Resource::profile_path(), dialogs_state);
        if (filesystem::exists(filename) && keyfile->load_from_file(filename)) {
            // restore visible dialogs first; that state is up-to-date
            docking_container->load_container_state(keyfile.get(), include_floating);

            // then load transient data too; it may be older than above
            if (include_floating) {
                try {
                    load_transient_state(keyfile.get());
                } catch (Glib::Error &error) {
                    std::cerr << G_STRFUNC << ": transient state not loaded - " << error.what() << std::endl;
                }
            }
        }
        else {
            // state not available or not valid; prepare defaults
            dialog_defaults();
        }
    } catch (Glib::Error &error) {
        std::cerr << G_STRFUNC << ": dialogs state not loaded - " << error.what() << std::endl;
    }
}

void DialogManager::remove_dialog_floating_state(const Glib::ustring& dialog_type) {
    auto it = _floating_dialogs.find(dialog_type);
    if (it != _floating_dialogs.end()) {
        _floating_dialogs.erase(it);
    }
}

// apply defaults when dialog state cannot be loaded / doesn't exist:
// here we can define which dialogs should by default be open floating rather then docked
void DialogManager::dialog_defaults() {
    std::shared_ptr<Glib::KeyFile> floating;
    // strings are dialog types
    _floating_dialogs["DocumentProperties"] = floating;
    _floating_dialogs["FilterEffects"] = floating;
    _floating_dialogs["Input"] = floating;
    _floating_dialogs["Preferences"] = floating;
    _floating_dialogs["XMLEditor"] = floating;
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
