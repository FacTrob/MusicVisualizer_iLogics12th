#pragma once
#include <vector>
#include <string>

#pragma pack(push, 1)
struct WAVHeader
{
    char chunkID[4];        // "RIFF"
    uint32_t chunkSize;     // File size - 8
    char format[4];         // "WAVE"
    char subchunk1ID[4];    // "fmt "
    uint32_t subchunk1Size; // 16 for PCM
    uint16_t audioFormat;   // 1 for PCM
    uint16_t numChannels;   // Mono = 1, Stereo = 2
    uint32_t sampleRate;    // 44100, 48000, etc.
    uint32_t byteRate;      // sampleRate * numChannels * bitsPerSample / 8
    uint16_t blockAlign;    // numChannels * bitsPerSample / 8
    uint16_t bitsPerSample; // 16, 24, 32
    char subchunk2ID[4];    // "data"
    uint32_t subchunk2Size; // Number of bytes in data
};
#pragma pack(pop)

class AudioLoader
{
public:
    AudioLoader();
    ~AudioLoader();

    bool LoadWAVFile(const std::string& filename, std::vector<float>& audioData, int& sampleRate);

private:
    bool ValidateWAVHeader(const WAVHeader& header);
    void ConvertToFloat(const std::vector<uint8_t>& rawData, std::vector<float>& floatData,
        int bitsPerSample, int numChannels);
};