/*
 * Code for handling extensions (i.e., scripts)
 *
 * Authors:
 *   Bryce Harrington <bryce@osdl.org>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define __INKSCAPE_EXTENSION_IMPLEMENTATION_SCRIPT_C__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "path-prefix.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <svg/svg.h>
#include <interface.h>
#include <document.h>
#include <inkscape.h>
#include <desktop.h>
#include <desktop-handles.h>
#include <selection.h>
#include <file.h>
#include <sp-object.h>
#include <sp-namedview.h>

#include "../system.h"

#include <extension/extension.h>
#include <extension/db.h>
#include "implementation.h"
#include "script.h"

#ifdef WIN32
#include <windows.h>
#endif

/** This is the command buffer that gets allocated from the stack */
#define BUFSIZE (255)

/* Namespaces */
namespace Inkscape {
namespace Extension {
namespace Implementation {

/* Real functions */
/**
    \return    A script object
    \brief     This function creates a script object and sets up the
               variables.

   This function just sets the command to NULL.  It should get built
   officially in the load function.  This allows for less allocation
   of memory in the unloaded state.
*/
Script::Script() :
    Implementation(),
    command(NULL),
    helper_extension(NULL)
{
}

/**
    \return    A string with the complete string with the relative directory expanded
    \brief     This function takes in a Repr that contains a reldir entry
               and returns that data with the relative directory expanded.
               Mostly it is here so that relative directories all get used
               the same way.
    \param     reprin   The SPRepr with the reldir in it.

    Basically this function looks at an attribute of the Repr, and makes
    a decision based on that.  Currently, it is only working with the
    'extensions' relative directory, but there will be more of them.
    One thing to notice is that this function always returns an allocated
    string.  This means that the caller of this function can always
    free what they are given (and should do it too!).
*/
gchar *
Script::solve_reldir (SPRepr * reprin) {
    const gchar * reldir;

    reldir = sp_repr_attr(reprin, "reldir");

    if (reldir == NULL) {
        return g_strdup(sp_repr_content(sp_repr_children(reprin)));
    }

    if (!strcmp(reldir, "extensions")) {
        return g_build_filename(INKSCAPE_EXTENSIONDIR, sp_repr_content(sp_repr_children(reprin)), NULL);
    } else {
        return g_strdup(sp_repr_content(sp_repr_children(reprin)));
    }

    return NULL;
}

/**
    \return   Whether the command given exists, including in the path
    \brief    This function is used to find out if something exists for
              the check command.  It can look in the path if required.
    \param    command   The command or file that should be looked for

    The first thing that this function does is check to see if the
    incoming file name has a directory delimiter in it.  This would
    mean that it wants to control the directories, and should be
    used directly.

    If not, the path is used.  Each entry in the path is stepped through,
    attached to the string, and then tested.  If the file is found
    then a TRUE is returned.  If we get all the way through the path
    then a FALSE is returned, the command could not be found.
*/
bool
Script::check_existance (const gchar * command)
{
    gchar * path;
    gchar * orig_path;

    if (*command == '\0') {
        /* We check the simple case first. */
        return FALSE;
    }

    if (g_utf8_strchr (command, -1, G_DIR_SEPARATOR) != NULL) {
        /* Don't search when it contains a slash.  */
        if (g_file_test(command, G_FILE_TEST_EXISTS))
            return TRUE;
        else
            return FALSE;
    }


    path = g_strdup(g_getenv("PATH"));

    if (path == NULL) {
        /* There is no `PATH' in the environment.
           The default search path is the current directory */
        path = g_strdup(G_SEARCHPATH_SEPARATOR_S);
    }
    orig_path = path;

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
            final_name = g_strdup(command);
        } else {
            final_name = g_build_filename(local_path, command, NULL);
        }

        if (g_file_test(final_name, G_FILE_TEST_EXISTS)) {
            g_free(final_name);
            g_free(orig_path);
            return TRUE;
        }

        g_free(final_name);
    }

