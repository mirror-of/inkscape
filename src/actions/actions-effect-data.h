// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Authors:
 *   Sushant A A <sushant.co19@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef INK_ACTIONS_EFFECT_DATA_H
#define INK_ACTIONS_EFFECT_DATA_H

#include <glibmm/ustring.h>
#include <glibmm/varianttype.h>
#include <map>
#include <utility>
#include <vector>

class InkActionEffectData
{
public:
    InkActionEffectData()  = default;

    // Get Map
    std::map<std::string, std::pair<std::string,Glib::ustring>> give_all_data();

    // For detecting filters
    bool is_filter(std::string  submenu_name);

    // For detecting Extensions
    bool is_extensions(std::string submenu_name) ;

    // Add Data
    void add_data(std::string effect_id, std::string effect_submenu_name, Glib::ustring const &effect_name) ;

private:
    std::map<std::string, std::pair<std::string,Glib::ustring>> data;
};

#endif // INK_ACTIONS_EFFECT_DATA_H