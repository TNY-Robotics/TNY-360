#include "modules/Sound.hpp"
#include "modules/Log.hpp"
#include "driver/gpio.h"
#include "driver/i2s_pdm.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <atomic>
#include <vector>
#include <cmath>
#include <memory.h>

namespace Sound
{
    constexpr int SAMPLE_RATE = 44100; // 44.1 kHz
    constexpr float PI = 3.14159265f;
    constexpr int CHUNK_SAMPLES = 1024; // Number of samples per audio chunk

    constexpr gpio_num_t SOUND_GPIO_PIN = GPIO_NUM_1; // GPIO pin for sound output
    constexpr uint32_t   STACK_SIZE     = 4096;
    constexpr uint32_t   QUEUE_LENGTH   = 32; // Max notes in the queue

    bool initialized = false;

    static i2s_chan_handle_t tx_handle = NULL;
    static QueueHandle_t noteQueue = NULL;
    static TaskHandle_t taskHandle = NULL;

    static std::atomic<float> global_volume(0.5f); // Thread-safe
    static std::atomic<bool> is_playing(false);
    static std::atomic<bool> stop_requested(false);

    // audio generation task
    void AudioTask(void *pvParameters)
    {
        Note currentNote;
        // Allocation unique du buffer aligné
        std::vector<int16_t> chunkBuffer(CHUNK_SAMPLES);

        while (true)
        {
            // Attente de la prochaine note
            if (xQueueReceive(noteQueue, &currentNote, portMAX_DELAY) == pdPASS)
            {
                is_playing = true;
                stop_requested = false;

                // Calcul : Combien de samples total pour cette durée ?
                size_t total_samples_needed = (size_t)(currentNote.duration_s * SAMPLE_RATE);
                
                // Calcul : Combien de "paquets" de 1024 on doit envoyer ?
                size_t chunks_count = total_samples_needed / CHUNK_SAMPLES;
                
                // Le reste (si la note ne tombe pas pile sur un multiple de 1024)
                size_t leftover_samples = total_samples_needed % CHUNK_SAMPLES;

                bool is_silence = (currentNote.volume <= 0.001f || currentNote.frequency <= 1.0f);
                float effective_vol = currentNote.volume * global_volume.load();
                float amplitude = 30000.0f * effective_vol;
                
                // Phase locale pour cette note
                float phase = 0.0f; 
                
                // IMPORTANT : Pas de Log ici !

                // 1. Boucle des paquets pleins
                for (size_t c = 0; c < chunks_count; c++)
                {
                    if (stop_requested) break;

                    if (is_silence) {
                         // memset est ultra rapide
                        memset(chunkBuffer.data(), 0, CHUNK_SAMPLES * sizeof(int16_t));
                        phase = 0.0f; 
                    } else {
                        for (size_t i = 0; i < CHUNK_SAMPLES; i++) {
                            chunkBuffer[i] = (int16_t)(amplitude * sin(phase));
                            phase += (2.0f * PI * currentNote.frequency) / SAMPLE_RATE;
                            if (phase > 2.0f * PI) phase -= 2.0f * PI;
                        }
                    }

                    size_t bytes_written;
                    // On envoie le paquet
                    i2s_channel_write(tx_handle, chunkBuffer.data(), CHUNK_SAMPLES * sizeof(int16_t), &bytes_written, portMAX_DELAY);
                }

                // 2. Gestion du petit bout restant (pour être précis sur la durée)
                if (!stop_requested && leftover_samples > 0)
                {
                    if (is_silence) {
                        memset(chunkBuffer.data(), 0, leftover_samples * sizeof(int16_t));
                    } else {
                        for (size_t i = 0; i < leftover_samples; i++) {
                            chunkBuffer[i] = (int16_t)(amplitude * sin(phase));
                            phase += (2.0f * PI * currentNote.frequency) / SAMPLE_RATE;
                            if (phase > 2.0f * PI) phase -= 2.0f * PI;
                        }
                    }
                    size_t bytes_written;
                    i2s_channel_write(tx_handle, chunkBuffer.data(), leftover_samples * sizeof(int16_t), &bytes_written, portMAX_DELAY);
                }

                // Fin de note
                if (uxQueueMessagesWaiting(noteQueue) == 0) {
                    is_playing = false;
                }
            }
        }
    }

    Error Init()
    {
        if (initialized) return Error::Ok;

        // create the note queue
        noteQueue = xQueueCreate(QUEUE_LENGTH, sizeof(Note));
        if (noteQueue == NULL)
        {
            Log::Add(Log::Level::Error, "Sound: Failed to create note queue");
            return Error::Unknown;
        }

        // config i2s pdm
        i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
        chan_cfg.auto_clear = true;

        // create i2s channel
        if (i2s_new_channel(&chan_cfg, &tx_handle, NULL) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Sound: Failed to create I2S channel");
            return Error::Unknown;
        }

        i2s_pdm_tx_config_t pdm_tx_cfg = {
            .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
            .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
            .gpio_cfg = {
                .clk = GPIO_NUM_NC,
                .dout = SOUND_GPIO_PIN,
                .invert_flags = {
                    .clk_inv = 0,
                },
            },
        };

        // initialize and enable i2s pdm tx mode
        if (i2s_channel_init_pdm_tx_mode(tx_handle, &pdm_tx_cfg) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Sound: Failed to init I2S PDM TX mode");
            return Error::Unknown;
        }
        if (i2s_channel_enable(tx_handle) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Sound: Failed to enable I2S channel");
            return Error::Unknown;
        }

        // start audio task
        BaseType_t ret = xTaskCreatePinnedToCore(AudioTask, "SoundAudioTask", STACK_SIZE, NULL, 5, &taskHandle, 1);
        if (ret != pdPASS)
        {
            Log::Add(Log::Level::Error, "Sound: Failed to create audio task");
            return Error::Unknown;
        }

        initialized = true;
        return Error::Ok;
    }

    Error Play(const Note& note)
    {
        // enqueue the note
        if (xQueueSend(noteQueue, &note, 0) != pdPASS) {
            Log::Add(Log::Level::Warning, "Sound: Queue full, note dropped");
            return Error::Unknown;
        }
        return Error::Ok;
    }

    Error Play(const Partition& partition)
    {
        // fill with all notes
        for(int i = 0; i < partition.count; i++) {
            Play(partition.notes[i]);
        }
        return Error::Ok;
    }

    Error Stop()
    {
        // empty queue, and signal stop
        xQueueReset(noteQueue);
        stop_requested = true;

        return Error::Ok;
    }

    bool IsPlaying()
    {
        return is_playing.load();
    }

    Error SetVolume(float volume)
    {
        if (volume < 0.0f || volume > 1.0f) {
            return Error::InvalidParameters;
        }
        global_volume = volume;
        return Error::Ok;
    }
}