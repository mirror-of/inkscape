#ifndef __FILE_DIALOG_H__
#define __FILE_DIALOG_H__


#include <glib.h>
#include <extension/db.h>

namespace Inkscape
{
namespace UI
{
namespace Dialogs
{


/**
 * Used for setting filters and options, and
 * reading them back from user selections.
 */
typedef enum {
    SVG_TYPES,
    IMPORT_TYPES,
    EXPORT_TYPES
    } FileDialogType;

/**
 * Used for returning the type selected in a SaveAs
 */
typedef enum {
    SVG_NAMESPACE,
    SVG_NAMESPACE_WITH_EXTENSIONS
    } FileDialogSelectionType;

/**
 * Architecture-specific data
 */
typedef struct FileOpenNativeData_def FileOpenNativeData;


/**
 * This class provides an implementation-independent API for
 * file "Open" dialogs.  Using a standard interface obviates the need
 * for ugly #ifdefs in file open code
 */
class FileOpenDialog
{
public:


    /**
     * Constructor.
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     */
    FileOpenDialog(const char *path, FileDialogType fileTypes, const char *title);


    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    ~FileOpenDialog();

    /**
     * Show an OpenFile file selector.
     * @return the selected path if user selected one, else NULL
     */
    bool show();

    /**
     * Return the 'key' (filetype) of the selection, if any
     * @return a pointer to a string if successful (which must
     * be later freed with g_free(), else NULL.
     */
	Inkscape::Extension::Extension * getSelectionType()
        { return extension; }
	gchar * getFilename (void) { return filename; }

private:

    /**
     * The extension to use to write this file
     */
	Inkscape::Extension::Extension * extension;

	/**
	 * Filename that was given
	 */
	gchar * filename;


    /**
     * Architecture-specific data
     */
    FileOpenNativeData *nativeData;

}; //FileOpenDialog






/**
 * Architecture-specific data
 */
typedef struct FileSaveNativeData_def FileSaveNativeData;

/**
 * This class provides an implementation-independent API for
 * file "Save" dialogs.
 */
class FileSaveDialog
{
public:

    /**
     * Constructor.
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     * @param key a list of file types from which the user can select
     */
    FileSaveDialog(const char *path, FileDialogType fileTypes, const char *title, const char * default_key);


    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    ~FileSaveDialog();


    /**
     * Show an SaveAs file selector.
     * @return the selected path if user selected one, else NULL
     */
    bool show();

    /**
     * Return the 'key' (filetype) of the selection, if any
     * @return a pointer to a string if successful (which must
     * be later freed with g_free(), else NULL.
     */
	Inkscape::Extension::Extension * getSelectionType()
        { return extension; }
	gchar * getFilename (void) { return filename; }

private:

    /**
     * The extension to use to write this file
     */
	Inkscape::Extension::Extension * extension;

	/**
	 * Filename that was given
	 */
	gchar * filename;

    /**
     * Architecture-specific data
     */
    FileSaveNativeData *nativeData;

}; //FileSaveDialog


}; //namespace Dialogs
}; //namespace UI
}; //namespace Inkscape





#endif /* __FILE_DIALOG_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
