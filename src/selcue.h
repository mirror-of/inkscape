#ifndef __SP_SELCUE_H__
#define __SP_SELCUE_H__

/*
 * Helper object for showing selected items
 *
 * Authors:
 *   bulia byak <bulia@users.sf.net>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

enum {
        SP_SELCUE_NONE,
        SP_SELCUE_MARK,
        SP_SELCUE_BBOX
};

struct SPSelCue {
	SPDesktop *desktop;
	SPSelection *selection;
	SigC::Connection sel_changed_connection;
	SigC::Connection sel_modified_connection;
	GSList *item_bboxes;
};

void sp_sel_cue_init(SPSelCue *selcue, SPDesktop *desktop);
void sp_sel_cue_shutdown(SPSelCue *selcue);
void sp_sel_cue_update_item_bboxes (SPSelCue * selcue);

#endif
