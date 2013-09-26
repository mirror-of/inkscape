/** @file
 * @brief Recolor Artwork dialog
*/
 
#ifndef INKSCAPE_UI_DIALOG_RECOLOR_ARTWORK_H
#define INKSCAPE_UI_DIALOG_RECOLOR_ARTWORK_H
 
//----------------------------------------
//No knowledge of any other .h right now.

#include "ui/widget/panel.h"
#include "ui/widget/object-composite-settings.h"
#include "ui/dialog/desktop-tracker.h"

#include <gtkmm/notebook.h>
#include "ui/widget/style-subject.h"
//-----------------------------------------

namespace Inkscape {
namespace UI {

namespace Widget {
class NotebookPage;
}

namespace Dialog {

class RecolorArtwork : public UI::Widget::Panel {
public:
  RecolorArtwork();
  virtual ~RecolorArtwork();

  static RecolorArtwork &getInstance() { return *new RecolorArtwork(); }


  virtual void setDesktop(SPDesktop *desktop);

  void selectionChanged(Inkscape::Application *inkscape,
                          Inkscape::Selection *selection);
						  
	void showPageRC();		//Right now named as Page because only one Page to show.

protected:
  Gtk::Notebook _notebook;

  UI::Widget::NotebookPage *_pageRC;
    
  UI::Widget::StyleSubject::Selection  _subject;
  UI::Widget::ObjectCompositeSettings  _composite_settings;

	Gtk::HBox &_createPageTabLabel(const Glib::ustring &label,
                                   const char *label_image);

  void _layoutPageRC();
	void _savePagePref(guint page_num);

#if WITH_GTKMM_3_0
    void _onSwitchPage(Gtk::Widget *page, guint pagenum);
#else
    void _onSwitchPage(GtkNotebookPage *page, guint pagenum);
#endif
	
private:
    RecolorArtwork(RecolorArtwork const &d);
    RecolorArtwork& operator=(RecolorArtwork const &d);

    void setTargetDesktop(SPDesktop *desktop);

    DesktopTracker deskTrack;
    SPDesktop *targetDesktop;
    Gtk::Widget *recolorWdgt;
	  //name of main widget responsible for chnages.
	   sigc::connection desktopChangeConn;

};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape


#endif // INKSCAPE_UI_DIALOG_RECOLOR_ARTWORK_H

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
