#ifndef DATAMAP_HPP
#define DATAMAP_HPP

#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <cstring>
#include <sstream>
#include <malloc.h>
#include <poll.h>
#include <signal.h>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <fims/libfims.h>
#include <fims/fps_utils.h>

// logging and formatting utilites, DO NOT REMOVE
#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/bundled/format.h"
#include "spdlog/fmt/bundled/ranges.h"

#include "asset.h"
#include "assetVar.h"
#include "channel.h"

#include "ESSLogger.hpp"
#include "chrono_utils.hpp"
#include "ess_utils.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define closesocket close

#include <fims/libfims.h>
#include <csignal>

#include "varMapUtils.h"

#include "formatters.hpp"
#include "scheduler.h"
#include <unordered_map>

// include simulink types
#include "../SL_files/autogen/rtwtypes.h"

// thread controller
#define SETUP 100
#define GET 101
#define RUN 102
#define SEND 103
#define DONE 104
#define ERROR 109

// core controller
#define SETUP_CONTEXT 11
#define RUN_CONTEXT 12
#define ERROR_ON_CONTEXT 19
#define WAITING_FOR_DONE 10
#define STALL -10
#define SEVERITY 1

// "setup" macros
#define EXCEEDED_TIME_LIMIT -1
#define STANDBY 0
#define SETUP_READY 1
#define SETUP_CHECK 2

// reload
#define FULL_RELOAD 0
#define PARTIAL_RELOAD 1

enum DataMapType
{
    BOOLEAN_T,
    INT_T,
    UINT_T,
    INT16_T,
    UINT16_T,
    INT32_T,
    UINT32_T,
    INT64_T,
    UINT64_T,
    ULONG_T,
    ULONGLONG_T,
    DOUBLE_T,
    REAL_T,
    REAL32_T,
    REAL64_T,
    TIME_T,
    STRING,
    CHAR_ARRAY,
    dflt
};

struct DataItem
{
    std::string name;
    size_t offset;
    DataMapType type = DataMapType::dflt;
};

struct DataMap
{
    DataMap()  // constructor
    {
    }

    ~DataMap()  // destructor
    {
        for (const auto& dataItem : dataItems)
        {
            delete dataItem.second;
        }
    }

    std::unordered_map<std::string, DataItem*> dataItems;
    std::string name;
    std::string amname;

    // in pair, first string is amap, second string is dmap
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> transferBlocks;

    void addDataItem(char* name, int offset, DataMapType type);
    void addTransferItem(std::string, std::string, std::string);
    void showTransferItems(std::string bname);
    void sendToAmap(varsmap& vmap, std::string bname, asset_manager* am,
                    uint8_t* dataArea);  // transfer data , using a named transfer block , from the dataMap to the amap
    bool setDataItemToAmap(varsmap& vmap, asset_manager* am, const std::string& amapName, const std::string& mapName,
                           uint8_t* dataArea);
    void getFromAmap(std::string bname, asset_manager* am,
                     uint8_t* dataArea);  // transfer data , using a named transfer block , from the amap to the dataMap
    bool getDataItemFromAmap(asset_manager* am, const std::string& amapName, const std::string& mapName,
                             uint8_t* dataArea);
};

class ess_thread
{
public:
    ess_thread(char* _name)
    {
        name = strdup(_name);
        running = false;
    }
    ~ess_thread() { free((void*)name); }

    // fields
    std::thread id;
    char* name;
    assetVar* threadAV;

    // this is our incoming event channel used by the thread
    channel<std::pair<int, assetVar*>>
        wakeChan;  // set as a pair to pass signal along with context ("i want you to run this signal against this aV")
    int wakeup;
    bool running;
    double delay;
    int core;

    // map to determine which contexts are running on our thread
    // the first string is the assetVars name and the pair is the assetVar itself and the num assoc w that assetVar
    std::unordered_map<std::string, std::pair<assetVar*, int>> contexts;
};

class Heartbeat
{
private:
    // Private constructor and destructor for singleton
    Heartbeat()  // constructor
    {
    }
    ~Heartbeat()  // destructor
    {
    }

public:
    // Public method to get the instance of the singleton
    static Heartbeat& getInstance()
    {
        static Heartbeat instance;
        return instance;
    }

    // vector of essthread classes
    std::vector<ess_thread*> threads;
};

void EssThread(ess_thread* ess);  // used by core to call thread

extern std::unordered_map<std::string, std::unique_ptr<DataMap>>
    dataMaps;  // a map of all dataMaps and their names. used to access dataMaps outside of their creation function
extern std::unordered_map<std::string, void*> modelFcnRef;  // a map of function names to their setup and run functions
extern std::unordered_map<std::string, ess_thread*> threadMaps;  // a map of all the threads

asset_manager* getOrMakeAm(VarMapUtils* vm, varsmap& vmap, const char* pname,
                           const char* amname);  // defined in dataMap_thread but used in SL ref files
assetVar* getOrMakeThreadAV(varsmap& vmap, VarMapUtils* vm,
                            std::string name);  // defined in dataMapUtils and used in dataMapCore
std::string replaceSlashAndColonWithUnderscore(const std::string& inputString);

// template expansion helper functions
std::string formatTemplateNumber(const std::string unformattedInput, int number);
std::string replaceKeyInURI(const std::string& uri, const std::string& key, const std::string& rep);
bool isValidURI(std::string input);

void signalThread(assetVar* targAv, int signal);  // this function is how the core controller signals the thread
void startThread(assetVar* aV, varsmap& vmap, char* tname);
void signalError(assetVar* aV, std::string funcName, std::string errMsg, bool fault);

void heartbeatSetup(varsmap& vmap, assetVar* aV);

#endif