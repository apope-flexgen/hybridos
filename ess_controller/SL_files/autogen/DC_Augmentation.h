//
// Trial License - for use to evaluate programs for possible purchase as
// an end-user only.
//
// File: DC_Augmentation.h
//
// Code generated for Simulink model 'DC_Augmentation'.
//
// Model version                  : 1.66
// Simulink Coder version         : 9.8 (R2022b) 13-May-2022
// C/C++ source code generated on : Wed Oct 18 11:48:44 2023
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives:
//    1. Execution efficiency
//    2. RAM efficiency
// Validation result: Not run
//
#ifndef RTW_HEADER_DC_Augmentation_h_
#define RTW_HEADER_DC_Augmentation_h_
#include "rtwtypes.h"

// Macros for accessing real-time model data structure
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

// Exported data declaration

// Const memory section
// Declaration for custom storage class: Const
extern const boolean_T CLOSED;         // Referenced by: '<S2>/Chart'
extern const int64_T FAULT;            // Referenced by: '<S2>/Chart'
extern const int64_T MAINT;            // Referenced by: '<S2>/Chart'
extern const int64_T OFF;              // Referenced by:
                                          //  '<S2>/Chart'
                                          //  '<S3>/Chart'

extern const int64_T RUN;              // Referenced by:
                                          //  '<S2>/Chart'
                                          //  '<S3>/Chart'
                                          //  '<S8>/Chart'

extern const int64_T RUN_BALANCE;      // Referenced by:
                                          //  '<S2>/Chart'
                                          //  '<S7>/Chart'
                                          //  '<S8>/Chart'

extern const int64_T RUN_DEFAULT;      // Referenced by:
                                          //  '<S2>/Chart'
                                          //  '<S8>/Chart'

extern const int64_T RUN_OFF;          // Referenced by:
                                          //  '<S2>/Chart'
                                          //  '<S8>/Chart'

extern const int64_T RUN_SUPPORT;      // Referenced by:
                                          //  '<S2>/Chart'
                                          //  '<S7>/Chart'
                                          //  '<S8>/Chart'

extern const int64_T STARTUP;          // Referenced by:
                                          //  '<S2>/Chart'
                                          //  '<S3>/Chart'


// Class declaration for model DC_Augmentation
class DC_Augmentation_class final
{
  // public data and function members
 public:
  // Block signals and states (default storage) for system '<S1>/DcdcRampRate'
  struct DW_DcdcRampRate {
    uint8_T is_c4_DC_Augmentation;     // '<S1>/DcdcRampRate'
  };

  // Block signals and states (default storage) for system '<Root>'
  struct DW {
    real_T dcdc_reg_pwr_lim;           // '<Root>/dcdc_reg_pwr_lim'
    real_T bms_1_lo_pwr_lim;           // '<Root>/bms_1_lo_pwr_lim'
    real_T bms_1_reg_pwr_lim;          // '<Root>/bms_1_reg_pwr_lim'
    real_T bms_2_reg_pwr_lim;          // '<Root>/bms_2_reg_pwr_lim'
    real_T Switch;                     // '<S1>/Switch'
    real_T Switch4;                    // '<S1>/Switch4'
    real_T Switch2;                    // '<S1>/Switch2'
    real_T ActvPwrCmd;                 // '<S3>/Chart'
    real_T offcmd;                     // '<S3>/Chart'
    real_T oncmd;                      // '<S3>/Chart'
    real_T DcdcPcmd;                   // '<S8>/Chart'
    real_T PcsPcmd;                    // '<S8>/Chart'
    real_T PcsQcmd;                    // '<S8>/Chart'
    real_T dcaModeLast_DSTATE;         // '<Root>/dcaModeLast'
    real_T Delay_DSTATE;               // '<S1>/Delay'
    real_T Delay2_DSTATE;              // '<S1>/Delay2'
    real_T Delay1_DSTATE;              // '<S1>/Delay1'
    real_T startupComplete;            // '<S2>/Chart'
    int32_T sfEvent;                   // '<S2>/Chart'
    uint8_T is_Start_Stop_Handler;     // '<S3>/Chart'
    uint8_T is_active_c1_DC_Augmentation;// '<S3>/Chart'
    uint8_T is_c3_DC_Augmentation;     // '<S2>/Chart'
    uint8_T is_NoFault;                // '<S2>/Chart'
    uint8_T is_Run;                    // '<S2>/Chart'
    uint8_T is_SocBalance;             // '<S2>/Chart'
    uint8_T is_ActvPwrSupport;         // '<S2>/Chart'
    uint8_T is_active_c3_DC_Augmentation;// '<S2>/Chart'
    uint8_T is_c2_DC_Augmentation;     // '<S8>/Chart'
    uint8_T is_On;                     // '<S8>/Chart'
    uint8_T is_BalanceSOC;             // '<S8>/Chart'
    uint8_T is_active_c2_DC_Augmentation;// '<S8>/Chart'
  };

