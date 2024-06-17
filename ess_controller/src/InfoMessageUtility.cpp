#ifndef INFOMESSAGEUTILITY_CPP
#define INFOMESSAGEUTILITY_CPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include <cctype>

namespace InfoMessageUtility
{
std::string handleCmdTimedOutFail(const std::string& control, const std::string& controlSuccess)
{
    return "HandleCmd for " + control + " has timed out and " + controlSuccess + " was still false.";
}

std::string handleCmdInProgress(const std::string& control)
{
    return "HandleCmd for " + control + " is not complete yet";
}

std::string controlSuccessMessage(const std::string& control, const std::string& aname)
{
    return "Successful process of " + control + " for the " + aname;
}

std::string controlAlarmMessage(const std::string& control, std::string& aname)
{
    for (auto& c : aname)
    {
        c = std::toupper(c);
    }
    return aname + " " + control +
           " - Preconditions were not met or failed to verify command was sent. View ESS Events page for more details";
}

std::string verifyControlAlarmMessage(const std::string& verify, std::string& aname)
{
    for (auto& c : aname)
    {
        c = std::toupper(c);
    }
    return aname + " " + verify + " - Command was sent, but the expected result was not observed before timeout";
}

std::string handlerEventMessage(const std::string& status, const std::string& scheduledFunc,
                                const std::string& infoMessage)
{
    return status + " | " + scheduledFunc + " | " + infoMessage;
}

std::string logicCheckFail(const std::string& logic)
{
    return "This logic check failed: " + logic;
}
}

#endif
