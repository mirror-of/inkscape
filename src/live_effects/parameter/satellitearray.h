// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_SATELLITEARRAY_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_SATELLITEARRAY_H

/*
 * Inkscape::LivePathEffectParameters
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treestore.h>
#include <sigc++/sigc++.h>

#include "live_effects/lpeobject.h"
#include "live_effects/parameter/array.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/satellite-reference.h"

class SPObject;

namespace Inkscape {
namespace LivePathEffect {
class SatelliteReference;
class SatelliteArrayParam : public ArrayParam<std::shared_ptr<SatelliteReference>>
{
public:
    class ModelColumns;

    SatelliteArrayParam(const Glib::ustring &label, const Glib::ustring &tip, const Glib::ustring &key,
                        Inkscape::UI::Widget::Registry *wr, Effect *effect, bool visible);

    ~SatelliteArrayParam() override;
    Gtk::Widget *param_newWidget() override;
    bool param_readSVGValue(const gchar *strvalue) override;
    void link(SPObject *to, size_t pos = Glib::ustring::npos);
    void unlink(std::shared_ptr<SatelliteReference> to);
    void unlink(SPObject *to);
    bool is_connected(){ return linked_connections.size() != 0; };
    void clear();
    void setUpdating(bool updating) { _updating = updating; }
    bool getUpdating() const { return _updating; }
    void start_listening();
protected:
    void quit_listening();
    bool _updateLink(const Gtk::TreeIter &iter, std::shared_ptr<SatelliteReference> lpref);
    bool _selectIndex(const Gtk::TreeIter &iter, int *i);
    void updatesignal();
    ModelColumns *_model;
    Glib::RefPtr<Gtk::TreeStore> _store;
    Gtk::TreeView *_tree;
    Gtk::ScrolledWindow *_scroller;
    Gtk::CellRendererText *_text_renderer;
    Gtk::CellRendererToggle *_toggle_active;
    Gtk::TreeView::Column *_name_column;
    void on_link_button_click();
    void on_remove_button_click();
    void on_up_button_click();
    void on_down_button_click();
    void on_active_toggled(const Glib::ustring &item);

private:
    bool _updating = false;
    void update();
    void initui();
    bool _visible;
    std::vector<sigc::connection> linked_connections;
    std::vector<SPObject *> param_get_satellites() override;
    SatelliteArrayParam(const SatelliteArrayParam &) = delete;
    SatelliteArrayParam &operator=(const SatelliteArrayParam &) = delete;
};

} // namespace LivePathEffect

} // namespace Inkscape

#endif

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
