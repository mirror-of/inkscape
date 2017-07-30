#ifndef SEEN_SP_BUTTON_H
#define SEEN_SP_BUTTON_H

/**
 * Generic button widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include <gtkmm/togglebutton.h>
#include <sigc++/connection.h>
#include "icon-size.h"

struct SPAction;

namespace Inkscape {
namespace UI {
namespace View {
class View;
}

namespace Widget {

class Button : public Gtk::ToggleButton {
private:
    Gtk::IconSize  _lsize;
    SPAction      *_action;
    SPAction      *_doubleclick_action;

    sigc::connection _c_set_active;
    sigc::connection _c_set_sensitive;

    // True if handling of "clicked" signal should be blocked
    bool _block_on_clicked;

    void init();
    void set_action(SPAction *action);
    void set_doubleclick_action(SPAction *action);
    void action_set_active(bool active);
    void set_composed_tooltip(SPAction *action);

protected:
    virtual void on_clicked() override;
    virtual bool on_event(GdkEvent *event) override;
    virtual void get_preferred_width_vfunc( int &minimum_width,
                                            int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &minimum_width,
                                            int &natural_width) const override;

public:
    Button(Gtk::IconSize  size,
           SPAction      *action,
           SPAction      *doubleclick_action);

    Button(Gtk::IconSize             size,
           Inkscape::UI::View::View *view,
           const gchar              *name,
           const gchar              *tip);

    void toggle_set_down(bool down);
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape
#endif // !SEEN_SP_BUTTON_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
