#include "audio/MusicProvider.hpp"
#include "common/LittleFS.hpp"
#include "common/Log.hpp"
#include <memory.h>
#include <cstdlib> // pour free()

MusicProvider::MusicProvider()
{
}

MusicProvider::~MusicProvider()
{
    // On libère la mémoire quand le mixer détruit ce provider
    if (file_content != nullptr)
    {
        // Attention: utilise delete[] si LoadFileContent utilise "new char[]",
        // ou free() si LoadFileContent utilise "malloc". 
        // Je mets free() par défaut pour l'embarqué.
        free(file_content); 
        file_content = nullptr;
    }
}

Status MusicProvider::loadFromFile(const char* filepath)
{
    if (Status err = LittleFS::Init(); err != Status::Ok)
    {
        // LOG_ERROR(TAG, "Failed to initialize LittleFS: %d", ErrorToString(err));
        return err;
    }

    if (Status err = LittleFS::LoadFileContent(filepath, &file_content, &content_size); err != Status::Ok)
    {
        // LOG_ERROR(TAG, "Failed to load WAV file '%s': %d", filepath, ErrorToString(err));
        return err;
    }

    // --- Parsing minimaliste du Header WAV pour trouver le début du son ---
    read_position = 0;
    
    // On cherche le bloc "data" (qui contient les vrais samples audio)
    // C'est beaucoup plus sûr que de juste sauter 44 octets !
    for (size_t i = 0; i < content_size - 4; i++)
    {
        if (file_content[i] == 'd' && file_content[i+1] == 'a' && 
            file_content[i+2] == 't' && file_content[i+3] == 'a')
        {
            // On a trouvé "data". Les 4 octets suivants sont la taille des données.
            // Les samples commencent juste après (donc i + 8).
            read_position = i + 8;
            break;
        }
    }

    if (read_position == 0 || read_position >= content_size)
    {
        LOG_WARNING(TAG, "Fichier '%s' invalide ou aucun chunk 'data' trouvé. Lecture depuis le début.", filepath);
        read_position = 44; // Fallback standard
    }
    
    LOG_INFO(TAG, "Fichier WAV '%s' (Taille: %u octets, debut data: %u)", filepath, content_size, read_position);

    return Status::Ok;
}

void MusicProvider::stop()
{
    read_position = content_size; // force end, will be cleaned up by mixer
}

bool MusicProvider::provideSamples(Speaker::Sample* buffer, size_t sampleCount)
{
    if (file_content == nullptr || read_position >= content_size)
    {
        LOG_DEBUG(TAG, "End of music file reached or file not loaded.");
        return false; // Fichier terminé (ou erreur de chargement), demande de destruction
    }

    // Calculer combien d'octets il nous reste à lire
    size_t bytesLeft = content_size - read_position;
    
    // Combien d'octets on veut lire (1 sample = sizeof(int16_t) = 2 octets)
    size_t bytesToRead = sampleCount * sizeof(Speaker::Sample);

    // Si on a moins d'octets restants que ce qui est demandé, on lit juste ce qu'il reste
    if (bytesToRead > bytesLeft)
    {
        bytesToRead = bytesLeft;
    }

    // Copie directe de la mémoire (l'ESP32 est Little Endian, le WAV aussi, c'est magique)
    memcpy(buffer, file_content + read_position, bytesToRead);
    
    // Avancer la tête de lecture
    read_position += bytesToRead;

    // Si on a atteint la fin du fichier avant d'avoir rempli tout le buffer,
    // on remplit le reste du buffer avec du silence (des 0) pour éviter les glitchs sonores.
    size_t samplesRead = bytesToRead / sizeof(Speaker::Sample);
    if (samplesRead < sampleCount)
    {
        memset(buffer + samplesRead, 0, (sampleCount - samplesRead) * sizeof(Speaker::Sample));
    }

    return true; // On a fourni des samples, continuez la lecture
}