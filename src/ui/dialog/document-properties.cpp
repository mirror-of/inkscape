/** @file
 * @brief Document properties dialog, Gtkmm-style
 */
/* Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2006-2008 Johan Engelen  <johan@shouraizou.nl>
 * Copyright (C) 2000 - 2008 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "config.h"

#include <gtkmm.h>

#if ENABLE_LCMS
#include <lcms.h>
#endif // ENABLE_LCMS

#include "display/canvas-grid.h"
#include "document-properties.h"
#include "document.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "guides.h"
#include "helper/units.h"
#include "inkscape.h"
#include "io/sys.h"
#include "preferences.h"
#include "sp-guide.h"
#include "sp-namedview.h"
#include "sp-object-repr.h"
#include "sp-root.h"
#include "sp-script.h"
#include "ui/widget/color-picker.h"
#include "ui/widget/scalar-unit.h"
#include "verbs.h"
#include "widgets/icon.h"
#include "xml/node-event-vector.h"
#include "xml/repr.h"

#if ENABLE_LCMS
#include "color-profile.h"
#endif // ENABLE_LCMS

using std::pair;

namespace Inkscape {
namespace UI {
namespace Dialog {

#define SPACE_SIZE_X 15
#define SPACE_SIZE_Y 10


//===================================================

//---------------------------------------------------

static void on_child_added(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void * data);
static void on_child_removed(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void * data);
static void on_repr_attr_changed (Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer);

static Inkscape::XML::NodeEventVector const _repr_events = {
    on_child_added, // child_added
    on_child_removed, // child_removed
    on_repr_attr_changed,
    NULL, // content_changed
    NULL  // order_changed
};


DocumentProperties & DocumentProperties::getInstance()
{
    DocumentProperties & instance = *new DocumentProperties();
    instance._init();

    return instance;
}

DocumentProperties::DocumentProperties()
    : UI::Widget::Panel ("", "/dialogs/documentoptions", SP_VERB_DIALOG_NAMEDVIEW),
      _page_page(1, 1, true, true),
      _guides_page(getDesktop(), _widget_registry),
      _snap_page(1, 1),
      _color_management_page(1, 1),
      _scripting_page(1, 1),
    //---------------------------------------------------------------
      _show_border_checkbox(_("Show page _border"), _("If set, rectangular page border is shown"), "showborder", _widget_registry, false),
      _border_on_top_checkbox(_("Border on _top of drawing"), _("If set, border is always on top of the drawing"), "borderlayer", _widget_registry, false),
      _show_border_shadow_checkbox(_("_Show border shadow"), _("If set, page border shows a shadow on its right and lower side"), "inkscape:showpageshadow", _widget_registry, false),
      _background_color_picker(_("Back_ground:"), _("Background color"), _("Color and transparency of the page background (also used for bitmap export)"), "pagecolor", "inkscape:pageopacity", _widget_registry),
      _border_color_picker(_("Border _color:"), _("Page border color"), _("Color of the page border"), "bordercolor", "borderopacity", _widget_registry),
      _default_unit_menu(_("Default _units:"), "inkscape:document-units", _widget_registry),
      _page_sizer(_widget_registry),
    //---------------------------------------------------------------
      _create_grid_label("", Gtk::ALIGN_LEFT),
      _create_grid_button(Q_("Grid|_New"), _("Create new grid.")),
      _remove_grid_button(_("_Remove"), _("Remove selected grid.")),
      _defined_grids_label("", Gtk::ALIGN_LEFT)
    //---------------------------------------------------------------
{
    _tt.enable();
    _getContents()->set_spacing (4);
    _getContents()->pack_start(_notebook, true, true);

    _notebook.append_page(_page_page,             _("Page"));
    _notebook.append_page(_guides_page,           _("Guides"));
    _notebook.append_page(_grids_page,            _("Grids"));
    _notebook.append_page(_snap_page,             _("Snap"));
    _notebook.append_page(_color_management_page, _("Color Management"));
    _notebook.append_page(_scripting_page,        _("Scripting"));

    _buildPagePage();
    _buildGridsPage();
    _buildSnapPage();
#if ENABLE_LCMS
    _buildCmsPage();
#endif // ENABLE_LCMS
    _buildScriptingPage();

    _create_grid_button.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::_onNewGrid));
    _remove_grid_button.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::_onRemoveGrid));

    signalDocumentReplaced().connect(sigc::mem_fun(*this, &DocumentProperties::_handleDocumentReplaced));
    signalActivateDesktop().connect(sigc::mem_fun(*this, &DocumentProperties::_handleActivateDesktop));
    signalDeactiveDesktop().connect(sigc::mem_fun(*this, &DocumentProperties::_handleDeactivateDesktop));
}

void DocumentProperties::_init()
{
    update();

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(getDesktop()));
    repr->addListener (&_repr_events, this);
    Inkscape::XML::Node *root = SP_OBJECT_REPR(sp_desktop_document(getDesktop())->root);
    root->addListener (&_repr_events, this);

    show_all_children();
    _remove_grid_button.hide();
}

DocumentProperties::~DocumentProperties()
{
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(getDesktop()));
    repr->removeListenerByData (this);
    Inkscape::XML::Node *root = SP_OBJECT_REPR(sp_desktop_document(getDesktop())->root);
    root->removeListenerByData (this);
}

//========================================================================

/**
 * Helper function that attaches widgets in a 3xn table. The widgets come in an
 * array that has two entries per table row. The two entries code for four
 * possible cases: (0,0) means insert space in first column; (0, non-0) means
 * widget in columns 2-3; (non-0, 0) means label in columns 1-3; and
 * (non-0, non-0) means two widgets in columns 2 and 3.
**/
inline void attach_all(Gtk::Table &table, Gtk::Widget *const arr[], unsigned const n, int start = 0)
{
    for (unsigned i = 0, r = start; i < n; i += 2)
    {
        if (arr[i] && arr[i+1])
        {
            table.attach(*arr[i],   1, 2, r, r+1,
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            table.attach(*arr[i+1], 2, 3, r, r+1,
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        }
        else
        {
            if (arr[i+1]) {
                Gtk::AttachOptions yoptions = (Gtk::AttachOptions)0;
                if (dynamic_cast<Inkscape::UI::Widget::PageSizer*>(arr[i+1])) {
                    // only the PageSizer in Document Properties|Page should be stretched vertically
                    yoptions = Gtk::FILL|Gtk::EXPAND;
                }
                table.attach(*arr[i+1], 1, 3, r, r+1,
                      Gtk::FILL|Gtk::EXPAND, yoptions, 0,0);
            }
            else if (arr[i])
            {
                Gtk::Label& label = reinterpret_cast<Gtk::Label&>(*arr[i]);
                label.set_alignment (0.0);
                table.attach (label, 0, 3, r, r+1,
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            }
            else
            {
                Gtk::HBox *space = manage (new Gtk::HBox);
                space->set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);
                table.attach (*space, 0, 1, r, r+1,
                      (Gtk::AttachOptions)0, (Gtk::AttachOptions)0,0,0);
            }
        }
        ++r;
    }
}

void DocumentProperties::_buildPagePage()
{
    _page_page.show();

    Gtk::Label* label_gen = manage (new Gtk::Label);
    label_gen->set_markup (_("<b>General</b>"));
    Gtk::Label* label_bor = manage (new Gtk::Label);
    label_bor->set_markup (_("<b>Border</b>"));
    Gtk::Label *label_for = manage (new Gtk::Label);
    label_for->set_markup (_("<b>Page Size</b>"));
    _page_sizer.init();

    Gtk::Widget *const widget_array[] =
    {
        label_gen,         0,
        0,                 &_default_unit_menu,
        _background_color_picker._label,    &_background_color_picker,
        0,                 0,
        label_for,         0,
        0,                 &_page_sizer,
        0,                 0,
        label_bor,         0,
        0,                 &_show_border_checkbox,
        0,                 &_border_on_top_checkbox,
        0,                 &_show_border_shadow_checkbox,
        _border_color_picker._label,  &_border_color_picker,
    };

    attach_all(_page_page.table(), widget_array, G_N_ELEMENTS(widget_array));
}

void DocumentProperties::_buildSnapPage()
{
    _snap_page.show();

    _snap_distance_controls.init(_("Snap _distance"), _("Snap only when _closer than:"), _("Always snap"),
                  _("Snapping distance, in screen pixels, for snapping to objects"), _("Always snap to objects, regardless of their distance"),
                  _("If set, objects only snap to another object when it's within the range specified below"),
                  "objecttolerance", _widget_registry);

    _grid_snap_distance_controls.init(_("Snap d_istance"), _("Snap only when c_loser than:"), _("Always snap"),
                  _("Snapping distance, in screen pixels, for snapping to grid"), _("Always snap to grids, regardless of the distance"),
                  _("If set, objects only snap to a grid line when it's within the range specified below"),
                  "gridtolerance", _widget_registry);

    _guide_snap_distance_controls.init(_("Snap dist_ance"), _("Snap only when close_r than:"), _("Always snap"),
                _("Snapping distance, in screen pixels, for snapping to guides"), _("Always snap to guides, regardless of the distance"),
                _("If set, objects only snap to a guide when it's within the range specified below"),
                "guidetolerance", _widget_registry);

    Gtk::Label *label_o = manage (new Gtk::Label);
    label_o->set_markup (_("<b>Snap to objects</b>"));
    Gtk::Label *label_gr = manage (new Gtk::Label);
    label_gr->set_markup (_("<b>Snap to grids</b>"));
    Gtk::Label *label_gu = manage (new Gtk::Label);
    label_gu->set_markup (_("<b>Snap to guides</b>"));

    Gtk::Widget *const array[] =
    {
        label_o,            0,
        0,                  _snap_distance_controls._vbox,
        0,                  0,
        label_gr,           0,
        0,                  _grid_snap_distance_controls._vbox,
        0,                  0,
        label_gu,           0,
        0,                  _guide_snap_distance_controls._vbox
    };

    attach_all(_snap_page.table(), array, G_N_ELEMENTS(array));
 }

#if ENABLE_LCMS
static void lcms_profile_get_name (cmsHPROFILE   profile, const gchar **name)
{
  if (profile)
    {
      *name = cmsTakeProductDesc (profile);

      if (! *name)
        *name = cmsTakeProductName (profile);

      if (*name && ! g_utf8_validate (*name, -1, NULL))
        *name = _("(invalid UTF-8 string)");
    }
  else
    {
      *name = _("None");
    }
}

void DocumentProperties::_populateAvailableProfiles(){
    Glib::ListHandle<Gtk::Widget*> children = _available_color_profiles_menu.get_children();
    for ( Glib::ListHandle<Gtk::Widget*>::iterator it2 = children.begin(); it2 != children.end(); ++it2 ) {
        _available_color_profiles_menu.remove(**it2);
        delete(*it2);
    }

    std::list<Glib::ustring> sources = ColorProfile::getProfileDirs();

    // Use this loop to iterate through a list of possible document locations.
    for ( std::list<Glib::ustring>::const_iterator it = sources.begin(); it != sources.end(); ++it ) {
        if ( Inkscape::IO::file_test( it->c_str(), G_FILE_TEST_EXISTS )
             && Inkscape::IO::file_test( it->c_str(), G_FILE_TEST_IS_DIR )) {
            GError *err = 0;
            GDir *directory = g_dir_open(it->c_str(), 0, &err);
            if (!directory) {
                gchar *safeDir = Inkscape::IO::sanitizeString(it->c_str());
                g_warning(_("Color profiles directory (%s) is unavailable."), safeDir);
                g_free(safeDir);
            } else {
                gchar *filename = 0;
                while ((filename = (gchar *)g_dir_read_name(directory)) != NULL) {
                    gchar* full = g_build_filename(it->c_str(), filename, NULL);
                    if ( !Inkscape::IO::file_test( full, G_FILE_TEST_IS_DIR ) ) {
                        cmsErrorAction( LCMS_ERROR_SHOW );
                        cmsHPROFILE hProfile = cmsOpenProfileFromFile(full, "r");
                        if (hProfile != NULL){
                            const gchar* name;
                            lcms_profile_get_name(hProfile, &name);
                            Gtk::MenuItem* mi = manage(new Gtk::MenuItem());
                            mi->set_data("filepath", g_strdup(full));
                            mi->set_data("name", g_strdup(name));
                            Gtk::HBox *hbox = manage(new Gtk::HBox());
                            hbox->show();
                            Gtk::Label* lbl = manage(new Gtk::Label(name));
                            lbl->show();
                            hbox->pack_start(*lbl, true, true, 0);
                            mi->add(*hbox);
                            mi->show_all();
                            _available_color_profiles_menu.append(*mi);
        //                    g_free((void*)name);
                            cmsCloseProfile(hProfile);
                        }
                    }
                    g_free(full);
                }
                g_dir_close(directory);
            }
        }
    }
    _available_color_profiles_menu.show_all();
}

/**
 * Cleans up name to remove disallowed characters.
 * Some discussion at http://markmail.org/message/bhfvdfptt25kgtmj
 * Allowed ASCII first characters:  ':', 'A'-'Z', '_', 'a'-'z'
 * Allowed ASCII remaining chars add: '-', '.', '0'-'9', 
 *
 * @param str the string to clean up.
 */
static void sanitizeName( Glib::ustring& str )
{
    if (str.size() > 1) {
        char val = str.at(0);
        if (((val < 'A') || (val > 'Z'))
            && ((val < 'a') || (val > 'z'))
            && (val != '_')
            && (val != ':')) {
            str.replace(0, 1, "-");
        }
        for (Glib::ustring::size_type i = 1; i < str.size(); i++) {
            char val = str.at(i);
            if (((val < 'A') || (val > 'Z'))
                && ((val < 'a') || (val > 'z'))
                && ((val < '0') || (val > '9'))
                && (val != '_')
                && (val != ':')
                && (val != '-')
                && (val != '.')) {
                str.replace(i, 1, "-");
            }
        }
    }
}

/**
 * Store this profile in the SVG document (create <color-profile> element in the XML)
 */
void DocumentProperties::_linkSelectedProfile()
{
    // TODO remove use of 'active' desktop
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop){
        g_warning("No active desktop");
    } else {
        if (!_available_color_profiles_menu.get_active()){
            g_warning("No color profile available.");
            return;
        }
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
        Inkscape::XML::Node *cprofRepr = xml_doc->createElement("svg:color-profile");
        gchar* tmp = static_cast<gchar*>(_available_color_profiles_menu.get_active()->get_data("name"));
        Glib::ustring nameStr = tmp ? tmp : "profile"; // TODO add some auto-numbering to avoid collisions
        sanitizeName(nameStr);
        cprofRepr->setAttribute("name", nameStr.c_str());
        cprofRepr->setAttribute("xlink:href", (gchar*) _available_color_profiles_menu.get_active()->get_data("filepath"));

        // Checks whether there is a defs element. Creates it when needed
        Inkscape::XML::Node *defsRepr = sp_repr_lookup_name(xml_doc, "svg:defs");
        if (!defsRepr){
            defsRepr = xml_doc->createElement("svg:defs");
            xml_doc->root()->addChild(defsRepr, NULL);
        }

        g_assert(SP_ROOT(desktop->doc()->root)->defs);
        defsRepr->addChild(cprofRepr, NULL);

        // TODO check if this next line was sometimes needed. It being there caused an assertion.
        //Inkscape::GC::release(defsRepr);

        // inform the document, so we can undo
        sp_document_done(desktop->doc(), SP_VERB_EDIT_LINK_COLOR_PROFILE, _("Link Color Profile"));

        _populateLinkedProfilesBox();
    }
}

void DocumentProperties::_populateLinkedProfilesBox()
{
    _linked_color_profile_list_store->clear();
    const GSList *current = sp_document_get_resource_list( SP_ACTIVE_DOCUMENT, "iccprofile" );
    if (current) _linked_color_profiles_xml_observer.set(SP_OBJECT(current->data)->parent);
    while ( current ) {
        SPObject* obj = SP_OBJECT(current->data);
        Inkscape::ColorProfile* prof = reinterpret_cast<Inkscape::ColorProfile*>(obj);
        Gtk::TreeModel::Row row = *(_linked_color_profile_list_store->append());
        row[_linked_color_profile_list_columns.name_column] = prof->name;
//        row[_LinkedProfilesListColumns.previewColumn] = "Color Preview";
        current = g_slist_next(current);
    }
}

void DocumentProperties::_externalScriptsListButtonRelease(GdkEventButton* event)
{
    if((event->type == GDK_BUTTON_RELEASE) && (event->button == 3)) {
        _external_script_list_context_menu.popup(event->button, event->time);
    }
}

void DocumentProperties::_linkedProfilesListButtonRelease(GdkEventButton* event)
{
    if((event->type == GDK_BUTTON_RELEASE) && (event->button == 3)) {
        _linked_color_profile_list_context_menu.popup(event->button, event->time);
    }
}

void DocumentProperties::_cmsCreatePopupMenu(Gtk::Widget& parent, sigc::slot<void> rem)
{
    Gtk::MenuItem* mi = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::REMOVE));
    _linked_color_profile_list_context_menu.append(*mi);
    mi->signal_activate().connect(rem);
    mi->show();
    _linked_color_profile_list_context_menu.accelerate(parent);
}


