#include <glib.h>
#include <libcroco/cr-cascade.h>
#include <libcroco/cr-parser.h>
#include <libcroco/cr-statement.h>
#include <libcroco/cr-string.h>
#include "xml/node.h"
#include "xml/repr.h"
#include "document.h"
#include "sp-style-elem.h"
#include "attributes.h"
using Inkscape::XML::TEXT_NODE;

static void sp_style_elem_init(SPStyleElem *style_elem);
static void sp_style_elem_class_init(SPStyleElemClass *klass);
static void sp_style_elem_build(SPObject *object, SPDocument *doc, Inkscape::XML::Node *repr);
static void sp_style_elem_set(SPObject *object, unsigned const key, gchar const *const value);
static void sp_style_elem_read_content(SPObject *);
static Inkscape::XML::Node *sp_style_elem_write(SPObject *, Inkscape::XML::Node *, guint flags);

static SPObjectClass *parent_class;

GType
sp_style_elem_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPStyleElemClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_style_elem_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPStyleElem),
            16,     /* n_preallocs */
            (GInstanceInitFunc) sp_style_elem_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPStyleElem", &info, (GTypeFlags) 0);
    }

    return type;
}

static void
sp_style_elem_class_init(SPStyleElemClass *klass)
{
    parent_class = (SPObjectClass *)g_type_class_ref(SP_TYPE_OBJECT);
    /* FIXME */

    klass->build = sp_style_elem_build;
    klass->set = sp_style_elem_set;
    klass->read_content = sp_style_elem_read_content;
    klass->write = sp_style_elem_write;
}

static void
sp_style_elem_init(SPStyleElem *style_elem)
{
    media_set_all(style_elem->media);
    style_elem->is_css = false;
}

static void
sp_style_elem_set(SPObject *object, unsigned const key, gchar const *const value)
{
    g_return_if_fail(object);
    SPStyleElem &style_elem = *SP_STYLE_ELEM(object);

    switch (key) {
        case SP_ATTR_TYPE: {
            if (!value) {
                /* TODO: `type' attribute is required.  Give error message as per
                   http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing. */
                style_elem.is_css = false;
            } else {
                /* fixme: determine what whitespace is allowed.  Will probably need to ask on SVG
                 * list; though the relevant RFC may give info on its lexer. */
                style_elem.is_css = ( g_ascii_strncasecmp(value, "text/css", 8) == 0
                                      && ( value[8] == '\0' ||
                                           value[8] == ';'    ) );
            }
            break;
        }

#if 0 /* unfinished */
        case SP_ATTR_MEDIA: {
            parse_media(style_elem, value);
            break;
        }
#endif

        /* title is ignored. */
        default: {
            if (parent_class->set) {
                parent_class->set(object, key, value);
            }
            break;
        }
    }
}

static Inkscape::XML::Node *
sp_style_elem_write(SPObject *const object, Inkscape::XML::Node *repr, guint const flags)
{
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = sp_repr_new("svg:style");
    }

    g_return_val_if_fail(object, repr);
    SPStyleElem &style_elem = *SP_STYLE_ELEM(object);
    if (flags & SP_OBJECT_WRITE_BUILD) {
        g_warning("nyi: Forming <style> content for SP_OBJECT_WRITE_BUILD.");
        /* fixme: Consider having the CRStyleSheet be a member of SPStyleElem, and then
           pretty-print to a string s, then sp_repr_add_child(repr, sp_repr_new_text(s), NULL). */
    }
    if (style_elem.is_css) {
        repr->setAttribute("type", "text/css");
    }
    /* todo: media */

    if (((SPObjectClass *) parent_class)->write)
        ((SPObjectClass *) parent_class)->write(object, repr, flags);

    return repr;
}


/** Returns the concatenation of the content of the text children of the specified object. */
static GString *
concat_children(Inkscape::XML::Node const &repr)
{
    GString *ret = g_string_sized_new(0);
    // effic: 0 is just to catch bugs.  Increase to something reasonable.
    for (Inkscape::XML::Node const *rch = repr.firstChild(); rch != NULL; rch = rch->next()) {
        if ( rch->type() == TEXT_NODE ) {
            ret = g_string_append(ret, rch->content());
        }
    }
    return ret;
}



/* Callbacks for SAC-style libcroco parser. */

struct ParseTmp
{
    CRStyleSheet *const stylesheet;
    CRStatement *curr_stmt;
    unsigned magic;
    static unsigned const ParseTmp_magic = 0x23474397;  // from /dev/urandom

    ParseTmp(CRStyleSheet *const stylesheet) :
        stylesheet(stylesheet),
        curr_stmt(NULL),
        magic(ParseTmp_magic)
    { }

    bool hasMagic() const {
        return magic == ParseTmp_magic;
    }

    ~ParseTmp()
    {
        g_return_if_fail(hasMagic());
        magic = 0;
    }
};

