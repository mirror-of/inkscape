/*
 * Implementation of the file dialog interfaces defined in filedialog.h
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/entry.h>
#include <gdkmm/pixbuf.h>

#include <map>

#include <string.h>

#include "interface.h"
#include "filedialog.h"
#include "helper/sp-intl.h"
#include "prefs-utils.h"

#include <dialogs/dialog-events.h>
#include <extension/extension.h>
#include <extension/db.h>

//for the preview widget
#include "document.h"
#include "inkscape.h"
#include "svg-view.h"
#include "uri.h"


namespace Inkscape
{
namespace UI
{
namespace Dialogs
{

/*#########################################################################
### SVG Preview Widget
#########################################################################*/
/**
 * Simple class for displaying an SVG file in the "preview widget."
 * Currently, this is just a wrapper of the sp_svg_view Gtk widget.
 * Hopefully we will eventually replace with a pure Gtkmm widget.
 */
class SVGPreview : public Gtk::VBox
{
    public:
        SVGPreview();
        ~SVGPreview();

        bool setDocument(SPDocument *doc);

        bool setFileName(const char *filename);

        bool setFromMem(const char *xmlBuffer);

        bool set(const char *fileName, int dialogType);

        bool setURI(URI &uri);

    private:

        SPDocument *document;

        GtkWidget *viewerGtk;

};


bool SVGPreview::setDocument(SPDocument *doc)
{
    if (document)
        sp_document_unref(document);

    sp_document_ref(doc);
    document = doc;

    //This should remove it from the box, and free resources
    if (viewerGtk)
        {
        gtk_widget_destroy(viewerGtk);
        }

     viewerGtk  = sp_svg_view_widget_new(doc);
     GtkWidget *vbox = (GtkWidget *)gobj();
     gtk_box_pack_start (GTK_BOX (vbox), viewerGtk, TRUE, TRUE, 0);
     gtk_widget_show(viewerGtk);



    return true;
}

bool SVGPreview::setFileName(const char *filename)
{ 
    SPDocument *doc = sp_document_new (filename, 0, 0);
    if (!doc)
        {
        g_warning("SVGView: error loading document '%s'\n",filename);
        return false;
        }

    setDocument(doc);

    return true;
}



bool SVGPreview::setFromMem(const char *xmlBuffer)
{ 
    gint len = (gint)strlen(xmlBuffer);
    SPDocument *doc = sp_document_new_from_mem (xmlBuffer, len, 0, 0);
    if (!doc)
        {
        g_warning("SVGView: error loading buffer '%s'\n",xmlBuffer);
        return false;
        }

    setDocument(doc);

    return true;
}



