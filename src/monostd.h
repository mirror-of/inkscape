/* 
 * Compatibility defines for poor platforms which don't have
 * 'unistd.h'. Currently only WIN32 supported.
 */
#ifndef WIN32
#error "Only win32 supported yet."
#endif

#ifndef MONOSTD_H
#define MONOSTD_H
#include <glib.h> /* part of the compatibiloity defines are here */
#include <io.h>
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif
#include <direct.h>
#define mkdir(n,f) _mkdir(n)

#endif
