
#include "dataMap.h"

// extern "C++" {
//     void dataMapThreadCleanup();
// }

std::unordered_map<std::string, std::unique_ptr<DataMap>> dataMaps;
std::unordered_map<std::string, void*> modelFcnRef;
std::unordered_map<std::string, ess_thread*> threadMaps;

// used in interface files, contains information about where to put info gotten from amap or where to get info to send
// to amap
void DataMap::addDataItem(char* name, int offset, DataMapType type)
{
    auto dataItem = new DataItem;
    dataItem->name = name;
    dataItem->offset = offset;
    dataItem->type = type;
    // delete and replace dataItems with the same name
    if (dataItems[std::string(name)])
    {
        // this dataItem already exists, we need to delete it
        delete dataItems[std::string(name)];
    }
    dataItems[std::string(name)] = dataItem;
}

// used in interface files, determines what gets sent to/from amap
void DataMap::addTransferItem(std::string bname, std::string aname, std::string dname)
{
    std::pair<std::string, std::string> item = std::make_pair(aname, dname);
    transferBlocks[bname].push_back(item);
}

// only used for debugging
void DataMap::showTransferItems(std::string bname)
{
    if (transferBlocks.find(bname) != transferBlocks.end())
    {
        FPS_PRINT_INFO(" Transfer Block name [{}] ", bname);
        FPS_PRINT_INFO("  [amap name]  <=> [dmap name]", NULL);
        FPS_PRINT_INFO("______________________________", NULL);
        for (auto xx : transferBlocks[bname])
        {
            FPS_PRINT_INFO("  [{}]  <=> [{}]", xx.first, xx.second);
        }
        std::cout << std::endl;
    }
}

// gets values from the amap and stores it in our model object for each element in our transfer block
void DataMap::getFromAmap(std::string bname, asset_manager* am, uint8_t* dataArea)
{
    if (transferBlocks.find(bname) != transferBlocks.end())
    {
        // FPS_PRINT_INFO("Get values from Amap using Transfer Block name [{}] ", bname);
        for (auto xx : transferBlocks[bname])
        {
            // if this is our transfer block, do nothing. This block allows for functions to not have inputs from the
            // amap
            if (xx.first == "" && xx.second == "")
                return;

            if (!getDataItemFromAmap(am, xx.first, xx.second, dataArea))
            {
                // getting here means we cant find our amapname in the amap or cant find our dataItem name in the
                // datamaps list of dataitems
                FPS_PRINT_ERROR(
                    "Amap name [{}] does not exist in amap or dataItem name [{}] does not exist in datamap [{}]'s list of dataItems. {} did not get values from amap. Check setupDatamap function for errors",
                    xx.first, xx.second, name, __func__);
                std::string errMsg =
                    "Amap name [" + xx.first + "] or dataItem name [" + xx.second +
                    "] were not found. Make sure amap name is set in setupAmap and dataItem name is set using addDataItem";
                throw std::logic_error(errMsg);
            }
        }
    }
    else
    {
        FPS_PRINT_ERROR(
            "Cannot find transfer block name: {}. Stopping {}. Check datamap configs and addTransferItem for issues",
            bname, __func__);
        std::string bnameError = "no transfer block [" + bname + "] found in datamap [" + name + "]";
        throw std::invalid_argument(bnameError);
    }
}

// sends values from our model object to the amap for each element in our transfer block
void DataMap::sendToAmap(varsmap& vmap, std::string bname, asset_manager* am, uint8_t* dataArea)
{
    if (transferBlocks.find(bname) != transferBlocks.end())
    {
        // FPS_PRINT_INFO("Send values to Amap using Transfer Block name [{}] ", bname);
        for (auto xx : transferBlocks[bname])
        {
            // if this is our transfer block, do nothing. This block allows for functions to not have outputs to the
            // amap
            if (xx.first == "" && xx.second == "")
                return;

            if (!setDataItemToAmap(vmap, am, xx.first, xx.second, dataArea))
            {
                // getting here means we cant find our amapname in the amap ir cant find our dataItem name in the
                // datamaps list of dataitems
                FPS_PRINT_ERROR(
                    "Amap name [{}] does not exist in amap or dataItem name [{}] does not exist in datamap [{}]'s list of dataItems. {} did not send values to amap. Check setupDatamap function for errors",
                    xx.first, xx.second, name, __func__);
                std::string errMsg =
                    "Amap name [" + xx.first + "] or dataItem name [" + xx.second +
                    "] were not found. Make sure amap name is set in setupAmap and dataItem name is set using addDataItem";
                throw std::logic_error(errMsg);
            }
        }
    }
    else
    {
        FPS_PRINT_ERROR(
            "Cannot find transfer block name: {}. Stopping {}. Check datamap configs and addTransferItem for issues",
            bname, __func__);
        std::string bnameError = "no transfer block [" + bname + "] found in datamap [" + name + "]";
        throw std::invalid_argument(bnameError);
    }
}

