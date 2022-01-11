// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 1999-2007, 2021 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SP_EXPORT_H
#define SP_EXPORT_H

#include <gtkmm.h>

#include "export-batch.h"
#include "export-helper.h"
#include "export-single.h"
#include "extension/output.h"
#include "ui/dialog/dialog-base.h"
#include "ui/widget/scrollprotected.h"
#include "ui/widget/unit-menu.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

enum notebook_page
{
    SINGLE_IMAGE = 0,
    BATCH_EXPORT
};

class Export : public DialogBase
{
public:
    Export();
    ~Export() override;

    static Export &getInstance() { return *new Export(); }

private:
    Glib::RefPtr<Gtk::Builder> builder;
    Gtk::Box *container = nullptr;            // Main Container
    Gtk::Notebook *export_notebook = nullptr; // Notebook Container for single and batch export

private:
    SingleExport *single_image = nullptr;
    BatchExport *batch_export = nullptr;

private:
    Inkscape::Preferences *prefs = nullptr;

    // setup default values of widgets
    void setDefaultNotebookPage();
    std::map<notebook_page, int> pages;

private:
    // signals callback
    void onRealize();
    void onPageSwitch(Widget *page, guint page_number);

private:
    void documentReplaced() override;
    void desktopReplaced() override;
    void selectionChanged(Inkscape::Selection *selection) override;
    void selectionModified(Inkscape::Selection *selection, guint flags) override;
};
} // namespace Dialog
} // namespace UI
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :