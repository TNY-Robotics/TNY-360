#pragma once
#include "settings/core.hpp"

struct RobotConfig : public SettingGroup {
    
    RobotConfig() : SettingGroup(nullptr, "") {}

    BEGIN_GROUP(I2cGroup, i2c)
        BEGIN_GROUP(PrimaryGroup, primary)
            PARAM(int, gpio_sda, 21)
            PARAM(int, gpio_scl, 47)
        END_GROUP(primary)

        BEGIN_GROUP(SecondaryGroup, secondary)
            PARAM(int, gpio_sda, 9)
            PARAM(int, gpio_scl, 48)
        END_GROUP(secondary)
    END_GROUP(i2c)

    void loadFromStream(std::istream &inputStream);
    void loadFromString(const char* jsonString);
    size_t saveToBuffer(char* outputBuffer, size_t maxLen);
    bool saveToStream(std::ostream &outputStream);
};

extern RobotConfig config;