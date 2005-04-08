/**
    \file bluredge.cpp
 
    A plug-in to add an effect to blur the edges of an object. 
*/
/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <desktop.h>
#include <selection.h>
#include <helper/action.h>
#include <prefs-utils.h>
#include <path-chemistry.h>

#include <extension/implementation/implementation.h>
#include <extension/extension.h>
#include <extension/effect.h>

#include <glibmm/i18n.h>

namespace Inkscape {
namespace Extension {
namespace Plugin {

/** \brief  Implementation class of the GIMP gradient plugin.  This mostly
            just creates a namespace for the GIMP gradient plugin today.
*/
class BlurEdge : public Inkscape::Extension::Implementation::Implementation {

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
BlurEdge::load (Inkscape::Extension::Extension *module)
{
    // std::cout << "Hey, I'm Blur Edge, I'm loading!" << std::endl;
    return TRUE;
}

/**
    \brief  This actually blurs the edge.
    \param  module   The effect that was called (unused)
    \param  document What should be edited.
*/
void
BlurEdge::effect (Inkscape::Extension::Effect *module, SPView *document)
{
    Inkscape::Selection * selection     = ((SPDesktop *)document)->selection;

    float width = module->get_param_float("blur-width");
    int   steps = module->get_param_int("num-steps");

    double old_offset = prefs_get_double_attribute("options.defaultoffsetwidth", "value", 1.0);

    std::list<SPItem *> items;
    selection->list(items);
    selection->clear();

    std::list<SPItem *> new_items;
    for(std::list<SPItem *>::iterator item = items.begin();
            item != items.end(); item++) {
        SPItem * spitem = *item;

        Inkscape::XML::Node * new_items[steps];
        Inkscape::XML::Node * new_group = sp_repr_new("svg:g");
        (SP_OBJECT_REPR(spitem)->parent())->appendChild(new_group);
        /** \todo  Need to figure out how to get from XML::Node to SPItem */
        /* new_items.push_back(); */

        double orig_opacity = sp_repr_css_double_property(sp_repr_css_attr(SP_OBJECT_REPR(spitem), "style"), "opacity", 1.0);
        char opacity_string[64];
        sprintf(opacity_string, "%f", orig_opacity / (steps));

        for (int i = 0; i < steps; i++) {
            double offset = (width / (float)(steps - 1) * (float)i) - (width / 2.0);

            new_items[i] = (SP_OBJECT_REPR(spitem))->duplicate();

            SPCSSAttr * css = sp_repr_css_attr(new_items[i], "style");
            sp_repr_css_set_property(css, "opacity", opacity_string);
            sp_repr_css_change(new_items[i], css, "style");

            new_group->appendChild(new_items[i]);
            selection->add(new_items[i]);
            sp_selected_path_to_curves();

            if (offset < 0.0) {
                /* Doing an inset here folks */
                offset *= -1.0;
                prefs_set_double_attribute("options.defaultoffsetwidth", "value", offset);
                sp_action_perform(Inkscape::Verb::get(SP_VERB_SELECTION_INSET)->get_action(document), NULL);
            } else if (offset == 0.0) {
            } else {
                prefs_set_double_attribute("options.defaultoffsetwidth", "value", offset);
                sp_action_perform(Inkscape::Verb::get(SP_VERB_SELECTION_OFFSET)->get_action(document), NULL);
            }

            selection->clear();
        }

    }

    prefs_set_double_attribute("options.defaultoffsetwidth", "value", old_offset);

    selection->clear();
    selection->addStlItemList(items);
    selection->addStlItemList(new_items);

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
    myplug = new Inkscape::Extension::Plugin::BlurEdge();

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

Gtk::Widget *
prefs_effect (inkscape_extension * in_ext, SPView * view)
{
    return myplug->prefs_effect(reinterpret_cast<Inkscape::Extension::Effect *>(in_ext), view);
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
    effect,
    prefs_effect
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
