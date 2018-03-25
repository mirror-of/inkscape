#ifndef SEEN_SPIN_BUTTON_TOOL_ITEM_H
#define SEEN_SPIN_BUTTON_TOOL_ITEM_H

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/toolitem.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class SpinButton;

/**
 * \brief A spin-button with a label that can be added to a toolbar
 */
class SpinButtonToolItem : public Gtk::ToolItem
{
private:
    SpinButton *_btn; ///< The spin-button within the widget
public:
    SpinButtonToolItem(const Glib::ustring&                 label_text,
                       const Glib::RefPtr<Gtk::Adjustment>& adjustment,
                       double                               climb_rate = 0.0,
                       double                               digits     = 0);

    void set_all_tooltip_text(const Glib::ustring& text);
};
} // namespace Widget
} // namespace UI
} // namespace Inkscape
#endif // SEEN_SPIN_BUTTON_TOOL_ITEM_H
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
