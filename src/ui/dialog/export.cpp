// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Peter Bostrom
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 1999-2007, 2012, 2021 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

// This has to be included prior to anything that includes setjmp.h, it croaks otherwise
#include "export.h"

#include <glibmm/convert.h>
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>
#include <gtkmm.h>
#include <png.h>

#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "extension/db.h"
#include "file.h"
#include "helper/png-write.h"
#include "inkscape-window.h"
#include "inkscape.h"
#include "io/resource.h"
#include "io/sys.h"
#include "message-stack.h"
#include "object/object-set.h"
#include "object/sp-namedview.h"
#include "object/sp-root.h"
#include "preferences.h"
#include "selection-chemistry.h"
#include "ui/dialog-events.h"
#include "ui/dialog/dialog-notebook.h"
#include "ui/dialog/filedialog.h"
#include "ui/interface.h"
#include "ui/widget/scrollprotected.h"
#include "ui/widget/unit-menu.h"

#ifdef _WIN32

#endif

using Inkscape::Util::unit_table;
namespace Inkscape {
namespace UI {
namespace Dialog {

Export::Export()
    : DialogBase("/dialogs/export/", "Export")
{
    std::string gladefile = get_filename_string(Inkscape::IO::Resource::UIS, "dialog-export.glade");

    try {
        builder = Gtk::Builder::create_from_file(gladefile);
    } catch (const Glib::Error &ex) {
        g_error("Glade file loading failed for export screen");
        return;
    }

    prefs = Inkscape::Preferences::get();

    builder->get_widget("Export Dialog Box", container);
    add(*container);
    show_all_children();

    builder->get_widget("Export Notebook", export_notebook);

    // Initialise Single Export and its objects
    builder->get_widget_derived("Single Image", single_image);
    single_image->initialise(builder);

    // Initialise Batch Export and its objects
    builder->get_widget_derived("Batch Export", batch_export);
    batch_export->initialise(builder);

    if (single_image) {
        single_image->setDesktop(getDesktop());
        single_image->setApp(getApp());
    }
    if (batch_export) {
        batch_export->setDesktop(getDesktop());
        batch_export->setApp(getApp());
    }

    // Callback when container is finally mapped on window. All intialisation like set active is done inside it.
    container->signal_realize().connect(sigc::mem_fun(*this, &Export::onRealize));
    export_notebook->signal_switch_page().connect(sigc::mem_fun(*this, &Export::onPageSwitch));
}

Export::~Export() {}

// When conainer is visible then setup all widgets.
// It prevents gtk_is_widget assertion warning probably.
void Export::onRealize()
{
    if (single_image) {
        single_image->setDesktop(getDesktop());
        single_image->setApp(getApp());
        single_image->setup();
    }
    if (batch_export) {
        batch_export->setDesktop(getDesktop());
        batch_export->setApp(getApp());
        batch_export->setup();
    }
    setDefaultNotebookPage();
}

// Set current page based on preference/last visited page
void Export::setDefaultNotebookPage()
{
    pages[BATCH_EXPORT] = export_notebook->page_num(*batch_export);
    pages[SINGLE_IMAGE] = export_notebook->page_num(*single_image);
    export_notebook->set_current_page(pages[SINGLE_IMAGE]);
}

void Export::documentReplaced()
{
    if (single_image) {
        single_image->setDocument(getDocument());
    }
    if (batch_export) {
        batch_export->setDocument(getDocument());
    }
}

void Export::desktopReplaced()
{
    if (single_image) {
        single_image->setDesktop(getDesktop());
    }
    if (batch_export) {
        batch_export->setDesktop(getDesktop());
    }
}

void Export::selectionChanged(Inkscape::Selection *selection)
{
    auto current_page = export_notebook->get_current_page();
    if (current_page == pages[SINGLE_IMAGE]) {
        single_image->selectionChanged(selection);
    }
    if (current_page == pages[BATCH_EXPORT]) {
        batch_export->selectionChanged(selection);
    }
}
void Export::selectionModified(Inkscape::Selection *selection, guint flags)
{
    auto current_page = export_notebook->get_current_page();
    if (current_page == pages[SINGLE_IMAGE]) {
        single_image->selectionModified(selection, flags);
    }
    if (current_page == pages[BATCH_EXPORT]) {
        batch_export->selectionModified(selection, flags);
    }
}

void Export::onPageSwitch(Widget *page, guint page_number)
{
    auto desktop = getDesktop();
    if (desktop) {
        auto selection = desktop->getSelection();

        if (page_number == pages[SINGLE_IMAGE]) {
            single_image->selectionChanged(selection);
        }
        if (page_number == pages[BATCH_EXPORT]) {
            batch_export->selectionChanged(selection);
        }
    }
}

} // namespace Dialog
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
