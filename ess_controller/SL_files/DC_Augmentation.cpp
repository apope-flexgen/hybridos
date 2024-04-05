//
// Trial License - for use to evaluate programs for possible purchase as
// an end-user only.
//
// File: DC_Augmentation.cpp
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
#include "DC_Augmentation.h"
#include "rtwtypes.h"
#include <cmath>

// Named constants for Chart: '<S1>/DcdcRampRate'
const uint8_T IN_Down{ 1U };

const uint8_T IN_Same{ 2U };

const uint8_T IN_Up{ 3U };

// Named constants for Chart: '<S8>/Chart'
const uint8_T IN_BMS_1_High{ 1U };

const uint8_T IN_BMS_2_High{ 2U };

const uint8_T IN_BalanceSOC{ 1U };

const uint8_T IN_Off{ 1U };

const uint8_T IN_On{ 2U };

const uint8_T IN_SocBalanced{ 3U };

const uint8_T IN_Standby{ 2U };

const uint8_T IN_SupportPower{ 3U };

// Named constants for Chart: '<S2>/Chart'
const uint8_T IN_ActvPwrSupport{ 1U };

const uint8_T IN_Fault{ 1U };

const uint8_T IN_Maintenance{ 1U };

const uint8_T IN_Negative_Power{ 1U };

const uint8_T IN_Negative_power{ 1U };

const uint8_T IN_NoFault{ 2U };

const uint8_T IN_Off_j{ 2U };

const uint8_T IN_Positive_power{ 2U };

const uint8_T IN_Run{ 3U };

const uint8_T IN_SocBalance{ 2U };

const uint8_T IN_Startup{ 4U };

const int32_T event_PwrSupportTransition{ 0 };

const int32_T event_SocBalanceTransition{ 1 };

// Exported data definition

// Const memory section
// Definition for custom storage class: Const
const boolean_T CLOSED{ 1u };  // Referenced by: '<S2>/Chart'

const int64_T FAULT{ (-100L) };  // Referenced by: '<S2>/Chart'

const int64_T MAINT{ (300L) };  // Referenced by: '<S2>/Chart'

const int64_T OFF{ (0L) };  // Referenced by:
                            //  '<S2>/Chart'
                            //  '<S3>/Chart'

const int64_T RUN{ (100L) };  // Referenced by:
                              //  '<S2>/Chart'
                              //  '<S3>/Chart'
                              //  '<S8>/Chart'

const int64_T RUN_BALANCE{ (112L) };  // Referenced by:
                                      //  '<S2>/Chart'
                                      //  '<S7>/Chart'
                                      //  '<S8>/Chart'

const int64_T RUN_DEFAULT{ (111L) };  // Referenced by:
                                      //  '<S2>/Chart'
                                      //  '<S8>/Chart'

const int64_T RUN_OFF{ (110L) };  // Referenced by:
                                  //  '<S2>/Chart'
                                  //  '<S8>/Chart'

const int64_T RUN_SUPPORT{ (113L) };  // Referenced by:
                                      //  '<S2>/Chart'
                                      //  '<S7>/Chart'
                                      //  '<S8>/Chart'

const int64_T STARTUP{ (200L) };  // Referenced by:
                                  //  '<S2>/Chart'
                                  //  '<S3>/Chart'

//
// System initialize for atomic system:
//    '<S1>/DcdcRampRate'
//    '<S1>/PcsQRampRate'
//    '<S1>/PcsRampRate'
//
void DC_Augmentation_class::DcdcRampRate_Init(self_DcdcRampRate* DC_Augmentation_self_arg, real_T rtu_pCmd,
                                              real_T* rty_pOut)
{
    // Chart: '<S1>/DcdcRampRate'
    DC_Augmentation_self_arg->dwork.is_c4_DC_Augmentation = IN_Same;
    *rty_pOut = rtu_pCmd;
}

//
// Output and update for atomic system:
//    '<S1>/DcdcRampRate'
//    '<S1>/PcsQRampRate'
//    '<S1>/PcsRampRate'
//
void DC_Augmentation_class::DcdcRampRate(self_DcdcRampRate* DC_Augmentation_self_arg, real_T rtu_kWperS,
                                         real_T rtu_pCmd, real_T rtu_updtRate, real_T rtu_pLast, real_T* rty_pOut)
{
    switch (DC_Augmentation_self_arg->dwork.is_c4_DC_Augmentation)
    {
        case IN_Down:
            if (rtu_pCmd == rtu_pLast)
            {
                DC_Augmentation_self_arg->dwork.is_c4_DC_Augmentation = IN_Same;
                *rty_pOut = rtu_pCmd;
            }
            else if (rtu_pCmd > rtu_pLast)
            {
                DC_Augmentation_self_arg->dwork.is_c4_DC_Augmentation = IN_Up;
                *rty_pOut = rtu_kWperS * rtu_updtRate + rtu_pLast;
                if (*rty_pOut > rtu_pCmd)
                {
                    *rty_pOut = rtu_pCmd;
                }
            }
            else
            {
                *rty_pOut = rtu_pLast - rtu_kWperS * rtu_updtRate;
                if (*rty_pOut < rtu_pCmd)
                {
                    *rty_pOut = rtu_pCmd;
                }
            }
            break;

        case IN_Same:
            if (rtu_pCmd > rtu_pLast)
            {
                DC_Augmentation_self_arg->dwork.is_c4_DC_Augmentation = IN_Up;
                *rty_pOut = rtu_kWperS * rtu_updtRate + rtu_pLast;
                if (*rty_pOut > rtu_pCmd)
                {
                    *rty_pOut = rtu_pCmd;
                }
            }
            else if (rtu_pCmd < rtu_pLast)
            {
                DC_Augmentation_self_arg->dwork.is_c4_DC_Augmentation = IN_Down;
                *rty_pOut = rtu_pLast - rtu_kWperS * rtu_updtRate;
                if (*rty_pOut < rtu_pCmd)
                {
                    *rty_pOut = rtu_pCmd;
                }
            }
            else
            {
                *rty_pOut = rtu_pCmd;
            }
            break;

        default:
            // case IN_Up:
            if (rtu_pCmd == rtu_pLast)
            {
                DC_Augmentation_self_arg->dwork.is_c4_DC_Augmentation = IN_Same;
                *rty_pOut = rtu_pCmd;
            }
            else if (rtu_pCmd < rtu_pLast)
            {
                DC_Augmentation_self_arg->dwork.is_c4_DC_Augmentation = IN_Down;
                *rty_pOut = rtu_pLast - rtu_kWperS * rtu_updtRate;
                if (*rty_pOut < rtu_pCmd)
                {
                    *rty_pOut = rtu_pCmd;
                }
            }
            else
            {
                *rty_pOut = rtu_kWperS * rtu_updtRate + rtu_pLast;
                if (*rty_pOut > rtu_pCmd)
                {
                    *rty_pOut = rtu_pCmd;
                }
            }
            break;
    }
}

