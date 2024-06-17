//
// Trial License - for use to evaluate programs for possible purchase as
// an end-user only.
//
// File: func1.h
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
#ifndef RTW_HEADER_func1_h_
#define RTW_HEADER_func1_h_
#include "rtwtypes.h"
#include <cstring>

// Class declaration for model func1
class func1 final
{
  // public data and function members
 public:
  // Block signals and states (default storage) for model 'func1'
  struct DW_func1_T {
    boolean_T Delay_DSTATE;            // '<S1>/Delay'
  };

  // model step function
  void step(int32_T arg_In1, boolean_T arg_In2, int32_T *arg_Out1);

  // Copy Constructor
  func1(func1 const&) = delete;

  // Assignment Operator
  func1& operator= (func1 const&) & = delete;

  // Move Constructor
  func1(func1 &&) = delete;

  // Move Assignment Operator
  func1& operator= (func1 &&) = delete;

  // Constructor
  func1();

  // Destructor
  ~func1();

  // private data and function members
 private:
  // Block states
  DW_func1_T func1_DW;
};

//-
//  The generated code includes comments that allow you to trace directly
//  back to the appropriate location in the model.  The basic format
//  is <system>/block_name, where system is the system number (uniquely
//  assigned by Simulink) and block_name is the name of the block.
//
//  Use the MATLAB hilite_system command to trace the generated code back
//  to the model.  For example,
//
//  hilite_system('<S3>')    - opens system 3
//  hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
//
//  Here is the system hierarchy for this model
//
//  '<Root>' : 'func1'
//  '<S1>'   : 'func1/Subsystem'

#endif                                 // RTW_HEADER_func1_h_

//
// File trailer for generated code.
//
// [EOF]
//
