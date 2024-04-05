/**
 * @file RunSystemCmd.cpp
 * @brief Information related to system requests (ex.: bms power on, power off,
 * etc.) are encapulated in a command object. The command object will execute an
 * action depending on the request
 * @date 2021-05-17
 */

#ifndef RUNSYSTEMCMD_CPP
#define RUNSYSTEMCMD_CPP

#include "asset.h"

extern "C++" {
int RunSystemCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

/** The Command interface */
class Command
{
public:
    virtual ~Command() {}
    virtual void execute(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av) const = 0;
};

/** Receiver - this is responsible for carrying out the command request */
class SystemCmdReceiver
{
public:
    /**
     * @brief System command for powering on asset
     *
     * @param vmap the global data map
     * @param amap the local data map
     * @param aname the asset manager/asset name
     * @param p_fims the interface used for data interchange
     * @param av the assetVar to run the command for
     */
    void PowerOn(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
    {
        UNUSED(aname);
        UNUSED(p_fims);
        FPS_ERROR_PRINT("%s >> Powering on asset [%s]...\n", __func__, av->am->name.c_str());
        VarMapUtils* vm = av->am->vm;

        // Check for # of faults
        int faultCnt = amap["FaultCnt"]->getiVal();
        char* cval = (char*)"On Ready";
        if (faultCnt > 1)
        {
            FPS_ERROR_PRINT("%s >> Powering on asset [%s] with fault. Setting status to fault\n", __func__,
                            av->am->name.c_str());
            cval = (char*)"On Fault";
        }
        vm->setVal(vmap, amap["SystemStatus"]->comp.c_str(), amap["SystemStatus"]->name.c_str(), cval);

        // Set register indicating power on ready
    }

    /**
     * @brief System command for powering off asset
     *
     * @param vmap the global data map
     * @param amap the local data map
     * @param aname the asset manager/asset name
     * @param p_fims the interface used for data interchange
     * @param av the assetVar to run the command for
     */
    void PowerOff(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
    {
        UNUSED(aname);
        UNUSED(p_fims);
        FPS_ERROR_PRINT("%s >> Powering off asset [%s]...\n", __func__, av->am->name.c_str());
        VarMapUtils* vm = av->am->vm;

        // Check for # of faults
        int faultCnt = amap["FaultCnt"]->getiVal();
        char* cval = (char*)"Off Ready";
        if (faultCnt > 1)
        {
            FPS_ERROR_PRINT("%s >> Powering off asset [%s] with fault. Setting status to fault\n", __func__,
                            av->getfName());
            cval = (char*)"Off Fault";
        }
        vm->setVal(vmap, amap["SystemStatus"]->comp.c_str(), amap["SystemStatus"]->name.c_str(), cval);

        // Set register indicating power off ready
    }
};

/** Invoker - this is responsible for sending a request to run a command*/
class SystemCmdInvoker
{
private:
    std::shared_ptr<Command> m_syscmd;

public:
    /**
     * @brief Set the command object
     *
     * @param cmd the command object to set
     */
    void setCmd(std::shared_ptr<Command> cmd) { m_syscmd = std::move(cmd); }

    /**
     * @brief Run the command by passing the request to the receiver
     *
     * @param vmap the global data map
     * @param amap the local data map
     * @param aname the asset manager/asset name
     * @param p_fims the interface used for data interchange
     * @param av the assetVar to run the command for
     */
    void runCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
    {
        FPS_ERROR_PRINT("%s >> Executing system command\n", __func__);
        if (m_syscmd)
            m_syscmd->execute(vmap, amap, aname, p_fims, av);
    }
};

/** Concrete command(s) */
class SystemCmd : public Command
{
private:
    std::shared_ptr<SystemCmdReceiver> m_receiver;  // The receiver that will carry out the command request
    std::string m_cmd;                              // The name of the command to run
public:
    explicit SystemCmd(std::shared_ptr<SystemCmdReceiver> receiver, std::string& cmd)
        : m_receiver(std::move(receiver)), m_cmd(cmd)
    {
    }

    /**
     * @brief Run the command using the receiver's method(s)
     *
     * @param vmap the global data map
     * @param amap the local data map
     * @param aname the asset manager/asset name
     * @param p_fims the interface used for data interchange
     * @param av the assetVar that contains the command to run
     */
    void execute(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av) const override
    {
        if (m_cmd != "Power On" && m_cmd != "Power Off")
        {
            FPS_ERROR_PRINT("%s >> The system cmd [%s] provided is not supported.\n", __func__, m_cmd.c_str());
            return;
        }
        if (m_cmd == "Power On")
        {
            m_receiver->PowerOn(vmap, amap, aname, p_fims, av);
        }
        if (m_cmd == "Power Off")
        {
            m_receiver->PowerOff(vmap, amap, aname, p_fims, av);
        }
    }
};

/**
 * @brief Runs the system command for a particular asset and updates the asset's
 * state
 *
 * @param vmap the global data map
 * @param amap the local data map
 * @param aname the asset manager/asset name
 * @param p_fims the interface used for data interchange
 * @param av the assetVar that contains the command to run
 */
int RunSystemCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    FPS_DEBUG_PRINT("%s >> assetVar [%s]  aname [%s]\n", __func__, av->name.c_str(), aname);
    asset_manager* fixed_am = av->am->vm->getaM(vmap, aname);
    // Make sure we are using the right asset manager for the assetVar based on
    // the given aname
    if (fixed_am)
        av->am = fixed_am;
    FPS_ERROR_PRINT(
        "%s >> Running for assetVar [%s] with aname [%s] and asset "
        "manager (fixed)  [%s]\n",
        __func__, av->name.c_str(), aname, fixed_am->name.c_str());

    VarMapUtils* vm = av->am->vm;
    std::string reloadStr = "RunSystemCmd";
    int reload = vm->CheckReload(vmap, amap, aname, reloadStr.c_str());
    if (reload < 2)
    {
        FPS_ERROR_PRINT("%s >>  %s  reload first run  %d \n", __func__, aname, reload);
        int ival = 0;
        char* cval = (char*)"Init";
        amap["FaultCnt"] = vm->setLinkVal(vmap, aname, "/status", "FaultCnt", ival);
        amap["SystemStatus"] = vm->setLinkVal(vmap, aname, "/status", "SystemStatus", cval);

        if (reload < 1)
        {
            FPS_ERROR_PRINT("%s >> Setting assetVar [%s] for %s to /status\n", __func__, av->name.c_str(), aname);
            amap[av->name] = vm->setLinkVal(vmap, aname, "/status", av->name.c_str(), av);
        }
        reload = 2;
        amap[reloadStr] = vm->setLinkVal(vmap, aname, "/reload", reloadStr.c_str(), reload);
        amap[reloadStr]->setVal(reload);
    }

    // Look at key cmd value
    // Key cmd value is stored in av
    std::string keyCmd(av->getcVal());

    // Or, in config, have key cmd value remapped to different value with string
    // data type
    std::unique_ptr<SystemCmdInvoker> invoker(new SystemCmdInvoker());
    std::shared_ptr<SystemCmdReceiver> receiver(new SystemCmdReceiver());
    std::shared_ptr<SystemCmd> syscmd(new SystemCmd(receiver, keyCmd));
    invoker->setCmd(syscmd);
    invoker->runCmd(vmap, amap, aname, p_fims, av);

    return 0;
}

#endif