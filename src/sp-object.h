#ifndef __SP_OBJECT_H__
#define __SP_OBJECT_H__

/*
 * Abstract base class for all nodes
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/* SPObject flags */

/* Async modification flags */
#define SP_OBJECT_MODIFIED_FLAG (1 << 0)
#define SP_OBJECT_CHILD_MODIFIED_FLAG (1 << 1)
#define SP_OBJECT_PARENT_MODIFIED_FLAG (1 << 2)
#define SP_OBJECT_STYLE_MODIFIED_FLAG (1 << 3)
#define SP_OBJECT_VIEWPORT_MODIFIED_FLAG (1 << 4)
#define SP_OBJECT_USER_MODIFIED_FLAG_A (1 << 5)
#define SP_OBJECT_USER_MODIFIED_FLAG_B (1 << 6)
#define SP_OBJECT_USER_MODIFIED_FLAG_C (1 << 7)

/* Conveneience */
#define SP_OBJECT_FLAGS_ALL 0xff

/* Flags that mark object as modified */
/* Object, Child, Style, Viewport, User */
#define SP_OBJECT_MODIFIED_STATE (SP_OBJECT_FLAGS_ALL & ~(SP_OBJECT_PARENT_MODIFIED_FLAG))

/* Flags that will propagate downstreams */
/* Parent, Style, Viewport, User */
#define SP_OBJECT_MODIFIED_CASCADE (SP_OBJECT_FLAGS_ALL & ~(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))

/* Generic */
#define SP_OBJECT_IS_CLONED(o) (((SPObject *) (o))->cloned)

/* Write flags */
#define SP_OBJECT_WRITE_BUILD (1 << 0)
#define SP_OBJECT_WRITE_EXT (1 << 1)
#define SP_OBJECT_WRITE_ALL (1 << 2)

/* Convenience stuff */
#define SP_OBJECT_ID(o) (((SPObject *) (o))->id)
#define SP_OBJECT_REPR(o) (((SPObject *)  (o))->repr)
#define SP_OBJECT_DOCUMENT(o) (((SPObject *) (o))->document)
#define SP_OBJECT_PARENT(o) (((SPObject *) (o))->parent)
#define SP_OBJECT_NEXT(o) (((SPObject *) (o))->next)
#define SP_OBJECT_HREFCOUNT(o) (((SPObject *) (o))->hrefcount)
#define SP_OBJECT_STYLE(o) (((SPObject *) (o))->style)
#define SP_OBJECT_TITLE(o) sp_object_title_get ((SPObject *) (o))
#define SP_OBJECT_DESCRIPTION(o) sp_object_description_get ((SPObject *) (o))

#include <sigc++/sigc++.h>
#include <glib-object.h>

#include "forward.h"
#include "version.h"
#include "xml/xml-forward.h"

typedef void (* SPObjectMethod) (SPObject *object, gpointer data);

typedef enum {
	SP_NO_EXCEPTION,
	SP_INDEX_SIZE_ERR,
	SP_DOMSTRING_SIZE_ERR,
	SP_HIERARCHY_REQUEST_ERR,
	SP_WRONG_DOCUMENT_ERR,
	SP_INVALID_CHARACTER_ERR,
	SP_NO_DATA_ALLOWED_ERR,
	SP_NO_MODIFICATION_ALLOWED_ERR,
	SP_NOT_FOUND_ERR,
	SP_NOT_SUPPORTED_ERR,
	SP_INUSE_ATTRIBUTE_ERR,
	SP_INVALID_STATE_ERR,
	SP_SYNTAX_ERR,
	SP_INVALID_MODIFICATION_ERR,
	SP_NAMESPACE_ERR,
	SP_INVALID_ACCESS_ERR
} SPExceptionType;

class SPException;

struct SPException {
	SPExceptionType code;
};

#define SP_EXCEPTION_INIT(ex) {(ex)->code = SP_NO_EXCEPTION;}
#define SP_EXCEPTION_IS_OK(ex) (!(ex) || ((ex)->code == SP_NO_EXCEPTION))

class SPCtx;

struct SPCtx {
	/* Unused */
	unsigned int flags;
};

enum {
	SP_XML_SPACE_DEFAULT,
	SP_XML_SPACE_PRESERVE
};

class SPIXmlSpace;
struct SPIXmlSpace {
	guint set : 1;
	guint value : 1;
};

class SPObject;

