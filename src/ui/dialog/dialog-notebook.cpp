// SPDX-License-Identifier: GPL-2.0-or-later

/** @file
 * @brief A wrapper for Gtk::Notebook.
 *
 * Authors: see git history
 *   Tavmjong Bah
 *
 * Copyright (c) 2018 Tavmjong Bah, Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "dialog-notebook.h"

#include <vector>
#include <glibmm/i18n.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/menu.h>

#include "enums.h"
#include "inkscape.h"
#include "inkscape-window.h"
#include "ui/dialog/dialog-base.h"
#include "ui/dialog/dialog-data.h"
#include "ui/dialog/dialog-container.h"
#include "ui/dialog/dialog-multipaned.h"
#include "ui/dialog/dialog-window.h"
#include "ui/icon-loader.h"
#include "ui/util.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

std::list<DialogNotebook *> DialogNotebook::_instances;

/**
 * DialogNotebook constructor.
 *
 * @param container the parent DialogContainer of the notebook.
 */
DialogNotebook::DialogNotebook(DialogContainer *container)
    : Gtk::ScrolledWindow()
    , _container(container)
    , _labels_auto(true)
    , _detaching_duplicate(false)
    , _selected_page(nullptr)
    , _label_visible(true)
{
    set_name("DialogNotebook");
    set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    set_shadow_type(Gtk::SHADOW_NONE);
    set_vexpand(true);
    set_hexpand(true);

    // =========== Getting preferences ==========
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs == nullptr) {
        return;
    }
    gint labelstautus = prefs->getInt("/options/notebooklabels/value", PREFS_NOTEBOOK_LABELS_AUTO);
    _labels_auto = labelstautus == PREFS_NOTEBOOK_LABELS_AUTO;
    _labels_off = labelstautus == PREFS_NOTEBOOK_LABELS_OFF;

    // ============= Notebook menu ==============
    _notebook.set_name("DockedDialogNotebook");
    _notebook.set_show_border(false);
    _notebook.set_group_name("InkscapeDialogGroup");
    _notebook.set_scrollable(true);

    Gtk::MenuItem *new_menu_item = nullptr;

    int row = 0;
    // Close tab
    new_menu_item = Gtk::manage(new Gtk::MenuItem(_("Close Current Tab")));
    _conn.emplace_back(
        new_menu_item->signal_activate().connect(sigc::mem_fun(*this, &DialogNotebook::close_tab_callback)));
    _menu.attach(*new_menu_item, 0, 2, row, row + 1);
    row++;

    // Close notebook
    new_menu_item = Gtk::manage(new Gtk::MenuItem(_("Close Panel")));
    _conn.emplace_back(
        new_menu_item->signal_activate().connect(sigc::mem_fun(*this, &DialogNotebook::close_notebook_callback)));
    _menu.attach(*new_menu_item, 0, 2, row, row + 1);
    row++;

    // Move to new window
    new_menu_item = Gtk::manage(new Gtk::MenuItem(_("Move Tab to New Window")));
    _conn.emplace_back(
        new_menu_item->signal_activate().connect([=]() { pop_tab_callback(); }));
    _menu.attach(*new_menu_item, 0, 2, row, row + 1);
    row++;

    // Separator menu item
    // new_menu_item = Gtk::manage(new Gtk::SeparatorMenuItem());
    // _menu.attach(*new_menu_item, 0, 2, row, row + 1);
    // row++;

    struct Dialog {
        Glib::ustring key;
        Glib::ustring label;
        Glib::ustring order;
        Glib::ustring icon_name;
        DialogData::Category category;
    };
    std::vector<Dialog> all_dialogs;
    all_dialogs.reserve(dialog_data.size());
    for (auto&& kv : dialog_data) {
        const auto& key = kv.first;
        const auto& data = kv.second;
        if (data.category == DialogData::Other) continue;
        // for sorting dialogs alphabetically, remove '_'
        auto order = data.label;
        auto underscore = order.find('_');
        if (underscore != Glib::ustring::npos) {
            order = order.erase(underscore, 1);
        }
        all_dialogs.push_back(Dialog { .key = key, .label = data.label, .order = order, .icon_name = data.icon_name, .category = data.category });
    }
    // sort by categories and then by names
    std::sort(all_dialogs.begin(), all_dialogs.end(), [](const Dialog& a, const Dialog& b){
        if (a.category != b.category) return a.category < b.category;
        return a.order < b.order;
    });

    int col = 0;
    DialogData::Category category = DialogData::Other;
    for (auto&& data : all_dialogs) {
        if (data.category != category) {
            if (col > 0) row++;

            auto separator = Gtk::make_managed<Gtk::SeparatorMenuItem>();
            _menu.attach(*separator, 0, 2, row, row + 1);
            row++;

            category = data.category;
            auto sep = Gtk::make_managed<Gtk::MenuItem>();
            sep->set_label(dialog_categories[category].uppercase());
            sep->get_style_context()->add_class("menu-category");
            sep->set_sensitive(false);
            _menu.attach(*sep, 0, 2, row, row + 1);
            col = 0;
            row++;
        }
        auto key = data.key;
        auto dlg = Gtk::make_managed<Gtk::MenuItem>();
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 8);
        box->pack_start(*Gtk::make_managed<Gtk::Image>(data.icon_name, Gtk::ICON_SIZE_MENU), false, true);
        box->pack_start(*Gtk::make_managed<Gtk::Label>(data.label, Gtk::ALIGN_START, Gtk::ALIGN_CENTER, true), false, true);
        dlg->add(*box);
        dlg->signal_activate().connect([=](){
            // get desktop's container, it may be different than current '_container'!
            if (auto desktop = SP_ACTIVE_DESKTOP) {
                if (auto container = desktop->getContainer()) {
                    container->new_dialog(key);
                }
            }
        });
        _menu.attach(*dlg, col, col + 1, row, row + 1);
        col++;
        if (col > 1) {
            col = 0;
            row++;
        }
    }
    if (prefs->getBool("/theme/symbolicIcons", true)) {
        _menu.get_style_context()->add_class("symbolic");
    }

    _conn.emplace_back(
        _labels_auto_button.signal_toggled().connect(sigc::mem_fun(*this, &DialogNotebook::on_labels_changed)));
    _conn.emplace_back(
        _labels_active_button.signal_toggled().connect(sigc::mem_fun(*this, &DialogNotebook::on_labels_changed)));

    _menu.show_all_children();

    Gtk::Button* menubtn = Gtk::manage(new Gtk::Button());
    menubtn->set_image_from_icon_name("go-down-symbolic");
    menubtn->signal_clicked().connect([=](){ _menu.popup_at_widget(menubtn, Gdk::GRAVITY_SOUTH, Gdk::GRAVITY_NORTH, nullptr); });
    _notebook.set_action_widget(menubtn, Gtk::PACK_END);
    menubtn->show();
    menubtn->set_relief(Gtk::RELIEF_NORMAL);
    menubtn->set_valign(Gtk::ALIGN_CENTER);
    menubtn->set_halign(Gtk::ALIGN_CENTER);
    menubtn->set_can_focus(false);
    menubtn->set_name("DialogMenuButton");

    // =============== Signals ==================
    _conn.emplace_back(signal_size_allocate().connect(sigc::mem_fun(*this, &DialogNotebook::on_size_allocate_scroll)));
    _conn.emplace_back(_notebook.signal_drag_begin().connect(sigc::mem_fun(*this, &DialogNotebook::on_drag_begin)));
    _conn.emplace_back(_notebook.signal_drag_end().connect(sigc::mem_fun(*this, &DialogNotebook::on_drag_end)));
    _conn.emplace_back(_notebook.signal_page_added().connect(sigc::mem_fun(*this, &DialogNotebook::on_page_added)));
    _conn.emplace_back(_notebook.signal_page_removed().connect(sigc::mem_fun(*this, &DialogNotebook::on_page_removed)));
    _conn.emplace_back(_notebook.signal_switch_page().connect(sigc::mem_fun(*this, &DialogNotebook::on_page_switch)));

    // ============= Finish setup ===============
    _reload_context = true;
    add(_notebook);
    show_all();

    _instances.push_back(this);
}

