//
// Trial License - for use to evaluate programs for possible purchase as
// an end-user only.
//
// File: func2.h
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
#ifndef RTW_HEADER_func2_h_
#define RTW_HEADER_func2_h_
#include "rtwtypes.h"
#include <cstring>

// Class declaration for model func2
class func2 final
{
  // public data and function members
 public:
  // model step function
  void step(int32_T arg_In1, int32_T *arg_Out1);

  // Copy Constructor
  func2(func2 const&) = delete;

  // Assignment Operator
  func2& operator= (func2 const&) & = delete;

  // Move Constructor
  func2(func2 &&) = delete;

  // Move Assignment Operator
  func2& operator= (func2 &&) = delete;

  // Constructor
  func2();

  // Destructor
  ~func2();
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
//  '<Root>' : 'func2'

#endif                                 // RTW_HEADER_func2_h_

//
// File trailer for generated code.
//
// [EOF]
//
