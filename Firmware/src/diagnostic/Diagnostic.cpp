#include "diagnostic/Diagnostic.hpp"
#include "common/LED.hpp"
#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/NVS.hpp"
#include "audio/Speaker.hpp"
#include "audio/SoundMixer.hpp"
#include "drivers/CameraDriver.hpp"
#include "drivers/IMUDriver.hpp"
#include "drivers/PowerDriver.hpp"
#include "drivers/MotorDriver.hpp"
#include "drivers/AnalogDriver.hpp"

namespace Diagnostic
{
    static constexpr const char* TAG = "Diagnostic";
    static bool diag_mode_enabled = false;

    Status Init()
    {
        // Initialize diagnostic system
        return Status::Ok;
    }
    
    Status Denit()
    {
        // Deinitialize diagnostic system
        return Status::Ok;
    }

    void SetDiagnosticMode(bool enable)
    {
        diag_mode_enabled = enable;
    }

    Status RebootInDiagnosticMode()
    {
        NVS::Handle* nvsHandle;
        if (Status err = NVS::Open("boot", &nvsHandle); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Error opening NVS 'boot' namespace : %d", err);
            return Status::Failure;
        }
        // Don't skip diagnostic on next boot (false)
        bool skip_diag = false;
        if (Status err = nvsHandle->set<bool>("skip_diag", skip_diag); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Error writing skip diagnostic flag to NVS : %d", err);
            NVS::Close(nvsHandle);
            return Status::Failure;
        }
        NVS::Close(nvsHandle);
        esp_restart();
        return Status::Ok; // Will never be reached but compiler goes brrr if not here
    }

    bool IsDiagnosticModeEnabled()
    {
        return diag_mode_enabled;
    }

    namespace Check
    {
        namespace
        {
            void check_begin()
            {
                // Clear previous error events
                ::Error::ClearErrorEvents();
            }

            uint8_t check_end()
            {
                // Check if any error events were recorded
                if (::Error::GetErrorCount() <= 0)
                {
                    return 0; // No errors
                }
                // Return the most recent error event ID
                return ::Error::GetErrorEventByIndex(0).eventId;
            }
        }

        uint8_t LED()
        {
            check_begin();
            if (Status err = ::LED::Init(); err != Status::Ok) return check_end();
            // if (Status err = ::LED::Deinit(); err != Status::Ok) return check_end();
            return check_end();
        }

        uint8_t I2C()
        {
            check_begin();
            if (Status status = ::I2C::Init(); status != Status::Ok) return check_end();
            // if (Status status = ::I2C::Deinit(); status != Status::Ok) return check_end();
            return check_end();
        }

        uint8_t Speaker()
        {
            check_begin();
            ::Speaker s;
            ::SoundMixer mixer(s);
            if (Status err = s.init(); err != Status::Ok) return check_end();
            if (Status err = mixer.init(); err != Status::Ok) return check_end();
            if (Status err = mixer.deinit(); err != Status::Ok) return check_end();
            if (Status err = s.deinit(); err != Status::Ok) return check_end();
            return check_end();
        }

        uint8_t Microphone()
        {
            check_begin();
            // TODO
            return check_end();
        }

        uint8_t Camera()
        {
            check_begin();
            ::CameraDriver cam;
            if (Status err = cam.init(); err != Status::Ok) return check_end();
            if (Status err = cam.deinit(); err != Status::Ok) return check_end();
            return check_end();
        }

        uint8_t Laser()
        {
            check_begin();
            // TODO
            return check_end();
        }

        uint8_t IMU()
        {
            check_begin();
            if (Status err = ::IMUDriver::Init(); err != Status::Ok) return check_end();
            if (Status err = ::IMUDriver::Deinit(); err != Status::Ok) return check_end();
            return check_end();
        }

        uint8_t Power()
        {
            check_begin();
            if (Status err = ::PowerDriver::Init(); err != Status::Ok) return check_end();
            if (Status err = ::PowerDriver::Deinit(); err != Status::Ok) return check_end();
            return check_end();
        }

        uint8_t MotorDriver()
        {
            check_begin();
            if (Status err = ::MotorDriver::Init(); err != Status::Ok) return check_end();
            if (Status err = ::MotorDriver::Deinit(); err != Status::Ok) return check_end();
            return check_end();
        }

        uint8_t AnalogReader()
        {
            check_begin();
            if (Status err = ::AnalogDriver::Init(); err != Status::Ok) return check_end();
            if (Status err = ::AnalogDriver::Deinit(); err != Status::Ok) return check_end();
            return check_end();
        }

        uint8_t ControlLoop()
        {
            check_begin();
            // TODO
            return check_end();
        }

        uint8_t DecisionLoop()
        {
            check_begin();
            // TODO
            return check_end();
        }
    }
}