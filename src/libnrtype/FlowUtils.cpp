/*
 *  FlowUtils.cpp
 */

#include "FlowUtils.h"
#include "FlowEater.h"

flow_brk::flow_brk(flow_maker* i_dad,int i_no,box_sol &i_box)
{
	dad=i_dad;
	no=i_no;
	prev_box_brk=prev_line_brk=-1;
	next=-1;
	used_box=i_box;
	elem_no=elem_pos=0;	
	u_st_no=u_st_pos=u_en_no=u_en_pos=0;
	sol_box.width=0;
	sol_box.nb_letter=0;
	para_end=false;
	delta_score=0;
}
flow_brk::~flow_brk(void)
{
}
double          flow_brk::Score(int root_brk)
{
	double  sum=0;
	int     c_no=no;
	while ( c_no > root_brk ) {
		sum+=dad->brks[c_no].delta_score;
		c_no=dad->brks[c_no].prev_box_brk;
	}
	return sum;
}
double          flow_brk::Length(int root_brk)
{
	double  sum=0;
	int     c_no=no;
	while ( c_no > root_brk ) {
		sum+=dad->brks[c_no].used_box.x_end-dad->brks[c_no].used_box.x_start;
		c_no=dad->brks[c_no].prev_box_brk;
	}
	return sum;
}
void						flow_brk::FillBox(box_sol &redux,bool flow_rtl)
{
	redux=used_box;
	if ( flow_rtl ) {
		redux.x_end=redux.x_start;
	} else {
		redux.x_start=redux.x_end;
	}
}
void            flow_brk::LinkAfter(int p_brk,bool sameLine)
{
	if ( p_brk < 0 ) return;
	prev_box_brk=p_brk;
	if ( sameLine ) {
		prev_line_brk=dad->brks[p_brk].prev_line_brk;
	} else {
		prev_line_brk=p_brk;
	}
}
void            flow_brk::SetEnd(int i_en_no,int i_en_pos)
{
	elem_no=i_en_no;
	elem_pos=i_en_pos;
	SetContent(elem_no,elem_pos,elem_no,elem_pos);
}
void            flow_brk::SetContent(int i_st_no,int i_st_pos,int i_en_no,int i_en_pos)
{
	u_st_no=i_st_no;
	u_st_pos=i_st_pos;
	u_en_no=i_en_no;
	u_en_pos=i_en_pos;
}

/*
 *
 */

flow_tasks::flow_tasks(flow_maker* i_dad)
{
	nbTask=maxTask=0;
	tasks=NULL;
	last_id=0;
	min_pending=0;
}
flow_tasks::~flow_tasks(void)
{
	if ( tasks ) free(tasks);
	nbTask=maxTask=0;
	tasks=NULL;
}

void              flow_tasks::Thaw(int t_id)
{
	for (int i=0;i<nbTask;i++) {
		if ( tasks[i].t_id == t_id && tasks[i].state == task_freezed ) {
			tasks[i].state=task_pending;
			if ( i < min_pending ) min_pending=i;
			break;
		}
	}
}
int 							flow_tasks::Push(int i_brk,flow_requirement &i_req,bool freezed,bool force)
{
	for (int i=0;i<nbTask;i++) {
		if ( tasks[i].brk == i_brk && i_req.Equals(tasks[i].req) ) {
			if ( force == true && tasks[i].state != task_done ) {
				// already here, already waiting
				// nota: if done already, it is repooled
				return -1;
			}
			if ( force == false ) {
				// already here, already waiting or done
				return -1;
			}
		}
	}
	if ( nbTask >= maxTask ) {
		maxTask=2*nbTask+1;
		tasks=(one_task*)realloc(tasks,maxTask*sizeof(one_task));
	}
	tasks[nbTask].t_id=last_id++;
	tasks[nbTask].brk=i_brk;
	tasks[nbTask].req=i_req;
	tasks[nbTask].state=(freezed)?task_freezed:task_pending;
	nbTask++;
	return tasks[nbTask-1].t_id;
}
bool							flow_tasks::Pop(int &i_brk,flow_requirement &i_req,int &t_id)
{
	for (int i=min_pending;i<nbTask;i++) {
		if ( tasks[i].state == task_pending ) {
			tasks[i].state=task_done;
			i_brk=tasks[i].brk;
			i_req=tasks[i].req;
			t_id=tasks[i].t_id;
			if ( i > min_pending ) min_pending=i;
			return true;
		}
	}
	return false;
}

