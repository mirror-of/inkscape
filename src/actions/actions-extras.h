// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Extra data associated with actions.
 *
 * Copyright (C) 2019 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#ifndef INK_ACTIONS_EXTRAS_H
#define INK_ACTIONS_EXTRAS_H

#include <map>

#include <glibmm/ustring.h>

class InkActionData {
 public:
  InkActionData(Glib::ustring& label, Glib::ustring& section, Glib::ustring& tooltip)
    : action_label(label)
    , action_section(section)
    , action_tooltip(tooltip)
    {
    }
  
  Glib::ustring action_label;
  Glib::ustring action_section;
  Glib::ustring action_tooltip;
};

class InkActionExtras {

 public:

  static Glib::ustring get_label_for_action(Glib::ustring& action_name);
  static Glib::ustring get_section_for_action(Glib::ustring& action_name);
  static Glib::ustring get_tooltip_for_action(Glib::ustring& action_name);

 private:

  InkActionExtras() = delete;
  static void read_data();

 public:
  static std::map<Glib::ustring, InkActionData> data;
};

#endif // INK_ACTIONS_EXTRAS_H

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
