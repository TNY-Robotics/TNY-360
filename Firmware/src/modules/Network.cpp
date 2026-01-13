#include "modules/Network.hpp"
#include "modules/WebServer.hpp"
#include "modules/WebSocket.hpp"
#include "modules/DNSServer.hpp"
#include "modules/CaptivePortal.hpp"
#include "modules/WebInterface.hpp"
#include "modules/Log.hpp"

namespace Network
{
    Error SetupSTA()
    {
        if (WebServer::Init() != Error::Ok) {
            Log::Add(Log::Level::Error, "WebServer Init failed");
            return Error::Unknown;
        }
        if (WebServer::Start(5621) != Error::Ok) {
            Log::Add(Log::Level::Error, "WebServer Start failed");
            return Error::Unknown;
        }
        if (WebSocket::Init() != Error::Ok) {
            Log::Add(Log::Level::Error, "WebSocket Init failed");
            return Error::Unknown;
        }
        if (WebSocket::Attach() != Error::Ok) {
            Log::Add(Log::Level::Error, "WebSocket Attach failed");
            return Error::Unknown;
        }
        if (WebInterface::Init() != Error::Ok) {
            Log::Add(Log::Level::Error, "WebInterface Init failed");
            return Error::Unknown;
        }
        if (WebInterface::Start(80) != Error::Ok) {
            Log::Add(Log::Level::Error, "WebInterface Start failed");
            return Error::Unknown;
        }
        return Error::Ok;
    }

    Error SetupAP()
    {
        if (WebServer::Init() != Error::Ok) {
            Log::Add(Log::Level::Error, "WebServer Init failed");
            return Error::Unknown;
        }
        if (WebServer::Start(80) != Error::Ok) {
            Log::Add(Log::Level::Error, "WebServer Start failed");
            return Error::Unknown;
        }
        if (DNSServer::Init() != Error::Ok) {
            Log::Add(Log::Level::Error, "DNSServer Init failed");
            return Error::Unknown;
        }
        if (DNSServer::Start() != Error::Ok) {
            Log::Add(Log::Level::Error, "DNSServer Start failed");
            return Error::Unknown;
        }
        if (CaptivePortal::Init() != Error::Ok) {
            Log::Add(Log::Level::Error, "CaptivePortal Init failed");
            return Error::Unknown;
        }
        if (CaptivePortal::Attach() != Error::Ok) {
            Log::Add(Log::Level::Error, "CaptivePortal Attach failed");
            return Error::Unknown;
        }
        return Error::Ok;
    }
}