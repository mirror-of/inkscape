
/*
 * System abstraction utility routines
 *
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glib.h>
#include <gtkmm.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkmessagedialog.h>

#include "prefs-utils.h"
#include "sys.h"

#ifdef WIN32

// For now to get at is_os_wide().
#include "extension/internal/win32.h"
using Inkscape::Extension::Internal::PrintWin32;

#endif

//#define INK_DUMP_FILENAME_CONV 1
#undef INK_DUMP_FILENAME_CONV

//#define INK_DUMP_FOPEN 1
#undef INK_DUMP_FOPEN

void dump_str(gchar const *str, gchar const *prefix);
void dump_ustr(Glib::ustring const &ustr);

extern guint update_in_progress;


#define DEBUG_MESSAGE(key, ...) \
{\
    gint dump = prefs_get_int_attribute_limited("options.bulia", #key, 0, 0, 1);\
    gint dumpD = prefs_get_int_attribute_limited("options.bulia", #key"D", 0, 0, 1);\
    gint dumpD2 = prefs_get_int_attribute_limited("options.bulia", #key"D2", 0, 0, 1);\
    dumpD &= ( (update_in_progress == 0) || dumpD2 );\
    if ( dump )\
    {\
        g_message( __VA_ARGS__ );\
\
    }\
    if ( dumpD )\
    {\
        GtkWidget *dialog = gtk_message_dialog_new(NULL,\
                                                   GTK_DIALOG_DESTROY_WITH_PARENT, \
                                                   GTK_MESSAGE_INFO,    \
                                                   GTK_BUTTONS_OK,      \
                                                   __VA_ARGS__          \
                                                   );\
        g_signal_connect_swapped(dialog, "response",\
                                 G_CALLBACK(gtk_widget_destroy),        \
                                 dialog);                               \
        gtk_widget_show_all( dialog );\
    }\
}




void Inkscape::IO::dump_fopen_call( char const *utf8name, char const *id )
{
#ifdef INK_DUMP_FOPEN
    Glib::ustring str;
    for ( int i = 0; utf8name[i]; i++ )
    {
        if ( utf8name[i] == '\\' )
        {
            str += "\\\\";
        }
        else if ( (utf8name[i] >= 0x20) && ((0x0ff & utf8name[i]) <= 0x7f) )
        {
            str += utf8name[i];
        }
        else
        {
            gchar tmp[32];
            g_snprintf( tmp, sizeof(tmp), "\\x%02x", (0x0ff & utf8name[i]) );
            str += tmp;
        }
    }
    g_message( "fopen call %s for [%s]", id, str.data() );
#endif
}

FILE *Inkscape::IO::fopen_utf8name( char const *utf8name, char const *mode )
{
    static gint counter = 0;
    FILE* fp = NULL;

    DEBUG_MESSAGE( dumpOne, "entering fopen_utf8name( '%s', '%s' )[%d]", utf8name, mode, (counter++) );

#ifndef WIN32
    DEBUG_MESSAGE( dumpOne, "           STEP 0              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
    gchar *filename = g_filename_from_utf8( utf8name, -1, NULL, NULL, NULL );
    if ( filename )
    {
        DEBUG_MESSAGE( dumpOne, "           STEP 1              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        fp = std::fopen(filename, mode);
        DEBUG_MESSAGE( dumpOne, "           STEP 2              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        g_free(filename);
        DEBUG_MESSAGE( dumpOne, "           STEP 3              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        filename = 0;
    }
#else
    Glib::ustring how( mode );
    how.append("b");
    DEBUG_MESSAGE( dumpOne, "   calling is_os_wide()       ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
    if ( PrintWin32::is_os_wide() )
    {
        DEBUG_MESSAGE( dumpOne, "           is_os_wide() true   ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        gunichar2 *wideName = g_utf8_to_utf16( utf8name, -1, NULL, NULL, NULL );
        DEBUG_MESSAGE( dumpOne, "           STEP 1              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        if ( wideName )
        {
            DEBUG_MESSAGE( dumpOne, "           STEP 2              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
            gunichar2 *wideMode = g_utf8_to_utf16( how.c_str(), -1, NULL, NULL, NULL );
            DEBUG_MESSAGE( dumpOne, "           STEP 3              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
            if ( wideMode )
            {
                DEBUG_MESSAGE( dumpOne, "           STEP 4              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
                fp = _wfopen( (wchar_t*)wideName, (wchar_t*)wideMode );
                DEBUG_MESSAGE( dumpOne, "           STEP 5              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
                g_free( wideMode );
                DEBUG_MESSAGE( dumpOne, "           STEP 6              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
                wideMode = 0;
            }
            else
            {
                DEBUG_MESSAGE( dumpOne, "           STEP 7              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
                g_message("Unable to convert mode from UTF-8 to UTF-16");
            }
            DEBUG_MESSAGE( dumpOne, "           STEP 8              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
            g_free( wideName );
            DEBUG_MESSAGE( dumpOne, "           STEP 9              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
            wideName = 0;
        }
        else
        {
            DEBUG_MESSAGE( dumpOne, "           STEP A              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
            g_message("Unable to convert filename from UTF-8 to UTF-16");
        }
    }
    else
    {
        DEBUG_MESSAGE( dumpOne, "           is_os_wide() false  ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        gchar *filename = g_filename_from_utf8( utf8name, -1, NULL, NULL, NULL );
        DEBUG_MESSAGE( dumpOne, "           STEP 1              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        fp = std::fopen(filename, how.c_str());
        DEBUG_MESSAGE( dumpOne, "           STEP 2              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        g_free(filename);
        DEBUG_MESSAGE( dumpOne, "           STEP 3              ( '%s', '%s' )[%d]", utf8name, mode, (counter++) );
        filename = 0;
    }
#endif

    DEBUG_MESSAGE( dumpOne, "leaving fopen_utf8name( '%s', '%s' )[%d]", utf8name, mode, (counter++) );

    return fp;
}


int Inkscape::IO::mkdir_utf8name( char const *utf8name )
{
    static gint counter = 0;
    int retval = -1;

    DEBUG_MESSAGE( dumpMk, "entering mkdir_utf8name( '%s' )[%d]", utf8name, (counter++) );

#ifndef WIN32
    DEBUG_MESSAGE( dumpMk, "           STEP 0              ( '%s' )[%d]", utf8name, (counter++) );
    gchar *filename = g_filename_from_utf8( utf8name, -1, NULL, NULL, NULL );
    if ( filename )
    {
        DEBUG_MESSAGE( dumpMk, "           STEP 1              ( '%s' )[%d]", utf8name, (counter++) );
        retval = ::mkdir(filename, S_IRWXU | S_IRGRP | S_IXGRP);
        DEBUG_MESSAGE( dumpMk, "           STEP 2              ( '%s' )[%d]", utf8name, (counter++) );
        g_free(filename);
        DEBUG_MESSAGE( dumpMk, "           STEP 3              ( '%s' )[%d]", utf8name, (counter++) );
        filename = 0;
    }
#else
    DEBUG_MESSAGE( dumpMk, "   calling is_os_wide()       ( '%s' )[%d]", utf8name, (counter++) );
    if ( PrintWin32::is_os_wide() )
    {
        DEBUG_MESSAGE( dumpMk, "           is_os_wide() true   ( '%s' )[%d]", utf8name, (counter++) );
        gunichar2 *wideName = g_utf8_to_utf16( utf8name, -1, NULL, NULL, NULL );
        DEBUG_MESSAGE( dumpMk, "           STEP 1              ( '%s' )[%d]", utf8name, (counter++) );
        if ( wideName )
        {
            DEBUG_MESSAGE( dumpMk, "           STEP 2              ( '%s' )[%d]", utf8name, (counter++) );
            retval = _wmkdir( (wchar_t*)wideName );
            DEBUG_MESSAGE( dumpMk, "           STEP 3              ( '%s' )[%d]", utf8name, (counter++) );
            g_free( wideName );
            DEBUG_MESSAGE( dumpMk, "           STEP 4              ( '%s' )[%d]", utf8name, (counter++) );
            wideName = 0;
        }
        else
        {
            DEBUG_MESSAGE( dumpMk, "           STEP 5              ( '%s' )[%d]", utf8name, (counter++) );
            g_message("Unable to convert filename from UTF-8 to UTF-16");
        }
    }
    else
    {
        DEBUG_MESSAGE( dumpMk, "           is_os_wide() false  ( '%s' )[%d]", utf8name, (counter++) );
        gchar *filename = g_filename_from_utf8( utf8name, -1, NULL, NULL, NULL );
        DEBUG_MESSAGE( dumpMk, "           STEP 1              ( '%s' )[%d]", utf8name, (counter++) );
        retval = ::mkdir(filename);
        DEBUG_MESSAGE( dumpMk, "           STEP 2              ( '%s' )[%d]", utf8name, (counter++) );
        g_free(filename);
        DEBUG_MESSAGE( dumpMk, "           STEP 3              ( '%s' )[%d]", utf8name, (counter++) );
        filename = 0;
    }
#endif

    DEBUG_MESSAGE( dumpMk, "leaving mkdir_utf8name( '%s' )[%d]", utf8name, (counter++) );

    return retval;
}

bool Inkscape::IO::file_test( char const *utf8name, GFileTest test )
{
    bool exists;
    gchar *filename;

    if (!g_utf8_validate(utf8name, -1, NULL)) {
        filename = g_strdup(utf8name);
    }
    else {
        filename = g_filename_from_utf8 ( utf8name, -1, NULL, NULL, NULL );
    }
    exists = g_file_test (filename, test);
    g_free(filename);
    return exists;
}

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