bool SVGPreview::set(const char *fName, int dialogType)
{

    if (!g_file_test(fName, G_FILE_TEST_EXISTS))
        return false;

    if (dialogType == SVG_TYPES &&
           (g_str_has_suffix(fName, ".svg") ||   g_str_has_suffix(fName, ".svgz"))
         )
        {
        bool retval = setFileName(fName);
        return retval;
        }
    else if ((dialogType == IMPORT_TYPES || dialogType == EXPORT_TYPES) &&
                 (
                  g_str_has_suffix(fName, ".bmp" ) ||
                  g_str_has_suffix(fName, ".gif" ) ||
                  g_str_has_suffix(fName, ".jpg" ) ||
                  g_str_has_suffix(fName, ".jpeg") ||
                  g_str_has_suffix(fName, ".png" ) ||
                  g_str_has_suffix(fName, ".tif" ) ||
                  g_str_has_suffix(fName, ".tiff")
                 )
             )
        {

        /*#####################################
        # LET'S HAVE SOME FUN WITH SVG!
        # Instead of just loading an image, why
        # don't we make a lovely little svg and
        # display it nicely?
        #####################################*/

        //Arbitrary size of svg doc -- rather 'portrait' shaped
        gint previewWidth  = 400;
        gint previewHeight = 600;

        //Get some image info. Smart pointer does not need to be deleted
        Glib::RefPtr<Gdk::Pixbuf> img = Gdk::Pixbuf::create_from_file(fName);
        gint imgWidth  = img->get_width();
        gint imgHeight = img->get_height();

        //Find the minimum scale to fit the image inside the preview area
        double scaleFactorX = (0.9 *(double)previewWidth)  / ((double)imgWidth);
        double scaleFactorY = (0.9 *(double)previewHeight) / ((double)imgHeight);
        double scaleFactor = scaleFactorX;
        if (scaleFactorX > scaleFactorY)
            scaleFactor = scaleFactorY;

        //Now get the resized values
        gint scaledImgWidth  = (int) (scaleFactor * (double)imgWidth);
        gint scaledImgHeight = (int) (scaleFactor * (double)imgHeight);

        //center the image on the area
        gint imgX = (previewWidth  - scaledImgWidth)  / 2;        
        gint imgY = (previewHeight - scaledImgHeight) / 2;

        //wrap a rectangle around the image
        gint rectX      = imgX-1;        
        gint rectY      = imgY-1;        
        gint rectWidth  = scaledImgWidth +2;        
        gint rectHeight = scaledImgHeight+2;        

        //Our template.  Modify to taste
        gchar *xformat =
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<svg\n"
          "xmlns=\"http://www.w3.org/2000/svg\"\n"
          "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
          "width=\"%d\" height=\"%d\">\n"
          "<image x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"\n"
          "xlink:href=\"%s\"/>\n"
          "<rect\n"
            "style=\"fill:none;fill-opacity:0.75000000;fill-rule:evenodd;"
              "stroke:#000000;stroke-width:4.0;stroke-linecap:butt;"
              "stroke-linejoin:miter;stroke-opacity:1.0000000;"
              "stroke-miterlimit:4.0000000;stroke-dasharray:none;\"\n"
            "x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"/>\n"
          "<text\n"
            "style=\"font-size:24.000000;font-style:normal;font-weight:normal;"
              "fill:#000000;fill-opacity:1.0000000;stroke:none;stroke-width:1.0000000pt;"
              "stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000;"
              "font-family:Bitstream Vera Sans;\"\n"
            "x=\"10\" y=\"26\">%dw x %dh</text>\n"
          "</svg>\n\n";

        //Fill in the template
        gchar *xmlBuffer = g_strdup_printf(xformat, 
               previewWidth, previewHeight,
               imgX, imgY, scaledImgWidth, scaledImgHeight,
               fName,
               rectX, rectY, rectWidth, rectHeight,
               imgWidth, imgHeight);

        //g_message("%s\n", xmlBuffer);

        //now show it!
        bool retval = setFromMem(xmlBuffer);
        g_free(xmlBuffer);
        return retval;
        }

    return false;

}


SVGPreview::SVGPreview()
{
    if (!INKSCAPE)
        inkscape_application_init("");
    document = NULL;
    viewerGtk = NULL;
    set_size_request(150,150);
}

SVGPreview::~SVGPreview()
{

}



/*#########################################################################
### F I L E    O P E N
#########################################################################*/

/**
 * Our implementation class for the FileOpenDialog interface..
 */
class FileOpenDialogImpl : public FileOpenDialog, public Gtk::FileChooserDialog
{
    public:
        FileOpenDialogImpl(const char *dir,
                           FileDialogType fileTypes,
                           const char *title);
                           

        virtual ~FileOpenDialogImpl();

        bool show();
    
        Inkscape::Extension::Extension * getSelectionType();

        gchar * getFilename ();


    protected:



    private:


        /**
         * What type of 'open' are we? (open, import, place, etc)
         */
        FileDialogType dialogType;

        /**
         * Our svg preview widget
         */
        SVGPreview svgPreview;

        /**
         * Callback for seeing if the preview needs to be drawn
         */
        void updatePreviewCallback();

        /**
         * Fix to allow the user to type the file name 
         */
        Gtk::Entry fileNameEntry;