void DocumentProperties::_scriptingCreatePopupMenu(Gtk::Widget& parent, sigc::slot<void> rem)
{
    Gtk::MenuItem* mi = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::REMOVE));
    _external_script_list_context_menu.append(*mi);
    mi->signal_activate().connect(rem);
    mi->show();
    _external_script_list_context_menu.accelerate(parent);
}

void DocumentProperties::_removeSelectedProfile(){
    Glib::ustring name;
    if(_linked_color_profile_list.get_selection()) {
        Gtk::TreeModel::iterator i = _linked_color_profile_list.get_selection()->get_selected();

        if(i){
            name = (*i)[_linked_color_profile_list_columns.name_column];
        } else {
            return;
        }
    }

    const GSList *current = sp_document_get_resource_list( SP_ACTIVE_DOCUMENT, "iccprofile" );
    while ( current ) {
        SPObject* obj = SP_OBJECT(current->data);
        Inkscape::ColorProfile* prof = reinterpret_cast<Inkscape::ColorProfile*>(obj);
        if (!name.compare(prof->name)){
            sp_repr_unparent(obj->repr);
            sp_document_done(SP_ACTIVE_DOCUMENT, SP_VERB_EDIT_REMOVE_COLOR_PROFILE, _("Remove linked color profile"));
        }
        current = g_slist_next(current);
    }

    _populateLinkedProfilesBox();
}

