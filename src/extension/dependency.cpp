/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <iostream>
#include <stdio.h>

#include <glibmm/module.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>

#include "config.h"
#include "path-prefix.h"

#include "dependency.h"
#include "extension.h"
#include "db.h"

namespace Inkscape {
namespace Extension {

gchar const * Dependency::_type_str[] = {
    "executable",
    "file",
    "extension",
    "plugin"
};

gchar const * Dependency::_location_str[] = {
    "path",
    "extensions",
    "absolute"
};

/**
    \brief   Create a dependency using an XML definition
    \param   in_repr   XML definition of the dependency

    This function mostly looks for the 'location' and 'type' attributes
    and turns them into the enums of the same name.  This makes things
    a little bit easier to use later.  Also, a pointer to the core
    content is pulled out -- also to make things easier.
*/
Dependency::Dependency (SPRepr * in_repr)
{
    _type = TYPE_CNT;
    _location = LOCATION_PATH;
    _repr = in_repr;
    _string = NULL;

    sp_repr_ref(_repr);

    const gchar * location = sp_repr_attr(_repr, "location");
    for (int i = 0; i < LOCATION_CNT && location != NULL; i++) {
        if (!strcmp(location, _location_str[i])) {
            _location = (location_t)i;
            break;
        }
    }

    const gchar * type = sp_repr_attr(_repr, "type");
    for (int i = 0; i < TYPE_CNT && type != NULL; i++) {
        if (!strcmp(type, _type_str[i])) {
            _type = (type_t)i;
            break;
        }
    }

    _string = sp_repr_content(sp_repr_children(_repr));

    return;
}

/**
    \brief   This depenency is not longer needed

    Unreference the XML structure.
*/
Dependency::~Dependency (void)
{
    sp_repr_unref(_repr);
}

/**
    \brief   Check if the dependency passes.
    \return  Whether or not the dependency passes.

    This function depends largely on all of the enums.  The first level
    that is evaluted is the \c _type.

    If the type is \c TYPE_EXTENSION then the id for the extension is
    looked up in the database.  If the extension is found, and it is
    not deactivated, the dependency passes.

    If the type is \c TYPE_PLUGIN then the path for the plugin is found
    using the Glib::Module routines.  When the path is found, then there
    is a check to see if the file exists using the \c file_test function.

    If the type is \c TYPE_EXECUTABLE or \c TYPE_FILE things are getting
    even more interesting because now the \c _location variable is also
    taken into account.  First, the difference between the two is that
    the file test for \c TYPE_EXECUTABLE also tests to make sure the
    file is executable, besides checking that it exists.

    If the \c _location is \c LOCATION_EXTENSIONS then the \c INKSCAPE_EXTENSIONDIR
    is put on the front of the string with \c build_filename.  Then the
    appopriate filetest is run.

    If the \c _location is \c LOCATION_ABSOLUTE then the file test is
    run directly on the string.

    If the \c _location is \c LOCATION_PATH or not specified then the
    path is used to find the file.  Each entry in the path is stepped
    through, attached to the string, and then tested.  If the file is
    found then a TRUE is returned.  If we get all the way through the
    path then a FALSE is returned, the command could not be found.
*/
bool
Dependency::check (void) const
{
    // std::cout << "Checking: " << *this << std::endl;

    if (_string == NULL) return FALSE;

    switch (_type) {
        case TYPE_EXTENSION: {
            Extension * myext = db.get(_string);
            if (myext == NULL) return FALSE;
            if (myext->deactivated()) return FALSE;
            break;
        }
        case TYPE_PLUGIN: {
            if (!Glib::Module::get_supported()) {
                return FALSE;
            }

            std::string path = Glib::Module::build_path(INKSCAPE_PLUGINDIR, _string);
            if (!Glib::file_test(path, Glib::FILE_TEST_EXISTS))
                return FALSE;
            break;
        }
        case TYPE_EXECUTABLE:
        case TYPE_FILE: {
            Glib::FileTest filetest = Glib::FILE_TEST_EXISTS;
            if (_type == TYPE_EXECUTABLE) {
                filetest |= Glib::FILE_TEST_IS_EXECUTABLE;
            }

            std::string location(_string);
            switch (_location) {
                case LOCATION_EXTENSIONS: {
                    location = Glib::build_filename(INKSCAPE_EXTENSIONDIR, location);
                } /* PASS THROUGH!!! */
                case LOCATION_ABSOLUTE: {
                    if (!Glib::file_test(location, filetest)) {
                        // std::cout << "Failing on location: " << location << std::endl;
                        return FALSE;
                    }
                    break;
                }
                /* The default case is to look in the path */
                case LOCATION_PATH:
                default: {
                    gchar * path = g_strdup(g_getenv("PATH"));

                    if (path == NULL) {
                        /* There is no `PATH' in the environment.
                           The default search path is the current directory */
                        path = g_strdup(G_SEARCHPATH_SEPARATOR_S);
                    }

                    gchar * orig_path = path;

                    for (; path != NULL;) {
                        gchar * local_path;
                        gchar * final_name;

                        local_path = path;
                        path = g_utf8_strchr(path, -1, G_SEARCHPATH_SEPARATOR);
                        if (path == NULL) {
                            break;
                        }
                        /* Not sure whether this is UTF8 happy, but it would seem
                           like it considering that I'm searching (and finding)
                           the ':' character */
                        if (path != local_path && path != NULL) {
                            path[0] = '\0';
                            path++;
                        } else {
                            path = NULL;
                        }

                        if (local_path == '\0') {
                            final_name = g_strdup(_string);
                        } else {
                            final_name = g_build_filename(local_path, _string, NULL);
                        }

                        if (Glib::file_test(final_name, filetest)) {
                            g_free(final_name);
                            g_free(orig_path);
                            return TRUE;
                        }

                        g_free(final_name);
                    }

                    g_free(orig_path);
                    return FALSE; /* Reverse logic in this one */
                    break;
                }
            } /* switch _location */
            break;
        } /* TYPE_FILE, TYPE_EXECUTABLE */
        default:
            return FALSE;
    } /* switch _type */

    return TRUE;
}

/**
    \brief   Print out a dependency to a string.
*/
std::ostream &
operator<< (std::ostream &out_file, const Dependency & in_dep)
{
    out_file << "Dependency::";
    out_file << "  type: " << in_dep._type_str[in_dep._type];
    out_file << "  location: " << in_dep._location_str[in_dep._location];
    out_file << "  string: " << in_dep._string;

    return out_file;
}

};}; /* namespace Inkscape, Extension */

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
