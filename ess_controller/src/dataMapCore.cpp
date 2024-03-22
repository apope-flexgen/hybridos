
#include "dataMap.h"

extern "C++" {
    int RunThread(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV); 
    int CoreAmapAccess(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV); 
    int HeartbeatTimer(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int ThreadSetup(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);

    // model object setup functions
    // int setupReference(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    // int setupDC_Augmentation(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    // int setupHighLevelController(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    // int setupLowLevelController(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);

}

// called in ess_controller.cpp
// sets up all datamap functions that need to be run on the scheduler
int SetupDatamapSched(scheduler* sched, asset_manager* am)
{
    const char* aname = am->name.c_str();

    am->vm->setFunc(*am->vmap,  aname, "RunThread",             (void*)&RunThread);
    am->vm->setFunc(*am->vmap,  aname, "CoreAmapAccess",        (void*)&CoreAmapAccess);
    am->vm->setFunc(*am->vmap,  aname, "ThreadSetup",           (void*)&ThreadSetup);
    am->vm->setFunc(*am->vmap,  aname, "HeartbeatTimer",        (void*)&HeartbeatTimer);

    // model object setup functions
    // am->vm->setFunc(*am->vmap,  aname, "Reference",             (void*)&setupReference);
    // am->vm->setFunc(*am->vmap,  aname, "DC_Augmentation",       (void*)&setupDC_Augmentation);
    // am->vm->setFunc(*am->vmap,  aname, "HighLevelController",   (void*)&setupHighLevelController);
    // am->vm->setFunc(*am->vmap,  aname, "LowLevelController",    (void*)&setupLowLevelController);

    return 0;
}

// checks our aV for all needed params returns false if we have a bad parameter
bool checkParams(varsmap &vmap, assetVar *aV)
{
    if(!aV->gotParam("debug"))
    {
        aV->setParam("debug", false);
    }
    bool debug = aV->getbParam("debug");

    if (!aV->gotParam("enabled"))
    {
        aV->setParam("enabled", true);
    }
    // "enabled" is a bool
    bool enabled = aV->getbParam("enabled");

    // "enable" is a uri that we get from a string
    bool enable = true;         // default to true
    if (aV->gotParam("enable"))
    {
        if (aV->getcParam("enable") != NULL)
        {
            // if getcParam does not return null, the param is a string that is the uri of another aV
            std::string uri = aV->getcParam("enable");

            assetVar* enableAV = aV->am->vm->getVar(vmap, (char*)uri.c_str(), nullptr);
            if (!enableAV)
            {
                FPS_PRINT_ERROR("Could not find assetVar with \"enable\" param string [{}]. Defaulting enable to true", uri);
            }
            else
            {
                enable = enableAV->getbVal();
            }            
        }
    }
    
    if (enable==false || enabled==false)
    {
        // if either is false, we are disabled
        if (debug) FPS_PRINT_INFO("This assetVar [{}] not enabled", aV->name);
        return false;
    }

    if (!aV->gotParam("threadName"))  // no thread name
    {
        // if no thread name is given, default behavior is to make a new thread
        int num_thread = threadMaps.size() + 1;
        std::string num = std::to_string(num_thread);

        std::string dflt = "Default_Thread_" + num;
        aV->setParam("threadName", (char*)dflt.c_str());

        if (debug) FPS_PRINT_ERROR("{} could not find \"thread_name\" parameter in assetVar [{}]. Using default name [{}]", __func__, aV->name, dflt);
    }
    
    if (!aV->gotParam("datamapName"))  // no datamap name
    {
        // if no given datamap name, use default one
        // default name is "Default_Datamap_X" where X is the number of current datamaps + 1

        int num_datamap = dataMaps.size() + 1;
        std::string num = std::to_string(num_datamap);

        std::string dflt = "Default_Datamap_" + num;
        aV->setParam("datamapName", (char*)dflt.c_str());

        // make new datamap here
        DataMap* dm = dataMaps[dflt];
        if (!dm)
        {
            dm = new DataMap;
        }
        dm->name = dflt;
        dataMaps[dm->name] = dm;

        if (debug) FPS_PRINT_INFO("{} could not find \"datamap_name\" parameter in assetVar [{}]. Creating new dataMap using default name [{}]", __func__, aV->name, dflt);
    }
    else
    {
        std::string dmName = aV->getcParam("datamapName");
        if (!dataMaps[dmName])
        {
            DataMap* dm = new DataMap;
            dm->name = aV->getcParam("datamapName");
            dataMaps[dm->name] = dm;
        }
        
    }

    // used to reload assetVar on startup or when we encounter an error
    if(!aV->gotParam("reload"))
    {
        aV->setParam("reload", FULL_RELOAD);
    }

    // this is the variable that controls our state machine
    if (!aV->gotParam("runStatus"))
    {
        aV->setParam("runStatus", SETUP_CONTEXT);
    }
    
    // this is set true after our AV's function has get, run, and sent data back to amap, and is ready to run again
    if (!aV->gotParam("done"))
    {
        aV->setParam("done", false);
    }

    return true;
}

// rewrite this aV so that functions that have instances are expanded out to additional consecutive functions
void reConfigure(assetVar* aV)
{
    bool debug = aV->getbParam("debug");

    // if this aV has already been reconfigured, do nothing
    if (!aV->gotParam("reconfigured"))
    {
        aV->setParam("reconfigured", false);
    }

    if (aV->getbParam("reconfigured"))
    {
        // if this is true, we have already been configured
        return;
    }

    // put # of instances per function in a vector
    std::vector<int> instances;
    instances.push_back(0);     // the 0th element should have 0 instances

    int num = 0;
    while (++num)
    {
        std::string numStr = std::to_string(num);
        std::string funcX = "func" + numStr;
        if (aV->gotParam((char*)funcX.c_str()))
        {
            // check if we have a "funcX_instances" parameter
            std::string funcX_instances = funcX + "_instances";
            if (aV->gotParam((char*)funcX_instances.c_str()))
            {
                // funcX does have instances, so assign instances.at(X) = the number of instances for funcX
                int num_instances_for_this_func = aV->getiParam((char*)funcX_instances.c_str());
                instances.push_back(num_instances_for_this_func);
            }
            else
            {   
                // if no funcX_instances param exists, funcX only has 1 instance
                instances.push_back(1);
            }
        }
        else
        {
            // we did not have the parameter "funcX" in our aV, leave this while loop
            if (debug) FPS_PRINT_INFO("No {} param found", funcX);
            break; 
        }
    }

    // use a vector of funcNames to get the name of each function
    std::vector<std::string> funcNamesVec;
    funcNamesVec.push_back("0th element - no func name");      // the 0th element should not be used

    // iterate over instance vector
    for (int element = 1; element < (int)instances.size(); element++)
    {
        // need to get the function name of func<element>
        std::string funcX = "func" + std::to_string(element);
        std::string funcName = aV->getcParam((char*)funcX.c_str());

        // for each instance of this function, push this funcName onto the funcNamesVector
        for (int i = 1; i <= instances.at(element); i++)
        {
            // add which instance of the function this is to the end of the func name
            std::string funcNameNum = funcName + "_" + std::to_string(i);

            // insert the func Name into a vector
            funcNamesVec.push_back(funcNameNum);
            
            if (debug) FPS_PRINT_INFO("pushed [{}] to funcNamesVec.at({})", funcNameNum, (int)funcNamesVec.size() - 1);
        }
    }
    // once we get here, the index of funcNamesVec is its function number and the value string is the function name

    // loop over the funcNamesVector
    for (int x = 1; x < (int)funcNamesVec.size(); x++)
    {
        // get function number and function name
        std::string funcX = "func" + std::to_string(x);
        std::string funcName = funcNamesVec.at(x);    // this is the string from the funcNamesVec. it is the function name

        // assign funcX parameter to have value funcName
        aV->setParam((char*)funcX.c_str(), (char*)funcName.c_str());

        if (debug) FPS_PRINT_INFO(" editing aV [{}]'s parameter [{}] to be [{}]", aV->name, funcX, funcNamesVec.at(x));
    }
    
    aV->setParam("reconfigured", true);
}

// add aV function info to threadAV and create aV/int pair on thread class
void addAVtoThread(assetVar* aV, ess_thread* ess, assetVar* threadAV)
{
    bool debug = aV->getbParam("debug");
    std::string dmName(aV->getcParam("datamapName"));

    // check if we have already reloaded and context exists in thread class
    int numberForThisAV;
    if (ess->contexts.find(aV->name) != ess->contexts.end())
    {
        // if we find our aV in contexts, get that aV's number
        // .second is an int, the second element of the aV/int pair
        numberForThisAV = ess->contexts[aV->name].second;
    }
    else
    {
        // make new number for this aV
        numberForThisAV = ess->contexts.size() + 1;
    }

    // create parameter strings to add to thread AV
    std::string num = std::to_string(numberForThisAV);
    std::string funcNum = "func" + num;
    std::string funcNumDatamap = funcNum + "datamap";
    std::string funcNumRunning = funcNum + "running";
    
    // set our threadAV params
    threadAV->setParam((char*)funcNumDatamap.c_str(), (char*)dmName.c_str());
    threadAV->setParam((char*)funcNumRunning.c_str(), true);
    
    // add the names of all our functions to the thread aV
    if (!aV->gotParam("func2"))
    {
        // there is only one function on this aV
        std::string funcNumName = aV->getcParam("func1");
        threadAV->setParam((char*)funcNum.c_str(), (char*)funcNumName.c_str());

        // we only have 1 function on aV, so it will always be the active function
        aV->setParam("numberOfFunctions", 1);
        aV->setParam("runningFunction", 1);
    }
    else
    {
        // this means we have multiple functions on this aV, must add all of them

        // loop over each "funcX" param and add it to threadAV
        int num = 0;
        while (++num)
        {
            // check if we have a funcX param on our aV
            std::string numStr = std::to_string(num);
            std::string funcX = "func" + numStr;
            if (!aV->gotParam((char*)funcX.c_str()))
            {
                // we have set up all the funcs listed on our av and now we have found a func# that doesnt exist, break out of loop
                if (debug) FPS_PRINT_INFO("Could not find [{}] param. Added all func params to threadAV", funcX);

                // we have num - 1 # of functions on this aV
                aV->setParam("numberOfFunctions", num-1);
                aV->setParam("runningFunction", 1);
                break;
            } 

            // get the functions actual name
            std::string funcNumName = aV->getcParam((char*)funcX.c_str());

            // get the aV number and func number for this function
            std::string funcGroupNum = funcNum + "_" + numStr;

            // add it to thread AV
            threadAV->setParam((char*)funcGroupNum.c_str(), (char*)funcNumName.c_str());

            if (debug) FPS_PRINT_INFO("Adding \"{}: {}\" as a param on thread AV", funcGroupNum, funcNumName);
        }
    }

    // store this assetVar in our threads list of all assetVars and given it an assigned number
    ess->contexts[aV->name] = std::make_pair(aV, numberForThisAV);
}

// if RunThread runs successfully, we decrement this aV's overrun counter by a configurable value (default 1)
void decOverrun(assetVar* aV, assetVar* threadAV)
{
    if (!aV->gotParam("decrementOverrun"))
    {
        aV->setParam("decrementOverrun", threadAV->getiParam("decrementOverrun"));
    }
    int dec = aV->getiParam("decrementOverrun");

    int counter = aV->getiParam("overrunCounter") - dec;
    if (counter < 0)
    {
        aV->setParam("overrunCounter", 0);
    }
    else
    {
        aV->setParam("overrunCounter", counter);
    }
}

// set the transfer AV so that it can be access from anywhere and it can talk to its parent
void setTransferAV(assetVar* aV, assetVar* transferAV, std::string comp)
{
    bool debug = aV->getbParam("debug");

    std::string dmName(aV->getcParam("datamapName"));

    // set all initial fields of new av
    transferAV->name = dmName + "_transfer";
    transferAV->setParam("datamapName", (char*)dmName.c_str());

    std::string uri = aV->comp + ":" + aV->name;
    transferAV->setParam("parentAV", (char*)uri.c_str());

    transferAV->am = aV->am;

    transferAV->setParam("debug", debug);

    // put the transfer AV we need for sendToAmap in our asset managers amap
    aV->am->amap[dmName + "_transfer"] = transferAV;
    if (debug) FPS_PRINT_INFO("adding aV [{}] to asset manager [{}]'s amap with key [{}_transfer]", transferAV->name, aV->am->name, dmName);

    // tell this aV (our "parentAV") that we have a transfer AV now
    aV->setParam("transfer_AV", (char*)comp.c_str());
}

// count and return the number of iterations we have been in SETUP_CHECK, return failure if we have exceeded our setupTimeLimit (set in templateAV)
int limitSetupTime(assetVar* aV, assetVar* threadAV)
{
    bool debug = aV->getbParam("debug");

    // create a counter variable to make sure that we get set up on time
    if (!aV->gotParam("checkSetupCounter"))
    {
        aV->setParam("checkSetupCounter", 0);
    }
    int check_setup_iteration = aV->getiParam("checkSetupCounter");

    // get the limit to our setup counter
    if (!aV->gotParam("checkSetupLimit"))
    {
        // set a parameter for a setup timer
        if (!aV->gotParam("setupTimeLimit"))
        {
            // if this aV does not have a setupTimeLimit parameter, default to the value on our threadAV (gotten from TEMPLATE AV)
            aV->setParam("setupTimeLimit", threadAV->getdParam("setupTimeLimit"));
        }
        double setupTimeLimit = aV->getdParam("setupTimeLimit");
        double every = aV->getdParam("repTime");

        if (every == 0) every = 1;      // if somehow our repTime is 0, we dont want to divide by 0        

        if (debug) FPS_PRINT_INFO("    setupTimeLimit is [{}] and every is [{}]", setupTimeLimit, every);

        // convert setupTimeLimit (in seconds) to # of iterations of RunThread by dividing our setupTimeLimit by the number of times we run per second (repTime). round up if there is a remainder
        double iteration_limit = std::ceil(setupTimeLimit / every);
        aV->setParam("checkSetupLimit", iteration_limit);

        if (debug) FPS_PRINT_INFO("SETUP_CHECK will check {} times before signaling an error", iteration_limit);
    }
    int check_setup_limit = aV->getiParam("checkSetupLimit");

    if (check_setup_iteration > check_setup_limit)  // check_setup_limit is configurable in the templateAV from the setupTimeLimit
    {
        // if we check set up X amount of times and it still isnt set up, signal an error
        double setupTimeLimit = aV->getdParam("setupTimeLimit");
        std::string errorMsg = "The time it is taking to set up our functions has exceeded our setupTimeLimit of " + std::to_string(setupTimeLimit) + " seconds. Ensure source files for aV [{}]'s setup functions exist in Makefile and functions are linked in SetupDatamapSched()";
        aV->setParam("errorType", (char*)"fault");
        aV->setParam("errorMsg", (char*)errorMsg.c_str());

        // reset our setup checker
        aV->setParam("checkSetupCounter", 0);

        if (debug) FPS_PRINT_ERROR("We did not get set up in the given limit of {} seconds, signalingerror", setupTimeLimit);

        signalThread(aV, ERROR);
        return EXCEEDED_TIME_LIMIT;
    }

    if (debug) FPS_PRINT_INFO("On iteration [{}]. iteration limit is [{}]", check_setup_iteration, check_setup_limit);

    return check_setup_iteration;
    // "checkSetupCounter" param is increased in the setupDone function
}

// loop over every setupX parameter and return true if its done; return false if not done
bool setupDone(assetVar* aV, int setup_iteration)
{
    bool debug = aV->getbParam("debug");
    int num = 0;
    while (++num)
    {
        // determine which function number we are setting up
        std::string numStr = std::to_string(num);
        std::string setupNum = "setup" + numStr;

        if (aV->gotParam((char*)setupNum.c_str()))
        {
            if (!aV->getbParam((char*)setupNum.c_str()))
            {
                // this setup function is not done yet, we can stop checking setup params and return false
                if (debug) FPS_PRINT_INFO(" [{}] is [{}] on aV [{}]", setupNum, aV->getbParam((char*)setupNum.c_str()), aV->name);
                aV->setParam("checkSetupCounter", setup_iteration + 1);
                return false;
            }
        }
        else
        {
            // every "setupX" we checked returned true and there are no more setupX params, return true
            if (debug) FPS_PRINT_INFO("There is no [{}] param, so we do not need to keep checking for more params", setupNum);
            return true;
        }
    }

    return false;   // we should never get to this return statement
}


// this is the function that manages our thread and all of the datamap contexts its running
int RunThread(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    if(!checkParams(vmap, aV)) return 0;

    // local variables 
    asset_manager *am = aV->am;
	VarMapUtils *vm = am->vm;

    essPerf ePerf(am, aname, __func__);
    bool debug = aV->getbParam("debug");
    bool logging_enabled = getLoggingEnabled(vmap, *vm);
    char* LogDir = getLogDir(vmap, *vm);

    aV->setParam("logging_enabled", logging_enabled);
    aV->setParam("LogDir", LogDir);

    std::string tname(aV->getcParam("threadName"));                             // tname = thread name
    std::string dmName(aV->getcParam("datamapName"));                           // dmName = datamap name
    assetVar *threadAV = getOrMakeThreadAV(vmap, aV->am->vm, tname);

    ess_thread *ess = threadMaps[tname];
    if(ess)
    {
        // setup phase
        int reload = aV->getiParam("reload");
        if (reload < 2)
        {
            if (reload < 1)
            {
                // full reload - needs to do everything partial does and more
                if (debug) FPS_PRINT_INFO("   FULL RELOAD for [{}]", aV->name);

                // rework the aV so that the instances of functions are expanded
                reConfigure(aV);

                // store info from aV in thread class and on thread AV
                addAVtoThread(aV, ess, threadAV);

                // set our heartbeat timeout if we dont have one
                if (!aV->gotParam("heartbeatTimeout"))
                {
                    // use the default value from our templateAV
                    aV->setParam("heartbeatTimeout", threadAV->getdParam("heartbeatTimeout"));
                } 

            }
            
            // partial reload
            if (debug) FPS_PRINT_INFO("   partial RELOAD for [{}]", aV->name);

            // tell our controller we're ready to be set up
            aV->setParam("runStatus", SETUP_CONTEXT);
            aV->setParam("setup", SETUP_READY);

            // reset our controller to the setup state
            aV->setParam("done", false);

            aV->setParam("error", false);

            aV->setParam("reload", 2);

            aV->setParam("overrunCounter", 0);

            aV->setParam("fault", false);
            aV->setParam("alarm", false);
        }

        // check if our thread has told us that we are done
        // dont set runStatus if our error flag is high (error flag is set by the fault handling state)
        if (aV->getbParam("done") && !aV->getbParam("error"))
        {
            aV->setParam("runStatus", RUN_CONTEXT);
            aV->setParam("done", false);
            if (debug) FPS_PRINT_INFO("function(s) for aV [{}] have finished running, ready to run again", aV->name);
        }

        // thread controller 
        ess->core = aV->getiParam("runStatus");

        // run our context and dec overrun
        if(ess->core == RUN_CONTEXT)        // 12
        {
            // decrement our overrun counter if needed
            decOverrun(aV, threadAV);

            // signal our thread to run all the functions on our assetVar
            aV->setParam("runStatus", WAITING_FOR_DONE);
            signalThread(aV, GET);
            if (debug) FPS_PRINT_INFO("signaling thread [{}] to start run sequence (GET) for datamap [{}]", tname, dmName);
        }

        // flag setup function to create datamap and amap
        if(ess->core == SETUP_CONTEXT)      // 11
        {
            // SETUP_CONTEXT creates an assetVar, uses that to run our setup function, then stays in SETUP_CHECK until all setup functions are done

            if (!aV->gotParam("setup"))
            {
                aV->setParam("setup", SETUP_READY);
            }
            int setup_state = aV->getiParam("setup");   // controls what setup state we are in; set by partial reload

            // create our transfer AV and send this assetVar to the thread so it can set us up
            if (setup_state == SETUP_READY)
            {
                // make new transfer av that will be used for running our setup and datamap amap access functions on the core scheduler
                double dval = 0.0;
                std::string comp = "/control/transfer:" + dmName;
                assetVar *transferAV = vm->getVar(vmap, (char*)comp.c_str(), nullptr);
                if (!transferAV)
                {
                    // if this transferAV doesnt exist already, make it
                    transferAV = vm->makeVar(vmap, comp.c_str(), nullptr, dval);
                }

                // set all the fields our transferAV needs
                setTransferAV(aV, transferAV, comp);
                if (debug) FPS_PRINT_INFO("     got transfer AV [{}] in aV [{}] setup", transferAV->name, aV->name);

                // tell our thread to set up all the functions on this aV
                signalThread(aV, SETUP);

                // put this aV in setup STANDBY mode to wait until all setup params have been made and all its functions have been scheduled
                aV->setParam("setup", STANDBY); 

                // setup is set to SETUP_CHECK after all the setup functions are kicked off by the thread
            }
            
            if (setup_state == SETUP_CHECK)
            {
                // use a timer to ensure setup works within given time
                int setup_iteration = limitSetupTime(aV, threadAV);
                if (setup_iteration == EXCEEDED_TIME_LIMIT) return 0;       // error msg and flagging is done in limitSetupTime
                
                // check if each setup parameter is done
                if (!setupDone(aV, setup_iteration)) return 0;              // if setup is not done, return and wait for next iteration   

                // if we get here it is because setupDone returned true
                if (debug) FPS_PRINT_INFO("Set up of aV [{}] is done, setting \"runStatus\" to RUN_CONTEXT", aV->name);
                aV->setParam("runStatus", RUN_CONTEXT);

                // enable our heartbeat
                aV->setParam("heartbeatEnable", true);

                // reset our setup checker
                aV->setParam("checkSetupCounter", 0);
            }

            if (setup_state == STANDBY)
            {
                // if we get into this standby state, start our timer by counting each iteration we are in this state and incrementing "checkSetupCounter"
                int setup_iteration = limitSetupTime(aV, threadAV);
                if (setup_iteration == EXCEEDED_TIME_LIMIT) return 0;       // error msg and flagging is done in limitSetupTime

                if (debug) FPS_PRINT_INFO("In STANDBY setup state on iteration [{}]", setup_iteration);

                // we dont need to call setupDone because the setup functions have not even been scheduled yet
                // just inc our counter and return
                aV->setParam("checkSetupCounter", setup_iteration + 1);
                return 0;

                // the thread will change our state to SETUP_CHECK if everything is working properly and the counter will not reset because the same params/functions are used in SETUP_CHECK
            }
            
        }

        // handle the error we got
        if(ess->core == ERROR_ON_CONTEXT)   // 19
        {        
            // link our current amap to the /config/ess FaultDestination and AlarmDestination
            double dval = 0;
            linkVals(*vm, vmap, am->amap, "ess", "/config", dval, "FaultDestination", "AlarmDestination");

            // determine if fault or alarm
            if (!aV->gotParam("errorType"))
            {
                aV->setParam("errorType", (char*)"alarm");
            }
            std::string errorType = aV->getcParam("errorType");
            
            bool fault;
            if (errorType == "fault")
            {
                fault = true;
            }
            else if (errorType == "alarm")
            {
                fault = false;
            }
            else
            {
                FPS_PRINT_ERROR("Error was triggered on aV [{}] with unknown error type [{}]. Setting error type to fault", aV->name, errorType);
                fault = true;
            }

            // Default fault/alarm destinations 
            char* fltDest = (char*)"/faults/ess";
            char* alrmDest = (char*)"/alarms/ess";

            if (!am->amap["FaultDestination"]->getcVal())
                am->amap["FaultDestination"]->setVal(fltDest);
            if (!am->amap["AlarmDestination"]->getcVal())
                am->amap["AlarmDestination"]->setVal(alrmDest);

            // get destination based on if fault or alarm
            char* faultOrAlarm = fault ? am->amap["FaultDestination"]->getcVal() : am->amap["AlarmDestination"]->getcVal();

            // add assetVar to destination URI
            const std::string destUri = fmt::format("{}:dataMapError", faultOrAlarm);

            // get the error message
            if (!aV->gotParam("errorMsg"))
            {
                FPS_PRINT_ERROR("In Error state and do not have an errorMsg.");
                aV->setParam("errorMsg", (char*)"No error message found");
            }
            std::string msg = aV->getcParam("errorMsg");       
            
            // add assetVar where the error occured to errorMsg
            std::string errMsg = fmt::format("aV [{}] got error message: {}", aV->name, msg);
            

            // Send alarm info to UI
            if (debug) FPS_PRINT_INFO(" aV [{}] is sending an error to [{}] with message [{}]", aV->name, destUri, errMsg);
            vm->sendAlarm(vmap, aV, destUri.c_str(), nullptr, errMsg.c_str(), SEVERITY);        // eventually, have severity variable

            if (fault)
            {
                // after getting an error, stall our aV
                FPS_PRINT_ERROR("Stalling aV [{}] because of error: [{}]", aV->name, msg);
                aV->setParam("runStatus", STALL);
                aV->setParam("error", true);                // set this param so that RunThread doesnt run anymore functions after we've faulted (this param is checked in the "done" block)
                aV->setParam("fault", true);

                // disable our heartbeat timer
                aV->setParam("heartbeatEnable", false);

                // tell our threadAV that we have stopped
                int funcNum = ess->contexts[aV->name].second;
                std::string funcNumRunning = "func" + std::to_string(funcNum) + "running";
                threadAV->setParam((char*)funcNumRunning.c_str(), false);
                if (debug) FPS_PRINT_INFO("Setting threadAV [{}] param [{}] to false", threadAV->name, funcNumRunning);
            }
            else
            {
                FPS_PRINT_WARN("Alarm triggered on aV [{}] with errorMsg: [{}]", aV->name, errMsg);
                aV->setParam("alarm", true);

                if (!threadAV->gotParam("AlarmsOn"))
                {
                    threadAV->setParam("AlarmsOn", (char*)aV->name.c_str());
                }
                else
                {
                    std::string alarmsOn = threadAV->getcParam("AlarmsOn");

                    if (alarmsOn.find(aV->name) == std::string::npos)
                    {
                        // this means that the substring aV->name is not already contained within alarmsOn. add aV->name to this string
                        alarmsOn = alarmsOn + ", " + aV->name;
                        threadAV->setParam("AlarmsOn", (char*)alarmsOn.c_str());
                    }
                    
                    // if aV->name is already in our alarmsOn string, we dont want to add it again and spam the param
                }
                
                // we want to trigger our aV to run again so we dont get a 1 cycle delay
                aV->setParam("error", false);                                               // clear this flag that was set by the thread so we can run again
                aV->setParam("runStatus", WAITING_FOR_DONE);
                signalThread(aV, GET);
                if (debug) FPS_PRINT_INFO("In ERROR_ON_CONTEXT alarm state and signaling thread [{}] to start run sequence (GET) for datamap [{}]", tname, dmName);

            }
            
            // clear error type and message
            aV->setParam("errorMsg", (char*)"[none]");
            aV->setParam("errorType", (char*)"[none]");
        }

        // check if we are overran
        if(ess->core == WAITING_FOR_DONE)   // 10
        {
            FPS_PRINT_WARN("{} tried to run function(s) on assetVar [{}] but previous run has not completed", __func__, aV->name);

            if (!aV->gotParam("incrementOverrun"))
            {
                aV->setParam("incrementOverrun", threadAV->getiParam("incrementOverrun"));
            }
            if (!aV->gotParam("overrunLimit"))
            {
                aV->setParam("overrunLimit", threadAV->getiParam("overrunLimit"));
            }

            int inc = aV->getiParam("incrementOverrun");            // default value of 5
            int counter = aV->getiParam("overrunCounter") + inc;

            int limit = aV->getiParam("overrunLimit");
            
            if (counter > limit)     // capacity is set by threadAV template
            {
                FPS_PRINT_ERROR("Exceeded overrun error limit. Setting an error to this aV [{}]", aV->name);

                std::string errorMsg = "overrunCounter has exceeded our overrunLimit of " + std::to_string(limit);
                aV->setParam("errorType", (char*)"fault");
                aV->setParam("errorMsg", (char*)errorMsg.c_str());

                signalThread(aV, ERROR);
            }
            else
            {
                // set an alarm for overrunning but we're still under our limit
                FPS_PRINT_WARN("AV [{}] has overrun (counter = [{}]) but we arent over our limit of [{}]", aV->name, counter, limit);

                std::string errorMsg = fmt::format("AV [{}] has overrun (counter = [{}]) but we arent over our limit of [{}]", aV->name, counter, limit);
                aV->setParam("errorType", (char*)"alarm");
                aV->setParam("errorMsg", (char*)errorMsg.c_str());

                signalThread(aV, ERROR);
            }
            

            aV->setParam("overrunCounter", counter);

            ESSLogger::get().critical("Datamap [{}] function has overrun on assetVar [{}]", aV->getcParam("datamapName"), aV->name);
            if (logging_enabled)
            {
                std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "datamap_errors", "txt");
                ESSLogger::get().logIt(dirAndFile);
            }
        }        

    }
    else 
    {
        // we did not find this thread name in list of all threads, starting a new thread
        if (debug) FPS_PRINT_INFO("Calling startThread with thread name: [{}]", tname, tname);
        startThread(aV, vmap, (char*)tname.c_str());
    }

    return 0;
}

