/**
 * \brief Object for managing a set of dialogs, including their signals and
 *        construction/caching/destruction of them.
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Jon Phillips <jon@rejon.org>
 *   
 * Copyright (C) 2004 Bryce Harrington
 * Copyright (C) 2005 Jon Phillips
 * 
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ui/dialog/dialog-manager.h"

#include "ui/dialog/align-and-distribute.h"
#include "ui/dialog/inkscape-preferences.h"
#include "ui/dialog/document-preferences.h"
#include "ui/dialog/export.h"
#include "ui/dialog/extension-editor.h"
#include "ui/dialog/fill-and-stroke.h"
#include "ui/dialog/find.h"
#include "ui/dialog/layer-editor.h"
#include "ui/dialog/messages.h"
#include "ui/dialog/text-properties.h"
#include "ui/dialog/transformation.h"
#include "ui/dialog/xml-editor.h"
#include "ui/dialog/memory.h"


namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 *  This class is provided as a container for Inkscape's various
 *  dialogs.  This allows Inkscape::Application to treat the various
 *  dialogs it invokes, as abstractions.  
 *
 *  DialogManager is essentially a cache of dialogs.  It lets us
 *  initialize dialogs lazily - instead of constructing them during
 *  application startup, they're constructed the first time they're
 *  actually invoked by Inkscape::Application.  The constructed
 *  dialog is held here after that, so future invokations of the 
 *  dialog don't need to get re-constructed each time.  The memory for
 *  the dialogs are then reclaimed when the DialogManager is destroyed.
 *
 *  In addition, DialogManager also serves as a signal manager for
 *  dialogs.  It provides a set of signals that can be sent to all
 *  dialogs for doing things such as hiding/unhiding them, etc.  
 *  DialogManager ensures that every dialog it handles will listen
 *  to these signals.
 *
 *  Note that I considered implementing this class a bit differently.
 *  Instead of holding a pointer to each different dialog, I thought
 *  about creating a map of strings or quarks to Dialog pointers.  This
 *  abstraction would have the advantage of letting us iterate over
 *  all the dialogs instead of having to use the long lists of calls.
 *  I decided to do it this way because I don't expect this
 *  code to change that much, other than maybe adding or renaming 
 *  individual dialogs once and a while, but I think the map approach
 *  might have some advantages if we ever wanted to dynamically add
 *  new dialogs to the manager (such as from extensions).  So maybe
 *  some day this should be reimplemented that way.
 *  
 */
DialogManager::DialogManager()
    : _about_dialog(NULL),
      _align_and_distribute_dialog(NULL),
      _inkscape_preferences_dialog(NULL),
      _debug_dialog(NULL),
      _document_preferences_dialog(NULL),
      _export_dialog(NULL),
      _extension_editor_dialog(NULL),
      _fill_and_stroke_dialog(NULL),
      _find_dialog(NULL),
      _layer_editor_dialog(NULL),
      _messages_dialog(NULL),
      _object_properties_dialog(NULL),
      _text_properties_dialog(NULL),
      _trace_dialog(NULL),
      _transformation_dialog(NULL),
      _xml_editor_dialog(NULL),
      _memory_dialog(NULL)
{
}

DialogManager::~DialogManager() 
{
    // TODO:  Disconnect the signals
    // TOOD:  Do we need to explicitly delete the dialogs?
    //        Appears to cause a segfault if we do
}


/**
 * Gets a dilog by dialog name, which is the key to the map which contains
 * a pointer to a dialog.
 */
Dialog* DialogManager::getDialog(gchar const* dlgName) 
{
    return getDialog(g_quark_from_string(dlgName));
}

/**
 * Gets a dilog by dialog name, which is the key to the map which contains
 * a pointer to a dialog.
 */
Dialog* DialogManager::getDialog(GQuark q) 
{
    DialogMap::const_iterator iter = _dialog_map.find(q);
    if (iter != _dialog_map.end()) {
        return (*iter).second; // dialog found
    } else {
        // TODO:  Look up the class providing q and instantiate it
        // This probably needs to be a Factory object or something...
        // or else maybe create a hash of GQuark to new functions for
        // the corresponding class.
        return NULL;    // dialog not found
    }
}

/**
 * Adds a dialog to the map structure and connects it to the standard
 * signals for a dialog.
 */
void DialogManager::addDialog(gchar const* dlgName, Dialog * dlg) 
{
    addDialog(g_quark_from_string(dlgName), dlg);
}

/**
 * Adds a dialog to the map structure and connects it to the standard
 * signals for a dialog.
 */
