/*
 * This is the code that moves all of the SVG loading and saving into
 * the module format.  Really Inkscape is built to handle these formats
 * internally, so this is just calling those internal functions.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2002-2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <vector>

#include <gtkmm.h>

#include <giomm/file.h>
#include <giomm/file.h>

#include "document.h"
#include "inkscape.h"
#include "preferences.h"
#include "extension/output.h"
#include "extension/input.h"
#include "extension/system.h"
#include "file.h"
#include "svg.h"
#include "file.h"
#include "display/cairo-utils.h"
#include "extension/system.h"
#include "extension/output.h"
#include "xml/attribute-record.h"
#include "xml/simple-document.h"

#include "object/sp-namedview.h"
#include "object/sp-image.h"
#include "object/sp-root.h"
#include "util/units.h"
#include "selection-chemistry.h"

// TODO due to internal breakage in glibmm headers, this must be last:
#include <glibmm/i18n.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

#include "clear-n_.h"


using Inkscape::Util::List;
using Inkscape::XML::AttributeRecord;
using Inkscape::XML::Node;

/*
 * Removes all sodipodi and inkscape elements and attributes from an xml tree. 
 * used to make plain svg output.
 */
static void pruneExtendedNamespaces( Inkscape::XML::Node *repr )
{
    if (repr) {
        if ( repr->type() == Inkscape::XML::ELEMENT_NODE ) {
            std::vector<gchar const*> attrsRemoved;
            for ( List<AttributeRecord const> it = repr->attributeList(); it; ++it ) {
                const gchar* attrName = g_quark_to_string(it->key);
                if ((strncmp("inkscape:", attrName, 9) == 0) || (strncmp("sodipodi:", attrName, 9) == 0)) {
                    attrsRemoved.push_back(attrName);
                }
            }
            // Can't change the set we're interating over while we are iterating.
            for ( std::vector<gchar const*>::iterator it = attrsRemoved.begin(); it != attrsRemoved.end(); ++it ) {
                repr->setAttribute(*it, nullptr);
            }
        }

        std::vector<Inkscape::XML::Node *> nodesRemoved;
        for ( Node *child = repr->firstChild(); child; child = child->next() ) {
            if((strncmp("inkscape:", child->name(), 9) == 0) || strncmp("sodipodi:", child->name(), 9) == 0) {
                nodesRemoved.push_back(child);
            } else {
                pruneExtendedNamespaces(child);
            }
        }
        for ( std::vector<Inkscape::XML::Node *>::iterator it = nodesRemoved.begin(); it != nodesRemoved.end(); ++it ) {
            repr->removeChild(*it);
        }
    }
}

/*
 * Similar to the above sodipodi and inkscape prune, but used on all documents
 * to remove problematic elements (for example Adobe's i:pgf tag) only removes
 * known garbage tags.
 */
static void pruneProprietaryGarbage( Inkscape::XML::Node *repr )
{
    if (repr) {
        std::vector<Inkscape::XML::Node *> nodesRemoved;
        for ( Node *child = repr->firstChild(); child; child = child->next() ) { 
            if((strncmp("i:pgf", child->name(), 5) == 0)) {
                nodesRemoved.push_back(child);
                g_warning( "An Adobe proprietary tag was found which is known to cause issues. It was removed before saving.");
            } else {
                pruneProprietaryGarbage(child);
            }
        }
        for ( std::vector<Inkscape::XML::Node *>::iterator it = nodesRemoved.begin(); it != nodesRemoved.end(); ++it ) { 
            repr->removeChild(*it);
        }
    }
}

/**
 *  \return    None
 *
 *  \brief     Create new markers where necessary to simulate the SVG 2 marker attribute 'orient'
 *             value 'auto-start-reverse'.
 *
 *  \param     repr  The current element to check.
 *  \param     defs  A pointer to the <defs> element.
 *  \param     css   The properties of the element to check.
 *  \param     property  Which property to check, either 'marker' or 'marker-start'.
 *
 */
