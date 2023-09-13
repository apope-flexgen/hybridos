
#include "asset.h"
#include "scheduler.h"
#include "formatters.hpp"
#include <filesystem>
#include <fstream>
#include "AssetSetupUtility.h"
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

// this is currently meant to be "included" into vmConfig

extern "C++"
{
    int SetupBMS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
};


// Dummy Setup Function
int SetupBMS(varsmap &vmap, varmap &amap, const char* _aname, fims* p_fims, assetVar* aV)
{

    VarMapUtils *vm = aV->am->vm;

    int num_racks = 20;
    if (aV->gotParam("num_racks"))
    {
        num_racks = aV->getiParam("num_racks");
    }
    int num_bms = 2;
    if (aV->gotParam("num_bms"))
    {
        num_bms = aV->getiParam("num_bms");
    }


   //Use global string variables to get stdcfg path for bms
    std::string stdcfgBmsConfigPath = stdcfgPath + stdcfgBmsDir;

    if (std::filesystem::exists(stdcfgBmsConfigPath)) {
        // Directory exists, you can proceed with opening it
        // ... your code to open and process the directory ...
    } else {
        std::cerr << "Directory does not exist: " << stdcfgBmsConfigPath << std::endl;
    }

    //Create vector and populate it with all filenames within given path from above
    std::vector<std::string> fileNamesVector;
    for (const auto & entry : std::filesystem::directory_iterator(stdcfgBmsConfigPath)) {
        std::filesystem::path fileNamePath = entry.path();
        std::string fileName = fileNamePath.string();
        fileNamesVector.push_back(fileName);
    }


    //Create vectors for each of the types of stdcfg bms files. v_bms_manager, bms_manager, bms_rack
    std::vector<std::string> stdVBmsManagerFiles;
    std::vector<std::string> stdBmsManagerFiles;
    std::vector<std::string> stdBmsRackFiles;

    //size of the filenames vector which is a value that is frequently used
    int fileNamesVectorSize =  fileNamesVector.size();


    //for loop to cycle through all of the filenames and get their respective file contents then convert them to strings
    //after that it pushes the data to the correct vector dependent on what type of file it is
    for(int i = 0; i < fileNamesVectorSize; i++){

        //get the first filename by looking at the back of the vector
        std::string currFileName = fileNamesVector.back();
        fileNamesVector.pop_back();

        std::string pathToFileName = currFileName;
        std::ifstream jsonFile (pathToFileName);
        std::string currentJsonString;


        if (jsonFile.is_open()) {
        std::string jsonStringHolder((std::istreambuf_iterator<char>(jsonFile)),
                                      (std::istreambuf_iterator<char>()));

        currentJsonString.clear(); // Clear the string before appending.
        currentJsonString = jsonStringHolder;

        jsonFile.close();
        } else {
            std::cerr << "Error opening file." << std::endl;
            return 1; // Return an error code to indicate failure.
        }


        //this section parses the first 3 words in the filename which is how it is determined what config file type it is

        currFileName.erase(0, currFileName.find(stdcfgBmsConfigPath) + stdcfgBmsConfigPath.length());


        char delimiter = '_';

        std::vector<std::string> parsedElements;
        size_t startPos = 0;
        size_t endPos;

        while ((endPos = currFileName.find(delimiter, startPos)) != std::string::npos) {
            // Extract the substring between delimiters
            std::string element = currFileName.substr(startPos, endPos - startPos);
            parsedElements.push_back(element);

            // Update the starting position for the next iteration
            startPos = endPos + 1;
        }

        // Extract the last element after the last delimiter
        std::string lastElement = currFileName.substr(startPos);
        parsedElements.push_back(lastElement);



        std::string target = "v";

        auto iter1 = std::find(parsedElements.begin(), parsedElements.end(), target);

        if (iter1 != parsedElements.end()) {
            //std_v_bms_manager
            stdVBmsManagerFiles.push_back(currentJsonString);
        } else {

            target = "rack.json";
            auto iter2 = std::find(parsedElements.begin(), parsedElements.end(), target);
            if (iter2 != parsedElements.end()) {
                //std_bms_rack
                stdBmsRackFiles.push_back(currentJsonString);
            } else {
                //std_bms_manager
                stdBmsManagerFiles.push_back(currentJsonString);
            }

        }

    }

    //This section converts all the config contents vectors into arrays of the same size
    int vBmsManagerSize = stdVBmsManagerFiles.size();

    std::string stdVBmsManagerStrings [vBmsManagerSize];
    for(int i = 0; i < vBmsManagerSize; i++){
        stdVBmsManagerStrings[i] = stdVBmsManagerFiles.back();
        stdVBmsManagerFiles.pop_back();
    }

    int bmsManagerSize = stdBmsManagerFiles.size();

    std::string stdBmsManagerStrings [bmsManagerSize];
    for(int i = 0; i < bmsManagerSize; i++){
        stdBmsManagerStrings[i] = stdBmsManagerFiles.back();
        stdBmsManagerFiles.pop_back();
    }

    int bmsRacksSize = stdBmsRackFiles.size();

    std::string stdBmsRackStrings [bmsRacksSize];
    for(int i = 0; i < bmsRacksSize; i++){
        stdBmsRackStrings[i] = stdBmsRackFiles.back();
        stdBmsRackFiles.pop_back();
    }


    //sends to the vmap all contents from the v_bms_manager files
    for(int i = 0; i < vBmsManagerSize; i++){
        vm->configure_vmapStr(vmap, stdVBmsManagerStrings[i].c_str(), nullptr, nullptr, true);
    }

    for ( int a = 1 ; a <= num_bms; a++) {

        std::string bmsId = fmt::format("bms_{}", a);

        //sends to the vmap all contents from the bms_manager files
        for(int b = 0; b < bmsManagerSize; b++){
            std::string currBmsManagerString = stdBmsManagerStrings[b];
            currBmsManagerString = ReplaceString(currBmsManagerString, std::string("##BMS_ID##"), std::string(bmsId)); 
            vm->configure_vmapStr(vmap, currBmsManagerString.c_str(), nullptr, nullptr, true);
        }


        for ( int c = 1 ; c <= num_racks; c++) {

            std::string rackId = fmt::format("rack_{}", c);

            //sends to the vmap all contents from the bms_rack files
            for(int d = 0; d < bmsRacksSize; d++){
                std::string currBmsRackString = stdBmsRackStrings[d];
                currBmsRackString = ReplaceString(currBmsRackString, std::string("##BMS_ID##"), std::string(bmsId)); 
                currBmsRackString = ReplaceString(currBmsRackString, std::string("##RACK_ID##"), std::string(rackId)); 
                vm->configure_vmapStr(vmap, currBmsRackString.c_str(), nullptr, nullptr, true);
            }
        }
    }

    return 0;
}

