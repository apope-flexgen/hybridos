//
// File: LowLevelController.cpp
//
// Code generated for Simulink model 'LowLevelController'.
//
// Model version                  : 1.1
// Simulink Coder version         : 9.8 (R2022b) 13-May-2022
// C/C++ source code generated on : Wed Jan 10 11:22:26 2024
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Windows64)
// Code generation objectives:
//    1. Execution efficiency
//    2. RAM efficiency
// Validation result: Not run
//
#include "LowLevelController.h"

// Model step function
void LowLevelController::step()
{
  // Outport: '<Root>/LowLevelOut' incorporates:
  //   Inport: '<Root>/Common_IN'
  //   Inport: '<Root>/Unique_IN'
  //   Sum: '<Root>/Sum'

  rtY.LowLevelOut = rtU.Common_IN + rtU.Unique_IN;
}

// Model initialize function
void LowLevelController::initialize()
{
  // (no initialization code required)
}

// Constructor
LowLevelController::LowLevelController() :
  rtU(),
  rtY(),
  rtM()
{
  // Currently there is no constructor body generated.
}

// Destructor
LowLevelController::~LowLevelController()
{
  // Currently there is no destructor body generated.
}

// Real-Time Model get method
LowLevelController::RT_MODEL * LowLevelController::getRTM()
{
  return (&rtM);
}

//
// File trailer for generated code.
//
// [EOF]
//
