#ifndef SEEN_SP_CONN_END_PAIR
#define SEEN_SP_CONN_END_PAIR

#include <glib/gtypes.h>

#include "xml/xml-forward.h"
#include "forward.h"
class SPConnEnd;


class SPConnEndPair {
public:
    SPConnEndPair(SPPath *);
    ~SPConnEndPair();
    void release();
    void setAttr(unsigned const key, gchar const *const value);
    void writeRepr(SPRepr *const repr) const;
    void getAttachedItems(SPItem *[2]) const;

private:
    SPConnEnd *_connEnd[2];
};


void sp_conn_end_pair_build(SPObject *object);


#endif /* !SEEN_SP_CONN_END_PAIR */

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
