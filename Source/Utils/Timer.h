#pragma once
#include <Windows.h>

class Timer
{
public:
    Timer();
    ~Timer();

    void Start();
    void Tick();
    float GetDeltaTime() const { return m_deltaTime; }
    float GetTotalTime() const { return m_totalTime; }
    int GetFPS() const { return m_fps; }

private:
    void UpdateFPS();

    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_startTime;
    LARGE_INTEGER m_currentTime;
    LARGE_INTEGER m_previousTime;

    float m_deltaTime;
    float m_totalTime;

    int m_fps;
    int m_frameCount;
    float m_fpsTimer;
};