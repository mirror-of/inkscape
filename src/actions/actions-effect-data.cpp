// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 *
 * Effect Data to store dacreationta related to creating of
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

#include <glibmm/i18n.h>

std::map<std::string, std::pair<std::string,Glib::ustring>>
InkActionEffectData::give_all_data()
{
    return data;
}

void
InkActionEffectData::add_data ( std::string effect_id, std::string effect_submenu_name, 
                                Glib::ustring const &effect_name ) 
{
    data.insert({effect_id,{effect_submenu_name,effect_name}});
}