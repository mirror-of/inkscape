
#include "filedialog.h"
#include "modules/win32.h"

#include <windows.h>

#include <glib.h>

#define UNSAFE_SCRATCH_BUFFER_SIZE 4096

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
    char *dir;
    char *filter;
    char *title;
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

    if ( !dir )
        dir = "";
    nativeData->dir    = g_strdup(dir);
    nativeData->filter = g_strdup(filter);
    nativeData->title  = g_strdup(title);

}



FileOpenDialog::~FileOpenDialog() {

    //do any cleanup here
    if ( nativeData ) {
        g_free(nativeData->dir);
        g_free(nativeData->filter);
        g_free(nativeData->title);
        g_free( nativeData );
    }
}



char *
FileOpenDialog::show() {

    if ( !nativeData ) {
        //error
        return NULL;
    }

    gchar *result = NULL;
    gint  retval  = FALSE;

    //Jon's UNICODE patch
    if ( sp_win32_is_os_wide() ) {
        gunichar2 fnbufW[UNSAFE_SCRATCH_BUFFER_SIZE * sizeof(gunichar2)] = {0};
        gunichar2* dirW    = 
            g_utf8_to_utf16( nativeData->dir,    -1, NULL, NULL, NULL );
        gunichar2* filterW = 
            g_utf8_to_utf16( nativeData->filter, -1, NULL, NULL, NULL );
        gunichar2* titleW  = 
            g_utf8_to_utf16( nativeData->title,  -1, NULL, NULL, NULL );
        OPENFILENAMEW ofn = {
            sizeof (OPENFILENAMEW),
            NULL,                   // hwndOwner
            NULL,                   // hInstance
            (const WCHAR *)filterW, // lpstrFilter
            NULL,                   // lpstrCustomFilter
            0,                      // nMaxCustFilter
            1,                      // nFilterIndex
            (WCHAR *)fnbufW,        // lpstrFile
            sizeof (fnbufW) / sizeof(WCHAR), // nMaxFile
            NULL,                   // lpstrFileTitle
            0,                      // nMaxFileTitle
            (const WCHAR *)dirW,    // lpstrInitialDir
            (const WCHAR *)titleW,  // lpstrTitle
            OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, // Flags
            0,                      // nFileOffset
            0,                      // nFileExtension
            NULL,                   // lpstrDefExt
            0,                      // lCustData
            NULL,                   // lpfnHook
            NULL                    // lpTemplateName
        };

        retval = GetOpenFileNameW (&ofn);
        if (retval)
            result = g_utf16_to_utf8( fnbufW, -1, NULL, NULL, NULL );

        g_free( dirW );
        g_free( filterW );
        g_free( titleW );

    } else {
        gchar *dir    = nativeData->dir;
        gchar *filter = nativeData->filter;
        gchar *title  = nativeData->title;
        gchar fnbuf[UNSAFE_SCRATCH_BUFFER_SIZE] = {0};

        OPENFILENAMEA ofn = {
            sizeof (OPENFILENAMEA),
            NULL,                  // hwndOwner
            NULL,                  // hInstance
            (const CHAR *)filter,  // lpstrFilter
            NULL,                  // lpstrCustomFilter
            0,                     // nMaxCustFilter
            1,                     // nFilterIndex
            fnbuf,                 // lpstrFile
            sizeof (fnbuf),        // nMaxFile
            NULL,                  // lpstrFileTitle
            0,                     // nMaxFileTitle
            (const CHAR *)dir,     // lpstrInitialDir
            (const CHAR *)title,   // lpstrTitle
            OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, // Flags
            0,                     // nFileOffset
            0,                     // nFileExtension
            NULL,                  // lpstrDefExt
            0,                     // lCustData
            NULL,                  // lpfnHook
            NULL                   // lpTemplateName
        };

        retval = GetOpenFileNameA (&ofn);
        if ( retval )
            result = g_strdup(fnbuf);
    }

    if ( !retval ) {
        //int errcode = CommDlgExtendedError();
        return NULL;
    }

    return result;

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




