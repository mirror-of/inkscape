#define __ARIKKEI_TOKEN_C__

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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "arikkei-token.h"

ArikkeiToken *
arikkei_token_set_from_string (ArikkeiToken *this_tok, const gchar *text)
{
	this_tok->cdata = text;
	this_tok->start = 0;
	this_tok->end = (text) ? strlen (text) : 0;
	return this_tok;
}

ArikkeiToken *
arikkei_token_set_from_data (ArikkeiToken *this_tok, const gchar *data, int start, int end)
{
	this_tok->cdata = data;
	this_tok->start = start;
	this_tok->end = end;
	return this_tok;
}

ArikkeiToken *
arikkei_token_set_from_token (ArikkeiToken *this_tok, const ArikkeiToken *src)
{
	this_tok->cdata = src->cdata;
	this_tok->start = src->start;
	this_tok->end = src->end;
	return this_tok;
}

unsigned int
arikkei_token_is_equal (const ArikkeiToken *this_tok, const ArikkeiToken *t)
{
	if (!arikkei_token_is_valid (this_tok)) return 0;
	if (!arikkei_token_is_valid (t)) return 0;
	if (!arikkei_token_is_empty (this_tok)) {
		if (!arikkei_token_is_empty (t)) {
			int llen, rlen;
			llen = this_tok->end - this_tok->start;
			rlen = t->end - t->start;
			return ((llen == rlen) && !strncmp (this_tok->cdata + this_tok->start, t->cdata + t->start, llen));
		} else {
			return 0;
		}
	} else {
		if (!arikkei_token_is_empty (t)) {
			return 0;
		} else {
			return 1;
		}
	}
}

unsigned int
arikkei_token_is_equal_string (const ArikkeiToken *this_tok, const gchar *t)
{
	if (!arikkei_token_is_valid (this_tok)) return 0;
	return !arikkei_token_strcmp (this_tok, t);
}

gchar *
arikkei_token_strdup (const ArikkeiToken *this_tok)
{
	if (arikkei_token_is_valid (this_tok)) {
		gchar *b;
		int len;
		len = this_tok->end - this_tok->start;
		b = (gchar*)malloc (len + 1);
		if (len) strncpy (b, this_tok->cdata + this_tok->start, len);
		b[len] = 0;
		return b;
	} else {
		return NULL;
	}
}

int
arikkei_token_strcpy (const ArikkeiToken *this_tok, gchar *b)
{
	if (arikkei_token_is_valid (this_tok)) {
		int len;
		len = this_tok->end - this_tok->start;
		if (len) strncpy (b, this_tok->cdata + this_tok->start, len);
		b[len] = 0;
		return len;
	} else {
		b[0] = 0;
		return 0;
	}
}

int
arikkei_token_strncpy (const ArikkeiToken *this_tok, gchar *b, size_t size)
{
	if (size < 1) return 0;
	if (arikkei_token_is_valid (this_tok) && (size > 1)) {
		int len;
		len = this_tok->end - this_tok->start;
		if (len > (size - 1)) len = size - 1;
		if (len) strncpy (b, this_tok->cdata + this_tok->start, len);
		b[len] = 0;
		return len;
	} else {
		b[0] = 0;
		return 0;
	}
}

int
arikkei_token_strcmp (const ArikkeiToken *this_tok, const gchar *b)
{
	if (!arikkei_token_is_valid (this_tok)) return -1;
	if (b) {
		return arikkei_token_strncmp (this_tok, b, strlen (b));
	} else {
		return arikkei_token_is_empty (this_tok);
	}
}

int
arikkei_token_strncmp (const ArikkeiToken *this_tok, const gchar *b, size_t size)
{
	if (!arikkei_token_is_valid (this_tok)) return -1;
	if (!arikkei_token_is_empty (this_tok)) {
		if (size > 0) {
			int len, clen, cval;
			len = this_tok->end - this_tok->start;
			clen = (len < size) ? len : size;
			cval = strncmp (this_tok->cdata + this_tok->start, b, clen);
			if (cval) return cval;
			if (len < size) return -1;
			if (len > size) return 1;
			return 0;
		} else {
			return 1;
		}
	} else {
		if (size > 0) {
			return -1;
		} else {
			return 0;
		}
	}
}

ArikkeiToken *
arikkei_token_get_first_line (const ArikkeiToken *this_tok, ArikkeiToken *dst)
{
	return arikkei_token_get_line (this_tok, dst, this_tok->start);
}

