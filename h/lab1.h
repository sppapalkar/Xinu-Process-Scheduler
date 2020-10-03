#ifndef _LAB1_H_
#define _LAB1_H_

#define EXPDISTSCHED 1
#define LINUXSCHED 2 
#define LAMBDA 0.1

extern int scheduler;
typedef struct epoch_proc{
    int priority;
    int goodness;
    int quantum;
    long timestamp;
}epoch_proc;
extern epoch_proc epochtab[];

void setschedclass (int sched_class);
int getschedclass();

#endif