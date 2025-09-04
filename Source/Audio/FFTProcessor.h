#pragma once
#include <vector>
#include <complex>
#include <fftw3.h>

struct FFTResult
{
    std::vector<float> magnitudes;
    std::vector<float> phases;
    int sampleCount;
    float maxMagnitude;
};

class FFTProcessor
{
public:
    FFTProcessor(int fftSize = 4096);
    ~FFTProcessor();

    FFTResult ProcessFFT(const std::vector<float>& audioData);
    void ApplyWindow(std::vector<float>& data);

    int GetFFTSize() const { return m_fftSize; }

private:
    void InitializeFFTW();
    void CleanupFFTW();
    void ApplyHannWindow(std::vector<float>& data);

    int m_fftSize;
    double* m_input;
    fftw_complex* m_output;
    fftw_plan m_plan;
    std::vector<float> m_window;
};