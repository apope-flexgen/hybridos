#ifndef UPDATETODBI_CPP
#define UPDATETODBI_CPP

#include "asset.h"
#include "ess_utils.hpp"
#include "formatters.hpp"

extern "C++" {
int UpdateToDbi(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

/**
 * @brief Initializes the parameters to be used for running system command
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the assetVar to initialize parameters for
 */
void setupUpdateDbiParams(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    UNUSED(vmap);
    UNUSED(amap);
    FPS_PRINT_DEBUG("Setting up params for [{}]", av->getfName());

    // Amount of time to wait before sending update request to dbi
    if (!av->gotParam("updateTimeout"))
        av->setParam("updateTimeout", 0);
    if (!av->gotParam("currUpdateTime"))
        av->setParam("currUpdateTime", av->getdParam("updateTimeout"));

    // Document in the DBI that will contain the collection of variables
    if (!av->gotParam("document"))
    {
        auto doc = fmt::format("{}_{}_saved_variables", aname, av->name);
        av->setParam("document", (char*)doc.c_str());
    }

    // The structure of the data that will be send to dbi
    // Either full (:f) by default or compact (:c)
    if (!av->gotParam("includeMetaData"))
        av->setParam("includeMetaData", false);
}

/**
 * @brief Sends an update request to the dbi
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the fims object for data interchange
 * @param av the assetVar containing the collection of other assetVars targeted
 * for dbi updates
 */
int UpdateToDbi(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    if (!checkAv(vmap, amap, aname, p_fims, av))
        return -1;

    if (!p_fims)
    {
        FPS_PRINT_WARN("p_fims is null", NULL);
        return -1;
    }

    FPS_PRINT_DEBUG("av [{}] av->am [{}] aname [{}]", av->getfName(), av->am ? av->am->name : "null", cstr{ aname });
    VarMapUtils* vm = av->am->vm;

    // Setup parameters needed for updating dbi function
    setupUpdateDbiParams(vmap, amap, aname, av);

    double tNow = vm->get_time_dbl();
    double tDiff = tNow - av->getdParam("tLast");
    double updateTimeout = av->getdParam("updateTimeout");
    double currUpdateTime = av->getdParam("currUpdateTime");
    bool includeMetaData = av->getbParam("includeMetaData");
    const auto document = fmt::format("{}", av->getcParam("document"));

    // Wait until current update time reaches 0 before sending an update request
    // to dbi
    if (currUpdateTime > 0)
        ESSUtils::decrementTime(av, currUpdateTime, tDiff, "currUpdateTime");

    if (currUpdateTime <= 0)
    {
        FPS_PRINT_DEBUG(
            "Update time reached 0. Now attempting to send update "
            "request to dbi for asset [{}] with [{}]",
            cstr{ aname }, av->getfName());

        // Reset the current update time to the configured update time
        av->setParam("currUpdateTime", updateTimeout);

        // The list of variables to update to the dbi
        std::vector<assetVar*> dbiVars;

        // If we are unable to retrieve the list of operands, skip update to dbi
        if (!ESSUtils::getAvList(vmap, amap, av, dbiVars))
        {
            FPS_PRINT_WARN(
                "Unable to retrieve list of assetVars for [{}]. Skipping "
                "update to dbi",
                av->getfName());
            return -1;
        }
        if (dbiVars.size() <= 0)
        {
            FPS_PRINT_WARN("The list of assetVars for [{}] is empty. Skipping update to dbi", av->getfName());
            return -1;
        }

        // The collection of variables to update to the dbi, where the key is the
        // uri of the variable and the value is the variable name and contents Ex.:
        //     key = uriKey + assetVar name (/components/bms_1/charge_energy ->
        //     #components#bms_1/charge_energy) value = assetVar's metadata
        std::vector<std::pair<std::string, std::string>> varsToUpdate;

        // Add the variables to the collection of variables to update to the dbi
        for (auto it = dbiVars.begin(); it != dbiVars.end(); it++)
        {
            FPS_PRINT_DEBUG("Got dbiAv [{}] from assetVar [{}] >> dbiAv->comp: [{}]", (*it)->getfName(),
                            (*it)->getfName(), (*it)->comp);
            const std::string msg = includeMetaData ? fmt::format("{:f}", *it) : fmt::format("{:c}", *it);

            const std::string key = fmt::format("{}/{}", vm->run_replace((*it)->comp, "/", "#"), (*it)->name);
            FPS_PRINT_DEBUG("dbiAv->comp: [{}]  key: [{}]  msg: [{}]", (*it)->comp, key, msg);

            varsToUpdate.emplace_back(std::make_pair(key, msg));
        }

        // Compose fims message containing all targeted variables to update
        // Then, trigger a fims send to send update request to dbi
        for (const auto& entry : varsToUpdate)
        {
            const std::string dest = fmt::format("/dbi/ess_controller/{}/{}", document, entry.first);
            const std::string body = fmt::format("{}", entry.second);
            FPS_PRINT_DEBUG("Sending update request to dest: [{}] with body: [{}]", dest, body);
            if (p_fims->Send("set", dest.c_str(), nullptr, body.c_str()) <= 0)
                FPS_PRINT_WARN("Unable to send fims set to dest: [{}] with body: [{}]", dest, body);
        }
    }

    av->setParam("tLast", vm->get_time_dbl());
    return 0;
}

#endif