    return FALSE;
}

/**
    \return   none
    \brief    This function 'loads' an extention, basically it determines
              the full command for the extention and stores that.
    \param    module  The extention to be loaded.

    The most difficult part about this function is finding the actual
    command through all of the Reprs.  Basically it is hidden down a
    couple of layers, and so the code has to move down too.  When
    the command is actually found, it has its relative directory
    solved.

    At that point all of the loops are exited, and there is an
    if statement to make sure they didn't exit because of not finding
    the command.  If that's the case, the extention doesn't get loaded
    and should error out at a higher level.
*/

bool
Script::load (Inkscape::Extension::Extension * module)
{
    SPRepr * child_repr;
    gchar * command_text = NULL;

    if (module->loaded()) {
        return TRUE;
    }

    helper_extension = NULL;

    /* This should probably check to find the executable... */
    child_repr = sp_repr_children(module->get_repr());
    while (child_repr != NULL) {
        if (!strcmp(sp_repr_name(child_repr), "script")) {
            child_repr = sp_repr_children(child_repr);
            while (child_repr != NULL) {
                if (!strcmp(sp_repr_name(child_repr), "command")) {
                    command_text = solve_reldir(child_repr);
                }
                if (!strcmp(sp_repr_name(child_repr), "helper_extension")) {
                    helper_extension = g_strdup(sp_repr_content(sp_repr_children(child_repr)));
                }
                child_repr = sp_repr_next(child_repr);
            }

            break;
        }
        child_repr = sp_repr_next(child_repr);
    }

    g_return_val_if_fail(command_text != NULL, FALSE);

    g_free(command);
    command = command_text;

    return TRUE;
}

/**
    \return   None.
    \brief    Unload this puppy!
    \param    module  Extension to be unloaded.

    This function just sets the module to unloaded.  It free's the
    command if it has been allocated.
*/
void
Script::unload (Inkscape::Extension::Extension * module)
{
    g_free(command);
    g_free(helper_extension);
    return;
}

/**
    \return   Whether the check passed or not
    \brief    Check every dependency that was given to make sure we should keep this extension
    \param    module  The Extension in question

*/
bool
Script::check (Inkscape::Extension::Extension * module)
{
    SPRepr * child_repr;
    gchar * command_text;

    child_repr = sp_repr_children(module->get_repr());
    while (child_repr != NULL) {
        if (!strcmp(sp_repr_name(child_repr), "script")) {
            child_repr = sp_repr_children(child_repr);
            while (child_repr != NULL) {
                if (!strcmp(sp_repr_name(child_repr), "check")) {
                    command_text = solve_reldir(child_repr);
                    
                    if (command_text != NULL) {
                    /* I've got the command */
                        bool existance;

                        existance = check_existance(command_text);
                                                // printf("%s exists %s\n", command_text, existance ? "TRUE" : "FALSE");
                        g_free(command_text);
                        if (!existance)
                            return FALSE;
                    }
                }

                if (!strcmp(sp_repr_name(child_repr), "helper_extension")) {
                    const gchar * helper;

                    helper = sp_repr_content(sp_repr_children(child_repr));
                    if (Inkscape::Extension::db.get(helper) == NULL) {
                        //printf("Couldn't find helper\n");
                        return FALSE;
                    }
                }

                child_repr = sp_repr_next(child_repr);
            }

            break;
        }
        child_repr = sp_repr_next(child_repr);
    }

    return TRUE;
}

/**
    \return   A dialog for preferences
    \brief    A stub funtion right now
    \param    module    Module who's preferences need getting
    \param    filename  Hey, the file you're getting might be important

    This function should really do something, right now it doesn't.
*/
GtkDialog *
Script::prefs_input (Inkscape::Extension::Input * module, const gchar * filename)
{
    /* Sad, this should really do something... */
    return NULL;
}

/**
    \return   A dialog for preferences
    \brief    A stub funtion right now
    \param    module    Module whose preferences need getting

    This function should really do something, right now it doesn't.
*/
GtkDialog *
Script::prefs_output (Inkscape::Extension::Output * module)
{
    /* Sad, this should really do something... */
    return NULL;
}

/**
    \return   A dialog for preferences
    \brief    A stub funtion right now
    \param    module    Module who's preferences need getting

    This function should really do something, right now it doesn't.
*/
Gdk::NativeWindow
Script::prefs_effect (Inkscape::Extension::Effect * module, SPView * view)
{
    /* Sad, this should really do something... */
    return 0;
}

/**
    \return  A new document that has been opened
    \brief   This function uses a filename that is put in, and calls
             the extension's command to create an SVG file which is
             returned.
    \param   module   Extension to use.
    \param   filename File to open.

    First things first, this function needs a temporary file name.  To
    create on of those the function g_mkstemp is used, with a filename with
    the header of ink_ext_.

    The extension is then executed using the 'execute' function
    with the filname coming in, and the temporary filename.  After
    That executing, the SVG should be in the temporary file.

    Finally, the temporary file is opened using the SVG input module and
    a document is returned.  That document has its filename set to
    the incoming filename (so that it's not the temporary filename).
    That document is then returned from this function.
*/
SPDocument *
Script::open (Inkscape::Extension::Input * module, const gchar * filename)
{
    char tempfilename_out_x[] = "/tmp/ink_ext_XXXXXX";
    gchar * tempfilename_out;
    SPDocument * mydoc;

    tempfilename_out = (gchar *)tempfilename_out_x;

    if (g_mkstemp((char *)tempfilename_out) == -1) {
        /* Error, couldn't create temporary filename */
        if (errno == EINVAL) {
            /* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
            perror("Extension::Script:  template for filenames is misconfigured.\n");
            exit(-1);        
        } else if (errno == EEXIST) {
            /* Now the  contents of template are undefined. */
            perror("Extension::Script:  Could not create a unique temporary filename\n");
            return NULL;
        } else {
            perror("Extension::Script:  Unknown error creating temporary filename\n");
            exit(-1);
        }
    }

    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError* error = NULL;
    gchar* local_filename = g_filename_from_utf8 ( filename,
                                 -1,  &bytesRead,  &bytesWritten, &error);

    execute(command, local_filename, (gchar *)tempfilename_out);

    g_free(local_filename);

    if (helper_extension == NULL) {
        mydoc = Inkscape::Extension::open(Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG), tempfilename_out);
    } else {
        mydoc = Inkscape::Extension::open(Inkscape::Extension::db.get(helper_extension), tempfilename_out);
    }

