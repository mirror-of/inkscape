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

#include "EcmaBinding.h"

namespace Inkscape {

class SVGViewer : public Gtk::ScrolledWindow
{
public:

    /**
     *  Sets the already-parsed document with the SVG to be displayed.
     *  Normally called from within Inkscape.
     */
    bool setDocument(SPDocument *doc);

    /**
     *  Sets the URI of the SVG to be displayed.  Normally called
     *  by user-written code.
     */
    bool setURI(URI &uri);

    /*
     *  Constructor
     */
    SVGViewer();

    /*
     *  Destructor
     */
    virtual ~SVGViewer();

protected:

    /**
     * Trace messages
     */
    void trace(char *fmt, ...);

    /**
     * Error messages
     */
    void error(char *fmt, ...);


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

private:

    /*
     *
     */
    EcmaBinding *engine;

};


} // namespace Inkscape

#endif // __SVG_VIEWER_H__


