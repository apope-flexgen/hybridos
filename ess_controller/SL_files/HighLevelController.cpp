//
// File: HighLevelController.cpp
//
// Code generated for Simulink model 'HighLevelController'.
//
// Model version                  : 1.2
// Simulink Coder version         : 9.8 (R2022b) 13-May-2022
// C/C++ source code generated on : Thu Jan 11 10:07:35 2024
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Windows64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#include "HighLevelController.h"
#include "rtwtypes.h"

// Model step function
void HighLevelController::step()
{
  int32_T tmp;

  // Switch: '<Root>/Switch' incorporates:
  //   Constant: '<Root>/Constant'
  //   Constant: '<Root>/Constant1'
  //   Inport: '<Root>/In1'

  if (HighLevelController_U.In1) {
    tmp = 1;
  } else {
    tmp = -1;
  }

  // Sum: '<Root>/Sum' incorporates:
  //   Delay: '<Root>/Delay'
  //   Switch: '<Root>/Switch'

  HighLevelController_Y.Out1 += tmp;

  // Abs: '<Root>/Abs'
  if (HighLevelController_Y.Out1 < 0) {
    tmp = -HighLevelController_Y.Out1;
  } else {
    tmp = HighLevelController_Y.Out1;
  }

  // Switch: '<Root>/Switch1' incorporates:
  //   Abs: '<Root>/Abs'
  //   Constant: '<Root>/Constant2'
  //   RelationalOperator: '<Root>/GreaterThan'

  if (tmp >= 100) {
    // Sum: '<Root>/Sum' incorporates:
    //   Constant: '<Root>/Constant3'

    HighLevelController_Y.Out1 = 0;
  }

  // End of Switch: '<Root>/Switch1'
}

// Model initialize function
void HighLevelController::initialize()
{
  // (no initialization code required)
}

// Model terminate function
void HighLevelController::terminate()
{
  // (no terminate code required)
}

// Constructor
HighLevelController::HighLevelController() :
  HighLevelController_U(),
  HighLevelController_Y(),
  HighLevelController_M()
{
  // Currently there is no constructor body generated.
}

// Destructor
HighLevelController::~HighLevelController()
{
  // Currently there is no destructor body generated.
}

// Real-Time Model get method
HighLevelController::RT_MODEL_HighLevelController_T * HighLevelController::
  getRTM()
{
  return (&HighLevelController_M);
}

//
// File trailer for generated code.
//
// [EOF]
//
