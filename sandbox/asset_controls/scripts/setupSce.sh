#!/bin/bash
#####
# G. Briggs 
# This script sets up necessary inputs to do basic validation of the voltage balancing algorithm 
# on real SCE configurations in ess controller without PSM.
#####
fims_set="/usr/local/bin/fims_send -m set -r /$$ -u"


#Active power setpoint
$fims_set /ess_0/controls/ess_0/ActivePowerSetpoint 2420

#Battery energies. 
$fims_set /ess_1/status/bms/RemainingChargeEnergy 1000
$fims_set /ess_1/status/bms/RemainingDischargeEnergy 1000
$fims_set /ess_2/status/bms/RemainingChargeEnergy 500
$fims_set /ess_2/status/bms/RemainingDischargeEnergy 500

#Battery Voltages
$fims_set /ess_1/status/bms/Voltage 1316
$fims_set /ess_2/status/bms/Voltage 1328

#Battery Currents
$fims_set /ess_1/status/bms/Current 0
$fims_set /ess_2/status/bms/Current 0

#Inverter power limits
$fims_set /ess_1/limits/pcs/RatedActivePowerPct 100
$fims_set /ess_2/limits/pcs/RatedActivePowerPct 100

$fims_set /ess_0/master/ess_1_ls/chargeable_power 2000
$fims_set /ess_0/master/ess_1_ls/dischargeable_power 2000
$fims_set /ess_0/master/ess_2_ls/chargeable_power 2000
$fims_set /ess_0/master/ess_2_ls/dischargeable_power 2000

$fims_set /ess_0/status/ess_0/BMSMaxChargePower 4000
$fims_set /ess_0/status/ess_0/BMSMaxDischargePower 4000

$fims_set /ess_1/limits/ess_1/MaxChargePower -- -1815
$fims_set /ess_1/limits/ess_1/MaxDischargePower 1815
$fims_set /ess_2/limits/ess_2/MaxChargePower -- -1815
$fims_set /ess_2/limits/ess_2/MaxDischargePower 1815



