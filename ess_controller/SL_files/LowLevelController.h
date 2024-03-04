//
// File: LowLevelController.h
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
#ifndef RTW_HEADER_LowLevelController_h_
#define RTW_HEADER_LowLevelController_h_
#include "rtwtypes.h"
// #include "rtw_continuous.h"    // This is an empty file

// Macros for accessing real-time model data structure
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

// Class declaration for model LowLevelController
class LowLevelController final
{
  // public data and function members
 public:
  // External inputs (root inport signals with default storage)
  struct ExtU {
    int32_T Common_IN;                 // '<Root>/Common_IN'
    int32_T Unique_IN;                 // '<Root>/Unique_IN'
  };

  // External outputs (root outports fed by signals with default storage)
  struct ExtY {
    int32_T LowLevelOut;               // '<Root>/LowLevelOut'
  };

  // Real-time Model Data Structure
  struct RT_MODEL {
    const char_T * volatile errorStatus;
  };

  // Copy Constructor
  LowLevelController(LowLevelController const&) = delete;

  // Assignment Operator
  LowLevelController& operator= (LowLevelController const&) & = delete;

  // Move Constructor
  LowLevelController(LowLevelController &&) = delete;

  // Move Assignment Operator
  LowLevelController& operator= (LowLevelController &&) = delete;

  // Real-Time Model get method
  LowLevelController::RT_MODEL * getRTM();

  // External inputs
  ExtU rtU;

  // External outputs
  ExtY rtY;

  // model initialize function
  static void initialize();

  // model step function
  void step();

  // Constructor
  LowLevelController();

  // Destructor
  ~LowLevelController();

  // private data and function members
 private:
  // Real-Time Model
  RT_MODEL rtM;
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
//  '<Root>' : 'LowLevelController'

#endif                                 // RTW_HEADER_LowLevelController_h_

//
// File trailer for generated code.
//
// [EOF]
//
