//
// File: HighLevelController.h
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
#ifndef RTW_HEADER_HighLevelController_h_
#define RTW_HEADER_HighLevelController_h_
#include "rtwtypes.h"
// #include "HighLevelController_types.h"   // this is an empty file

// Macros for accessing real-time model data structure
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm) ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val) ((rtm)->errorStatus = (val))
#endif

// Class declaration for model HighLevelController
class HighLevelController final
{
    // public data and function members
public:
    // External inputs (root inport signals with default storage)
    struct ExtU_HighLevelController_T
    {
        boolean_T In1;  // '<Root>/In1'
    };

    // External outputs (root outports fed by signals with default storage)
    struct ExtY_HighLevelController_T
    {
        int32_T Out1;  // '<Root>/Out1'
    };

    // Real-time Model Data Structure
    struct RT_MODEL_HighLevelController_T
    {
        const char_T* volatile errorStatus;
    };

    // Copy Constructor
    HighLevelController(HighLevelController const&) = delete;

    // Assignment Operator
    HighLevelController& operator=(HighLevelController const&) & = delete;

    // Move Constructor
    HighLevelController(HighLevelController&&) = delete;

    // Move Assignment Operator
    HighLevelController& operator=(HighLevelController&&) = delete;

    // Real-Time Model get method
    HighLevelController::RT_MODEL_HighLevelController_T* getRTM();

    // External inputs
    ExtU_HighLevelController_T HighLevelController_U;

    // External outputs
    ExtY_HighLevelController_T HighLevelController_Y;

    // Root inports set method
    void setExternalInputs(const ExtU_HighLevelController_T* pExtU_HighLevelController_T)
    {
        HighLevelController_U = *pExtU_HighLevelController_T;
    }

    // Root outports get method
    const ExtY_HighLevelController_T& getExternalOutputs() const { return HighLevelController_Y; }

    // model initialize function
    static void initialize();

    // model step function
    void step();

    // model terminate function
    static void terminate();

    // Constructor
    HighLevelController();

    // Destructor
    ~HighLevelController();

    // private data and function members
private:
    // Real-Time Model
    RT_MODEL_HighLevelController_T HighLevelController_M;
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
//  '<Root>' : 'HighLevelController'

#endif  // RTW_HEADER_HighLevelController_h_

//
// File trailer for generated code.
//
// [EOF]
//
