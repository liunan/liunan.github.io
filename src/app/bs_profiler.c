#include <stdio.h>

#include "bs_profiler.h"

bs_profiler_t profiler; 

#define BS_PROFILER_INIT_TASK(PROFILER, TASK) \
  (PROFILER).pTasks[TASK].totalTime = 0; \
  (PROFILER).pTasks[TASK].pName = #TASK; 

void 
bs_profiler_init()
{
  BS_PROFILER_INIT_TASK(profiler, BS_PROFILER_TASK_DRAW); 
  BS_PROFILER_INIT_TASK(profiler, BS_PROFILER_TASK_SIMULATION);
  BS_PROFILER_INIT_TASK(profiler, BS_PROFILER_TASK_OCTREE);
  BS_PROFILER_INIT_TASK(profiler, BS_PROFILER_TASK_OCTREE_TEST_BOX);
  BS_PROFILER_INIT_TASK(profiler, BS_PROFILER_TASK_OCTREE_TEST_LINE_SEGMENT);
  BS_PROFILER_INIT_TASK(profiler, BS_PROFILER_TASK_BLOB_COLLECTIVE); 

  profiler.startTime = clock();   
}

void 
bs_profiler_start(bs_profiler_task_type_t taskType)
{
  profiler.pTasks[taskType].startTime = clock(); 
}
                  
void 
bs_profiler_stop(bs_profiler_task_type_t taskType)
{
  clock_t stopTime; 
  
  stopTime = clock(); 
  
  profiler.pTasks[taskType].totalTime += stopTime - profiler.pTasks[taskType].startTime; 
}

void 
bs_profiler_report()
{
  bs_int32_t i;
  bs_float_t totalTime;

  totalTime = (bs_float_t)(clock() - profiler.startTime) / (bs_float_t)CLOCKS_PER_SEC; 
    
  printf("--- PROFILER ----------------------------------------------------\n"); 
  
  printf("All Tasks: %.2fs\n", totalTime); 
  for(i = 0; i < BS_PROFILER_TASK_END_ENUM; i++)
  {
    bs_float_t usedTime, percentOfTotal; 
    
    usedTime = (bs_float_t)profiler.pTasks[i].totalTime / (bs_float_t)CLOCKS_PER_SEC; 
    percentOfTotal = usedTime / totalTime; 
    percentOfTotal *= 100.0f; 
      
    printf("%.2fs, %.2f%%, %s\n", usedTime, percentOfTotal, profiler.pTasks[i].pName); 
  }

  printf("-----------------------------------------------------------------\n"); 

  for(i = 0; i < BS_PROFILER_TASK_END_ENUM; i++)
  {
    profiler.pTasks[i].totalTime = 0; 
  }
  profiler.startTime = clock(); 
}
