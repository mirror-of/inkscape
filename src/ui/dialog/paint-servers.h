// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief Paint Servers dialog
 */
/* Authors:
 *   Valentin Ionita
 *   Rafael Siejakowski <rs@rs-math.net>
 *
 * Copyright (C) 2019 Valentin Ionita
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_PAINT_SERVERS_H
#define INKSCAPE_UI_DIALOG_PAINT_SERVERS_H

#include <glibmm/i18n.h>
#include <gtkmm.h>

#include "display/drawing.h"
#include "ui/dialog/dialog-base.h"

class SPObject;

namespace Inkscape {
namespace UI {
namespace Dialog {

class PaintServersColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    Gtk::TreeModelColumn<Glib::ustring> id;
    Gtk::TreeModelColumn<Glib::ustring> paint;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> pixbuf;
    Gtk::TreeModelColumn<Glib::ustring> document;

    PaintServersColumns()
    {
        add(id);
        add(paint);
        add(pixbuf);
        add(document);
    }
};

struct PaintDescription
{
    /** Pointer to the document from which the paint originates */
    SPDocument *source_document = nullptr;

    /** Title of the document from which the paint originates, or "Current document" */
    Glib::ustring doc_title;

    /** ID of the the paint server within the document */
    Glib::ustring id;

    /** URL of the paint within the document */
    Glib::ustring url;

    /** Bitmap preview of the paint */
    Glib::RefPtr<Gdk::Pixbuf> bitmap;

    PaintDescription(SPDocument *source_doc, Glib::ustring title, Glib::ustring const &&paint_url)
        : source_document{source_doc}
        , doc_title{std::move(title)}
        , id{} // id will be filled in when generating the bitmap
        , url{paint_url}
        , bitmap{nullptr}
    {}

    /** Two paints are considered the same if they have the same urls */
    bool operator==(PaintDescription const &other) const { return url == other.url; }
};

/**
 * This dialog serves as a preview for different types of paint servers,
 * currently only predefined. It can set the fill or stroke of the selected
 * object to the to the paint server you select.
 *
 * Patterns and hatches are loaded from the preferences paths and displayed
 * for each document, for all documents and for the current document.
 */

class PaintServersDialog : public DialogBase
{
public:
    ~PaintServersDialog() override;
    static PaintServersDialog &getInstance() { return *new PaintServersDialog(); }

    void documentReplaced() override;

private:
    // No default constructor, noncopyable, nonassignable
    PaintServersDialog();
    PaintServersDialog(PaintServersDialog const &d) = delete;
    PaintServersDialog operator=(PaintServersDialog const &d) = delete;

    void _cleanupUnused();
    void _createPaints(std::vector<PaintDescription> &collection);
    PaintDescription _descriptionFromIterator(Gtk::ListStore::iterator const &iter) const;
    std::vector<SPObject *> extract_elements(SPObject *item);
    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf(SPDocument *, Glib::ustring const &, Glib::ustring &);
    void _instantiatePaint(PaintDescription &paint);
    void _loadFromCurrentDocument();
    void _loadPaintsFromDocument(SPDocument *document, std::vector<PaintDescription> &output);
    void _loadStockPaints();
    void _regenerateAll();
    void onPaintClicked(const Gtk::TreeModel::Path &path);
    void onPaintSourceDocumentChanged();
    void on_target_changed();

    bool target_selected; ///< whether setting fill (true) or stroke (false)
    const Glib::ustring ALLDOCS;
    const Glib::ustring CURRENTDOC;
    std::map<Glib::ustring, Glib::RefPtr<Gtk::ListStore>> store;
    Glib::ustring current_store;
    std::map<Glib::ustring, SPDocument *> document_map;
    SPDocument *preview_document;
    Inkscape::Drawing renderDrawing;
    Gtk::ComboBoxText *dropdown;
    Gtk::IconView *icon_view;
    Gtk::ComboBoxText *target_dropdown;
    PaintServersColumns const columns;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // SEEN INKSCAPE_UI_DIALOG_PAINT_SERVERS_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-basic-offset:2
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=2:tabstop=8:softtabstop=2:fileencoding=utf-8:textwidth=99 :
