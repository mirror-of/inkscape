/*
 * Inkscape::ObjectHierarchy - tracks a hierarchy of active SPObjects
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_OBJECT_HIERARCHY_H
#define SEEN_INKSCAPE_OBJECT_HIERARCHY_H

#include <exception>
#include <list>
#include <glib/gmessages.h>

class SPObject;

namespace Inkscape {

/**
 * An Inkscape::ObjectHierarchy is useful for situations where one wishes
 * to keep a reference to an SPObject, but fall back on one of its ancestors
 * when that object is removed.
 *
 * That cannot be accomplished simply by hooking the "release" signal of the
 * SPObject, as by the time that signal is emitted, the object's parent
 * field has already been cleared.
 *
 * There are also some subtle refcounting issues to take into account.
 *
 * @see SPObject
 */

class ObjectHierarchy {
public:
    ObjectHierarchy(SPObject *top=NULL);
    ~ObjectHierarchy();

    SPObject *top() {
        return !_hierarchy.empty() ? _hierarchy.back().object : NULL;
    }
    void setTop(SPObject *object);

    SPObject *bottom() {
        return !_hierarchy.empty() ? _hierarchy.front().object : NULL;
    }
    void setBottom(SPObject *object);

private:
    struct Record {
        Record(SPObject *o, gulong id) : object(o), handler_id(id) {}

        SPObject *object;
        gulong handler_id;
    };

    ObjectHierarchy(ObjectHierarchy const &); // no copy
    void operator=(ObjectHierarchy const &); // no assign

    void _addTop(SPObject *senior, SPObject *junior);
    void _trimAbove(SPObject *limit);

    void _addBottom(SPObject *senior, SPObject *junior);
    void _trimBelow(SPObject *limit);

    Record _attach(SPObject *object);
    void _detach(Record const &record);

    void _clear() { _trimBelow(NULL); }

    static void _trim_for_release(SPObject *released, ObjectHierarchy *hier);

    std::list<Record> _hierarchy;
};

}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
