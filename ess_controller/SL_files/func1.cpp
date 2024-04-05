//
// Trial License - for use to evaluate programs for possible purchase as
// an end-user only.
//
// File: func1.cpp
//
// Code generated for Simulink model 'func1'.
//
// Model version                  : 1.20
// Simulink Coder version         : 9.8 (R2022b) 13-May-2022
// C/C++ source code generated on : Thu Sep 14 13:20:07 2023
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Windows64)
// Code generation objectives:
//    1. Execution efficiency
//    2. RAM efficiency
// Validation result: Not run
//
#include "func1.h"
#include "rtwtypes.h"

// Output and update for referenced model: 'func1'
void func1::step(int32_T arg_In1, boolean_T arg_In2, int32_T* arg_Out1)
{
    int32_T tmp;
    int32_T tmp_0;

    // Switch: '<S1>/Switch1' incorporates:
    //   Constant: '<S1>/Constant2'
    //   Delay: '<S1>/Delay'
    //   RelationalOperator: '<S1>/NotEqual'

    if (arg_In2 != func1_DW.Delay_DSTATE)
    {
        tmp = 0;
    }
    else
    {
        tmp = arg_In1;
    }

    // Switch: '<S1>/Switch' incorporates:
    //   Constant: '<S1>/Constant'
    //   Constant: '<S1>/Constant1'

    if (arg_In2 != 0u)
    {
        tmp_0 = 1;
    }
    else
    {
        tmp_0 = -1;
    }

    // Sum: '<S1>/Sum' incorporates:
    //   Switch: '<S1>/Switch'
    //   Switch: '<S1>/Switch1'

    *arg_Out1 = tmp + tmp_0;

    // Update for Delay: '<S1>/Delay'
    func1_DW.Delay_DSTATE = arg_In2;
}

// Constructor
func1::func1() : func1_DW()
{
    // Currently there is no constructor body generated.
}

// Destructor
func1::~func1()
{
    // Currently there is no destructor body generated.
}

//
// File trailer for generated code.
//
// [EOF]
//