    sp_document_set_uri(mydoc, (const gchar *)filename);

    unlink((char *)tempfilename_out);

    return mydoc;
}

/**
    \return   none
    \brief    This function uses an extention to save a document.  It first
              creates an SVG file of the document, and then runs it through
              the script.
    \param    module    Extention to be used
    \param    doc       Document to be saved
    \param    filename  The name to save the final file as

    Well, at some point people need to save - it is really what makes
    the entire application useful.  And, it is possible that someone
    would want to use an extetion for this, so we need a function to
    do that eh?

    First things first, the document is saved to a temporary file that
    is an SVG file.  To get the temporary filename g_mkstemp is used with
    sp_ext_ as a prefix.  Don't worry, this file gets deleted at the
    end of the function.

    After we have the SVG file, then extention_execute is called with
    the temporary file name and the final output filename.  This should
    put the output of the script into the final output file.  We then
    delete the temporary file.
*/
void
Script::save (Inkscape::Extension::Output * module, SPDocument * doc, const gchar * filename)
{
    gchar tempfilename_in_x[] = "/tmp/ink_ext_XXXXXX";
    gchar * tempfilename_in;

    tempfilename_in = tempfilename_in_x;

    if (g_mkstemp(tempfilename_in) == -1) {
        /* Error, couldn't create temporary filename */
        if (errno == EINVAL) {
            /* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
            perror("Extension::Script:  template for filenames is misconfigured.\n");
            exit(-1);        
        } else if (errno == EEXIST) {
            /* Now the  contents of template are undefined. */
            perror("Extension::Script:  Could not create a unique temporary filename\n");
            return;
        } else {
            perror("Extension::Script:  Unknown error creating temporary filename\n");
            exit(-1);
        }
    }

    if (helper_extension == NULL) {
        Inkscape::Extension::save(Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE), doc, tempfilename_in, FALSE, FALSE, FALSE);
    } else {
        Inkscape::Extension::save(Inkscape::Extension::db.get(helper_extension), doc, tempfilename_in, FALSE, FALSE, FALSE);
    }

    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError* error = NULL;
    gchar* local_filename = g_filename_from_utf8 ( filename,
                                 -1,  &bytesRead,  &bytesWritten, &error);

    execute(command, (gchar *)tempfilename_in, local_filename);

    g_free(local_filename);

    unlink(tempfilename_in);

    return;
}

