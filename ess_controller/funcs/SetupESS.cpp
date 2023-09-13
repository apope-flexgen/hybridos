
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
    int SetupESS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
};


// Dummy Setup Function
int SetupESS(varsmap &vmap, varmap &amap, const char* _aname, fims* p_fims, assetVar* aV)
{
    VarMapUtils *vm = aV->am->vm;


   //Use global string variables to get stdcfg path for ess
    std::string stdcfgEssConfigPath = stdcfgPath + stdcfgEssDir;

    if (std::filesystem::exists(stdcfgEssConfigPath)) {
        // Directory exists, you can proceed with opening it
        // ... your code to open and process the directory ...
    } else {
        std::cerr << "Directory does not exist: " << stdcfgEssConfigPath << std::endl;
    }

    //Create vector and populate it with all filenames within given path from above
    std::vector<std::string> fileNamesVector;
    for (const auto & entry : std::filesystem::directory_iterator(stdcfgEssConfigPath)) {
        std::filesystem::path fileNamePath = entry.path();
        std::string fileName = fileNamePath.string();
        fileNamesVector.push_back(fileName);
    }


    //Create vectors for each of the ess files
    std::vector<std::string> stdEssManagerFiles;


    //size of the filenames vector which is a value that is frequently used
    int fileNamesVectorSize =  fileNamesVector.size();



    //for loop to cycle through all of the filenames and get their respective file contents then convert them to strings
    //after that it pushes the data to the correct vector dependent on what type of file it is
    for(int i = 0; i < fileNamesVectorSize; i++){

        // printf("\n\n BOOM -> 6 \n\n");


        //get the first filename by looking at the back of the vector
        std::string currFileName = fileNamesVector.back();
        fileNamesVector.pop_back();

        //this section should take the path + filename and end up giving the contents string in "currentJsonString" 

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

        stdEssManagerFiles.push_back(currentJsonString);


    }

    //This section converts all the config contents vectors into arrays of the same size
    int essManagerSize = stdEssManagerFiles.size();

    std::string stdEssManagerStrings [essManagerSize];
    for(int i = 0; i < essManagerSize; i++){
        stdEssManagerStrings[i] = stdEssManagerFiles.back();
        stdEssManagerFiles.pop_back();
    }

    //sends to the vmap all contents from the v_ess_manager files
    for(int i = 0; i < essManagerSize; i++){
            vm->configure_vmapStr(vmap, stdEssManagerStrings[i].c_str(), aV->am, nullptr, true);
    }

    return 0;
}

