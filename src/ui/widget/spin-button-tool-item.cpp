#include "spin-button-tool-item.h"

#include <gtkmm/box.h>

#include "spinbutton.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * \brief Handler for the button's "focus-in-event" signal
 *
 * \param focus_event The event that triggered the signal
 *
 * \detail This just logs the current value of the spin-button
 *         and sets the _transfer_focus flag
 */
bool
SpinButtonToolItem::on_btn_focus_in_event(GdkEventFocus * /* focus_event */)
{
    _last_val = _btn->get_value();
    _transfer_focus = true;

    return false; // Event not consumed
}

/**
 * \brief Handler for the button's "focus-out-event" signal
 *
 * \param focus_event The event that triggered the signal
 *
 * \detail This just unsets the _transfer_focus flag
 */
bool
SpinButtonToolItem::on_btn_focus_out_event(GdkEventFocus * /* focus_event */)
{
    _transfer_focus = false;

    return false; // Event not consumed
}

/**
 * \brief Handler for the button's "key-press-event" signal
 *
 * \param key_event The event that triggered the signal
 *
 * \detail If the ESC key was pressed, restore the last value and defocus.
 *         If the Enter key was pressed, just defocus.
 */
bool
SpinButtonToolItem::on_btn_key_press_event(GdkEventKey *key_event)
{
    bool was_consumed = false; // Whether event has been consumed or not
    auto display = Gdk::Display::get_default();
    auto keymap  = display->get_keymap();
    guint key = 0;
    gdk_keymap_translate_keyboard_state(keymap, key_event->hardware_keycode,
                                        static_cast<GdkModifierType>(key_event->state),
                                        0, &key, 0, 0, 0);

    switch(key) {
        case GDK_KEY_Escape:
        {
            _transfer_focus = true;
            _btn->set_value(_last_val);
            // defocus
            was_consumed = true;
        }
        break;

        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
        {
            _transfer_focus = true;
            // defocus
            was_consumed = true;
        }
        break;

        case GDK_KEY_Tab:
        {
            _transfer_focus = false;
            //was_consumed = process_tab(1)
        }
        break;

        case GDK_KEY_ISO_Left_Tab:
        {
            _transfer_focus = false;
            //was_consumed = process_tab(1)
        }
        break;

        // TODO: Enable variable step-size if this is ever used
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
        case GDK_KEY_Page_Up:
        case GDK_KEY_KP_Page_Up:
        case GDK_KEY_Page_Down:
        case GDK_KEY_KP_Page_Down:
        break;

        case GDK_KEY_z:
        case GDK_KEY_Z:
        {
            _transfer_focus = false;
            _btn->set_value(_last_val);
            was_consumed = true;
        }
        break;
    }

    return was_consumed;
}

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
    : _btn(Gtk::manage(new SpinButton(adjustment, climb_rate, digits))),
      _last_val(0.0),
      _transfer_focus(false)
{
    // Handle button events
    auto btn_focus_in_event_cb = sigc::mem_fun(*this, &SpinButtonToolItem::on_btn_focus_in_event);
    _btn->signal_focus_in_event().connect(btn_focus_in_event_cb);

    auto btn_focus_out_event_cb = sigc::mem_fun(*this, &SpinButtonToolItem::on_btn_focus_out_event);
    _btn->signal_focus_out_event().connect(btn_focus_out_event_cb);

    auto btn_key_press_event_cb = sigc::mem_fun(*this, &SpinButtonToolItem::on_btn_key_press_event);
    _btn->signal_key_press_event().connect(btn_key_press_event_cb);

    // Create a label
    auto label = Gtk::manage(new Gtk::Label(label_text));

    // Arrange the widgets in a horizontal box
    auto hbox = Gtk::manage(new Gtk::Box());
    hbox->set_spacing(3);
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
