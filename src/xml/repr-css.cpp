#define SP_REPR_CSS_C

#include <math.h>
#include <string.h>
#include <stdio.h>

#include <glib.h>

#include "xml/repr.h"
#include "xml/attribute-record.h"
#include "xml/sp-css-attr.h"
#include "xml/simple-node.h"

struct SPCSSAttrImpl : public Inkscape::XML::SimpleNode, public SPCSSAttr {
public:
	SPCSSAttrImpl() : SimpleNode(g_quark_from_static_string("css")) {}

	Inkscape::XML::NodeType type() const { return Inkscape::XML::ELEMENT_NODE; }

protected:
	SimpleNode *_duplicate() const { return new SPCSSAttrImpl(*this); }
};

static void sp_repr_css_add_components (SPCSSAttr * css, Inkscape::XML::Node * repr, const gchar * attr);

SPCSSAttr *
sp_repr_css_attr_new (void)
{
	return new SPCSSAttrImpl();
}

void
sp_repr_css_attr_unref (SPCSSAttr * css)
{
	g_assert (css != NULL);
	sp_repr_unref ((Inkscape::XML::Node *) css);
}

SPCSSAttr * sp_repr_css_attr (Inkscape::XML::Node * repr, const gchar * attr)
{
	SPCSSAttr * css;

	g_assert (repr != NULL);
	g_assert (attr != NULL);

	css = sp_repr_css_attr_new ();

	sp_repr_css_add_components (css, repr, attr);

	return css;
}

SPCSSAttr * sp_repr_css_attr_inherited (Inkscape::XML::Node * repr, const gchar * attr)
{
	SPCSSAttr * css;
	Inkscape::XML::Node * current;

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
sp_repr_css_add_components (SPCSSAttr * css, Inkscape::XML::Node * repr, const gchar * attr)
{
	g_assert (css != NULL);
	g_assert (repr != NULL);
	g_assert (attr != NULL);

	const char *data = repr->attribute(attr);

	sp_repr_css_attr_add_from_string (css, data);

	return;
}

const char *
sp_repr_css_property (SPCSSAttr * css, const gchar * name, const gchar * defval)
{
	const char * attr;

	g_assert (css != NULL);
	g_assert (name != NULL);

	attr = ((Inkscape::XML::Node *)css)->attribute(name);

	if (attr == NULL) return defval;

	return attr;
}

void
sp_repr_css_set_property (SPCSSAttr * css, const gchar * name, const gchar * value)
{
	g_assert (css != NULL);
	g_assert (name != NULL);

	sp_repr_set_attr ((Inkscape::XML::Node *) css, name, value);
}

double
sp_repr_css_double_property (SPCSSAttr * css, const gchar * name, double defval)
{
	g_assert (css != NULL);
	g_assert (name != NULL);

	return sp_repr_get_double_attribute ((Inkscape::XML::Node *) css, name, defval);
}

void
sp_repr_css_set (Inkscape::XML::Node * repr, SPCSSAttr * css, const gchar * attr)
{
	Inkscape::XML::AttributeRecord const * a;
	const char *key;
	const char *val;
	char c[4096], *p;

	g_assert (repr != NULL);
	g_assert (css != NULL);
	g_assert (attr != NULL);

	c[0] = '\0';
	p = c;

	for (a = css->attributeList() ; a != NULL; a = a->next) {
		key = SP_REPR_ATTRIBUTE_KEY (a);
		val = SP_REPR_ATTRIBUTE_VALUE (a);
		p += g_snprintf (p, c + 4096 - p, "%s:%s;", key, val);
	}

	/* Get rid of trailing `;'. */
	if (p != c) {
		--p;
		*p = '\0';
	}

	/* g_print ("style: %s\n", c); */
	sp_repr_set_attr (repr, attr, (c[0]) ? c : NULL);
}

void
sp_repr_css_merge (SPCSSAttr * dst, SPCSSAttr * src)
{
	Inkscape::XML::AttributeRecord const * attr;
	const char * key, * val;

	g_assert (dst != NULL);
	g_assert (src != NULL);

	for (attr = src->attributeList() ; attr != NULL; attr = attr->next) {
		key = SP_REPR_ATTRIBUTE_KEY (attr);
		val = SP_REPR_ATTRIBUTE_VALUE (attr);
		sp_repr_set_attr ((Inkscape::XML::Node *) dst, key, val);
	}
}

void
sp_repr_css_attr_add_from_string (SPCSSAttr *css, const gchar *data)
{
	if (data != NULL) {
		char *new_str = g_strdup (data);
		char **token = g_strsplit (new_str, ";", 32);
		for (char **ctoken = token; *ctoken != NULL; ctoken++) {
			char *current = g_strstrip (* ctoken);
			char *key = current;
			char *val;
			for (val = key; *val != '\0'; val++)
				if (*val == ':') break;
			if (*val == '\0') break;
			*val++ = '\0';
			key = g_strstrip (key);
			val = g_strstrip (val);
			if (*val == '\0') break;

			if (!css->attribute(key))
				sp_repr_set_attr ((Inkscape::XML::Node *) css, key, val);
		}
		g_strfreev (token);
		g_free (new_str);
	}
}

void
sp_repr_css_print (SPCSSAttr * css)
{
	g_print ("== SPCSSAttr:\n");
	for (Inkscape::XML::AttributeRecord const *attr = css->attributeList(); attr != NULL; attr = attr->next) {
		g_print("%s: %s\n", SP_REPR_ATTRIBUTE_KEY(attr), SP_REPR_ATTRIBUTE_VALUE(attr).cString());
	}
}

void
sp_repr_css_change (Inkscape::XML::Node * repr, SPCSSAttr * css, const gchar * attr)
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
sp_repr_css_change_recursive (Inkscape::XML::Node * repr, SPCSSAttr * css, const gchar * attr)
{
	Inkscape::XML::Node * child;

	g_assert (repr != NULL);
	g_assert (css != NULL);
	g_assert (attr != NULL);

	sp_repr_css_change (repr, css, attr);

	for (child = repr->firstChild(); child != NULL; child = child->next()) {
		sp_repr_css_change_recursive (child, css, attr);
	}
}
