// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */


#ifndef INKSCAPE_EXTENSION_EFFECT_H__
#define INKSCAPE_EXTENSION_EFFECT_H__

#include <glibmm/i18n.h>
#include "extension.h"
#include "inkscape-application.h"

namespace Gtk {
	class Box;
}

class SPDocument;

namespace Inkscape {


namespace Extension {
class PrefDialog;

/** \brief  Effects are extensions that take a document and do something
            to it in place.  This class adds the extra functions required
            to make extensions effects.
*/
class Effect : public Extension {
    /** \brief  This is the last effect that was used.  This is used in
                a menu item to rapidly recall the same effect. */
    static Effect * _last_effect;
    Inkscape::XML::Node *find_menu (Inkscape::XML::Node * menustruct, const gchar *name);
    void get_menu (Inkscape::XML::Node * pattern,std::string& sub_menu);

    /** \brief  Menu node created for this effect */
    Inkscape::XML::Node * _menu_node;

    /** \brief  The preference dialog if it is shown */
    PrefDialog * _prefDialog;
public:
    Effect(Inkscape::XML::Node *in_repr, Implementation::Implementation *in_imp, std::string *base_directory);
    ~Effect  () override;

    bool         prefs   (Inkscape::UI::View::View * doc);
    void         effect  (Inkscape::UI::View::View * doc);

    /** \brief  Whether a working dialog should be shown */
    bool _workingDialog;

    /** \brief  Static function to get the last effect used */
    static Effect *  get_last_effect () { return _last_effect; };
    static void      set_last_effect (Effect * in_effect);

    static void      place_menus ();
    void             place_menu (Inkscape::XML::Node * menus);

    Gtk::Box *    get_info_widget();

    bool no_doc; // if true, the effect does not process SVG document at all, so no need to save, read, and watch for errors
    bool no_live_preview; // if true, the effect does not need "live preview" checkbox in its dialog

    PrefDialog *get_pref_dialog ();
    void        set_pref_dialog (PrefDialog * prefdialog);
private:
    static gchar *   remove_ (gchar * instr);
};

} }  /* namespace Inkscape, Extension */
#endif /* INKSCAPE_EXTENSION_EFFECT_H__ */

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