// Function to map data from the asset_manager to the DataMap data area
// currently only supports types found in DataMapType enum
bool DataMap::getDataItemFromAmap(asset_manager* am, const std::string& amapName, const std::string& mapName,
                                  uint8_t* dataArea)
{
    if (am->amap.find(amapName) != am->amap.end() && dataItems.find(mapName) != dataItems.end())
    {
        DataItem* dataItem = dataItems[mapName];
        switch (dataItem->type)
        {
            case DataMapType::INT_T:

                *(int*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
                break;

            case DataMapType::DOUBLE_T:

                *(double*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
                break;

            case DataMapType::BOOLEAN_T:

                *(bool*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getbVal();
                break;

            case DataMapType::UINT_T:

                *(uint_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
                break;

            case DataMapType::INT16_T:

                *(int16_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
                break;

            case DataMapType::UINT16_T:

                *(uint16_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
                break;

            case DataMapType::INT32_T:

                *(int32_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
                break;

            case DataMapType::UINT32_T:

                *(uint32_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
                break;

            case DataMapType::INT64_T:

                *(int64_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
                break;

            case DataMapType::UINT64_T:

                *(uint64_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
                break;

            case DataMapType::REAL_T:

                *(real_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
                break;

            case DataMapType::REAL32_T:

                *(real32_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
                break;

            case DataMapType::REAL64_T:

                *(real64_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
                break;

            case DataMapType::ULONG_T:

                *(ulong_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
                break;

            case DataMapType::ULONGLONG_T:

                *(ulonglong_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
                break;

            case DataMapType::TIME_T:

                *(time_T*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
                break;

            case DataMapType::STRING:

                *(std::string*)(&dataArea[dataItem->offset]) = am->amap[amapName]->getcVal();
                break;

            case DataMapType::CHAR_ARRAY:

                *(char**)(&dataArea[dataItem->offset]) = am->amap[amapName]->getcVal();
                break;

            default:

                FPS_PRINT_ERROR("This dataItem has an unknown type [{}]", typeid(dataItem->type).name());
                std::string errMsg = "dataItem correspoding to amap name [" + amapName + "] and dataItem name [" +
                                     mapName + "] in datamap [" + name + "] has an unknown type";
                throw std::logic_error(errMsg);
                break;
        }
    }
    else
    {
        return false;
    }

    return true;
}

// Function to map data from the DataMap data area to the asset_manager. returns false if not found
// currently only supports types in DataMapType enum
bool DataMap::setDataItemToAmap(varsmap& vmap, asset_manager* am, const std::string& amapName,
                                const std::string& mapName, uint8_t* dataArea)
{
    if (am->amap.find(amapName) != am->amap.end() && dataItems.find(mapName) != dataItems.end() &&
        am->amap[amapName] != nullptr)
    {
        VarMapUtils* vm = am->vm;
        DataItem* dataItem = dataItems[mapName];
        assetVar* av = am->amap[amapName];

        if (dataItem->type == DataMapType::INT_T || dataItem->type == DataMapType::UINT_T ||
            dataItem->type == DataMapType::INT16_T || dataItem->type == DataMapType::UINT16_T ||
            dataItem->type == DataMapType::INT32_T || dataItem->type == DataMapType::UINT32_T ||
            dataItem->type == DataMapType::INT64_T || dataItem->type == DataMapType::UINT64_T)
        {
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), *(int*)(&dataArea[dataItem->offset]));
        }
        else if (dataItem->type == DataMapType::DOUBLE_T || dataItem->type == DataMapType::REAL_T ||
                 dataItem->type == DataMapType::REAL32_T || dataItem->type == DataMapType::REAL64_T ||
                 dataItem->type == DataMapType::TIME_T || dataItem->type == DataMapType::ULONG_T ||
                 dataItem->type == DataMapType::ULONGLONG_T)
        {
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), *(double*)(&dataArea[dataItem->offset]));
        }
        else if (dataItem->type == DataMapType::BOOLEAN_T)
        {
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), *(bool*)(&dataArea[dataItem->offset]));
        }
        else if (dataItem->type == DataMapType::STRING || dataItem->type == DataMapType::CHAR_ARRAY)
        {
            vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), *(char**)(&dataArea[dataItem->offset]));
        }
        else
        {
            FPS_PRINT_ERROR("This dataItem has an unknown type [{}]", typeid(dataItem->type).name());
            std::string errMsg = "dataItem correspoding to amap name [" + amapName + "] and dataItem name [" + mapName +
                                 "] in datamap [" + name + "] has an unknown type";
            throw std::logic_error(errMsg);
        }
        // char case delayed until use case is created and char is needed
    }
    else
    {
        return false;
    }

    return true;
}

// given an amname, either find and return or create an asset manager with that name. if we create an asset manager, the
// pname is its parent asset manager (ess by default)
asset_manager* getOrMakeAm(VarMapUtils* vm, varsmap& vmap, const char* pname, const char* amname)
{
    char* essName = vm->getSysName(vmap);
    auto essam = vm->getaM(vmap, essName);
    asset_manager* am = nullptr;
    int debug = 0;

    if (!pname)
    {
        // if we dont get a pname, make ess am the parent
        pname = essName;
    }

    if (debug)
        FPS_PRINT_INFO("got aname: {} and pname: {}", amname, pname);
    // get parent asset manager or make one ourselves
    auto pam = vm->getaM(vmap, pname);
    if (!pam)
    {
        if (debug)
            FPS_PRINT_INFO(" pname [{}]  not found, have to make one", pname);
        pam = new asset_manager(pname);
        vm->setaM(vmap, pname, pam);
        // set up the rest of the am
        pam->setFrom(essam);  // pam's parent is ess
        essam->addManAsset(pam, pname);
        if (debug)
            FPS_PRINT_INFO(" added {} as a new parent asset manager", pname);
    }
    // get asset manager or make one ourselves
    am = vm->getaM(vmap, amname);
    if (!am)
    {
        if (debug)
            FPS_PRINT_INFO(" amname [{}]  not found, have to make one", amname);
        am = new asset_manager(amname);
        vm->setaM(vmap, amname, am);
        // set up the rest of the am
        am->setFrom(pam);
        pam->addManAsset(am, amname);
        if (debug)
            FPS_PRINT_INFO(" added {} as a new asset manager", amname);
    }

    return am;
}

// Function to replace all slashes and colons with underscores for displaying in amap
std::string replaceSlashAndColonWithUnderscore(const std::string& inputString)
{
    std::string modifiedString = inputString;

    // we dont want to replace the first slash, but we want to replace all subsequent slashes or colons
    size_t firstSlashPos = modifiedString.find("/");
    size_t slashPos = modifiedString.find("/", firstSlashPos + 1);
    size_t colonPos = modifiedString.find(":");

    // iterate over string while looking for colons/slashes
    while (slashPos != std::string::npos || colonPos != std::string::npos)
    {
        if (slashPos == colonPos && slashPos != std::string::npos)
        {
            slashPos = modifiedString.find("/", slashPos + 1);
            colonPos = modifiedString.find(":", colonPos + 1);
            continue;
        }

        if (slashPos < colonPos)
        {
            modifiedString.replace(slashPos, 1, "_");
            slashPos = modifiedString.find("/", slashPos + 1);
        }
        else
        {
            modifiedString.replace(colonPos, 1, "_");
            colonPos = modifiedString.find(":", colonPos + 1);
        }
    }

    return modifiedString;
}

// Function to format a number according to the provided format specifier using regex
std::string formatTemplateNumber(const std::string unformattedInput, int number)
{
    // Define the regex pattern to match the format specifier -> (any number of characters)('{')(some number)('d}')
    std::regex pattern(R"(^(.*)\{([^}]*)d\}$)");
    std::smatch match;

    // Use regex to match the format specifier
    if (std::regex_match(unformattedInput, match, pattern))
    {
        if (match.size() != 3)
        {
            // Expecting three parts: full match, prefix, and format inside braces
            FPS_PRINT_ERROR(
                "Attempting to parse [{}] but input is incorrect format. Be sure to include a prefix and your number surrounded by braces and ending with a lowercase d",
                unformattedInput);
            std::string err = fmt::format("Replacment specifier [{}] is incorrectly formatted", unformattedInput);
            throw std::invalid_argument(err);
        }

        // Extract the actual format inside the braces
        std::string prefix = match[1].str();
        std::string formatted = match[2].str();

        // Use fmt::format to format the number according to the extracted specifier
        std::string formattedNum = fmt::format("{" + formatted + "}", number);

        // prepend the perfix
        return prefix + formattedNum;
    }
    else
    {
        FPS_PRINT_ERROR("Attempting to parse [{}] but input is incorrect format", unformattedInput);
        std::string err = fmt::format("Replacment specifier [{}] is incorrectly formatted", unformattedInput);
        throw std::invalid_argument(err);
    }
}

// Function to replace occurrences of 'key' with 'rep' in the 'uri' string
std::string replaceKeyInURI(const std::string& uri, const std::string& key, const std::string& rep)
{
    std::string result = uri;
    size_t pos = 0;

    // Loop to find and replace all occurrences of 'key' with 'rep'
    while ((pos = result.find(key, pos)) != std::string::npos)
    {
        result.replace(pos, key.length(), rep);
        pos += rep.length();  // Move past the last replaced position
    }

    return result;
}

// check if a given input string is a valid uri
bool isValidURI(std::string input)
{
    if (input.empty())
    {
        return false;  // Early exit for empty strings
    }

    // Check if the string starts with '/'
    if (input[0] != '/')
    {
        FPS_PRINT_ERROR("input string [{}] does not start with a '/' and is not a valid URI", input);

        return false;
    }

    // Check if the string contains ':'
    if (input.find(':') == std::string::npos)
    {
        FPS_PRINT_ERROR("input string [{}] does not contain a ':' and is not a valid URI", input);

        return false;
    }

    // Both conditions are met
    return true;
}

std::string global_template_uri;
// returns an assetVar or creates one using the values from the template AV
assetVar* getOrMakeThreadAV(varsmap& vmap, VarMapUtils* vm, std::string name)
{
    assetVar* aV = nullptr;

    std::string comp = "/control/thread:" + name;
    aV = vm->getVar(vmap, (char*)comp.c_str(), nullptr);
    if (aV)
    {
        return aV;
    }

    // if we get here, our aV doesnt exist yet
    bool bval = true;  // value is true or false based on if the thread is running or not
    aV = vm->makeVar(vmap, (char*)comp.c_str(), nullptr, bval);

    // set all of our default values for the threadAV
    aV->setParam("overrunLimit", 50);
    aV->setParam("decrementOverrun", 1);   // every successful run, dec our overrunCounter by 1
    aV->setParam("incrementOverrun", 5);   // every time we overrun, inc our overrunCounter by 5
    aV->setParam("setupTimeLimit", 120);   // default setup time limit is 2 minutes
    aV->setParam("heartbeatTimeout", 10);  // default heartbeat timeout is 10 seconds

    // check the global template uri. if it hasnt been set up, we dont have a template AV
    if ((global_template_uri.c_str()[0] != '/') && (global_template_uri.c_str()[0] != '_'))
    {
        if (0)
            FPS_PRINT_INFO(
                " No template av found from global_template_uri: [{}]. Using system default values for dataMap operation",
                global_template_uri);
        return aV;
    }

    // if it has been set up, use the template AV
    assetVar* templateAV = vm->getVar(vmap, (char*)global_template_uri.c_str(), nullptr);

    // if we have a value in our templateAV, use that instead of the hard coded
    if (templateAV)
    {
        if (templateAV->gotParam("overrunLimit"))
        {
            aV->setParam("overrunLimit", templateAV->getiParam("overrunLimit"));
        }

        if (templateAV->gotParam("decrementOverrun"))
        {
            aV->setParam("decrementOverrun", templateAV->getiParam("decrementOverrun"));
        }

        if (templateAV->gotParam("incrementOverrun"))
        {
            aV->setParam("incrementOverrun", templateAV->getiParam("incrementOverrun"));
        }

        if (templateAV->gotParam("setupTimeLimit"))
        {
            aV->setParam("setupTimeLimit", templateAV->getdParam("setupTimeLimit"));
        }

        if (templateAV->gotParam("heartbeatTimeout"))
        {
            aV->setParam("heartbeatTimeout", templateAV->getdParam("heartbeatTimeout"));
        }
    }
    else
    {
        if (0)
            FPS_PRINT_INFO("No template aV found from comp [{}]. Using system default values for dataMap operation",
                           global_template_uri);
    }

    return aV;
}

// this function runs once on the core scheduler and sets our thread template aV to have whatever uri is set in config
int ThreadSetup(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);

    global_template_uri = aV->comp + ":" + aV->name;

    if (0)
        FPS_PRINT_INFO("thread Template assetVar has uri: {}", global_template_uri);

    return 0;
}

// this function gets called 1 time and it schedules our heartbeat timer
bool heartbeatSetupDone = false;
void heartbeatSetup(varsmap& vmap, assetVar* aV)
{
    // we only ever want this function to run one time
    if (heartbeatSetupDone)
        return;

    // create a heartbeat aV and schedule HeartbeatTimer
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    bool debug = false;

    // get or make the AV we are running the heartbeat timer on
    std::string comp = "/control/thread:heartbeat";
    assetVar* heartbeatAV = vm->getVar(vmap, (char*)comp.c_str(), nullptr);
    if (!heartbeatAV)
    {
        // if this heartbeatAV doesnt exist already, make it
        double dval = 0;
        heartbeatAV = vm->makeVar(vmap, comp.c_str(), nullptr, dval);
        heartbeatAV->am = aV->am;
        if (debug)
            FPS_PRINT_INFO("made new heartbeatAV [{}]", heartbeatAV->name);
    }

    // set up new schedItem
    auto schItem = new schedItem();
    char* myid = (char*)"heartbeat_timer";
    char* schedUri = (char*)comp.c_str();  // set to be the same as the schItem assetVar's comp
    char* schedFcn = (char*)"HeartbeatTimer";
    char* schedTarg = (char*)comp.c_str();  // same as our uri

    // set our repTime (default of 5 seconds)
    double every = 5;

    // check if we have a valid comp for our global template uri
    if ((global_template_uri.c_str()[0] == '/') && (global_template_uri.c_str()[0] == '_'))
    {
        // if we have a valid comp, get the templateAV
        assetVar* templateAV = vm->getVar(vmap, (char*)global_template_uri.c_str(), nullptr);

        // set our repTime from our template AV if it exists
        if (templateAV)
        {
            if (templateAV->gotParam("heartbeatTimeout"))
            {
                every = templateAV->getdParam("heartbeatTimeout") / 2;
            }
        }
    }

    schItem->setUp(myid, "ess", schedUri, schedFcn, 0, vm->get_time_dbl(), every, 0, schedTarg);

    // set params of our schedItem's aV
    schItem->av = heartbeatAV;                             // set to run on the comp as our TEMPLATE AV
    schItem->av->setParam("runTime", vm->get_time_dbl());  // tell scheduler to run asap
    schItem->av->setParam("repTime",
                          every);  // need to set the schedItem and the schedAV to have the proper rep and runTime
    schItem->av->setParam("update", true);  // tell this schedItem to update so that it runs faster after the first run

    if (debug)  // prints new schedItem
    {
        FPS_PRINT_INFO(" Set up new schedItem [{}] to run [{}] on aV [{}]", schItem->id, schItem->func,
                       schItem->av->name);
        schItem->show();
    }

    // put our new schedItem on the scheduler and wake it up
    auto reqChan = (channel<schedItem*>*)am->reqChan;
    reqChan->put(schItem);
    if (am->wakeChan)
    {
        am->wakeChan->put(0);
    }

    if (debug)
        FPS_PRINT_INFO("HeartbeatTimer has been set up to run every [{}] seconds on aV [{}:{}]", every,
                       schItem->av->comp, schItem->av->name);
    heartbeatSetupDone = true;
}