DialogNotebook::~DialogNotebook()
{
    // disconnect signals first, so no handlers are invoked when removing pages
    for_each(_conn.begin(), _conn.end(), [&](auto c) { c.disconnect(); });
    for_each(_connmenu.begin(), _connmenu.end(), [&](auto c) { c.disconnect(); });
    for_each(_tab_connections.begin(), _tab_connections.end(), [&](auto it) { it.second.disconnect(); });

    // Unlink and remove pages
    for (int i = _notebook.get_n_pages(); i >= 0; --i) {
        DialogBase *dialog = dynamic_cast<DialogBase *>(_notebook.get_nth_page(i));
        _container->unlink_dialog(dialog);
        _notebook.remove_page(i);
    }

    _conn.clear();
    _connmenu.clear();
    _tab_connections.clear();

    _instances.remove(this);
}

void DialogNotebook::add_highlight_header()
{
    const auto &style = _notebook.get_style_context();
    style->add_class("nb-highlight");
}

void DialogNotebook::remove_highlight_header()
{
    const auto &style = _notebook.get_style_context();
    style->remove_class("nb-highlight");
}

/**
 * Adds a widget as a new page with a tab.
 */
void DialogNotebook::add_page(Gtk::Widget &page, Gtk::Widget &tab, Glib::ustring label)
{
    _reload_context = true;
    page.set_vexpand();

    int page_number = _notebook.append_page(page, tab);
    _notebook.set_tab_reorderable(page);
    _notebook.set_tab_detachable(page);
    _notebook.show_all();
    _notebook.set_current_page(page_number);
}

