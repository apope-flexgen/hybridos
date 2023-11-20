#ifndef INFOMESSAGEUTILITY_HPP
#define INFOMESSAGEUTILITY_HPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>


namespace InfoMessageUtility
{
    std::string handleCmdTimedOutFail(const std::string& control, const std::string& controlSuccess);
    std::string handleCmdInProgress(const std::string& control, const std::string& phase);
    std::string controlSuccessMessage(const std::string& control, const std::string& aname);
    std::string controlAlarmMessage(const std::string& control, std::string& aname);
    std::string verifyControlAlarmMessage(const std::string& verify, std::string& aname);
    std::string handlerEventMessage(const std::string& status, const std::string& scheduledFunc, const std::string& infoMessage);
    std::string logicCheckFail(const std::string& logic);
}


#endif