// Function for Chart: '<S2>/Chart'
void DC_Augmentation_class::inner_default_Negative_power(void)
{
    // Inport: '<Root>/BMS_1_ChargePwrLim'
    if (ExtUPointer_ref_U->BMS_1_ChargePwrLim >= rtDW.bms_1_reg_pwr_lim)
    {
        int32_T b_previousEvent;
        b_previousEvent = rtDW.sfEvent;
        rtDW.sfEvent = event_SocBalanceTransition;
        if (rtDW.is_NoFault == IN_Run)
        {
            Run();
        }

        rtDW.sfEvent = b_previousEvent;
    }

    // End of Inport: '<Root>/BMS_1_ChargePwrLim'
}

// Function for Chart: '<S2>/Chart'
void DC_Augmentation_class::inner_default_Positive_power(void)
{
    // Inport: '<Root>/BMS_1_DischargePwrLim'
    if (ExtUPointer_ref_U->BMS_1_DischargePwrLim >= rtDW.bms_1_reg_pwr_lim)
    {
        int32_T b_previousEvent;
        b_previousEvent = rtDW.sfEvent;
        rtDW.sfEvent = event_SocBalanceTransition;
        if (rtDW.is_NoFault == IN_Run)
        {
            Run();
        }

        rtDW.sfEvent = b_previousEvent;
    }

    // End of Inport: '<Root>/BMS_1_DischargePwrLim'
}

// Function for Chart: '<S2>/Chart'
void DC_Augmentation_class::enter_internal_Positive_power(void)
{
    inner_default_Positive_power();
}

// Function for Chart: '<S2>/Chart'
void DC_Augmentation_class::enter_internal_Negative_power(void)
{
    inner_default_Negative_power();
}

// Function for Chart: '<S2>/Chart'
void DC_Augmentation_class::inner_default_Positive_power_e(void)
{
    // Inport: '<Root>/BMS_1_DischargePwrLim' incorporates:
    //   Inport: '<Root>/BMS_2_DischargePwrLim'
    //   Inport: '<Root>/DCDC_1_Plim'

    if ((ExtUPointer_ref_U->BMS_1_DischargePwrLim < rtDW.bms_1_lo_pwr_lim) &&
        (ExtUPointer_ref_U->BMS_2_DischargePwrLim >= rtDW.bms_2_reg_pwr_lim) &&
        (ExtUPointer_ref_U->DCDC_1_Plim >= rtDW.dcdc_reg_pwr_lim))
    {
        int32_T b_previousEvent;
        b_previousEvent = rtDW.sfEvent;
        rtDW.sfEvent = event_PwrSupportTransition;
        if (rtDW.is_NoFault == IN_Run)
        {
            Run();
        }

        rtDW.sfEvent = b_previousEvent;
    }

    // End of Inport: '<Root>/BMS_1_DischargePwrLim'
}

// Function for Chart: '<S2>/Chart'
void DC_Augmentation_class::enter_internal_Positive_power_l(void)
{
    inner_default_Positive_power_e();
}

// Function for Chart: '<S2>/Chart'
void DC_Augmentation_class::inner_default_Negative_Power(void)
{
    // Inport: '<Root>/BMS_1_ChargePwrLim' incorporates:
    //   Inport: '<Root>/BMS_2_ChargePwrLim'
    //   Inport: '<Root>/DCDC_1_Plim'

    if ((ExtUPointer_ref_U->BMS_1_ChargePwrLim < rtDW.bms_1_lo_pwr_lim) &&
        (ExtUPointer_ref_U->BMS_2_ChargePwrLim >= rtDW.bms_2_reg_pwr_lim) &&
        (ExtUPointer_ref_U->DCDC_1_Plim >= rtDW.dcdc_reg_pwr_lim))
    {
        int32_T b_previousEvent;
        b_previousEvent = rtDW.sfEvent;
        rtDW.sfEvent = event_PwrSupportTransition;
        if (rtDW.is_NoFault == IN_Run)
        {
            Run();
        }

        rtDW.sfEvent = b_previousEvent;
    }

    // End of Inport: '<Root>/BMS_1_ChargePwrLim'
}

// Function for Chart: '<S2>/Chart'
void DC_Augmentation_class::enter_internal_Negative_Power(void)
{
    inner_default_Negative_Power();
}

// Function for Chart: '<S2>/Chart'
void DC_Augmentation_class::enter_internal_SocBalance(void)
{
    if (rtDW.ActvPwrCmd < 0.0)
    {
        rtDW.is_SocBalance = IN_Negative_Power;
        enter_internal_Negative_Power();
    }
    else
    {
        rtDW.is_SocBalance = IN_Positive_power;
        enter_internal_Positive_power_l();
    }
}