// scheduler function that runs getFromAmap and sendToAmap for its aV
int CoreAmapAccess(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    asset_manager *am = aV->am;
	VarMapUtils *vm = am->vm;

    if (!aV->gotParam("debug"))
    {
        aV->setParam("debug", false);
    }
    int debug = aV->getbParam("debug");

    bool logging_enabled = getLoggingEnabled(vmap, *aV->am->vm);
    char* LogDir = getLogDir(vmap, *aV->am->vm);

    if (debug) FPS_PRINT_INFO(" running {} using assetVar [{}]  at time [{}]", __func__, aV->name, vm->get_time_dbl());

    std::string dmName = "ERROR";           // default to this name in case we dont get a datamap name and throw an error to our catch block
    std::string op = "ERROR";

    // if we cant find our parent AV, time out
    if (!aV->gotParam("parentAV"))     
    {
        FPS_PRINT_ERROR("assetVar [{}] does not have a \"parent_AV\" parameter and we cannot send a signal back to the thread. Timing out", aV->name);
        return 0;
    }
    std::string parent_uri = aV->getcParam("parentAV");
    
    assetVar *parent_AV = vm->getVar(vmap, (char*)parent_uri.c_str(), nullptr);
    if (!parent_AV)
    {
        FPS_PRINT_ERROR("Could not find assetVar with comp [{}]. This assetVar ({}) has no parent and cannot signal back to the thread. Timing out", parent_uri, aV->name);
        return 0;
    }
    if (debug) FPS_PRINT_INFO("Got parent_AV [{}]", parent_uri);

    // GET or SEND
    try
    {
        if (!aV->gotParam("datamapName"))
        {
            FPS_PRINT_ERROR("In {} function and assetVar [{}] did not properly set a datamap name parameter", __func__, parent_uri);
            std::string nameError = "transferAV for [" + parent_uri + "] has no datamap name parameter";
            throw std::invalid_argument(nameError);
        }
        dmName = aV->getcParam("datamapName");

        if (!aV->gotParam("function"))
        {
            FPS_PRINT_ERROR("No \"function\" parameter was set by aV [{}] in scheduleDMfunc()", parent_uri);
            std::string paramError = "No 'function' parameter set to this aV by " + parent_uri;
            throw std::invalid_argument(paramError);
        }
        std::string funcName_i = aV->getcParam("function");

        // get the function name
        size_t underscore_index = funcName_i.find_last_of('_');                  // gets the index of the last '_' -> the '_' separates the func name and the instance
        std::string funcName = funcName_i.substr(0, underscore_index);

        // get the instance from the func name
        std::string instanceNum = funcName_i.substr(underscore_index + 1);
        int instance = std::stoi(instanceNum);
        

        // get the run function pointer from our global map
        DataMap *dm = dataMaps[dmName];
        if (!dm)
        {
            // we could not find that datamap name in our global map of all datamaps, signal error
            FPS_PRINT_ERROR("Datamap name [{}] not found in global list of dataMaps", dmName);
            std::string nameError = "Could not find datamap with name  [" + dmName + "]";
            throw std::invalid_argument(nameError);
        }

        // get the amap specific to this assetVar and instance
        std::string amname = parent_uri + "_" + funcName_i + "_asset_manager";
        am = vm->getaM(vmap, amname.c_str());
        if (!am)
        {
            // this asset manager should be created in setup function
            FPS_PRINT_ERROR("Could not find asset manager [{}], we do not have access to an amap. Ensure setup function for {} creates an asset manager ", amname, funcName);
            std::string amError = "Asset manager [" + amname +"] does not exist.";
            throw std::invalid_argument(amError);
        }
        
        // are we getting from or sending to the amap?
        if (!aV->gotParam("operation"))
        {
            FPS_PRINT_ERROR("In {} function and assetVar [{}] never set an operation parameter; it does not know whether to get from or send to amap", __func__, parent_uri);
            std::string opError = "transferAV for [" + parent_uri + "] has no operation parameter";
            throw std::invalid_argument(opError);
        }
        op = aV->getcParam("operation");

        if (op == "get")
        {
            std::string getInputsKey = funcName + "Inputs";

            // use global map to get reference to model input block
            void *inputRef = modelFcnRef[getInputsKey];
            if (!inputRef)
            {
                // our datamap name does not have the associated ModelObjectInputs
                FPS_PRINT_ERROR("Datamap Inputs [{}] does not exist in global map of model object inputs. Check [{}] setup file", getInputsKey, funcName);
                std::string keyError = "Could not find [" + getInputsKey + "] in global map of model object inputs";
                throw std::invalid_argument(keyError);
            }

            // cast inputRef to a function pointer
            uint8_t* (*getFuncInputs)(std::string, int) = reinterpret_cast<uint8_t* (*)(std::string, int)>(inputRef);

            // run the function that returns this models inputs
            uint8_t* modelInputs = getFuncInputs(parent_uri, instance);

            // get the transferBlock for this specific instance
            std::string inputBlock = funcName_i + "Inputs";

            // run getFromAmap on this datamap
            dm->getFromAmap(inputBlock, am, modelInputs);
            if (debug) FPS_PRINT_INFO("    just ran dm->getFromAmap using datamap [{}] and am [{}]", dm->name, am->name); 

            signalThread(parent_AV, RUN);

        }
        else if (op == "send")
        {
            std::string getOutputsKey = funcName + "Outputs";

            // use map to get dmReferenceOutput
            void *outputRef = modelFcnRef[getOutputsKey];
            if (!outputRef)
            {
                // our datamap name does not have the associated ModelObjectOutputs
                FPS_PRINT_ERROR("Datamap Outputs [{}] does not exist in global map of model object outputs. Check [{}] setup file", getOutputsKey, funcName);
                std::string keyError = "Could not find [" + getOutputsKey + "] in global map of model object outputs";
                throw std::invalid_argument(keyError);
            }

            // cast outputRef to a function pointer
            uint8_t* (*getFuncOutputs)(std::string, int) = reinterpret_cast<uint8_t* (*)(std::string, int)>(outputRef);

            // run the function that returns this models outputs
            uint8_t* modelOutputs = getFuncOutputs(parent_uri, instance);

            // get our transferBlock for this specific instance
            std::string outputBlock = funcName_i + "Outputs";

            // run send to amap using this asset manager and datamap
            dm->sendToAmap(vmap, outputBlock, am, modelOutputs);
            if (debug) FPS_PRINT_INFO("    just ran dm->sendToAmap using datamap [{}] and am [{}]\n", dm->name, am->name);

            signalThread(parent_AV, DONE);

        }
        else
        {
            // unknown operation
            FPS_PRINT_ERROR("In {} function and assetVar [{}] triggered an unknown operation parameter [{}]", __func__, parent_uri, op);
            std::string opError = "AssetVar [" + parent_uri + "] does not know what to do with operation [" + op + "]";
            throw std::invalid_argument(opError);
        }
        
    }
    catch(const std::exception& e)
    {
        if (debug) FPS_PRINT_ERROR("Tried to run {} using operation [{}] and datamap [{}] model object and failed: {}", __func__, op, dmName, e.what());

        std::string errorMsg = e.what();
        parent_AV->setParam("errorType", (char*)"fault");
        parent_AV->setParam("errorMsg", (char*)errorMsg.c_str());

        ESSLogger::get().critical("While trying to run {} with datamap [{}] and assetVar [{}], we got this error: [{}] ", __func__, dmName, aV->name, e.what());
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "datamap_errors", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }
        
        signalThread(parent_AV, ERROR);
    }

    return 0;
}


