#pragma once
#include "FFTProcessor.h"
#include <vector>

struct FrequencyBand
{
    float amplitude;      // 0.0 - 1.0
    float frequency;      // Center frequency in Hz
    float smoothedAmplitude; // Smoothed for animation
    int binStart;         // Starting FFT bin
    int binEnd;           // Ending FFT bin
};

enum class FrequencyRange
{
    SubBass,    // 20-60 Hz
    Bass,       // 60-250 Hz
    LowMid,     // 250-500 Hz
    Mid,        // 500-2000 Hz
    HighMid,    // 2000-4000 Hz
    Presence,   // 4000-6000 Hz
    Brilliance  // 6000-20000 Hz
};

class FrequencyAnalyzer
{
public:
    FrequencyAnalyzer();
    ~FrequencyAnalyzer();

    std::vector<FrequencyBand> AnalyzeFrequencies(const FFTResult& fftResult, int sampleRate);
    void SetSmoothingFactor(float factor) { m_smoothingFactor = factor; }

    // Get specific frequency ranges
    float GetBassLevel() const;
    float GetMidLevel() const;
    float GetTrebleLevel() const;

private:
    void InitializeFrequencyBands(int fftSize, int sampleRate);
    int FrequencyToBin(float frequency, int fftSize, int sampleRate);
    float BinToFrequency(int bin, int fftSize, int sampleRate);
    void SmoothAmplitudes(std::vector<FrequencyBand>& bands);

    std::vector<FrequencyBand> m_frequencyBands;
    float m_smoothingFactor;
    bool m_initialized;
};