  // External inputs (root inport signals), for system '<Root>'
  struct ExtUPointer {
    real_T Pcmd;                       // '<Root>/Pcmd'
    real_T Qcmd;                       // '<Root>/Qcmd'
    real_T StartVal;                   // '<Root>/StartVal'
    real_T StopVal;                    // '<Root>/StopVal'
    real_T FltClr;                     // '<Root>/FltClr'
    real_T MaintCmd;                   // '<Root>/MaintCmd'
    real_T DcaFltActv;                 // '<Root>/DcaFltActv'
    real_T BMS_1_SOC;                  // '<Root>/BMS_1_SOC'
    real_T BMS_1_Volts;                // '<Root>/BMS_1_Volts'
    real_T BMS_1_Current;              // '<Root>/BMS_1_Current'
    real_T BMS_1_Capacity;             // '<Root>/BMS_1_Capacity'
    real_T BMS_1_ChargePwrLim;         // '<Root>/BMS_1_ChargePwrLim'
    real_T BMS_1_DischargePwrLim;      // '<Root>/BMS_1_DischargePwrLim'
    real_T BMS_1_ChargeCurrentLim;     // '<Root>/BMS_1_ChargeCurrentLim'
    real_T BMS_1_DischargeCurrentLim;  // '<Root>/BMS_1_DischargeCurrentLim'
    real_T BMS_2_SOC;                  // '<Root>/BMS_2_SOC'
    real_T BMS_2_Volts;                // '<Root>/BMS_2_Volts'
    real_T BMS_2_Current;              // '<Root>/BMS_2_Current'
    real_T BMS_2_Capacity;             // '<Root>/BMS_2_Capacity'
    real_T BMS_2_ChargePwrLim;         // '<Root>/BMS_2_ChargePwrLim'
    real_T BMS_2_DischargePwrLim;      // '<Root>/BMS_2_DischargePwrLim'
    real_T BMS_2_ChargeCurrentLim;     // '<Root>/BMS_2_ChargeCurrentLim'
    real_T BMS_2_DischargeCurrentLim;  // '<Root>/BMS_2_DischargeCurrentLim'
    real_T DCDC_1_V1;                  // '<Root>/DCDC_1_V1'
    real_T DCDC_1_V2;                  // '<Root>/DCDC_1_V2'
    real_T DCDC_1_Current;             // '<Root>/DCDC_1_Current'
    real_T DCDC_1_Power;               // '<Root>/DCDC_1_Power'
    real_T DCDC_1_Plim;                // '<Root>/DCDC_1_Plim'
    real_T PCS_1_DcVlt;                // '<Root>/PCS_1_DcVlt'
    real_T PCS_1_DcCurrent;            // '<Root>/PCS_1_DcCurrent'
    real_T PCS_1_ActivePower;          // '<Root>/PCS_1_ActivePower'
    real_T PCS_1_Plim;                 // '<Root>/PCS_1_Plim'
    real_T PCS_1_Qlim;                 // '<Root>/PCS_1_Qlim'
    real_T PCS_1_PRamp;                // '<Root>/PCS_1_PRamp'
    real_T PCS_1_Qramp;                // '<Root>/PCS_1_Qramp'
    real_T DCDC_1_PRamp;               // '<Root>/DCDC_1_PRamp'
    real_T DCDC_1_PNom;                // '<Root>/DCDC_1_PNom'
    real_T BMS_1_PNom;                 // '<Root>/BMS_1_PNom'
    real_T BMS_2_PNom;                 // '<Root>/BMS_2_PNom'
    boolean_T StartStop;               // '<Root>/StartStop'
    boolean_T BMS_1_DCDisconnectStt;   // '<Root>/BMS_1_DCDisconnectStt'
    boolean_T BMS_2_DCDisconnectStt;   // '<Root>/BMS_2_DCDisconnectStt'
    boolean_T DCDC_1_Status;           // '<Root>/DCDC_1_Status'
    boolean_T PCS_1_Status;            // '<Root>/PCS_1_Status'
  };

