#define DIR_UTIL_C

#include <errno.h>
#include <stdlib.h>
#include <glib.h>
#include "dir-util.h"
#include <string.h>

const gchar *
sp_relative_path_from_path (const gchar * path, const gchar * base)
{
	const gchar * p, * b, * r;

	if (path == NULL) return NULL;
	if (base == NULL) return path;

	p = path;
	b = base;
	r = path;

	while ((* p == * b) && (* p != '\0') && (* b != '\0')) {
		if (* p == G_DIR_SEPARATOR) r = p + 1;
		p++;
		b++;
	}

	if ((* b == '\0') && (* p == G_DIR_SEPARATOR))
		return p + 1;
	if ((* b == '\0') && (* p != '\0'))
		return p;
	return path;
}

const gchar *
sp_filename_from_path (const gchar * path)
{
	const gchar * p;

	if (path == NULL) return NULL;

	p = path;
	while (* p != '\0') p++;

	while ((* p != G_DIR_SEPARATOR) && (p >= path)) p--;
	p++;

	return p;
}

const gchar *
sp_extension_from_path (const gchar * path)
{
	const gchar * p;

	if (path == NULL) return NULL;

	p = path;
	while (* p != '\0') p++;

	while ((* p != G_DIR_SEPARATOR) && (* p != '.') && (p >= path)) p--;
	if (* p != '.') return NULL;
	p++;

	return p;
}


/* current == "./", parent == "../" */
static char dots[] = {'.', '.', G_DIR_SEPARATOR, '\0'};
static char *parent = dots;
static char *current = dots + 1;

/**
 * \brief   Convert an relative path name into absolute.
 *
 *	\param path	relative path
 *	\param base	base directory (must be absolute path)
 *	\param result	result buffer
 *	\param size	size of result buffer
 *	\return		!= NULL: absolute path
 *			== NULL: error

\comment
 based on functions by Shigio Yamaguchi.
 FIXME:TODO: force it to also do path normalization of the entire resulting path, 
 i.e. get rid of any .. and . in any place, even if 'path' is already absolute 
 (now it returns it unchanged in this case)

 */
char *
inkscape_rel2abs (const char *path, const char *base, char *result, const size_t size)
{
  const char *pp, *bp;
  /* endp points the last position which is safe in the result buffer. */
  const char *endp = result + size - 1;
  char *rp;
  int length;
  if (*path == G_DIR_SEPARATOR)
    {
      if (strlen (path) >= size)
	goto erange;
	strcpy (result, path);
	goto finish;
    }
  else if (*base != G_DIR_SEPARATOR || !size)
    {
      errno = EINVAL;
      return (NULL);
    }
  else if (size == 1)
    goto erange;
  if (!strcmp (path, ".") || !strcmp (path, current))
    {
      if (strlen (base) >= size)
	goto erange;
      strcpy (result, base);
      /* rp points the last char. */
      rp = result + strlen (base) - 1;
      if (*rp == G_DIR_SEPARATOR)
	*rp = 0;
      else
	rp++;
      /* rp point NULL char */
      if (*++path == G_DIR_SEPARATOR)
	{
	  /* Append G_DIR_SEPARATOR to the tail of path name. */
	  *rp++ = G_DIR_SEPARATOR;
	  if (rp > endp)
	    goto erange;
	  *rp = 0;
	}
      goto finish;
    }
  bp = base + strlen (base);
  if (*(bp - 1) == G_DIR_SEPARATOR)
    --bp;
  /* up to root. */
  for (pp = path; *pp && *pp == '.';)
    {
      if (!strncmp (pp, parent, 3))
	{
	  pp += 3;
	  while (bp > base && *--bp != G_DIR_SEPARATOR)
	    ;
	}
      else if (!strncmp (pp, current, 2))
	{
	  pp += 2;
	}
      else if (!strncmp (pp, "..\0", 3))
	{
	  pp += 2;
	  while (bp > base && *--bp != G_DIR_SEPARATOR)
	    ;
	}
      else
	break;
    }
  /* down to leaf. */
  length = bp - base;
  if (length >= size)
    goto erange;
  strncpy (result, base, length);
  rp = result + length;
  if (*pp || *(pp - 1) == G_DIR_SEPARATOR || length == 0)
    *rp++ = G_DIR_SEPARATOR;
  if (rp + strlen (pp) > endp)
    goto erange;
  strcpy (rp, pp);
finish:
  return result;
erange:
  errno = ERANGE;
  return (NULL);
}

char *
inkscape_abs2rel (const char *path, const char *base, char *result, const size_t size)
{
  const char *pp, *bp, *branch;
  /* endp points the last position which is safe in the result buffer. */
  const char *endp = result + size - 1;
  char *rp;

  if (*path != G_DIR_SEPARATOR)
    {
      if (strlen (path) >= size)
	goto erange;
      strcpy (result, path);
      goto finish;
    }
  else if (*base != G_DIR_SEPARATOR || !size)
    {
      errno = EINVAL;
      return (NULL);
    }
  else if (size == 1)
    goto erange;
  /* seek to branched point. */
  branch = path;
  for (pp = path, bp = base; *pp && *bp && *pp == *bp; pp++, bp++)
    if (*pp == G_DIR_SEPARATOR)
      branch = pp;
  if ((*pp == 0 || *pp == G_DIR_SEPARATOR && *(pp + 1) == 0) &&
      (*bp == 0 || *bp == G_DIR_SEPARATOR && *(bp + 1) == 0))
    {
      rp = result;
      *rp++ = '.';
      if (*pp == G_DIR_SEPARATOR || *(pp - 1) == G_DIR_SEPARATOR)
	*rp++ = G_DIR_SEPARATOR;
      if (rp > endp)
	goto erange;
      *rp = 0;
      goto finish;
    }
  if (*pp == 0 && *bp == G_DIR_SEPARATOR || *pp == G_DIR_SEPARATOR && *bp == 0)
    branch = pp;
  /* up to root. */
  rp = result;
  for (bp = base + (branch - path); *bp; bp++)
    if (*bp == G_DIR_SEPARATOR && *(bp + 1) != 0)
      {
	if (rp + 3 > endp)
	  goto erange;
	*rp++ = '.';
	*rp++ = '.';
	*rp++ = G_DIR_SEPARATOR;
      }
  if (rp > endp)
    goto erange;
  *rp = 0;
  /* down to leaf. */
  if (*branch)
    {
      if (rp + strlen (branch + 1) > endp)
	goto erange;
      strcpy (rp, branch + 1);
    }
  else
    *--rp = 0;
finish:
  return result;
erange:
  errno = ERANGE;
  return (NULL);
}
