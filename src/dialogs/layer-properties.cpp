/**
 *
 * \brief  Dialog for renaming layers
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.com>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/dialog.h>
#include <gtkmm/notebook.h>
#include <gtkmm/separator.h>
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/stock.h>

#include "dialogs/dialog-events.h"
#include "helper/sp-intl.h"
#include "inkscape.h"
#include "desktop.h"
#include "document.h"
#include "desktop-handles.h"
#include "layer-fns.h"

#include "layer-properties.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

static void hideCallback(GtkObject *object, gpointer dlgPtr)
{
    LayerPropertiesDialog *dlg = (LayerPropertiesDialog *) dlgPtr;
    dlg->hide();
}

static void unhideCallback(GtkObject *object, gpointer dlgPtr)
{
    LayerPropertiesDialog *dlg = (LayerPropertiesDialog *) dlgPtr;
    if (!dlg->userHidden()) {
        dlg->show();
        dlg->raise();
    }
}

LayerPropertiesDialog::LayerPropertiesDialog()
: _strategy(NULL)
{
    GtkWidget *dlg = GTK_WIDGET(gobj());
    g_assert(dlg);

    Gtk::VBox *mainVBox = get_vbox();

    // Rename Layer widgets
    _layer_name_entry.signal_activate()
        .connect(sigc::mem_fun(*this, &LayerPropertiesDialog::_apply));
    _layer_name_entry.set_activates_default(true);
    _layer_name_hbox.pack_end(_layer_name_entry, false, false, 4);
    _layer_name_label.set_label(_("Layer Name:"));
    _layer_name_hbox.pack_end(_layer_name_label, false, false, 4);
    mainVBox->pack_start(_layer_name_hbox, false, false, 4);

    // Buttons
    _close_button.set_use_stock(true);
    _close_button.set_label(Gtk::Stock::CANCEL.id);

    _apply_button.set_use_underline(true);
    _apply_button.set_flags(Gtk::CAN_DEFAULT);

    _close_button.signal_clicked()
        .connect(sigc::mem_fun(*this, &LayerPropertiesDialog::_close));
    _apply_button.signal_clicked()
        .connect(sigc::mem_fun(*this, &LayerPropertiesDialog::_apply));

    add_action_widget(_close_button, Gtk::RESPONSE_CLOSE);
    add_action_widget(_apply_button, Gtk::RESPONSE_APPLY);

    // Event Handlers
    // TODO:  Gtkmmify
    g_signal_connect( G_OBJECT(dlg), "event", 
                      GTK_SIGNAL_FUNC(sp_dialog_event_handler), dlg );
    g_signal_connect( G_OBJECT(INKSCAPE), "dialogs_hide", 
                      G_CALLBACK(hideCallback), (void *)this );
    g_signal_connect( G_OBJECT(INKSCAPE), "dialogs_unhide", 
                      G_CALLBACK(unhideCallback), (void *)this );

    set_default_response(Gtk::RESPONSE_APPLY);
    sp_transientize(dlg);
    show_all_children();
}

void LayerPropertiesDialog::_showDialog(LayerPropertiesDialog::Strategy &strategy,
                                        SPDesktop *desktop)
{
    _strategy = &strategy;

    _strategy->setup(*this);

    show();
    present();
    _userHidden = false;
}

void
LayerPropertiesDialog::_apply()
{
    g_assert(_strategy != NULL);

    _strategy->perform(*this);
    sp_document_done(SP_DT_DOCUMENT(SP_ACTIVE_DESKTOP));
    _userHidden = true;
    hide();
}

void
LayerPropertiesDialog::_close()
{
    g_assert(_strategy != NULL);

    _userHidden = true;
    hide();
}

void LayerPropertiesDialog::Rename::setup(LayerPropertiesDialog &dialog) {
    SPDesktop *desktop=SP_ACTIVE_DESKTOP;
    dialog.set_title(_("Rename Layer"));
    gchar const *name=desktop->currentLayer()->label();
    dialog._layer_name_entry.set_text(( name ? name : "" ));
    dialog._apply_button.set_label(_("_Rename"));
}

void LayerPropertiesDialog::Rename::perform(LayerPropertiesDialog &dialog) {
    SPDesktop *desktop=SP_ACTIVE_DESKTOP;
    Glib::ustring name(dialog._layer_name_entry.get_text());
    desktop->currentLayer()->setLabel(
        ( name.empty() ? NULL : (gchar *)name.c_str() )
    );
    sp_document_done(SP_DT_DOCUMENT(desktop));
    // TRANSLATORS: This means "The layer has been renamed"
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Renamed layer"));
}

void LayerPropertiesDialog::Create::setup(LayerPropertiesDialog &dialog) {
    dialog.set_title(_("Create Layer"));
    dialog._layer_name_entry.set_text("");
    dialog._apply_button.set_label(_("C_reate"));
}

void LayerPropertiesDialog::Create::perform(LayerPropertiesDialog &dialog) {
    SPDesktop *desktop=SP_ACTIVE_DESKTOP;
    SPObject *layer=Inkscape::create_layer(desktop->currentRoot(), desktop->currentLayer());
    desktop->setCurrentLayer(layer);
    SP_DT_SELECTION(desktop)->clear();
    Glib::ustring name(dialog._layer_name_entry.get_text());
    if (!name.empty()) {
        desktop->currentLayer()->setLabel((gchar *)name.c_str());
    }
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("New layer created."));
}

} // namespace
} // namespace
} // namespace


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