  // External outputs (root outport signals), for system '<Root>'
  struct ExtYPointer {
    real_T DCDC_1_Pcmd;                // '<Root>/DCDC_1_Pcmd'
    real_T PCS_1_Pcmd;                 // '<Root>/PCS_1_Pcmd'
    real_T PCS_1_Qcmd;                 // '<Root>/PCS_1_Qcmd'
    real_T DcaMode;                    // '<Root>/DcaMode'
    real_T RunMode;                    // '<Root>/RunMode'
  };

  // Real-time Model Data Structure
  struct RT_MODEL {
    const char_T * volatile errorStatus;
  };

  // model data, for system '<S1>/DcdcRampRate'
  struct self_DcdcRampRate {
    DW_DcdcRampRate dwork;
  };

  // Copy Constructor
  DC_Augmentation_class(DC_Augmentation_class const&) = delete;

  // Assignment Operator
  DC_Augmentation_class& operator= (DC_Augmentation_class const&) & = delete;

  // Move Constructor
  DC_Augmentation_class(DC_Augmentation_class &&) = delete;

  // Move Assignment Operator
  DC_Augmentation_class& operator= (DC_Augmentation_class &&) = delete;

  // Real-Time Model get method
  DC_Augmentation_class::RT_MODEL * getRTM();

  // Constructor
  DC_Augmentation_class(ExtUPointer *rtExtUPointer_l, ExtYPointer
                        *rtExtYPointer_o);

  // External inputs (root inport signals)
  ExtUPointer *ExtUPointer_ref_U;

  // External outputs (root outport signals)
  ExtYPointer *ExtYPointer_ref_Y;

  // set method for External inputs (root inport signals)
  void setExternalInputs(const ExtUPointer &ExtUPointer_U);

  // get method for External outputs (root outport signals)
  const ExtYPointer &getExternalOutputs() const;

  // model initialize function
  void initialize();

  // model step function
  void step();

  // Destructor
  ~DC_Augmentation_class();

  // private data and function members
 private:
  // Block states
  DW rtDW;

  // model data
  self_DcdcRampRate self_sf_PcsRampRate;

  // model data
  self_DcdcRampRate self_sf_PcsQRampRate;

  // model data
  self_DcdcRampRate self_sf_DcdcRampRate;

  // private member function(s) for subsystem '<S1>/DcdcRampRate'
  static void DcdcRampRate_Init(self_DcdcRampRate *DC_Augmentation_self_arg,
    real_T rtu_pCmd, real_T *rty_pOut);
  static void DcdcRampRate(self_DcdcRampRate *DC_Augmentation_self_arg, real_T
    rtu_kWperS, real_T rtu_pCmd, real_T rtu_updtRate, real_T rtu_pLast, real_T
    *rty_pOut);

  // private member function(s) for subsystem '<Root>'
  void inner_default_Negative_power(void);
  void inner_default_Positive_power(void);
  void enter_internal_Positive_power(void);
  void enter_internal_Negative_power(void);
  void inner_default_Positive_power_e(void);
  void enter_internal_Positive_power_l(void);
  void inner_default_Negative_Power(void);
  void enter_internal_Negative_Power(void);
  void enter_internal_SocBalance(void);
  void Run(void);

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
//  '<Root>' : 'DC_Augmentation'
//  '<S1>'   : 'DC_Augmentation/DCA_Asset_Ctrl'
//  '<S2>'   : 'DC_Augmentation/DCA_Mode_Ctrl'
//  '<S3>'   : 'DC_Augmentation/DCA_input_handler'
//  '<S4>'   : 'DC_Augmentation/DCA_Asset_Ctrl/DcdcRampRate'
//  '<S5>'   : 'DC_Augmentation/DCA_Asset_Ctrl/PcsQRampRate'
//  '<S6>'   : 'DC_Augmentation/DCA_Asset_Ctrl/PcsRampRate'
//  '<S7>'   : 'DC_Augmentation/DCA_Asset_Ctrl/Power_Limits_Handler'
//  '<S8>'   : 'DC_Augmentation/DCA_Asset_Ctrl/Power_Split_Handler'
//  '<S9>'   : 'DC_Augmentation/DCA_Asset_Ctrl/Power_Limits_Handler/Chart'
//  '<S10>'  : 'DC_Augmentation/DCA_Asset_Ctrl/Power_Split_Handler/Chart'
//  '<S11>'  : 'DC_Augmentation/DCA_Mode_Ctrl/Chart'
//  '<S12>'  : 'DC_Augmentation/DCA_input_handler/Chart'

#endif                                 // RTW_HEADER_DC_Augmentation_h_

//
// File trailer for generated code.
//
// [EOF]
//