/**
 * \brief Object for managing a set of dialogs, including their signals and
 *        construction/caching/destruction of them.
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_MANAGER_H
#define INKSCAPE_UI_DIALOG_MANAGER_H

#include "dialog.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class DialogManager {
public:
    DialogManager();
    virtual ~DialogManager();

    sigc::signal<void> show_dialogs;
    sigc::signal<void> show_f12;
    sigc::signal<void> hide_dialogs;
    sigc::signal<void> hide_f12;
    sigc::signal<void> transientize;

    /* TODO:  Implement these.  These are provided to enable us to 
              dynamically add/remove dialogs at run time (this might
              be useful for extensions that wish to add or replace
              the standard dialogs. */
    /*
    Dialog* getDialog(gchar const* dlgName); 
    void    addDialog(gchar const* dlgName, Dialog * dlg);
    void    deleteDialog(gchar const* dlgName);
    */

    Dialog* getAboutDialog();
    Dialog* getAlignAndDistributeDialog();
    Dialog* getInkscapePreferencesDialog();
    Dialog* getDocumentPreferencesDialog();
    Dialog* getDebugDialog();
    Dialog* getExportDialog();
    Dialog* getExtensionEditorDialog();
    Dialog* getFillAndStrokeDialog();
    Dialog* getFindDialog();
    Dialog* getLayerEditorDialog();
    Dialog* getMessagesDialog();
    Dialog* getObjectPropertiesDialog();
    Dialog* getTextPropertiesDialog();
    Dialog* getTraceDialog();
    Dialog* getTransformDialog();
    Dialog* getTransformationDialog();
    Dialog* getXmlEditorDialog();

protected:
    DialogManager(DialogManager const &d);
    DialogManager& operator=(DialogManager const &d);

    /* map<gchar const*, Dialog*>    _dialog_map; */

    Dialog            *_about_dialog;
    Dialog            *_align_and_distribute_dialog;
    Dialog            *_inkscape_preferences_dialog;
    Dialog            *_debug_dialog;
    Dialog            *_document_preferences_dialog;
    Dialog            *_export_dialog;
    Dialog            *_extension_editor_dialog;
    Dialog            *_fill_and_stroke_dialog;
    Dialog            *_find_dialog;
    Dialog            *_layer_editor_dialog;
    Dialog            *_messages_dialog;
    Dialog            *_object_properties_dialog;
    Dialog            *_text_properties_dialog;
    Dialog            *_trace_dialog;
    Dialog            *_transformation_dialog;
    Dialog            *_xml_editor_dialog;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_UI_DIALOG_MANAGER_H

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