static void remove_marker_auto_start_reverse(Inkscape::XML::Node *repr,
                                             Inkscape::XML::Node *defs,
                                             SPCSSAttr *css,
                                             Glib::ustring const &property)
{
    Glib::ustring value = sp_repr_css_property (css, property.c_str(), "");

    if (!value.empty()) {

        // Find reference <marker>
        static Glib::RefPtr<Glib::Regex> regex = Glib::Regex::create("url\\(#([A-z0-9#]*)\\)");
        Glib::MatchInfo matchInfo;
        regex->match(value, matchInfo);

        if (matchInfo.matches()) {

            std::string marker_name = matchInfo.fetch(1);
            Inkscape::XML::Node *marker = sp_repr_lookup_child (defs, "id", marker_name.c_str());
            if (marker) {

                // Does marker use "auto-start-reverse"?
                if (strncmp(marker->attribute("orient"), "auto-start-reverse", 17)==0) {

                    // See if a reversed marker already exists.
                    Glib::ustring marker_name_reversed = marker_name + "_reversed";
                    Inkscape::XML::Node *marker_reversed =
                        sp_repr_lookup_child (defs, "id", marker_name_reversed.c_str());

                    if (!marker_reversed) {

                        // No reversed marker, need to create!
                        marker_reversed = repr->document()->createElement("svg:marker");

                        // Copy attributes
                        for (List<AttributeRecord const> iter = marker->attributeList();
                             iter ; ++iter) {
                            marker_reversed->setAttribute(g_quark_to_string(iter->key), iter->value);
                        }

                        // Override attributes
                        marker_reversed->setAttribute("id", marker_name_reversed.c_str());
                        marker_reversed->setAttribute("orient", "auto");

                        // Find transform
                        const char* refX = marker_reversed->attribute("refX");
                        const char* refY = marker_reversed->attribute("refY");
                        std::string transform = "rotate(180";
                        if (refX) {
                            transform += ",";
                            transform += refX;

                            if (refY) {
                                if (refX) {
                                    transform += ",";
                                    transform += refY;
                                } else {
                                    transform += ",0,";
                                    transform += refY;
                                }
                            }
                        }
                        transform += ")";

                        // We can't set a transform on a marker... must create group first.
                        Inkscape::XML::Node *group = repr->document()->createElement("svg:g");
                        group->setAttribute("transform", transform);
                        marker_reversed->addChild(group, nullptr);

                        // Copy all marker content to group.
                        for (auto child = marker->firstChild() ; child != nullptr ; child = child->next() ) {
                            auto new_child = child->duplicate(repr->document());
                            group->addChild(new_child, nullptr);
                            new_child->release();
                        }

                        // Add new marker to <defs>.
                        defs->addChild(marker_reversed, marker);
                        marker_reversed->release();
                     }

                    // Change url to reference reversed marker.
                    std::string marker_url("url(#" + marker_name_reversed + ")");
                    sp_repr_css_set_property(css, "marker-start", marker_url.c_str());

                    // Also fix up if property is marker shorthand.
                    if (property == "marker") {
                        std::string marker_old_url("url(#" + marker_name + ")");
                        sp_repr_css_unset_property(css, "marker");
                        sp_repr_css_set_property(css, "marker-mid", marker_old_url.c_str());
                        sp_repr_css_set_property(css, "marker-end", marker_old_url.c_str());
                    }

                    sp_repr_css_set(repr, css, "style");

                } // Uses auto-start-reverse
            }
        }
    }
}

