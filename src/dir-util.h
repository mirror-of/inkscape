#ifndef __SP_DIR_UTIL_H__
#define __SP_DIR_UTIL_H__

/*
 * path-util.h
 *
 * here are functions sp_relative_path & cousins
 * maybe they are already implemented in standard libs
 *
 */

const char *sp_relative_path_from_path (const char * path, const char * base);
const char *sp_filename_from_path (const char * path);
const char *sp_extension_from_path (const char * path);

#endif
