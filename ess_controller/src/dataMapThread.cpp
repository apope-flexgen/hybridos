
#include "dataMap.h"

// problem: assetVars can be used by the ess at any time, but we expect our assetVars to have particular paramters while we are using them in the thread
// the thread has access to assetVars that might go away in the vmap at any time
// replace the dictionary ref in the main vmap to sever the link
// todo for after mvp: we need to discuss how to handle the sharing of data between the core and the thread on assetVars
// should we isolate assetVars so they cant be altered by other things? should we expand the datamap class to contain the data?


// send this signal to this assetVar
void signalThread(assetVar *targAv, int signal)
{
    if (!targAv->gotParam("debug"))
    {
        targAv->setParam("debug", false);
    }
    bool debug = targAv->getbParam("debug");

    // the params of the aV have already been checked when the aV was first run in RunThread
    std::string name(targAv->getcParam("threadName"));
    if (debug) FPS_PRINT_INFO("   signaling Thread [{}] with aV [{}] (am [{}]) and signal [{}]", name, targAv->name, targAv->am->name, signal);

    ess_thread *ess = threadMaps[name];
    if(ess)
    {
        std::pair<int, assetVar*> avPair(signal, targAv);
        ess->wakeChan.put(avPair);
    }
    else
    {
        FPS_PRINT_ERROR("{} could not find thread with name [{}]. No signal sent", __func__, name);
    }
}

// starts a new thread and gives it a thread assetVar
void startThread(assetVar *aV, varsmap& vmap, char *tname)
{
    if (!aV->gotParam("debug"))
    {
        aV->setParam("debug", false);
    }
    bool debug = aV->getbParam("debug");

    ess_thread *ess = threadMaps[tname];
    if (!ess)
    {
        if (debug) FPS_PRINT_INFO("creating thread [{}]", tname);
        ess = new ess_thread(tname);

        // create a new assetVar as a manager for the thread
        assetVar *tav = getOrMakeThreadAV(vmap, aV->am->vm, std::string(tname));
        ess->threadAV = tav;

        tav->setVal(true);                                  // thread is running, value is true
        ess->running = 1;
        ess->id = std::thread(EssThread, ess);
        
        if (debug) FPS_PRINT_INFO(" thread [{}] started", tname);
        threadMaps[tname]=ess;

        // add this new thread to the Heartbeat singleton
        Heartbeat& heart = Heartbeat::getInstance();
        heart.threads.push_back(ess);

    }
    else 
    {
        FPS_PRINT_INFO(" thread [{}] is already running", tname);
    }
}

