#define SP_REPR_CSS_C

#include <math.h>
#include <string.h>
#include <stdio.h>

#include <glib.h>

#include "repr.h"
#include "repr-private.h"

struct _SPCSSAttr {
	SPRepr repr;
};

static void sp_repr_css_add_components (SPCSSAttr * css, SPRepr * repr, const char * attr);

SPCSSAttr *
sp_repr_css_attr_new (void)
{
	SPRepr * css;
	css = sp_repr_new ("css");
	g_return_val_if_fail (css != NULL, NULL);
	return (SPCSSAttr *) css;
}

void
sp_repr_css_attr_unref (SPCSSAttr * css)
{
	g_assert (css != NULL);
	sp_repr_unref ((SPRepr *) css);
}

SPCSSAttr * sp_repr_css_attr (SPRepr * repr, const char * attr)
{
	SPCSSAttr * css;

	g_assert (repr != NULL);
	g_assert (attr != NULL);

	css = sp_repr_css_attr_new ();

	sp_repr_css_add_components (css, repr, attr);

	return css;
}

SPCSSAttr * sp_repr_css_attr_inherited (SPRepr * repr, const char * attr)
{
	SPCSSAttr * css;
	SPRepr * current;

	g_assert (repr != NULL);
	g_assert (attr != NULL);

	css = sp_repr_css_attr_new ();

	sp_repr_css_add_components (css, repr, attr);
	current = sp_repr_parent (repr);

	while (current) {
		sp_repr_css_add_components (css, current, attr);
		current = sp_repr_parent (current);
	}

	return css;
}

static void
sp_repr_css_add_components (SPCSSAttr * css, SPRepr * repr, const char * attr)
{
	const char * data;
	char * new_str;
	char * current;
	char ** token, ** ctoken;
	char * key, * val;

	g_assert (css != NULL);
	g_assert (repr != NULL);
	g_assert (attr != NULL);

	data = sp_repr_attr (repr, attr);

	if (data != NULL) {
		new_str = g_strdup (data);
		token = g_strsplit (new_str, ";", 32);
		for (ctoken = token; *ctoken != NULL; ctoken++) {
			current = g_strstrip (* ctoken);
			key = current;
			for (val = key; *val != '\0'; val++)
				if (*val == ':') break;
			if (*val == '\0') break;
			*val++ = '\0';
			key = g_strstrip (key);
			val = g_strstrip (val);
			if (*val == '\0') break;

			if (!sp_repr_attr_is_set ((SPRepr *) css, key))
				sp_repr_set_attr ((SPRepr *) css, key, val);
		}
		g_strfreev (token);
		g_free (new_str);
	}

	return;
}

const char *
sp_repr_css_property (SPCSSAttr * css, const char * name, const char * defval)
{
	const char * attr;

	g_assert (css != NULL);
	g_assert (name != NULL);

	attr = sp_repr_attr ((SPRepr *) css, name);

	if (attr == NULL) return defval;

	return attr;
}

void
sp_repr_css_set_property (SPCSSAttr * css, const char * name, const char * value)
{
	g_assert (css != NULL);
	g_assert (name != NULL);

	sp_repr_set_attr ((SPRepr *) css, name, value);
}

double
sp_repr_css_double_property (SPCSSAttr * css, const char * name, double defval)
{
	g_assert (css != NULL);
	g_assert (name != NULL);

	return sp_repr_get_double_attribute ((SPRepr *) css, name, defval);
}

void
sp_repr_css_set (SPRepr * repr, SPCSSAttr * css, const char * attr)
{
	SPReprAttr * a;
	const char *key;
	char *val;
	char c[4096], *p;

	g_assert (repr != NULL);
	g_assert (css != NULL);
	g_assert (attr != NULL);

	c[0] = '\0';
	p = c;

	for (a = ((SPRepr *) css)->attributes; a != NULL; a = a->next) {
		key = SP_REPR_ATTRIBUTE_KEY (a);
		val = SP_REPR_ATTRIBUTE_VALUE (a);
		p += g_snprintf (p, c + 4096 - p, "%s:%s;", key, val);
	}
	/* g_print ("style: %s\n", c); */
	sp_repr_set_attr (repr, attr, (c[0]) ? c : NULL);
}

static void
sp_repr_css_merge (SPCSSAttr * dst, SPCSSAttr * src)
{
	SPReprAttr * attr;
	const char * key, * val;

	g_assert (dst != NULL);
	g_assert (src != NULL);

	for (attr = ((SPRepr *) src)->attributes; attr != NULL; attr = attr->next) {
		key = SP_REPR_ATTRIBUTE_KEY (attr);
		val = SP_REPR_ATTRIBUTE_VALUE (attr);
		sp_repr_set_attr ((SPRepr *) dst, key, val);
	}

}

void
sp_repr_css_change (SPRepr * repr, SPCSSAttr * css, const char * attr)
{
	SPCSSAttr * current;

	g_assert (repr != NULL);
	g_assert (css != NULL);
	g_assert (attr != NULL);

	current = sp_repr_css_attr (repr, attr);
	sp_repr_css_merge (current, css);
	sp_repr_css_set (repr, current, attr);

	sp_repr_css_attr_unref (current);
}

void
sp_repr_css_change_recursive (SPRepr * repr, SPCSSAttr * css, const char * attr)
{
	SPRepr * child;

	g_assert (repr != NULL);
	g_assert (css != NULL);
	g_assert (attr != NULL);

	sp_repr_css_change (repr, css, attr);

	for (child = repr->children; child != NULL; child = child->next) {
		sp_repr_css_change_recursive (child, css, attr);
	}
}

