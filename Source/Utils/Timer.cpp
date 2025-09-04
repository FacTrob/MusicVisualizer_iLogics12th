#include "Timer.h"

Timer::Timer() : m_deltaTime(0.0f), m_totalTime(0.0f), m_fps(0), m_frameCount(0), m_fpsTimer(0.0f)
{
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_startTime);
    m_previousTime = m_startTime;
}

Timer::~Timer()
{
}

void Timer::Start()
{
    QueryPerformanceCounter(&m_startTime);
    m_previousTime = m_startTime;
    m_totalTime = 0.0f;
    m_deltaTime = 0.0f;
    m_frameCount = 0;
    m_fpsTimer = 0.0f;
}

void Timer::Tick()
{
    QueryPerformanceCounter(&m_currentTime);

    // Calculate delta time
    m_deltaTime = static_cast<float>(m_currentTime.QuadPart - m_previousTime.QuadPart) / m_frequency.QuadPart;

    // Cap delta time to prevent huge jumps
    if (m_deltaTime > 0.1f)
        m_deltaTime = 0.1f;

    m_totalTime = static_cast<float>(m_currentTime.QuadPart - m_startTime.QuadPart) / m_frequency.QuadPart;

    m_previousTime = m_currentTime;

    UpdateFPS();
}

void Timer::UpdateFPS()
{
    m_frameCount++;
    m_fpsTimer += m_deltaTime;

    if (m_fpsTimer >= 1.0f)
    {
        m_fps = m_frameCount;
        m_frameCount = 0;
        m_fpsTimer = 0.0f;
    }
}