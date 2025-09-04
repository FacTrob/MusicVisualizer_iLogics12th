#include "FrequencyAnalyzer.h"
#include <algorithm>
#include <cmath>

FrequencyAnalyzer::FrequencyAnalyzer()
    : m_smoothingFactor(0.8f), m_initialized(false)
{
}

FrequencyAnalyzer::~FrequencyAnalyzer()
{
}

std::vector<FrequencyBand> FrequencyAnalyzer::AnalyzeFrequencies(const FFTResult& fftResult, int sampleRate)
{
    if (!m_initialized)
    {
        InitializeFrequencyBands(fftResult.sampleCount, sampleRate);
        m_initialized = true;
    }

    // Update amplitude values for each frequency band
    for (auto& band : m_frequencyBands)
    {
        float totalAmplitude = 0.0f;
        int binCount = 0;

        // Sum amplitudes across the frequency band's bin range
        for (int bin = band.binStart; bin <= band.binEnd && bin < fftResult.magnitudes.size(); ++bin)
        {
            totalAmplitude += fftResult.magnitudes[bin];
            binCount++;
        }

        // Calculate average amplitude for this band
        if (binCount > 0)
        {
            band.amplitude = totalAmplitude / binCount;

            // Normalize by max magnitude to get 0-1 range
            if (fftResult.maxMagnitude > 0.0f)
            {
                band.amplitude /= fftResult.maxMagnitude;
            }

            // Clamp to [0, 1]
            band.amplitude = std::clamp(band.amplitude, 0.0f, 1.0f);
        }
        else
        {
            band.amplitude = 0.0f;
        }
    }

    // Apply smoothing for animation
    SmoothAmplitudes(m_frequencyBands);

    return m_frequencyBands;
}

void FrequencyAnalyzer::InitializeFrequencyBands(int fftSize, int sampleRate)
{
    m_frequencyBands.clear();

    // Define frequency ranges
    struct FrequencyRange
    {
        float minFreq;
        float maxFreq;
        const char* name;
    };

    std::vector<FrequencyRange> ranges = {
        {20.0f, 60.0f, "SubBass"},
        {60.0f, 250.0f, "Bass"},
        {250.0f, 500.0f, "LowMid"},
        {500.0f, 2000.0f, "Mid"},
        {2000.0f, 4000.0f, "HighMid"},
        {4000.0f, 6000.0f, "Presence"},
        {6000.0f, 20000.0f, "Brilliance"}
    };

    // Create frequency bands
    for (const auto& range : ranges)
    {
        FrequencyBand band;
        band.frequency = (range.minFreq + range.maxFreq) * 0.5f; // Center frequency
        band.binStart = FrequencyToBin(range.minFreq, fftSize, sampleRate);
        band.binEnd = FrequencyToBin(range.maxFreq, fftSize, sampleRate);
        band.amplitude = 0.0f;
        band.smoothedAmplitude = 0.0f;

        // Ensure valid bin ranges
        band.binStart = (band.binStart > 0) ? band.binStart : 0;
        band.binEnd = (band.binEnd < fftSize / 2) ? band.binEnd : fftSize / 2;

        if (band.binStart <= band.binEnd)
        {
            m_frequencyBands.push_back(band);
        }
    }

    // Add additional fine-grained bands for more detailed visualization
    int numDetailBands = 16;
    float minLogFreq = log10f(80.0f);   // Start from 80 Hz
    float maxLogFreq = log10f(8000.0f); // Up to 8 kHz
    float logStep = (maxLogFreq - minLogFreq) / numDetailBands;

    for (int i = 0; i < numDetailBands; ++i)
    {
        float logFreq1 = minLogFreq + i * logStep;
        float logFreq2 = minLogFreq + (i + 1) * logStep;

        float freq1 = powf(10.0f, logFreq1);
        float freq2 = powf(10.0f, logFreq2);

        FrequencyBand band;
        band.frequency = (freq1 + freq2) * 0.5f;
        band.binStart = FrequencyToBin(freq1, fftSize, sampleRate);
        band.binEnd = FrequencyToBin(freq2, fftSize, sampleRate);
        band.amplitude = 0.0f;
        band.smoothedAmplitude = 0.0f;

        // Ensure valid bin ranges
        band.binStart = (band.binStart > 0) ? band.binStart : 0;
        band.binEnd = (band.binEnd < fftSize / 2) ? band.binEnd : fftSize / 2;

        if (band.binStart <= band.binEnd)
        {
            m_frequencyBands.push_back(band);
        }
    }
}

int FrequencyAnalyzer::FrequencyToBin(float frequency, int fftSize, int sampleRate)
{
    return static_cast<int>((frequency * fftSize) / sampleRate);
}

float FrequencyAnalyzer::BinToFrequency(int bin, int fftSize, int sampleRate)
{
    return (static_cast<float>(bin) * sampleRate) / fftSize;
}

void FrequencyAnalyzer::SmoothAmplitudes(std::vector<FrequencyBand>& bands)
{
    for (auto& band : bands)
    {
        // Exponential smoothing
        band.smoothedAmplitude = m_smoothingFactor * band.smoothedAmplitude +
            (1.0f - m_smoothingFactor) * band.amplitude;
    }
}

float FrequencyAnalyzer::GetBassLevel() const
{
    float bassLevel = 0.0f;
    int count = 0;

    for (const auto& band : m_frequencyBands)
    {
        if (band.frequency >= 60.0f && band.frequency <= 250.0f)
        {
            bassLevel += band.smoothedAmplitude;
            count++;
        }
    }

    return count > 0 ? bassLevel / count : 0.0f;
}

float FrequencyAnalyzer::GetMidLevel() const
{
    float midLevel = 0.0f;
    int count = 0;

    for (const auto& band : m_frequencyBands)
    {
        if (band.frequency >= 250.0f && band.frequency <= 4000.0f)
        {
            midLevel += band.smoothedAmplitude;
            count++;
        }
    }

    return count > 0 ? midLevel / count : 0.0f;
}

float FrequencyAnalyzer::GetTrebleLevel() const
{
    float trebleLevel = 0.0f;
    int count = 0;

    for (const auto& band : m_frequencyBands)
    {
        if (band.frequency >= 4000.0f)
        {
            trebleLevel += band.smoothedAmplitude;
            count++;
        }
    }

    return count > 0 ? trebleLevel / count : 0.0f;
}