void DialogManager::addDialog(GQuark dlgName, Dialog * dlg) 
{
    _dialog_map[dlgName] = dlg; 

    hide_dialogs.connect(sigc::mem_fun(*_dialog_map[dlgName],
                                       &Dialog::onHideDialogs));
    hide_f12.connect(sigc::mem_fun(*_dialog_map[dlgName],
                                   &Dialog::onHideF12));
    show_dialogs.connect(sigc::mem_fun(*_dialog_map[dlgName],
                                       &Dialog::onShowDialogs));
    show_f12.connect(sigc::mem_fun(*_dialog_map[dlgName],
                                   &Dialog::onShowF12));
}

/**
 * Deletes a dialog from the map structure and from existence.
 */
bool DialogManager::deleteDialog(gchar const* dlgName) 
{
    return deleteDialog(g_quark_from_string(dlgName));
}

/**
 * Deletes a dialog from the map structure and from existence.
 */
bool DialogManager::deleteDialog(GQuark q) 
{
    DialogMap::iterator iter = _dialog_map.find(q);
    if (iter != _dialog_map.end()) {
        delete (*iter).second;
        _dialog_map.erase(iter);
        return true;
    } else {
        return false;
    }
}

/**
 * Deletes all dialogs
 */
void DialogManager::deleteAllDialogs()
{
    DialogMap::iterator iter = _dialog_map.begin();
    while (iter != _dialog_map.end()) {
        delete (*iter).second;
        ++iter;
    }
    _dialog_map.clear();
}


#if 1==2
Dialog* DialogManager::getAboutDialog() {
    if (_about_dialog == NULL) {
        _about_dialog = About::create();
        addDialog("About", _about_dialog);
    }
    return _about_dialog;
}
#endif

Dialog* DialogManager::getAlignAndDistributeDialog() {
    if (_align_and_distribute_dialog == NULL) {
        _align_and_distribute_dialog = new AlignAndDistribute;
        hide_dialogs.connect(sigc::mem_fun(*_align_and_distribute_dialog, 
                                           &AlignAndDistribute::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_align_and_distribute_dialog, 
                                       &AlignAndDistribute::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_align_and_distribute_dialog, 
                                           &AlignAndDistribute::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_align_and_distribute_dialog, 
                                       &AlignAndDistribute::onShowF12));
    }
    return _align_and_distribute_dialog;
}

Dialog* DialogManager::getInkscapePreferencesDialog() {
    if (_inkscape_preferences_dialog == NULL) {
        _inkscape_preferences_dialog = new InkscapePreferences;
        hide_dialogs.connect(sigc::mem_fun(*_inkscape_preferences_dialog, 
                                           &InkscapePreferences::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_inkscape_preferences_dialog, 
                                       &InkscapePreferences::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_inkscape_preferences_dialog, 
                                           &InkscapePreferences::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_inkscape_preferences_dialog, 
                                       &InkscapePreferences::onShowF12));
    }
    return _inkscape_preferences_dialog;
}


Dialog* DialogManager::getDocumentPreferencesDialog() {
    if (_document_preferences_dialog == NULL) {
        _document_preferences_dialog = new DocumentPreferences;
        hide_dialogs.connect(sigc::mem_fun(*_document_preferences_dialog, 
                                           &DocumentPreferences::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_document_preferences_dialog, 
                                       &DocumentPreferences::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_document_preferences_dialog, 
                                           &DocumentPreferences::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_document_preferences_dialog, 
                                       &DocumentPreferences::onShowF12));
    }
    return _document_preferences_dialog;
}

Dialog* DialogManager::getExportDialog() {
    if (_export_dialog == NULL) {
        _export_dialog = new Export;
        hide_dialogs.connect(sigc::mem_fun(*_export_dialog, 
                                           &Export::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_export_dialog, 
                                       &Export::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_export_dialog, 
                                           &Export::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_export_dialog, 
                                       &Export::onShowF12));
    }
    return _export_dialog;
}

Dialog* DialogManager::getExtensionEditorDialog() {
    if (_extension_editor_dialog == NULL) {
        _extension_editor_dialog = new ExtensionEditor;
        hide_dialogs.connect(sigc::mem_fun(*_extension_editor_dialog, 
                                           &ExtensionEditor::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_extension_editor_dialog, 
                                       &ExtensionEditor::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_extension_editor_dialog, 
                                           &ExtensionEditor::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_extension_editor_dialog, 
                                       &ExtensionEditor::onShowF12));
    }
    return _export_dialog;
}