// Function for Chart: '<S2>/Chart'
void DC_Augmentation_class::Run(void)
{
    // Update for Outport: '<Root>/DcaMode'
    ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(RUN);

    // Inport: '<Root>/MaintCmd'
    if (ExtUPointer_ref_U->MaintCmd != 0.0)
    {
        rtDW.is_ActvPwrSupport = 0;
        rtDW.is_SocBalance = 0;
        rtDW.is_Run = 0;

        // Outport: '<Root>/RunMode'
        ExtYPointer_ref_Y->RunMode = static_cast<real_T>(RUN_OFF);
        rtDW.is_NoFault = IN_Maintenance;

        // Update for Outport: '<Root>/DcaMode'
        ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(MAINT);
    }
    else if (rtDW.offcmd != 0.0)
    {
        rtDW.is_ActvPwrSupport = 0;
        rtDW.is_SocBalance = 0;
        rtDW.is_Run = 0;

        // Outport: '<Root>/RunMode'
        ExtYPointer_ref_Y->RunMode = static_cast<real_T>(RUN_OFF);
        rtDW.is_NoFault = IN_Off_j;

        // Update for Outport: '<Root>/DcaMode'
        ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(OFF);
    }
    else
    {
        switch (rtDW.is_Run)
        {
            case IN_ActvPwrSupport:
                if (rtDW.sfEvent == event_SocBalanceTransition)
                {
                    rtDW.is_ActvPwrSupport = 0;
                    rtDW.is_Run = IN_SocBalance;

                    // Outport: '<Root>/RunMode'
                    ExtYPointer_ref_Y->RunMode = static_cast<real_T>(RUN_BALANCE);
                    enter_internal_SocBalance();
                }
                else
                {
                    switch (rtDW.is_ActvPwrSupport)
                    {
                        case IN_Negative_power:
                            if (rtDW.ActvPwrCmd >= 0.0)
                            {
                                rtDW.is_ActvPwrSupport = IN_Positive_power;
                                enter_internal_Positive_power();
                            }
                            else
                            {
                                inner_default_Negative_power();
                            }
                            break;

                        case IN_Positive_power:
                            if (rtDW.ActvPwrCmd < 0.0)
                            {
                                rtDW.is_ActvPwrSupport = IN_Negative_power;
                                enter_internal_Negative_power();
                            }
                            else
                            {
                                inner_default_Positive_power();
                            }
                            break;
                    }
                }
                break;

            case IN_SocBalance:
                if (rtDW.sfEvent == event_PwrSupportTransition)
                {
                    rtDW.is_SocBalance = 0;
                    rtDW.is_Run = IN_ActvPwrSupport;

                    // Outport: '<Root>/RunMode'
                    ExtYPointer_ref_Y->RunMode = static_cast<real_T>(RUN_SUPPORT);
                    if (rtDW.ActvPwrCmd < 0.0)
                    {
                        rtDW.is_ActvPwrSupport = IN_Negative_power;
                        enter_internal_Negative_power();
                    }
                    else
                    {
                        rtDW.is_ActvPwrSupport = IN_Positive_power;
                        enter_internal_Positive_power();
                    }
                }
                else
                {
                    switch (rtDW.is_SocBalance)
                    {
                        case IN_Negative_Power:
                            if (rtDW.ActvPwrCmd >= 0.0)
                            {
                                rtDW.is_SocBalance = IN_Positive_power;
                                enter_internal_Positive_power_l();
                            }
                            else
                            {
                                inner_default_Negative_Power();
                            }
                            break;

                        case IN_Positive_power:
                            if (rtDW.ActvPwrCmd < 0.0)
                            {
                                rtDW.is_SocBalance = IN_Negative_Power;
                                enter_internal_Negative_Power();
                            }
                            else
                            {
                                inner_default_Positive_power_e();
                            }
                            break;
                    }
                }
                break;
        }
    }

    // End of Inport: '<Root>/MaintCmd'
}

