/*
 * Separate the inkscape paths from the prefix code, as that is kind of
 * a separate package (binreloc)
 * 	http://autopackage.org/downloads.html
 *
 * Since the directories set up by autoconf end up in config.h, we can't
 * _change_ them, since config.h isn't protected by a set of
 * one-time-include directives and is repeatedly re-included by some
 * chains of .h files.  As a result, nothing should refer to those
 * define'd directories, and instead should use only the paths defined here.
 *
 */
#ifndef _PATH_PREFIX_H_
#define _PATH_PREFIX_H_

#include "prefix.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef ENABLE_BINRELOC
#  define INKSCAPE_APPICONDIR     BR_DATADIR( "/pixmaps" )
#  define INKSCAPE_EXTENSIONDIR   BR_DATADIR( "/inkscape/extensions" )
#  define INKSCAPE_GRADIENTSDIR   BR_DATADIR( "/inkscape/gradients" )
#  define INKSCAPE_PIXMAPDIR      BR_DATADIR( "/inkscape/icons" )
#  define INKSCAPE_MARKERSDIR     BR_DATADIR( "/inkscape/markers" )
#  define INKSCAPE_PATTERNSDIR    BR_DATADIR( "/inkscape/patterns" )
#  define INKSCAPE_SCREENSDIR     BR_DATADIR( "/inkscape/screens" )
#  define INKSCAPE_TUTORIALSDIR   BR_DATADIR( "/inkscape/tutorials" )
#else
#  ifdef WIN32
#    define INKSCAPE_APPICONDIR   "pixmaps"
#    define INKSCAPE_EXTENSIONDIR "share\\extensions"
#    define INKSCAPE_GRADIENTSDIR "share\\gradients"
#    define INKSCAPE_PIXMAPDIR    "share\\icons"
#    define INKSCAPE_MARKERSDIR   "share\\markers"
#    define INKSCAPE_PATTERNSDIR  "share\\patterns"
#    define INKSCAPE_SCREENSDIR   "share\\screens"
#    define INKSCAPE_TUTORIALSDIR "share\\tutorials"
#  else
#    define INKSCAPE_APPICONDIR   INKSCAPE_DATADIR "/pixmaps"
#    define INKSCAPE_EXTENSIONDIR INKSCAPE_DATADIR "/inkscape/extensions"
#    define INKSCAPE_GRADIENTSDIR INKSCAPE_DATADIR "/inkscape/gradients"
#    define INKSCAPE_PIXMAPDIR    INKSCAPE_DATADIR "/inkscape/icons"
#    define INKSCAPE_MARKERSDIR   INKSCAPE_DATADIR "/inkscape/markers"
#    define INKSCAPE_PATTERNSDIR  INKSCAPE_DATADIR "/inkscape/patterns"
#    define INKSCAPE_SCREENSDIR   INKSCAPE_DATADIR "/inkscape/screens"
#    define INKSCAPE_TUTORIALSDIR INKSCAPE_DATADIR "/inkscape/tutorials"
#  endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PATH_PREFIX_H_ */
