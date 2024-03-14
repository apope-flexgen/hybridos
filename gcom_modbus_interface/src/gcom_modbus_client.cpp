// gcom_modbus_client.cpp
// s .reynolds & phil wilshire
// 11_01_2023
// self review 11_22_2023


#include <map>
#include <any>
#include <iostream>
#include <csignal>
#include <string>
#include <sstream>
#include <memory>
#include <ios>

#include <unistd.h>
#include <string.h>

#include "logger/logger.h"
#include "gcom_config.h"
#include "gcom_fims.h"
#include "gcom_timer.h"
#include "gcom_iothread.h"
#include "gcom_modbus_decode.h"
#include "shared_utils.h"
#include "gcom_stats.h"
#include "gcom_utils.h"
#include "version.h"


using namespace std::chrono_literals;
using namespace std::string_view_literals;


// stick the raw (any) config in here
std::map<std::string, std::any> gcom_map;


// This is the master config
struct cfg myCfg;

void signal_handler(int sig)
{
    myCfg.keep_running = false;
    FPS_INFO_LOG("Received signal: [%s]", strsignal(sig));
    signal(sig, SIG_DFL);
}



void print_help()
{
    std::cout << " Gcom Modbus Client :" 
        << std::endl;
    std::cout << " use  gcom_modbus_client <-c | -u > <config name> [override options] " 
        << std::endl;

    std::cout << "\n    override options :" 
        << std::endl;

    gcom_show_overrides();
}


int main(const int argc, const char *argv[]) noexcept
{   
    myCfg.git_version_info.init();
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    bool debug = false;
    myCfg.use_dbi = false;
    int command_line_overrides_start = 2;
    myCfg.filename = std::string("nofile");
    if (argc == 1) // just the name of the program
    {
        print_help();
        return 0;
    }

    if (argc > 1) // the name of the program plus some stuff 
    {
        command_line_overrides_start = 2;
        myCfg.filename = argv[1];
        if ((myCfg.filename == "-c") || (myCfg.filename == "-u"))
        {
            command_line_overrides_start = 3;
            if (myCfg.filename == "-u")
            {
                myCfg.use_dbi = true;
            }
            if (argc > 2)
            {
                myCfg.filename = argv[2];
            }
        }
    }   
    if (myCfg.filename == "nofile") // just the name of the program
    {
        print_help();
        printf(" gcom_modbus_client :: No config file specified");
        return 0;
    }

    const char *filename =  myCfg.filename.c_str();
    {
        std::string logger_filename = removeSlashesAndExtension(myCfg.filename);
        Logging::Init(logger_filename, (const int)0, (const char **)nullptr);
    }

    FPS_INFO_LOG("build: %s", myCfg.git_version_info.get_build());
    FPS_INFO_LOG("commit: %s", myCfg.git_version_info.get_commit());
    FPS_INFO_LOG("tag: %s", myCfg.git_version_info.get_tag());



    myCfg.connection.debug = false;
    
    //bool fileOk = gcom_load_overrides(myCfg, command_line_overrides_start, argc, argv);
    bool fileOk = gcom_load_cfg_file(gcom_map, filename, myCfg, debug);
    fileOk |= gcom_load_overrides(myCfg, command_line_overrides_start, argc, argv);

    // move this here to allow the "ip:" override to have an effect.
    if (!myCfg.connection.is_RTU)
    {
        char new_ip[HOST_NAME_MAX + 1];
        new_ip[HOST_NAME_MAX] = '\0';
        auto ret = hostname_to_ip(myCfg.connection.ip_address, new_ip, HOST_NAME_MAX);
        if (ret == 0)
        {
            myCfg.connection.ip_address = new_ip;
        }
        else
        {
            FPS_ERROR_LOG("ip_address \"%s\" isn't valid or can't be found from the local service file", myCfg.connection.ip_address);
            return 0;
        }
    }


    if (!fileOk)
    {
        FPS_ERROR_LOG("Unable to locate or load config file, Quitting");
        return -1;
    }

    if (fileOk)
    {
        gcom_setup_pubs(gcom_map, myCfg, 2.0, debug);

        if (myCfg.connection.stats_uri.length() > 0){
            gcom_setup_stats_pubs(myCfg, 5);
        }
        FPS_INFO_LOG("Subs found:");

        for (auto &mysub : myCfg.subs)
        {
            FPS_INFO_LOG("\t" + mysub);
        }

        start_process(myCfg);
    
        start_fims(myCfg.subs, myCfg);
        // 100 mS pause
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (!myCfg.fims_running)
        {
            FPS_ERROR_LOG("Unable to start up fims, Quitting");
            stop_process(myCfg);
            return -1;
        }
        myCfg.fims_connected = true;
        StartThreads(myCfg, debug);
        runTimer();


        while (myCfg.keep_running)
        {
            std::this_thread::sleep_for(1000ms);
        }

        FPS_INFO_LOG("Shutting down");
        stopTimer();
        StopThreads(myCfg, false);
        stop_process(myCfg);
        stop_fims(myCfg);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

}



