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

#include "FilePath.h"

#include <string.h>
#ifndef WIN32
#include <unistd.h>
#define HAS_PROC_SELF_EXE  //to get path of executable
#else
#include <direct.h>
#define _WIN32_IE 0x0400
#define HAS_SHGetSpecialFolderPath
#define HAS_GetModuleFileName
#include <shlobj.h> //to get appdata path
#endif

using namespace Inkscape;




gchar * FilePath::getExecutablePath(void)
{
  gchar *path  = "";
  int   strLen = 0;
  char  pathBuf[Inkscape::MAXPATH + 1];
#ifdef HAS_PROC_SELF_EXE
  strLen = readlink("/proc/self/exe", pathBuf, Inkscape::MAXPATH);
#else
#ifdef HAS_GetModuleFileName
  strLen = (int)GetModuleFileName(Inkscape::MAXPATH, pathBuf, Inkscape::MAXPATH);
#endif
#endif
  if (strLen <= 0)
    {
    g_print("Unable to retrieve the path of the executable.\n");
    return NULL;
    }
  for (int i=strLen-1 ; i>0 ; i--)
    {
    if (path[i]=='/')
      {
      path[i] = '\0';
      break;
      }
    }
  path = getCanonicalPath(pathBuf);
  strLen = strlen(path);  //Now remove file name from end of path
  for (int i=strLen-1 ; i>0 ; i--)
    {
    if (path[i]=='/')
      {
      path[i] = '\0';
      break;
      }
    }
  return path;
}


gchar * FilePath::getCanonicalPath(gchar *path)
{
  if (!path) //null?
    return NULL;
  int strLen = strlen(path);
  if (strLen<1) //0 length?
    return g_strdup(path);
  char  pathBuf[Inkscape::MAXPATH + 1];
  int destLen = 0;
  for (int i=0 ; i<strLen ; i++)
    {
    gchar ch = path[i];
    if (ch == '\\')
      ch = '/';
#ifdef OSX
    else if (ch == ':')
      ch = '/';
#endif
    pathBuf[destLen++] = ch;
    }
  pathBuf[destLen] = '\0';
  gchar *newPath = g_strdup(pathBuf);
  return newPath;
}


gchar * FilePath::getNativePath(gchar *path)
{
  if (!path) //null?
    return NULL;
  int strLen = strlen(path);
  if (strLen<1) //0 length?
    return g_strdup(path);
  char  pathBuf[Inkscape::MAXPATH + 1];
  int destLen = 0;
  for (int i=0 ; i<strLen ; i++)
    {
    gchar ch = path[i];
    if (ch == '/')
#ifdef WIN32
      ch = '/';
#else
#ifdef OSX
      ch = ':';
#endif
#endif
    pathBuf[destLen++] = ch;
    }
  pathBuf[destLen] = '\0';
  gchar *newPath = g_strdup(pathBuf);
  return newPath;
}

gchar * FilePath::getRelativePath(gchar *head, gchar *tail)
{
  if (!head) //null?
    return NULL;
  //it is ok for tail to be NULL
  gchar *fullPath = g_build_path("/", head, tail, NULL);
  int strLen = strlen(fullPath);

  int destLen = 0;
  char  destBuf[Inkscape::MAXPATH + 1];
  for (int i=0 ; i<strLen ; )  //reduce, if possible
    {
    if (i<=strLen-2 && strncmp(&(fullPath[i]), "//", 2)==0)
      i+=2;
    else if (i<=strLen-3 && strncmp(&(fullPath[i]), "/./", 3)==0)
      i+=3;
    else
      destBuf[destLen++] = fullPath[i++];
    }
  destBuf[destLen] = '\0';
  gchar *newPath = g_strdup(destBuf);
  g_free(fullPath);
  return newPath;
}

gchar * FilePath::getResourcePath(gchar *fileName, gint type)
{
  gchar *exePath = getExecutablePath();
  if (!exePath)
    exePath = g_strdup(".");
  gchar *path;
  switch (type)
    {
    case FilePath::PIXMAP:
      path = getRelativePath(exePath, "../share/inkscape/pixmaps");
      break;
    case FilePath::DATA:
      path = getRelativePath(exePath, "../share/inkscape/data");
      break;
    case FilePath::GLADE:
      path = getRelativePath(exePath, "../share/inkscape/glade");
      break;
    case FilePath::MODULES:
      path = getRelativePath(exePath, "../share/inkscape/modules");
      break;
    default:
      path = g_strdup(exePath);
    }
  g_free(exePath);
  gchar *fullPath = getRelativePath(path, fileName);
  g_free(path);
  return fullPath;
}


gchar * FilePath::getNativeResourcePath(gchar *fileName, gint type)
{
  gchar *canonicalPath = getResourcePath(fileName, type);
  if (!canonicalPath)
    return g_strdup(fileName);
  gchar *nativePath = getNativePath(canonicalPath);
  g_free(canonicalPath);
  return nativePath;
}



gchar * FilePath::getProfilePath(void)
{
  static const gchar *profileDir=NULL;
  if (!profileDir)
    {
    profileDir = g_get_home_dir();
    }
#ifdef HAS_SHGetSpecialFolderPath
  if (!profileDir) //only try this is previous attempt fails
    {
    char pathBuf[MAX_PATH+1];
    if (SHGetSpecialFolderPath(NULL, pathBuf, CSIDL_APPDATA, 1))
      profileDir = g_strdup(pathBuf);
    }
#endif
  if (!profileDir)
    {
    profileDir = getExecutablePath();
    }
  if (!profileDir)
    {
    profileDir = g_strdup(".");
    }
  return g_build_path("/", profileDir, "inkscape", NULL);
}

gchar * FilePath::getNativeProfilePath(void)
{
  gchar *path = getProfilePath();
  gchar *nativePath = getNativePath(path);
  g_free(path);
  return nativePath;
}


gchar * FilePath::getPreferencesPath(void)
{
  gchar *profilePath = getProfilePath();
  return g_build_path("/", profilePath, "preferences.xml", NULL);
}

gchar * FilePath::getNativePreferencesPath(void)
{
  gchar *path = getPreferencesPath();
  gchar *nativePath = getNativePath(path);
  g_free(path);
  return nativePath;
}



















/**
 * Constructor
 */
FilePath::FilePath(void)
{

}



/**
 * Destructor
 */
FilePath::~FilePath(void)
{

}






