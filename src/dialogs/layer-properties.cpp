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
    : _vbox(false, 4)
{
    set_title(_("Layer Properties"));
//    sp_transientize();

    add(_hbox);
    _hbox.pack_start(_vbox, true, true);

    // Buttons
    _vbox.pack_start(_hbuttonbox, false, false);
    _hbuttonbox.pack_start(_button_apply, false, false);
    _hbuttonbox.pack_start(_button_close, false, false);

    _button_apply.signal_clicked()
        .connect(sigc::mem_fun(*this, &LayerPropertiesDialog::apply));
    _button_close.signal_clicked()
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
