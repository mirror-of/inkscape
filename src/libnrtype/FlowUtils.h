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
	double             score_malus;
	
	bool               Equals(flow_requirement& a) {
		if ( fabs(ascent-a.ascent) > 0.01 ) return false;
		if ( fabs(descent-a.descent) > 0.01 ) return false;
		if ( fabs(leading-a.leading) > 0.01 ) return false;
		if ( next_line != a.next_line ) return false;
		if ( min_elem_no < 0 && a.min_elem_no >= 0 ) return false;
		if ( min_elem_no >= 0 && a.min_elem_no < 0 ) return false;
		if ( min_elem_no >= 0 && min_elem_pos != a.min_elem_pos ) return false;
		return true;
	};
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

enum {
	task_pending      = 0,
	task_done         = 1,
	task_freezed      = 2
};

class flow_tasks {
public:
	flow_maker*       dad;
	typedef struct one_task {
		int               brk;
		flow_requirement  req;
		int               state;
		int               t_id;
	} one_task;
	int               last_id;
	int               nbTask,maxTask;
	one_task*         tasks;
	int               min_pending;
	
	flow_tasks(flow_maker* i_dad);
	~flow_tasks(void);

	void              Thaw(int t_id);
	int 							Push(int i_brk,flow_requirement &i_req,bool freezed=false,bool force=true);
	bool							Pop(int &i_brk,flow_requirement &i_req,int &t_id);
};

#endif
