#ifndef _STATE_HPP
#define _STATE_HPP
/*
 * state.h
 */

/*
 * the system state module is a key system component
 * states have a process to monitor and control
 * that process can have many smaller substates.
 * each state will perform measurements on data , produce alerts and warnings if
 * needed states will also have transitions to other states. these trasitions
 * take the form of delivering commands and monitoring system responses to those
 * commnds. a state will have a command input lets take the "power up" state as
 * an example. the system powers up, configures a default environment. starts
 * communications with other systems. issues start up commands to its
 * subsystems. runs data monitoring functions and transitions to either a
 * ready state or a failed state.
 *
 *  BMS in more detail
 *      load config ( load stored config)
 *      monitor connection ( wait for comms/ watchdogs)
 *      monitor states for subsystems. (hot start perhaps)
 *      enter hot start state.
 *      start up any substate monitors
 *      monitor commands to see if we need to change state
 *      monitor processes to see if we need to transition ( fault / estop etc)
 *      the system will run a number of functions on incoming data.
 *      over/under  current / temp /voltage
 *      any alert or alarm will cause the system to issue warnings sound alarms
 * and even maybe shutdown. hybridOS determined about 5 states STOP STANDBY
 * START RUN1 RUN2 ESTOP lets consider some more RESET POWERUP and FAULT
 *      The HYBRIDOS model of Sequences/ paths and steps look like a good one so
 * we'll try and follow that.
 *
 *
 */
