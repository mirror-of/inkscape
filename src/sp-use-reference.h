#ifndef SEEN_SP_USE_REFERENCE_H
#define SEEN_SP_USE_REFERENCE_H

/*
 * The reference corresponding to href of <use> element.
 *
 * Copyright (C) 2004 Bulia Byak
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <forward.h>
#include <uri-references.h>
#include <sigc++/sigc++.h>


class SPUseReference : public Inkscape::URIReference {
public:
    SPUseReference(SPObject *owner) : URIReference(owner) {}

    SPItem *getObject() const {
        return (SPItem *)URIReference::getObject();
    }

protected:
    virtual bool _acceptObject(SPObject * const obj) const;

};

class Path;
struct SPRepr;

class SPUsePath : public SPUseReference {
public:	
	Path           *originalPath;	
	bool           sourceDirty;
	
	SPObject       *owner;
	gchar					 *sourceHref;
  SPRepr         *sourceRepr;
	SPObject			 *sourceObject;
	
	gulong           _modified_connection;
	SigC::Connection _delete_connection;
	SigC::Connection _changed_connection;
	SigC::Connection _transformed_connection;

	SPUsePath(SPObject* i_owner);
	~SPUsePath(void);
	
	void            link(char* to);
	void            unlink(void);
	void            start_listening(SPObject* to);
	void            quit_listening(void);
	void            refresh_source(void);
};

#endif /* !SEEN_SP_USE_REFERENCE_H */

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
