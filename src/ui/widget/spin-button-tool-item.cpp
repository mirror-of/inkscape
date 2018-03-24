#include "spin-button-tool-item.h"

#include <gtkmm/spinbutton.h>

namespace Inkscape {
namespace UI {
namespace Widget {
SpinButtonToolItem::SpinButtonToolItem(Glib::ustring&                       label,
                                       const Glib::RefPtr<Gtk::Adjustment>& adjustment,
                                       double                               climb_rate,
                                       double                               digits)
    : _btn(Gtk::manage(new Gtk::SpinButton(adjustment, climb_rate, digits)))
{
   auto hbox = Gtk::manage(new Gtk::Box());
   hbox->set_spacing(3);
   hbox->pack_start(*_btn);
   add(*hbox);
   show_all();
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
