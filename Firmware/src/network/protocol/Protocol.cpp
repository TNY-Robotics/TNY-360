#include "network/protocol/Protocol.hpp"
#include "common/Log.hpp"

#include "network/protocol/modules/system.hpp"
#include "network/protocol/modules/protocol.hpp"
#include "network/protocol/modules/gait.hpp"
#include "network/protocol/modules/body.hpp"
#include "network/protocol/modules/leg.hpp"
#include "network/protocol/modules/joint.hpp"
#include "network/protocol/modules/motor.hpp"
#include "network/protocol/modules/imu.hpp"
#include "network/protocol/modules/power.hpp"
#include "network/protocol/modules/adc.hpp"
#include "network/protocol/modules/i2c.hpp"
#include "network/protocol/modules/wifi.hpp"

namespace Protocol
{
    Dispatcher dispatcher;
}

Error Protocol::Init()
{
    System::Register(dispatcher);
    Protocol::Register(dispatcher);
    Gait::Register(dispatcher);
    Body::Register(dispatcher);
    Leg::Register(dispatcher);
    Joint::Register(dispatcher);
    Motor::Register(dispatcher);
    IMU::Register(dispatcher);
    Power::Register(dispatcher);
    ADC::Register(dispatcher);
    I2C::Register(dispatcher);
    WiFi::Register(dispatcher);
    // ErrorHandle(ErrorStruct::ProtocolInitFailed);
    return Error::None;
}

Error Protocol::Deinit()
{
    return Error::None;
}

Protocol::Dispatcher& Protocol::GetDispatcher()
{
    return dispatcher;
}


void Protocol::Dispatcher::registerModule(uint8_t module_id, ActionCallback* actions, uint8_t nb_actions) {
    this->modules[module_id].actions = actions;
    this->modules[module_id].nb_actions = nb_actions;
}

void Protocol::Dispatcher::handlePacket(ITransport* transport, void* context, const uint8_t* buffer, size_t len) {
    if (len < sizeof(MessageHeader))
    {
        LOG_DEBUG(TAG, "Packet is too small (%u < %d)", len, sizeof(MessageHeader));
        return;
    }

    const MessageHeader* header = reinterpret_cast<const MessageHeader*>(buffer);
    
    if (header->type != MessageType::Request)
    {
        LOG_DEBUG(TAG, "Packet isn't of type request (%u != 1)", header->type);
        return;
    }

    RequestContext ctx = {
        .transport = transport,
        .transport_context = context,
        .msg_id = header->msg_id,
        .expected_len = header->length
    };
    
    uint8_t module_id = header->cmd_id & 0xFF;
    uint8_t action_id = (header->cmd_id >> 8) & 0xFF;

    Module& mod = modules[module_id];
    if (mod.actions == nullptr) {
        LOG_DEBUG(TAG, "Module %u not found", module_id);
        ctx.respond(ResponseStatus::UnknownModule);
        return; 
    }

    if (action_id >= mod.nb_actions || mod.actions[action_id] == nullptr) {
        LOG_DEBUG(TAG, "Action %u not found in module %u", action_id, module_id);
        ctx.respond(ResponseStatus::UnknownAction);
        return;
    }
    const uint8_t* payload = buffer + sizeof(MessageHeader);

    mod.actions[action_id](ctx, payload);
}