#pragma once
#include "network/protocol/Protocol.hpp"
#include <esp_system.h>
#include "common/RPC.hpp"
#include "drivers/AnalogDriver.hpp"

struct ADCData {
    float voltages[16];
};

namespace Protocol
{
namespace ADC
{
    constexpr uint8_t MODULE_ID = 0x0F;

    /** <API_REF>
     * @module adc 0x0F
     * @action getAllChannels 0x00
     * @desc Gets the current voltage readings of all ADC channels.
     * @result voltages float32[16] Array of voltage readings for all ADC channels in volts.
      * @impl done
     */
    static void GetAllChannels(const RequestContext& ctx, const uint8_t* payload)
    {
        RPC::ExecuteThreadSafe<ADCData>([](){
            ADCData data;
            AnalogDriver::GetVoltages(data.voltages);
            return data;
        }, [ctx](ADCData data){
            ctx.respond(ResponseStatus::Ok, (uint8_t*) data.voltages, sizeof(data.voltages));
        });
    }


    static ActionCallback actions[] = {
        GetAllChannels, // 0x00
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}