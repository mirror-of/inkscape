//#########################################################################
//## $Id$
//#########################################################################
#ifndef __SVG_VIEWER_H__
#define __SVG_VIEWER_H__

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gdkmm/colormap.h>

#include <document.h>
#include <uri.h>

namespace Inkscape {

class SVGViewer : public Gtk::ScrolledWindow
{
public:

    /*
     *
     */
    SVGViewer();

    /**
     *
     */
    bool setDocument(SPDocument *doc);

    /**
     *
     */
    bool setURI(URI &uri);

    /*
     *
     */
    virtual ~SVGViewer();

protected:

    //Overridden default signal handlers:

    /*
     *
     */
    virtual void on_realize();

    /*
     *
     */
    virtual bool on_expose_event(GdkEventExpose* e);


    /*
     *
     */
    Glib::RefPtr<Gdk::GC> gc_;

    /*
     *
     */
    Gtk::VBox  mainVBox;
 
    /*
     *
     */
    Gdk::Color blue_, red_;


    /*
     *
     */
    SPDocument *document;

};


} // namespace Inkscape

#endif // __SVG_VIEWER_H__