// Model step function
void DC_Augmentation_class::step()
{
    real_T rtb_DcdcChrgPlim;
    real_T rtb_DcdcDischgPlim;
    real_T rtb_ReactvPwrCmd;
    real_T rtb_Sum;
    real_T rtb_dcaModeLast;
    real_T tmp;

    // Delay: '<Root>/dcaModeLast'
    rtb_dcaModeLast = rtDW.dcaModeLast_DSTATE;

    // Chart: '<S3>/Chart' incorporates:
    //   Inport: '<Root>/FltClr'
    //   Inport: '<Root>/Pcmd'
    //   Inport: '<Root>/Qcmd'
    //   Inport: '<Root>/StartStop'

    if (rtDW.is_active_c1_DC_Augmentation == 0U)
    {
        rtDW.is_active_c1_DC_Augmentation = 1U;
        rtDW.ActvPwrCmd = ExtUPointer_ref_U->Pcmd;
        rtb_ReactvPwrCmd = ExtUPointer_ref_U->Qcmd;
        rtDW.is_Start_Stop_Handler = IN_Off;
        rtDW.offcmd = 0.0;
        if (ExtUPointer_ref_U->StartStop != 0u)
        {
            rtDW.oncmd = 1.0;
        }

        rtb_dcaModeLast = ExtUPointer_ref_U->FltClr;
    }
    else
    {
        rtDW.ActvPwrCmd = ExtUPointer_ref_U->Pcmd;
        rtb_ReactvPwrCmd = ExtUPointer_ref_U->Qcmd;
        if (rtDW.is_Start_Stop_Handler == IN_Off)
        {
            if ((rtb_dcaModeLast == RUN) || (rtb_dcaModeLast == STARTUP))
            {
                rtDW.is_Start_Stop_Handler = IN_On;
                rtDW.offcmd = 0.0;
                if (ExtUPointer_ref_U->StartStop == 0u)
                {
                    rtDW.offcmd = 1.0;
                }
            }
            else if (ExtUPointer_ref_U->StartStop != 0u)
            {
                rtDW.oncmd = 1.0;
            }

            // case IN_On:
        }
        else if ((rtb_dcaModeLast != RUN) && (rtb_dcaModeLast != STARTUP))
        {
            rtDW.is_Start_Stop_Handler = IN_Off;
            rtDW.offcmd = 0.0;
            if (ExtUPointer_ref_U->StartStop != 0u)
            {
                rtDW.oncmd = 1.0;
            }
        }
        else if (ExtUPointer_ref_U->StartStop == 0u)
        {
            rtDW.offcmd = 1.0;
        }

        rtb_dcaModeLast = ExtUPointer_ref_U->FltClr;
    }

    // End of Chart: '<S3>/Chart'

    // Gain: '<Root>/dcdc_reg_pwr_lim' incorporates:
    //   Inport: '<Root>/DCDC_1_PNom'

    rtDW.dcdc_reg_pwr_lim = 0.5 * ExtUPointer_ref_U->DCDC_1_PNom;

    // Gain: '<Root>/bms_1_lo_pwr_lim' incorporates:
    //   Inport: '<Root>/BMS_1_PNom'

    rtDW.bms_1_lo_pwr_lim = 0.05 * ExtUPointer_ref_U->BMS_1_PNom;

    // Gain: '<Root>/bms_1_reg_pwr_lim' incorporates:
    //   Inport: '<Root>/BMS_1_PNom'

    rtDW.bms_1_reg_pwr_lim = 0.2 * ExtUPointer_ref_U->BMS_1_PNom;

    // Gain: '<Root>/bms_2_reg_pwr_lim' incorporates:
    //   Inport: '<Root>/BMS_2_PNom'

    rtDW.bms_2_reg_pwr_lim = 0.2 * ExtUPointer_ref_U->BMS_2_PNom;

    // Chart: '<S2>/Chart' incorporates:
    //   Inport: '<Root>/BMS_1_DCDisconnectStt'
    //   Inport: '<Root>/BMS_2_DCDisconnectStt'
    //   Inport: '<Root>/DCDC_1_Status'
    //   Inport: '<Root>/DcaFltActv'
    //   Inport: '<Root>/MaintCmd'
    //   Inport: '<Root>/PCS_1_Status'

    rtDW.sfEvent = -1;
    if (rtDW.is_active_c3_DC_Augmentation == 0U)
    {
        rtDW.is_active_c3_DC_Augmentation = 1U;
        rtDW.is_c3_DC_Augmentation = IN_NoFault;
        rtDW.is_NoFault = IN_Off_j;

        // Outport: '<Root>/DcaMode'
        ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(OFF);
    }
    else
    {
        switch (rtDW.is_c3_DC_Augmentation)
        {
            case IN_Fault:
                // Outport: '<Root>/DcaMode'
                ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(FAULT);
                if (rtb_dcaModeLast != 0.0)
                {
                    rtDW.is_c3_DC_Augmentation = IN_NoFault;
                    rtDW.is_NoFault = IN_Off_j;

                    // Outport: '<Root>/DcaMode'
                    ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(OFF);
                }
                break;

            case IN_NoFault:
                if (ExtUPointer_ref_U->DcaFltActv != 0.0)
                {
                    if (rtDW.is_NoFault == IN_Run)
                    {
                        rtDW.is_ActvPwrSupport = 0;
                        rtDW.is_SocBalance = 0;
                        rtDW.is_Run = 0;

                        // Outport: '<Root>/RunMode'
                        ExtYPointer_ref_Y->RunMode = static_cast<real_T>(RUN_OFF);
                        rtDW.is_NoFault = 0;
                    }
                    else
                    {
                        rtDW.is_NoFault = 0;
                    }

                    rtDW.is_c3_DC_Augmentation = IN_Fault;

                    // Outport: '<Root>/DcaMode'
                    ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(FAULT);
                }
                else
                {
                    switch (rtDW.is_NoFault)
                    {
                        case IN_Maintenance:
                            // Outport: '<Root>/DcaMode'
                            ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(MAINT);
                            if (!(ExtUPointer_ref_U->MaintCmd != 0.0))
                            {
                                rtDW.is_NoFault = IN_Off_j;

                                // Outport: '<Root>/DcaMode'
                                ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(OFF);
                            }
                            break;

                        case IN_Off_j:
                            // Outport: '<Root>/DcaMode'
                            ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(OFF);
                            if (rtDW.oncmd != 0.0)
                            {
                                rtDW.is_NoFault = IN_Startup;

                                // Outport: '<Root>/DcaMode'
                                ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(STARTUP);

                                //
                                if ((ExtUPointer_ref_U->BMS_1_DCDisconnectStt == CLOSED) &&
                                    (ExtUPointer_ref_U->BMS_2_DCDisconnectStt == CLOSED) &&
                                    (ExtUPointer_ref_U->DCDC_1_Status != 0u) && (ExtUPointer_ref_U->PCS_1_Status != 0u))
                                {
                                    rtDW.startupComplete = 1.0;
                                }
                            }
                            break;

                        case IN_Run:
                            Run();
                            break;

                        case IN_Startup:
                            // Outport: '<Root>/DcaMode'
                            ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(STARTUP);
                            if ((ExtUPointer_ref_U->MaintCmd != 0.0) && (rtDW.startupComplete != 0.0))
                            {
                                rtDW.is_NoFault = IN_Maintenance;

                                // Outport: '<Root>/DcaMode'
                                ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(MAINT);
                            }
                            else if (rtDW.startupComplete != 0.0)
                            {
                                rtDW.is_NoFault = IN_Run;

                                // Outport: '<Root>/DcaMode'
                                ExtYPointer_ref_Y->DcaMode = static_cast<real_T>(RUN);
                                rtDW.startupComplete = 0.0;
                                rtDW.is_Run = IN_SocBalance;

                                // Outport: '<Root>/RunMode'
                                ExtYPointer_ref_Y->RunMode = static_cast<real_T>(RUN_BALANCE);
                                enter_internal_SocBalance();

                                //
                            }
                            else if ((ExtUPointer_ref_U->BMS_1_DCDisconnectStt == CLOSED) &&
                                     (ExtUPointer_ref_U->BMS_2_DCDisconnectStt == CLOSED) &&
                                     (ExtUPointer_ref_U->DCDC_1_Status != 0u) &&
                                     (ExtUPointer_ref_U->PCS_1_Status != 0u))
                            {
                                rtDW.startupComplete = 1.0;
                            }
                            break;
                    }
                }
                break;
        }
    }

    // End of Chart: '<S2>/Chart'

    // Sum: '<S1>/DeltaSOC' incorporates:
    //   Inport: '<Root>/BMS_1_SOC'
    //   Inport: '<Root>/BMS_2_SOC'

    rtb_dcaModeLast = ExtUPointer_ref_U->BMS_1_SOC - ExtUPointer_ref_U->BMS_2_SOC;

    // Sum: '<Root>/Sum' incorporates:
    //   Inport: '<Root>/BMS_1_Capacity'
    //   Inport: '<Root>/BMS_2_Capacity'

    rtb_Sum = ExtUPointer_ref_U->BMS_2_Capacity + ExtUPointer_ref_U->BMS_1_Capacity;

    // Chart: '<S8>/Chart' incorporates:
    //   Inport: '<Root>/BMS_2_Capacity'
    //   Outport: '<Root>/DcaMode'
    //   Outport: '<Root>/RunMode'

    if (rtDW.is_active_c2_DC_Augmentation == 0U)
    {
        rtDW.is_active_c2_DC_Augmentation = 1U;
        rtDW.is_c2_DC_Augmentation = IN_Off;
        rtDW.PcsPcmd = 0.0;
        rtDW.DcdcPcmd = 0.0;
        rtDW.PcsQcmd = 0.0;
    }
    else if (rtDW.is_c2_DC_Augmentation == IN_Off)
    {
        if (ExtYPointer_ref_Y->DcaMode == RUN)
        {
            rtDW.is_c2_DC_Augmentation = IN_On;
            if (ExtYPointer_ref_Y->RunMode == RUN_BALANCE)
            {
                rtDW.is_On = IN_BalanceSOC;
                rtDW.is_BalanceSOC = IN_SocBalanced;
                rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                rtDW.DcdcPcmd = ExtUPointer_ref_U->BMS_2_Capacity / rtb_Sum * rtDW.ActvPwrCmd;
                rtDW.PcsQcmd = rtb_ReactvPwrCmd;
            }
            else
            {
                rtDW.is_On = IN_Standby;
                rtDW.PcsPcmd = 0.0;
                rtDW.DcdcPcmd = 0.0;
                rtDW.PcsQcmd = 0.0;
            }
        }

        // case IN_On:
    }
    else if (ExtYPointer_ref_Y->DcaMode != RUN)
    {
        rtDW.is_BalanceSOC = 0;
        rtDW.is_On = 0;
        rtDW.is_c2_DC_Augmentation = IN_Off;
        rtDW.PcsPcmd = 0.0;
        rtDW.DcdcPcmd = 0.0;
        rtDW.PcsQcmd = 0.0;
    }
    else
    {
        switch (rtDW.is_On)
        {
            case IN_BalanceSOC:
                if ((ExtYPointer_ref_Y->RunMode == RUN_DEFAULT) || (ExtYPointer_ref_Y->RunMode == RUN_OFF))
                {
                    rtDW.is_BalanceSOC = 0;
                    rtDW.is_On = IN_Standby;
                    rtDW.PcsPcmd = 0.0;
                    rtDW.DcdcPcmd = 0.0;
                    rtDW.PcsQcmd = 0.0;
                }
                else if (ExtYPointer_ref_Y->RunMode == RUN_SUPPORT)
                {
                    rtDW.is_BalanceSOC = 0;
                    rtDW.is_On = IN_SupportPower;
                    rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                    rtDW.DcdcPcmd = rtDW.ActvPwrCmd;
                }
                else
                {
                    switch (rtDW.is_BalanceSOC)
                    {
                        case IN_BMS_1_High:
                            if (rtb_dcaModeLast < 0.5)
                            {
                                rtDW.is_BalanceSOC = IN_SocBalanced;
                                rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                                rtDW.DcdcPcmd = ExtUPointer_ref_U->BMS_2_Capacity / rtb_Sum * rtDW.ActvPwrCmd;
                                rtDW.PcsQcmd = rtb_ReactvPwrCmd;
                            }
                            else
                            {
                                rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                                rtDW.PcsQcmd = rtb_ReactvPwrCmd;
                                if (rtDW.ActvPwrCmd >= 0.0)
                                {
                                    rtDW.DcdcPcmd = 0.0;
                                }
                                else
                                {
                                    rtDW.DcdcPcmd = rtDW.ActvPwrCmd;
                                }
                            }
                            break;

                        case IN_BMS_2_High:
                            if (rtb_dcaModeLast > -0.5)
                            {
                                rtDW.is_BalanceSOC = IN_SocBalanced;
                                rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                                rtDW.DcdcPcmd = ExtUPointer_ref_U->BMS_2_Capacity / rtb_Sum * rtDW.ActvPwrCmd;
                                rtDW.PcsQcmd = rtb_ReactvPwrCmd;
                            }
                            else
                            {
                                rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                                rtDW.PcsQcmd = rtb_ReactvPwrCmd;
                                if (rtDW.ActvPwrCmd >= 0.0)
                                {
                                    rtDW.DcdcPcmd = rtDW.ActvPwrCmd;
                                }
                                else
                                {
                                    rtDW.DcdcPcmd = 0.0;
                                }
                            }
                            break;

                        default:
                            // case IN_SocBalanced:
                            if (rtb_dcaModeLast > 2.0)
                            {
                                rtDW.is_BalanceSOC = IN_BMS_1_High;
                                rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                                rtDW.PcsQcmd = rtb_ReactvPwrCmd;
                                if (rtDW.ActvPwrCmd >= 0.0)
                                {
                                    rtDW.DcdcPcmd = 0.0;
                                }
                                else
                                {
                                    rtDW.DcdcPcmd = rtDW.ActvPwrCmd;
                                }
                            }
                            else if (rtb_dcaModeLast < -2.0)
                            {
                                rtDW.is_BalanceSOC = IN_BMS_2_High;
                                rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                                rtDW.PcsQcmd = rtb_ReactvPwrCmd;
                                if (rtDW.ActvPwrCmd >= 0.0)
                                {
                                    rtDW.DcdcPcmd = rtDW.ActvPwrCmd;
                                }
                                else
                                {
                                    rtDW.DcdcPcmd = 0.0;
                                }
                            }
                            else
                            {
                                rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                                rtDW.DcdcPcmd = ExtUPointer_ref_U->BMS_2_Capacity / rtb_Sum * rtDW.ActvPwrCmd;
                                rtDW.PcsQcmd = rtb_ReactvPwrCmd;
                            }
                            break;
                    }
                }
                break;

            case IN_Standby:
                if (ExtYPointer_ref_Y->RunMode == RUN_BALANCE)
                {
                    rtDW.is_On = IN_BalanceSOC;
                    rtDW.is_BalanceSOC = IN_SocBalanced;
                    rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                    rtDW.DcdcPcmd = ExtUPointer_ref_U->BMS_2_Capacity / rtb_Sum * rtDW.ActvPwrCmd;
                    rtDW.PcsQcmd = rtb_ReactvPwrCmd;
                }
                else if (ExtYPointer_ref_Y->RunMode == RUN_SUPPORT)
                {
                    rtDW.is_On = IN_SupportPower;
                    rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                    rtDW.DcdcPcmd = rtDW.ActvPwrCmd;
                }
                break;

            default:
                // case IN_SupportPower:
                if (ExtYPointer_ref_Y->RunMode == RUN_BALANCE)
                {
                    rtDW.is_On = IN_BalanceSOC;
                    rtDW.is_BalanceSOC = IN_SocBalanced;
                    rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                    rtDW.DcdcPcmd = ExtUPointer_ref_U->BMS_2_Capacity / rtb_Sum * rtDW.ActvPwrCmd;
                    rtDW.PcsQcmd = rtb_ReactvPwrCmd;
                }
                else if ((ExtYPointer_ref_Y->RunMode == RUN_DEFAULT) || (ExtYPointer_ref_Y->RunMode == RUN_OFF))
                {
                    rtDW.is_On = IN_Standby;
                    rtDW.PcsPcmd = 0.0;
                    rtDW.DcdcPcmd = 0.0;
                    rtDW.PcsQcmd = 0.0;
                }
                else
                {
                    rtDW.PcsPcmd = rtDW.ActvPwrCmd;
                    rtDW.DcdcPcmd = rtDW.ActvPwrCmd;
                }
                break;
        }
    }

    // End of Chart: '<S8>/Chart'

    // Chart: '<S7>/Chart' incorporates:
    //   Inport: '<Root>/BMS_1_ChargePwrLim'
    //   Inport: '<Root>/BMS_1_DischargePwrLim'
    //   Inport: '<Root>/BMS_2_ChargePwrLim'
    //   Inport: '<Root>/BMS_2_DischargePwrLim'
    //   Inport: '<Root>/DCDC_1_Plim'
    //   Inport: '<Root>/DCDC_1_Power'
    //   Inport: '<Root>/PCS_1_ActivePower'
    //   Inport: '<Root>/PCS_1_Plim'
    //   Outport: '<Root>/RunMode'

    rtb_DcdcChrgPlim = 0.0;
    rtb_DcdcDischgPlim = 0.0;
    rtb_ReactvPwrCmd = 0.0;
    rtb_dcaModeLast = 0.0;
    if (ExtYPointer_ref_Y->RunMode == RUN_BALANCE)
    {
        rtb_DcdcChrgPlim = (std::abs(ExtUPointer_ref_U->BMS_2_ChargePwrLim) <= ExtUPointer_ref_U->DCDC_1_Plim
                                ? std::abs(ExtUPointer_ref_U->BMS_2_ChargePwrLim)
                                : ExtUPointer_ref_U->DCDC_1_Plim) <=
                                   std::abs(ExtUPointer_ref_U->BMS_1_DischargePwrLim) -
                                       ExtUPointer_ref_U->PCS_1_ActivePower
                               ? std::abs(ExtUPointer_ref_U->BMS_2_ChargePwrLim) <= ExtUPointer_ref_U->DCDC_1_Plim
                                     ? std::abs(ExtUPointer_ref_U->BMS_2_ChargePwrLim)
                                     : ExtUPointer_ref_U->DCDC_1_Plim
                               : std::abs(ExtUPointer_ref_U->BMS_1_DischargePwrLim) -
                                     ExtUPointer_ref_U->PCS_1_ActivePower;
        rtb_DcdcDischgPlim = (std::abs(ExtUPointer_ref_U->BMS_2_DischargePwrLim) <= ExtUPointer_ref_U->DCDC_1_Plim
                                  ? std::abs(ExtUPointer_ref_U->BMS_2_DischargePwrLim)
                                  : ExtUPointer_ref_U->DCDC_1_Plim) <= std::abs(ExtUPointer_ref_U->BMS_1_ChargePwrLim) +
                                                                           ExtUPointer_ref_U->PCS_1_ActivePower
                                 ? std::abs(ExtUPointer_ref_U->BMS_2_DischargePwrLim) <= ExtUPointer_ref_U->DCDC_1_Plim
                                       ? std::abs(ExtUPointer_ref_U->BMS_2_DischargePwrLim)
                                       : ExtUPointer_ref_U->DCDC_1_Plim
                                 : std::abs(ExtUPointer_ref_U->BMS_1_ChargePwrLim) +
                                       ExtUPointer_ref_U->PCS_1_ActivePower;
        rtb_ReactvPwrCmd = ExtUPointer_ref_U->PCS_1_Plim <=
                                   std::abs(ExtUPointer_ref_U->BMS_1_ChargePwrLim) - ExtUPointer_ref_U->DCDC_1_Power
                               ? ExtUPointer_ref_U->PCS_1_Plim
                               : std::abs(ExtUPointer_ref_U->BMS_1_ChargePwrLim) - ExtUPointer_ref_U->DCDC_1_Power;
        rtb_dcaModeLast = ExtUPointer_ref_U->PCS_1_Plim <=
                                  std::abs(ExtUPointer_ref_U->BMS_1_DischargePwrLim) + ExtUPointer_ref_U->DCDC_1_Power
                              ? ExtUPointer_ref_U->PCS_1_Plim
                              : std::abs(ExtUPointer_ref_U->BMS_1_DischargePwrLim) + ExtUPointer_ref_U->DCDC_1_Power;
    }
    else if (ExtYPointer_ref_Y->RunMode == RUN_SUPPORT)
    {
        rtb_DcdcChrgPlim = (ExtUPointer_ref_U->PCS_1_Plim <= ExtUPointer_ref_U->DCDC_1_Plim
                                ? ExtUPointer_ref_U->PCS_1_Plim
                                : ExtUPointer_ref_U->DCDC_1_Plim) <= std::abs(ExtUPointer_ref_U->BMS_2_ChargePwrLim)
                               ? ExtUPointer_ref_U->PCS_1_Plim <= ExtUPointer_ref_U->DCDC_1_Plim
                                     ? ExtUPointer_ref_U->PCS_1_Plim
                                     : ExtUPointer_ref_U->DCDC_1_Plim
                               : std::abs(ExtUPointer_ref_U->BMS_2_ChargePwrLim);
        rtb_DcdcDischgPlim = (ExtUPointer_ref_U->PCS_1_Plim <= ExtUPointer_ref_U->DCDC_1_Plim
                                  ? ExtUPointer_ref_U->PCS_1_Plim
                                  : ExtUPointer_ref_U->DCDC_1_Plim) <=
                                     std::abs(ExtUPointer_ref_U->BMS_2_DischargePwrLim)
                                 ? ExtUPointer_ref_U->PCS_1_Plim <= ExtUPointer_ref_U->DCDC_1_Plim
                                       ? ExtUPointer_ref_U->PCS_1_Plim
                                       : ExtUPointer_ref_U->DCDC_1_Plim
                                 : std::abs(ExtUPointer_ref_U->BMS_2_DischargePwrLim);
        rtb_ReactvPwrCmd = rtb_DcdcChrgPlim;
        rtb_dcaModeLast = rtb_DcdcDischgPlim;
    }

    // End of Chart: '<S7>/Chart'

    // Delay: '<S1>/Delay'
    rtb_Sum = rtDW.Delay_DSTATE;

    // Switch: '<S7>/Switch2' incorporates:
    //   Constant: '<S7>/Constant'
    //   RelationalOperator: '<S7>/Equal'

    if (rtb_Sum == 0.0)
    {
        tmp = rtDW.DcdcPcmd;
    }
    else
    {
        tmp = rtb_Sum;
    }

    // Switch: '<S7>/Switch6' incorporates:
    //   Switch: '<S7>/Switch2'

    if (tmp > 0.0)
    {
        rtb_DcdcChrgPlim = rtb_DcdcDischgPlim;
    }

    // End of Switch: '<S7>/Switch6'

    // Switch: '<S1>/Switch' incorporates:
    //   Abs: '<S1>/Abs'
    //   Abs: '<S1>/Abs3'
    //   RelationalOperator: '<S1>/GreaterThan'

    if (std::abs(rtDW.DcdcPcmd) > std::abs(rtb_DcdcChrgPlim))
    {
        // Switch: '<S1>/Switch1' incorporates:
        //   Constant: '<S1>/Constant'
        //   RelationalOperator: '<S1>/GreaterThan1'

        if (rtDW.DcdcPcmd >= 0.0)
        {
            // Switch: '<S1>/Switch'
            rtDW.Switch = rtb_DcdcChrgPlim;
        }
        else
        {
            // Switch: '<S1>/Switch' incorporates:
            //   Gain: '<S1>/Gain'

            rtDW.Switch = -rtb_DcdcChrgPlim;
        }

        // End of Switch: '<S1>/Switch1'
    }
    else
    {
        // Switch: '<S1>/Switch'
        rtDW.Switch = rtDW.DcdcPcmd;
    }

    // End of Switch: '<S1>/Switch'

    // Chart: '<S1>/DcdcRampRate' incorporates:
    //   Constant: '<S1>/UpdateRate'
    //   Inport: '<Root>/DCDC_1_PRamp'
    //   Outport: '<Root>/DCDC_1_Pcmd'

    DcdcRampRate(&self_sf_DcdcRampRate, ExtUPointer_ref_U->DCDC_1_PRamp, rtDW.Switch, 0.1, rtb_Sum,
                 &ExtYPointer_ref_Y->DCDC_1_Pcmd);

    // Abs: '<S1>/Abs2' incorporates:
    //   Abs: '<S1>/Abs1'

    rtb_Sum = std::abs(rtDW.PcsPcmd);

    // Switch: '<S1>/Switch4' incorporates:
    //   Abs: '<S1>/Abs2'
    //   Abs: '<S1>/Abs5'
    //   Inport: '<Root>/PCS_1_Qlim'
    //   RelationalOperator: '<S1>/GreaterThan4'

    if (rtb_Sum > std::abs(ExtUPointer_ref_U->PCS_1_Qlim))
    {
        // Switch: '<S1>/Switch5' incorporates:
        //   Constant: '<S1>/Constant2'
        //   RelationalOperator: '<S1>/GreaterThan5'

        if (rtDW.PcsQcmd >= 0.0)
        {
            // Switch: '<S1>/Switch4'
            rtDW.Switch4 = ExtUPointer_ref_U->PCS_1_Qlim;
        }
        else
        {
            // Switch: '<S1>/Switch4' incorporates:
            //   Gain: '<S1>/Gain2'

            rtDW.Switch4 = -ExtUPointer_ref_U->PCS_1_Qlim;
        }

        // End of Switch: '<S1>/Switch5'
    }
    else
    {
        // Switch: '<S1>/Switch4'
        rtDW.Switch4 = rtDW.PcsPcmd;
    }

    // End of Switch: '<S1>/Switch4'

    // Chart: '<S1>/PcsQRampRate' incorporates:
    //   Constant: '<S1>/UpdateRate'
    //   Delay: '<S1>/Delay2'
    //   Inport: '<Root>/PCS_1_Qramp'
    //   Outport: '<Root>/PCS_1_Qcmd'

    DcdcRampRate(&self_sf_PcsQRampRate, ExtUPointer_ref_U->PCS_1_Qramp, rtDW.Switch4, 0.1, rtDW.Delay2_DSTATE,
                 &ExtYPointer_ref_Y->PCS_1_Qcmd);

    // Delay: '<S1>/Delay1'
    rtb_DcdcChrgPlim = rtDW.Delay1_DSTATE;

    // Switch: '<S7>/Switch1' incorporates:
    //   Constant: '<S7>/Constant1'
    //   RelationalOperator: '<S7>/Equal1'

    if (rtb_DcdcChrgPlim == 0.0)
    {
        tmp = rtDW.PcsPcmd;
    }
    else
    {
        tmp = rtb_DcdcChrgPlim;
    }

    // Switch: '<S7>/Switch7' incorporates:
    //   Switch: '<S7>/Switch1'

    if (tmp > 0.0)
    {
        rtb_ReactvPwrCmd = rtb_dcaModeLast;
    }

    // End of Switch: '<S7>/Switch7'

    // Switch: '<S1>/Switch2' incorporates:
    //   Abs: '<S1>/Abs4'
    //   RelationalOperator: '<S1>/GreaterThan2'

    if (rtb_Sum > std::abs(rtb_ReactvPwrCmd))
    {
        // Switch: '<S1>/Switch3' incorporates:
        //   Constant: '<S1>/Constant1'
        //   RelationalOperator: '<S1>/GreaterThan3'

        if (rtDW.PcsPcmd >= 0.0)
        {
            // Switch: '<S1>/Switch2'
            rtDW.Switch2 = rtb_ReactvPwrCmd;
        }
        else
        {
            // Switch: '<S1>/Switch2' incorporates:
            //   Gain: '<S1>/Gain1'

            rtDW.Switch2 = -rtb_ReactvPwrCmd;
        }

        // End of Switch: '<S1>/Switch3'
    }
    else
    {
        // Switch: '<S1>/Switch2'
        rtDW.Switch2 = rtDW.PcsPcmd;
    }

    // End of Switch: '<S1>/Switch2'

    // Chart: '<S1>/PcsRampRate' incorporates:
    //   Constant: '<S1>/UpdateRate'
    //   Inport: '<Root>/PCS_1_PRamp'
    //   Outport: '<Root>/PCS_1_Pcmd'

    DcdcRampRate(&self_sf_PcsRampRate, ExtUPointer_ref_U->PCS_1_PRamp, rtDW.Switch2, 0.1, rtb_DcdcChrgPlim,
                 &ExtYPointer_ref_Y->PCS_1_Pcmd);

    // Update for Delay: '<Root>/dcaModeLast' incorporates:
    //   Outport: '<Root>/DcaMode'

    rtDW.dcaModeLast_DSTATE = ExtYPointer_ref_Y->DcaMode;

    // Update for Delay: '<S1>/Delay' incorporates:
    //   Outport: '<Root>/DCDC_1_Pcmd'

    rtDW.Delay_DSTATE = ExtYPointer_ref_Y->DCDC_1_Pcmd;

    // Update for Delay: '<S1>/Delay2' incorporates:
    //   Outport: '<Root>/PCS_1_Qcmd'

    rtDW.Delay2_DSTATE = ExtYPointer_ref_Y->PCS_1_Qcmd;

    // Update for Delay: '<S1>/Delay1' incorporates:
    //   Outport: '<Root>/PCS_1_Pcmd'

    rtDW.Delay1_DSTATE = ExtYPointer_ref_Y->PCS_1_Pcmd;
}