/**
    \return    none
    \brief     This function uses an extention as a effect on a document.
    \param     module   Extention to effect with.
    \param     doc      Document to run through the effect.

    This function is a little bit trickier than the previous two.  It
    needs two temporary files to get it's work done.  Both of these
    files have random names created for them using the g_mkstemp function
    with the sp_ext_ prefix in the temporary directory.  Like the other
    functions, the temporary files are deleted at the end.

    To save/load the two temporary documents (both are SVG) the internal
    modules for SVG load and save are used.  They are both used through
    the module system function by passing their keys into the functions.

    The command itself is built a little bit differently than in other
    functions because the effect support selections.  So on the command
    line a list of all the ids that are selected is included.  Currently,
    this only works for a single selected object, but there will be more.
    The command string is filled with the data, and then after the execution
    it is freed.

    The execute function is used at the core of this function
    to execute the Script on the two SVG documents (actually only one
    exists at the time, the other is created by that script).  At that
    point both should be full, and the second one is loaded.
*/
void
Script::effect (Inkscape::Extension::Effect * module, SPView * doc)
{
    char tempfilename_in_x[] = "/tmp/ink_ext_XXXXXX";
    gchar * tempfilename_in;
    char tempfilename_out_x[] = "/tmp/ink_ext_XXXXXX";
    gchar * tempfilename_out;
    SPDocument * mydoc;

    tempfilename_in = (char *)tempfilename_in_x;

    if (g_mkstemp(tempfilename_in) == -1) {
        /* Error, couldn't create temporary filename */
        if (errno == EINVAL) {
            /* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
            perror("Extension::Script:  template for filenames is misconfigured.\n");
            exit(-1);        
        } else if (errno == EEXIST) {
            /* Now the  contents of template are undefined. */
            perror("Extension::Script:  Could not create a unique temporary filename\n");
            return;
        } else {
            perror("Extension::Script:  Unknown error creating temporary filename\n");
            exit(-1);
        }
    }

    tempfilename_out = (char *)tempfilename_out_x;

    if (g_mkstemp(tempfilename_out) == -1) {
        /* Error, couldn't create temporary filename */
        if (errno == EINVAL) {
            /* The  last  six characters of template were not XXXXXX.  Now template is unchanged. */
            perror("Extension::Script:  template for filenames is misconfigured.\n");
            exit(-1);        
        } else if (errno == EEXIST) {
            /* Now the  contents of template are undefined. */
            perror("Extension::Script:  Could not create a unique temporary filename\n");
            return;
        } else {
            perror("Extension::Script:  Unknown error creating temporary filename\n");
            exit(-1);
        }
    }

    Inkscape::Extension::save(Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE), doc->doc, tempfilename_in, FALSE, FALSE, FALSE);

    Glib::ustring local_command(command);

    /** \todo Should be some sort of checking here.. don't know how to
              do this with structs instead of classes */
    SPDesktop * desktop = (SPDesktop *)doc;
    if (desktop != NULL) {
        std::list<SPItem *> selected;
        desktop->selection->list(selected);

        if (!selected.empty()) {
            for (std::list<SPItem *>::iterator currentItem = selected.begin();
                 currentItem != selected.end(); currentItem++) {

                local_command += " --id=";
                local_command += SP_OBJECT_ID(*currentItem);
            }
        }
    }

    execute(local_command.c_str(), tempfilename_in, tempfilename_out);

    mydoc = Inkscape::Extension::open(Inkscape::Extension::db.get(SP_MODULE_KEY_INPUT_SVG), tempfilename_out);

    unlink(tempfilename_in);
    unlink(tempfilename_out);

    /* Do something with mydoc.... */
    /* TODO: This creates a new window, which really isn't
     * ideal...  there needs to be a better way to do this. */
    if (1) {
        SPViewWidget *dtw;

        g_return_if_fail (mydoc != NULL);

        dtw = sp_desktop_widget_new (sp_document_namedview (mydoc, NULL));
        sp_document_unref (mydoc);
        g_return_if_fail (dtw != NULL);

        sp_create_window (dtw, TRUE);
    }

    return;
}

