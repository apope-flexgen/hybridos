ESS Controller Lab Testing

Author: Ess_Controller Team

Date Created: 
* 02/22/2021

Date Modified:
* 02/22/2021 - reorganized sections and added power limit test case

# Overview
This document addresses the Ess_controler Lab Testing operation
Based on the Lab Test Plan issued .. 02/15/2021 ( add doc ref)

Refer to [insert link to running ess controller doc here] for more details on how to start up the ess controller and
run the web server, web ui, and other modules.

# Configuration
Config files used ( please correct these names)

### ESS Controller
* ess_controller.json
* site_manager_server.json
* ess_schedule.json
* pcs_manager.json
* pcs_pe_template.json
* bms_manager.json
* bms_catl_template.json
* site_manager.json
* gpio_controller.json
* gpio_site_manager.json
* gpio_sched.json

### PCS modbus
* pcsm_client_0114.json (may need to rename in the future)

### BMS modbus
* bms_rtp_lab_test_full_reduced.json (may need to rename in the future)

### Site Controller
* site_manager_server.json
* site_controller_modbus_server_0208_reduced.json
* site_controller_modbus_client_0208.json

# Running the System

## simple command line running

cd git/ess_controller
build/release/ess_controller
build/release/gpio_controller

----

## simple command line running with proper configs

`cd git/ess_controller`
`build/release/ess_controller /usr/local/etc/configs/ess_controller`

`cd git/ess_controller`
`build/release/gpio_controller /usr/local/etc/configs/gpio_controller`

----

## running as a service (sysctl)
----
## Run the ess_controler [insert instructions]
----
## Run the gpio_controler [insert instructions]
----
## Run the Modbus PCS Client [insert instructions]

`/usr/local/bin/modbus_interface/modbus_client ~/git/ess_controller/maps/pcsm_client_0114.json`
        
(release version)  
`/usr/local/bin/modbus_interface/modbus_client /usr/local/etc/config/ess_controller/modbus_interface/pcs_modbus_client.json`

----

## Run the Modbus BMS Client [insert instructions]

`/usr/local/bin/modbus_interface/modbus_client  ~/git/ess_controller/maps/bms_rtp_lab_test_full_reduced.json`

(release version)  
`/usr/local/bin/modbus_interface/modbus_client /usr/local/etc/config/ess_controller/modbus_interface/bms_modbus_client.json`

# Test Cases
## Communications Testing

## check for data published from BMS

`/usr/local/bin/fims/fims_listen -s /components/catl_ems_bms_rw`  
  (add example)

## check for data published from PCS

`/usr/local/bin/fims/fims_listen -s /components/pcsm_general`  
  (add example)

## check for data received  by the ESS from BMS
watch `/usr/local/bin/fims/fims_send -m get -r /$$  -u /ess/components/catl_ems_bms_rw`  
  (add example)

## check for data received by the ESS from PCS
watch `/usr/local/bin/fims/fims_send -m get -r /$$  -u /ess/components/pcsm_general`  
  (add example)

## check for data received by the ESS from GPIO (??)
watch `/usr/local/bin/fims/fims_send -m get -r /$$  -u /ess/components/gpio`  
  (add example)

## Power Limitation
* Description: Checks if power charge/discharge csv file is converted into a two-way data map and if the power degrade value  
can be received based on the given soc and temperature.
* Test file: test/test_parse_csv.cpp

----

## Modbus Data Validation

### BMS data check script
  `git/ess_controller/scripts/lab_test/check_bms_data.sh`

### PCS data check script
  `git/ess_controller/scripts/lab_test/check_pcs_data.sh`

### Gpio data check script  (to do)
  `git/ess_controller/scripts/lab_test/check_gpio_data.sh`

### Verify Commands from Command Line (simulation)
  `git/ess_controller/scripts/StartStop.sh`  
  ( expected output)


#### Start

    ./fims_send -m set -r /me -u /ess/assets/bms/summary/start false
    ./fims_send -m set -r /me -u /ess/assets/bms/summary/start true
    ./fims_send -m get -r /me -u /ess/components/catl_bms_ems_r/bms_status
    expect 1
    
    ./fims_send -m set -r /me -u /ess/assets/pcs/summary/start false
    ./fims_send -m set -r /me -u /ess/assets/pcs/summary/start true
    ./fims_send -m get -r /me -u /ess/components/pcsm_internal_visualization/current_status 
    expect 0


#### Stop

    ./fims_send -m set -r /me -u /ess/assets/pcs/summary/stop false
    ./fims_send -m set -r /me -u /ess/assets/pcs/summary/stop true
    ./fims_send -m get -r /me -u /ess/components/pcsm_internal_visualization/current_status 
    expect 2 ??

    ./fims_send -m set -r /me -u /ess/assets/bms/summary/start false
    ./fims_send -m set -r /me -u /ess/assets/bms/summary/start true
    ./fims_send -m get -r /me -u /ess/components/catl_bms_ems_r/bms_status
    expect 0

