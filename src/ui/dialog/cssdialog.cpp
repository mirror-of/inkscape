// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief A dialog for CSS selectors
 */
/* Authors:
 *   Kamalpreet Kaur Grewal
 *   Tavmjong Bah
 *
 * Copyright (C) Kamalpreet Kaur Grewal 2016 <grewalkamal005@gmail.com>
 * Copyright (C) Tavmjong Bah 2017 <tavmjong@free.fr>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "cssdialog.h"

#include "verbs.h"
#include "selection.h"
#include "message-context.h"
#include "message-stack.h"
#include "ui/icon-loader.h"
#include "ui/widget/iconrenderer.h"

#include "xml/attribute-record.h"
#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * Constructor
 * A treeview whose each row corresponds to a CSS property of selector selected.
 * New CSS property can be added by clicking '+' at bottom of the CSS pane. '-'
 * in front of the CSS property row can be clicked to delete the CSS property.
 * Besides clicking on an already selected property row makes the property editable
 * and clicking 'Enter' updates the property with changes reflected in the
 * drawing.
 */
CssDialog::CssDialog():
    UI::Widget::Panel("/dialogs/css", SP_VERB_DIALOG_CSS),
    _desktop(nullptr)
{
    set_size_request(20, 15);
    _mainBox.pack_start(_scrolledWindow, Gtk::PACK_EXPAND_WIDGET);
    _treeView.set_headers_visible(true);
    _scrolledWindow.add(_treeView);
    _scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    _store = Gtk::ListStore::create(_cssColumns);
    _treeView.set_model(_store);

    Inkscape::UI::Widget::IconRenderer * addRenderer = manage(new Inkscape::UI::Widget::IconRenderer());
    addRenderer->add_icon("edit-delete");

    int addCol = _treeView.append_column("", *addRenderer) - 1;
    Gtk::TreeViewColumn *col = _treeView.get_column(addCol);

    _propRenderer = Gtk::manage(new Gtk::CellRendererText());
    _propRenderer->property_editable() = true;
    int nameColNum = _treeView.append_column("CSS Property", *_propRenderer) - 1;
    _propCol = _treeView.get_column(nameColNum);
    if (_propCol) {
      _propCol->add_attribute(_propRenderer->property_text(), _cssColumns._propertyLabel);
    }

    _sheetRenderer = Gtk::manage(new Gtk::CellRendererText());
    _sheetRenderer->property_editable() = true;
    int sheetColNum = _treeView.append_column("Style Sheet", *_sheetRenderer) - 1;
    _sheetCol = _treeView.get_column(sheetColNum);
    if (_sheetCol) {
      _sheetCol->add_attribute(_sheetRenderer->property_text(), _cssColumns._styleSheetVal);
    }

    _attrRenderer = Gtk::manage(new Gtk::CellRendererText());
    _attrRenderer->property_editable() = false;
    int attrColNum = _treeView.append_column("Style Attribute", *_attrRenderer) - 1;
    _attrCol = _treeView.get_column(attrColNum);
    if (_attrCol) {
      _attrCol->add_attribute(_attrRenderer->property_text(), _cssColumns._styleAttrVal);
    }

    status.set_halign(Gtk::ALIGN_START);
    status.set_valign(Gtk::ALIGN_CENTER);
    status.set_size_request(1, -1);
    status.set_markup("");
    status.set_line_wrap(true);
    status_box.pack_start( status, TRUE, TRUE, 0);
    _getContents()->pack_end(status_box, false, false, 2);

    _message_stack = std::make_shared<Inkscape::MessageStack>();
    _message_context = std::unique_ptr<Inkscape::MessageContext>(new Inkscape::MessageContext(_message_stack));
    _message_changed_connection = _message_stack->connectChanged(
            sigc::bind(sigc::ptr_fun(_set_status_message), GTK_WIDGET(status.gobj())));


    GtkWidget *child = sp_get_icon_image("list-add", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_show(child);
    _buttonAddProperty.add(*manage(Glib::wrap(child)));
    _buttonAddProperty.set_relief(Gtk::RELIEF_NONE);
    _buttonAddProperty.set_tooltip_text("Add a new property");

    _mainBox.pack_end(_buttonBox, Gtk::PACK_SHRINK);
    _buttonBox.pack_start(_buttonAddProperty, Gtk::PACK_SHRINK);

    _getContents()->pack_start(_mainBox, Gtk::PACK_EXPAND_WIDGET);

    css_reset_context(0);
    setDesktop(getDesktop());

    _buttonAddProperty.signal_clicked().connect(sigc::mem_fun(*this, &CssDialog::_addProperty));
}


/**
 * @brief CssDialog::~CssDialog
 * Class destructor
 */
CssDialog::~CssDialog()
{
    setDesktop(nullptr);
    _message_changed_connection.disconnect();
    _message_context = nullptr;
    _message_stack = nullptr;
    _message_changed_connection.~connection();
}

void CssDialog::_set_status_message(Inkscape::MessageType /*type*/, const gchar *message, GtkWidget *widget)
{
    if (widget) {
        gtk_label_set_markup(GTK_LABEL(widget), message ? message : "");
    }
}


/**
 * @brief CssDialog::setDesktop
 * @param desktop
 * This function sets the 'desktop' for the CSS pane.
 */
void CssDialog::setDesktop(SPDesktop* desktop)
{
    _desktop = desktop;
}

/**
 * Sets the CSSDialog status bar, depending on which attr is selected.
 */
void CssDialog::css_reset_context(gint css)
{
    if (css == 0) {
        _message_context->set(Inkscape::NORMAL_MESSAGE,
                              _("<b>Click</b> CSS property to edit."));
    }
    else {
        const gchar *name = g_quark_to_string(css);
        _message_context->setF(Inkscape::NORMAL_MESSAGE,
                               _("Propery <b>%s</b> selected. Press <b>Ctrl+Enter</b> when done editing to commit changes."), name);
    }
}

/**
 * @brief CssDialog::_addProperty
 * This function is a slot to signal_clicked for '+' button at the bottom of CSS
 * panel. A new row is added, double clicking which text for new property can be
 * added.
 */
void CssDialog::_addProperty()
{
    _propRow = *(_store->append());
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape
