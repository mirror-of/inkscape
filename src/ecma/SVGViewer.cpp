//#########################################################################
//## $Id$
//#########################################################################
#include "SVGViewer.h"

#include "inkscape.h"
#include "svg-view.h"

namespace Inkscape {

//#########################################################################
//## PUBLIC METHODS
//#########################################################################
bool SVGViewer::setDocument(SPDocument *doc)
{
    if (document)
        sp_document_unref(document);

    sp_document_ref(doc);
    document = doc;

    GtkWidget *viewerGtk = sp_svg_view_widget_new(doc);
    Gtk::Widget *viewerMM = Glib::wrap(viewerGtk);
    mainVBox.pack_start(*viewerMM);

    engine->processDocument(doc);

    viewerMM->show();

    return true;
}


bool SVGViewer::setURI(URI &uri)
{ 
    SPDocument *doc = sp_document_new (uri.toString(), 0, 0);
    if (!doc)
        {
        printf("SVGView: error loading document '%s'\n", uri.toString());
        return false;
        }

    setDocument(doc);

    return true;
}


//#########################################################################
//## EVENTS
//#########################################################################


void SVGViewer::on_realize()
{
  // We need to call the base on_realize()
  Gtk::ScrolledWindow::on_realize();

  // Now we can allocate any additional resources we need
  Glib::RefPtr<Gdk::Window> window = get_window();
  gc_ = Gdk::GC::create(window);
  window->set_background(red_);
  window->clear();
  gc_->set_foreground(blue_);
}

bool SVGViewer::on_expose_event(GdkEventExpose*)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  window->clear();
  //window->draw_line(gc_, 1, 1, 100, 100);
  return true;
}



//#########################################################################
//## CONSTRUCTOR  /  DESTRUCTOR
//#########################################################################

SVGViewer::SVGViewer()
{
    // get_window() would return 0 because the Gdk::Window has not yet been realized
    // So we can only allocate the colors here - the rest will happen in on_realize().
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap ();

    blue_ = Gdk::Color("blue");
    red_ = Gdk::Color("red");

    add(mainVBox);

    mainVBox.show();

    colormap->alloc_color(blue_);
    colormap->alloc_color(red_);

    if (!INKSCAPE)
        inkscape_application_init("");

    //Make our new binding engine
    engine = new EcmaBinding(INKSCAPE);

    document = NULL;
}


SVGViewer::~SVGViewer()
{
    if (document)
        sp_document_unref(document);

    delete engine;
}


}; //namespace Inkscape

//#########################################################################
//## END    OF    FILE
//#########################################################################


