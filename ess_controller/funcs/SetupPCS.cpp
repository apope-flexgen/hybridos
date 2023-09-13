
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
    int SetupPCS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
};

// Dummy Setup Function
int SetupPCS(varsmap &vmap, varmap &amap, const char* _aname, fims* p_fims, assetVar* aV)
{
    VarMapUtils *vm = aV->am->vm;

    int num_modules = 0;
    if (aV->gotParam("num_modules"))
    {
        num_modules = aV->getiParam("num_modules");
    }
    int num_pcs = 2;
    if (aV->gotParam("num_pcs"))
    {
        num_pcs = aV->getiParam("num_pcs");
    }

   //Use global string variables to get stdcfg path for pcs
    std::string stdcfgPcsConfigPath = stdcfgPath + stdcfgPcsDir;

    if (std::filesystem::exists(stdcfgPcsConfigPath)) {
        // Directory exists, you can proceed with opening it
        // ... your code to open and process the directory ...
    } else {
        std::cerr << "Directory does not exist: " << stdcfgPcsConfigPath << std::endl;
    }

    //Create vector and populate it with all filenames within given path from above
    std::vector<std::string> fileNamesVector;
    for (const auto & entry : std::filesystem::directory_iterator(stdcfgPcsConfigPath)) {
        std::filesystem::path fileNamePath = entry.path();
        std::string fileName = fileNamePath.string();
        fileNamesVector.push_back(fileName);
    }


    //Create vectors for each of the types of stdcfg pcs files. v_pcs_manager, pcs_manager, pcs_module
    std::vector<std::string> stdVPcsManagerFiles;
    std::vector<std::string> stdPcsManagerFiles;
    std::vector<std::string> stdPcsModuleFiles;

    //size of the filenames vector which is a value that is frequently used
    int fileNamesVectorSize =  fileNamesVector.size();



    //for loop to cycle through all of the filenames and get their respective file contents then convert them to strings
    //after that it pushes the data to the correct vector dependent on what type of file it is
    for(int i = 0; i < fileNamesVectorSize; i++){

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



        currFileName.erase(0, currFileName.find(stdcfgPcsConfigPath) + stdcfgPcsConfigPath.length());

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
            //std_v_pcs_manager
            stdVPcsManagerFiles.push_back(currentJsonString);
        } else {

            target = "module.json";
            auto iter2 = std::find(parsedElements.begin(), parsedElements.end(), target);
            if (iter2 != parsedElements.end()) {
                //std_pcs_module
                stdPcsModuleFiles.push_back(currentJsonString);
            } else {
                //std_pcs_manager
                stdPcsManagerFiles.push_back(currentJsonString);
            }

        }

    }

    //This section converts all the config contents vectors into arrays of the same size
    int vPcsManagerSize = stdVPcsManagerFiles.size();

    std::string stdVPcsManagerStrings [vPcsManagerSize];
    for(int i = 0; i < vPcsManagerSize; i++){
        stdVPcsManagerStrings[i] = stdVPcsManagerFiles.back();
        stdVPcsManagerFiles.pop_back();
    }

    int pcsManagerSize = stdPcsManagerFiles.size();

    std::string stdPcsManagerStrings [pcsManagerSize];
    for(int i = 0; i < pcsManagerSize; i++){
        stdPcsManagerStrings[i] = stdPcsManagerFiles.back();
        stdPcsManagerFiles.pop_back();
    }

    int pcsModulesSize = stdPcsModuleFiles.size();

    std::string stdPcsModuleStrings [pcsModulesSize];
    for(int i = 0; i < pcsModulesSize; i++){
        stdPcsModuleStrings[i] = stdPcsModuleFiles.back();
        stdPcsModuleFiles.pop_back();
    }


    //sends to the vmap all contents from the v_pcs_manager files
    for(int i = 0; i < vPcsManagerSize; i++){
            vm->configure_vmapStr(vmap, stdVPcsManagerStrings[i].c_str(), aV->am, nullptr, true);
    }


    for ( int a = 1 ; a <= num_pcs; a++) {

        std::string pcsId = fmt::format("pcs_{}", a);
        // printf("expanding %s \n\n", pcsId.c_str());

        //sends to the vmap all contents from the pcs_manager files
        for(int b = 0; b < pcsManagerSize; b++){
            std::string currPcsManagerString = stdPcsManagerStrings[b];
            currPcsManagerString = ReplaceString(currPcsManagerString, std::string("##PCS_ID##"), std::string(pcsId)); 
            vm->configure_vmapStr(vmap, currPcsManagerString.c_str(), aV->am, nullptr, true);
        }


        for ( int c = 1 ; c <= num_modules; c++) {

            std::string moduleId = fmt::format("module_{}", c);

            //sends to the vmap all contents from the pcs_module files
            for(int d = 0; d < pcsModulesSize; d++){
                std::string currPcsModuleString = stdPcsModuleStrings[d];
                currPcsModuleString = ReplaceString(currPcsModuleString, std::string("##PCS_ID##"), std::string(pcsId)); 
                currPcsModuleString = ReplaceString(currPcsModuleString, std::string("##MODULE_ID##"), std::string(moduleId)); 
                vm->configure_vmapStr(vmap, currPcsModuleString.c_str(), aV->am, nullptr, true);
            }
        }
    }

    return 0;
}

