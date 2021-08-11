// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Abhishek Sharma
 *
 * Copyright (C) 2002-2007 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "effect.h"

#include "execution-env.h"
#include "inkscape.h"
#include "timer.h"

#include "helper/action.h"
#include "implementation/implementation.h"
#include "prefdialog/prefdialog.h"
#include "ui/view/view.h"
#include "inkscape-application.h"


/* Inkscape::Extension::Effect */

namespace Inkscape {
namespace Extension {

Effect * Effect::_last_effect = nullptr;

// Adds effect to Gio::Actions
void 
action_effect (Effect* effect)
{
    if (effect->_workingDialog) {
        effect->prefs(InkscapeApplication::instance()->get_active_view());
    } else { 
        effect->effect(InkscapeApplication::instance()->get_active_view());   
    }
}

// Modifying string to get submenu id
std::string
action_menu_name (std::string menu)
{
    transform(menu.begin(), menu.end(), menu.begin(), ::tolower);
    for (auto &x:menu) {
        if (x==' ') {
            x = '-';
        }
    }
    return menu;
}

Effect::Effect (Inkscape::XML::Node *in_repr, Implementation::Implementation *in_imp, std::string *base_directory)
    : Extension(in_repr, in_imp, base_directory)
    , _menu_node(nullptr), _workingDialog(true)
    , _prefDialog(nullptr) 
{    
    Inkscape::XML::Node * local_effects_menu = nullptr;

    // cant use documnent level because it is not defined 
    static auto app = InkscapeApplication::instance();
    
    if (!app) {
        std::cerr << "effect: no app!" << std::endl;
        return;
    }

    if(!INKSCAPE.use_gui())
        return;

// #ifdef _WIN32

    // This is a weird hack
    if (!strcmp(this->get_id(), "org.inkscape.filter.dropshadow"))
        return;

    bool hidden = false;

    no_doc = false;
    no_live_preview = false;

    // Setting initial value of discription to name of action incase if there is no discription
    Glib::ustring discription  = get_name();

    if (repr != nullptr) {

        for (Inkscape::XML::Node *child = repr->firstChild(); child != nullptr; child = child->next()) {
            if (!strcmp(child->name(), INKSCAPE_EXTENSION_NS "effect")) {
                if (child->attribute("needs-document") && !strcmp(child->attribute("needs-document"), "false")) {
                    no_doc = true;
                }
                if (child->attribute("needs-live-preview") && !strcmp(child->attribute("needs-live-preview"), "false")) {
                    no_live_preview = true;
                }
                if (child->attribute("implements-custom-gui") && !strcmp(child->attribute("implements-custom-gui"), "true")) {
                    _workingDialog = false;
                }
                for (Inkscape::XML::Node *effect_child = child->firstChild(); effect_child != nullptr; effect_child = effect_child->next()) {
                    if (!strcmp(effect_child->name(), INKSCAPE_EXTENSION_NS "effects-menu")) {
                        // printf("Found local effects menu in %s\n", this->get_name());
                        local_effects_menu = effect_child->firstChild();
                        if (effect_child->attribute("hidden") && !strcmp(effect_child->attribute("hidden"), "true")) {
                            hidden = true;
                        }
                    }
                    if (!strcmp(effect_child->name(), INKSCAPE_EXTENSION_NS "menu-tip") ||
                            !strcmp(effect_child->name(), INKSCAPE_EXTENSION_NS "_menu-tip")) {
                        // printf("Found local effects menu in %s\n", this->get_name());
                        discription = effect_child->firstChild()->content();
                    }
                } // children of "effect"
                break; // there can only be one effect
            } // find "effect"
        } // children of "inkscape-extension"
    } // if we have an XML file

    std::string action_id = "app."+std::string(get_id());

    static auto gapp = InkscapeApplication::instance()->gtk_app();
    gapp->add_action( this->get_id(),sigc::bind<Effect*>(sigc::ptr_fun(&action_effect), this));
    
    if (!hidden) {
        
        // Submenu retrival as a string
        std::string sub_menu;
        get_menu(local_effects_menu,sub_menu);
        sub_menu = action_menu_name(sub_menu);
        
        if (local_effects_menu->attribute("name") && !strcmp(local_effects_menu->attribute("name"), ("Filters"))) {
        
            std::vector<std::vector<Glib::ustring>>raw_data_filter = {{ action_id, get_name(),"Filter",discription}};
            app->get_action_extra_data().add_data(raw_data_filter);
            sub_menu = sub_menu.substr(1);
        
        } else {
        
            std::vector<std::vector<Glib::ustring>>raw_data_effect = {{ action_id, get_name(),"Effect",discription}};
            app->get_action_extra_data().add_data(raw_data_effect);            
            sub_menu="effect"+sub_menu;
        }

        // Add submenu to effect data
        app->get_action_effect_data().add_data(get_id(), sub_menu, get_name() );
    }

INKSCAPE.use_gui()

// #endif // ifdef _WIN32
}

void
Effect::get_menu (Inkscape::XML::Node * pattern,std::string& sub_menu) 
{
    Glib::ustring mergename;

    if (pattern == nullptr) {
        mergename = get_name();
    } else {
        gchar const *menuname = pattern->attribute("name");
        if (menuname == nullptr) menuname = pattern->attribute("_name");
        if (menuname == nullptr) return;

        if (_translation_enabled) {
            mergename = get_translation(menuname);
        } else {
            mergename = _(menuname);
        }
        
        // Makeing sub menu string
        sub_menu += "-";
        sub_menu += menuname;
    }

    if (pattern != nullptr) {
        get_menu( pattern->firstChild(),sub_menu);
    }
}

Effect::~Effect ()
{
    if (get_last_effect() == this)
        set_last_effect(nullptr);
    if (_menu_node) {
        if (_menu_node->parent()) {
            _menu_node->parent()->removeChild(_menu_node);
        }
        Inkscape::GC::release(_menu_node);
    }
    return;
}

bool
Effect::prefs (Inkscape::UI::View::View * doc)
{
    if (_prefDialog != nullptr) {
        _prefDialog->raise();
        return true;
    }

    if (widget_visible_count() == 0) {
        effect(doc);
        return true;
    }

    if (!loaded())
        set_state(Extension::STATE_LOADED);
    if (!loaded()) return false;

    Glib::ustring name = this->get_name();
    _prefDialog = new PrefDialog(name, nullptr, this);
    _prefDialog->show();

    return true;
}

/**
    \brief  The function that 'does' the effect itself
    \param  doc  The Inkscape::UI::View::View to do the effect on

    This function first insures that the extension is loaded, and if not,
    loads it.  It then calls the implementation to do the actual work.  It
    also resets the last effect pointer to be this effect.  Finally, it
    executes a \c SPDocumentUndo::done to commit the changes to the undo
    stack.
*/
void
Effect::effect (Inkscape::UI::View::View * doc)
{
    //printf("Execute effect\n");
    if (!loaded())
        set_state(Extension::STATE_LOADED);
    if (!loaded()) return;
    ExecutionEnv executionEnv(this, doc, nullptr, _workingDialog, true);
    execution_env = &executionEnv;
    timer->lock();
    executionEnv.run();
    if (executionEnv.wait()) {
        executionEnv.commit();
    } else {
        executionEnv.cancel();
    }
    timer->unlock();

    return;
}

/** \brief  Sets which effect was called last
    \param in_effect  The effect that has been called

    This function sets the static variable \c _last_effect 

    If the \c in_effect variable is \c NULL then the last effect
    verb is made insensitive.
*/
void
Effect::set_last_effect (Effect * in_effect)
{
    _last_effect = in_effect;
    return;
}

Inkscape::XML::Node *
Effect::find_menu (Inkscape::XML::Node * menustruct, const gchar *name)
{
    if (menustruct == nullptr) return nullptr;
    for (Inkscape::XML::Node * child = menustruct;
            child != nullptr;
            child = child->next()) {
        if (!strcmp(child->name(), name)) {
            return child;
        }
        Inkscape::XML::Node * firstchild = child->firstChild();
        if (firstchild != nullptr) {
            Inkscape::XML::Node *found = find_menu (firstchild, name);
            if (found) {
                return found;
            }
        }
    }
    return nullptr;
}


Gtk::Box *
Effect::get_info_widget()
{
    return Extension::get_info_widget();
}

PrefDialog *
Effect::get_pref_dialog ()
{
    return _prefDialog;
}

void
Effect::set_pref_dialog (PrefDialog * prefdialog)
{
    _prefDialog = prefdialog;
    return;
}

} }  /* namespace Inkscape, Extension */

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
