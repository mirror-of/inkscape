
#include "filedialog.h"
#include "module.h"

#include <windows.h>

#include <glib.h>

namespace Inkscape
{
namespace UI
{
namespace Dialogs
{


/*#################################
# F I L E    O P E N
#################################*/

struct FileOpenNativeData_def {
    OPENFILENAME ofn;
    gchar fnbuf[4096];
};

FileOpenDialog::FileOpenDialog(
    const char *dir, FileDialogType fileTypes, const char *title) {

    nativeData = (FileOpenNativeData *)
            g_malloc(sizeof (FileOpenNativeData));
    if ( !nativeData ) {
        // do we want exceptions?
        return;
        }
    gchar *filter = "";
    if ( fileTypes == SVG_TYPES )
        filter = "SVG files\0*.svg;*.svgz\0All files\0*\0";
    else if ( fileTypes == IMPORT_TYPES )
        filter = "Image files\0*.svg;*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.xpm\0"
                 "SVG files\0*.svg\0"
                 "All files\0*\0";

    nativeData->fnbuf[0] = '\0';

    OPENFILENAME ofn = {
	    sizeof (OPENFILENAME),
	    NULL,                       // hwndOwner
	    NULL,                       // hInstance
	    (const CHAR *)filter,       // lpstrFilter
	    NULL,                       // lpstrCustomFilter
	    0,                          // nMaxCustFilter 
	    1,                          // nFilterIndex
	    nativeData->fnbuf,          // lpstrFile
	    sizeof (nativeData->fnbuf), // nMaxFile
	    NULL,                       // lpstrFileTitle
	    0,                          // nMaxFileTitle
	    (const CHAR *)dir,          // lpstrInitialDir
	    (const CHAR *)title,        // lpstrTitle
	    OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, // Flags
	    0,                          // nFileOffset
	    0,                          // nFileExtension
	    NULL,                       // lpstrDefExt
	    0,                          // lCustData
	    NULL,                       // lpfnHook
	    NULL                        // lpTemplateName
        };
    nativeData->ofn = ofn;

}



FileOpenDialog::~FileOpenDialog() {

    //do any cleanup here
    if ( nativeData )
        g_free( nativeData );

}



char *
FileOpenDialog::show() {

    if ( !nativeData ) {
        //error
        return NULL;
        }

    int retval  = GetOpenFileName (&(nativeData->ofn));

    if ( !retval ) {
        //int errcode = CommDlgExtendedError();
        return NULL;
        }
    return g_strdup (nativeData->fnbuf);

}



/*#################################
# F I L E    S A V E
#################################*/

struct FileSaveNativeData_def {
    OPENFILENAME ofn;
    gchar fnbuf[4096];
};



FileSaveDialog::FileSaveDialog(
   const char *dir, FileDialogType fileTypes, const char *title) {

    nativeData = (FileSaveNativeData *)
            g_malloc(sizeof (FileSaveNativeData));
    if ( !nativeData ) {
        //do we want exceptions?
        return;
        }

    gchar *filter = "";
    if ( fileTypes == SVG_TYPES )
        filter = "SVG with extension namespaces\0*\0Standard SVG\0*\0";
    else if ( fileTypes == EXPORT_TYPES )
        filter = "Image files\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.xpm\0"
                 "SVG files\0*.svg\0"
                 "All files\0*\0";

    nativeData->fnbuf[0] = '\0';

    OPENFILENAME ofn = {
	    sizeof (OPENFILENAME),
	    NULL,                       // hwndOwner
	    NULL,                       // hInstance
	    "SVG with extension namespaces\0*\0Standard SVG\0*\0", // lpstrFilter
	    NULL,                       // lpstrCustomFilter
	    0,                          // nMaxCustFilter 
	    1,                          // nFilterIndex
	    nativeData->fnbuf,          // lpstrFile
	    sizeof (nativeData->fnbuf), // nMaxFile
	    NULL,                       // lpstrFileTitle
	    0,                          // nMaxFileTitle
	    (const CHAR *)dir,          // lpstrInitialDir
	    "Save document to file",    // lpstrTitle
	    OFN_HIDEREADONLY,           // Flags
	    0,                          // nFileOffset
	    0,                          // nFileExtension
	    NULL,                       // lpstrDefExt
	    0,                          // lCustData
	    NULL,                       // lpfnHook
	    NULL                        // lpTemplateName
        };
    nativeData->ofn = ofn;

}

FileSaveDialog::~FileSaveDialog() {

    //do any cleanup here
    if ( nativeData )
        g_free( nativeData );
}

char *
FileSaveDialog::show() {

    if (!nativeData)
        return NULL;
    int retval = GetSaveFileName (&(nativeData->ofn));
    if (!retval) {
        //int errcode = CommDlgExtendedError();
        return NULL;
        }

    if (nativeData->ofn.nFilterIndex != 2)
        selectionType = SVG_NAMESPACE_WITH_EXTENSIONS;
    else
        selectionType = SVG_NAMESPACE;

    return g_strdup (nativeData->fnbuf);

}








}; //namespace Dialogs
}; //namespace UI
}; //namespace Inkscape




