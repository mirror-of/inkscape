#define DIR_UTIL_C

#include <glib.h>
#include "dir-util.h"

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

