#ifndef __BS_PROFILER_H__
#define __BS_PROFILER_H__

#include <time.h>

#include "bs_types.h"

typedef enum bs_profiler_task_type_e
{
  BS_PROFILER_TASK_DRAW = 0,
  BS_PROFILER_TASK_SIMULATION,
  BS_PROFILER_TASK_OCTREE,
  BS_PROFILER_TASK_OCTREE_TEST_BOX,
  BS_PROFILER_TASK_OCTREE_TEST_LINE_SEGMENT,
  BS_PROFILER_TASK_BLOB_COLLECTIVE,
  BS_PROFILER_TASK_END_ENUM
} bs_profiler_task_type_t; 

typedef struct bs_profiler_task_st
{
  char *pName; 
  clock_t totalTime; 
  clock_t startTime; 
} bs_profiler_task_t; 

typedef struct bs_profiler_st 
{
  bs_profiler_task_t pTasks[BS_PROFILER_TASK_END_ENUM]; 
  clock_t startTime;   
} bs_profiler_t; 

void 
bs_profiler_init(); 

void 
bs_profiler_start(bs_profiler_task_type_t taskType);  
                  
void 
bs_profiler_stop(bs_profiler_task_type_t taskType);  

void 
bs_profiler_report(); 

#endif
