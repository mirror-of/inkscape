// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * Desktop main menu bar code.
 */
/*
 * Authors:
 *   Tavmjong Bah       <tavmjong@free.fr>
 *   Alex Valavanis     <valavanisalex@gmail.com>
 *   Patrick Storz      <eduard.braun2@gmx.de>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Kris De Gussem     <Kris.DeGussem@gmail.com>
 *   Sushant A.A.       <sushant.co19@gmail.com>
 *
 * Copyright (C) 2018 Authors
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 * Read the file 'COPYING' for more information.
 *
 */

#include "menubar.h"

#include <iostream>
#include <iomanip>
#include <map>

#include "inkscape-application.h" // Open recent
#include "preferences.h"          // Use icons or not
#include "io/resource.h"          // UI File location

// =================== Main Menu ================
void
build_menu()
{
    std::string filename = Inkscape::IO::Resource::get_filename(Inkscape::IO::Resource::UIS, "menus.ui");
    auto refBuilder = Gtk::Builder::create();

    try
    {
        refBuilder->add_from_file(filename);
    }
    catch (const Glib::Error& err)
    {
        std::cerr << "build_menu: failed to load Main menu from: "
                    << filename <<": "
                    << err.what() << std::endl;
    }

    const auto object = refBuilder->get_object("menus");
#if GTK_CHECK_VERSION(4, 0 ,0)
    const auto gmenu = std::dynamic_pointer_cast<Gio::Menu>(object);
#else
    const auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
#endif

    if (!gmenu) {
        std::cerr << "build_menu: failed to build Main menu!" << std::endl;
    } else {

        static auto app = InkscapeApplication::instance();

        { // Filters and Extensions

            auto effects_object = refBuilder->get_object("effect-menu-effects");
            auto filters_object = refBuilder->get_object("filter-menu-filters");
            auto effects_menu   = Glib::RefPtr<Gio::Menu>::cast_dynamic(effects_object);
            auto filters_menu   = Glib::RefPtr<Gio::Menu>::cast_dynamic(filters_object);

            if (!filters_menu) {
                std::cerr << "build_menu(): Couldn't find Filters menu entry!" << std::endl;
            }
            if (!effects_menu) {
                std::cerr << "build_menu(): Couldn't find Extensions menu entry!" << std::endl;
            }

            std::map<Glib::ustring, Glib::RefPtr<Gio::Menu>> submenus;

            for (auto [ entry_id, submenu_name_list, entry_name ] : app->get_action_effect_data().give_all_data())
            {
                if (submenu_name_list.size() > 0) {

                    // Effect data is used for both filters menu and extensions menu... we need to
                    // add to correct menu. 'submenu_name_list' either starts with 'Effects' or 'Filters'.
                    // Note "Filters" is translated!
                    Glib::ustring path; // Only used as index to map of submenus.
                    auto top_menu = filters_menu;
                    if (submenu_name_list.front() == "Effects") {
                        top_menu = effects_menu;
                        path += "Effects";
                    } else {
                        path += "Filters";
                    }
                    submenu_name_list.pop_front();

                    if (top_menu) { // It's possible that the menu doesn't exist (Kid's Inkscape?)
                        auto current_menu = top_menu;
                        for (auto submenu_name : submenu_name_list) {
                            path += submenu_name + "-";
                            auto it = submenus.find(path);
                            if (it == submenus.end()) {
                                auto new_gsubmenu = Gio::Menu::create();
                                submenus[path] = new_gsubmenu;
                                current_menu->append_submenu(submenu_name, new_gsubmenu);
                                current_menu = new_gsubmenu;
                            } else {
                                current_menu = it->second;
                            }
                        }
                        current_menu->append(entry_name, "app." + entry_id);
                    } else {
                        std::cerr << "build_menu(): menu doesn't exist!" << std::endl; // Warn for now.
                    }
                }
            }
        }

        { // Recent file
            auto recent_manager = Gtk::RecentManager::get_default();
            auto recent_files = recent_manager->get_items(); // all recent files not necessarily inkscape only

            int max_files = Inkscape::Preferences::get()->getInt("/options/maxrecentdocuments/value");

            auto sub_object = refBuilder->get_object("recent-files");
            auto sub_gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(sub_object);

            for (auto const &recent_file : recent_files) {
                // check if given was generated by inkscape
                bool valid_file = recent_file->has_application(g_get_prgname()) or
                                recent_file->has_application("org.inkscape.Inkscape") or
                                recent_file->has_application("inkscape")
#ifdef _WIN32
                                or recent_file->has_application("inkscape.exe")
#endif
                                ;

                valid_file = valid_file and recent_file->exists();

                if (not valid_file) {
                    continue;
                }

                if (max_files-- <= 0) {
                    break;
                }

                std::string action_name = "app.file-open-window('"+recent_file->get_uri_display()+"')";
                sub_gmenu->append(recent_file->get_short_name(),action_name);
            }
        }

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        auto useicons = static_cast<UseIcons>(prefs->getInt("/theme/menuIcons", 0));
        if (useicons != UseIcons::always) {
            // Remove all or some icons.
            auto gmenu_copy = Gio::Menu::create();
            rebuild_menu (gmenu, gmenu_copy, useicons);
            app->gtk_app()->set_menubar(gmenu_copy);
        } else {
            // Show all icons.
            app->gtk_app()->set_menubar(gmenu);
        }
    }
}