// Called by remove_marker_context_paint() for each property value ("marker", "marker-start", ...).
static void remove_marker_context_paint (Inkscape::XML::Node *repr,
                                         Inkscape::XML::Node *defs,
                                         Glib::ustring property)
{
    // Value of 'marker', 'marker-start', ... property.
    std::string value("url(#");
    value += repr->attribute("id");
    value += ")";

    // Generate a list of elements that reference this marker.
    std::vector<Inkscape::XML::Node *> to_fix_fill_stroke =
        sp_repr_lookup_property_many(repr->root(), property, value);

    for (auto it: to_fix_fill_stroke) {

        // Figure out value of fill... could be inherited.
        SPCSSAttr* css = sp_repr_css_attr_inherited (it, "style");
        Glib::ustring fill   = sp_repr_css_property (css, "fill",   "");
        Glib::ustring stroke = sp_repr_css_property (css, "stroke", "");

        // Name of new marker.
        Glib::ustring marker_fixed_id = repr->attribute("id");
        if (!fill.empty()) {
            marker_fixed_id += "_F" + fill;
        }
        if (!stroke.empty()) {
            marker_fixed_id += "_S" + stroke;
        }

        // See if a fixed marker already exists.
        // Could be more robust, assumes markers are direct children of <defs>.
        Inkscape::XML::Node* marker_fixed = sp_repr_lookup_child(defs, "id", marker_fixed_id.c_str());

        if (!marker_fixed) {

            // Need to create new marker.

            marker_fixed = repr->duplicate(repr->document());
            marker_fixed->setAttribute("id", marker_fixed_id);

            // This needs to be turned into a function that fixes all descendents.
            for (auto child = marker_fixed->firstChild() ; child != nullptr ; child = child->next()) {
                // Find style.
                SPCSSAttr* css = sp_repr_css_attr ( child, "style" );

                Glib::ustring fill2   = sp_repr_css_property (css, "fill",   "");
                if (fill2 == "context-fill" ) {
                    sp_repr_css_set_property (css, "fill", fill.c_str());
                }
                if (fill2 == "context-stroke" ) {
                    sp_repr_css_set_property (css, "fill", stroke.c_str());
                }

                Glib::ustring stroke2 = sp_repr_css_property (css, "stroke", "");
                if (stroke2 == "context-fill" ) {
                    sp_repr_css_set_property (css, "stroke", fill.c_str());
                }
                if (stroke2 == "context-stroke" ) {
                    sp_repr_css_set_property (css, "stroke", stroke.c_str());
                }

                sp_repr_css_set(child, css, "style");
            }

            defs->addChild(marker_fixed, repr);
            marker_fixed->release();
        }

        Glib::ustring marker_value = "url(#" + marker_fixed_id + ")";
        sp_repr_css_set_property (css, property.c_str(), marker_value.c_str());
        sp_repr_css_set (it, css, "style");
    }
}

static void remove_marker_context_paint (Inkscape::XML::Node *repr,
                                         Inkscape::XML::Node *defs)
{
    if (strncmp("svg:marker", repr->name(), 10) == 0) {

        if (!repr->attribute("id")) {

            std::cerr << "remove_marker_context_paint: <marker> without 'id'!" << std::endl;

        } else {

            // First see if we need to do anything.
            bool need_to_fix = false;

            // This needs to be turned into a function that searches all descendents.
            for (auto child = repr->firstChild() ; child != nullptr ; child = child->next()) {

                // Find style.
                SPCSSAttr* css = sp_repr_css_attr ( child, "style" );
                Glib::ustring fill   = sp_repr_css_property (css, "fill",   "");
                Glib::ustring stroke = sp_repr_css_property (css, "stroke", "");
                if (fill   == "context-fill"   ||
                    fill   == "context-stroke" ||
                    stroke == "context-fill"   ||
                    stroke == "context-stroke" ) {
                    need_to_fix = true;
                    break;
                }
            }

            if (need_to_fix) {

                // Now we need to search document for all elements that use this marker.
                remove_marker_context_paint (repr, defs, "marker");
                remove_marker_context_paint (repr, defs, "marker-start");
                remove_marker_context_paint (repr, defs, "marker-mid");
                remove_marker_context_paint (repr, defs, "marker-end");
            }
        }
    }
}

/*
 * Recursively transform SVG 2 to SVG 1.1, if possible.
 */
