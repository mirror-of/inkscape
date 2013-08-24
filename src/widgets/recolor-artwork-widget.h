
#ifndef SEEN_DIALOGS_RECOLOR_ARTWORK_WIDGET_H
#define SEEN_DIALOGS_RECOLOR_ARTWORK_WIDGET_H

namespace Inkscape {
namespace Widgets {

Gtk::Widget *createRecolorArtworkWidget( );

} // namespace Widgets
} // namespace Inkscape

namespace Gtk {
class Widget;
}

class SPDesktop;

Gtk::Widget *recolor_artwork_widget_new(void);

void recolor_artwork_widget_set_desktop(Gtk::Widget *widget, SPDesktop *desktop);

#endif // SEEN_DIALOGS_RECOLOR_ARTWORK_WIDGET_H