ArikkeiToken *
arikkei_token_get_line (const ArikkeiToken *this_tok, ArikkeiToken *dst, int s)
{
	if (!arikkei_token_is_empty (this_tok)) {
		const gchar *p;
		int e;
		p = this_tok->cdata;
		e = s;
		while ((e < this_tok->end) && ((p[e] >= 32) || (p[e] == 9))) e += 1;
		arikkei_token_set_from_data (dst, p, s, e);
	} else {
		arikkei_token_set_from_data (dst, this_tok->cdata, 0, 0);
	}
	return dst;
}

ArikkeiToken *
arikkei_token_next_line (const ArikkeiToken *this_tok, ArikkeiToken *dst, const ArikkeiToken *line)
{
	if (!arikkei_token_is_empty (this_tok)) {
		if (arikkei_token_is_valid (line)) {
			const gchar *p;
			int s;
			p = this_tok->cdata;
			s = line->end;
			while ((s < this_tok->end) && (p[s] < 32) && (p[s] != 9)) s += 1;
			return arikkei_token_get_line (this_tok, dst, s);
		} else {
			return arikkei_token_get_first_line (this_tok, dst);
		}
	} else {
		arikkei_token_set_from_data (dst, this_tok->cdata, 0, 0);
	}
	return dst;
}

ArikkeiToken *
arikkei_token_get_token (const ArikkeiToken *this_tok, ArikkeiToken *dst, int s, unsigned int space_is_separator)
{
	if (!arikkei_token_is_empty (this_tok)) {
		gchar *p;
		while ((s < this_tok->end) && (p[s] == 32)) s += 1;
		if (s < this_tok->end) {
			int e;
			while ((e < this_tok->end) && ((p[e] > 32) || ((p[e] == 32) && !space_is_separator))) e += 1;
			arikkei_token_set_from_data (dst, this_tok->cdata, s, e);
		} else {
			arikkei_token_set_from_data (dst, this_tok->cdata, s, this_tok->end);
		}
	} else {
		arikkei_token_set_from_data (dst, this_tok->cdata, 0, 0);
	}
	return dst;
}

ArikkeiToken *
arikkei_token_next_token (const ArikkeiToken *this_tok, ArikkeiToken *dst, const ArikkeiToken *token, unsigned int space_is_separator)
{
	if (!arikkei_token_is_empty (this_tok)) {
		if (arikkei_token_is_valid (token)) {
			return arikkei_token_get_token (this_tok, dst, token->end + 1, space_is_separator);
		} else {
			return arikkei_token_get_token (this_tok, dst, this_tok->start, space_is_separator);
		}
	} else {
		arikkei_token_set_from_data (dst, this_tok->cdata, 0, 0);
	}
	return dst;
}

int
arikkei_token_tokenize (ArikkeiToken *this_tok, ArikkeiToken *tokens, int maxtokens, unsigned int space_is_separator, unsigned int multi)
{
	const gchar *p;
	int ntokens, s;
	if (arikkei_token_is_empty (this_tok)) return 0;
	ntokens = 0;
	p = this_tok->cdata;
	s = this_tok->start;
	while ((s < this_tok->end) && (ntokens < maxtokens)) {
		int e;
		while ((e < this_tok->end) && ((p[e] > 32) || ((p[e] == 32) && !space_is_separator))) e += 1;
		if (ntokens == (maxtokens - 1)) {
			while ((e < this_tok->end) && ((p[e] >= 32) || (p[e] == 9))) e += 1;
		}
		arikkei_token_set_from_data (tokens + ntokens, this_tok->cdata, s, e);
		s = e + 1;
		if (multi) {
			while ((s < this_tok->end) && ((p[s] < 32) || ((p[s] == 32) && space_is_separator))) s += 1;
		}
		ntokens += 1;
	}
	return ntokens;
}

int
arikkei_token_tokenize_ws (ArikkeiToken *this_tok, ArikkeiToken *tokens, int maxtokens, const gchar *ws, unsigned int multi)
{
	int len, ntokens, s;
	if (arikkei_token_is_empty (this_tok)) return 0;
	len = strlen (ws);
	ntokens = 0;
	s = this_tok->start;
	while ((s < this_tok->end) && (ntokens < maxtokens)) {
		int e;
		if (ntokens != (maxtokens - 1)) {
			e = s;
			while (e < this_tok->end) {
				int i;
				for (i = 0; i < len; i++) {
					if (this_tok->cdata[e] == ws[i]) break;
				}
				if (i < len) break;
				e += 1;
			}
		} else {
			e = this_tok->end;
		}
		arikkei_token_set_from_data (tokens + ntokens, this_tok->cdata, s, e);
		s = e + 1;
		if (multi) {
			while (s < this_tok->end) {
				int i;
				for (i = 0; i < len; i++) {
					if (this_tok->cdata[s] == ws[i]) break;
				}
				if (i < len) break;
				s += 1;
			}
		}
		ntokens += 1;
	}
	return ntokens;
}