        /**
         * Callback for user input into fileNameEntry
         */
        void fileNameEntryChangedCallback();

        /**
         * Callback for user changing which item is selected on the list
         */
        void fileSelectedCallback();


        /**
         * Filter name->extension lookup
         */
        std::map<Glib::ustring, Inkscape::Extension::Extension *> extensionMap;

        /**
         * The extension to use to write this file
         */
        Inkscape::Extension::Extension * extension;

        /**
         * Filename that was given
         */
        Glib::ustring myFilename;

};





/**
 * Callback for checking if the preview needs to be redrawn
 */
void FileOpenDialogImpl::updatePreviewCallback()
{
    gchar *fName = (gchar *)get_preview_filename().c_str();
    if (!fName)
        return;

    bool retval = svgPreview.set(fName, dialogType);
    set_preview_widget_active(retval);

}





/**
 * Callback for fileNameEntry widget
 */
void FileOpenDialogImpl::fileNameEntryChangedCallback()
{
    gchar *fName = (gchar *)fileNameEntry.get_text().c_str(); 
    //g_message("User hit return.  Text is '%s'\n", fName);

    if (g_file_test(fName, G_FILE_TEST_IS_REGULAR))
       {
       //dialog with either (1) select a regular file or (2) cd to dir
       set_filename(fileNameEntry.get_text());
       //is it a regular file? do the same as 'OK'
       if (g_file_test(fName, G_FILE_TEST_IS_REGULAR))
           response(GTK_RESPONSE_OK);
       }

}





/**
 * Callback for fileNameEntry widget
 */
void FileOpenDialogImpl::fileSelectedCallback()
{
    //g_message("User selected '%s'\n",
    //       get_filename().c_str());

    fileNameEntry.set_text(get_filename());
}






/**
 * Constructor.  Not called directly.  Use the factory.
 */
FileOpenDialogImpl::FileOpenDialogImpl(const char *dir,
                                       FileDialogType fileTypes,
                                       const char *title) :
                                       Gtk::FileChooserDialog(Glib::ustring(title)) {

    /* One file at a time */
    set_select_multiple(false);

    /* Initalize to Autodetect */
    extension = NULL;
    /* No filename to start out with */
    myFilename  = "";

    /* Set our dialog type (open, import, etc...)*/
    dialogType = fileTypes;


    /* Set the pwd and/or the filename */
    if (dir != NULL)
        set_current_folder(dir);

    Gtk::FileFilter topFilter;
    topFilter.set_name(_("Autodetect"));
    extensionMap[Glib::ustring(_("Autodetect"))]=NULL;
    topFilter.add_pattern("*");
    add_filter(topFilter);

    GSList *extension_list = Inkscape::Extension::db.get_input_list();
    if (extension_list == NULL) {
        // Another exception
        g_warning("Internal error.  We need extensions.\n");
        return;
    }

    for (GSList *current_item = g_slist_next(extension_list);
                   current_item; current_item = g_slist_next(current_item)) {

        Inkscape::Extension::DB::IOExtensionDescription * ioext = 
              reinterpret_cast<Inkscape::Extension::DB::IOExtensionDescription *>(current_item->data);

        Gtk::FileFilter filter;
        Glib::ustring uname(_(ioext->name));
        filter.set_name(uname);
        Glib::ustring upattern("*");
        upattern += ioext->file_extension;
        //g_message("ext for %s: '%s'\n", ioext->name,upattern.c_str());
        filter.add_pattern(upattern);
        add_filter(filter);
        extensionMap[uname]=ioext->extension;

    }

    Inkscape::Extension::db.free_list(extension_list);

    //###### Add a preview widget
    set_preview_widget(svgPreview);
    set_preview_widget_active(false);

    //Catch selection-changed events, so we can adjust the text widget
    signal_update_preview().connect( 
         sigc::mem_fun(*this, &FileOpenDialogImpl::updatePreviewCallback) );


    //###### Add a text entry bar, and tie it to file chooser events
    fileNameEntry.set_text(get_current_folder());
    set_extra_widget(fileNameEntry);
    fileNameEntry.grab_focus();

    //Catch when user hits [return] on the text field
    fileNameEntry.signal_activate().connect( 
         sigc::mem_fun(*this, &FileOpenDialogImpl::fileNameEntryChangedCallback) );

    //Catch selection-changed events, so we can adjust the text widget
    signal_selection_changed().connect( 
         sigc::mem_fun(*this, &FileOpenDialogImpl::fileSelectedCallback) );


    add_button(Gtk::Stock::CANCEL, GTK_RESPONSE_CANCEL);
    add_button(Gtk::Stock::OPEN,   GTK_RESPONSE_OK);

}





