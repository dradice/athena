//========================================================================================
// Athena++ astrophysical MHD code
// Copyright(C) 2014 James M. Stone <jmstone@princeton.edu> and other code contributors
// Licensed under the 3-clause BSD License, see LICENSE file for details
//========================================================================================
//! \file task_list.cpp
//  \brief functions for TaskList base class


// C headers

// C++ headers
//#include <vector>
// used to be needed for vector of pointers in DoTaskListOneStage()

// Athena++ headers
#include "../athena.hpp"
#include "../globals.hpp"
#include "../mesh/mesh.hpp"
#include "task_list.hpp"

#ifdef OPENMP_PARALLEL
#include <omp.h>
#endif

//----------------------------------------------------------------------------------------
// TaskList constructor

TaskList::TaskList(Mesh *pm) {
  pmy_mesh_=pm;
  ntasks = 0;
  nstages = 0;
}

// destructor

TaskList::~TaskList() {
}

//----------------------------------------------------------------------------------------
//! \fn enum TaskListStatus TaskList::DoAllAvailableTasks
//  \brief do all tasks that can be done (are not waiting for a dependency to be
//  cleared) in this TaskList, return status.

enum TaskListStatus TaskList::DoAllAvailableTasks(MeshBlock *pmb, int stage,
                                                  TaskState &ts) {
  int skip=0;
  enum TaskStatus ret;
  if (ts.num_tasks_left == 0) return TL_NOTHING_TO_DO;

  for (int i=ts.indx_first_task; i<ntasks; i++) {
    Task &taski=task_list_[i];
    if ((taski.task_id & ts.finished_tasks) == 0ULL) { // task not done
      // check if dependency clear
      if (((taski.dependency & ts.finished_tasks) == taski.dependency)) {
        ret=(this->*task_list_[i].TaskFunc)(pmb, stage);
        if (ret!=TASK_FAIL) { // success
          ts.num_tasks_left--;
          ts.finished_tasks |= taski.task_id;
          if (skip==0) ts.indx_first_task++;
          if (ts.num_tasks_left == 0) return TL_COMPLETE;
          if (ret==TASK_NEXT) continue;
          return TL_RUNNING;
        }
      }
      skip++; // increment number of tasks processed

    } else if (skip==0) { // this task is already done AND it is at the top of the list
      ts.indx_first_task++;
    }
  }
  return TL_STUCK; // there are still tasks to do but nothing can be done now
}

//----------------------------------------------------------------------------------------
//! \fn void TaskList::DoTaskListOneStage(Mesh *pmesh, int stage)
//  \brief completes all tasks in this list, will not return until all are tasks done

void TaskList::DoTaskListOneStage(Mesh *pmesh, int stage) {
  int nthreads = pmesh->GetNumMeshThreads();
  int nmb = pmesh->GetNumMeshBlocksThisRank(Globals::my_rank);

  // construct the MeshBlock array on this process
  MeshBlock **pmb_array = new MeshBlock*[nmb];
  MeshBlock *pmb = pmesh->pblock;
  for (int n=0; n < nmb; ++n) {
    pmb_array[n] = pmb;
    pmb = pmb->next;
  }

  // clear the task states, startup the integrator and initialize mpi calls
  #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
  for (int i=0; i<nmb; ++i) {
    pmb_array[i]->tasks.Reset(ntasks);
    StartupTaskList(pmb_array[i], stage);
  }

  int nmb_left = nmb;
  // cycle through all MeshBlocks and perform all tasks possible
  while (nmb_left > 0) {
    // KNOWN ISSUE: Workaround for unknown OpenMP race condition. See #183 on GitHub.
    #pragma omp parallel for reduction(- : nmb_left) \
                         num_threads(nthreads) schedule(dynamic,1)
    for (int i=0; i<nmb; ++i) {
      if (DoAllAvailableTasks(pmb_array[i],stage,pmb_array[i]->tasks) == TL_COMPLETE) {
        nmb_left--;
      }
    }
  }
  delete [] pmb_array;
  return;
}
