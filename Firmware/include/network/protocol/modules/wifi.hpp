#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/BinaryWriter.hpp"
#include "common/Log.hpp"
#include "network/WiFiManager.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace WiFi
{
    constexpr uint8_t MODULE_ID = 0x10;

    /** <API_REF>
     * @module wifi 0x10
     * @action ConnectToAP 0x00
     * @desc Connects to a Wi-Fi access point with the given SSID and password.
     * @arg ssid string The SSID of the access point to connect to.
     * @arg password string The password of the access point to connect to.
     * @impl done
     */
    static void ConnectToAP(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        char ssid[32];
        if (reader.readString(ssid, sizeof(ssid)) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }
        char password[64];
        if (reader.readString(password, sizeof(password)) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        if (man.connectToAP(ssid, password, true, true) != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action DisconnectFromAP 0x01
     * @desc Disconnects from the currently connected access point.
     * @impl done
     */
    static void DisconnectFromAP(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        if (man.disconnectFromAP() != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action HasStoredAPCredentials 0x02
     * @desc Returns whether a known access point is stored in the robot's memory.
     * @result has_credentials bool Whether a known access point is stored in the robot's memory
     * @impl done
     */
    static void HasStoredAPCredentials(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        bool has_credentials = false;
        if (man.hasStoredAPCredentials(has_credentials) != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok, (uint8_t*)&has_credentials, sizeof(has_credentials));
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action ForgetStoredAPCredentials 0x03
     * @desc Forgets the stored access point credentials in the robot's memory.
     * @impl done
     */
    static void ForgetStoredAPCredentials(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        if (man.forgetStoredAPCredentials() != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action StartAP 0x04
     * @desc Starts the robot's access point.
     * @impl done
     */
    static void StartAP(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        if (man.startAP() != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action StopAP 0x05
     * @desc Stops the robot's access point.
     * @impl done
     */
    static void StopAP(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        if (man.stopAP() != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action GetState 0x06
     * @desc Returns the current state of the WiFiManager, see WiFiManager::State for more details.
     * @result state uint8 Current state of the WiFiManager.
     * @impl done
     */
    static void GetState(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        WiFiManager::State state = man.getState();
        ctx.respond(ResponseStatus::Ok, (uint8_t*)&state, sizeof(state));
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action GetMode 0x07
     * @desc Returns the current mode of the WiFiManager, see WiFiManager::Mode for more details.
     * @result mode uint8 Current mode of the WiFiManager, deduced from the current state.
     * @impl done
     */
    static void GetMode(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        WiFiManager::Mode mode = man.getMode();
        ctx.respond(ResponseStatus::Ok, (uint8_t*)&mode, sizeof(mode));
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action GetSSID 0x08
     * @desc Returns the SSID of the currently connected WiFi network.
     * @result ssid string The SSID of the currently connected WiFi network.
     * @impl done
     */
    static void GetSSID(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        const char* ssid = man.getSSID();
        
        uint8_t data[strlen(ssid) + 3]; // 2 bytes for length + 1 byte for null terminator
        BinaryWriter writer(data, sizeof(data));
        if (Status err = writer.writeString(ssid); err != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok, data, sizeof(data));
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action GetIPAddr 0x09
     * @desc Returns the IP address of the robot on the currently connected WiFi network.
     * @result ip_addr string The IP address of the robot.
     * @impl done
     */
    static void GetIPAddr(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        const char* ip_addr = man.getIPAddr();
        uint8_t data[strlen(ip_addr) + 3]; // 2 bytes for length + 1 byte for null terminator
        BinaryWriter writer(data, sizeof(data));
        if (Status err = writer.writeString(ip_addr); err != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok, data, sizeof(data));
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action GetMACAddr 0x0A
     * @desc Returns the MAC address of the robot.
     * @result mac_addr string The MAC address of the robot.
     * @impl done
     */
    static void GetMACAddr(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        const char* mac_addr = man.getMACAddr();
        uint8_t data[strlen(mac_addr) + 3]; // 2 bytes for length + 1 byte for null terminator
        BinaryWriter writer(data, sizeof(data));
        if (Status err = writer.writeString(mac_addr); err != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok, data, sizeof(data));
    }

    /** <API_REF>
     * @module wifi 0x10
     * @action ScanAP 0x0B
     * @desc Scans for available Wi-Fi access points and returns them as a list.
     * @result nb_aps uint16 The number of access points found.
     * @result aps APDescription[] An array of APDescription structures, each containing the SSID, RSSI, and encryption type of an access point.
     * @impl done
     */
    static void ScanAP(const RequestContext& ctx, const uint8_t* payload)
    {
        WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();
        man.scanAP([](const APDescription* ap_list, size_t ap_count, void* user_data) {
            const RequestContext* ctx = static_cast<const RequestContext*>(user_data);
            if (!ctx)
            {
                LOG_ERROR(WiFiManager::TAG, "ScanAP callback called with null context");
                return;
            }

            size_t total_length = sizeof(uint16_t); // For the number of APs
            for (size_t i = 0; i < ap_count; ++i)
            {
                total_length += sizeof(APDescription::ssid); // SSID
                total_length += sizeof(APDescription::rssi); // RSSI
                total_length += sizeof(APDescription::authmode); // Authentication mode
            }

            uint8_t* response_data = new uint8_t[total_length];
            BinaryWriter writer(response_data, total_length);

            // Write the number of APs
            writer.write((uint16_t) ap_count);

            // Write each AP's details
            for (size_t i = 0; i < ap_count; ++i)
            {
                writer.write(ap_list[i].ssid);
                writer.write<int8_t>(ap_list[i].rssi);
                writer.write<uint8_t>(static_cast<uint8_t>(ap_list[i].authmode));
            }

            ctx->respond(ResponseStatus::Ok, response_data, total_length);
            delete[] response_data;
        }, (void*)&ctx);
    }

    static ActionCallback actions[] = {
        ConnectToAP,               // 0x00
        DisconnectFromAP,          // 0x01
        HasStoredAPCredentials,    // 0x02
        ForgetStoredAPCredentials, // 0x03
        StartAP,                   // 0x04
        StopAP,                    // 0x05
        GetState,                  // 0x06
        GetMode,                   // 0x07
        GetSSID,                   // 0x08
        GetIPAddr,                 // 0x09
        GetMACAddr,                // 0x0A
        ScanAP,                   // 0x0B
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}