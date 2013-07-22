/**
 * @file
 * Document metadata dialog, Gtkmm-style.
 */
/* Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2000 - 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "document-metadata.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "inkscape.h"
#include "rdf.h"
#include "sp-namedview.h"
#include "ui/widget/entity-entry.h"
#include "verbs.h"
#include "xml/node-event-vector.h"


namespace Inkscape {
namespace UI {
namespace Dialog {

#define SPACE_SIZE_X 15
#define SPACE_SIZE_Y 15

//===================================================

//---------------------------------------------------

static void on_repr_attr_changed (Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer);

static Inkscape::XML::NodeEventVector const _repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    on_repr_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


DocumentMetadata &
DocumentMetadata::getInstance()
{
    DocumentMetadata &instance = *new DocumentMetadata();
    instance.init();
    return instance;
}


DocumentMetadata::DocumentMetadata()
#if WITH_GTKMM_3_0
    : UI::Widget::Panel ("", "/dialogs/documentmetadata", SP_VERB_DIALOG_METADATA)
#else
    : UI::Widget::Panel ("", "/dialogs/documentmetadata", SP_VERB_DIALOG_METADATA),
      _page_metadata1(1, 1), _page_metadata2(1, 1)
#endif
{
    hide();
    _getContents()->set_spacing (4);
    _getContents()->pack_start(_notebook, true, true);

    _page_metadata1.set_border_width(2);
    _page_metadata2.set_border_width(2);
   
#if WITH_GTKMM_3_0
    _page_metadata1.set_column_spacing(2);
    _page_metadata2.set_column_spacing(2);
    _page_metadata1.set_row_spacing(2);
    _page_metadata2.set_row_spacing(2);
#else 
    _page_metadata1.set_spacings(2);
    _page_metadata2.set_spacings(2);
#endif
    
    _notebook.append_page(_page_metadata1, _("Metadata"));
    _notebook.append_page(_page_metadata2, _("License"));

    signalDocumentReplaced().connect(sigc::mem_fun(*this, &DocumentMetadata::_handleDocumentReplaced));
    signalActivateDesktop().connect(sigc::mem_fun(*this, &DocumentMetadata::_handleActivateDesktop));
    signalDeactiveDesktop().connect(sigc::mem_fun(*this, &DocumentMetadata::_handleDeactivateDesktop));

}

void
DocumentMetadata::init()
{
    update();

    Inkscape::XML::Node *repr = sp_desktop_namedview(getDesktop())->getRepr();
    repr->addListener (&_repr_events, this);

    show_all_children();
}

DocumentMetadata::~DocumentMetadata()
{
    Inkscape::XML::Node *repr = sp_desktop_namedview(getDesktop())->getRepr();
    repr->removeListenerByData (this);

    for (RDElist::iterator it = _rdflist.begin(); it != _rdflist.end(); ++it)
        delete (*it);
}

/**
 * Update dialog widgets from desktop.
 */
void DocumentMetadata::update()
{
    if (_wr.isUpdating()) return;

    _wr.setUpdating (true);
    set_sensitive (true);

    //-----------------------------------------------------------meta pages
    /* update the RDF entities */
    for (RDElist::iterator it = _rdflist.begin(); it != _rdflist.end(); ++it)
        (*it)->update (SP_ACTIVE_DOCUMENT);

    _licensor.update (SP_ACTIVE_DOCUMENT);

    _wr.setUpdating (false);
}

void 
DocumentMetadata::_handleDocumentReplaced(SPDesktop* desktop, SPDocument *)
{
    Inkscape::XML::Node *repr = sp_desktop_namedview(desktop)->getRepr();
    repr->addListener (&_repr_events, this);
    update();
}

void 
DocumentMetadata::_handleActivateDesktop(Inkscape::Application *, SPDesktop *desktop)
{
    Inkscape::XML::Node *repr = sp_desktop_namedview(desktop)->getRepr();
    repr->addListener(&_repr_events, this);
    update();
}

void
DocumentMetadata::_handleDeactivateDesktop(Inkscape::Application *, SPDesktop *desktop)
{
    Inkscape::XML::Node *repr = sp_desktop_namedview(desktop)->getRepr();
    repr->removeListenerByData(this);
}

//--------------------------------------------------------------------

/**
 * Called when XML node attribute changed; updates dialog widgets.
 */
static void on_repr_attr_changed(Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer data)
{
    if (DocumentMetadata *dialog = static_cast<DocumentMetadata *>(data))
	dialog->update();
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
