/*
 * Copyright 2013-2019 Automatak, LLC
 *
 * Licensed to Green Energy Corp (www.greenenergycorp.com) and Automatak
 * LLC (www.automatak.com) under one or more contributor license agreements.
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership. Green Energy Corp and Automatak LLC license
 * this file to you under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ASIODNP3_FPSLOGGER_H
#define ASIODNP3_FSPLOGGER_H

#include <opendnp3/logging/ILogHandler.h>
#include <opendnp3/util/Uncopyable.h>

#include <memory>
#include <mutex>
#include "dnp3_utils.h"


namespace opendnp3
{

std::ostringstream& operator<<(std::ostringstream& ss, const opendnp3::LogLevels& filters);

/**
 * LogHandler that prints all log messages to the console
 */
class fpsLogger final : public opendnp3::ILogHandler, private opendnp3::Uncopyable
{

public:

    virtual void log(ModuleId module, const char* id, LogLevel level, char const* location, char const* message);
    static std::shared_ptr<opendnp3::ILogHandler> Create(sysCfg *fpsDB, bool printLocation = false)
    {
        return std::make_shared<fpsLogger>(fpsDB, printLocation);
    };

    fpsLogger(sysCfg* fpsDB, bool printLocation) :  printLocation(printLocation) {sysdb = fpsDB;};

private:
    sysCfg* sysdb;
    bool printLocation;

    std::mutex mutex;
};

} // namespace asiodnp3

#endif
