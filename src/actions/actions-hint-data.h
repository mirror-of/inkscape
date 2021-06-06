#ifndef INK_ACTIONS_HINT_DATA_H
#define INK_ACTIONS_HINT_DATA_H

#include <glibmm/ustring.h>
#include <glibmm/varianttype.h>
#include <map>
#include <utility>
#include <vector>
#include <iostream>

class InkActionHintData
{
public:
    InkActionHintData() = default ;

    std::vector<Glib::ustring> get_actions();

    void add_data(std::vector<std::vector<Glib::ustring>> &raw_data);

    Glib::ustring get_tooltip_hint_for_action(Glib::ustring const &action_name, bool translated = true);

private:
    std::map<Glib::ustring, Glib::ustring> data;
};

#endif // INK_ACTIONS_HINT_DATA_H