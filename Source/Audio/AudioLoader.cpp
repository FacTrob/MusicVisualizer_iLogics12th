#include "AudioLoader.h"
#include <fstream>
#include <iostream>
#include <algorithm>

AudioLoader::AudioLoader()
{
}

AudioLoader::~AudioLoader()
{
}

bool AudioLoader::LoadWAVFile(const std::string& filename, std::vector<float>& audioData, int& sampleRate)
{
    std::cout << "Attempting to load file: " << filename << std::endl;

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::cout << "File opened successfully" << std::endl;

    // Read RIFF header
    char riffHeader[12];
    file.read(riffHeader, 12);

    if (file.gcount() != 12)
    {
        std::cout << "Failed to read RIFF header" << std::endl;
        file.close();
        return false;
    }

    if (strncmp(riffHeader, "RIFF", 4) != 0 || strncmp(riffHeader + 8, "WAVE", 4) != 0)
    {
        std::cout << "Not a valid WAV file" << std::endl;
        file.close();
        return false;
    }

    std::cout << "RIFF/WAVE header OK" << std::endl;

    // 청크를 순차적으로 읽기
    uint16_t audioFormat = 0;
    uint16_t numChannels = 0;
    uint32_t fileSampleRate = 0;
    uint16_t bitsPerSample = 0;
    std::vector<uint8_t> audioDataRaw;

    while (!file.eof())
    {
        char chunkId[4];
        uint32_t chunkSize;

        file.read(chunkId, 4);
        if (file.gcount() != 4) break;

        file.read(reinterpret_cast<char*>(&chunkSize), 4);
        if (file.gcount() != 4) break;

        std::cout << "Found chunk: " << std::string(chunkId, 4) << " size: " << chunkSize << std::endl;

        if (strncmp(chunkId, "fmt ", 4) == 0)
        {
            // fmt 청크 읽기
            if (chunkSize < 16)
            {
                std::cout << "fmt chunk too small: " << chunkSize << std::endl;
                file.close();
                return false;
            }

            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&numChannels), 2);
            file.read(reinterpret_cast<char*>(&fileSampleRate), 4);
            file.seekg(4, std::ios::cur); // byteRate 건너뛰기
            file.seekg(2, std::ios::cur); // blockAlign 건너뛰기
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            // 나머지 fmt 데이터 건너뛰기
            if (chunkSize > 16)
            {
                file.seekg(chunkSize - 16, std::ios::cur);
            }

            std::cout << "Format info - Format: " << audioFormat << ", Channels: " << numChannels
                << ", Sample Rate: " << fileSampleRate << ", Bits: " << bitsPerSample << std::endl;
        }
        else if (strncmp(chunkId, "data", 4) == 0)
        {
            // data 청크 읽기
            std::cout << "Reading audio data: " << chunkSize << " bytes" << std::endl;
            audioDataRaw.resize(chunkSize);
            file.read(reinterpret_cast<char*>(audioDataRaw.data()), chunkSize);

            if (file.gcount() != chunkSize)
            {
                std::cout << "Failed to read audio data. Read " << file.gcount() << " bytes, expected " << chunkSize << std::endl;
                file.close();
                return false;
            }
            break; // data 청크를 찾았으니 종료
        }
        else
        {
            // 다른 청크는 건너뛰기
            std::cout << "Skipping chunk: " << std::string(chunkId, 4) << std::endl;
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    file.close();

    // 유효성 검사
    if (audioFormat != 1)
    {
        std::cout << "Unsupported audio format: " << audioFormat << " (expected PCM = 1)" << std::endl;
        return false;
    }

    if (bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32)
    {
        std::cout << "Unsupported bit depth: " << bitsPerSample << std::endl;
        return false;
    }

    if (audioDataRaw.empty())
    {
        std::cout << "No audio data found" << std::endl;
        return false;
    }

    std::cout << "Audio data read successfully" << std::endl;

    // Convert to float format
    ConvertToFloat(audioDataRaw, audioData, bitsPerSample, numChannels);
    sampleRate = fileSampleRate;

    std::cout << "Conversion completed successfully" << std::endl;
    std::cout << "Final audio data size: " << audioData.size() << " samples" << std::endl;
    std::cout << "Duration: " << static_cast<float>(audioData.size()) / sampleRate << " seconds" << std::endl;

    return true;
}