/* Helper class used by Script::execute */
class pipe_t {
public:
  /* These functions set errno if they return false.
     I'm not sure whether that's a good idea or not, but it should be reasonably
     straightforward to change it if needed. */
  bool open(char* command, int mode);
  bool close();

  /* These return the number of bytes read/written. */
  size_t read(void* buffer, size_t size);
  size_t write(const void* buffer, size_t size);

  enum {
    mode_read  = 1<<0,
    mode_write = 1<<1,
  };

private:
#ifdef WIN32
  /* This is used to translate win32 errors into errno errors.
     It only recognizes a few win32 errors for the moment though. */
  static int translate_error(DWORD err);

  HANDLE hpipe;
#else
  FILE* ppipe;
#endif
};

/**
    \return   none
    \brief    This is the core of the extension file as it actually does
              the execution of the extension.
    \param    in_command  The command to be executed
    \param    filein      Filename coming in
    \param    fileout     Filename of the out file

    The first thing that this function does is build the command to be
    executed.  This consists of the first string (in_command) and then
    the filename for input (filein).  This file is put on the command
    line.

    The next thing is that this function does is open a pipe to the
    command and get the file handle in the ppipe variable.  It then
    opens the output file with the output file handle.  Both of these
    operations are checked extensively for errors.

    After both are opened, then the data is copied from the output
    of the pipe into the file out using fread and fwrite.  These two
    functions are used because of their primitive nature they make
    no assumptions about the data.  A buffer is used in the transfer,
    but the output of fread is stored so the exact number of bytes
    is handled gracefully.

    At the very end (after the data has been copied) both of the files
    are closed, and we return to what we were doing.
*/
void
Script::execute (const gchar * in_command, const gchar * filein, const gchar * fileout)
{
    pipe_t pipe;
    FILE * pfile;
    char buf[BUFSIZE];
    char  * command;
    int num_read;

    g_return_if_fail(in_command != NULL);

    /* Get the commandline to be run */
    /* TODO:  Perhaps replace with a sprintf? */
    command = g_strdup_printf("%s \"%s\"", in_command, filein);

    // std::cout << "Command to run: " << command << std::endl;

    bool open_success = pipe.open(command, pipe_t::mode_read);
    g_free(command);

    /* Run script */
    if (!open_success) {
      /* Error - could not open pipe - check errno */
      if (errno == EINVAL) {
        perror("Extension::Script:  Invalid mode argument in popen\n");
      } else if (errno == ECHILD) {
        perror("Extension::Script:  Cannot obtain child extension status in popen\n");
      } else {
        perror("Extension::Script:  Unknown error for popen\n");
      }
      return;
    }

        Inkscape::IO::dump_fopen_call(fileout, "J");
    pfile = Inkscape::IO::fopen_utf8name(fileout, "w");

    if (pfile == NULL) {
      /* Error - could not open file */
      if (errno == EINVAL) {
        perror("Extension::Script:  The mode provided to fopen was invalid\n");
      } else {
        perror("Extension::Script:  Unknown error attempting to open temporary file\n");
      }
      return;
    }

    /* Copy pipe output to a temporary file */
    while ((num_read = pipe.read(buf, BUFSIZE)) != 0) {
        fwrite(buf, 1, num_read, pfile);
    }

    /* Close file */
    if (fclose(pfile) == EOF) {
      if (errno == EBADF) {
        perror("Extension::Script:  The filedescriptor for the temporary file is invalid\n");
        return;
      } else {
        perror("Extension::Script:  Unknown error closing temporary file\n");
      }
    }

    /* Close pipe */
    if (!pipe.close()) {
      if (errno == EINVAL) {
        perror("Extension::Script:  Invalid mode set for pclose\n");
      } else if (errno == ECHILD) {
        perror("Extension::Script:  Could not obtain child status for pclose\n");
      } else {
        perror("Extension::Script:  Unknown error for pclose\n");
      }
    }

    return;
}