static void transform_2_to_1( Inkscape::XML::Node *repr, Inkscape::XML::Node *defs = nullptr )
{
    if (repr) {

        // std::cout << "transform_2_to_1: " << repr->name() << std::endl;

        // Things we do once per node. -----------------------

        // Find defs, if does not exist, create.
        if (defs == nullptr) {
            defs = sp_repr_lookup_name (repr, "svg:defs");
        }
        if (defs == nullptr) {
            defs = repr->document()->createElement("svg:defs");
            repr->root()->addChild(defs, nullptr);
        }

        // Find style.
        SPCSSAttr* css = sp_repr_css_attr ( repr, "style" );

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();

        // Individual items ----------------------------------

        // SVG 2 marker attribute orient:auto-start-reverse:
        if ( prefs->getBool("/options/svgexport/marker_autostartreverse", false) ) {
            // Do "marker-start" first for efficiency reasons.
            remove_marker_auto_start_reverse(repr, defs, css, "marker-start");
            remove_marker_auto_start_reverse(repr, defs, css, "marker");
        }

        // SVG 2 paint values 'context-fill', 'context-stroke':
        if ( prefs->getBool("/options/svgexport/marker_contextpaint", false) ) {
            remove_marker_context_paint(repr, defs);
        }

        // SVG 2 wrapped text to SVG 1.1 text:  NEED TO FINISH
        if (strncmp("svg:text", repr->name(), 8) == 0) {
            for ( List<AttributeRecord const> it = repr->attributeList(); it; ++it ) {
                const gchar* attrName = g_quark_to_string(it->key);
                // std::cout << "  " << (attrName?attrName:"unknown attribute") << std::endl;
                if (strncmp("style", attrName, 5) == 0) {
                    // std::cout << " found style!" << std::endl;
                    break;
                }
            }
        }

        // *** To Do ***
        // Context fill & stroke outside of markers
        // Paint-Order
        // Meshes
        // Hatches

        for ( Node *child = repr->firstChild(); child; child = child->next() ) {
            transform_2_to_1 (child, defs);
        }
    }
}




/**
    \return   None
    \brief    What would an SVG editor be without loading/saving SVG
              files.  This function sets that up.

    For each module there is a call to Inkscape::Extension::build_from_mem
    with a rather large XML file passed in.  This is a constant string
    that describes the module.  At the end of this call a module is
    returned that is basically filled out.  The one thing that it doesn't
    have is the key function for the operation.  And that is linked at
    the end of each call.
*/
void
Svg::init()
{
    /* SVG in */
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("SVG Input") "</name>\n"
            "<id>" SP_MODULE_KEY_INPUT_SVG "</id>\n"
            "<param name='link_svg' type='optiongroup' appearance='full' _gui-text='" N_("SVG Image Import Type:") "' >\n"
                    "<_option value='include' >" N_("Include SVG image as editable object(s) in the current file") "</_option>\n"
                    "<_option value='embed' >" N_("Embed the SVG file in a image tag (not editable in this document)") "</_option>\n"
                    "<_option value='link' >" N_("Link the SVG file in a image tag (not editable in this document).") "</_option>\n"
                  "</param>\n"
            "<param name='scale' appearance='minimal' type='optiongroup' _gui-text='" N_("Image Rendering Mode:") "' _gui-description='" N_("When an image is upscaled, apply smoothing or keep blocky (pixelated). (Will not work in all browsers.)") "' >\n"
                    "<_option value='auto' >" N_("None (auto)") "</_option>\n"
                    "<_option value='optimizeQuality' >" N_("Smooth (optimizeQuality)") "</_option>\n"
                    "<_option value='optimizeSpeed' >" N_("Blocky (optimizeSpeed)") "</_option>\n"
                  "</param>\n"

            "<param name=\"do_not_ask\" _gui-description='" N_("Hide the dialog next time and always apply the same actions.") "' _gui-text=\"" N_("Don't ask again") "\" type=\"boolean\" >false</param>\n"
            "<input>\n"
                "<extension>.svg</extension>\n"
                "<mimetype>image/svg+xml</mimetype>\n"
                "<filetypename>" N_("Scalable Vector Graphic (*.svg)") "</filetypename>\n"
                "<filetypetooltip>" N_("Inkscape native file format and W3C standard") "</filetypetooltip>\n"
                "<output_extension>" SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE "</output_extension>\n"
            "</input>\n"
        "</inkscape-extension>", new Svg());

    /* SVG out Inkscape */
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("SVG Output Inkscape") "</name>\n"
            "<id>" SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE "</id>\n"
            "<output>\n"
                "<extension>.svg</extension>\n"
                "<mimetype>image/x-inkscape-svg</mimetype>\n"
                "<filetypename>" N_("Inkscape SVG (*.svg)") "</filetypename>\n"
                "<filetypetooltip>" N_("SVG format with Inkscape extensions") "</filetypetooltip>\n"
                "<dataloss>false</dataloss>\n"
            "</output>\n"
        "</inkscape-extension>", new Svg());

    /* SVG out */
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("SVG Output") "</name>\n"
            "<id>" SP_MODULE_KEY_OUTPUT_SVG "</id>\n"
            "<output>\n"
                "<extension>.svg</extension>\n"
                "<mimetype>image/svg+xml</mimetype>\n"
                "<filetypename>" N_("Plain SVG (*.svg)") "</filetypename>\n"
                "<filetypetooltip>" N_("Scalable Vector Graphics format as defined by the W3C") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>", new Svg());

    return;
}


