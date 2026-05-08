#pragma once
#include <cstdlib>

namespace ErrorCode
{
    constexpr uint8_t None = 0x00;                      // 0b00000000
    constexpr uint8_t I2cBusPrimaryInitFailed = 0x01;   // 0b00000001
    constexpr uint8_t I2cBusSecondaryInitFailed = 0x02; // 0b00000010
    constexpr uint8_t ScreenInitFailed = 0x03;          // 0b00000011
    constexpr uint8_t MenusInitFailed = 0x04;           // 0b00000100
    constexpr uint8_t SpeakerInitFailed = 0x05;         // 0b00000101
    constexpr uint8_t MixerInitFailed = 0x06;           // 0b00000110
    constexpr uint8_t WiFiInitFailed = 0x07;            // 0b00000111
    constexpr uint8_t WebInterfaceInitFailed = 0x08;    // 0b00001000
    constexpr uint8_t WebSocketInitFailed = 0x09;       // 0b00001001
    constexpr uint8_t UpdateInitFailed = 0x0A;          // 0b00001010
    constexpr uint8_t ProtocolInitFailed = 0x0B;        // 0b00001011
    constexpr uint8_t DriverInitFailed = 0x0C;          // 0b00001100
    constexpr uint8_t ReaderInitFailed = 0x0D;          // 0b00001101
    constexpr uint8_t IMUInitFailed = 0x0E;             // 0b00001110
    constexpr uint8_t CameraInitFailed = 0x0F;          // 0b00001111
}

namespace ErrorStruct
{
    struct ErrorStruct
    {
        uint8_t code;
        const char* msg;
    };

    constexpr ErrorStruct None =                      {0x00, "No error"};
    constexpr ErrorStruct I2cBusPrimaryInitFailed =   {ErrorCode::I2cBusPrimaryInitFailed,   "Failed to initialize primary I2C bus"};
    constexpr ErrorStruct I2cBusSecondaryInitFailed = {ErrorCode::I2cBusSecondaryInitFailed, "Failed to initialize secondary I2C bus"};
    constexpr ErrorStruct ScreenInitFailed =          {ErrorCode::ScreenInitFailed,          "Failed to initialize Screen module"};
    constexpr ErrorStruct MenusInitFailed =           {ErrorCode::MenusInitFailed,           "Failed to initialize Menus system"};
    constexpr ErrorStruct SpeakerInitFailed =         {ErrorCode::SpeakerInitFailed,         "Failed to initialize Speaker module"};
    constexpr ErrorStruct MixerInitFailed =           {ErrorCode::MixerInitFailed,           "Failed to initialize Audio Mixer"};
    constexpr ErrorStruct WiFiInitFailed =            {ErrorCode::WiFiInitFailed,            "Failed to initialize Wi-Fi"};
    constexpr ErrorStruct WebInterfaceInitFailed =    {ErrorCode::WebInterfaceInitFailed,    "Failed to initialize Web Interface"};
    constexpr ErrorStruct WebSocketInitFailed =       {ErrorCode::WebSocketInitFailed,       "Failed to initialize WebSocket"};
    constexpr ErrorStruct UpdateInitFailed =          {ErrorCode::UpdateInitFailed,          "Failed to initialize Update module"};
    constexpr ErrorStruct ProtocolInitFailed =        {ErrorCode::ProtocolInitFailed,        "Failed to initialize protocol"};
    constexpr ErrorStruct DriverInitFailed =          {ErrorCode::DriverInitFailed,          "Failed to initialize Motor Driver"};
    constexpr ErrorStruct ReaderInitFailed =          {ErrorCode::ReaderInitFailed,          "Failed to initialize Analog Reader"};
    constexpr ErrorStruct IMUInitFailed =             {ErrorCode::IMUInitFailed,             "Failed to initialize IMU module"};
    constexpr ErrorStruct CameraInitFailed =          {ErrorCode::CameraInitFailed,          "Failed to initialize Camera module"};
}