void
DocumentProperties::_buildCmsPage()
{
    _color_management_page.show();

    Gtk::Label *label_link= manage (new Gtk::Label("", Gtk::ALIGN_LEFT));
    label_link->set_markup (_("<b>Linked Color Profiles:</b>"));
    Gtk::Label *label_avail = manage (new Gtk::Label("", Gtk::ALIGN_LEFT));
    label_avail->set_markup (_("<b>Available Color Profiles:</b>"));

    _link_color_profile_button.set_label(_("Link Profile"));

    _color_management_page.set_spacing(4);
    gint row = 0;

    label_link->set_alignment(0.0);
    _color_management_page.table().attach(*label_link, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    row++;
    _color_management_page.table().attach(_linked_color_profile_list_scroller, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    row++;

    Gtk::HBox* spacer = Gtk::manage(new Gtk::HBox());
    spacer->set_size_request(SPACE_SIZE_X, SPACE_SIZE_Y);
    _color_management_page.table().attach(*spacer, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    row++;

    label_avail->set_alignment(0.0);
    _color_management_page.table().attach(*label_avail, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    row++;
    _color_management_page.table().attach(_available_color_profiles_option_menu, 0, 2, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    _color_management_page.table().attach(_link_color_profile_button, 2, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);

    _populateAvailableProfiles();

    _available_color_profiles_option_menu.set_menu(_available_color_profiles_menu);
    _available_color_profiles_option_menu.set_history(0);
    _available_color_profiles_option_menu.show_all();

    //# Set up the Linked Profiles combo box
    _linked_color_profile_list_store = Gtk::ListStore::create(_linked_color_profile_list_columns);
    _linked_color_profile_list.set_model(_linked_color_profile_list_store);
    _linked_color_profile_list.append_column(_("Profile Name"), _linked_color_profile_list_columns.name_column);
//    _LinkedProfilesList.append_column(_("Color Preview"), _LinkedProfilesListColumns.previewColumn);
    _linked_color_profile_list.set_headers_visible(false);
// TODO restore?    _LinkedProfilesList.set_fixed_height_mode(true);

    _populateLinkedProfilesBox();

    _linked_color_profile_list_scroller.add(_linked_color_profile_list);
    _linked_color_profile_list_scroller.set_shadow_type(Gtk::SHADOW_IN);
    _linked_color_profile_list_scroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _linked_color_profile_list_scroller.set_size_request(-1, 90);

    _link_color_profile_button.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::_linkSelectedProfile));

    _linked_color_profile_list.signal_button_release_event().connect_notify(sigc::mem_fun(*this, &DocumentProperties::_linkedProfilesListButtonRelease));
    _cmsCreatePopupMenu(_linked_color_profile_list, sigc::mem_fun(*this, &DocumentProperties::_removeSelectedProfile));

    const GSList *current = sp_document_get_resource_list( SP_ACTIVE_DOCUMENT, "defs" );
    if (current) {
        _linked_color_profiles_xml_observer.set(SP_OBJECT(current->data)->parent);
    }
    _linked_color_profiles_xml_observer.signal_changed().connect(sigc::mem_fun(*this, &DocumentProperties::_populateLinkedProfilesBox));
}
#endif // ENABLE_LCMS

void
DocumentProperties::_buildScriptingPage()
{
    _scripting_page.show();

    Gtk::Label *label_script= manage (new Gtk::Label("", Gtk::ALIGN_LEFT));
    label_script->set_markup (_("<b>External script files:</b>"));

    _add_external_script_button.set_label(_("Add"));

    _scripting_page.set_spacing(4);
    gint row = 0;

    label_script->set_alignment(0.0);
    _scripting_page.table().attach(*label_script, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    row++;
    _scripting_page.table().attach(_external_script_list_scroller, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    row++;

    Gtk::HBox* spacer = Gtk::manage(new Gtk::HBox());
    spacer->set_size_request(SPACE_SIZE_X, SPACE_SIZE_Y);
    _scripting_page.table().attach(*spacer, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    row++;

    _scripting_page.table().attach(_script_entry, 0, 2, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    _scripting_page.table().attach(_add_external_script_button, 2, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    row++;

    //# Set up the External Scripts box
    _external_script_list_store = Gtk::ListStore::create(_external_script_list_columns);
    _external_script_list.set_model(_external_script_list_store);
    _external_script_list.append_column(_("Filename"), _external_script_list_columns.filename_column);
    _external_script_list.set_headers_visible(true);
// TODO restore?    _ExternalScriptsList.set_fixed_height_mode(true);

    _populateExternalScriptsBox();

    _external_script_list_scroller.add(_external_script_list);
    _external_script_list_scroller.set_shadow_type(Gtk::SHADOW_IN);
    _external_script_list_scroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _external_script_list_scroller.set_size_request(-1, 90);

    _add_external_script_button.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::_addExternalScript));

#if ENABLE_LCMS
    _external_script_list.signal_button_release_event().connect_notify(sigc::mem_fun(*this, &DocumentProperties::_externalScriptsListButtonRelease));
    _scriptingCreatePopupMenu(_external_script_list, sigc::mem_fun(*this, &DocumentProperties::_removeExternalScript));
#endif // ENABLE_LCMS

//TODO: review this observers code:
    const GSList *current = sp_document_get_resource_list( SP_ACTIVE_DOCUMENT, "script" );
    if (current) {
        _external_scripts_xml_observer.set(SP_OBJECT(current->data)->parent);
    }
    _external_scripts_xml_observer.signal_changed().connect(sigc::mem_fun(*this, &DocumentProperties::_populateExternalScriptsBox));
}


void DocumentProperties::_addExternalScript(){
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop){
        g_warning("No active desktop");
    } else {
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
        Inkscape::XML::Node *scriptRepr = xml_doc->createElement("svg:script");
        scriptRepr->setAttribute("xlink:href", (gchar*) _script_entry.get_text().c_str());
        _script_entry.set_text("");

        xml_doc->root()->addChild(scriptRepr, NULL);

        // inform the document, so we can undo
        sp_document_done(desktop->doc(), SP_VERB_EDIT_ADD_EXTERNAL_SCRIPT, _("Add external script..."));

        _populateExternalScriptsBox();
    }
}

void DocumentProperties::_removeExternalScript(){
    Glib::ustring name;
    if(_external_script_list.get_selection()) {
        Gtk::TreeModel::iterator i = _external_script_list.get_selection()->get_selected();

        if(i){
            name = (*i)[_external_script_list_columns.filename_column];
        } else {
            return;
        }
    }

    const GSList *current = sp_document_get_resource_list( SP_ACTIVE_DOCUMENT, "script" );
    while ( current ) {
        SPObject* obj = SP_OBJECT(current->data);
        SPScript* script = (SPScript*) obj;
        if (name == script->xlinkhref){
            sp_repr_unparent(obj->repr);
            sp_document_done(SP_ACTIVE_DOCUMENT, SP_VERB_EDIT_REMOVE_EXTERNAL_SCRIPT, _("Remove external script"));
        }
        current = g_slist_next(current);
    }

    _populateExternalScriptsBox();

}

void DocumentProperties::_populateExternalScriptsBox(){
    _external_script_list_store->clear();
    const GSList *current = sp_document_get_resource_list( SP_ACTIVE_DOCUMENT, "script" );
    if (current) _external_scripts_xml_observer.set(SP_OBJECT(current->data)->parent);
    while ( current ) {
        SPObject* obj = SP_OBJECT(current->data);
        SPScript* script = (SPScript*) obj;
        if (script->xlinkhref)
        {
            Gtk::TreeModel::Row row = *(_external_script_list_store->append());
            row[_external_script_list_columns.filename_column] = script->xlinkhref;
        }

        current = g_slist_next(current);
    }
}

/**
* Called for _updating_ the dialog (e.g. when a new grid was manually added in XML)
*/
void
DocumentProperties::updateGridsPage()
{
    SPDesktop *dt = getDesktop();
    SPNamedView *nv = sp_desktop_namedview(dt);

    //remove all tabs
    while (_grids_notebook.get_n_pages() != 0) {
        _grids_notebook.remove_page(-1); // this also deletes the page.
    }

    //add tabs
    bool grids_present = false;
    for (GSList const * l = nv->grids; l != NULL; l = l->next) {
        Inkscape::CanvasGrid * grid = (Inkscape::CanvasGrid*) l->data;
        if (!grid->repr->attribute("id")) continue; // update_gridspage is called again when "id" is added
        Glib::ustring name(grid->repr->attribute("id"));
        const char *icon = NULL;
        switch (grid->getGridType()) {
            case GRID_RECTANGULAR:
                icon = "grid-rectangular";
                break;
            case GRID_AXONOMETRIC:
                icon = "grid-axonometric";
                break;
            default:
                break;
        }
        _grids_notebook.append_page(*grid->newWidget(), _createPageTabLabel(name, icon));
        grids_present = true;
    }
    _grids_notebook.show_all();

    if (grids_present)
        _remove_grid_button.set_sensitive(true);
    else
        _remove_grid_button.set_sensitive(false);
}

/**
 * Build grid page of dialog.
 */
void
DocumentProperties::_buildGridsPage()
{
    /// \todo FIXME: gray out snapping when grid is off.
    /// Dissenting view: you want snapping without grid.

    SPDesktop *dt = getDesktop();
    SPNamedView *nv = sp_desktop_namedview(dt);
    (void)nv;

    _create_grid_label.set_markup(_("<b>Creation</b>"));
    _defined_grids_label.set_markup(_("<b>Defined grids</b>"));
    _create_grid_bbox.pack_start(_create_grid_type_combo, true, true);
    _create_grid_bbox.pack_start(_create_grid_button, true, true);

    for (gint t = 0; t <= GRID_MAXTYPENR; t++) {
        _create_grid_type_combo.append_text( CanvasGrid::getName( (GridType) t ) );
    }
    _create_grid_type_combo.set_active_text( CanvasGrid::getName(GRID_RECTANGULAR) );

    _grids_spacer.set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);

    _grids_page.set_spacing(4);
    _grids_page.pack_start(_create_grid_label, false, false);
    _grids_page.pack_start(_create_grid_bbox, false, false);
    _grids_page.pack_start(_grids_spacer, false, false);
    _grids_page.pack_start(_defined_grids_label, false, false);
    _grids_page.pack_start(_grids_notebook, false, false);
    _grids_page.pack_start(_remove_grid_button, false, false);

    updateGridsPage();
}



/**
 * Update dialog widgets from desktop. Also call updateWidget routines of the grids.
 */
void
DocumentProperties::update()
{
    if (_widget_registry.isUpdating()) return;

    SPDesktop *dt = getDesktop();
    SPNamedView *nv = sp_desktop_namedview(dt);

    _widget_registry.setUpdating (true);
    set_sensitive (true);

    //-----------------------------------------------------------page page
    _background_color_picker.setRgba32 (nv->pagecolor);
    _show_border_checkbox.setActive (nv->showborder);
    _border_on_top_checkbox.setActive (nv->borderlayer == SP_BORDER_LAYER_TOP);
    _border_color_picker.setRgba32 (nv->bordercolor);
    _show_border_shadow_checkbox.setActive (nv->showpageshadow);

    if (nv->doc_units)
        _default_unit_menu.setUnit (nv->doc_units);

    double const doc_w_px = sp_document_width(sp_desktop_document(dt));
    double const doc_h_px = sp_document_height(sp_desktop_document(dt));
    _page_sizer.setDim (doc_w_px, doc_h_px);
    _page_sizer.updateFitMarginsUI(SP_OBJECT_REPR(nv));

    //-----------------------------------------------------------guide page

    _guides_page.update();

    //-----------------------------------------------------------snap page

    _snap_distance_controls.setValue (nv->snap_manager.snapprefs.getObjectTolerance());
    _grid_snap_distance_controls.setValue (nv->snap_manager.snapprefs.getGridTolerance());
    _guide_snap_distance_controls.setValue (nv->snap_manager.snapprefs.getGuideTolerance());


    //-----------------------------------------------------------grids page

    updateGridsPage();

    //------------------------------------------------Color Management page

#if ENABLE_LCMS
    _populateLinkedProfilesBox();
    _populateAvailableProfiles();
#endif // ENABLE_LCMS

    _widget_registry.setUpdating (false);
}

// TODO: copied from fill-and-stroke.cpp factor out into new ui/widget file?
Gtk::HBox&
DocumentProperties::_createPageTabLabel(const Glib::ustring& label, const char *label_image)
{
    Gtk::HBox *_tab_label_box = manage(new Gtk::HBox(false, 0));
    _tab_label_box->set_spacing(4);
    _tab_label_box->pack_start(*Glib::wrap(sp_icon_new(Inkscape::ICON_SIZE_DECORATION,
                                                       label_image)));

    Gtk::Label *_tab_label = manage(new Gtk::Label(label, true));
    _tab_label_box->pack_start(*_tab_label);
    _tab_label_box->show_all();

    return *_tab_label_box;
}

//--------------------------------------------------------------------

// JGLASSMY: this is unused.  delete it.  it could be registered via Dialog::signal_response() if that were desired.
void
DocumentProperties::_onResponse (int id)
{
    if (id == Gtk::RESPONSE_DELETE_EVENT || id == Gtk::RESPONSE_CLOSE)
    {
        _background_color_picker.closeWindow();
        _border_color_picker.closeWindow();
        _guides_page.close();
    }

    if (id == Gtk::RESPONSE_CLOSE)
        hide();
}

void
DocumentProperties::_handleDocumentReplaced(SPDesktop* desktop, SPDocument *document)
{
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(desktop));
    repr->addListener(&_repr_events, this);
    Inkscape::XML::Node *root = SP_OBJECT_REPR(document->root);
    root->addListener(&_repr_events, this);
    update();
}

void
DocumentProperties::_handleActivateDesktop(Inkscape::Application *, SPDesktop *desktop)
{
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(desktop));
    repr->addListener(&_repr_events, this);
    Inkscape::XML::Node *root = SP_OBJECT_REPR(sp_desktop_document(desktop)->root);
    root->addListener(&_repr_events, this);
    update();
}

void
DocumentProperties::_handleDeactivateDesktop(Inkscape::Application *, SPDesktop *desktop)
{
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(desktop));
    repr->removeListenerByData(this);
    Inkscape::XML::Node *root = SP_OBJECT_REPR(sp_desktop_document(desktop)->root);
    root->removeListenerByData(this);
}

static void
on_child_added(Inkscape::XML::Node */*repr*/, Inkscape::XML::Node */*child*/, Inkscape::XML::Node */*ref*/, void *data)
{
    if (DocumentProperties *dialog = static_cast<DocumentProperties *>(data))
        dialog->updateGridsPage();
}

static void
on_child_removed(Inkscape::XML::Node */*repr*/, Inkscape::XML::Node */*child*/, Inkscape::XML::Node */*ref*/, void *data)
{
    if (DocumentProperties *dialog = static_cast<DocumentProperties *>(data))
        dialog->updateGridsPage();
}



/**
 * Called when XML node attribute changed; updates dialog widgets.
 */
static void
on_repr_attr_changed (Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer data)
{
    if (DocumentProperties *dialog = static_cast<DocumentProperties *>(data))
        dialog->update();
}


/*########################################################################
# BUTTON CLICK HANDLERS    (callbacks)
########################################################################*/

void
DocumentProperties::_onNewGrid()
{
    SPDesktop *dt = getDesktop();
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(dt));
    SPDocument *doc = sp_desktop_document(dt);

    Glib::ustring typestring = _create_grid_type_combo.get_active_text();
    CanvasGrid::writeNewGridToRepr(repr, doc, CanvasGrid::getGridTypeFromName(typestring.c_str()));

    // toggle grid showing to ON:
    dt->showGrids(true);
}