/*
 * Refcounting
 *
 * Owner is here for debug reasons, you can set it to NULL safely
 * Ref should return object, NULL is error, unref return always NULL
 */

SPObject *sp_object_ref (SPObject *object, SPObject *owner);
SPObject *sp_object_unref (SPObject *object, SPObject *owner);

SPObject *sp_object_href (SPObject *object, gpointer owner);
SPObject *sp_object_hunref (SPObject *object, gpointer owner);

struct SPObject : public GObject {
	enum CollectionPolicy {
		COLLECT_WITH_PARENT,
		ALWAYS_COLLECT
	};

	unsigned int cloned : 1;
	unsigned int uflags : 8;
	unsigned int mflags : 8;
	SPIXmlSpace xml_space;
	unsigned int hrefcount; /* number of xlink:href references */
	unsigned int _total_hrefcount; /* our hrefcount + total descendants */
	SPDocument *document; /* Document we are part of */
	SPObject *parent; /* Our parent (only one allowed) */
	SPObject *children; /* Our children */
	SPObject *next; /* Next object in linked list */
	SPRepr *repr; /* Our xml representation */
	gchar *id; /* Our very own unique id */
	SPStyle *style;

	bool isAncestorOf(SPObject *object);

	SPObject *firstChild() { return children; }

	SPObject *appendChildRepr(SPRepr *repr);

	CollectionPolicy collectionPolicy() const { return _collection_policy; }
	void setCollectionPolicy(CollectionPolicy policy) {
		_collection_policy = policy;
	}
	void requestOrphanCollection();
	void collectOrphan() {
		if ( _total_hrefcount == 0 ) {
			deleteObject(false);
		}
	}

	void deleteObject(bool propagate, bool propagate_descendants);
	void deleteObject(bool propagate=true) {
		deleteObject(propagate, propagate);
	}

	SigC::Connection connectDelete(SigC::Slot1<void, SPObject *> slot) {
		return _delete_signal.connect(slot);
	}

	/* successor is the SPObject which has replaced this one (if any);
	 * it is mainly useful for ensuring we can correctly perform a
	 * series of moves or deletes, even if the objects in question
	 * have been replaced in the middle of the sequence.
	 */
	SPObject *successor() { return _successor; }
	void setSuccessor(SPObject *successor) {
		g_assert(successor != NULL);
		g_assert(_successor == NULL);
		g_assert(successor->_successor == NULL);
		sp_object_ref(successor, NULL);
		_successor = successor;
	}

	/* modifications; all three sets of methods should
	 * probably ultimately be protected, as they are not
	 * really part of its public interface.  However,
	 * other parts of the code to occasionally use them at
	 * present. */

	/** @brief Updates the object's repr based on the object's
	 *         state.
	 *
	 *  This method updates the the repr attached to the object
	 *  to reflect the object's current state; see the two-argument
	 *  version for details.
	 *
	 *  @param flags object write flags that apply to this update
	 *
	 *  @return the updated repr
	 */
	SPRepr *updateRepr(unsigned int flags=SP_OBJECT_WRITE_EXT);

	/** @brief Updates the given repr based on the object's
	 *         state.
	 *
	 *  This method updates the given repr to reflect the object's
	 *  current state.  There are several flags that affect this:
	 *
	 *   SP_OBJECT_WRITE_BUILD - create new reprs
	 *
	 *   SP_OBJECT_WRITE_EXT   - write elements and attributes
	 *                           which are not part of pure SVG
	 *                           (i.e. the Inkscape and Sodipodi
	 *                           namespaces)
	 *
	 *   SP_OBJECT_WRITE_ALL   - create all nodes and attributes,
	 *                           even those which might be redundant
	 *
	 *  @param repr the repr to update
	 *  @param flags object write flags that apply to this update
	 *
	 *  @return the updated repr
	 */
	SPRepr *updateRepr(SPRepr *repr, unsigned int flags);

	/** @brief Queues an deferred update of this object's display.
	 *
	 *  This method sets flags to indicate updates to be performed
	 *  later, during the idle loop.
	 *
	 *  There are several flags permitted here:
	 *
	 *   SP_OBJECT_MODIFIED_FLAG - the object has been modified
	 *
	 *   SP_OBJECT_CHILD_MODIFIED_FLAG - a child of the object has been
	 *                                   modified
	 *
	 *   SP_OBJECT_STYLE_MODIFIED_FLAG - the object's style has been
	 *                                   modified
	 *
	 *  There are also some subclass-specific modified flags
	 *  which are hardly ever used.
	 *
	 *  One of either MODIFIED or CHILD_MODIFIED is required.
	 *
	 *  @param flags flags indicating what to update
	 */
	void requestDisplayUpdate(unsigned int flags);

