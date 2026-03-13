/**
 * HEY ! This file is gonna get pretty big, and it could be a good idea to split it later.
 **/

#pragma once
#include "common/Log.hpp"
#include "Robot.hpp"

using CallbackResolver = void(*)(const Protocol::Response& res);
using HandlerCallback = void(*)(const Protocol::Request& req, CallbackResolver resolve);

constexpr const char* TAG = "ProtocolCommands";

typedef struct CommandHandler
{
    uint8_t cmd;
    uint8_t len;
    HandlerCallback callback;
} CommandHandler;
