#include <glib/gmem.h>

#include "attributes.h"
#include "sp-conn-end-pair.h"
#include "sp-conn-end.h"
#include "sp-object.h"
#include "uri.h"
#include "display/curve.h"
#include "libnr/nr-matrix-div.h"
#include "xml/repr.h"

SPConnEndPair::SPConnEndPair(SPPath *const owner)
{
    for (unsigned handle_ix = 0; handle_ix <= 1; ++handle_ix) {
        this->_connEnd[handle_ix] = new SPConnEnd(SP_OBJECT(owner));
        this->_connEnd[handle_ix]->_changed_connection
            = this->_connEnd[handle_ix]->ref.changedSignal()
            .connect(sigc::bind(sigc::ptr_fun(sp_conn_end_href_changed),
                                this->_connEnd[handle_ix], owner, handle_ix));
    }
}

SPConnEndPair::~SPConnEndPair()
{
    for (unsigned handle_ix = 0; handle_ix < 2; ++handle_ix) {
        delete this->_connEnd[handle_ix];
        this->_connEnd[handle_ix] = NULL;
    }
}

void
SPConnEndPair::release()
{
    for (unsigned handle_ix = 0; handle_ix < 2; ++handle_ix) {
        this->_connEnd[handle_ix]->_changed_connection.disconnect();
        this->_connEnd[handle_ix]->_delete_connection.disconnect();
        this->_connEnd[handle_ix]->_transformed_connection.disconnect();
        g_free(this->_connEnd[handle_ix]->href);
        this->_connEnd[handle_ix]->href = NULL;
        this->_connEnd[handle_ix]->ref.detach();
    }
}

void
sp_conn_end_pair_build(SPObject *object)
{
    sp_object_read_attr(object, "inkscape:connection-start");
    sp_object_read_attr(object, "inkscape:connection-end");
}

void
SPConnEndPair::setAttr(unsigned const key, gchar const *const value)
{
    unsigned const handle_ix = key - SP_ATTR_CONNECTION_START;
    g_assert( handle_ix <= 1 );
    this->_connEnd[handle_ix]->setAttacherHref(value);
}

void
SPConnEndPair::writeRepr(SPRepr *const repr) const
{
    for (unsigned handle_ix = 0; handle_ix < 2; ++handle_ix) {
        if (this->_connEnd[handle_ix]->ref.getURI()) {
            char const * const attr_strs[] = {"inkscape:connection-start",
                                              "inkscape:connection-end"};
            gchar *uri_string = this->_connEnd[handle_ix]->ref.getURI()->toString();
            sp_repr_set_attr(repr, attr_strs[handle_ix], uri_string);
            g_free(uri_string);
        }
    }
}

void
SPConnEndPair::getAttachedItems(SPItem *h2attItem[2]) const {
    for (unsigned h = 0; h < 2; ++h) {
        h2attItem[h] = this->_connEnd[h]->ref.getObject();
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
