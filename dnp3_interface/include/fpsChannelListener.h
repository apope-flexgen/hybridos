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
#ifndef ASIODNP3_FPSCHANNELLISTENER_H
#define ASIODNP3_FPSCHANNELLISTENER_H

#include "opendnp3/util/Uncopyable.h"
#include "opendnp3/channel/IChannelListener.h"

#include <iostream>
#include <memory>

#include "dnp3_utils.h"

namespace opendnp3
{

/**
 * Callback interface for receiving information about a running channel
 */
class fpsChannelListener final : public IChannelListener, private opendnp3::Uncopyable
{
public:
    virtual void OnStateChange(opendnp3::ChannelState state) override
    {
        // TODO emit events
        if (sysdb->enable_state_events)
        {
            std::string cstate = opendnp3::ChannelStateSpec::to_string(state);
            FPS_ERROR_PRINT("fps channel state change: [%s]\n", cstate.c_str());
            char message[1024];
            snprintf(message, sizeof(message), "state change [%s]"
                        , cstate.c_str());

            emit_event_filt(sysdb, nullptr, message, 1);
        }
    }

    static std::shared_ptr<IChannelListener> Create(sysCfg* fpsDB)
    {
        return std::make_shared<fpsChannelListener>(fpsDB);
    }

    fpsChannelListener(sysCfg* fpsDB) {sysdb = fpsDB;};
    sysCfg* sysdb;
};

} 

#endif
