/* insert.c  -  insert */

#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <lab1.h>
/*------------------------------------------------------------------------
 * insert.c  --  insert an process into a q list in key order
 *------------------------------------------------------------------------
 */
extern long ctr1000;
int insert(int proc, int head, int key)
{
	int	next;			/* runs through list		*/
	int	prev;

	next = q[head].qnext;
	while (q[next].qkey < key)	/* tail has maxint as key	*/
		next = q[next].qnext;
	q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey  = key;
	q[prev].qnext = proc;
	q[next].qprev = proc;

	// Insert time stamp to implement round robin
	if(getschedclass() == LINUXSCHED){
		epochtab[proc].timestamp = ctr1000;
	}

	return(OK);
}
