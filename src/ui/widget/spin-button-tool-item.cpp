#include "spin-button-tool-item.h"

#include <gtkmm/box.h>

#include "spinbutton.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * \brief Create a new SpinButtonToolItem
 *
 * \param[in] label_text The text to display in the toolbar
 * \param[in] adjustment The Gtk::Adjustment to attach to the spinbutton
 * \param[in] climb_rate The climb rate for the spin button (default = 0)
 * \param[in] digits     Number of decimal places to display
 */
SpinButtonToolItem::SpinButtonToolItem(const Glib::ustring&                 label_text,
                                       const Glib::RefPtr<Gtk::Adjustment>& adjustment,
                                       double                               climb_rate,
                                       double                               digits)
    : _btn(Gtk::manage(new SpinButton(adjustment, climb_rate, digits)))
{
    // Arrange the widgets in a horizontal box
    auto hbox = Gtk::manage(new Gtk::Box());
    hbox->set_spacing(3);

    auto label = Gtk::manage(new Gtk::Label(label_text));
    hbox->pack_start(*label);
    hbox->pack_start(*_btn);
    add(*hbox);
    show_all();
}

/**
 * \brief Set the tooltip to display on this (and all child widgets)
 *
 * \param[in] text The tooltip to display
 */
void
SpinButtonToolItem::set_all_tooltip_text(const Glib::ustring& text)
{
    set_tooltip_text(text);
    _btn->set_tooltip_text(text);
}
} // namespace Widget
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
