//========================================================================================
// Athena++ astrophysical MHD code
// Copyright(C) 2014 James M. Stone <jmstone@princeton.edu> and other code contributors
// Licensed under the 3-clause BSD License, see LICENSE file for details
//========================================================================================
//! \file z4c_awa_tests.cpp
//  \brief Initial conditions for Apples with Apples Test

#include <cassert> // assert
#include <iostream>

// Athena++ headers
#include "../athena.hpp"
#include "../athena_arrays.hpp"
#include "../parameter_input.hpp"
#include "../coordinates/coordinates.hpp"
#include "../mesh/mesh.hpp"
#include "../z4c/z4c.hpp"

using namespace std;

//========================================================================================
//! \fn void MeshBlock::ProblemGenerator(ParameterInput *pin)
//  \brief Sets the initial conditions.
//========================================================================================

void MeshBlock::ProblemGenerator(ParameterInput *pin) {
  string test = pin->GetOrAddString("problem", "test", "Minkowski");

  if(test == "robust_stability") {
    pz4c->ADMRobustStability(pz4c->storage.adm);
    pz4c->GaugeRobStab(pz4c->storage.u);
  } else if(test == "linear_wave1") {
    pz4c->ADMLinearWave1(pz4c->storage.adm);
    pz4c->GaugeGeodesic(pz4c->storage.u);
  } else if(test == "linear_wave1_Gaussian") {
    pz4c->ADMLinearWave1Gaussian(pz4c->storage.adm);
    pz4c->GaugeGeodesic(pz4c->storage.u);
  } else if(test == "linear_wave2") {
    pz4c->ADMLinearWave2(pz4c->storage.adm);
    pz4c->GaugeGeodesic(pz4c->storage.u);
  } else if(test == "simple_gauge_wave") {
    pz4c->ADMMinkowski(pz4c->storage.adm);
    pz4c->GaugeSimpleGaugeWave(pz4c->storage.u);
  } else if(test == "gauge_wave1") {
    pz4c->ADMGaugeWave1(pz4c->storage.adm);
    pz4c->GaugeGaugeWave1(pz4c->storage.u);
  } else if(test == "gauge_wave1_shifted") {
    pz4c->ADMGaugeWave1_shifted(pz4c->storage.adm);
    pz4c->GaugeGaugeWave1_shifted(pz4c->storage.u);
  } else if(test == "gauge_wave2") {
    pz4c->ADMGaugeWave2(pz4c->storage.adm);
    pz4c->GaugeGaugeWave2(pz4c->storage.u);
#ifdef GSL
  } else if (test == "polarised_Gowdy") {
    pz4c->ADMPolarisedGowdy(pz4c->storage.adm);
    pz4c->GaugePolarisedGowdy(pz4c->storage.u);
#endif // GSL
  } else {
    pz4c->ADMMinkowski(pz4c->storage.adm);
    pz4c->GaugeGeodesic(pz4c->storage.u);
  }

  // Constructing Z4c vars from ADM ones
  pz4c->ADMToZ4c(pz4c->storage.adm, pz4c->storage.u);

  return;
}