static void
start_selector_cb(CRDocHandler *a_handler,
                  CRSelector *a_sel_list)
{
    g_return_if_fail(a_handler && a_sel_list);
    ParseTmp *const parse_tmp = static_cast<ParseTmp *>(a_handler->app_data);
    g_return_if_fail(parse_tmp && parse_tmp->hasMagic());
    CRStatement *ruleset = cr_statement_new_ruleset(parse_tmp->stylesheet, a_sel_list, NULL, NULL);
    g_return_if_fail(ruleset && ruleset->type == RULESET_STMT);
    g_return_if_fail(!parse_tmp->curr_stmt);
    parse_tmp->curr_stmt = ruleset;
}

static void
end_selector_cb(CRDocHandler *a_handler,
                CRSelector *a_sel_list)
{
    g_return_if_fail(a_handler && a_sel_list);
    ParseTmp *const parse_tmp = static_cast<ParseTmp *>(a_handler->app_data);
    g_return_if_fail(parse_tmp && parse_tmp->hasMagic());
    CRStatement *const ruleset = parse_tmp->curr_stmt;
    g_return_if_fail(ruleset && ruleset->type == RULESET_STMT);
    g_return_if_fail(ruleset->kind.ruleset->sel_list == a_sel_list);
    parse_tmp->stylesheet->statements = cr_statement_append(parse_tmp->stylesheet->statements,
                                                            ruleset);
    parse_tmp->curr_stmt = NULL;
}

static void
property_cb(CRDocHandler *const a_handler,
            CRString *const a_name,
            CRTerm *const a_value, gboolean const a_important)
{
    g_return_if_fail(a_handler && a_name);
    ParseTmp *const parse_tmp = static_cast<ParseTmp *>(a_handler->app_data);
    g_return_if_fail(parse_tmp && parse_tmp->hasMagic());
    CRStatement *const ruleset = parse_tmp->curr_stmt;
    g_return_if_fail(ruleset
                     && ruleset->type == RULESET_STMT);
    CRDeclaration *const decl = cr_declaration_new(ruleset, cr_string_dup(a_name), a_value);
    g_return_if_fail(decl);
    decl->important = a_important;
    CRStatus const append_status = cr_statement_ruleset_append_decl(ruleset, decl);
    g_return_if_fail(append_status == CR_OK);
}

static void
sp_style_elem_read_content(SPObject *const object)
{
    SPStyleElem &style_elem = *SP_STYLE_ELEM(object);

    /* fixme: If there's more than one <style> element in a document, then the document stylesheet
     * will be set to a random one of them, even switching between them.
     *
     * However, I don't see in the spec what's supposed to happen when there are multiple <style>
     * elements.  The wording suggests that <style>'s content should be a full stylesheet.
     * http://www.w3.org/TR/REC-CSS2/cascade.html#cascade says that "The author specifies style
     * sheets for a source document according to the conventions of the document language. For
     * instance, in HTML, style sheets may be included in the document or linked externally."
     * (Note the plural in both sentences.)  Whereas libcroco's CRCascade allows only one author
     * stylesheet.  CRStyleSheet has no next/prev members that I can see, nor can I see any append
     * stuff.
     *
     * Dodji replies "right, that's *bug*"; just an unexpected oversight.
     */

    GString *const text = concat_children(*style_elem.repr);
    CRParser *parser = cr_parser_new_from_buf(reinterpret_cast<guchar *>(text->str), text->len,
                                              CR_UTF_8, FALSE);

    /* I see a cr_statement_parse_from_buf for returning a CRStatement*, but no corresponding
       cr_stylesheet_parse_from_buf.  And cr_statement_parse_from_buf takes a char*, not a
       CRInputBuf, and doesn't provide a way for calling it in a loop over the one buffer.
       (I.e. it doesn't tell us where it got up to in the buffer.)

       There's also the generic cr_parser_parse_stylesheet (or just cr_parser_parse), but that
       just calls user-supplied callbacks rather than constructing a CRStylesheet.
    */
    CRDocHandler *sac_handler = cr_doc_handler_new();
    // impl: ref_count inited to 0, so cr_parser_destroy suffices to delete sac_handler.
    g_return_if_fail(sac_handler);  // out of memory
    CRStyleSheet *const stylesheet = cr_stylesheet_new(NULL);
    ParseTmp parse_tmp(stylesheet);
    sac_handler->app_data = &parse_tmp;
    sac_handler->start_selector = start_selector_cb;
    sac_handler->end_selector = end_selector_cb;
    sac_handler->property = property_cb;
    /* todo: start_media, end_media. */
    /* todo: Test error condition. */
    cr_parser_set_sac_handler(parser, sac_handler);
    CRStatus const parse_status = cr_parser_parse(parser);
    g_return_if_fail(parse_status == CR_OK);  // fixme: this is just for debugging.
    g_assert(sac_handler->app_data == &parse_tmp);
    cr_cascade_set_sheet(style_elem.document->style_cascade, stylesheet, ORIGIN_AUTHOR);

    cr_parser_destroy(parser);

    //object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_style_elem_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) parent_class)->build) {
        ((SPObjectClass *) parent_class)->build(object, document, repr);
    }

    sp_style_elem_read_content(object);

    sp_object_read_attr(object, "type");
    sp_object_read_attr(object, "media");
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
