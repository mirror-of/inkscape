#ifndef __SP_XML_REPR_ACTION_H__
#define __SP_XML_REPR_ACTION_H__

typedef struct _SPReprAction SPReprAction;
typedef struct _SPReprActionAdd SPReprActionAdd;
typedef struct _SPReprActionDel SPReprActionDel;
typedef struct _SPReprActionChgAttr SPReprActionChgAttr;
typedef struct _SPReprActionChgContent SPReprActionChgContent;
typedef struct _SPReprActionChgOrder SPReprActionChgOrder;

typedef enum {
	SP_REPR_ACTION_INVALID,
	SP_REPR_ACTION_ADD,
	SP_REPR_ACTION_DEL,
	SP_REPR_ACTION_CHGATTR,
	SP_REPR_ACTION_CHGCONTENT,
	SP_REPR_ACTION_CHGORDER
} SPReprActionType;

struct _SPReprActionAdd {
	SPRepr *child;
	SPRepr *ref;
};

struct _SPReprActionDel {
	SPRepr *child;
	SPRepr *ref;
};

struct _SPReprActionChgAttr {
	int key;
	unsigned char *oldval, *newval;
};

struct _SPReprActionChgContent {
	unsigned char *oldval, *newval;
};

struct _SPReprActionChgOrder {
	SPRepr *child;
	SPRepr *oldref, *newref;
};

struct _SPReprAction {
	SPReprAction *next;
	SPReprActionType type;
	SPRepr *repr;
	int serial;
	union {
		SPReprActionAdd add;
		SPReprActionDel del;
		SPReprActionChgAttr chgattr;
		SPReprActionChgContent chgcontent;
		SPReprActionChgOrder chgorder;
	} act;
};

void sp_repr_begin_transaction (SPReprDoc *doc);
void sp_repr_rollback (SPReprDoc *doc);
void sp_repr_commit (SPReprDoc *doc);
SPReprAction *sp_repr_commit_undoable (SPReprDoc *doc);

void sp_repr_undo_log (SPReprDoc *doc, SPReprAction *log);
void sp_repr_replay_log (SPReprDoc *doc, SPReprAction *log);
SPReprAction *sp_repr_coalesce_log (SPReprAction *a, SPReprAction *b);
void sp_repr_free_log (SPReprAction *log);

SPReprAction *sp_repr_log_add (SPReprAction *log, SPRepr *repr,
                               SPRepr *child, SPRepr *ref);
SPReprAction *sp_repr_log_remove (SPReprAction *log, SPRepr *repr,
                                  SPRepr *child, SPRepr *ref);

/* these two reference oldval directly */
SPReprAction *sp_repr_log_chgattr (SPReprAction *log, SPRepr *repr, int key,
                                   unsigned char *oldval,
                                   const unsigned char *newval);
SPReprAction *sp_repr_log_chgcontent (SPReprAction *log, SPRepr *repr,
                                      unsigned char *oldval,
                                      const unsigned char *newval);

SPReprAction *sp_repr_log_chgorder (SPReprAction *log, SPRepr *repr,
                                    SPRepr *child,
                                    SPRepr *oldref, SPRepr *newref);

#endif