void DocumentProperties::_onRemoveGrid()
{
    gint pagenum = _grids_notebook.get_current_page();
    if (pagenum == -1) // no pages
      return;

    SPDesktop *dt = getDesktop();
    SPNamedView *nv = sp_desktop_namedview(dt);
    Inkscape::CanvasGrid * found_grid = NULL;
    int i = 0;
    for (GSList const * l = nv->grids; l != NULL; l = l->next, i++) {  // not a very nice fix, but works.
        Inkscape::CanvasGrid * grid = (Inkscape::CanvasGrid*) l->data;
        if (pagenum == i) {
            found_grid = grid;
            break; // break out of for-loop
        }
    }
    if (found_grid) {
        // delete the grid that corresponds with the selected tab
        // when the grid is deleted from SVG, the SPNamedview handler automatically deletes the object, so found_grid becomes an invalid pointer!
        found_grid->repr->parent()->removeChild(found_grid->repr);
        sp_document_done(sp_desktop_document(dt), SP_VERB_DIALOG_NAMEDVIEW, _("Remove grid"));
    }
}

DocumentProperties::GuidesPage::GuidesPage(SPDesktop * desktop, UI::Widget::Registry & widget_registry)
    : UI::Widget::NotebookPage(1, 1),
      _desktop(desktop),
      _show_guides_checkbox(_("Show _guides"), _("Show or hide guides"), "showguides", widget_registry),
      _snap_to_guides_checkbox(_("_Snap guides while dragging"), _("While dragging a guide, snap to object nodes or bounding box corners ('Snap to nodes' or 'snap to bounding box corners' must be enabled; only a small part of the guide near the cursor will snap)"), "inkscape:snap-from-guide", widget_registry),
      _guide_color_picker(_("Guide co_lor:"), _("Guideline color"), _("Color of guidelines"), "guidecolor", "guideopacity", widget_registry),
      _highlighted_guide_color_picker(_("_Highlight color:"), _("Highlighted guideline color"), _("Color of a guideline when it is under mouse"), "guidehicolor", "guidehiopacity", widget_registry),
      _delete_all_guides_button(Q_("GuidesPage|_Delete"), _("Delete selected guides.")),
      _create_guides_around_page_button(("Guides around _page"), _("Create a guide aligned with each of the four borders of the document."))
{
    Gtk::Label *guides_label = Gtk::manage(new Gtk::Label);
    guides_label->set_markup(_("<b>Guides</b>"));

    _guide_list_store = Gtk::ListStore::create(_guide_list_columns);
    _guide_list.set_model(_guide_list_store);
    _guide_list.append_column(_("ID"), _guide_list_columns.guide_column);
    _guide_list.get_column(0)->set_visible(false);
    _guide_list.append_column(_("Label"), _guide_list_columns.default_label_column);
    _guide_list.set_headers_visible(true);
    _guide_list.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
    _guide_list_scroller.add(_guide_list);
    _guide_list_scroller.set_shadow_type(Gtk::SHADOW_IN);
    _guide_list_scroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _guide_list_scroller.set_size_request(-1, 150);

    _guide_list.signal_row_activated().connect(sigc::mem_fun(*this, &DocumentProperties::GuidesPage::_activateSelectedGuide));
    _guide_list.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &DocumentProperties::GuidesPage::_updateSelectionStatus));

    _delete_all_guides_button.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::GuidesPage::_deleteSelectedGuides));

    _create_guides_around_page_button.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::GuidesPage::_createGuidesAroundPage));

    Gtk::Widget *const widget_array[] =
    {
        guides_label,                             0,
        0,                                        &_show_guides_checkbox,
        _guide_color_picker._label,               &_guide_color_picker,
        _highlighted_guide_color_picker._label,   &_highlighted_guide_color_picker,
        0,                                        &_snap_to_guides_checkbox,
        _createManagedSpacer(),                   0,
        &_guide_list_scroller,                    0,
        &_delete_all_guides_button,               0,
        _createManagedSpacer(),                   0,
        &_create_guides_around_page_button,       0
    };

    attach_all(table(), widget_array, G_N_ELEMENTS(widget_array));

    _guides_observer.signal_changed().connect(sigc::mem_fun(*this, &DocumentProperties::GuidesPage::_populateGuideList));

    _populateGuideList();
}