/**
    \return    A new document just for you!
    \brief     This function takes in a filename of a SVG document and
               turns it into a SPDocument.
    \param     mod   Module to use
    \param     uri   The path or URI to the file (UTF-8)

    This function is really simple, it just calls sp_document_new...
*/
SPDocument *
Svg::open (Inkscape::Extension::Input *mod, const gchar *uri)
{
    auto file = Gio::File::create_for_uri(uri);
    const auto path = file->get_path();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool ask = prefs->getBool("/dialogs/import/ask");
    Glib::ustring link_svg  = prefs->getString("/dialogs/import/link_svg");
    Glib::ustring scale = prefs->getString("/dialogs/import/scale");
    bool is_import = false;
    if (strcmp(prefs->getString("/options/openmethod/value").c_str(), "import") == 0) {
        is_import = true;
    }
    if(INKSCAPE.use_gui() && is_import && ask) {
        Glib::ustring mod_link_svg = mod->get_param_optiongroup("link_svg");
        Glib::ustring mod_scale = mod->get_param_optiongroup("scale");
        if( link_svg.compare( mod_link_svg) != 0 ) {
            link_svg = mod_link_svg;
        }
        prefs->setString("/dialogs/import/link_svg", link_svg );
        if( scale.compare( mod_scale ) != 0 ) {
            scale = mod_scale;
        }
        prefs->setString("/dialogs/import/scale", scale );
        prefs->setBool("/dialogs/import/ask", !mod->get_param_bool("do_not_ask") );
    }
    
    SPDocument * doc = SPDocument::createNewDoc (nullptr, TRUE, TRUE);
    if (link_svg.compare("include") != 0 && is_import) {
        bool embed = ( link_svg.compare( "embed" ) == 0 );
        SPDocument * ret = SPDocument::createNewDoc(uri, TRUE);
        SPNamedView *nv = sp_document_namedview(doc, nullptr);
        Glib::ustring display_unit = nv->display_units->abbr;
        if (display_unit.empty()) {
            display_unit = "px";
        }
        double width  = ret->getWidth().value(display_unit);
        double height = ret->getHeight().value(display_unit);
        // Create image node
        Inkscape::XML::Document *xml_doc = doc->getReprDoc();
        Inkscape::XML::Node *image_node = xml_doc->createElement("svg:image");

        // Added 11 Feb 2014 as we now honor "preserveAspectRatio" and this is
        // what Inkscaper's expect.
        image_node->setAttribute("preserveAspectRatio", "none");
        image_node->setAttribute("width", Glib::ustring::format(width));
        image_node->setAttribute("height", Glib::ustring::format(height));
        Glib::ustring scale = prefs->getString("/dialogs/import/scale");
        if( scale.compare( "auto" ) != 0 ) {
            SPCSSAttr *css = sp_repr_css_attr_new();
            sp_repr_css_set_property(css, "image-rendering", scale.c_str());
            sp_repr_css_set(image_node, css, "style");
            sp_repr_css_attr_unref( css );
        }
        // convert filename to uri
        if (embed) {
            std::unique_ptr<Inkscape::Pixbuf> pb(Inkscape::Pixbuf::create_from_file(uri));
            if(pb) {
                sp_embed_svg(image_node, uri);
            }
        } else {
            gchar* _uri = g_filename_to_uri(uri, nullptr, nullptr);
            if(_uri) {
                image_node->setAttribute("xlink:href", _uri);
                g_free(_uri);
            } else {
                image_node->setAttribute("xlink:href", uri);
            }
        }
        // Add it to the current layer
        Inkscape::XML::Node *layer_node = xml_doc->createElement("svg:g");
        layer_node->setAttribute("inkscape:groupmode", "layer");
        layer_node->setAttribute("inkscape:label", "Image");
        doc->getRoot()->appendChildRepr(layer_node);
        layer_node->appendChild(image_node);
        Inkscape::GC::release(image_node);
        Inkscape::GC::release(layer_node);
        fit_canvas_to_drawing(doc);
        
        // Set viewBox if it doesn't exist
        if (!doc->getRoot()->viewBox_set) {
            doc->setViewBox(Geom::Rect::from_xywh(0, 0, doc->getWidth().value(doc->getDisplayUnit()), doc->getHeight().value(doc->getDisplayUnit())));
        }
        return doc;
    }
    if (!file->get_uri_scheme().empty()) {
        if (path.empty()) {
            try {
                char *contents;
                gsize length;
                file->load_contents(contents, length);
                return SPDocument::createNewDocFromMem(contents, length, 1);
            } catch (Gio::Error &e) {
                g_warning("Could not load contents of non-local URI %s\n", uri);
                return nullptr;
            }
        } else {
            uri = path.c_str();
        }
    }

    return SPDocument::createNewDoc(uri, TRUE);
}

