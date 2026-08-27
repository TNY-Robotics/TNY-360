#include "settings/settings.hpp"

void RobotConfig::loadFromStream(std::istream &inputStream)
{
    JsonDocument doc;
    if (!deserializeJson(doc, inputStream))
    {
        JsonObject rootObj = doc.as<JsonObject>();
        for (auto* child : children) child->loadFromJson(rootObj);
    }
}

void RobotConfig::loadFromString(const char* jsonString)
{
    JsonDocument doc;
    if (!deserializeJson(doc, jsonString))
    {
        JsonObject rootObj = doc.as<JsonObject>();
        for (auto* child : children) child->loadFromJson(rootObj);
    }
}

size_t RobotConfig::saveToBuffer(char* outputBuffer, size_t maxLen)
{
    JsonDocument doc;
    JsonObject rootObj = doc.to<JsonObject>();
    for (auto* child : children)
    {
        child->saveToJson(rootObj);
    }

    return serializeJsonPretty(doc, outputBuffer, maxLen);
}

bool RobotConfig::saveToStream(std::ostream &outputStream)
{
    JsonDocument doc;
    JsonObject rootObj = doc.to<JsonObject>();
    for (auto* child : children)
    {
        child->saveToJson(rootObj);
    }
    
    size_t bytesWritten = serializeJsonPretty(doc, outputStream);
    return bytesWritten > 0;
}