/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab1.h>
#include <math.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);

void setschedclass(int sched_class){
	scheduler = sched_class;
}

int getschedclass(){
	return scheduler;
}

int getrandomprocess(){
	int current;
	int random = (int) expdev(LAMBDA);
	
	current = q[rdyhead].qnext;

	// If no ready process exists return null process
	if(current == rdytail){
		return 0;
	}

	// Find process with priority greater than random number
	while(q[current].qkey < random && current!=rdytail){
		current = q[current].qnext;
	}

	// Find the last element with chosen priority
	while(current != rdytail && q[current].qkey == q[q[current].qnext].qkey){
		current = q[current].qnext;
	}

	// If random number is greater than all process find process with highest priority
	if(current == rdytail){
		current = q[rdytail].qprev;
	}
	
	return current;
}

void newepoch(){
	int i;
	// Iterate through proc tables
	for(i = 0; i<NPROC; i++){
		// If process is assigned
		if(proctab[i].pstate!=PRFREE){
			epochtab[i].priority = proctab[i].pprio;
			epochtab[i].goodness = epochtab[i].quantum + proctab[i].pprio;
			epochtab[i].quantum = (int)(0.5 * epochtab[i].quantum) + proctab[i].pprio;
		}
	}
}

int getnextprocess(){
	int nextpid = -1;
	int maxgoodness = 0;
	int current = q[rdyhead].qnext;
	
	// If no ready process exists return null process
	if(current == rdytail){
		return 0;
	}

	// Iterate through ready queue
	while(current!=rdytail){
		// Find process with max goodness
		if(epochtab[current].quantum > 0 && epochtab[current].goodness >= maxgoodness){
			// If multiple processes exist with same priority pick the one with lowest time stamp(Round Robin)
			if(nextpid != -1 && epochtab[current].goodness == maxgoodness){
				if(epochtab[nextpid].timestamp > epochtab[current].timestamp){
					nextpid = current;
					maxgoodness = epochtab[current].goodness;		
				}
			}
			else{
				nextpid = current;
				maxgoodness = epochtab[current].goodness;
			}
		}
		current = q[current].qnext;
	}
	
	// If no process find return 0
	if(nextpid == -1)
		return 0;
	return nextpid;
}
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */

int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	int nextpid;
	
	if(scheduler == EXPDISTSCHED){
		// Set optr pointer
		optr = &proctab[currpid];
		
		// Push the current process back into contention
		if(optr->pstate == PRCURR){
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}
		
		nextpid = getrandomprocess(); // Get next process id
		dequeue(nextpid);             // Deque the process
		
		nptr = &proctab[nextpid];	  // Set nptr to proc table entry
		nptr->pstate = PRCURR;        // Update State
		currpid = nextpid;			  // Update currpid
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
		
		// No context switch required if same process is chosen
		if(nptr == optr){
			return(OK);
		}
	}
	else if(scheduler == LINUXSCHED){
		// Pointer to old process
		optr = &proctab[currpid];
		
		// Set preempt to 0 if less than 0
		if(preempt<0){	
			preempt=0;
		}

		// Update quantum
		epochtab[currpid].quantum = preempt;
		// Update goodness
		if(epochtab[currpid].quantum == 0){
			epochtab[currpid].goodness = 0;
		}
		else{
			epochtab[currpid].goodness = epochtab[currpid].quantum + epochtab[currpid].priority;
		}

		// Push the current process back into contention
		if(optr->pstate == PRCURR){
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}
		
		// Get the process with highest goodness 
		nextpid = getnextprocess();

		// If no process found start new epoch
		if(nextpid == 0){
			newepoch();
			// Get new pid after starting an epoch
			nextpid = getnextprocess();
		}

		dequeue(nextpid);             // Deque the process
		nptr = &proctab[nextpid];	  // Set nptr to proc table entry
		nptr->pstate = PRCURR;        // Update State
		currpid = nextpid;			  // Update currpid
		preempt = epochtab[currpid].quantum; 
		
		// No context switch required if same process is chosen
		if(nptr == optr){
			return(OK);
		}
	}
	else{
		/* no switch needed if current process priority higher than next*/
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
		
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/

		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	}
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
