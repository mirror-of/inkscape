#ifndef SEEN_INKSCAPE_XML_SP_REPR_ACTION_FNS_H
#define SEEN_INKSCAPE_XML_SP_REPR_ACTION_FNS_H

class SPReprDoc;
class SPReprAction;

void sp_repr_begin_transaction (SPReprDoc *doc);
void sp_repr_rollback (SPReprDoc *doc);
void sp_repr_commit (SPReprDoc *doc);
SPReprAction *sp_repr_commit_undoable (SPReprDoc *doc);

void sp_repr_undo_log (SPReprAction *log);
void sp_repr_replay_log (SPReprAction *log);
SPReprAction *sp_repr_coalesce_log (SPReprAction *a, SPReprAction *b);
void sp_repr_free_log (SPReprAction *log);

#endif