/*
   Some design thoughts
   Lets give  the controller "requests"
   ie "please go to standby"
   A request first evaluates the current system state.
   "no coms == no go"
   A number of data points are evaluated with limits + time slots
   " temp must be betwwen 25 and 45 within 10 seconds."
   Th request can then proced to the first or next step  ( in fact the iniital
conditions are the first step) A step will set up some commands and evaluate
results within a time constraint. Warnings alarms and other errors are logged.
" warn : Its a little bit warm in here"
" alarm: Its too cold to go any futher"
" logging ; value xxy in target range in zz seconds"
So a typical command may be "open dc contactor"
We then  monitor the dc contactor state and consider the step complete when
done. "commands" "request":"standby" "fromstates":["init","stopped","standby"]
"prereq":"grid_volts", "greaterthan":900, "lessthan":1000,"time":5.0
"prereq":"grid_volts", "greaterthan":900, "lessthan":1000,"time":5.0
"step1":"opendc"
     "cmd":"open dc contactor" :"true"
                monitor "dc_contactor_state":"open" time"5.0 secs"
"step2":"openac"
     "cmd":"open ac contactor" :"true"
                monitor "ac_contactor_state":"open" time"5.0 secs"
"stepn":"set state" "standby"


"monitor"
  "allstates" freq 100mS
      monitor "bms_heartbeats" "incrementing" time 1.0 wrap 255  errorstate
"Estop" monitor "pcs_heartbeats" "incrementing" time 1.0 wrap 255  errorstate
"Estop" monitor "bms_mode" "value=OK"  errorstate "Estop" monitor "pcs_mode"
"value=OK"  errorstate "Estop"


  "standby"
        "monitor":[
                {"comp":"/system/components","var":"grid_volts", "min":900,
"max":1000, "onError": Stop"},
                {"comp":"/system/components","var":"dc_volts", "min":900,
"max":1000, "onError": Stop"}
                ]


   A request will issue a request for the system to enter a new state.
*/
/*
recap on how twins does it

discovertree
calculatetree
updatetree

unc discoverTree(node treeNode, input terminal, dt float64) (output terminal) {
        // fmt.Println("Discovering", node.asset.GetID(), input)
        childInput := node.asset.UpdateMode(input)
        collected := childInput
        for i, child := range node.children {
                if i == 0 {
                        collected = terminal{}
                }
                childOutput := discoverTree(child, childInput, dt)
                collected = combineTerminals(collected, childOutput)
        }
        output = node.asset.GetLoadLines(collected, dt)
        return output
}

ess updateMode
this does things with control words like if you are off and you get an on
command. look for set_active_power and stuff like that


transfer commands

processCtrlWordConfig(e, e.CtrlWord1Cfg, e.CtrlWord1)
        processCtrlWordConfig(e, e.CtrlWord2Cfg, e.CtrlWord2)
        processCtrlWordConfig(e, e.CtrlWord3Cfg, e.CtrlWord3)
        processCtrlWordConfig(e, e.CtrlWord4Cfg, e.CtrlWord4)  // get incoming
commands

        // ContactorControl allows discrete control over DC and AC contactors
        // If not set, only Oncmd or Offcmd are needed
        if !e.ContactorControl {
                if !e.On && e.Oncmd {
                        e.AcContactorCloseCmd, e.DcContactorCloseCmd = true,
true } else if e.On && e.Offcmd { e.AcContactorOpenCmd, e.DcContactorOpenCmd =
true, true
                }
        }


    >>toggle accontactor  set open/close command
        if !e.AcContactor && e.AcContactorCloseCmd {
                e.AcContactor, e.AcContactorCloseCmd = true, false
        } else if e.AcContactor && e.AcContactorOpenCmd {
                e.AcContactor, e.AcContactorOpenCmd = false, false
                e.Offcmd = true
        }

    >>toggle dccontactor  set open/close command
    >> set up racks

        if !e.DcContactor && e.DcContactorCloseCmd {
                e.DcContactor, e.DcContactorCloseCmd = true, false
                e.RacksInService = e.Racks
        } else if e.DcContactor && e.DcContactorOpenCmd {
                e.DcContactor, e.DcContactorOpenCmd = false, false
                e.RacksInService = 0
                e.Offcmd = true

        }

    >> follow gridform cmd
        if e.GridFormingCmd {
                e.GridForming, e.GridFormingCmd = true, false
        } else if e.GridFollowingCmd && e.GridForming {
                e.GridForming, e.GridFollowingCmd = false, false
        }

        // Turn on if conditions allow it
        if e.Oncmd && (!e.On || e.Standby) && e.AcContactor && e.DcContactor {
                e.On = true
                e.Oncmd = false
                e.Standby = false
        } else if e.On && e.Offcmd {
                e.On = false
                e.Offcmd = false
                e.Standby = false
        }
        if e.On && e.StandbyCmd {
                e.Standby = true
                e.StandbyCmd = false
        }
    asset interface

        UpdateMode(input terminal) (output terminal)
          see above
            GetLoadLines(input terminal, dt float64) (output terminal)
            DistributeVoltage(input terminal) (output terminal)
            CalculateState(input terminal, dt float64) (output terminal)
            DistributeLoad(input terminal) (output terminal)
            UpdateState(input terminal, dt float64) (output terminal)
            GetID() string
            GetAliases() []string
            Term() terminal
            Init()


        So lete walk through this again.

        We get a command input from the modbus_server
        set active_power_setpoint

this gets into the base namespace.
It is given to all the components to process.  -- update mode
we then run through it all again to do things
we finally update state to report back.

Each system manager will add commands to the asset name space based on the
incoming commands. The assets will provide responses in their namespace the
asset manager will then formulate the plan and pass it down lastly the systems
will provide an update on what they are doing.



example
   charge at 5000 A
   each ess will respond with  soc and charge capacity
   back up to the top and the charge load for each will be calculated based on
these responses. back down to give allowed charge commands to each . They will
respond Finally the last runupdates the state with the results.

   charge @5000 A
      ess1 I can take 1000
      ess2 I can take 2000
      ess3 I can take 4000


    total needed = 7000
    each gets 5000/7000 of their need.

    ess_1 714
    ess_2  1428
    ess_3  2857

    last pass read back status





*/
#endif
