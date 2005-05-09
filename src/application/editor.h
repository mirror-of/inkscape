/**
 * \brief  Class to manage an application used for editing SVG documents
 *         using GUI views
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_APPLICATION_EDITOR_H
#define INKSCAPE_APPLICATION_EDITOR_H

#include <gtkmm/window.h>
#include "app-prototype.h"

class SPDesktop;
class SPDocument;
class SPEventContext;

namespace Inkscape {
namespace XML {
class Document;
}
namespace UI {
namespace View {
class Edit;
}
}

namespace NSApplication {

class Editor : public AppPrototype
{
public:
    Editor(int argc, char **argv, bool use_gui=true);
    virtual ~Editor();

    Gtk::Window*    getWindow();
    SPDocument*     getActiveDocument();
    SPDesktop*      getActiveDesktop();
    SPEventContext* getEventContext();
    Glib::ustring   getName() const;

    int             loadPreferences();
    int             savePreferences();

    void            hideDialogs();
    void            unhideDialogs();
    void            toggleDialogs();

    void            nextDesktop();
    void            prevDesktop();

    void            refreshDisplay();
    void            exit();

    bool        lastViewOfDocument(SPDocument* doc, SPDesktop* view) const;
    
    bool        addDocument(SPDocument* doc);
    bool        deleteDocument(SPDocument* doc);

    bool        addView(SPDesktop* view);
    bool        deleteView(SPDesktop* view);

protected:
    UI::View::Edit *rep;

    Editor(Editor const &);
    Editor& operator=(Editor const &);

    Inkscape::XML::Document      *_preferences;
    GSList         *_documents;
    GSList         *_desktops;
    gchar          *_argv0;

    bool       _dialogs_toggle;
    bool       _save_preferences;
    bool       _use_gui;
};

} // namespace NSApplication
} // namespace Inkscape

#endif // INKSCAPE_APPLICATION_EDITOR_H

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
