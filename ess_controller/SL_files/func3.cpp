//
// Trial License - for use to evaluate programs for possible purchase as
// an end-user only.
//
// File: func3.cpp
//
// Code generated for Simulink model 'func3'.
//
// Model version                  : 1.8
// Simulink Coder version         : 9.8 (R2022b) 13-May-2022
// C/C++ source code generated on : Tue Aug 22 16:19:33 2023
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Windows64)
// Code generation objectives:
//    1. Execution efficiency
//    2. RAM efficiency
// Validation result: Not run
//
#include "func3.h"
#include "rtwtypes.h"

// Output and update for referenced model: 'func3'
void func3::step(int32_T arg_In1, int32_T *arg_Out1)
{
  // Switch: '<Root>/Switch' incorporates:
  //   Constant: '<Root>/Constant'
  //   Constant: '<Root>/Constant1'
  //   Constant: '<Root>/Constant2'
  //   Logic: '<Root>/OR'
  //   RelationalOperator: '<Root>/GreaterThan'
  //   RelationalOperator: '<Root>/LessThanOrEqual'

  if ((arg_In1 >= 100) || (arg_In1 <= -100)) {
    *arg_Out1 = 0;
  } else {
    *arg_Out1 = arg_In1;
  }

  // End of Switch: '<Root>/Switch'
}

// Constructor
func3::func3()
{
  // Currently there is no constructor body generated.
}

// Destructor
func3::~func3()
{
  // Currently there is no destructor body generated.
}

//
// File trailer for generated code.
//
// [EOF]
//
