#ifndef SEEN_CONN_AVOID_REF
#define SEEN_CONN_AVOID_REF

#include <forward.h>
#include <sigc++/connection.h>
#include "libavoid/shape.h"

class SPAvoidRef {
public:
    SPAvoidRef(SPItem *spitem);
    ~SPAvoidRef();

    SPItem *item;

    // true if avoiding, false if not.
    bool setting;
    bool new_setting;

    // libavoid's internal representation of the item.
    Avoid::ShapeRef *shapeRef;

    // A sigc connection for transformed signal.
    sigc::connection _transformed_connection;

    
    void setAvoid(char const *value);
    void handleSettingChange(void);
};


#endif /* !SEEN_CONN_AVOID_REF */

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