bool AudioLoader::ValidateWAVHeader(const WAVHeader& header)
{
    std::cout << "Validating WAV header..." << std::endl;

    // Check RIFF signature
    if (strncmp(header.chunkID, "RIFF", 4) != 0)
    {
        std::cout << "Invalid RIFF signature: " << std::string(header.chunkID, 4) << std::endl;
        return false;
    }
    std::cout << "RIFF signature OK" << std::endl;

    // Check WAVE format
    if (strncmp(header.format, "WAVE", 4) != 0)
    {
        std::cout << "Invalid WAVE format: " << std::string(header.format, 4) << std::endl;
        return false;
    }
    std::cout << "WAVE format OK" << std::endl;

    // Check fmt chunk
    if (strncmp(header.subchunk1ID, "fmt ", 4) != 0)
    {
        std::cout << "Invalid fmt chunk: " << std::string(header.subchunk1ID, 4) << std::endl;
        return false;
    }
    std::cout << "fmt chunk OK" << std::endl;

    // Check data chunk
    if (strncmp(header.subchunk2ID, "data", 4) != 0)
    {
        std::cout << "Invalid data chunk: " << std::string(header.subchunk2ID, 4) << std::endl;
        std::cout << "Expected 'data', got: ";
        for (int i = 0; i < 4; i++) {
            std::cout << "0x" << std::hex << (int)(unsigned char)header.subchunk2ID[i] << " ";
        }
        std::cout << std::dec << std::endl;
        return false;
    }
    std::cout << "data chunk OK" << std::endl;

    // Check PCM format
    if (header.audioFormat != 1)
    {
        std::cout << "Unsupported audio format: " << header.audioFormat << " (expected PCM = 1)" << std::endl;
        return false;
    }
    std::cout << "PCM format OK" << std::endl;

    // Check supported bit depths
    if (header.bitsPerSample != 16 && header.bitsPerSample != 24 && header.bitsPerSample != 32)
    {
        std::cout << "Unsupported bit depth: " << header.bitsPerSample << " (supported: 16, 24, 32)" << std::endl;
        return false;
    }
    std::cout << "Bit depth OK: " << header.bitsPerSample << std::endl;

    std::cout << "WAV header validation passed!" << std::endl;
    return true;
}

void AudioLoader::ConvertToFloat(const std::vector<uint8_t>& rawData, std::vector<float>& floatData,
    int bitsPerSample, int numChannels)
{
    size_t numSamples = rawData.size() / (bitsPerSample / 8) / numChannels;
    floatData.resize(numSamples);

    const uint8_t* data = rawData.data();

    for (size_t i = 0; i < numSamples; ++i)
    {
        float sample = 0.0f;

        if (bitsPerSample == 16)
        {
            // Convert from 16-bit signed integer
            int16_t* samples = (int16_t*)data;

            if (numChannels == 1)
            {
                sample = static_cast<float>(samples[i]) / 32768.0f;
            }
            else // Stereo - convert to mono by averaging
            {
                float left = static_cast<float>(samples[i * 2]) / 32768.0f;
                float right = static_cast<float>(samples[i * 2 + 1]) / 32768.0f;
                sample = (left + right) * 0.5f;
            }
        }
        else if (bitsPerSample == 24)
        {
            // Convert from 24-bit signed integer
            for (size_t ch = 0; ch < numChannels; ++ch)
            {
                size_t index = (i * numChannels + ch) * 3;
                int32_t value = (data[index + 2] << 16) | (data[index + 1] << 8) | data[index];

                // Sign extend
                if (value & 0x800000)
                    value |= 0xFF000000;

                sample += static_cast<float>(value) / 8388608.0f;
            }
            sample /= numChannels; // Average channels
        }
        else if (bitsPerSample == 32)
        {
            // Convert from 32-bit signed integer
            int32_t* samples = (int32_t*)data;

            if (numChannels == 1)
            {
                sample = static_cast<float>(samples[i]) / 2147483648.0f;
            }
            else // Stereo - convert to mono by averaging
            {
                float left = static_cast<float>(samples[i * 2]) / 2147483648.0f;
                float right = static_cast<float>(samples[i * 2 + 1]) / 2147483648.0f;
                sample = (left + right) * 0.5f;
            }
        }

        // Clamp to [-1.0, 1.0]
        floatData[i] = std::clamp(sample, -1.0f, 1.0f);
    }
}