### Active Current - charge

 script test_BMS_current.sh

   Check state of setpoint values

The active/reactive current setpoint values should be retrievable in 

    /controls/ess 

    and 

    /components/pcs

   /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActiveCurrentSetpoint
   /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastActiveCurrentSetpoint

   getting /components/pcs:ActiveCurrent
   /usr/local/bin/fims/fims_send -r /$$ -m get -u /components/pcs/ActiveCurrent

   getting /controls/ess:ReactiveCurrentSetpoint
   /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ReactiveCurrentSetpoint
   /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastReactiveCurrentSetpoint

    getting /components/pcs:ReactiveCurrent
    /usr/local/bin/fims/fims_send -r /$$ -m get -u /components/pcs/ReactiveCurrent


    Change active current setpoint in /controls/ess and check state...
    The active current setpoint in /controls/ess will be sent to /components/pcs, indicating a change in current

    setting /controls/ess:ActiveCurrentSetpoint
    /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/ess/ActiveCurrentSetpoint 1000

    setting /variables/ess:lastActiveCurrentSetpoint
    /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/ess/lastActiveCurrentSetpoint 0


    getting /controls/ess:ActiveCurrentSetpoint
   /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActiveCurrentSetpoint
   /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastActiveCurrentSetpoint

    getting /components/pcs:ActiveCurrent
    /usr/local/bin/fims/fims_send -r /$$ -m get -u /components/pcs/ActiveCurrent

### Active Current - discharge

### Active Current - limits

### Reactive Current - charge

### Reactive Current - discharge

### Reactive Current - limits

----

## Check Faults and Alarms

### Ess Controller Faults and Alarms

### CATL SBMU Controller Faults and Alarms

### CATL MBMU Controller Faults and Alarms

### PCS  Alarms

### PCS  Faults

----

## Ess Controller Hardware and UI

### Repeat All Tests with (Ess Controller) UI

### Ess Controller Faults and Alarms in Ess UI

### CATL SBMU Controller Faults and Alarms in Ess UI

### CATL MBMU Controller Faults and Alarms in Ess UI

### PCS  Alarms  in Ess UI

### PCS  Faults in Ess UI

### Check MBMU Data in Ess UI

### Check SBMU Data in Ess UI (Racks #1-#9)

### Check PCS Data in Ess UI

### Check Commands in Ess UI 
#### Start
#### Stop
#### Active Current - charge
#### Active Current - discharge
#### Active Current - limits
#### Reactive Current - charge
#### Reactive Current - discharge
#### Reactive Current - limits

----

## Repeat All Tests with (Site Controller) Command Line

### System Setup

### Ess Controller Faults and Alarms in Site Cl

### CATL SBMU Controller Faults and Alarms in Site Cl

### CATL MBMU Controller Faults and Alarms in Site Cl

### PCS  Alarms  in Site Cl

### PCS  Faults in Site Cl

### Check MBMU Data in Site Cl

### Check SBMU Data in Site Cl (Racks #1-#9)

### Check PCS Data in Site Cl

### Check Commands in Site Cl 

#### Start
#### Stop
#### Active Current - charge
#### Active Current - discharge
#### Active Current - limits
#### Reactive Current - charge
#### Reactive Current - discharge
#### Reactive Current - limits

----

## Repeat All Tests with (Site Controller) UI

### System Setup

### Ess Controller Faults and Alarms in Site UI

### CATL SBMU Controller Faults and Alarms in Site UI

### CATL MBMU Controller Faults and Alarms in Site UI

### PCS  Alarms  in Site UI

### PCS  Faults in Site UI

### Check MBMU Data in Site UI

### Check SBMU Data in Site UI (Racks #1-#9)

### Check PCS Data in Site UI

### Check Commands in Site UI 

#### Start
#### Stop
#### Active Current - charge
#### Active Current - discharge
#### Active Current - limits
#### Reactive Current - charge
#### Reactive Current - discharge
#### Reactive Current - limits

----

## Test Gpio Controller

### Setup

### Sim Tests (no attached hardare)

    Start Listener
    /usr/local/bin/fims/fims_listen  -s /components/gpio

    Turn on Sim
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/config/gpio/GPIOsim 1

    Send Sim Data
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/status/gpio/GPIObits 1
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/status/gpio/GPIObits 2
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/status/gpio/GPIObits 4
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/status/gpio/GPIObits 8
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/status/gpio/GPIObits 16
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/status/gpio/GPIObits 32
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/status/gpio/GPIObits 64
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/status/gpio/GPIObits 0

### Real Hw tests

    Start Listener
    /usr/local/bin/fims/fims_listen  -s /components/gpio

    Turn off sim mode
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/config/gpio/GPIOsim 0

