
#include <glibmm/ustring.h>

#include <view.h>
#include <document.h>
#include <desktop.h>
#include <selection.h>
#include <xml/repr.h>
#include <svg/ftos.h>

#include <extension/implementation/implementation.h>

namespace Inkscape {
namespace Extension {
namespace Plugin {

/** \brief  Implementation class of the GIMP gradient plugin.  This mostly
            just creates a namespace for the GIMP gradient plugin today.
*/
class Grid : public Inkscape::Extension::Implementation::Implementation {

public:
    bool load(Inkscape::Extension::Extension *module);
    void effect(Inkscape::Extension::Effect *module, SPView *document);
};

/**
    \brief  A function to allocated anything -- just an example here
    \param  module  Unused
    \return Whether the load was sucessful
*/
bool
Grid::load (Inkscape::Extension::Extension *module)
{
    std::cout << "Hey, I'm Grid, I'm loading!" << std::endl;
    return TRUE;
}

/**
    \brief  This actually draws the grid.
    \param  module   The effect that was called (unused)
    \param  document What should be edited.
*/
void
Grid::effect (Inkscape::Extension::Effect *module, SPView *document)
{
    SPSelection * selection     = ((SPDesktop *)document)->selection;

    NR::Rect bounding_area = NR::Rect(NR::Point(0,0), NR::Point(100,100));
    if (selection->isEmpty()) {
        /* get page size */
    } else {
        bounding_area = selection->bounds();
    }


    float spacing = 10.0;
    float line_width = 1.0;

    Glib::ustring path_data;

    for (NR::Point start_point = bounding_area.min();
            start_point[NR::X] <= (bounding_area.max())[NR::X];
            start_point[NR::X] += spacing) {
        NR::Point end_point = start_point;
        end_point[NR::Y] = (bounding_area.max())[NR::Y];
        gchar floatstring[64];

        path_data += "M ";
        sprintf(floatstring, "%f", start_point[NR::X]);
        path_data += floatstring;
        path_data += " ";
        sprintf(floatstring, "%f", start_point[NR::Y]);
        path_data += floatstring;
        path_data += " L ";
        sprintf(floatstring, "%f", end_point[NR::X]);
        path_data += floatstring;
        path_data += " ";
        sprintf(floatstring, "%f", end_point[NR::Y]);
        path_data += floatstring;
        path_data += " ";
    }

    for (NR::Point start_point = bounding_area.min();
            start_point[NR::Y] <= (bounding_area.max())[NR::Y];
            start_point[NR::Y] += spacing) {
        NR::Point end_point = start_point;
        end_point[NR::X] = (bounding_area.max())[NR::X];
        gchar floatstring[64];

        path_data += "M ";
        sprintf(floatstring, "%f", start_point[NR::X]);
        path_data += floatstring;
        path_data += " ";
        sprintf(floatstring, "%f", start_point[NR::Y]);
        path_data += floatstring;
        path_data += " L ";
        sprintf(floatstring, "%f", end_point[NR::X]);
        path_data += floatstring;
        path_data += " ";
        sprintf(floatstring, "%f", end_point[NR::Y]);
        path_data += floatstring;
        path_data += " ";
    }

    // std::cout << "Path Data: " << path_data << std::endl;

    SPRepr * current_layer = SP_OBJECT_REPR(((SPDesktop *)document)->currentLayer());
    SPRepr * path = sp_repr_new("svg:path");

    sp_repr_set_attr(path, "d", path_data.c_str());

    Glib::ustring style("fill:none;fill-opacity:0.75000000;fill-rule:evenodd;stroke:#000000;stroke-width:1.0000000pt;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000");
    sp_repr_set_attr(path, "style", style.c_str());

    sp_repr_append_child(current_layer, path);

    return;
}



}; /* namespace Plugin */
}; /* namespace Extension */
}; /* namespace Inkscape */

#include <extension/implementation/plugin-link.h>

/**
    \brief  This is the actual implementation here.  This is done as
            the more generic Implemetnation object so that this code
            can be stolen by someone else more easily.
*/
Inkscape::Extension::Implementation::Implementation * myplug;

/**
    \brief  A function with a C prototype to link back into Inkscape.  This
            function allocated a \c GimpGrad and then calls it's load.
*/
int
load (inkscape_extension * in_ext)
{
    myplug = new Inkscape::Extension::Plugin::Grid();

    return myplug->load(reinterpret_cast<Inkscape::Extension::Extension *>(in_ext));
}

/**
    \brief  A function with a C prototype to link back to Inkscape.  This
            function called the \c GimpGrad unload function and then deletes
            the object.
*/
void
unload (inkscape_extension * in_ext)
{
    myplug->unload(reinterpret_cast<Inkscape::Extension::Extension *>(in_ext));
    delete myplug;
    return;
}

void
effect (inkscape_extension * in_ext, SPView * view)
{
    return myplug->effect(reinterpret_cast<Inkscape::Extension::Effect *>(in_ext), view);
}
/**
    \brief  A structure holding all the functions that Inkscape uses to
            communicate with this plugin.  These are all C-prototype based
            for easy compliation.
*/
inkscape_plugin_function_table INKSCAPE_PLUGIN_NAME = {
    INKSCAPE_PLUGIN_VERSION,
    load,
    unload,
    NULL,
    NULL,
    effect
};

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
