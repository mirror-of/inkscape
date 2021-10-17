// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Effect Data to store data related to creating of
 * Filters and Effect manubar
 *
 * Authors:
 *   Sushant A A <sushant.co19@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */


#include "actions-effect-data.h"

#include <iostream>
#include <algorithm>
#include <tuple>

#include <glibmm/i18n.h>

std::vector<InkActionEffectData::datum>
InkActionEffectData::give_all_data()
{
    // Sort by menu tree and effect name.
    std::sort(data.begin(), data.end(), [](datum a, datum b) {
        auto a_list = std::get<1>(a);
        auto b_list = std::get<1>(b);
        auto a_it = a_list.begin();
        auto b_it = b_list.begin();
        while (a_it != a_list.end() && b_it != b_list.end()) {
            if (*a_it < *b_it) return true;
            if (*a_it > *b_it) return false;
            a_it++;
            b_it++;
        }
       if (a_it != a_list.end()) return *a_it < std::get<2>(b);  // Compare menu name with effect name.
       if (b_it != b_list.end()) return *b_it > std::get<2>(a);  // Compare menu name with effect name.
       return std::get<2>(a) < std::get<2>(b); // Same menu, order by effect name.
    });

    return data;
}

void
InkActionEffectData::add_data ( std::string effect_id, std::list<Glib::ustring> effect_submenu_name,
                                Glib::ustring const &effect_name )
{
    data.emplace_back(effect_id, effect_submenu_name, effect_name);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
