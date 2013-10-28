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
#include "sp-object.h"
#include "svg.h"
#include "file.h"
#include "extension/system.h"
#include "extension/output.h"
#include <vector>
#include "xml/attribute-record.h"
#include "xml/event-fns.h"
#include "document.h"
#include "factory.h"
#include "sp-factory.h"
#include "sp-root.h"
#include "sp-switch.h"
#include "sp-text.h"
#include "sp-flowtext.h"

#ifdef WITH_GNOME_VFS
# include <libgnomevfs/gnome-vfs.h>
#endif

namespace Inkscape {
namespace Extension {
namespace Internal {

#include "clear-n_.h"


using Inkscape::Util::List;
using Inkscape::XML::AttributeRecord;
using Inkscape::XML::Node;

void flowText(SPDocument* orig, SPDocument* doc);

static void pruneExtendedAttributes( Inkscape::XML::Node *repr )
{
    if (repr) {
        if ( repr->type() == Inkscape::XML::ELEMENT_NODE ) {
            std::vector<gchar const*> toBeRemoved;
            for ( List<AttributeRecord const> it = repr->attributeList(); it; ++it ) {
                const gchar* attrName = g_quark_to_string(it->key);
                if ((strncmp("inkscape:", attrName, 9) == 0) || (strncmp("sodipodi:", attrName, 9) == 0)) {
                    toBeRemoved.push_back(attrName);
                }
            }
            // Can't change the set we're interating over while we are iterating.
            for ( std::vector<gchar const*>::iterator it = toBeRemoved.begin(); it != toBeRemoved.end(); ++it ) {
                repr->setAttribute(*it, 0);
            }
        }

        for ( Node *child = repr->firstChild(); child; child = child->next() ) {
            pruneExtendedAttributes(child);
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
Svg::init(void)
{
    /* SVG in */
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("SVG Input") "</name>\n"
            "<id>" SP_MODULE_KEY_INPUT_SVG "</id>\n"
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

#ifdef WITH_GNOME_VFS
    gnome_vfs_init();
#endif


    return;
}


#ifdef WITH_GNOME_VFS
#define BUF_SIZE 8192

static gchar *
_load_uri (const gchar *uri)
{
    GnomeVFSHandle   *handle = NULL;
    GnomeVFSFileSize  bytes_read;

        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        GError* error = NULL;
        gchar* uri_local = g_filename_from_utf8( uri, -1, &bytesRead, &bytesWritten, &error);

        if ( uri_local == NULL ) {
            g_warning( "Error converting filename to locale encoding.");
        }

    GnomeVFSResult result = gnome_vfs_open (&handle, uri_local, GNOME_VFS_OPEN_READ);

    if (result != GNOME_VFS_OK) {
        g_warning("%s", gnome_vfs_result_to_string(result));
    }

    std::vector<gchar> doc;
    while (result == GNOME_VFS_OK) {
        gchar buffer[BUF_SIZE];
        result = gnome_vfs_read (handle, buffer, BUF_SIZE, &bytes_read);
        doc.insert(doc.end(), buffer, buffer+bytes_read);
    }

    return g_strndup(&doc[0], doc.size());
}
#endif


/**
    \return    A new document just for you!
    \brief     This function takes in a filename of a SVG document and
               turns it into a SPDocument.
    \param     mod   Module to use
    \param     uri   The path to the file (UTF-8)

    This function is really simple, it just calls sp_document_new...
*/
SPDocument *
Svg::open (Inkscape::Extension::Input */*mod*/, const gchar *uri)
{
#ifdef WITH_GNOME_VFS
    if (!gnome_vfs_initialized() || gnome_vfs_uri_is_local(gnome_vfs_uri_new(uri))) {
        // Use built-in loader instead of VFS for this
        return SPDocument::createNewDoc(uri, TRUE);
    }
    gchar * buffer = _load_uri(uri);
    if (buffer == NULL) {
        g_warning("Error:  Could not open file '%s' with VFS\n", uri);
        return NULL;
    }
    SPDocument * doc = SPDocument::createNewDocFromMem(buffer, strlen(buffer), 1);

    g_free(buffer);
    return doc;
#else
    return SPDocument::createNewDoc(uri, TRUE);
#endif
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
    g_return_if_fail(doc != NULL);
    g_return_if_fail(filename != NULL);

    bool const exportExtensions = ( !mod->get_id()
      || !strcmp (mod->get_id(), SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE)
      || !strcmp (mod->get_id(), SP_MODULE_KEY_OUTPUT_SVGZ_INKSCAPE));

    Inkscape::XML::Node *repr = NULL;
    SPDocument* newDoc = NULL;

    newDoc = SPDocument::createNewDoc(0, 0, true);
    repr = doc->getRoot()->updateRepr(newDoc->getReprDoc(),
            newDoc->getReprRoot(), SP_OBJECT_WRITE_BUILD);

    if (!exportExtensions) {
        pruneExtendedAttributes(repr);
    }

    flowText(doc, newDoc);

    if (!sp_repr_save_rebased_file(repr->document(), filename, SP_SVG_NS_URI,
                                   doc->getBase(), filename)) {
        throw Inkscape::Extension::Output::save_failed();
    }

    delete newDoc;
    return;
}

class FlowTextReplacer {

private:
    SPDocument* document;

    //we need this nonsense so that we can get a correctly-laid-out flowtext
    SPDocument* oldDocument;
    Inkscape::XML::Document* xml_document;

    Inkscape::XML::Node* getBasicText(SPFlowtext* flowtext) const {
        //This is probably an insane way to get a SPFlowtext with the correct layout, but I
        //can't figure out another way.
        SPObject* original_object = oldDocument->getObjectById(flowtext->getId());
        SPFlowtext* original_flowtext = SP_FLOWTEXT(original_object);
        flowtext->layout = original_flowtext->layout;
        Inkscape::XML::Node* basic_text = flowtext->getAsText();
        return basic_text;
    }

    SPItem* makeBasicTextItem(Inkscape::XML::Node* basic_text,
            SPItem* item) const {
        SPItem* new_item =
                reinterpret_cast<SPItem*>(SPFactory::instance().createObject(
                        NodeTraits::get_type_string(*basic_text)));
        new_item->invoke_build(document, basic_text, TRUE);
        new_item->doWriteTransform(basic_text, item->transform);
        new_item->updateRepr();
        return new_item;
    }

public:

    FlowTextReplacer (SPDocument* oldDocument, SPDocument* document) :
        document (document),
        oldDocument (oldDocument) {
        xml_document = document->getReprDoc();
    }

    bool operator () (SPObject* obj) const {
        SPItem* item = SP_ITEM(obj);
        if (!item)
            return true;

        /* This might be a switch or a flowtext.  If it's a flowtext, we need to wrap it
         * in a switch.  If it's a switch with <flowtext, text>, we need to update the
         * text.
        */

        if (SP_IS_FLOWTEXT(item)) {
            SPFlowtext* flowtext = SP_FLOWTEXT(item);

            Inkscape::XML::Node* basic_text = getBasicText(flowtext);
            if (!basic_text) {
                //this shouldn't happen, but when it does, there's
                //nothing we can do about it.
                g_warning("Could not get basic text from flowed text for %s", item->getId());
                return false;
            }

            Inkscape::XML::Node* flowtext_repr = flowtext->getRepr();
            Inkscape::XML::Node* parent = flowtext_repr->parent();

            Inkscape::XML::Node *switch_repr = xml_document->createElement("svg:switch");
            parent->addChild(switch_repr, flowtext_repr);

            //duplicate the flowtext so that we can place it into the switch
            Inkscape::XML::Node* flowtext_dup = flowtext_repr->duplicate(xml_document);

            flowtext_dup->setAttribute("requiredFeatures", FLOWROOT_FEATURE);

            SPObject* switch_obj = document->getObjectByRepr(switch_repr);
            switch_repr->addChild(flowtext_dup, NULL);
            switch_repr->addChild(basic_text, flowtext_dup);
            makeBasicTextItem(basic_text, item);

            flowtext_dup->release();

            //remove the old flowtext
            parent->removeChild(flowtext_repr);

            flowtext->deleteObject();
        } else {
            if (!SP_IS_SWITCH(item)) {
                return true;
            }

            Inkscape::XML::Node* switch_repr = item->getRepr();
            SPObject* text = NULL;
            SPFlowtext* flowtext = NULL;

            for (SPObject* child = item->firstChild(); child; child = child->getNext()) {
                if (SP_IS_FLOWTEXT(child)) {
                    flowtext = SP_FLOWTEXT(child);
                } else if (SP_IS_TEXT(child)) {
                    text = child;
                }
            }
            if (!flowtext) {
                //this switch doesn't contain a flowtext directly, so we don't need to think
                //about it.
                return true;
            }

            Inkscape::XML::Node* basic_text = getBasicText(flowtext);
            if (!basic_text) {
                //this shouldn't happen, but when it does, there's
                //nothing we can do about it.
                g_warning("Could not get basic text from flowed text for %s", item->getId());
                return false;
            }

            //remove the text, because we're going to reconstruct it and add it back
            if (text) {
                switch_repr->removeChild(text->getRepr());
                text->deleteObject();
            }

            flowtext->setAttribute("requiredFeatures", FLOWROOT_FEATURE);

            switch_repr->addChild(basic_text, flowtext->getRepr());
            makeBasicTextItem(basic_text, flowtext);
        }

        return false;
    }
};

void flowText(SPDocument *oldDoc, SPDocument* doc) {

    visit(doc->getRoot(), FlowTextReplacer(oldDoc, doc));

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