	/** @brief Updates the object's display immediately
	 *
	 *  This method is called during the idle loop by SPDocument
	 *  in order to update the object's display.
	 *
	 *  One additional flag is legal here:
	 *
	 *   SP_OBJECT_PARENT_MODIFIED_FLAG - the parent has been
	 *                                    modified
	 *
	 *  @param ctx an SPCtx which accumulates various state
	 *             during the recursive update -- beware! some
	 *             subclasses try to cast this to an SPItemCtx *
	 *
	 *  @param flags flags indicating what to update (in addition
	 *               to any already set flags)
	 */
	void updateDisplay(SPCtx *ctx, unsigned int flags);

	/** @brief Requests that a modification notification signal
	 *         be emitted later (e.g. during the idle loop)
	 *
	 *  @param flags flags indicating what has been modified
	 */
	void requestModified(unsigned int flags);

	/** @brief Emits a modification notification signal
	 *
	 *  @param flags indicating what has been modified
	 */
	void emitModified(unsigned int flags);

	void _sendDeleteSignalRecursive();
	void _updateTotalHRefCount(int increment);

	SigC::Signal1<void, SPObject *> _delete_signal;
	SPObject *_successor;
	CollectionPolicy _collection_policy;
};

struct SPObjectClass {
	GObjectClass parent_class;

	void (* build) (SPObject *object, SPDocument *doc, SPRepr *repr);
	void (* release) (SPObject *object);

	/* Virtual handlers of repr signals */
	void (* child_added) (SPObject *object, SPRepr *child, SPRepr *ref);
	void (* remove_child) (SPObject *object, SPRepr *child);

	void (* order_changed) (SPObject *object, SPRepr *child, SPRepr *old, SPRepr *new_repr);

	void (* set) (SPObject *object, unsigned int key, const gchar *value);

	void (* read_content) (SPObject *object);

	/* Update handler */
	void (* update) (SPObject *object, SPCtx *ctx, unsigned int flags);
	/* Modification handler */
	void (* modified) (SPObject *object, unsigned int flags);

	SPRepr * (* write) (SPObject *object, SPRepr *repr, unsigned int flags);
};

/*
 * Attaching/detaching
 *
 * Attach returns object itself, or NULL on error
 * Detach returns next object, NULL on error
 */

void sp_object_attach_reref (SPObject *parent, SPObject *object, SPObject *next);
void sp_object_reorder(SPObject *object, SPObject *next);
void sp_object_detach (SPObject *parent, SPObject *object);
void sp_object_detach_unref (SPObject *parent, SPObject *object);

SPObject *sp_object_first_child(SPObject *parent);
SPObject *sp_object_last_child(SPObject *parent);
SPObject *sp_object_get_child_by_repr(SPObject *object, SPRepr *repr);

void sp_object_invoke_build (SPObject * object, SPDocument * document, SPRepr * repr, unsigned int cloned);
void sp_object_invoke_release (SPObject *object);

void sp_object_set (SPObject *object, unsigned int key, const gchar *value);

void sp_object_read_attr (SPObject *object, const gchar *key);

/*
 * Get and set descriptive parameters
 *
 * These are inefficent, so they are not intended to be used interactively
 */

const gchar *sp_object_title_get (SPObject *object);
const gchar *sp_object_description_get (SPObject *object);
unsigned int sp_object_title_set (SPObject *object, const gchar *title);
unsigned int sp_object_description_set (SPObject *object, const gchar *desc);

/* Public */

const gchar *sp_object_tagName_get (const SPObject *object, SPException *ex);
const gchar *sp_object_getAttribute (const SPObject *object, const gchar *key, SPException *ex);
void sp_object_setAttribute (SPObject *object, const gchar *key, const gchar *value, SPException *ex);
void sp_object_removeAttribute (SPObject *object, const gchar *key, SPException *ex);

/* Style */

const gchar *sp_object_get_style_property (SPObject *object, const gchar *key, const gchar *def);

SPVersion sp_object_get_sodipodi_version (SPObject *object);

#endif