/**
    \return    None
    \brief     This is the function that does all of the SVG saves in
               Inkscape.  It detects whether it should do a Inkscape
               namespace save internally.
    \param     mod   Extension to use.
    \param     doc   Document to save.
    \param     uri   The filename to save the file to.

    This function first checks its parameters, and makes sure that
    we're getting good data.  It also checks the module ID of the
    incoming module to figure out whether this save should include
    the Inkscape namespace stuff or not.  The result of that comparison
    is stored in the exportExtensions variable.

    If there is not to be Inkscape name spaces a new document is created
    without.  (I think, I'm not sure on this code)

    All of the internally referenced imageins are also set to relative
    paths in the file.  And the file is saved.

    This really needs to be fleshed out more, but I don't quite understand
    all of this code.  I just stole it.
*/
void
Svg::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *filename)
{
    // std::cout << "Svg::save: " << (filename?filename:"null") << std::endl;

    g_return_if_fail(doc != nullptr);
    g_return_if_fail(filename != nullptr);
    Inkscape::XML::Document *rdoc = doc->rdoc;

    bool const exportExtensions = ( !mod->get_id()
      || !strcmp (mod->get_id(), SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE)
      || !strcmp (mod->get_id(), SP_MODULE_KEY_OUTPUT_SVGZ_INKSCAPE));

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool const transform2to1 =
        prefs->getBool("/dialogs/save_as/enable_svgexport", false);

    bool createNewDoc = !exportExtensions || transform2to1;

    // We prune the in-use document and deliberately loose data, because there
    // is no known use for this data at the present time.
    pruneProprietaryGarbage(rdoc->root());

    if (createNewDoc) {

        // We make a duplicate document so we don't prune the in-use document
        // and loose data. Perhaps the user intends to save as inkscape-svg next.
        Inkscape::XML::Document *new_rdoc = new Inkscape::XML::SimpleDocument();

        // Comments and PI nodes are not included in this duplication
        // TODO: Move this code into xml/document.h and duplicate rdoc instead of root. 
        new_rdoc->setAttribute("standalone", "no");
        new_rdoc->setAttribute("version", "2.0");

        // Get a new xml repr for the svg root node
        Inkscape::XML::Node *root = rdoc->root()->duplicate(new_rdoc);

        // Add the duplicated svg node as the document's rdoc
        new_rdoc->appendChild(root);
        Inkscape::GC::release(root);

        if (!exportExtensions) {
            pruneExtendedNamespaces(root);
        }

        if (transform2to1) {
            transform_2_to_1 (root);
            new_rdoc->setAttribute("version", "1.1");
        }

        rdoc = new_rdoc;
    }

    if (!sp_repr_save_rebased_file(rdoc, filename, SP_SVG_NS_URI,
                                   doc->getBase(), filename)) {
        throw Inkscape::Extension::Output::save_failed();
    }

    if (createNewDoc) {
        Inkscape::GC::release(rdoc);
    }

    return;
}

} } }  /* namespace inkscape, module, implementation */

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
