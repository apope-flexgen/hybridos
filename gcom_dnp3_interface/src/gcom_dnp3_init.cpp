#include <string>
#include <cstring>
#include "gcom_dnp3_init.h"

// As of 2/12/2024, these functions aren't currently used for anything...

int contains_flag(int argc, char *argv[], const char *flag)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], flag) == 0)
            return i + 1; // index of flag found, add one to get the corresponding detail
    }
    return -1; // not found
}

std::string extract_flag(int argc, char *argv[], int flag_pos)
{
    std::string current_arg = std::string(argv[flag_pos]);
    size_t flag_start = current_arg.find("=");
    if (flag_start < current_arg.length()) // if a flag=string arg has been given
    {
        return current_arg.substr(flag_start + 1);
    }
    else if (flag_pos < argc - 1) // If a flag / string pair has been given
    {
        return std::string(argv[flag_pos + 1]);
    }
    else
    {
        return "";
    }
}