// Model initialize function
void DC_Augmentation_class::initialize()
{
    // SystemInitialize for Chart: '<S2>/Chart'
    rtDW.sfEvent = -1;

    // SystemInitialize for Chart: '<S1>/DcdcRampRate' incorporates:
    //   Outport: '<Root>/DCDC_1_Pcmd'

    DcdcRampRate_Init(&self_sf_DcdcRampRate, rtDW.Switch, &ExtYPointer_ref_Y->DCDC_1_Pcmd);

    // SystemInitialize for Chart: '<S1>/PcsQRampRate' incorporates:
    //   Outport: '<Root>/PCS_1_Qcmd'

    DcdcRampRate_Init(&self_sf_PcsQRampRate, rtDW.Switch4, &ExtYPointer_ref_Y->PCS_1_Qcmd);

    // SystemInitialize for Chart: '<S1>/PcsRampRate' incorporates:
    //   Outport: '<Root>/PCS_1_Pcmd'

    DcdcRampRate_Init(&self_sf_PcsRampRate, rtDW.Switch2, &ExtYPointer_ref_Y->PCS_1_Pcmd);
}

// set method for External inputs (root inport signals)
void DC_Augmentation_class::setExternalInputs(const DC_Augmentation_class::ExtUPointer& ExtUPointer_U)
{
    *ExtUPointer_ref_U = ExtUPointer_U;
}

// get method for External outputs (root outport signals)
const DC_Augmentation_class::ExtYPointer& DC_Augmentation_class::getExternalOutputs() const
{
    return *ExtYPointer_ref_Y;
}

// Constructor
DC_Augmentation_class::DC_Augmentation_class(ExtUPointer* rtExtUPointer_l, ExtYPointer* rtExtYPointer_o)
    : ExtUPointer_ref_U(),
      ExtYPointer_ref_Y(),
      rtDW(),
      self_sf_PcsRampRate(),
      self_sf_PcsQRampRate(),
      self_sf_DcdcRampRate(),
      rtM()
{
    ExtUPointer_ref_U = rtExtUPointer_l;
    ExtYPointer_ref_Y = rtExtYPointer_o;
}

// Destructor
DC_Augmentation_class::~DC_Augmentation_class()
{
    // Currently there is no destructor body generated.
}

// Real-Time Model get method
DC_Augmentation_class::RT_MODEL* DC_Augmentation_class::getRTM()
{
    return (&rtM);
}

//
// File trailer for generated code.
//
// [EOF]
//