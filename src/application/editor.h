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

class SPDesktop;
class SPDocument;
class SPEventContext;

namespace Inkscape {
namespace XML {
class Document;
}

namespace NSApplication {

class Editor
{
public:
    Editor(int argc, char **argv, gboolean use_gui=true);
    virtual ~Editor();

    Gtk::Window*    getImpl();
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

    gboolean        lastViewOfDocument(SPDocument* doc, SPDesktop* view) const;
    
    gboolean        addDocument(SPDocument* doc);
    gboolean        deleteDocument(SPDocument* doc);

    gboolean        addView(SPDesktop* view);
    gboolean        deleteView(SPDesktop* view);

protected:
    class EditorImpl;
    EditorImpl *rep;

    Editor(Editor const &);
    Editor& operator=(Editor const &);

    Inkscape::XML::Document      *_preferences;
    GSList         *_documents;
    GSList         *_desktops;
    gchar          *_argv0;

    gboolean       _dialogs_toggle;
    gboolean       _save_preferences;
    gboolean       _use_gui;
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