/**
 * Public factory.  Called by file.cpp, among others.
 */
FileOpenDialog * FileOpenDialog::create(const char *path, 
                                        FileDialogType fileTypes,
                                        const char *title)
{
    FileOpenDialog *dialog = new FileOpenDialogImpl(path, fileTypes, title);
    return dialog;
}




/**
 * Destructor
 */
FileOpenDialogImpl::~FileOpenDialogImpl()
{

}




/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileOpenDialogImpl::show()
{

    set_modal(true);
    gint b = run();  //Dialog
    hide();

    if (b == GTK_RESPONSE_OK)
        {
        if (get_filter()) {
            //Get which extension was chosen, if any
            extension = extensionMap[get_filter()->get_name()];
            }
        myFilename = get_filename();
        return TRUE;
        }
    else
        {
        return FALSE;
        }
}




/**
 * Get the file extension type that was selected by the user. Valid after an [OK]
 */
Inkscape::Extension::Extension *
FileOpenDialogImpl::getSelectionType()
{ 
    return extension;
}


/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
gchar *
FileOpenDialogImpl::getFilename (void)
{ 
    return g_strdup(myFilename.c_str());
}









/*#########################################################################
# F I L E    S A V E
#########################################################################*/

/**
 * Our implementation of the FileSaveDialog interface.
 */
class FileSaveDialogImpl : public FileSaveDialog, public Gtk::FileChooserDialog
{

    public:
        FileSaveDialogImpl(const char *dir,
                           FileDialogType fileTypes,
                           const char *title,
                           const char * default_key);

        virtual ~FileSaveDialogImpl();

        bool show();

        Inkscape::Extension::Extension * getSelectionType();

        gchar * getFilename ();

    protected:

        //# Child widgets
        Gtk::CheckButton checkbox;

    private:

        /**
         * What type of 'open' are we? (save, export, etc)
         */
        FileDialogType dialogType;

        /**
         * Our svg preview widget
         */
        SVGPreview svgPreview;

        /**
         * Callback for seeing if the preview needs to be drawn
         */
        void updatePreviewCallback();

        /**
         * Filter name->extension lookup
         */
        std::map<Glib::ustring, Inkscape::Extension::Extension *> extensionMap;

        bool append_extension;

        /**
         * The extension to use to write this file
         */
        Inkscape::Extension::Extension * extension;

        /**
         * Filename that was given
         */
        Glib::ustring myFilename;
};






/**
 * Callback for checking if the preview needs to be redrawn
 */
void FileSaveDialogImpl::updatePreviewCallback()
{
    gchar *fName = (gchar *)get_preview_filename().c_str();
    if (!fName)
        return;

    bool retval = svgPreview.set(fName, dialogType);
    set_preview_widget_active(retval);

}



/**
 * Constructor
 */
