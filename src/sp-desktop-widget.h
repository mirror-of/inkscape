#ifndef SEEN_SP_DESKTOP_WIDGET_H
#define SEEN_SP_DESKTOP_WIDGET_H

#include <glib/gtypes.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkwidget.h>

#include <display/display-forward.h>
#include <libnr/nr-point.h>
#include "forward.h"
#include "message.h"
#include "view.h"

#define SP_TYPE_DESKTOP_WIDGET (sp_desktop_widget_get_type ())
#define SP_DESKTOP_WIDGET(o) (GTK_CHECK_CAST ((o), SP_TYPE_DESKTOP_WIDGET, SPDesktopWidget))
#define SP_DESKTOP_WIDGET_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_DESKTOP_WIDGET, SPDesktopWidgetClass))
#define SP_IS_DESKTOP_WIDGET(o) (GTK_CHECK_TYPE ((o), SP_TYPE_DESKTOP_WIDGET))
#define SP_IS_DESKTOP_WIDGET_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_DESKTOP_WIDGET))

namespace Inkscape { namespace Widgets { class LayerSelector; } }

struct SPDesktopWidget {
    SPViewWidget viewwidget;

    unsigned int update : 1;

    SPDesktop *desktop;

    // The root vbox of the window layout.
    GtkWidget *vbox;

    GtkWidget *menubar, *statusbar;

    GtkWidget *hscrollbar, *vscrollbar, *vscrollbar_box;

    GtkWidget *tool_toolbox, *aux_toolbox, *commands_toolbox;

    /* Rulers */
    GtkWidget *hruler, *vruler;
    double dt2r;
    NR::Point ruler_origin;

    GtkWidget *sticky_zoom;
    GtkWidget *coord_status;
    GtkWidget *select_status;
    GtkWidget *zoom_status;
    gulong zoom_update;

    gint coord_status_id, select_status_id;

    SPCanvas *canvas;

    GtkAdjustment *hadj, *vadj;

    Inkscape::Widgets::LayerSelector *layer_selector;

    void setMessage(Inkscape::MessageType type, gchar const *message);
    void startRenameLayer();
};

struct SPDesktopWidgetClass {
    SPViewWidgetClass parent_class;
};

GtkType sp_desktop_widget_get_type();

/* Constructor */

gint sp_desktop_widget_set_focus(GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw);

void sp_desktop_widget_show_decorations(SPDesktopWidget *dtw, gboolean show);

void sp_desktop_widget_layout(SPDesktopWidget *dtw);


#endif /* !SEEN_SP_DESKTOP_WIDGET_H */

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