/**
 * Moves a page from a different notebook to this one.
 */
void DialogNotebook::move_page(Gtk::Widget &page)
{
    // Find old notebook
    Gtk::Notebook *old_notebook = dynamic_cast<Gtk::Notebook *>(page.get_parent());
    if (!old_notebook) {
        std::cerr << "DialogNotebook::move_page: page not in notebook!" << std::endl;
        return;
    }

    Gtk::Widget *tab = old_notebook->get_tab_label(page);
    Glib::ustring text = old_notebook->get_menu_label_text(page);

    // Keep references until re-attachment
    tab->reference();
    page.reference();

    old_notebook->detach_tab(page);
    _notebook.append_page(page, *tab);
    // Remove unnecessary references
    tab->unreference();
    page.unreference();

    // Set default settings for a new page
    _notebook.set_tab_reorderable(page);
    _notebook.set_tab_detachable(page);
    _notebook.show_all();
    _reload_context = true;
}

// ============ Notebook callbacks ==============

/**
 * Callback to close the current active tab.
 */
void DialogNotebook::close_tab_callback()
{
    int page_number = _notebook.get_current_page();

    if (_selected_page) {
        page_number = _notebook.page_num(*_selected_page);
        _selected_page = nullptr;
    }

    if (dynamic_cast<DialogBase*>(_notebook.get_nth_page(page_number))) {
        // is this a dialog in a floating window?
        if (auto window = dynamic_cast<DialogWindow*>(_container->get_toplevel())) {
            // store state of floating dialog before it gets deleted
            DialogManager::singleton().store_state(*window);
        }
    }

    // Remove page from notebook
    _notebook.remove_page(page_number);

    // Delete the signal connection
    remove_close_tab_callback(_selected_page);

    if (_notebook.get_n_pages() == 0) {
        close_notebook_callback();
        return;
    }

    // Update tab labels by comparing the sum of their widths to the allocation
    Gtk::Allocation allocation = get_allocation();
    on_size_allocate_scroll(allocation);
    _reload_context = true;
}

/**
 * Shutdown callback - delete the parent DialogMultipaned before destructing.
 */
void DialogNotebook::close_notebook_callback()
{
    // Search for DialogMultipaned
    DialogMultipaned *multipaned = dynamic_cast<DialogMultipaned *>(get_parent());
    if (multipaned) {
        multipaned->remove(*this);
    } else if (get_parent()) {
        std::cerr << "DialogNotebook::close_notebook_callback: Unexpected parent!" << std::endl;
        get_parent()->remove(*this);
    }
    delete this;
}

/**
 * Callback to move the current active tab.
 */
DialogWindow* DialogNotebook::pop_tab_callback()
{
    // Find page.
    Gtk::Widget *page = _notebook.get_nth_page(_notebook.get_current_page());

    if (_selected_page) {
        page = _selected_page;
        _selected_page = nullptr;
    }

    if (!page) {
        std::cerr << "DialogNotebook::pop_tab_callback: page not found!" << std::endl;
        return nullptr;
    }

    // Move page to notebook in new dialog window (attached to same Inkscape window as this dialog window).
    auto old_dialog_window = dynamic_cast<DialogWindow *>(get_toplevel());
    g_assert(old_dialog_window != nullptr);
    auto inkscape_window = old_dialog_window->get_inkscape_window();
    g_assert(inkscape_window != nullptr);
    auto window = new DialogWindow(inkscape_window, page);
    window->show_all();

    if (_notebook.get_n_pages() == 0) {
        close_notebook_callback();
        return window;
    }

    // Update tab labels by comparing the sum of their widths to the allocation
    Gtk::Allocation allocation = get_allocation();
    on_size_allocate_scroll(allocation);

    return window;
}

