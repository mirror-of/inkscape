
#ifndef __FILEPATH_H__
#define __FILEPATH_H__

/*#########################################################################
## $Id$
###########################################################################

Inkscape - Open Source Scalable Vector Graphics Editor
Copyright (C) 2004 inkscape.org

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Bugs/suggestions can be sent to bryce@users.sourceforge.net

#########################################################################*/


#include <glib.h>

namespace Inkscape
{
#ifdef MAX_PATH
const int MAXPATH = MAX_PATH;
#else
#ifdef PATH_MAX
const int MAXPATH = PATH_MAX;
#else
const int MAXPATH = 256;
#endif
#endif


/**
 * This class is a utility for finding files from
 * inkscape.  In the future, this will be a good place to
 * congregate all of the code that needs to find or open a file
 * somewhere.
 */
class FilePath
{
public:
enum
{
PIXMAP,
DATA,
GLADE,
MODULES
};


/**
 * Return the absolute path name of the Inkscape
 * executable.  Returns NULL if not found
 * @return pointer to name if successful.  Must be
 * freed by g_free().  NULL otherwise.
 */
static gchar *getExecutablePath(void);

/**
 * Return the canonical version of a path.
 * The path will only use forward-slashes.
 * For example,  c:\inkscape\data\icons.svg will
 * become c:/inkscape/data/icons.svg
 * @param path the path to convert into canonical form.
 * @return pointer to converted path if successful. Must be
 * freed by g_free().  NULL otherwise.
 */
static gchar *getCanonicalPath(gchar *path);

/**
 * Return the native version of a canonical path.
 * The path will be a format dependent on the current
 * platform.
 * For example,  c:/inkscape/data/icons.svg will
 * become c:\inkscape\data\icons.svg 
 * @param path the path to convert into native form.
 * @return pointer to converted path if successful. Must be
 * freed by g_free().  NULL otherwise.
 */
static gchar *getNativePath(gchar *path);


/**
 * Returns the complete path, of one path relative to another.
 * In addition to simply concatenating the paths, it also
 * does a bit op path reduction.  For example,  // becomes /
 * and  /./ becomes /
 * @param head the anchor path with which the second is relative
 * @param tail the subpath to the anchor
 * @return pointer to the generated path if successful.  Must be
 * freed by g_free().  NULL otherwise.
 */
static gchar *getRelativePath(gchar *head, gchar *tail);


/**
 * Returns a path relative to the executable for the given file name
 * and resource type.
 * @param fileName the name of the resource (data file, pixmap, etc) for
 * which to find the complete name.
 * @param type one of the enumerated types (FilePath::PIXMAP, FilePath::DATA, etc..)
 * which describe this file
 * @return pointer to the generated path if successful.  Must be
 * freed by g_free().  NULL otherwise.
 */
static gchar *getResourcePath(gchar *fileName, gint type);


/**
 * Returns a native path relative to the executable for the given file name
 * and resource type.  Same as getResourcePath(), except that on odd
 * machines like Mac or Windows, the name will be converted to what the
 * architecture wants.
 * @param fileName the name of the resource (data file, pixmap, etc) for
 * which to find the complete name.
 * @param type one of the enumerated types (FilePath::PIXMAP, FilePath::DATA, etc..)
 * which describe this file
 * @return pointer to the generated path if successful.  Must be
 * freed by g_free().  NULL otherwise.
 */
static gchar *getNativeResourcePath(gchar *fileName, gint type);




/**
 * Returns the location where the user's profile is stored, or an approximation
 * of it.   Tries  (1) what Glib says  (2) (if windows) the Shell's value
 * (3) The executable's dir, (4) The current dir.
 * @return pointer to the generated path if successful.  Must be
 * freed by g_free().  NULL otherwise.
 */
static gchar *getProfilePath(void);

/**
 * Same as getProfilePath(), but converted to the local architecture.
 * @return pointer to the generated path if successful.  Must be
 * freed by g_free().  NULL otherwise.
 */
static gchar *getNativeProfilePath(void);


/**
 * Returns the full path name of the preferences.xml file.
 * @return pointer to the generated path if successful.  Must be
 * freed by g_free().  NULL otherwise.
 */
static gchar * getPreferencesPath(void);

/**
 * Returns the native version of the full path name of the preferences.xml file.
 * @return pointer to the generated path if successful.  Must be
 * freed by g_free().  NULL otherwise.
 */
static gchar * getNativePreferencesPath(void);


/**
 * Constructor
 */
FilePath(void);


/**
 * Destructor
 */
virtual ~FilePath(void);


private:



}; // class application



} // namespace Inkscape



#endif /* __FILEPATH_H__ */
