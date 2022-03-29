// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * Context menu
 */
/* Authors:
 *   Tavmjong Bah
 *
 * Rewrite of code authored by:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2022 Tavmjong Bah
 * Copyright (C) 2012 Kris De Gussem
 * Copyright (C) 2010 authors
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "contextmenu.h"

#include <glibmm/i18n.h>

#include "desktop.h"
#include "document.h"
#include "layer-manager.h"
#include "page-manager.h"
#include "selection.h"

#include "object/sp-anchor.h"
#include "object/sp-image.h"
#include "object/sp-page.h"
#include "object/sp-shape.h"
#include "object/sp-text.h"

#include "ui/desktop/menu-icon-shift.h"

ContextMenu::ContextMenu(SPDesktop *desktop, SPObject *object, bool hide_layers_and_objects_menu_item)
{
    set_name("ContextMenu");

    SPItem *item = dynamic_cast<SPItem *>(object);

    // std::cout << "ContextMenu::ContextMenu: " << (item ? item->getId() : "no item") << std::endl;
    action_group = Gio::SimpleActionGroup::create();
    insert_action_group("ctx", action_group);
    auto document = desktop->getDocument();
    action_group->add_action("unhide-objects-below-cursor", sigc::bind<SPDocument*, bool>(sigc::mem_fun(this, &ContextMenu::unhide_or_unlock), document, true));
    action_group->add_action("unlock-objects-below-cursor", sigc::bind<SPDocument*, bool>(sigc::mem_fun(this, &ContextMenu::unhide_or_unlock), document, false));

    auto gmenu         = Gio::Menu::create(); // Main menu
    auto gmenu_section = Gio::Menu::create(); // Section (used multiple times)

    auto layer = Inkscape::LayerManager::asLayer(item);  // Layers have their own context menu in the Object and Layers dialog.
    auto root = desktop->layerManager().currentRoot();

    // Get a list of items under the cursor, used for unhiding and unlocking.
    auto point_document = desktop->point() * desktop->dt2doc();
    Geom::Rect b(point_document, point_document + Geom::Point(1, 1)); // Seems strange to use a rect!
    items_under_cursor = document->getItemsPartiallyInBox(desktop->dkey, b, true, true, true, true);
    bool has_hidden_below_cursor = false;
    bool has_locked_below_cursor = false;
    for (auto item : items_under_cursor) {
        if (item->isHidden()) {
            has_hidden_below_cursor = true;
        }
        if (item->isLocked()) {
            has_locked_below_cursor = true;
        }
    }
    // std::cout << "Items below cursor: " << items_under_cursor.size()
    //           << "  hidden: " << std::boolalpha << has_hidden_below_cursor
    //           << "  locked: " << std::boolalpha << has_locked_below_cursor
    //           << std::endl;

    // clang-tidy off

    // Undo/redo
    // gmenu_section = Gio::Menu::create();
    // AppendItemFromAction(gmenu_section, "doc.undo",      _("Undo"),       "edit-undo");
    // AppendItemFromAction(gmenu_section, "doc.redo",      _("Redo"),       "edit-redo");
    // gmenu->append_section(gmenu_section);

    if (auto page = dynamic_cast<SPPage *>(object)) {
        auto &page_manager = document->getPageManager();
        page_manager.selectPage(page);

        gmenu_section = Gio::Menu::create();
        AppendItemFromAction(gmenu_section, "doc.page-new", _("_New Page"), "pages-add");
        gmenu->append_section(gmenu_section);

        gmenu_section = Gio::Menu::create();
        AppendItemFromAction(gmenu_section, "doc.page-delete", _("_Delete Page"), "pages-remove");
        AppendItemFromAction(gmenu_section, "doc.page-move-backward", _("Move Page _Backward"), "pages-order-backwards");
        AppendItemFromAction(gmenu_section, "doc.page-move-forward", _("Move Page _Forward"), "pages-order-forwards");
        gmenu->append_section(gmenu_section);

    } else if (!layer) {
        // "item" is the object that was under the mouse when right-clicked. It determines what is shown
        // in the menu thus it makes the most sense that it is either selected or part of the current
        // selection.
        auto selection = desktop->selection;
        if (object && !selection->includes(object)) {
            selection->set(object);
        }

        gmenu_section = Gio::Menu::create();
        AppendItemFromAction(gmenu_section, "app.cut",       _("Cu_t"),       "edit-cut");
        AppendItemFromAction(gmenu_section, "app.copy",      _("_Copy"),      "edit-copy");
        AppendItemFromAction(gmenu_section, "win.paste",     _("_Paste"),     "edit-paste");
        gmenu->append_section(gmenu_section);

        gmenu_section = Gio::Menu::create();
        AppendItemFromAction(gmenu_section, "app.duplicate", _("Duplic_ate"), "edit-duplicate");
        AppendItemFromAction(gmenu_section, "app.delete-selection", _("_Delete"), "edit-delete");
        gmenu->append_section(gmenu_section);

        if (item) {

            // Dialogs
            auto gmenu_dialogs = Gio::Menu::create();
            if (!hide_layers_and_objects_menu_item) { // Hidden when context menu is popped up in Layers and Objects dialog!
                AppendItemFromAction(gmenu_dialogs, "win.dialog-open('Objects')",                   _("Layers and Objects..."), "dialog-objects"             );
            }
            AppendItemFromAction(gmenu_dialogs,     "win.dialog-open('ObjectProperties')",          _("_Object Properties..."), "dialog-object-properties"   );

            if (dynamic_cast<SPShape*>(item) || dynamic_cast<SPText*>(item) || dynamic_cast<SPGroup*>(item)) {
                AppendItemFromAction(gmenu_dialogs, "win.dialog-open('FillStroke')",                _("_Fill and Stroke..."),   "dialog-fill-and-stroke"     );
            }

            // Image dialogs (mostly).
            if (dynamic_cast<SPImage*>(item)) {
                AppendItemFromAction(     gmenu_dialogs, "win.dialog-open('ObjectAttributes')",          _("Image _Properties..."),  "dialog-fill-and-stroke");
                AppendItemFromAction(     gmenu_dialogs, "win.dialog-open('Trace')",                     _("_Trace Bitmap..."),      "bitmap-trace"          );
                auto image = dynamic_cast<SPImage*>(item);
                if (strncmp(image->href, "data", 4) == 0) {
                    // Image is embedded.
                    AppendItemFromAction( gmenu_dialogs, "app.org.inkscape.filter.extract_image",        _("Extract Image..."),      ""                      );
                } else {
                    // Image is linked.
                    AppendItemFromAction( gmenu_dialogs, "app.org.inkscape.filter.selected.embed_image", _("Embed Image"),           ""                      );
                    AppendItemFromAction( gmenu_dialogs, "app.element-image-edit",                       _("Edit Externally..."),    ""                      );
                }
            }

            // Text dialogs.
            if (dynamic_cast<SPText*>(item)) {
                AppendItemFromAction(     gmenu_dialogs, "win.dialog-open('Text')",                      _("_Text and Font..."),     "dialog-text-and-font"  );
                AppendItemFromAction(     gmenu_dialogs, "win.dialog-open('Spellcheck')",                _("Check Spellin_g..."),    "tools-check-spelling"  );
            }
            gmenu->append_section(gmenu_dialogs); // We might add to it later...

            if (!dynamic_cast<SPAnchor*>(item)) {
                // Item menu

                // Selection
                gmenu_section = Gio::Menu::create();
                auto gmenu_submenu = Gio::Menu::create();
                AppendItemFromAction(     gmenu_submenu, "win.select-same-fill-and-stroke",     _("Fill _and Stroke"),      "edit-select-same-fill-and-stroke");
                AppendItemFromAction(     gmenu_submenu, "win.select-same-fill",                _("_Fill Color"),           "edit-select-same-fill"           );
                AppendItemFromAction(     gmenu_submenu, "win.select-same-stroke-color",        _("_Stroke Color"),         "edit-select-same-stroke-color"   );
                AppendItemFromAction(     gmenu_submenu, "win.select-same-stroke-style",        _("Stroke St_yle"),         "edit-select-same-stroke-style"   );
                AppendItemFromAction(     gmenu_submenu, "win.select-same-object-type",         _("_Object Type"),          "edit-select-same-object-type"    );
                gmenu_section->append_submenu(_("Select Sa_me"), gmenu_submenu);
                gmenu->append_section(gmenu_section);

                // Groups and Layers
                gmenu_section = Gio::Menu::create();
                AppendItemFromAction(     gmenu_section, "win.selection-move-to-layer",         _("_Move to Layer..."),     ""                                );
                AppendItemFromAction(     gmenu_section, "app.selection-link",                  _("Create anchor (hyperlink)"),   ""                          );
                AppendItemFromAction(     gmenu_section, "app.selection-group",                 _("_Group"),                ""                                );
                if (dynamic_cast<SPGroup*>(item)) {
                    AppendItemFromAction( gmenu_section, "app.selection-ungroup",               _("_Ungroup"),              ""                                );
                    Glib::ustring label = Glib::ustring::compose(_("Enter group %1"), item->defaultLabel());
                    AppendItemFromAction( gmenu_section, "win.selection-group-enter",           label,                      ""                                );
                    if (item->getParentGroup()->isLayer() || item->getParentGroup() == root) {
                        // A layer should be a child of root or another layer.
                        AppendItemFromAction( gmenu_section, "win.layer-from-group",            _("Group to Layer"),        ""                                );
                    }
                }
                auto group = dynamic_cast<SPGroup*>(item->parent);
                if (group && !group->isLayer()) {
                    AppendItemFromAction( gmenu_section, "win.selection-group-exit",            _("Exit group"),            ""                                );
                    AppendItemFromAction( gmenu_section, "app.selection-ungroup-pop",           _("_Pop selection out of group"), ""                          );
                }
                gmenu->append_section(gmenu_section);

                // Clipping and Masking
                gmenu_section = Gio::Menu::create();
                if (selection->size() > 1) {
                    AppendItemFromAction( gmenu_section, "app.object-set-clip",                 _("Set Cl_ip"),             ""                                );
                }
                if (item->getClipObject()) {
                    AppendItemFromAction( gmenu_section, "app.object-release-clip",             _("Release C_lip"),         ""                                );
                } else {
                    AppendItemFromAction( gmenu_section, "app.object-set-clip-group",           _("Set Clip G_roup"),       ""                                );
                }
                if (selection->size() > 1) {
                    AppendItemFromAction( gmenu_section, "app.object-set-mask",                 _("Set Mask"),              ""                                );
                }
                if (item->getMaskObject()) {
                    AppendItemFromAction( gmenu_section, "app.object-release-mask",             _("Release Mask"),          ""                                );
                }
                gmenu->append_section(gmenu_section);

                // Hide and Lock
                gmenu_section = Gio::Menu::create();
                AppendItemFromAction(     gmenu_section, "app.selection-hide",                  _("Hide Selected Objects"), ""                                );
                AppendItemFromAction(     gmenu_section, "app.selection-lock",                  _("Lock Selected Objects"), ""                                );
                gmenu->append_section(gmenu_section);

            } else {
                // Anchor menu
                gmenu_section = Gio::Menu::create();
                AppendItemFromAction(     gmenu_section, "win.dialog-open('ObjectAttributes')", _("Link _Properties..."),   ""                                );
                AppendItemFromAction(     gmenu_section, "app.element-a-open-link",             _("_Open link in browser"), ""                                );
                AppendItemFromAction(     gmenu_section, "app.selection-ungroup",               _("_Remove Link"),          ""                                );
                AppendItemFromAction(     gmenu_section, "win.selection-group-enter",           _("Enter Group"),           ""                                );
                gmenu->append_section(gmenu_section);
            }
        }

        // Hidden or locked beneath cursor
        gmenu_section = Gio::Menu::create();
        if (has_hidden_below_cursor) {
            AppendItemFromAction( gmenu_section, "ctx.unhide-objects-below-cursor",     _("Unhide Objects Below Cursor"),  ""                         );
        }
        if (has_locked_below_cursor) {
            AppendItemFromAction( gmenu_section, "ctx.unlock-objects-below-cursor",     _("Unlock Objects Below Cursor"),  ""                         );
        }
        gmenu->append_section(gmenu_section);

    } else {
        // Layers: Only used in "Layers and Objects" dialog.

        gmenu_section = Gio::Menu::create();
        AppendItemFromAction(gmenu_section,     "win.layer-new",                _("_Add Layer..."),              "layer-new");
        AppendItemFromAction(gmenu_section,     "win.layer-duplicate",          _("D_uplicate Layer"),           "layer-duplicate");
        AppendItemFromAction(gmenu_section,     "win.layer-delete",             _("_Delete Layer"),              "layer-delete");
        AppendItemFromAction(gmenu_section,     "win.layer-rename",             _("Re_name Layer..."),           "layer-rename");
        AppendItemFromAction(gmenu_section,     "win.layer-to-group",           _("Layer to _Group"),            "dialog-objects");
        gmenu->append_section(gmenu_section);

        gmenu_section = Gio::Menu::create();
        AppendItemFromAction(gmenu_section,     "win.layer-raise",              _("_Raise Layer"),               "layer-raise");
        AppendItemFromAction(gmenu_section,     "win.layer-lower",              _("_Lower Layer"),               "layer-lower");
        gmenu->append_section(gmenu_section);

        gmenu_section = Gio::Menu::create();
        AppendItemFromAction(gmenu_section,     "win.layer-hide-toggle-others", _("_Hide/show other layers"),    "");
        AppendItemFromAction(gmenu_section,     "win.layer-hide-all",           _("_Hide all layers"),           "");
        AppendItemFromAction(gmenu_section,     "win.layer-unhide-all",         _("_Show all layers"),           "");
        gmenu->append_section(gmenu_section);

        gmenu_section = Gio::Menu::create();
        AppendItemFromAction(gmenu_section,     "win.layer-lock-toggle-others", _("_Lock/unlock other layers"),  "");
        AppendItemFromAction(gmenu_section,     "win.layer-lock-all",           _("_Lock all layers"),           "");
        AppendItemFromAction(gmenu_section,     "win.layer-unlock-all",         _("_Unlock all layers"),         "");
        gmenu->append_section(gmenu_section);

    }
    // clang-tidy on

    bind_model(gmenu, true);

    // Do not install this CSS provider; it messes up menus with icons (like popup menu with all dialogs).
    // It doesn't work well with context menu either, introducing disturbing visual glitch 
    // where menu shifts upon opening.
#if 0
    // Install CSS to shift icons into the space reserved for toggles (i.e. check and radio items).
    signal_map().connect(sigc::bind<Gtk::MenuShell *>(sigc::ptr_fun(shift_icons), this));
#endif

    // Set the style and icon theme of the new menu based on the desktop
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (Gtk::Window *window = desktop->getToplevel()) {
        if (window->get_style_context()->has_class("dark")) {
            get_style_context()->add_class("dark");
        } else {
            get_style_context()->add_class("bright");
        }
        if (prefs->getBool("/theme/symbolicIcons", false)) {
            get_style_context()->add_class("symbolic");
        } else {
            get_style_context()->add_class("regular");
        }
    }
}

void
ContextMenu::AppendItemFromAction(Glib::RefPtr<Gio::Menu> gmenu, Glib::ustring action, Glib::ustring label, Glib::ustring icon)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool show_icons = prefs->getInt("/theme/menuIcons_canvas", true);

    auto menu_item = Gio::MenuItem::create(label, action);
    if (icon != "" && show_icons) {
        auto _icon = Gio::Icon::create(icon);
        menu_item->set_icon(_icon);
    }
    gmenu->append_item(menu_item);
}

void
ContextMenu::unhide_or_unlock(SPDocument* document, bool unhide)
{
    for (auto item : items_under_cursor) {
        if (unhide) {
            if (item->isHidden()) {
                item->setHidden(false);
            }
        } else {
            if (item->isLocked()) {
                item->setLocked(false);
            }
        }
    }

    // We wouldn't be here if we didn't make a change.
    Inkscape::DocumentUndo::done(document, (unhide ? _("Unhid objects") : _("Unlocked objects")), "");
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