FileSaveDialogImpl::FileSaveDialogImpl(const char *dir, 
                                       FileDialogType fileTypes,
                                       const char *title,
                                       const char * default_key) :
                                       FileChooserDialog(Glib::ustring(title),
                                           Gtk::FILE_CHOOSER_ACTION_SAVE) {


    append_extension = (bool)prefs_get_int_attribute("dialogs.save_as", "append_extension", 1);

    /* One file at a time */
    set_select_multiple(false);

    /* Initalize to Autodetect */
    extension = NULL;
    /* No filename to start out with */
    myFilename  = "";

    /* Set our dialog type (save, export, etc...)*/
    dialogType = fileTypes;

    /* Set the pwd and/or the filename */
    if (dir != NULL)
        set_current_folder(dir);

    Gtk::FileFilter topFilter;
    topFilter.set_name(Glib::ustring(_("Autodetect")));
    extensionMap[Glib::ustring(_("Autodetect"))]=NULL;
    topFilter.add_pattern(Glib::ustring("*"));
    add_filter(topFilter);

    GSList *extension_list = Inkscape::Extension::db.get_input_list();
    if (extension_list == NULL) {
        // Another exception
        g_warning("Internal error.  We need extensions.\n");
        return;
    }

    for (GSList *current_item = g_slist_next(extension_list);
                   current_item; current_item = g_slist_next(current_item)) {

        Inkscape::Extension::DB::IOExtensionDescription * ioext = 
              reinterpret_cast<Inkscape::Extension::DB::IOExtensionDescription *>(current_item->data);

        Gtk::FileFilter filter;
        Glib::ustring uname(_(ioext->name));
        filter.set_name(uname);
        Glib::ustring upattern("*");
        upattern += ioext->file_extension;
        //g_message("ext for %s: '%s'\n", ioext->name,upattern.c_str());
        filter.add_pattern(upattern);
        add_filter(filter);
        extensionMap[uname]=ioext->extension;

    }

    Inkscape::Extension::db.free_list(extension_list);

    checkbox.set_label(Glib::ustring(_("Append filename extension automatically")));
    checkbox.set_active(append_extension);
    checkbox.show();
    get_vbox()->pack_end (checkbox, FALSE, FALSE, 0);

    //###### Add a preview widget
    set_preview_widget(svgPreview);
    set_preview_widget_active(false);

    //Catch selection-changed events, so we can adjust the text widget
    signal_update_preview().connect( 
         sigc::mem_fun(*this, &FileSaveDialogImpl::updatePreviewCallback) );


    //if (extension == NULL)
    //    checkbox.set_sensitive(FALSE);

    add_button(Gtk::Stock::CANCEL, GTK_RESPONSE_CANCEL);
    add_button(Gtk::Stock::SAVE,   GTK_RESPONSE_OK);

}



/**
 * Public factory method.  Used in file.cpp
 */
FileSaveDialog * FileSaveDialog::create(const char *path, 
                                        FileDialogType fileTypes,
                                        const char *title,
                                        const char * default_key)
{
    FileSaveDialog *dialog = new FileSaveDialogImpl(path, fileTypes, title, default_key);
    return dialog;
}





/**
 * Destructor
 */
FileSaveDialogImpl::~FileSaveDialogImpl()
{

}




/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileSaveDialogImpl::show() {

    /* Reset the default back to autodetect */
    set_modal (TRUE); //Window
    //sp_transientize (dlg);
    gint b = run();  //Dialog
    hide();

    if (b == GTK_RESPONSE_OK) {
        if (get_filter()) {
                //Get which extension was chosen, if any
                extension = extensionMap[get_filter()->get_name()];
        }
        myFilename = get_filename();
        return TRUE;

        append_extension = checkbox.get_active();
        prefs_set_int_attribute("dialogs.save_as", "append_extension", append_extension);
        if (extension != NULL)
            prefs_set_string_attribute("dialogs.save_as", "default", extension->get_id());
        else
            prefs_set_string_attribute("dialogs.save_as", "default", "");

        return TRUE;
    } else {
        return FALSE;
    }
}


/**
 * Get the file extension type that was selected by the user. Valid after an [OK]
 */
Inkscape::Extension::Extension *
FileSaveDialogImpl::getSelectionType()
{ 
    return extension;
}


/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
gchar *
FileSaveDialogImpl::getFilename (void)
{ 
    return g_strdup(myFilename.c_str());
}






}; //namespace Dialogs
}; //namespace UI
}; //namespace Inkscape