// triggers all the setup functions to run for a given aV
void setupFunctions(assetVar* aV)
{
    asset_manager *am = aV->am;
	VarMapUtils *vm = aV->am->vm;

    bool logging_enabled = aV->getbParam("logging_enabled");
    char* LogDir = aV->getcParam("LogDir");
    bool debug = aV->getbParam("debug");

    try
    {
        // use our asset managers amap to get the transfer AV that we need
        std::string dmName = aV->getcParam("datamapName");
        std::string transferName = dmName + "_transfer";
        assetVar *transferAV = vm->getVar(am->amap, (char*)transferName.c_str());
        if (!transferAV)
        {
            FPS_PRINT_ERROR("assetVar [{}] cannot find an assetVar with name [{}] on asset manager [{}] to run our setup functions on. Check RunThread SETUP_STATE", aV->name, transferName, am->name);
            std::string transError = "Could not find transfer aV for [" + aV->name + "]";
            throw std::invalid_argument(transError);
        }

        int num = 0;
        while (++num)
        {
            // determine which function number we are setting up
            std::string numStr = std::to_string(num);
            std::string funcNum = "func" + numStr;
            if (!aV->gotParam((char*)funcNum.c_str()))
            {
                // we have set up all the funcs listed on our av and now we have found a func# that doesnt exist, break out of loop
                if (debug) FPS_PRINT_INFO("Could not find [{}] param. Exiting setup loop", funcNum);
                break;
            } 

            // separate the function name from the full funcNum parameter (set up to be funcName.instance# in reConfigure)
            std::string reconfiguredFunc = aV->getcParam((char*)funcNum.c_str());   // this variable is the function after it has been reConfigured
            std::string setupID = reconfiguredFunc + "_setup";                      // this is only used as an ID for our schedItem

            size_t underscore_index = reconfiguredFunc.find_last_of('_');                  // gets the index of the last '_'  |  the '_' separates the func name and the instance
            std::string setupFunc = reconfiguredFunc.substr(0, underscore_index);          // get just the function name

            // get the instance
            std::string instanceStr = reconfiguredFunc.substr(underscore_index + 1);

            if (debug) FPS_PRINT_INFO("{} got param [\"{}\": \"{}\"] and is setting up [{}] instance [{}]", __func__, funcNum, reconfiguredFunc, setupFunc, reconfiguredFunc.substr(underscore_index + 1));

            // now schedule our setup function with the AV we just made
            auto schItem = new schedItem();

            // set up fields for new schedItem
            char *myid = (char*)setupID.c_str();                                // schedItem ID is the name of the function we are setting up + "_setup"
            char *schedFcn = (char*)setupFunc.c_str();                          // this is our setup function defined in config and linked to the function itself in SetupDatamapSched (at the top of this file)

            std::string comp = "/control/transfer:" + dmName;
            char *aname = (char*)am->name.c_str();                              // uses the same asset manager as this aV ( the datamaps asset manager )
            char *schedUri = (char*)comp.c_str();                               // set to be the same as the schItem assetVar's comp
            char *schedTarg = (char*)comp.c_str();                              // same as our uri
            
            // create new schedItem
            schItem->setUp(myid, aname, schedUri, schedFcn, 0, vm->get_time_dbl(), 0, 0, schedTarg);

            schItem->av = transferAV;                                           // our schItem should run on transferAV
            schItem->av->setParam("runTime", vm->get_time_dbl());               // tell scheduler to run asap

            // use a schItem->av param to initialize our instance setup counter
            std::string funcName_instance = setupFunc + "_instance";
            if (!schItem->av->gotParam((char*)funcName_instance.c_str()))
            {
                schItem->av->setParam((char*)funcName_instance.c_str(), 0);
            }

            if (debug)  // prints new schedItem
            {
                FPS_PRINT_INFO(" Set up new schedItem [{}] to run [{}] on aV [{}]", schItem->id, schItem->func, schItem->av->name);
                schItem->show();
            }

            // put our new schedItem on the scheduler
            auto reqChan = (channel <schedItem*>*)am->reqChan;
            reqChan->put(schItem);

            // create a flag to know when this setup function is done
            std::string setupNum = "setup" + numStr;
            aV->setParam((char*)setupNum.c_str(), false);

            // wake up the scheduler
            if (am->wakeChan)
            {
                am->wakeChan->put(0);
            }
        }

        // tell RunThread to start checking if all setup functions are done
        aV->setParam("setup", SETUP_CHECK);
    }
    catch(const std::exception& e)
    {
        if (debug) FPS_PRINT_ERROR("Tried to run set up functions for aV [{}] and failed: {}", aV->name, e.what());

        std::string errorMsg = e.what();
        aV->setParam("errorType", (char*)"fault");
        aV->setParam("errorMsg", (char*)errorMsg.c_str());

        ESSLogger::get().critical("While trying to setup the functions on assetVar [{}], we got this error: [{}] ", aV->name, e.what());
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "datamap_errors", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }
        
        signalThread(aV, ERROR);
    }
}

