/*
 *  FlowUtils.h
 */

#ifndef my_flow_utils
#define my_flow_utils

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "FlowDefs.h"

class flow_maker;
class flow_dest;

typedef struct flow_requirement {
	bool               rtl;
	double             ascent,descent,leading;
	bool               next_line;
	int                min_elem_no,min_elem_pos; // la boite min_elem_pos doit etre incluse pour passer le cap
	int                min_brk,min_fallback;
	double             score_malus;       ///french: malus: surcharge (car insurance)
	
	bool               Equals(flow_requirement& a) const {
		if ( fabs(ascent-a.ascent) > 0.01 ) return false;
		if ( fabs(descent-a.descent) > 0.01 ) return false;
		if ( fabs(leading-a.leading) > 0.01 ) return false;
		if ( next_line != a.next_line ) return false;
		if ( min_elem_no < 0 && a.min_elem_no >= 0 ) return false;
		if ( min_elem_no >= 0 && a.min_elem_no < 0 ) return false;
		if ( min_elem_no >= 0 && min_elem_pos != a.min_elem_pos ) return false;
		return true;
	};
	flow_requirement() : rtl(false), ascent(0.0), descent(0.0),
			     leading(0.0), next_line(false),
			     min_elem_no(0), min_elem_pos(0), min_brk(0),
			     min_fallback(0), score_malus(0.0) {};
} flow_requirement;

class flow_brk {
public:
	flow_maker*     dad;
	int             no;
	
	bool            rtl;
	box_sol         used_box;
	box_sizes       sol_box;
	int             elem_no,elem_pos;
	bool            para_end;

	int             u_st_no,u_st_pos,u_en_no,u_en_pos;
	
	int             prev_box_brk,prev_line_brk;
	int             next;
	
	double          delta_score;
	
	flow_brk(flow_maker* i_dad,int i_no,box_sol &i_box);
	~flow_brk(void);

	void						FillBox(box_sol &redux,bool flow_rtl);
	void            LinkAfter(int p_brk,bool sameLine);
	void            SetEnd(int i_en_no,int i_en_pos);
	void            SetContent(int i_st_no,int i_st_pos,int i_en_no,int i_en_pos);
	
	double          Score(int root_brk);
	double          Length(int root_brk);
};

/** \brief internal libnrtype class

Internal libnrtype class. Used to store a list of the jobs left to do
by the text flowing algorithms in flow_maker. It stores an index into
the flow_maker::brks array and a flow_requirement, whatever that is.

It is basically a FIFO but with a couple of tricks up its sleeves:
-# the ability to 'freeze' a new entry so it won't be returned just yet.
-# new entries identical to already-completed ones can be suppressed.
*/
class flow_tasks {
public:
	flow_tasks(flow_maker* i_dad);
	~flow_tasks(void);

	/** Change the task with the given ID to state task_pending. If the
    task wasn't task_freezed then nothing is done. */
    void              Thaw(int t_id);

    /** Adds a new task to the list.
      \param i_brk   an index into the flow_maker::brks array, although it
                     is treated as opaque by this class.
      \param i_req   the flow_requirement to store in the new task
      \param freezed if true then the new entry is created in state
                     task_freezed, instead of task_pending
      \param force   when false, if the new entry has the same brk and
                     flow_requirement as an existing entry, even if that
                     entry has been marked task_done, it will not be re-added.
                     If true then the new entry is added even if there's an
                     existing entry marked task_done. Pending or frozen
                     entries are never duplicated.
      \return  an opaque id identifying the task
    */
	int 							Push(int i_brk,flow_requirement const &i_req,bool freezed=false,bool force=true);

    /** Retrieves the next pending task from the list, then marks it task_done.
    Returns false when there are no more tasks available.
    */
	bool							Pop(int &i_brk,flow_requirement &i_req,int &t_id);
private:
    enum {
	    task_pending      = 0,
	    task_done         = 1,
	    task_freezed      = 2
    };

	typedef struct one_task {
		int               brk;
		flow_requirement  req;
		int               state;    ///a value from the above enum
		int               t_id;     ///unique identifier to identify this item to callers
	} one_task;
	int               nbTask,maxTask;
	one_task*         tasks;
	
    /** the index of the first item in #tasks with state==task_pending.
    An optimisation to avoid having to scan the whole list all the time. */
	int               min_pending;

	int               last_id;   ///counter to generate unique identifiers for #tasks
	flow_maker*       dad;
};

#endif
