#pragma once
#include "common/utils.hpp"
#include "common/config.hpp"

namespace Protocol
{
    constexpr const char* TAG = "Protocol";

    enum class ResponseStatus: uint16_t
    {
        Ok,
        UnknownModule,
        UnknownAction,
        InvalidParameters,
        UnknownError,
        NotFound,
        OutOfMemory
    };

    enum class MessageType: uint8_t
    {
        Request = 1,
        Response = 2,
        Event = 3
    };

    enum class MessageFlag: uint8_t
    {
        None = 0,
        RequireAck = 0x01
    };

    struct MessageHeader {
        MessageType type;
        MessageFlag flags;
        uint16_t msg_id;
        union {
            uint16_t cmd_id;   // Request
            ResponseStatus status;   // Response
            uint16_t event_id; // Event
        };
        uint16_t length;
    } __attribute__((packed));

    class ITransport {
    public:
        virtual void sendResponse(void* context, const MessageHeader& header, const uint8_t* payload) = 0;
    };

    struct RequestContext {
        ITransport* transport;
        void* transport_context;
        uint16_t msg_id;
        uint16_t expected_len;

        // Fonction helper pour répondre facilement depuis le callback
        void respond(ResponseStatus status, const uint8_t* payload = nullptr, uint16_t len = 0) const {
            MessageHeader res_header = {
                .type = MessageType::Response,
                .flags = MessageFlag::None,
                .msg_id = this->msg_id,
                .status = status,
                .length = len
            };
            if (transport != nullptr)
                transport->sendResponse(transport_context, res_header, payload);
        }
    };

    using ActionCallback = void(*)(const RequestContext& ctx, const uint8_t* payload);

    struct Module {
        ActionCallback* actions;
        uint8_t nb_actions;
    };

    class Dispatcher {
    private:
        Module modules[256];

    public:
        void registerModule(uint8_t module_id, ActionCallback* actions, uint8_t nb_actions);
        void handlePacket(ITransport* transport, void* context, const uint8_t* buffer, size_t len);
    };

    /**
     * @brief Initialize the Protocol system
     * @return Error Error code indicating success or failure
     */
    Status Init();

    /**
     * @brief Deinitialize the Protocol system
     * @return Error Error code indicating success or failure
     */
    Status Deinit();

    Dispatcher& GetDispatcher();
};