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
#ifndef NEW_FPSMASTERAPPLICATION_H
#define NEW_FPSMASTERAPPLICATION_H

#include <opendnp3/master/IMasterApplication.h>

#include <memory>

#include <iostream>

#include "dnp3_utils.h"

class fpsMasterApplication : public opendnp3::IMasterApplication
{
public:
    fpsMasterApplication(sysCfg* fpsDB){sysdb = fpsDB;};

    static std::shared_ptr<IMasterApplication> Create(sysCfg* fpsDB)
    {
        return std::make_shared<fpsMasterApplication>(fpsDB);
    }

    virtual void OnReceiveIIN(const opendnp3::IINField& iin) override final 
    {
        if(sysdb->debug)
            FPS_ERROR_PRINT ("Running [%s] IINField %u.%u \n", __FUNCTION__, iin.MSB, iin.LSB);
    }

    virtual void OnTaskStart(opendnp3::MasterTaskType type, opendnp3::TaskId id) override final 
    {
        sysdb->taskStart = Now().msSinceEpoch;
        if(sysdb->debug)
            FPS_ERROR_PRINT("Running [%s] TaskID :[%d] Task Type : [%s] Now [%lu] \n"
            , __FUNCTION__, id.GetId(), MasterTaskTypeSpec::to_string(type), Now().msSinceEpoch);
    }
    
   
    virtual void OnTaskComplete(const opendnp3::TaskInfo& info) override final {
        auto end =  Now().msSinceEpoch;
        auto elapsed = end - sysdb->taskStart; 
        //if(sysdb->debug)
        //std::cout << "Running ["<<__FUNCTION__<<" TaskID :"<< id << " Task Type :"<< type <<"]\n";
         if(sysdb->debug || (info.result != TaskCompletion::SUCCESS) || ((int)elapsed > sysdb->maxElapsed) )
        {
            if(sysdb->debug || (int)info.result != sysdb->last_result)
            {
                FPS_ERROR_PRINT("Running [%s] result [%s]  start[%lu] elapsed [%lu] \n"
                    , __FUNCTION__
                    , TaskCompletionSpec::to_string(info.result)
                    , sysdb->taskStart
                    , elapsed
                    );    //Code for adding timestamp
            }
        }
        bool flag =(info.result == TaskCompletion::SUCCESS);
        if (sysdb->pubOutputs)
        {
            sysdb->addPubOutputs();
        }
        sysdb->pubUris(flag);
        sysdb->pref++;
        
    }

    virtual bool AssignClassDuringStartup() override final
    {
        return false;
    }

    virtual void ConfigureAssignClassRequest(const opendnp3::WriteHeaderFunT& fun) override final {}

    virtual opendnp3::UTCTimestamp Now() override final;

    virtual void OnStateChange(opendnp3::LinkStatus value) override final 
    {  
        std::string cstate = LinkStatusSpec::to_string(value);
        char message[1024];
        snprintf(message, sizeof(message), "Link Status Change [%s]"
                    , cstate.c_str()
                    );
        FPS_ERROR_PRINT("State Change message [%s]\n"
                        , message
                        );
        emit_event_filt(sysdb, nullptr, message, 1);
    }    
    
    sysCfg* sysdb;

};

#endif
