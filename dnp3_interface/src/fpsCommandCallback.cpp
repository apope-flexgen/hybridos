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


#include "opendnp3/master/PrintingCommandResultCallback.h"
#include "fpsCommandCallback.h"

#include <iostream>

namespace opendnp3

// // namespace asiodnp3
{
// remove excess debug
CommandResultCallbackT fpsCommandCallback::Get()
{
    return [](const ICommandTaskResult& result) -> void {
        if(0)std::cout << "Received command result w/ summary: " << TaskCompletionSpec::to_human_string(result.summary) << std::endl;
        auto print = [](const CommandPointResult& res) {
            if(0)
                std::cout << "Func -:"<<__func__
                        <<" Header: " << res.headerIndex << " Index: " << res.index
                        << " State: " << CommandPointStateSpec::to_human_string(res.state)
                        << " Status: " << CommandStatusSpec::to_human_string(res.status)
                        << std::endl;
        };
        result.ForeachItem(print);
    };
}
CommandResultCallbackT fpsCommandCallback::Get(sysCfg* _sysdb)
{
    sysCfg * sys = _sysdb;
    return [sys](const ICommandTaskResult& result) -> void {
        if(sys->debug||((int)result.summary != 0 ))
        {
            if(sys->debug>0)
            {
                    std::cout << "Received command result: summary ["<<int(result.summary)<<"]: " 
                               << TaskCompletionSpec::to_human_string(result.summary) << std::endl;
            }
        }
        sys->last_result = (int)(result.summary);

        auto print = [sys](const CommandPointResult& res) {
            if(sys->debug> 1) // || (res.state != CommandPointState::SUCCESS ))
            {
                if(sys->last_res !=  (int)res.state)
                { 
                    if (1)std::cout  << "Func -:"<<__func__
                        << "Header: " << res.headerIndex << " Index: " << res.index
                        << " Int State: " << (int)res.state
                        << " State: " << CommandPointStateSpec::to_human_string(res.state)
                        << " Status: " << CommandStatusSpec::to_human_string(res.status)
                        << std::endl;
                }
            }
            sys->last_res = (int)(res.state);

        };
        result.ForeachItem(print);
    };
}

} // namespace opendnp3
