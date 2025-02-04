
/*
*   fpsCommandHandler.h
* author : pwilshire
*  5, Mat 2020
* start and end are used to signify the start ans completion of the response process   for an message from the master to the outstation
*
*/
#ifndef DNP3_fpsCommandHandler_H
#define DNP3_fpsCommandHandler_H

#include <opendnp3/outstation/ICommandHandler.h>

#include "dnp3_utils.h"

#include <memory>

class fpsCommandHandler final : public opendnp3::ICommandHandler
{
public:
    fpsCommandHandler(sysCfg* fpsDB){sysdb = fpsDB;};

    static std::shared_ptr<ICommandHandler> Create(sysCfg* fpsDB)
    {
        return std::make_shared<fpsCommandHandler>(fpsDB);
    }

    void Begin() override;
    void End() override;

    opendnp3::CommandStatus Select(const opendnp3::ControlRelayOutputBlock& command, uint16_t index) override;

    opendnp3::CommandStatus Operate(const opendnp3::ControlRelayOutputBlock& command, uint16_t index, IUpdateHandler& handler, opendnp3::OperateType opType) override;

    opendnp3::CommandStatus Select(const opendnp3::AnalogOutputInt16& command, uint16_t index) override;

    opendnp3::CommandStatus Operate(const opendnp3::AnalogOutputInt16& command, uint16_t index, IUpdateHandler& handler, opendnp3::OperateType opType) override;

    opendnp3::CommandStatus Select(const opendnp3::AnalogOutputInt32& command, uint16_t index) override;

    opendnp3::CommandStatus Operate(const opendnp3::AnalogOutputInt32& command, uint16_t index, IUpdateHandler& handler, opendnp3::OperateType opType) override;

    opendnp3::CommandStatus Select(const opendnp3::AnalogOutputFloat32& command, uint16_t index);

    opendnp3::CommandStatus Operate(const opendnp3::AnalogOutputFloat32& command, uint16_t index, IUpdateHandler& handler, opendnp3::OperateType opType) override;

    opendnp3::CommandStatus Select(const opendnp3::AnalogOutputDouble64& command, uint16_t index) override
    { return opendnp3::CommandStatus::NOT_SUPPORTED; }

    opendnp3::CommandStatus Operate(const opendnp3::AnalogOutputDouble64& command, uint16_t index, IUpdateHandler& handler, opendnp3::OperateType opType) override
    { return opendnp3::CommandStatus::NOT_SUPPORTED; }

private:

    opendnp3::CommandStatus GetPinAndState(uint16_t index, opendnp3::OperationType code, uint8_t& gpio, bool& state);

    sysCfg* sysdb;
};

#endif //DNP3_fpsCommandHandler_H