// ========= Signal handlers - notebook =========

/**
 * Signal handler to pop a dragged tab into its own DialogWindow.
 *
 * A failed drag means that the page was not dropped on an existing notebook.
 * Thus create a new window with notebook to move page to.
 *
 * BUG: this has inconsistent behavior on Wayland.
 */
void DialogNotebook::on_drag_end(const Glib::RefPtr<Gdk::DragContext> context)
{
    // Remove dropzone highlights
    MyDropZone::remove_highlight_instances();
    for (auto instance : _instances) {
        instance->remove_highlight_header();
    }

    bool set_floating = !context->get_dest_window();
    if (!set_floating && context->get_dest_window()->get_window_type() == Gdk::WINDOW_FOREIGN) {
        set_floating = true;
    }

    if (set_floating) {
        Gtk::Widget *source = Gtk::Widget::drag_get_source_widget(context);

        // Find source notebook and page
        Gtk::Notebook *old_notebook = dynamic_cast<Gtk::Notebook *>(source);
        if (!old_notebook) {
            std::cerr << "DialogNotebook::on_drag_end: notebook not found!" << std::endl;
        } else {
            // Find page
            Gtk::Widget *page = old_notebook->get_nth_page(old_notebook->get_current_page());
            if (page) {
                // Move page to notebook in new dialog window

                // Find InkscapeWindow.
                auto inkscape_window = dynamic_cast<InkscapeWindow *>(get_toplevel());
                if (!inkscape_window) {
                    // Not part of an InkscapeWindow, must be in a DialogWindow.
                    auto dialog_window = dynamic_cast<DialogWindow *>(get_toplevel());
                    g_assert (dialog_window != nullptr);
                    inkscape_window = dialog_window->get_inkscape_window();
                }
                g_assert(inkscape_window != nullptr);

                auto window = new DialogWindow(inkscape_window, page);

                // Move window to mouse pointer
                if (auto device = context->get_device()) {
                    int x = 0, y = 0;
                    device->get_position(x, y);
                    window->move(std::max(0, x - 50), std::max(0, y - 50));
                }

                window->show_all();
            }
        }
    }

    // Closes the notebook if empty.
    if (_notebook.get_n_pages() == 0) {
        close_notebook_callback();
        return;
    }

    // Update tab labels by comparing the sum of their widths to the allocation
    Gtk::Allocation allocation = get_allocation();
    on_size_allocate_scroll(allocation);
}

void DialogNotebook::on_drag_begin(const Glib::RefPtr<Gdk::DragContext> context)
{
    MyDropZone::add_highlight_instances();
    for (auto instance : _instances) {
        instance->add_highlight_header();
    }
}

/**
 * Signal handler to update dialog list when adding a page.
 */
void DialogNotebook::on_page_added(Gtk::Widget *page, int page_num)
{
    DialogBase *dialog = dynamic_cast<DialogBase *>(page);

    // Does current container/window already have such a dialog?
    if (dialog && _container->has_dialog_of_type(dialog)) {
        // We already have a dialog of the same type

        // Highlight first dialog
        DialogBase *other_dialog = _container->get_dialog(dialog->get_type());
        other_dialog->blink();

        // Remove page from notebook
        _detaching_duplicate = true; // HACK: prevent removing the initial dialog of the same type
        _notebook.detach_tab(*page);
        return;
    } else if (dialog) {
        // We don't have a dialog of this type

        // Add to dialog list
        _container->link_dialog(dialog);
    } else {
        // This is not a dialog
        return;
    }

    // add close tab signal
    add_close_tab_callback(page);

    // Switch tab labels if needed
    if (!_labels_auto) {
        toggle_tab_labels_callback(false);
    }

    // Update tab labels by comparing the sum of their widths to the allocation
    Gtk::Allocation allocation = get_allocation();
    on_size_allocate_scroll(allocation);
}