/*
 * Disable all or some menu icons.
 *
 * This is quite nasty:
 *
 * We must disable icons in the Gio::Menu as there is no way to pass
 * the needed information to the children of Gtk::PopoverMenu and no
 * way to set visibility via CSS.
 *
 * MenuItems are immutable and not copyable so you have to recreate
 * the menu tree. The format for accessing MenuItem data is not the
 * same as what you need to create a new MenuItem.
 *
 * NOTE: Input is a Gio::MenuModel, Output is a Gio::Menu!!
 */
#if GTK_CHECK_VERSION(4, 0, 0)
void rebuild_menu (std::shared_ptr<Gio::MenuModel> menu, std::shared_ptr<Gio::Menu> menu_copy, UseIcons useIcons) {
#else
void rebuild_menu (Glib::RefPtr<Gio::MenuModel>    menu, Glib::RefPtr<Gio::Menu>    menu_copy, UseIcons useIcons) {
#endif

    for (int i = 0; i < menu->get_n_items(); ++i) {

        Glib::ustring label;
        Glib::ustring action;
        Glib::ustring target;
        Glib::VariantBase icon;
        Glib::ustring use_icon;
        std::map<Glib::ustring, Glib::VariantBase> attributes;

        auto attribute_iter = menu->iterate_item_attributes(i);
        while (attribute_iter->next()) {

            // Attributes we need to create MenuItem or set icon.
            if          (attribute_iter->get_name() == "label") {
                label    = attribute_iter->get_value().print();
                label.erase(0, 1);
                label.erase(label.size()-1, 1);
            } else if   (attribute_iter->get_name() == "action") {
                action  = attribute_iter->get_value().print();
                action.erase(0, 1);
                action.erase(action.size()-1, 1);
            } else if   (attribute_iter->get_name() == "target") {
                target  = attribute_iter->get_value().print();
            } else if   (attribute_iter->get_name() == "icon") {
                icon     = attribute_iter->get_value();
            } else if (attribute_iter->get_name() == "use-icon") {
                use_icon =  attribute_iter->get_value().print();
            } else {
                // All the remaining attributes.
                attributes[attribute_iter->get_name()] = attribute_iter->get_value();
            }
        }
        Glib::ustring detailed_action = action;
        if (target.size() > 0) {
            detailed_action += "(" + target + ")";
        }

        // std::cout << "  label: " << std::setw(30) << label
        //           << "  use_icon (.ui): " << std::setw(6) << use_icon
        //           << "  icon: " << (icon ? "yes" : "no ")
        //           << "  useIcons: " << (int)useIcons
        //           << "  use_icon.size(): " << use_icon.size()
        //           << std::endl;
        auto menu_item = Gio::MenuItem::create(label, detailed_action);
        if (icon &&
            (useIcons == UseIcons::always ||
             (useIcons == UseIcons::as_requested && use_icon.size() > 0))) {
            menu_item->set_attribute_value("icon", icon);
        }

        // Add remaining attributes
        for (auto const& [key, value] : attributes) {
            menu_item->set_attribute_value(key, value);
        }

        // Add submenus
        auto link_iter = menu->iterate_item_links(i);
        while (link_iter->next()) {
            auto submenu = Gio::Menu::create();
            if (link_iter->get_name() == "submenu") {
                menu_item->set_submenu(submenu);
            } else if (link_iter->get_name() == "section") {
                menu_item->set_section(submenu);
            } else {
                std::cerr << "rebuild_menu: Unknown link type: " << link_iter->get_name() << std::endl;
            }
            rebuild_menu (link_iter->get_value(), submenu, useIcons);
        }

        menu_copy->append_item(menu_item);
    }
}

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