void DocumentProperties::GuidesPage::close()
{
    _guide_color_picker.closeWindow();
    _highlighted_guide_color_picker.closeWindow();
}

void DocumentProperties::GuidesPage::update()
{
    SPNamedView const * namedView = sp_desktop_namedview(_desktop);

    if(namedView) {
        _show_guides_checkbox.setActive(namedView->showguides);
        _snap_to_guides_checkbox.setActive(namedView->snap_manager.snapprefs.getSnapModeGuide());

        _guide_color_picker.setRgba32(namedView->guidecolor);
        _highlighted_guide_color_picker.setRgba32(namedView->guidehicolor);
    }

    _populateGuideList();
}

Gtk::HBox * DocumentProperties::GuidesPage::_createManagedSpacer()
{
    Gtk::HBox* spacer = Gtk::manage(new Gtk::HBox());
    spacer->set_size_request(SPACE_SIZE_X, SPACE_SIZE_Y);

    return spacer;
}

void DocumentProperties::GuidesPage::_activateSelectedGuide(Gtk::TreeModel::Path const & path, Gtk::TreeViewColumn * column)
{
    SPGuide * selected_guide = (*(_guide_list_store->get_iter(path)))[_guide_list_columns.guide_column];

    if(selected_guide) {
        Inkscape::UI::Dialogs::GuidelinePropertiesDialog::showDialog(selected_guide, _desktop);
    }
}