/**
 * Signal handler to update dialog list when removing a page.
 */
void DialogNotebook::on_page_removed(Gtk::Widget *page, int page_num)
{
    /**
     * When adding a dialog in a notebooks header zone of the same type as an existing one,
     * we remove it immediately, which triggers a call to this method. We use `_detaching_duplicate`
     * to prevent reemoving the initial dialog.
     */
    if (_detaching_duplicate) {
        _detaching_duplicate = false;
        return;
    }

    // Remove from dialog list
    DialogBase *dialog = dynamic_cast<DialogBase *>(page);
    if (dialog) {
        _container->unlink_dialog(dialog);
    }

    // remove old close tab signal
    remove_close_tab_callback(page);
}

/**
 * We need to remove the scrollbar to snap a whole DialogNotebook to width 0.
 *
 */
void DialogNotebook::on_size_allocate_scroll(Gtk::Allocation &a)
{
    // magic number
    const int MIN_HEIGHT = 60;

    // set or unset scrollbars to completely hide a notebook
    property_vscrollbar_policy().set_value(a.get_height() >= MIN_HEIGHT ? Gtk::POLICY_AUTOMATIC : Gtk::POLICY_EXTERNAL);

    set_allocation(a);
    // only update notebook tabs on horizontal changes
    if (a.get_width() != _prev_alloc_width) {
        on_size_allocate_notebook(a);
    }
}

/**
 * This function hides the tab labels if necessary (and _labels_auto == true)
 */
void DialogNotebook::on_size_allocate_notebook(Gtk::Allocation &a)
{
    
    // we unsetset scrollable happen when FULL mode on to prevent overflow with 
    // container at full size that make a unmaximice desctop freeze 
    _notebook.set_scrollable(false);
    if (!_labels_set_off && !_labels_auto) {
        toggle_tab_labels_callback(false);
    }
    if (!_labels_auto) {
        return;
    }

    int alloc_width = get_allocation().get_width();
    // Dont update on closed dialog container, prevent console errors
    if (alloc_width < 2) {
        _notebook.set_scrollable(true);
        return;
    }
    int nat_width = 0;
    int initial_width = 0;
    int total_width = 0;
    _notebook.get_preferred_width(initial_width, nat_width); // get current notebook allocation
    for (auto const &page : _notebook.get_children()) {
        Gtk::EventBox *cover = dynamic_cast<Gtk::EventBox *>(_notebook.get_tab_label(*page));
        if (!cover) {
            continue;
        }
        cover->show_all();
    }
    _notebook.get_preferred_width(total_width, nat_width); // get full notebook allocation (all open)
    prev_tabstatus = tabstatus;
    if (_single_tab_width != _none_tab_width && 
        (_none_tab_width && _none_tab_width > alloc_width || 
        (_single_tab_width > alloc_width && _single_tab_width < total_width)))
    {
        tabstatus = TabsStatus::NONE;
        if (_single_tab_width != initial_width || prev_tabstatus == TabsStatus::NONE) {
            _none_tab_width = initial_width;
        }
    } else {
        tabstatus = (alloc_width <= total_width) ? TabsStatus::SINGLE : TabsStatus::ALL;
        if (total_width != initial_width &&
            prev_tabstatus == TabsStatus::SINGLE && 
            tabstatus == TabsStatus::SINGLE) 
        {
            _single_tab_width = initial_width;
        }
    }
    if ((_single_tab_width && !_none_tab_width) || 
        (_single_tab_width && _single_tab_width == _none_tab_width)) 
    {
        _none_tab_width = _single_tab_width - 1;
    }    
     
    /* 
    std::cout << "::::::::::tabstatus::" << (int)tabstatus  << std::endl;
    std::cout << ":::::prev_tabstatus::" << (int)prev_tabstatus << std::endl;
    std::cout << "::::::::alloc_width::" << alloc_width << std::endl;
    std::cout << "::_prev_alloc_width::" << _prev_alloc_width << std::endl;
    std::cout << "::::::initial_width::" << initial_width << std::endl;
    std::cout << "::::::::::nat_width::" << nat_width << std::endl;
    std::cout << "::::::::total_width::" << total_width << std::endl;
    std::cout << "::::_none_tab_width::" << _none_tab_width  << std::endl;
    std::cout << "::_single_tab_width::" << _single_tab_width  << std::endl;
    std::cout << ":::::::::::::::::::::" << std::endl;    
    */
    
    _prev_alloc_width = alloc_width;
    bool show = tabstatus == TabsStatus::ALL;
    toggle_tab_labels_callback(show);
}

