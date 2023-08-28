//
// Trial License - for use to evaluate programs for possible purchase as
// an end-user only.
//
// File: func2.cpp
//
// Code generated for Simulink model 'func2'.
//
// Model version                  : 1.6
// Simulink Coder version         : 9.8 (R2022b) 13-May-2022
// C/C++ source code generated on : Tue Aug 22 16:19:30 2023
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Windows64)
// Code generation objectives:
//    1. Execution efficiency
//    2. RAM efficiency
// Validation result: Not run
//
#include "func2.h"
#include "rtwtypes.h"

// Output and update for referenced model: 'func2'
void func2::step(int32_T arg_In1, int32_T *arg_Out1)
{
  // Gain: '<Root>/Multiply'
  *arg_Out1 = arg_In1 << 1;
}

// Constructor
func2::func2()
{
  // Currently there is no constructor body generated.
}

// Destructor
func2::~func2()
{
  // Currently there is no destructor body generated.
}

//
// File trailer for generated code.
//
// [EOF]
//
