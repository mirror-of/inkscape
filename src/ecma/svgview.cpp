//#########################################################################
//## $Id$
//#########################################################################
#include "SVGViewer.h"
#include <gtkmm/main.h>
#include <gtkmm/window.h>

int main(int argc, char** argv)
{
   if (argc!=2)
       {
       printf("usage: svgview <svgfile>\n");
       return 1;
       }

   Gtk::Main kit(argc, argv);

   Gtk::Window win;

   Inkscape::SVGViewer area;
   Inkscape::URI uri(argv[1]);
   area.setURI(uri);
   win.add(area);
   area.show();

   Gtk::Main::run(win);

   return 0;
}


//#########################################################################
//## END    OF    FILE
//#########################################################################
