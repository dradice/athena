//======================================================================================
// Athena++ astrophysical MHD code
// Copyright (C) 2014 James M. Stone  <jmstone@princeton.edu>
//
// This program is free software: you can redistribute and/or modify it under the terms
// of the GNU General Public License (GPL) as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
// PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of GNU GPL in the file LICENSE included in the code
// distribution.  If not see <http://www.gnu.org/licenses/>.
//======================================================================================
//! \file radiation_integrator.cpp
//  \brief derived class for radiation integrator task list.
//======================================================================================

// C/C++ headers
#include <iostream>   // endl
#include <sstream>    // sstream
#include <stdexcept>  // runtime_error
#include <string>     // c_str()

// Athena++ classes headers
#include "../athena.hpp"
#include "../mesh/mesh.hpp"
#include "../radiation/integrators/rad_integrators.hpp"

// this class header
#include "task_list.hpp"

//--------------------------------------------------------------------------------------
//  RadiationIntegratorTaskList constructor
RadiationIntegratorTaskList::RadiationIntegratorTaskList(ParameterInput *pin, Mesh *pm)
  : TaskList(pm)
{
  nsub_steps = 1;
  integrator = pin->GetString("radiation","integrator");
  // Now assemble list of tasks for each step of chemistry integrator
  {using namespace RadiationIntegratorTaskNames;
    if (integrator == "jeans") {
      AddRadiationIntegratorTask(INT_LOC_JEANS,NONE);
    } else if (integrator == "six_ray") {
      //add six ray
    } else if (integrator == "const") {
      //do nothing, radiation field constant, remain initial value
      AddRadiationIntegratorTask(INT_CONST,NONE);
    } else if (integrator == "six_ray") {
    } else {
      std::stringstream msg;
      msg << "### FATAL ERROR in Radiation task list" << std::endl
        << "integrator=" << integrator << " not valid radiation integrator, " << std::endl
        << "choose from {jeans, six_ray, const}" << std::endl;
      throw std::runtime_error(msg.str().c_str());
    }
  } // end of using namespace block
}

//--------------------------------------------------------------------------------------
//! \fn
//  \brief Sets id and dependency for "ntask" member of task_list_ array, then iterates
//  value of ntask.  
void RadiationIntegratorTaskList::AddRadiationIntegratorTask(uint64_t id, uint64_t dep)
{
  task_list_[ntasks].task_id=id;
  task_list_[ntasks].dependency=dep;
  using namespace RadiationIntegratorTaskNames;
  switch((id)) {
    case (INT_LOC_JEANS):
      task_list_[ntasks].TaskFunc=
        static_cast<enum TaskStatus (TaskList::*)(MeshBlock*,int)>
        (&RadiationIntegratorTaskList::LocalIntegratorJeans);
      break;
    case (INT_CONST):
      task_list_[ntasks].TaskFunc=
        static_cast<enum TaskStatus (TaskList::*)(MeshBlock*,int)>
        (&RadiationIntegratorTaskList::ConstRadiation);
      break;
    //add six ray here
    default:
      std::stringstream msg;
      msg << "### FATAL ERROR in Add Radiation Task" << std::endl
          << "Invalid Task "<< id << " is specified" << std::endl;
      throw std::runtime_error(msg.str().c_str());
  }
  ntasks++;
  return;
}

//--------------------------------------------------------------------------------------
// Functions to integrate Radiation
enum TaskStatus RadiationIntegratorTaskList::LocalIntegratorJeans(MeshBlock *pmb,
                                                                 int step)
{
#ifdef INCLUDE_CHEMISTRY
  pmb->prad->pradintegrator->UpdateRadJeans();
#endif
  return TASK_SUCCESS;
}

enum TaskStatus RadiationIntegratorTaskList::ConstRadiation(MeshBlock *pmb, int step)
{
  return TASK_SUCCESS;
}
//add six ray here
