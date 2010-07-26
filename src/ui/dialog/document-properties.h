/** \file
 * \brief  Document Properties dialog
 */
/* Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2006-2008 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H
#define INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H

#include <list>
#include <sigc++/sigc++.h>//
#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include "ui/widget/notebook-page.h"
#include "ui/widget/page-sizer.h"
#include "ui/widget/registered-widget.h"
#include "ui/widget/registry.h"
#include "ui/widget/tolerance-slider.h"
#include "ui/widget/panel.h"

#include "xml/helper-observer.h"

namespace Inkscape {
    namespace UI {
        namespace Dialog {

class DocumentProperties : public UI::Widget::Panel {
public:
    static DocumentProperties & getInstance();
    static void destroy();

    void update();
    void updateGridsPage();

protected:
    class GuidesPage : public UI::Widget::NotebookPage {
    public:
        GuidesPage(SPDesktop * desktop, UI::Widget::Registry & widget_registry);
        void close();
        void update();

    private:
        GuidesPage(GuidesPage const &); // no copy
        GuidesPage & operator=(GuidesPage const &); // no assign

        class GuideListColumns : public Gtk::TreeModel::ColumnRecord {
        public:
            GuideListColumns() {
                add(guide_column);
                add(default_label_column);
            }
            Gtk::TreeModelColumn<SPGuide *> guide_column;
            Gtk::TreeModelColumn<Glib::ustring> default_label_column;
        };

        Gtk::HBox * _createManagedSpacer();

        void _activateSelectedGuide(Gtk::TreeModel::Path const & path, Gtk::TreeViewColumn * column);
        void _createGuidesAroundPage();
        void _deleteSelectedGuides();
        void _populateGuideList();
        void _updateSelectionStatus();

        SPDesktop *                       _desktop;

        UI::Widget::RegisteredCheckButton _show_guides_checkbox;
        UI::Widget::RegisteredCheckButton _snap_to_guides_checkbox;

        UI::Widget::RegisteredColorPicker _guide_color_picker;
        UI::Widget::RegisteredColorPicker _highlighted_guide_color_picker;

        Inkscape::XML::SignalObserver     _guides_observer;

        GuideListColumns                  _guide_list_columns;
        Glib::RefPtr<Gtk::ListStore>      _guide_list_store;
        Gtk::TreeView                     _guide_list;
        Gtk::ScrolledWindow               _guide_list_scroller;

        Gtk::Button                       _delete_all_guides_button;
        Gtk::Button                       _create_guides_around_page_button;
    };

    void  _buildPagePage();
    void  _buildSnapPage();
    void  _buildGridsPage();
#if ENABLE_LCMS
    void  _buildCmsPage();
#endif // ENABLE_LCMS
    void  _buildScriptingPage();
    void  _init();

    virtual void  _onResponse (int);
#if ENABLE_LCMS
    void  _populateAvailableProfiles();
    void  _populateLinkedProfilesBox();
    void  _linkSelectedProfile();
    void  _removeSelectedProfile();
    void  _linkedProfilesListButtonRelease(GdkEventButton * event);
    void  _cmsCreatePopupMenu(Gtk::Widget & parent, sigc::slot<void> rem);
#endif // ENABLE_LCMS

    void _externalScriptsListButtonRelease(GdkEventButton * event);
    void _populateExternalScriptsBox();
    void _addExternalScript();
    void _removeExternalScript();
    void _scriptingCreatePopupMenu(Gtk::Widget & parent, sigc::slot<void> rem);

    void _handleDocumentReplaced(SPDesktop * desktop, SPDocument * document);
    void _handleActivateDesktop(Inkscape::Application * application, SPDesktop * desktop);
    void _handleDeactivateDesktop(Inkscape::Application * application, SPDesktop * desktop);

    Gtk::HBox & _createPageTabLabel(Glib::ustring const & label, char const * label_image);

    Inkscape::XML::SignalObserver     _linked_color_profiles_xml_observer;
    Inkscape::XML::SignalObserver     _external_scripts_xml_observer;

    Gtk::Tooltips                     _tt;

    Gtk::Notebook                     _notebook;

    UI::Widget::NotebookPage          _page_page;
    GuidesPage                        _guides_page;
    Gtk::VBox                         _grids_page;
    UI::Widget::NotebookPage          _snap_page;
    UI::Widget::NotebookPage          _color_management_page;
    UI::Widget::NotebookPage          _scripting_page;

    UI::Widget::Registry              _widget_registry;

    //---------------------------------------------------------------
    UI::Widget::RegisteredCheckButton _show_border_checkbox;
    UI::Widget::RegisteredCheckButton _border_on_top_checkbox;
    UI::Widget::RegisteredCheckButton _show_border_shadow_checkbox;
    UI::Widget::RegisteredColorPicker _background_color_picker;
    UI::Widget::RegisteredColorPicker _border_color_picker;
    UI::Widget::RegisteredUnitMenu    _default_unit_menu;
    UI::Widget::PageSizer             _page_sizer;
    //---------------------------------------------------------------
    Gtk::Notebook                     _grids_notebook;
    Gtk::HBox                         _create_grid_bbox;
    Gtk::Label                        _create_grid_label;
    Gtk::Button                       _create_grid_button;
    Gtk::Button                       _remove_grid_button;
    Gtk::ComboBoxText                 _create_grid_type_combo;
    Gtk::Label                        _defined_grids_label;
    Gtk::HBox                         _grids_spacer;
    //---------------------------------------------------------------
    UI::Widget::ToleranceSlider       _snap_distance_controls;
    UI::Widget::ToleranceSlider       _grid_snap_distance_controls;
    UI::Widget::ToleranceSlider       _guide_snap_distance_controls;
    //---------------------------------------------------------------
    Gtk::Menu                         _available_color_profiles_menu;
    Gtk::OptionMenu                   _available_color_profiles_option_menu;
    Gtk::Button                       _link_color_profile_button;
    class LinkedColorProfileListColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            LinkedColorProfileListColumns()
               { add(name_column); add(preview_column);  }
            Gtk::TreeModelColumn<Glib::ustring> name_column;
            Gtk::TreeModelColumn<Glib::ustring> preview_column;
        };
    LinkedColorProfileListColumns     _linked_color_profile_list_columns;
    Glib::RefPtr<Gtk::ListStore>      _linked_color_profile_list_store;
    Gtk::TreeView                     _linked_color_profile_list;
    Gtk::ScrolledWindow               _linked_color_profile_list_scroller;
    Gtk::Menu                         _linked_color_profile_list_context_menu;
    //---------------------------------------------------------------
    Gtk::Button                       _add_external_script_button;
    class ExternalScriptsColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            ExternalScriptsColumns()
               { add(filename_column); }
            Gtk::TreeModelColumn<Glib::ustring> filename_column;
        };
    ExternalScriptsColumns            _external_script_list_columns;
    Glib::RefPtr<Gtk::ListStore>      _external_script_list_store;
    Gtk::TreeView                     _external_script_list;
    Gtk::ScrolledWindow               _external_script_list_scroller;
    Gtk::Menu                         _external_script_list_context_menu;
    Gtk::Entry                        _script_entry;

private:
    DocumentProperties();
    virtual ~DocumentProperties();

    // callback methods for buttons on grids page.
    void _onNewGrid();
    void _onRemoveGrid();
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
