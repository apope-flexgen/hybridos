#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <cstring>
#include <malloc.h>
#include <cjson/cJSON.h>
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

// include simulink types
#include "../SL_files/Reference.h"
#include "../SL_files/rtwtypes.h"

// functional ess datamap structs and functions
struct DataItem {
    std::string name;
    size_t offset;
    std::string type;
};

struct DataMap {
    std::unordered_map<std::string, DataItem*> dataItems;
    std::string name;

    // in pair, first string is amap, second string is dmap
    std::map<std::string, std::vector<std::pair<std::string,std::string>>> transferBlocks;
    
    void addDataItem(char *name, int offset, char *type);
    void addTransferItem(std::string, std::string, std::string);
    void showTransferItems(std::string bname);
    void sendToAmap(std::string bname, asset_manager* am, DataMap *dataMap, uint8_t* dataArea);      // transfer data , using a named transfer block , from the dataMap to the amap
    bool setDataItemToAmap(asset_manager *am, DataMap *dataMap, const std::string &amapName, const std::string &mapName, uint8_t* dataArea);
    void getFromAmap(std::string bname, asset_manager* am,  DataMap *dataMap, uint8_t* dataArea);    // transfer data , using a named transfer block , from the amap to the dataMap
    bool getDataItemFromAmap(asset_manager *am, DataMap *dataMap, const std::string &amapName, const std::string &mapName, uint8_t* dataArea);
};