#ifdef WIN32

bool pipe_t::open(char* command, int mode_p) {
  HANDLE pipe_write;

  // Create pipe
  {
    SECURITY_ATTRIBUTES secattrs;
    ZeroMemory(&secattrs,sizeof(secattrs));
    secattrs.nLength=sizeof(secattrs);
    secattrs.lpSecurityDescriptor=0;
    secattrs.bInheritHandle=TRUE;
    HANDLE t_pipe_read=0;
    if ( !CreatePipe(&t_pipe_read, &pipe_write, &secattrs, 0) ) {
      errno = translate_error(GetLastError());
      return false;
    }
    // This duplicate handle makes the read pipe uninheritable
    if ( !DuplicateHandle(GetCurrentProcess(), t_pipe_read, GetCurrentProcess(), &hpipe, 0, FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS) ) {
      int en = translate_error(GetLastError());
      CloseHandle(t_pipe_read);
      CloseHandle(pipe_write);
      errno = en;
      return false;
    }
  }

  // Create process
  {
    PROCESS_INFORMATION procinfo;
    STARTUPINFO startupinfo;
    ZeroMemory(&procinfo,sizeof(procinfo));
    ZeroMemory(&startupinfo,sizeof(startupinfo));
    startupinfo.cb=sizeof(startupinfo);
    //startupinfo.lpReserved=0;
    //startupinfo.lpDesktop=0;
    //startupinfo.lpTitle=0;
    startupinfo.dwFlags=STARTF_USESTDHANDLES;
    startupinfo.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
    startupinfo.hStdOutput=pipe_write;

    if ( !CreateProcess(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &startupinfo, &procinfo) ) {
      errno = translate_error(GetLastError());
      return false;
    }
    CloseHandle(procinfo.hThread);
    CloseHandle(procinfo.hProcess);
  }

  // Close our copy of the write handle
  CloseHandle(pipe_write);

  return true;
}

bool pipe_t::close() {
  BOOL retval = CloseHandle(hpipe);
  if ( !retval ) {
    errno = translate_error(GetLastError());
  }
  return retval != FALSE;
}

size_t pipe_t::read(void* buffer, size_t size) {
  DWORD bytes_read = 0;
  ReadFile(hpipe, buffer, size, &bytes_read, 0);
  return bytes_read;
}

size_t pipe_t::write(const void* buffer, size_t size) {
  DWORD bytes_written = 0;
  WriteFile(hpipe, buffer, size, &bytes_written, 0);
  return bytes_written;
}

int pipe_t::translate_error(DWORD err) {
  switch(err) {
  case ERROR_FILE_NOT_FOUND:
    return ENOENT;
  case ERROR_INVALID_HANDLE:
  case ERROR_INVALID_PARAMETER:
    return EINVAL;
  default:
    return 0;
  }
}

#else // Win32

bool pipe_t::open(char* command, int mode_p) {
  char popen_mode[4] = {0,0,0,0};
  char* popen_mode_cur = popen_mode;

  if ( (mode_p & mode_read) != 0 ) {
    *popen_mode_cur++ = 'r';
  }

  if ( (mode_p & mode_write) != 0 ) {
    *popen_mode_cur++ = 'w';
  }

  ppipe = popen(command, popen_mode);

  return ppipe != NULL;
}

bool pipe_t::close() {
  return fclose(ppipe) == 0;
}

size_t pipe_t::read(void* buffer, size_t size) {
  return fread(buffer, 1, size, ppipe);
}

size_t pipe_t::write(const void* buffer, size_t size) {
  return fwrite(buffer, 1, size, ppipe);
}

#endif // (Non-)Win32

}  /* Inkscape  */
}  /* module  */
}  /* Implementation  */

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