// schedules getFromAmap or sendToAmap to run on the core thread
void scheduleDMfunc(assetVar *aV, std::string operation)
{
    asset_manager *am = aV->am;
	VarMapUtils *vm = aV->am->vm;

    bool logging_enabled = aV->getbParam("logging_enabled");
    char* LogDir = aV->getcParam("LogDir");
    bool debug = aV->getbParam("debug");

    try
    {
        std::string name = aV->getcParam("datamapName");
        DataMap *dm = dataMaps[name];
        if (!dm)
        {
            // we could not find that datamap name in our global map of all datamaps, signal error
            FPS_PRINT_ERROR("Datamap name [{}] not found in global list of dataMaps", name);
            std::string nameError = "Could not find datamap with name  [" + name + "]";
            throw std::invalid_argument(nameError);
        }

        // use our asset managers amap to get the transfer AV that we need
        std::string transferName = name + "_transfer";
        assetVar *transferAV = vm->getVar(am->amap, (char*)transferName.c_str());
        if (!transferAV)
        {
            FPS_PRINT_ERROR("assetVar [{}] cannot find an assetVar with name [{}] on asset manager [{}] to run our setup functions on. Check RunThread SETUP_STATE", aV->name, transferName, am->name);
            std::string transError = "Could not find transfer aV for [" + aV->name + "]";
            throw std::invalid_argument(transError);
        }
        
        if (debug) FPS_PRINT_INFO("   got transferAV [{}] from am [{}]'s amap", transferAV->name, am->name);

        // set transferAV's params
        transferAV->setParam("operation", (char*)operation.c_str());        // tell this AV if we are getting or setting (gotten from function parameter)

        // tell our transferAV which function we are running (used for input/outputBlocks)
        if (!aV->gotParam("runningFunction"))
        {
            // default to running function 1
            aV->setParam("runningFunction", 1);
        }
        int activeFunction = aV->getiParam("runningFunction");
        std::string usingFunc = "func" + std::to_string(activeFunction);

        if (!aV->gotParam((char*)usingFunc.c_str()))
        {
            FPS_PRINT_ERROR("assetVar [{}] does not have \"{}\" parameter. Cannot run function", aV->name, usingFunc);
            std::string funcError = "Could not find [" + usingFunc + "] as a parameter in assetVar " + aV->name;
            throw std::invalid_argument(funcError);
        }
        std::string funcName = aV->getcParam((char*)usingFunc.c_str());      // name of function assoc with our func#
        std::string instanceAM = funcName + "_asset_manager";

        transferAV->setParam("function", (char*)funcName.c_str());

        // set up fields for new schedItem
        std::string compStr = "/control/transfer:" + dm->name;              // this matches the uri used to make our transferAV
        char *myid = (char*)transferAV->name.c_str();                       // schedItem ID is the same as the schedItem assetVar name
        char *aname = (char*)instanceAM.c_str();                            // sets schItem am to the instance's asset manager
        char *schedUri = (char*)compStr.c_str();                            // set to be the same as the schItem assetVar's comp
        char *schedFcn = (char*)"CoreAmapAccess";                           // function is defined in dataMap_core.cpp
        char *schedTarg = (char*)compStr.c_str();                           // same as our uri
        
        // create new schedItem
        auto schItem = new schedItem();
        schItem->setUp(myid, aname, schedUri, schedFcn, 0, vm->get_time_dbl(), 0, 0, schedTarg);

        schItem->av = transferAV;                                           // our schItem should run on transferAV
        schItem->av->setParam("runTime", vm->get_time_dbl());               // tell scheduler to run asap

        if (debug)  // prints new schedItem
        {
            FPS_PRINT_INFO(" Set up new schedItem [{}] to run [{}] on aV [{}]", schItem->id, schItem->func, schItem->av->name);
            schItem->show();
        }

        // put our new schedItem on the scheduler
        auto reqChan = (channel <schedItem*>*)am->reqChan;
        reqChan->put(schItem);
        if (am->wakeChan)
        {
            am->wakeChan->put(0);
        }
        if (debug) FPS_PRINT_INFO(" just scheduled \"CoreAmapAccess\" function to run [{}] on datamap [{}]", operation, name);

    }
    catch(const std::exception& e)
    {
        if (debug) FPS_PRINT_ERROR("Tried to run datamap function [{}] with aV [{}] and failed: {}", operation, aV->name, e.what());

        std::string errorMsg = e.what();
        aV->setParam("errorType", (char*)"fault");
        aV->setParam("errorMsg", (char*)errorMsg.c_str());

        ESSLogger::get().critical("While trying to run the [{}] function on assetVar [{}], we got this error: [{}] ", operation, aV->name, e.what());
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "datamap_errors", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }
        
        signalThread(aV, ERROR);
    }
}

