#ifndef __ARIKKEI_TOKEN_H__
#define __ARIKKEI_TOKEN_H__

/*
 * Arikkei
 *
 * Basic datatypes and code snippets
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 *
 */

/*
 * Sorry - as you can guess, it was C++ library intially
 * In plain C is is not half as nice, but still quite useful
 */

#include <glib.h>

typedef struct _ArikkeiToken ArikkeiToken;

struct _ArikkeiToken {
	const gchar *cdata;
	int start, end;
};

ArikkeiToken *arikkei_token_set_from_string (ArikkeiToken *this_token, const gchar *text);
ArikkeiToken *arikkei_token_set_from_data (ArikkeiToken *this_token, const gchar *data, int start, int end);
ArikkeiToken *arikkei_token_set_from_token (ArikkeiToken *this_token, const ArikkeiToken *src);

#define arikkei_token_is_valid(t) ((t)->cdata && ((t)->end >= (t)->start))
#define arikkei_token_is_empty(t) (!(t)->cdata || ((t)->end <= (t)->start))

unsigned int arikkei_token_is_equal (const ArikkeiToken *this_token, const ArikkeiToken *t);
unsigned int arikkei_token_is_equal_string (const ArikkeiToken *this_token, const gchar *t);

gchar *arikkei_token_strdup (const ArikkeiToken *this_token);
int arikkei_token_strcpy (const ArikkeiToken *this_token, gchar *b);
int arikkei_token_strncpy (const ArikkeiToken *this_token, gchar *b, size_t size);
int arikkei_token_strcmp (const ArikkeiToken *this_token, const gchar *b);
int arikkei_token_strncmp (const ArikkeiToken *this_token, const gchar *b, size_t size);

ArikkeiToken *arikkei_token_get_first_line (const ArikkeiToken *this_token, ArikkeiToken *dst);
ArikkeiToken *arikkei_token_get_line (const ArikkeiToken *this_token, ArikkeiToken *dst, int s);
ArikkeiToken *arikkei_token_next_line (const ArikkeiToken *this_token, ArikkeiToken *dst, const ArikkeiToken *line);
ArikkeiToken *arikkei_token_get_token (const ArikkeiToken *this_token, ArikkeiToken *dst, int s, unsigned int space_is_separator);
ArikkeiToken *arikkei_token_next_token (const ArikkeiToken *this_token, ArikkeiToken *dst, const ArikkeiToken *token, unsigned int space_is_separator);

int arikkei_token_tokenize (ArikkeiToken *this_token, ArikkeiToken *tokens, int maxtokens, unsigned int space_is_separator, unsigned int multi);
int arikkei_token_tokenize_ws (ArikkeiToken *this_token, ArikkeiToken *tokens, int maxtokens, const gchar *ws, unsigned int multi);

ArikkeiToken *arikkei_token_strip_start (ArikkeiToken *this_token, ArikkeiToken *dst);
ArikkeiToken *arikkei_token_strip_start_ws (ArikkeiToken *this_token, ArikkeiToken *dst, const gchar *ws);
ArikkeiToken *arikkei_token_strip_end (ArikkeiToken *this_token, ArikkeiToken *dst);
ArikkeiToken *arikkei_token_strip_end_ws (ArikkeiToken *this_token, ArikkeiToken *dst, const gchar *ws);
ArikkeiToken *arikkei_token_strip (ArikkeiToken *this_token, ArikkeiToken *dst);
ArikkeiToken *arikkei_token_strip_ws (ArikkeiToken *this_token, ArikkeiToken *dst, const gchar *ws);

gchar *arikkei_token_strconcat (const ArikkeiToken *tokens, int size, const gchar *separator);

#endif





