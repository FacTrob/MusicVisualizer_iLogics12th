#include "FFTProcessor.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FFTProcessor::FFTProcessor(int fftSize)
    : m_fftSize(fftSize), m_input(nullptr), m_output(nullptr), m_plan(nullptr)
{
    InitializeFFTW();
}

FFTProcessor::~FFTProcessor()
{
    CleanupFFTW();
}

void FFTProcessor::InitializeFFTW()
{
    // Allocate FFTW arrays
    m_input = fftw_alloc_real(m_fftSize);
    m_output = fftw_alloc_complex(m_fftSize / 2 + 1);

    // Create FFTW plan
    m_plan = fftw_plan_dft_r2c_1d(m_fftSize, m_input, m_output, FFTW_MEASURE);

    // Generate Hann window
    m_window.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i)
    {
        m_window[i] = 0.5f * (1.0f - cosf(2.0f * 3.14159265359f * i / (m_fftSize - 1)));
    }
}

void FFTProcessor::CleanupFFTW()
{
    if (m_plan)
    {
        fftw_destroy_plan(m_plan);
        m_plan = nullptr;
    }

    if (m_input)
    {
        fftw_free(m_input);
        m_input = nullptr;
    }

    if (m_output)
    {
        fftw_free(m_output);
        m_output = nullptr;
    }

    fftw_cleanup();
}

FFTResult FFTProcessor::ProcessFFT(const std::vector<float>& audioData)
{
    FFTResult result;
    result.sampleCount = m_fftSize;
    result.maxMagnitude = 0.0f;

    // Prepare input data
    std::vector<float> processData = audioData;

    // Pad or truncate to FFT size
    if (processData.size() < m_fftSize)
    {
        processData.resize(m_fftSize, 0.0f);
    }
    else if (processData.size() > m_fftSize)
    {
        processData.resize(m_fftSize);
    }

    // Apply window function
    ApplyWindow(processData);

    // Copy to FFTW input array
    for (int i = 0; i < m_fftSize; ++i)
    {
        m_input[i] = static_cast<double>(processData[i]);
    }

    // Execute FFT
    fftw_execute(m_plan);

    // Calculate magnitudes and phases
    int outputSize = m_fftSize / 2 + 1;
    result.magnitudes.resize(outputSize);
    result.phases.resize(outputSize);

    for (int i = 0; i < outputSize; ++i)
    {
        double real = m_output[i][0];
        double imag = m_output[i][1];

        // Calculate magnitude
        result.magnitudes[i] = static_cast<float>(sqrt(real * real + imag * imag));

        // Calculate phase
        result.phases[i] = static_cast<float>(atan2(imag, real));

        // Track maximum magnitude
        if (result.magnitudes[i] > result.maxMagnitude)
        {
            result.maxMagnitude = result.magnitudes[i];
        }
    }

    return result;
}

void FFTProcessor::ApplyWindow(std::vector<float>& data)
{
    ApplyHannWindow(data);
}

void FFTProcessor::ApplyHannWindow(std::vector<float>& data)
{
    for (size_t i = 0; i < data.size() && i < m_window.size(); ++i)
    {
        data[i] *= m_window[i];
    }
}