// threaded function that runs this functions model object step function
void stepDMfunc(assetVar* aV)
{
    bool logging_enabled = aV->getbParam("logging_enabled");
    char* LogDir = aV->getcParam("LogDir");
    bool debug = aV->getbParam("debug");

    try
    {
        if (!aV->gotParam("runningFunction"))
        {
            // default to func1 if no runningFunction param
            if (debug) FPS_PRINT_INFO("AV [{}] has no \"runningFunction\" parameter. Defaulting to run func1");
            aV->setParam("runningFunction", 1);
        }
        int funcNum = aV->getiParam("runningFunction");
        std::string usingFunc = "func" + std::to_string(funcNum);

        if (!aV->gotParam((char*)usingFunc.c_str()))
        {
            FPS_PRINT_ERROR("assetVar [{}] does not have \"{}\" parameter. Cannot run function", aV->name, usingFunc);
            std::string funcError = "Could not find [" + usingFunc + "] as a parameter in assetVar " + aV->name;
            throw std::invalid_argument(funcError);
        }
        std::string reconfiguredFunc = aV->getcParam((char*)usingFunc.c_str());      // name of function assoc with our func#

        // get the function name
        size_t underscore_index = reconfiguredFunc.find_last_of('_');                  // gets the index of the last '_' -> the '_' separates the func name and the instance
        std::string name = reconfiguredFunc.substr(0, underscore_index);

        // get the instance number
        std::string instanceStr = reconfiguredFunc.substr(underscore_index + 1);
        int instance = std::stoi(instanceStr);

        if (debug) FPS_PRINT_INFO("Using param [{}] to run instance [{}] of function [{}]", usingFunc, instance, name);

        // get the run function pointer from our global map
        void *funcRef = modelFcnRef[name];
        if (!funcRef)
        {
            // our datamap name does not have an associated ModelObjectRun function
            FPS_PRINT_ERROR("Function name [{}] does not have an entry in the global run function map. Check function interface setup", name);
            std::string funcError = "Could not find [" + name + "] in global map of model functions";
            throw std::invalid_argument(funcError);
        }

        // Access the specific function pointer you want from the array
        void (*runFunc)(int) = reinterpret_cast<void(*)(int)>(funcRef);

        // RUN
        if (debug) FPS_PRINT_INFO(">>>>> running model func: {} at time [{}]", name, aV->am->vm->get_time_dbl());
        runFunc(instance);
        if (debug) FPS_PRINT_INFO("<<<<< step function returned at time [{}]\n", aV->am->vm->get_time_dbl());


        // tell state machine to kick off scheduler function to run sendToAmap on the core scheduler asap and then notify core when its finshed
        signalThread(aV, SEND);

    }
    catch(const std::exception& e)
    {
        if (debug) FPS_PRINT_ERROR("Tried to run model object function associated with aV [{}] and failed: {}", aV->name, e.what());

        std::string errorMsg = e.what();
        aV->setParam("errorType", (char*)"fault");
        aV->setParam("errorMsg", (char*)errorMsg.c_str());

        ESSLogger::get().critical("While trying to run the function on assetVar [{}], we got this error: [{}] ", aV->name, e.what());
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "datamap_errors", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }
        
        signalThread(aV, ERROR);
        return;

    }
}