Dialog* DialogManager::getFillAndStrokeDialog() {
    if (_fill_and_stroke_dialog == NULL) {
        _fill_and_stroke_dialog = new FillAndStroke;
        hide_dialogs.connect(sigc::mem_fun(*_fill_and_stroke_dialog, 
                                           &FillAndStroke::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_fill_and_stroke_dialog, 
                                       &FillAndStroke::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_fill_and_stroke_dialog, 
                                           &FillAndStroke::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_fill_and_stroke_dialog, 
                                       &FillAndStroke::onShowF12));
    }
    return _fill_and_stroke_dialog;
}

Dialog* DialogManager::getFindDialog() {
    if (_find_dialog == NULL) {
        _find_dialog = new Find;
        hide_dialogs.connect(sigc::mem_fun(*_find_dialog, 
                                           &Find::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_find_dialog, 
                                       &Find::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_find_dialog, 
                                           &Find::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_find_dialog, 
                                       &Find::onShowF12));
    }
    return _find_dialog;
}

Dialog* DialogManager::getLayerEditorDialog() {
    if (_layer_editor_dialog == NULL) {
        _layer_editor_dialog = new LayerEditor;
        hide_dialogs.connect(sigc::mem_fun(*_layer_editor_dialog, 
                                           &LayerEditor::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_layer_editor_dialog, 
                                       &LayerEditor::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_layer_editor_dialog, 
                                           &LayerEditor::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_layer_editor_dialog, 
                                       &LayerEditor::onShowF12));
    }
    return _layer_editor_dialog;
}

Dialog* DialogManager::getMessagesDialog() {
    if (_messages_dialog == NULL) {
        _messages_dialog = new Messages;
        hide_dialogs.connect(sigc::mem_fun(*_messages_dialog, 
                                           &Messages::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_messages_dialog, 
                                       &Messages::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_messages_dialog, 
                                           &Messages::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_messages_dialog, 
                                       &Messages::onShowF12));
    }
    return _messages_dialog;
}

Dialog* DialogManager::getObjectPropertiesDialog() {
    if (_object_properties_dialog == NULL) {
/*
        _object_properties_dialog = ObjectProperties::create();
        hide_dialogs.connect(sigc::mem_fun(*_object_properties_dialog, 
                                           &ObjectProperties::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_object_properties_dialog, 
                                       &ObjectProperties::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_object_properties_dialog, 
                                           &ObjectProperties::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_object_properties_dialog, 
                                       &ObjectProperties::onShowF12));
*/
    }
    return _object_properties_dialog;
}

Dialog* DialogManager::getTextPropertiesDialog() {
    if (_text_properties_dialog == NULL) {
        _text_properties_dialog = new TextProperties;
        hide_dialogs.connect(sigc::mem_fun(*_text_properties_dialog, 
                                           &TextProperties::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_text_properties_dialog, 
                                       &TextProperties::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_text_properties_dialog, 
                                           &TextProperties::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_text_properties_dialog, 
                                       &TextProperties::onShowF12));
    }
    return _text_properties_dialog;

}

/*
Dialog* DialogManager::getTraceDialog() {
    if (_trace_dialog == NULL) {
        _trace_dialog = Trace::create();
        hide_dialogs.connect(sigc::mem_fun(*_trace_dialog, 
                                           &Trace::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_trace_dialog, 
                                       &Trace::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_trace_dialog, 
                                           &Trace::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_trace_dialog, 
                                       &Trace::onShowF12));
    }
    return _trace_dialog;
}
*/

Dialog* DialogManager::getTransformationDialog() {
    if (_transformation_dialog == NULL) {
        _transformation_dialog = new Transformation;
        hide_dialogs.connect(sigc::mem_fun(*_transformation_dialog, 
                                           &Transformation::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_transformation_dialog, 
                                       &Transformation::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_transformation_dialog, 
                                           &Transformation::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_transformation_dialog, 
                                       &Transformation::onShowF12));
    }
    return _transformation_dialog;
}

Dialog* DialogManager::getXmlEditorDialog() {
    if (_xml_editor_dialog == NULL) {
        _xml_editor_dialog = new XmlEditor;
        hide_dialogs.connect(sigc::mem_fun(*_xml_editor_dialog, 
                                           &XmlEditor::onHideDialogs));
        hide_f12.connect(sigc::mem_fun(*_xml_editor_dialog, 
                                       &XmlEditor::onHideF12));
        show_dialogs.connect(sigc::mem_fun(*_xml_editor_dialog, 
                                           &XmlEditor::onShowDialogs));
        show_f12.connect(sigc::mem_fun(*_xml_editor_dialog, 
                                       &XmlEditor::onShowF12));
    }
    return _xml_editor_dialog;
}

Dialog *DialogManager::getMemoryDialog() {
    if (_memory_dialog == NULL) {
        _memory_dialog = new Memory;
        addDialog("Memory", _memory_dialog);
    }
    return _memory_dialog;
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
