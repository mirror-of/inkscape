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
const char *sp_extension_from_path (const char * path);
char *inkscape_rel2abs (const char *path, const char *base, char *result, const size_t size);
char *inkscape_abs2rel (const char *path, const char *base, char *result, const size_t size);

#endif

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