// checks if all the functions on our aV have run and runs the next one or signals to the core that we are done
void nextOrDone(assetVar* aV)
{
    bool logging_enabled = aV->getbParam("logging_enabled");
    char* LogDir = aV->getcParam("LogDir");
    bool debug = aV->getbParam("debug");

    try
    {
        // get our active function (the one we just ran)
        if (!aV->gotParam("runningFunction"))
        {
            FPS_PRINT_ERROR("We do not have a \"runningFunction\" parameter and do not know what function just ran");
            std::string funcError = "Could not find [runningFunction] as a parameter in aV [" + aV->name + "]";
            throw std::invalid_argument(funcError);
        }
        int activeFcnNum = aV->getiParam("runningFunction");

        // how many total functions are on this aV?
        if (!aV->gotParam("numberOfFunctions"))
        {
            FPS_PRINT_ERROR("AssetVar [{}] does not have a \"numberOfFunctions\" param. This should be set in RunThread reload");
            std::string funcError = "Could not find [numberOfFunctions] as a parameter in aV [" + aV->name + "]";
            throw std::invalid_argument(funcError);

        }
        int totalNumOfFunctions = aV->getiParam("numberOfFunctions");

        if (debug) FPS_PRINT_INFO("    we just ran active function [{}] and we have a total of [{}] functions on this aV [{}]", activeFcnNum, totalNumOfFunctions, aV->name);

        if (activeFcnNum + 1 > totalNumOfFunctions)
        {
            // we have run a number of functions equal to our total number of functions; we do not need to run again
            if (debug) FPS_PRINT_INFO("We do not need to run again. setting 'done' to be RUN_SUCCESS");

            // reset our active function
            aV->setParam("runningFunction", 1);

            // tell RunThread we are done by setting flag high
            aV->setParam("done", true);

            // set our heartbeat as soon as all functions in our function group are done running
            aV->setParam("lastHeartbeat", aV->am->vm->get_time_dbl());
        }
        else
        {
            // we have not run all the functions on this aV, we do need to run again
            if (debug) FPS_PRINT_INFO("we DO need to run again. Rerunning get-run-send with active function #[{}]", activeFcnNum+1);

            // set active function to next one
            aV->setParam("runningFunction", activeFcnNum + 1);

            // signal the thread to run the process again
            signalThread(aV, GET);
        }
    }
    catch(const std::exception& e)
    {
        if (debug) FPS_PRINT_ERROR("Tried to run model object function associated with aV [{}] and failed: {}", aV->name, e.what());

        std::string errorMsg = e.what();
        aV->setParam("errorType", (char*)"fault");
        aV->setParam("errorMsg", (char*)errorMsg.c_str());

        ESSLogger::get().critical("While trying to run the function on assetVar [{}], we got this error: [{}] ", aV->name, e.what());
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "datamap_errors", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }
        
        signalThread(aV, ERROR);
    }
}

// define other functions that we would want to run in our state machine here and add them to the thread controller below

// handles the ess thread state machine
void EssThread(ess_thread* ess)
{
    FPS_PRINT_INFO("Essthread {} started", ess->name);

    std::pair<int, assetVar*> wakeChanPair;
    assetVar *thisContext = nullptr;
    bool debug = false;

    double delay = 1.0;
    // thread controller
    while (ess->running)
    {
        bool awake = ess->wakeChan.timedGet(wakeChanPair, delay);                // get a signal/aV pair off the thread wakeup channel
        if(awake)  // get signal, do thing, wait for next signal
        {
            ess->wakeup = wakeChanPair.first;
            thisContext = wakeChanPair.second;
            
            debug = thisContext->getbParam("debug");
            if (debug) FPS_PRINT_INFO("ESSThread: wakeup[{}] | thisContext[{}] | thisContext's am[{}] | datamap[{}]", ess->wakeup, thisContext->name, thisContext->am->name, thisContext->getcParam("datamapName"));
            
            if(ess->wakeup == SETUP)        // 100
            {
                setupFunctions(thisContext);
            }
            if(ess->wakeup == GET)          // 101
            {
                scheduleDMfunc(thisContext, "get");
            }
            if(ess->wakeup == RUN)          // 102
            {
                stepDMfunc(thisContext);
            }
            if(ess->wakeup == SEND)         // 103
            {
                scheduleDMfunc(thisContext, "send");
            }
            if(ess->wakeup == DONE)      // 104
            {
                nextOrDone(thisContext);
            }
            if(ess->wakeup == ERROR)    // 109
            {
                // tell the core controller that we have an error
                thisContext->setParam("runStatus", ERROR_ON_CONTEXT);
                thisContext->setParam("error", true);
                if (debug) FPS_PRINT_ERROR("Signaling to the core thread that we have an error", NULL);
            }
            
        }
    }

    FPS_PRINT_INFO("thread [{}] shutting down", ess->name);
    return;
}

void dataMapThreadCleanup()
{
    FPS_PRINT_INFO("Deleting remaining DataMaps and Threads");
    // delete all datamaps
    for (const auto& dm_pair : dataMaps)
    {
        delete dm_pair.second;
    }

    // delete all threads
    for (const auto& thread_pair : threadMaps)
    {
        ess_thread* ess = thread_pair.second;

        ess->running = false;
        if (ess->id.joinable())
        {
            ess->id.join();
        }
        
        delete ess;
    }
}