/**
 * Signal handler to toggle the tab labels internal state.
 */
void DialogNotebook::on_labels_changed() {
    _labels_auto = _labels_auto_button.get_active();
    _labels_off = _labels_off_button.get_active();
    _labels_set_off = false;
    if (_labels_active_button.get_active()) {
        tabstatus = TabsStatus::SINGLE;
    }
    toggle_tab_labels_callback(_labels_auto);
}

/**
 * Signal handler to close a tab when middle-clicking.
 */
bool DialogNotebook::on_tab_click_event(GdkEventButton *event, Gtk::Widget *page)
{
    if (event->type == GDK_BUTTON_PRESS) {
        if (event->button == 2) { // Close tab
            _selected_page = page;
            close_tab_callback();
        } else if (event->button == 3) { // Show menu
            _selected_page = page;
            reload_tab_menu();
            _menutabs.popup_at_pointer((GdkEvent *)event);
        }
    }

    return false;
}

void DialogNotebook::on_close_button_click_event(Gtk::Widget *page)
{
    _selected_page = page;
    close_tab_callback();
}

// ================== Helpers ===================

/**
 * Reload tab menu
 */
void DialogNotebook::reload_tab_menu()
{
    if (_reload_context) {
        _reload_context = false;
        Gtk::MenuItem* menuitem = nullptr;
        for_each(_connmenu.begin(), _connmenu.end(), [&](auto c) { c.disconnect(); });
        _connmenu.clear();
        for (auto widget : _menutabs.get_children()) {
            delete widget;
        }
        auto prefs = Inkscape::Preferences::get();
        bool symbolic = false;
        if (prefs->getBool("/theme/symbolicIcons", false)) {
            symbolic = true;
        }

        for (auto const &page : _notebook.get_children()) {
            Gtk::EventBox *cover = dynamic_cast<Gtk::EventBox *>(_notebook.get_tab_label(*page));
            if (!cover) {
                continue;
            }

            Gtk::Box *box = dynamic_cast<Gtk::Box *>(cover->get_child());
            
            if (!box) {
                continue;
            }
            auto childs = box->get_children();
            if (childs.size() < 2) {
                continue;
            }
            // Create a box to hold icon and label as Gtk::MenuItem derives from GtkBin and can
            // only hold one child
            Gtk::Box *boxmenu = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
            boxmenu->set_halign(Gtk::ALIGN_START);
            menuitem = Gtk::manage(new Gtk::MenuItem());
            menuitem->add(*boxmenu);

            Gtk::Label *label = dynamic_cast<Gtk::Label *>(childs[1]);
            Gtk::Label *labelto = Gtk::manage(new Gtk::Label(label->get_text()));
            
            Gtk::Image *icon = dynamic_cast<Gtk::Image *>(childs[0]);
            if (icon) {
                int min_width, nat_width;
                icon->get_preferred_width(min_width, nat_width);
                _icon_width = min_width;
                auto name = icon->get_icon_name();
                if (!name.empty()) {
                    if (symbolic && name.find("-symbolic") == Glib::ustring::npos) {
                        name += Glib::ustring("-symbolic");
                    }
                    Gtk::Image *iconend  = sp_get_icon_image(name, Gtk::ICON_SIZE_MENU);
                    boxmenu->pack_start(*iconend, false, false, 0);
                }
            }
            boxmenu->pack_start(*labelto, true,  true,  0);
            size_t pagenum = _notebook.page_num(*page);
            _connmenu.emplace_back(
                menuitem->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &DialogNotebook::change_page),pagenum)));
            
            _menutabs.append(*Gtk::manage(menuitem));
        }
    }
    _menutabs.show_all();
}
/**
 * Callback to toggle all tab labels to the selected state.
 * @param show: whether you want the labels to show or not
 */
