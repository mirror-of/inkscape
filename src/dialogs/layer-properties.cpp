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
    if (! dlg->userHidden()) {
        dlg->show();
        dlg->raise();
    }
}

LayerPropertiesDialog::LayerPropertiesDialog()
{
    GtkWidget *dlg = GTK_WIDGET(gobj());
    g_assert(dlg);

    Gtk::VBox *mainVBox = get_vbox();

    // Rename Layer widgets
    gchar const * name = getLayerName();
    if (name != NULL) {
        _layer_name_entry.set_text(name);
    }
    _layer_name_entry.signal_activate()
        .connect(sigc::mem_fun(*this, &LayerPropertiesDialog::_apply));
    _layer_name_entry.set_activates_default(true);
    _layer_name_hbox.pack_end(_layer_name_entry, false, false, 4);
    _layer_name_label.set_label(_("Layer Name:"));
    _layer_name_hbox.pack_end(_layer_name_label, false, false, 4);
    mainVBox->pack_start(_layer_name_hbox, false, false, 4);

    // Buttons
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

LayerPropertiesDialog::~LayerPropertiesDialog()
{
}

void
LayerPropertiesDialog::_apply()
{
    setLayerName((gchar*)_layer_name_entry.get_text().c_str());
    userHidden(true);
    hide();
}

void
LayerPropertiesDialog::_close()
{
    userHidden(true);
    hide();
}

void
LayerPropertiesDialog::update()
{
    // Get the current layer name
    gchar const * name = getLayerName();

    if (name != NULL) {
        // update based on the currently selected layer
        _layer_name_entry.set_text(name);
        _apply_button.set_sensitive(true);
    } else {
        _layer_name_entry.set_text("");
        _apply_button.set_sensitive(false);
    }
}

/* static instance, like done in DebugDialog */
static LayerPropertiesDialog *layerPropertiesDialogInstance = NULL;

LayerPropertiesDialog *LayerPropertiesDialog::getInstance()
{
    if ( !layerPropertiesDialogInstance )
    {
        layerPropertiesDialogInstance = new LayerPropertiesDialog();
    }
    return layerPropertiesDialogInstance;
}

void LayerPropertiesDialog::showInstance()
{
    LayerPropertiesDialog *dlg = getInstance();
    g_assert(dlg);
    dlg->show();
    dlg->present();
    dlg->userHidden(false);

    dlg->set_title(_("Rename Layer"));

    dlg->_close_button.set_use_stock(true);
    dlg->_close_button.set_label(Gtk::Stock::CANCEL.id);

    dlg->_apply_button.set_use_stock(false);
    dlg->_apply_button.set_use_underline(true);
    dlg->_apply_button.set_label(_("_Rename"));
}

void
LayerPropertiesDialog::setLayerName(gchar const * name)
{
    // Get the active desktop
    SPDesktop * const desktop = SP_ACTIVE_DESKTOP;
    g_return_if_fail (desktop != NULL);

    // Retrieve current layer
    SPObject * layer = desktop->currentLayer();
    g_return_if_fail (layer != NULL);

    // Set they layer's label to the retrieved text
    layer->setLabel(name);

    // Notify that we've made a change
    sp_document_done(SP_DT_DOCUMENT(desktop));
    // TRANSLATORS: This means "The layer has been renamed"
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Renamed layer"));
}

gchar const *
LayerPropertiesDialog::getLayerName() const
{
    // Get the active desktop
    SPDesktop * const desktop = SP_ACTIVE_DESKTOP;
    g_return_val_if_fail (desktop != NULL, NULL);

    // Retrieve current layer
    SPObject * layer = desktop->currentLayer();
    g_return_val_if_fail (layer != NULL, NULL);

    // Get the layer's name
    return layer->label();
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
