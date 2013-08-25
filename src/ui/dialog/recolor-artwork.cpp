/**
 * @file
 * Recolor Artwork dialog - implementation.
 *
 */

#include "ui/widget/notebook-page.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "fill-and-stroke.h"
#include "filter-chemistry.h"
#include "inkscape.h"
#include "selection.h"
#include "preferences.h"
#include "style.h"
#include "svg/css-ostringstream.h"
#include "ui/icon-names.h"
#include "verbs.h"
#include "widgets/fill-style.h"
#include "widgets/icon.h"
#include "widgets/paint-selector.h"
#include "widgets/stroke-style.h"
#include "xml/repr.h"

#include "ui/view/view-widget.h"

#include <gtkmm/table.h>

#include "recolor-artwork.h"
#include "recolor-artwork-widget.h"


namespace Inkscape {
namespace UI {
namespace Dialog {

RecolorArtwork::RecolorArtwork()
    : UI::Widget::Panel ("", "/dialogs/recolorartwork", SP_VERB_DIALOG_RECOLOR_ARTWORK),
	_pageRC(Gtk::manage(new UI::Widget::NotebookPage(1, 1, true, true))),
	_composite_settings(SP_VERB_DIALOG_RECOLOR_ARTWORK, "recolorartwork", UI::Widget::SimpleFilterModifier::BLUR),
    deskTrack(),
    targetDesktop(0),
	recolorWdgt(0),
    desktopChangeConn()
{
    Gtk::Box *contents = _getContents();
    contents->set_spacing(2);

    contents->pack_start(_notebook, true, true);

    _notebook.append_page(*_pageRC, _createPageTabLabel(_("_Recolor-Artwork"), INKSCAPE_ICON("object-fill")));	/////////VERRY IMPORTANT !!!
    
    //Purpose yet unknown.
	_notebook.signal_switch_page().connect(sigc::mem_fun(this, &RecolorArtwork::_onSwitchPage));

    _layoutPageRC();
    
    contents->pack_start(_composite_settings, true, true, 0);

    show_all_children();

    _composite_settings.setSubject(&_subject);

    // Connect this up last
    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &RecolorArtwork::setTargetDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));
}

RecolorArtwork::~RecolorArtwork()
{
    _composite_settings.setSubject(NULL);

    desktopChangeConn.disconnect();
    deskTrack.disconnect();
}

void RecolorArtwork::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    deskTrack.setBase(desktop);
}

void RecolorArtwork::setTargetDesktop(SPDesktop *desktop)
{
    if (targetDesktop != desktop) {
        targetDesktop = desktop;
		if (recolorWdgt) {
            recolor_artwork_widget_set_desktop(recolorWdgt, desktop);
			//std::cout<<"OOO";
        }
		/*if (recolorWdgt) {
            //####		
			//Notes: This function basically, passes the recolorWdgt to the current desktop and the rest is handled by fillWdgt.
			//sp_recolor_artwork_widget_set_desktop(fillWdgt, desktop); //IMPORTANT FUNCTION NAME !!
			//####
        }*/
        _composite_settings.setSubject(&_subject);
    }
}

#if WITH_GTKMM_3_0
void RecolorArtwork::_onSwitchPage(Gtk::Widget * /*page*/, guint pagenum)
#else
void RecolorArtwork::_onSwitchPage(GtkNotebookPage * /*page*/, guint pagenum)
#endif
{
    _savePagePref(pagenum);
}

void
RecolorArtwork::_savePagePref(guint page_num)
{
    // remember the current page
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/dialogs/recolorartwork/page", page_num);
}

void
RecolorArtwork::_layoutPageRC()
{
    recolorWdgt = manage(/*sp_recolor_artwork_widget_new()*/recolor_artwork_widget_new());	//function name important
	//####
#if WITH_GTKMM_3_0
    _pageRC->table().attach(*recolorWdgt, 0, 0, 1, 1);
#else
    _pageRC->table().attach(*recolorWdgt, 0, 1, 0, 1);
#endif
	//####
    //Syntax: .attach(child,left,right,top,bottom)
	//Original:
	//_pageRC.table().attach(*recolorWdgt, 0, 1, 0, 1);
}

//Respective codes for stroke and other etc are not included. Look in fill-and-stroke.cpp 

void
RecolorArtwork::showPageRC()
{
    present();
    _notebook.set_current_page(0);
    _savePagePref(0);				///Understand this bit of code.
}

Gtk::HBox&
RecolorArtwork::_createPageTabLabel(const Glib::ustring& label, const char *label_image)
{
    Gtk::HBox *_tab_label_box = manage(new Gtk::HBox(false, 4));
    _tab_label_box->pack_start(*Glib::wrap(sp_icon_new(Inkscape::ICON_SIZE_DECORATION,
                                                       label_image)));

    Gtk::Label *_tab_label = manage(new Gtk::Label(label, true));
    _tab_label_box->pack_start(*_tab_label);
    _tab_label_box->show_all();

    return *_tab_label_box;
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