#include "actions-hint-data.h"
#include <glibmm/i18n.h>

std::vector<Glib::ustring>
InkActionHintData::get_actions()
{
    std::vector<Glib::ustring> action_names;
    for (auto hint : data) {
        action_names.emplace_back(hint.first);
    }
    return action_names;
}

Glib::ustring
InkActionHintData::get_tooltip_hint_for_action(Glib::ustring const &action_name, bool translated) {

    Glib::ustring value;
    auto search = data.find(action_name);
    if (search != data.end()) {
        value = translated ? _(search->second.c_str())  :   search->second;
    }
    return value;
}

void
InkActionHintData::add_data(std::vector<std::vector<Glib::ustring>> &hint_data) {
    for (auto hint : hint_data) {
        data.emplace(hint[0], hint[1]);     // action name , hint  
    }
}