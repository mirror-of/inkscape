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

#include "dialogs/dialog-events.h"
#include "helper/sp-intl.h"
#include "inkscape.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"

#include "layer-properties.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

LayerPropertiesDialog::LayerPropertiesDialog()
{
    GtkWidget *dlg = GTK_WIDGET(gobj());
    g_assert(dlg);

    set_title(_("Layer Properties"));

    sp_transientize(dlg);

//    set_size_request(200,200);

    Gtk::VBox *mainVBox = get_vbox();

    _layer_name_label.set_label(_("Layer Name:"));
    _layer_name_hbox.pack_end(_layer_name_entry, false, false, 4);
    _layer_name_hbox.pack_end(_layer_name_label, false, false, 4);
    mainVBox->pack_start(_layer_name_hbox, false, false, 4);

    // Buttons
    add_button(_("Apply"), Gtk::RESPONSE_APPLY)->signal_clicked()
        .connect(sigc::mem_fun(*this, &LayerPropertiesDialog::apply));
    add_button(_("Close"), Gtk::RESPONSE_CLOSE)->signal_clicked()
        .connect(sigc::mem_fun(*this, &LayerPropertiesDialog::close));

    show_all_children();
}

LayerPropertiesDialog::~LayerPropertiesDialog()
{
}

void
LayerPropertiesDialog::apply()
{
    SPSelection * const selection = getSelection();
    g_return_if_fail (!selection->isEmpty());

    // TODO: Get the text from the widget
    // TODO: Update the selection with the text from widget

    _button_apply.set_sensitive(false);
}

void
LayerPropertiesDialog::close()
{
    hide();
}

void
LayerPropertiesDialog::update()
{
    SPSelection * const selection = getSelection();
    if (selection && !selection->isEmpty()) {
        // update based on the selection

        _button_apply.set_sensitive( true );
    } else {
        _button_apply.set_sensitive( false );
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
}

SPSelection *
LayerPropertiesDialog::getSelection()
{
    if (! SP_ACTIVE_DESKTOP) {
        return NULL;
    }

    return SP_DT_SELECTION (SP_ACTIVE_DESKTOP);
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
