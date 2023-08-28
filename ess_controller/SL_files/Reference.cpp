//
// Trial License - for use to evaluate programs for possible purchase as
// an end-user only.
//
// File: Reference.cpp
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
#include "Reference.h"
#include "rtwtypes.h"
#include "func1.h"
#include "func2.h"
#include "func3.h"

// Model step function
void Datamaps_test::step()
{
  // ModelReference: '<Root>/Model' incorporates:
  //   Delay: '<Root>/Delay'
  //   Inport: '<Root>/DMTestDirection'
  //   Outport: '<Root>/AdderResult'

  ModelMDLOBJ1.step(Reference_DW.Delay_DSTATE,
                    ExtUPointer_ref_U->DMTestDirection,
                    &ExtYPointer_ref_Y->AdderResult);

  // ModelReference: '<Root>/Model1' incorporates:
  //   Outport: '<Root>/AdderResult'
  //   Outport: '<Root>/GainResult'

  Model1MDLOBJ2.step(ExtYPointer_ref_Y->AdderResult,
                     &ExtYPointer_ref_Y->GainResult);

  // ModelReference: '<Root>/Model2' incorporates:
  //   Outport: '<Root>/DMTestOut'
  //   Outport: '<Root>/GainResult'

  Model2MDLOBJ3.step(ExtYPointer_ref_Y->GainResult,
                     &ExtYPointer_ref_Y->DMTestOut);

  // Outport: '<Root>/DMTestOutDblNoise' incorporates:
  //   Constant: '<Root>/Constant'
  //   DataTypeConversion: '<Root>/Data Type Conversion'
  //   Outport: '<Root>/DMTestOut'
  //   Sum: '<Root>/Sum'

  ExtYPointer_ref_Y->DMTestOutDblNoise = static_cast<real_T>
    (ExtYPointer_ref_Y->DMTestOut) + 0.215879;

  // Outport: '<Root>/DirectionFeedback' incorporates:
  //   Inport: '<Root>/DMTestDirection'

  ExtYPointer_ref_Y->DirectionFeedback = ExtUPointer_ref_U->DMTestDirection;

  // Update for Delay: '<Root>/Delay' incorporates:
  //   Outport: '<Root>/DMTestOut'

  Reference_DW.Delay_DSTATE = ExtYPointer_ref_Y->DMTestOut;
}

// Model initialize function
void Datamaps_test::initialize()
{
  // (no initialization code required)
}

// set method for External inputs (root inport signals)
void Datamaps_test::setExternalInputs(const Datamaps_test::
  ExtUPointer_Reference_T &ExtUPointer_U)
{
  *ExtUPointer_ref_U = ExtUPointer_U;
}

// get method for External outputs (root outport signals)
const Datamaps_test::ExtYPointer_Reference_T &Datamaps_test::getExternalOutputs()
  const
{
  return *ExtYPointer_ref_Y;
}

// Constructor
Datamaps_test::Datamaps_test(ExtUPointer_Reference_T *Reference_ExtUPointer_g,
  ExtYPointer_Reference_T *Reference_ExtYPointer_i):
  ExtUPointer_ref_U(),
  ExtYPointer_ref_Y(),
  Reference_DW()
{
  ExtUPointer_ref_U = Reference_ExtUPointer_g;
  ExtYPointer_ref_Y = Reference_ExtYPointer_i;
}

// Destructor
Datamaps_test::~Datamaps_test()
{
  // Currently there is no destructor body generated.
}

//
// File trailer for generated code.
//
// [EOF]
//