void DocumentProperties::GuidesPage::_createGuidesAroundPage()
{
    sp_guide_create_guides_around_page(_desktop);
}

//bool DocumentProperties::GuidesPage::_addGuideAtPath(Gtk::TreePath & path, std::list<SPGuide *> const & guide_list)
//{
//    SPGuide * selected_guide = (*(_guide_list_store->get_iter(path)))[_guide_list_columns.guide_column];
//
//    if(selected_guide) {
//        Inkscape::UI::Dialogs::GuidelinePropertiesDialog::showDialog(selected_guide, _desktop);
//    }
//
//    return true;
//}

void DocumentProperties::GuidesPage::_deleteSelectedGuides()
{
    std::list<SPGuide *> selected_guides;

//    _guide_list.get_selection()->selected_foreach(sigc::bind(sigc::mem_fun(*this, _addGuideAtPath), selected_paths));

    Glib::ListHandle<Gtk::TreeModel::Path, Gtk::TreePath_Traits> selected_paths = _guide_list.get_selection()->get_selected_rows();
    for (Glib::ListHandle<Gtk::TreeModel::Path, Gtk::TreePath_Traits>::iterator selected_path_iterator = selected_paths.begin(); selected_path_iterator != selected_paths.end(); selected_path_iterator++) {
        Gtk::TreeModel::Path path = *selected_path_iterator;

        SPGuide * selected_guide = (*(_guide_list_store->get_iter(path)))[_guide_list_columns.guide_column];

        selected_guides.push_back(selected_guide);
    }

    for (std::list<SPGuide *>::iterator guide_iterator = selected_guides.begin(); guide_iterator != selected_guides.end(); guide_iterator++) {
        sp_guide_remove(*guide_iterator);
    }

    if (! selected_guides.empty()) {
        sp_document_done(sp_desktop_document(_desktop), SP_VERB_DIALOG_NAMEDVIEW, _("Delete selected guide(s)"));
    }
}

void DocumentProperties::GuidesPage::_populateGuideList()
{
    _guide_list_store->clear();

    SPNamedView const * namedView = sp_desktop_namedview(_desktop);
    if (namedView) {
        _guides_observer.set(SP_NAMEDVIEW(namedView));

        GSList const * current = namedView->guides;

        while (current) {
            SPGuide * guide = SP_GUIDE(current->data);

            Gtk::TreeModel::Row newRow = *(_guide_list_store->append());
            newRow[_guide_list_columns.guide_column] = guide;
            newRow[_guide_list_columns.default_label_column] = Glib::ustring(guide->defaultLabel());

            current = g_slist_next(current);
        }
    }

    _updateSelectionStatus();
}

void DocumentProperties::GuidesPage::_updateSelectionStatus()
{
    _delete_all_guides_button.set_sensitive(_guide_list.get_selection() && _guide_list.get_selection()->count_selected_rows() > 0);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
