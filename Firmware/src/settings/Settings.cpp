#include "settings/Settings.hpp"
#include "common/LittleFS.hpp"
#include "common/Log.hpp"
#include "esp_system.h"
#include "settings/V1.hpp"
#include "settings/V2.hpp"
#include <fstream>

namespace Settings
{
    constexpr static const char* TAG = "Settings";

    RobotConfig config;
    bool initialized = false;

    /// INTERNAL
    Status __save();
    Status __load();

    Status Init()
    {
        if (initialized)
        {
            LOG_DEBUG(TAG, "Settings already initialized");
            return Status::Ok;
        }

        // Here we should first initialize the NVS and check for a saved robot version (to load the right config)
        // if not found, we should run the detection and save it.
        // But for now, we will just load the V1 config.
        // FIXME
        config = CONFIG_V1;

        if (Status err = LittleFS::Init(); err != Status::Ok)
        {
            return err;
        }

        // First load the settings from the settings.json file in userdata (if it exists)
        if (Status err = __load(); err != Status::Ok)
        {
            return err;
        }

        // Save it directly
        // WHY ? Because in case of new setting fields, we want to make sure the settings.json file is updated with the new fields :)
        // FIXME : There's a problem in this workflow here, we should check if the file changed before saving it (+ other things i don't remember, good luck future me)
        if (Status err = __save(); err != Status::Ok)
        {
            return err;
        }

        initialized = true;
        return Status::Ok;
    }
    
    Status Deinit()
    {
        return Status::Ok;
    }
    
    RobotConfig GetConfig()
    {
        return config;
    }
    
    Status SaveAndReboot()
    {
        if (Status err = __save(); err != Status::Ok)
        {
            return err;
        }
        esp_restart();
        return Status::Ok;
    }

    Status __load()
    {
        // Try and load the settings.json file in userdata
        size_t fileSize = 0;
        char path[32];
        snprintf(path, sizeof(path), "%s/settings.json", LittleFS::USERDATA_ROOT_FOLDER);
        if (Status err = LittleFS::GetFileSize(path, fileSize); err != Status::Ok)
        {
            // File doesn't exist, create a new one with default settings
            if (Status err = __save(); err != Status::Ok)
            {
                return err;
            }
            // Nothing to do, it will match the current settings
            return Status::Ok;
        }

        // File exists, load it
        std::ifstream inputFile(path);
        if (!inputFile.is_open())
        {
            return Status::Failure;
        }
        config.loadFromStream(inputFile);
        inputFile.close();
        return Status::Ok;
    }
    
    Status __save()
    {
        // First copy the current settings into .bak file
        size_t fileSize = 0;
        char from_path[32];
        char to_path[32];
        snprintf(from_path, sizeof(from_path), "%s/settings.json", LittleFS::USERDATA_ROOT_FOLDER);
        snprintf(to_path, sizeof(to_path), "%s/settings.json.bak", LittleFS::USERDATA_ROOT_FOLDER);

        // If the original file exists, copy it to the backup file
        if (Status err = LittleFS::GetFileSize(from_path, fileSize); err == Status::Ok)
        {
            if (FILE* fromFile = fopen(from_path, "rb"))
            {
                if (FILE* toFile = fopen(to_path, "wb"))
                {
                    char buffer[256];
                    size_t bytesRead;
                    while ((bytesRead = fread(buffer, 1, sizeof(buffer), fromFile)) > 0)
                    {
                        fwrite(buffer, 1, bytesRead, toFile);
                    }
                    fclose(toFile);
                }
                fclose(fromFile);
            }
        }

        // Now, save the current settings to the original file (sexy content streaming here that's cool)
        std::ofstream outputFile(from_path);
        if (!outputFile.is_open())
        {
            return Status::Failure;
        }
        RobotConfig currentConfig = GetConfig();
        currentConfig.saveToStream(outputFile);
        outputFile.close();
        return Status::Ok;
    }
}