void DialogNotebook::toggle_tab_labels_callback(bool show)
{
    _label_visible = show;
    for (auto const &page : _notebook.get_children()) {
        Gtk::EventBox *cover = dynamic_cast<Gtk::EventBox *>(_notebook.get_tab_label(*page));
        if (!cover) {
            continue;
        }

        Gtk::Box *box = dynamic_cast<Gtk::Box *>(cover->get_child());
        if (!box) {
            continue;
        }

        Gtk::Label *label = dynamic_cast<Gtk::Label *>(box->get_children()[1]);
        Gtk::Button *close = dynamic_cast<Gtk::Button *>(*box->get_children().rbegin());
        int n = _notebook.get_current_page();
        if (close && label) {
            if (page != _notebook.get_nth_page(n)) {
                show ? close->show() : close->hide();
                show ? label->show() : label->hide();
            } else if (tabstatus == TabsStatus::NONE || _labels_off) {
                close->hide();
                label->hide();
            } else {
                close->show();
                label->show();
            }
        }
    }
    _labels_set_off = _labels_off;
    if (_prev_alloc_width && prev_tabstatus != tabstatus ) {
        resize_widget_children(&_notebook);
    }
    if (show && _single_tab_width) {
        _notebook.set_scrollable(true);
    }
}

void DialogNotebook::on_page_switch(Gtk::Widget *curr_page, guint page_number)
{
    for (auto const &page : _notebook.get_children()) {
        if (_prev_alloc_width) {
            auto dialogbase = dynamic_cast<DialogBase*>(page);
            if (dialogbase) {
                dialogbase->setShowing(curr_page == page);
            }
        }
        if (_label_visible) {
            continue;
        }
        Gtk::EventBox *cover = dynamic_cast<Gtk::EventBox *>(_notebook.get_tab_label(*page));
        if (!cover) {
            continue;
        }

        if (cover == dynamic_cast<Gtk::EventBox *>(_notebook.get_tab_label(*curr_page))) {
            Gtk::Box *box = dynamic_cast<Gtk::Box *>(cover->get_child());
            Gtk::Label *label = dynamic_cast<Gtk::Label *>(box->get_children()[1]);
            Gtk::Button *close = dynamic_cast<Gtk::Button *>(*box->get_children().rbegin());

            if (label) {
                if (tabstatus == TabsStatus::NONE) {
                    label->hide();
                } else {
                    label->show();
                }
            }

            if (close) {
                if (tabstatus == TabsStatus::NONE) {
                    close->hide();
                } else {
                    close->show();
                }
            }
            continue;
        }

        Gtk::Box *box = dynamic_cast<Gtk::Box *>(cover->get_child());
        if (!box) {
            continue;
        }

        Gtk::Label *label = dynamic_cast<Gtk::Label *>(box->get_children()[1]);
        Gtk::Button *close = dynamic_cast<Gtk::Button *>(*box->get_children().rbegin());


        close->hide();
        label->hide();
    }
    if (_prev_alloc_width && !_label_visible) {
        queue_allocate(); 
    }
}

/**
 * Helper method that change the page
 */
void DialogNotebook::change_page(size_t pagenum)
{
    _notebook.set_current_page(pagenum);
}

/**
 * Helper method that adds the close tab signal connection for the page given.
 */
void DialogNotebook::add_close_tab_callback(Gtk::Widget *page)
{
    Gtk::Widget *tab = _notebook.get_tab_label(*page);
    auto *eventbox = static_cast<Gtk::EventBox *>(tab);
    auto *box = static_cast<Gtk::Box *>(*eventbox->get_children().begin());
    auto children = box->get_children(); 
    auto *close = static_cast<Gtk::Button *>(*children.crbegin());

    sigc::connection close_connection = close->signal_clicked().connect(
            sigc::bind<Gtk::Widget *>(sigc::mem_fun(*this, &DialogNotebook::on_close_button_click_event), page), true);

    sigc::connection tab_connection = tab->signal_button_press_event().connect(
        sigc::bind<Gtk::Widget *>(sigc::mem_fun(*this, &DialogNotebook::on_tab_click_event), page), true);

    _tab_connections.insert(std::pair<Gtk::Widget *, sigc::connection>(page, tab_connection));
    _tab_connections.insert(std::pair<Gtk::Widget *, sigc::connection>(page, close_connection));
}

/**
 * Helper method that removes the close tab signal connection for the page given.
 */
void DialogNotebook::remove_close_tab_callback(Gtk::Widget *page)
{
    auto tab_connection_it = _tab_connections.find(page);

    while (tab_connection_it != _tab_connections.end()) {
        (*tab_connection_it).second.disconnect();
        _tab_connections.erase(tab_connection_it);
        tab_connection_it = _tab_connections.find(page);
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
