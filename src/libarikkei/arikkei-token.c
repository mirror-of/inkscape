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

#include <malloc.h>
#include <string.h>

#include "arikkei-token.h"

ArikkeiToken *
arikkei_token_set_from_string (ArikkeiToken *this, const unsigned char *text)
{
	this->cdata = text;
	this->start = 0;
	this->end = (text) ? strlen (text) : 0;
	return this;
}

ArikkeiToken *
arikkei_token_set_from_data (ArikkeiToken *this, const unsigned char *data, int start, int end)
{
	this->cdata = data;
	this->start = start;
	this->end = end;
	return this;
}

ArikkeiToken *
arikkei_token_set_from_token (ArikkeiToken *this, const ArikkeiToken *src)
{
	this->cdata = src->cdata;
	this->start = src->start;
	this->end = src->end;
	return this;
}

unsigned int
arikkei_token_is_equal (const ArikkeiToken *this, const ArikkeiToken *t)
{
	if (!arikkei_token_is_valid (this)) return 0;
	if (!arikkei_token_is_valid (t)) return 0;
	if (!arikkei_token_is_empty (this)) {
		if (!arikkei_token_is_empty (t)) {
			int llen, rlen;
			llen = this->end - this->start;
			rlen = t->end - t->start;
			return ((llen == rlen) && !strncmp (this->cdata + this->start, t->cdata + t->start, llen));
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
arikkei_token_is_equal_string (const ArikkeiToken *this, const unsigned char *t)
{
	if (!arikkei_token_is_valid (this)) return 0;
	return !arikkei_token_strcmp (this, t);
}

unsigned char *
arikkei_token_strdup (const ArikkeiToken *this)
{
	if (arikkei_token_is_valid (this)) {
		unsigned char *b;
		int len;
		len = this->end - this->start;
		b = malloc (len + 1);
		if (len) strncpy (b, this->cdata + this->start, len);
		b[len] = 0;
		return b;
	} else {
		return NULL;
	}
}

int
arikkei_token_strcpy (const ArikkeiToken *this, unsigned char *b)
{
	if (arikkei_token_is_valid (this)) {
		int len;
		len = this->end - this->start;
		if (len) strncpy (b, this->cdata + this->start, len);
		b[len] = 0;
		return len;
	} else {
		b[0] = 0;
		return 0;
	}
}

int
arikkei_token_strncpy (const ArikkeiToken *this, unsigned char *b, size_t size)
{
	if (size < 1) return 0;
	if (arikkei_token_is_valid (this) && (size > 1)) {
		int len;
		len = this->end - this->start;
		if (len > (size - 1)) len = size - 1;
		if (len) strncpy (b, this->cdata + this->start, len);
		b[len] = 0;
		return len;
	} else {
		b[0] = 0;
		return 0;
	}
}

int
arikkei_token_strcmp (const ArikkeiToken *this, const unsigned char *b)
{
	if (!arikkei_token_is_valid (this)) return -1;
	if (b) {
		return arikkei_token_strncmp (this, b, strlen (b));
	} else {
		return arikkei_token_is_empty (this);
	}
}

int
arikkei_token_strncmp (const ArikkeiToken *this, const unsigned char *b, size_t size)
{
	if (!arikkei_token_is_valid (this)) return -1;
	if (!arikkei_token_is_empty (this)) {
		if (size > 0) {
			int len, clen, cval;
			len = this->end - this->start;
			clen = (len < size) ? len : size;
			cval = strncmp (this->cdata + this->start, b, clen);
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
arikkei_token_get_first_line (const ArikkeiToken *this, ArikkeiToken *dst)
{
	return arikkei_token_get_line (this, dst, this->start);
}

ArikkeiToken *
arikkei_token_get_line (const ArikkeiToken *this, ArikkeiToken *dst, int s)
{
	if (!arikkei_token_is_empty (this)) {
		const unsigned char *p;
		int e;
		p = this->cdata;
		e = s;
		while ((e < this->end) && ((p[e] >= 32) || (p[e] == 9))) e += 1;
		arikkei_token_set_from_data (dst, p, s, e);
	} else {
		arikkei_token_set_from_data (dst, this->cdata, 0, 0);
	}
	return dst;
}

ArikkeiToken *
arikkei_token_next_line (const ArikkeiToken *this, ArikkeiToken *dst, const ArikkeiToken *line)
{
	if (!arikkei_token_is_empty (this)) {
		if (arikkei_token_is_valid (line)) {
			const unsigned char *p;
			int s;
			p = this->cdata;
			s = line->end;
			while ((s < this->end) && (p[s] < 32) && (p[s] != 9)) s += 1;
			return arikkei_token_get_line (this, dst, s);
		} else {
			return arikkei_token_get_first_line (this, dst);
		}
	} else {
		arikkei_token_set_from_data (dst, this->cdata, 0, 0);
	}
	return dst;
}

ArikkeiToken *
arikkei_token_get_token (const ArikkeiToken *this, ArikkeiToken *dst, int s, unsigned int space_is_separator)
{
	if (!arikkei_token_is_empty (this)) {
		unsigned char *p;
		while ((s < this->end) && (p[s] == 32)) s += 1;
		if (s < this->end) {
			int e;
			while ((e < this->end) && ((p[e] > 32) || ((p[e] == 32) && !space_is_separator))) e += 1;
			arikkei_token_set_from_data (dst, this->cdata, s, e);
		} else {
			arikkei_token_set_from_data (dst, this->cdata, s, this->end);
		}
	} else {
		arikkei_token_set_from_data (dst, this->cdata, 0, 0);
	}
	return dst;
}

ArikkeiToken *
arikkei_token_next_token (const ArikkeiToken *this, ArikkeiToken *dst, const ArikkeiToken *token, unsigned int space_is_separator)
{
	if (!arikkei_token_is_empty (this)) {
		if (arikkei_token_is_valid (token)) {
			return arikkei_token_get_token (this, dst, token->end + 1, space_is_separator);
		} else {
			return arikkei_token_get_token (this, dst, this->start, space_is_separator);
		}
	} else {
		arikkei_token_set_from_data (dst, this->cdata, 0, 0);
	}
	return dst;
}

int
arikkei_token_tokenize (ArikkeiToken *this, ArikkeiToken *tokens, int maxtokens, unsigned int space_is_separator, unsigned int multi)
{
	const unsigned char *p;
	int ntokens, s;
	if (arikkei_token_is_empty (this)) return 0;
	ntokens = 0;
	p = this->cdata;
	s = this->start;
	while ((s < this->end) && (ntokens < maxtokens)) {
		int e;
		while ((e < this->end) && ((p[e] > 32) || ((p[e] == 32) && !space_is_separator))) e += 1;
		if (ntokens == (maxtokens - 1)) {
			while ((e < this->end) && ((p[e] >= 32) || (p[e] == 9))) e += 1;
		}
		arikkei_token_set_from_data (tokens + ntokens, this->cdata, s, e);
		s = e + 1;
		if (multi) {
			while ((s < this->end) && ((p[s] < 32) || ((p[s] == 32) && space_is_separator))) s += 1;
		}
		ntokens += 1;
	}
	return ntokens;
}

int
arikkei_token_tokenize_ws (ArikkeiToken *this, ArikkeiToken *tokens, int maxtokens, const unsigned char *ws, unsigned int multi)
{
	int len, ntokens, s;
	if (arikkei_token_is_empty (this)) return 0;
	len = strlen (ws);
	ntokens = 0;
	s = this->start;
	while ((s < this->end) && (ntokens < maxtokens)) {
		int e;
		if (ntokens != (maxtokens - 1)) {
			e = s;
			while (e < this->end) {
				int i;
				for (i = 0; i < len; i++) {
					if (this->cdata[e] == ws[i]) break;
				}
				if (i < len) break;
				e += 1;
			}
		} else {
			e = this->end;
		}
		arikkei_token_set_from_data (tokens + ntokens, this->cdata, s, e);
		s = e + 1;
		if (multi) {
			while (s < this->end) {
				int i;
				for (i = 0; i < len; i++) {
					if (this->cdata[s] == ws[i]) break;
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
arikkei_token_strip_start (ArikkeiToken *this, ArikkeiToken *dst)
{
	const unsigned char *p;
	int s;
	p = this->cdata;
	s = this->start;
	if (p) {
		while ((s < this->end) && (p[s] <= 32)) s += 1;
	}
	arikkei_token_set_from_data (dst, this->cdata, s, this->end);
	return dst;
}

ArikkeiToken *
arikkei_token_strip_start_ws (ArikkeiToken *this, ArikkeiToken *dst, const unsigned char *ws)
{
	int len, s;
	len = strlen (ws);
	s = this->start;
	if (this->cdata) {
		while (s < this->end) {
			int i;
			for (i = 0; i < len; i++) {
				if (this->cdata[s] == ws[i]) break;
			}
			if (i >= len) break;
			s += 1;
		}
	}
	arikkei_token_set_from_data (dst, this->cdata, s, this->end);
	return dst;
}

ArikkeiToken *
arikkei_token_strip_end (ArikkeiToken *this, ArikkeiToken *dst)
{
	const unsigned char *p;
	int e;
	p = this->cdata;
	e = this->end - 1;
	if (p) {
		while ((e >= this->start) && (p[e] <= 32)) e -= 1;
	}
	arikkei_token_set_from_data (dst, this->cdata, this->start, e + 1);
	return dst;
}

ArikkeiToken *
arikkei_token_strip_end_ws (ArikkeiToken *this, ArikkeiToken *dst, const unsigned char *ws)
{
	int len, e;
	len = strlen (ws);
	e = this->end - 1;
	if (this->cdata) {
		while (e >= this->start) {
			int i;
			for (i = 0; i < len; i++) {
				if (this->cdata[e] == ws[i]) break;
			}
			if (i >= len) break;
			e -= 1;
		}
	}
	arikkei_token_set_from_data (dst, this->cdata, this->start, e + 1);
	return dst;
}

ArikkeiToken *
arikkei_token_strip (ArikkeiToken *this, ArikkeiToken *dst)
{
	const unsigned char *p;
	int s, e;
	p = this->cdata;
	s = this->start;
	e = this->end - 1;
	if (p) {
		while ((s < this->end) && (p[s] <= 32)) s += 1;
		while ((e >= s) && (p[e] <= 32)) e -= 1;
	}
	arikkei_token_set_from_data (dst, this->cdata, s, e + 1);
	return dst;
}

ArikkeiToken *
arikkei_token_strip_ws (ArikkeiToken *this, ArikkeiToken *dst, const unsigned char *ws)
{
	int len, s, e;
	len = strlen (ws);
	s = this->start;
	e = this->end - 1;
	if (this->cdata) {
		while (s < this->end) {
			int i;
			for (i = 0; i < len; i++) {
				if (this->cdata[s] == ws[i]) break;
			}
			if (i >= len) break;
			s += 1;
		}
		while (e >= s) {
			int i;
			for (i = 0; i < len; i++) {
				if (this->cdata[e] == ws[i]) break;
			}
			if (i >= len) break;
			e -= 1;
		}
	}
	arikkei_token_set_from_data (dst, this->cdata, s, e + 1);
	return dst;
}

unsigned char *
arikkei_token_strconcat (const ArikkeiToken *tokens, int size, const unsigned char *separator)
{
	unsigned char *str, *p;
	int slen, len, i;
	slen = strlen (separator);
	len = 1;
	for (i = 0; i < size; i++) len += (tokens[i].end - tokens[i].start);
	len += (size - 1) * slen;
	str = malloc (len + 1);
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







