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
#define SP_OBJECT_WRITE_SODIPODI (1 << 1)
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

#include <glib-object.h>
#include "xml/repr.h"
#include "forward.h"

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

typedef struct _SPException SPException;

struct _SPException {
	SPExceptionType code;
};

#define SP_EXCEPTION_INIT(ex) {(ex)->code = SP_NO_EXCEPTION;}
#define SP_EXCEPTION_IS_OK(ex) (!(ex) || ((ex)->code == SP_NO_EXCEPTION))

typedef struct _SPCtx SPCtx;

struct _SPCtx {
	/* Unused */
	unsigned int flags;
};

struct _SPObject {
	GObject object;
	unsigned int cloned : 1;
	unsigned int uflags : 8;
	unsigned int mflags : 8;
	unsigned int hrefcount; /* number os xlink:href references */
	SPDocument *document; /* Document we are part of */
	SPObject *parent; /* Our parent (only one allowed) */
	SPObject *next; /* Next object in linked list */
	SPRepr *repr; /* Our xml representation */
	gchar *id; /* Our very own unique id */
	SPStyle *style;
};

struct _SPObjectClass {
	GObjectClass parent_class;

	void (* build) (SPObject *object, SPDocument *doc, SPRepr *repr);
	void (* release) (SPObject *object);

	/* Virtual handlers of repr signals */
	void (* child_added) (SPObject *object, SPRepr *child, SPRepr *ref);
	void (* remove_child) (SPObject *object, SPRepr *child);

	void (* order_changed) (SPObject *object, SPRepr *child, SPRepr *old, SPRepr *new);

	void (* set) (SPObject *object, unsigned int key, const unsigned char *value);

	void (* read_content) (SPObject *object);

	/* Update handler */
	void (* update) (SPObject *object, SPCtx *ctx, unsigned int flags);
	/* Modification handler */
	void (* modified) (SPObject *object, unsigned int flags);

	/* Compute next sequence number */
	gint (* sequence) (SPObject *object, gint seq);
	void (* forall) (SPObject *object, SPObjectMethod func, gpointer data);

	SPRepr * (* write) (SPObject *object, SPRepr *repr, unsigned int flags);
};

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

/*
 * Attaching/detaching
 *
 * Attach returns object itself, or NULL on error
 * Detach returns next object, NULL on error
 */

SPObject *sp_object_attach_reref (SPObject *parent, SPObject *object, SPObject *next);
SPObject *sp_object_detach (SPObject *parent, SPObject *object);
SPObject *sp_object_detach_unref (SPObject *parent, SPObject *object);

void sp_object_invoke_build (SPObject * object, SPDocument * document, SPRepr * repr, unsigned int cloned);
void sp_object_invoke_release (SPObject *object);

void sp_object_set (SPObject *object, unsigned int key, const unsigned char *value);

void sp_object_read_attr (SPObject *object, const gchar *key);

/* Styling */

/* Modification */
void sp_object_request_update (SPObject *object, unsigned int flags);
void sp_object_invoke_update (SPObject *object, SPCtx *ctx, unsigned int flags);
void sp_object_request_modified (SPObject *object, unsigned int flags);
void sp_object_invoke_modified (SPObject *object, unsigned int flags);

/* Sequence */
gint sp_object_sequence (SPObject *object, gint seq);

void sp_object_invoke_forall (SPObject *object, SPObjectMethod func, gpointer data);
/* Write object to repr */
SPRepr *sp_object_invoke_write (SPObject *object, SPRepr *repr, unsigned int flags);

/*
 * Get and set descriptive parameters
 *
 * These are inefficent, so they are not intended to be used interactively
 */

const unsigned char *sp_object_title_get (SPObject *object);
const unsigned char *sp_object_description_get (SPObject *object);
unsigned int sp_object_title_set (SPObject *object, const unsigned char *title);
unsigned int sp_object_description_set (SPObject *object, const unsigned char *desc);

/* Public */

const unsigned char *sp_object_tagName_get (const SPObject *object, SPException *ex);
const unsigned char *sp_object_getAttribute (const SPObject *object, const unsigned char *key, SPException *ex);
void sp_object_setAttribute (SPObject *object, const unsigned char *key, const unsigned char *value, SPException *ex);
void sp_object_removeAttribute (SPObject *object, const unsigned char *key, SPException *ex);

/* Style */

const guchar *sp_object_get_style_property (SPObject *object, const gchar *key, const gchar *def);

#endif