ArikkeiToken *
arikkei_token_strip_start (ArikkeiToken *this_tok, ArikkeiToken *dst)
{
	const gchar *p;
	int s;
	p = this_tok->cdata;
	s = this_tok->start;
	if (p) {
		while ((s < this_tok->end) && (p[s] <= 32)) s += 1;
	}
	arikkei_token_set_from_data (dst, this_tok->cdata, s, this_tok->end);
	return dst;
}

ArikkeiToken *
arikkei_token_strip_start_ws (ArikkeiToken *this_tok, ArikkeiToken *dst, const gchar *ws)
{
	int len, s;
	len = strlen (ws);
	s = this_tok->start;
	if (this_tok->cdata) {
		while (s < this_tok->end) {
			int i;
			for (i = 0; i < len; i++) {
				if (this_tok->cdata[s] == ws[i]) break;
			}
			if (i >= len) break;
			s += 1;
		}
	}
	arikkei_token_set_from_data (dst, this_tok->cdata, s, this_tok->end);
	return dst;
}

ArikkeiToken *
arikkei_token_strip_end (ArikkeiToken *this_tok, ArikkeiToken *dst)
{
	const gchar *p;
	int e;
	p = this_tok->cdata;
	e = this_tok->end - 1;
	if (p) {
		while ((e >= this_tok->start) && (p[e] <= 32)) e -= 1;
	}
	arikkei_token_set_from_data (dst, this_tok->cdata, this_tok->start, e + 1);
	return dst;
}

ArikkeiToken *
arikkei_token_strip_end_ws (ArikkeiToken *this_tok, ArikkeiToken *dst, const gchar *ws)
{
	int len, e;
	len = strlen (ws);
	e = this_tok->end - 1;
	if (this_tok->cdata) {
		while (e >= this_tok->start) {
			int i;
			for (i = 0; i < len; i++) {
				if (this_tok->cdata[e] == ws[i]) break;
			}
			if (i >= len) break;
			e -= 1;
		}
	}
	arikkei_token_set_from_data (dst, this_tok->cdata, this_tok->start, e + 1);
	return dst;
}

ArikkeiToken *
arikkei_token_strip (ArikkeiToken *this_tok, ArikkeiToken *dst)
{
	const gchar *p;
	int s, e;
	p = this_tok->cdata;
	s = this_tok->start;
	e = this_tok->end - 1;
	if (p) {
		while ((s < this_tok->end) && (p[s] <= 32)) s += 1;
		while ((e >= s) && (p[e] <= 32)) e -= 1;
	}
	arikkei_token_set_from_data (dst, this_tok->cdata, s, e + 1);
	return dst;
}

ArikkeiToken *
arikkei_token_strip_ws (ArikkeiToken *this_tok, ArikkeiToken *dst, const gchar *ws)
{
	int len, s, e;
	len = strlen (ws);
	s = this_tok->start;
	e = this_tok->end - 1;
	if (this_tok->cdata) {
		while (s < this_tok->end) {
			int i;
			for (i = 0; i < len; i++) {
				if (this_tok->cdata[s] == ws[i]) break;
			}
			if (i >= len) break;
			s += 1;
		}
		while (e >= s) {
			int i;
			for (i = 0; i < len; i++) {
				if (this_tok->cdata[e] == ws[i]) break;
			}
			if (i >= len) break;
			e -= 1;
		}
	}
	arikkei_token_set_from_data (dst, this_tok->cdata, s, e + 1);
	return dst;
}

gchar *
arikkei_token_strconcat (const ArikkeiToken *tokens, int size, const gchar *separator)
{
	gchar *str, *p;
	int slen, len, i;
	slen = strlen (separator);
	len = 1;
	for (i = 0; i < size; i++) len += (tokens[i].end - tokens[i].start);
	len += (size - 1) * slen;
	str = (gchar*)malloc (len + 1);
	p = str;
	for (i = 0; i < size; i++) {
		if ((i > 0) && (slen > 0)) {
			strncpy (p, separator, slen);
			p += slen;
		}
		p += arikkei_token_strcpy (tokens + i, p);
	}
	return str;
}







