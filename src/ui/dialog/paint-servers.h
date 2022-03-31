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
#include <sigc++/sigc++.h>

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

    PaintDescription(SPDocument *source_doc, Glib::ustring title, Glib::ustring const &&paint_url);
    void write_to_iterator(Gtk::ListStore::iterator &it, PaintServersColumns const *cols) const;

    /** Whether the paint with this description has a valid pixbuf preview */
    inline bool has_preview() const { return (bool)bitmap; }

    /** Two paints are considered the same if they have the same urls */
    inline bool operator==(PaintDescription const &other) const { return url == other.url; }
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
    using MaybeString = std::optional<Glib::ustring>;
public:
    ~PaintServersDialog() override;
    static PaintServersDialog &getInstance() { return *new PaintServersDialog(); }

    void documentReplaced() override;

private:
    // No default constructor, noncopyable, nonassignable
    PaintServersDialog();
    PaintServersDialog(PaintServersDialog const &d) = delete;
    PaintServersDialog operator=(PaintServersDialog const &d) = delete;

    void _addToStore(PaintDescription &paint);
    void _buildDialogWindow(char const *const glade_file);
    void _createPaints(std::vector<PaintDescription> &collection);
    PaintDescription _descriptionFromIterator(Gtk::ListStore::iterator const &iter) const;
    void _documentClosed();
    std::tuple<MaybeString, MaybeString> _findCommonFillAndStroke(std::vector<SPObject *> const &objects) const;
    static void _findPaints(SPObject *in, std::vector<Glib::ustring> &list);
    void _generateBitmapPreview(PaintDescription& paint);
    void _instantiatePaint(PaintDescription &paint);
    void _loadFromCurrentDocument();
    void _loadPaintsFromDocument(SPDocument *document, std::vector<PaintDescription> &output);
    void _loadStockPaints();
    void _regenerateAll();
    void _unpackGroups(SPObject *parent, std::vector<SPObject *> &output) const;
    std::vector<SPObject *> _unpackSelection(Selection *selection) const;
    void _updateActiveItem();
    void onPaintClicked(const Gtk::TreeModel::Path &path);
    void onPaintSourceDocumentChanged();
    void selectionChanged(Selection *selection) final;

    bool _targetting_fill; ///< whether setting fill (true) or stroke (false)
    const Glib::ustring ALLDOCS;
    const Glib::ustring CURRENTDOC;
    std::map<Glib::ustring, Glib::RefPtr<Gtk::ListStore>> store;
    Glib::ustring current_store;
    std::vector<std::unique_ptr<SPDocument>> _stock_documents;
    std::map<Glib::ustring, SPDocument *> document_map;
    SPDocument *preview_document = nullptr;
    Inkscape::Drawing renderDrawing;
    Gtk::ComboBoxText *dropdown = nullptr;
    Gtk::IconView *icon_view = nullptr;
    PaintServersColumns const columns;
    sigc::connection _defs_changed, _document_closed;
    MaybeString _common_stroke, _common_fill; ///< Common fill/stroke to all selected elements
    sigc::connection _item_activated;
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