// Heartbeat timer for our threads
int HeartbeatTimer(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* heartbeatAV)
{
    bool debug = false;
    if (heartbeatAV->gotParam("debug"))
    {
        debug = heartbeatAV->getbParam("debug");
    }
    
    Heartbeat& heart = Heartbeat::getInstance();

    // loop over each thread
    for (ess_thread* thread : heart.threads)
    {
        // loop over each aV on each thread
        for (auto& str_aV_pair : thread->contexts)
        {
            // .second is the aV/int pair; .first on that pair gives us the aV
            assetVar* aV = str_aV_pair.second.first;

            if (aV->gotParam("heartbeatEnable"))
            {
                // if we are watching this aV
                if (aV->getbParam("heartbeatEnable"))
                {
                    // speed up how often HeartbeatTimer runs if necessary
                    double timeout = aV->getdParam("heartbeatTimeout");
                    double current_heartbeat_timeout = heartbeatAV->getdParam("repTime");

                    if (current_heartbeat_timeout > (timeout/2))    // smaller means faster
                    {
                        // our heartbeat timer should run twice as fast as the smallest heartbeat timeout
                        // if we get into this if statement, that means the heartbeat timeout of this aV is faster than 2*our current heartbeat timeout
                        // set this as a warning but do not increase the speed at which this function runs

                        FPS_PRINT_INFO("Heartbeat timeout of aV [{}] is [{}] and our heartbeatTimer is currently running every [{}] seconds. HeartbeatTimer should run twice as fast as the fastest heartbeat timeout. Setting a warning to this AV [{}]", aV->name, timeout, current_heartbeat_timeout, aV->name);

                        // signal an error
                        std::string errorMsg = fmt::format("HeartbeatTimer repTime is [{}] and heartbeat timeout of aV [{}] is [{}]. HeartbeatTimer should run twice as fast as the fastest heartbeat timeout", current_heartbeat_timeout, aV->name, timeout);
                        aV->setParam("errorType", (char*)"alarm");
                        aV->setParam("errorMsg", (char*)errorMsg.c_str());

                        signalThread(aV, ERROR);

                    }

                    // if we have last_heartbeat param, check it
                    if (aV->gotParam("lastHeartbeat"))
                    {
                        double lastResetTime = aV->getdParam("lastHeartbeat");
                        double tNow = aV->am->vm->get_time_dbl();

                        if (tNow - lastResetTime > timeout)
                        {
                            if (debug) FPS_PRINT_INFO(" aV [{}]: last heartbeat was [{:.3f}] and it is now [{:.3f}]. we are [{:.3f}]s over our timeout of [{}]", aV->name, lastResetTime, tNow, tNow-lastResetTime, timeout);
                            
                            // signal an error
                            std::string errorMsg = fmt::format("It has been [{:.3f}]s since aV [{}]'s last heartbeat which is longer than its heartbeatTimeout of [{}]", tNow-lastResetTime, aV->name, timeout);
                            aV->setParam("errorType", (char*)"fault");
                            aV->setParam("errorMsg", (char*)errorMsg.c_str());

                            bool logging_enabled = aV->getbParam("logging_enabled");
                            char* LogDir = aV->getcParam("LogDir");

                            ESSLogger::get().critical("While HeartbeatTimer was running, aV [{}] got this error: [{}] ", aV->name, errorMsg);
                            if (logging_enabled)
                            {
                                std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "datamap_errors", "txt");
                                ESSLogger::get().logIt(dirAndFile);
                            }

                            signalThread(aV, ERROR);
                        }
                        else
                        {
                            // display the last heartbeat time for each function group
                            std::string fg = "_FuncGroup_";
                            std::string param = thread->name + fg + std::to_string(str_aV_pair.second.second);      // convert the int assoc w the aV to a string

                            heartbeatAV->setParam((char*)param.c_str(), lastResetTime);
                            if (debug) FPS_PRINT_INFO("Setting \"{}\" to its last heartbeat value of [{}]", param, lastResetTime);
                        }
                    }
                }
            }
        }
    } 

    return 0;
}
