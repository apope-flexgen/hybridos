//
// Trial License - for use to evaluate programs for possible purchase as
// an end-user only.
//
// File: Reference.h
//
// Code generated for Simulink model 'Reference'.
//
// Model version                  : 1.14
// Simulink Coder version         : 9.8 (R2022b) 13-May-2022
// C/C++ source code generated on : Wed Aug 23 10:41:45 2023
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Windows64)
// Code generation objectives:
//    1. Execution efficiency
//    2. RAM efficiency
// Validation result: Not run
//
#ifndef RTW_HEADER_Reference_h_
#define RTW_HEADER_Reference_h_
#include "rtwtypes.h"
#include "func1.h"
#include "func2.h"
#include "func3.h"

// Class declaration for model Reference
class Datamaps_test final
{
  // public data and function members
 public:
  // Block signals and states (default storage) for system '<Root>'
  struct DW_Reference_T {
    int32_T Delay_DSTATE;              // '<Root>/Delay'
  };

  // External inputs (root inport signals), for system '<Root>'
  struct ExtUPointer_Reference_T {
    boolean_T DMTestDirection;         // '<Root>/DMTestDirection'
  };

  // External outputs (root outport signals), for system '<Root>'
  struct ExtYPointer_Reference_T {
    real_T DMTestOutDblNoise;          // '<Root>/DMTestOutDblNoise'
    int32_T DMTestOut;                 // '<Root>/DMTestOut'
    int32_T AdderResult;               // '<Root>/AdderResult'
    int32_T GainResult;                // '<Root>/GainResult'
    boolean_T DirectionFeedback;       // '<Root>/DirectionFeedback'
  };

  // Copy Constructor
  Datamaps_test(Datamaps_test const&) = delete;

  // Assignment Operator
  Datamaps_test& operator= (Datamaps_test const&) & = delete;

  // Move Constructor
  Datamaps_test(Datamaps_test &&) = delete;

  // Move Assignment Operator
  Datamaps_test& operator= (Datamaps_test &&) = delete;

  // Constructor
  Datamaps_test(ExtUPointer_Reference_T *Reference_ExtUPointer_g,
                ExtYPointer_Reference_T *Reference_ExtYPointer_i);

  // External inputs (root inport signals)
  ExtUPointer_Reference_T *ExtUPointer_ref_U;

  // External outputs (root outport signals)
  ExtYPointer_Reference_T *ExtYPointer_ref_Y;

  // set method for External inputs (root inport signals)
  void setExternalInputs(const ExtUPointer_Reference_T &ExtUPointer_U);

  // get method for External outputs (root outport signals)
  const ExtYPointer_Reference_T &getExternalOutputs() const;

  // model initialize function
  static void initialize();

  // model step function
  void step();

  // Destructor
  ~Datamaps_test();

  // private data and function members
 private:
  // Block states
  DW_Reference_T Reference_DW;

  // model instance variable for '<Root>/Model'
  func1 ModelMDLOBJ1;

  // model instance variable for '<Root>/Model1'
  func2 Model1MDLOBJ2;

  // model instance variable for '<Root>/Model2'
  func3 Model2MDLOBJ3;
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
//  '<Root>' : 'Reference'

#endif                                 // RTW_HEADER_Reference_h_

//
// File trailer for generated code.
//
// [EOF]
//
