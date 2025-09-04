#include "GUIManager.h"
#include <sstream>
#include <iomanip>
#include <iostream>

// 키 디바운스 시간 (0.3초)
const float GUIManager::KEY_DEBOUNCE_TIME = 0.3f;

GUIManager::GUIManager()
    : m_hwnd(nullptr)
    , m_shouldLoadFile(false)
    , m_shouldTogglePlayback(false)
    , m_shouldExit(false)
    , m_isPlaying(false)
    , m_duration(0.0f)
    , m_currentTime(0.0f)
    , m_fftSize(4096)
    , m_maxFrequency(22050.0f)
    , m_time(0.0f)
    , m_showHelp(true)
    , m_lastKeyTime(0.0f)
    , m_lastKey(0)
    , m_initialized(false)
{
}

GUIManager::~GUIManager()
{
    Shutdown();
}

bool GUIManager::Initialize(HWND hwnd)
{
    m_hwnd = hwnd;
    m_initialized = true;
    return true;
}

void GUIManager::Shutdown()
{
    m_initialized = false;
}

void GUIManager::Update(float deltaTime)
{
    m_time += deltaTime;
}

void GUIManager::Render(HDC hdc)
{
    if (!m_initialized) return;

    // 반투명 배경
    DrawRect(hdc, 10, 10, 350, 250, RGB(20, 20, 20));

    int y = 30;
    const int lineHeight = 20;

    // 제목
    DrawText(hdc, "Music Visualizer", 20, y, RGB(100, 200, 255));
    y += lineHeight * 2;

    // 현재 파일 정보
    if (!m_currentFile.empty())
    {
        DrawText(hdc, "File: " + m_currentFile, 20, y, RGB(200, 200, 200));
        y += lineHeight;

        // 재생 상태
        std::string status = m_isPlaying ? "Playing" : "Paused";
        DrawText(hdc, "Status: " + status, 20, y, m_isPlaying ? RGB(100, 255, 100) : RGB(255, 255, 100));
        y += lineHeight;

        // 시간 정보
        if (m_duration > 0.0f)
        {
            int currentMin = (int)(m_currentTime / 60.0f);
            int currentSec = (int)m_currentTime % 60;
            int totalMin = (int)(m_duration / 60.0f);
            int totalSec = (int)m_duration % 60;

            std::ostringstream timeStream;
            timeStream << "Time: " << std::setfill('0') << std::setw(2) << currentMin
                << ":" << std::setw(2) << currentSec
                << " / " << std::setw(2) << totalMin
                << ":" << std::setw(2) << totalSec;

            DrawText(hdc, timeStream.str(), 20, y, RGB(200, 200, 200));
            y += lineHeight;

            // 진행률 바
            float progress = m_currentTime / m_duration;
            int barWidth = 300;
            int barHeight = 8;

            // 배경 바
            DrawRect(hdc, 20, y, barWidth, barHeight, RGB(50, 50, 50));
            // 진행률 바
            DrawRect(hdc, 20, y, (int)(barWidth * progress), barHeight, RGB(100, 200, 255));
            y += lineHeight * 2;
        }
    }
    else
    {
        DrawText(hdc, "No file loaded", 20, y, RGB(150, 150, 150));
        y += lineHeight * 2;
    }

    // FFT 정보
    std::ostringstream fftStream;
    fftStream << "FFT Size: " << m_fftSize << " | Max Freq: " << (int)m_maxFrequency << " Hz";
    DrawText(hdc, fftStream.str(), 20, y, RGB(180, 180, 180));
    y += lineHeight * 2;

    // 도움말 (처음 5초간 또는 H키로 토글)
    if (m_showHelp || m_time < 5.0f)
    {
        DrawText(hdc, "Controls:", 20, y, RGB(255, 200, 100));
        y += lineHeight;
        DrawText(hdc, "O - Load WAV file", 20, y, RGB(200, 200, 200));
        y += lineHeight;
        DrawText(hdc, "SPACE - Play/Pause", 20, y, RGB(200, 200, 200));
        y += lineHeight;
        DrawText(hdc, "H - Toggle help", 20, y, RGB(200, 200, 200));
        y += lineHeight;
        DrawText(hdc, "ESC - Exit", 20, y, RGB(200, 200, 200));
        y += lineHeight;

        // 키 입력 상태 표시
        if (m_time - m_lastKeyTime < 1.0f)
        {
            std::string keyInfo = "Last key: ";
            if (m_lastKey == VK_SPACE) keyInfo += "SPACE";
            else if (m_lastKey == 'O' || m_lastKey == 'o') keyInfo += "O";
            else if (m_lastKey == 'H' || m_lastKey == 'h') keyInfo += "H";
            else if (m_lastKey == VK_ESCAPE) keyInfo += "ESC";
            else keyInfo += std::to_string(m_lastKey);

            DrawText(hdc, keyInfo, 20, y, RGB(100, 255, 100));
        }
    }
}

void GUIManager::OnKeyDown(WPARAM key)
{
    // 키 디바운싱
    if (key == m_lastKey && (m_time - m_lastKeyTime) < KEY_DEBOUNCE_TIME)
    {
        return;
    }

    m_lastKey = key;
    m_lastKeyTime = m_time;

    switch (key)
    {
    case 'O':
    case 'o':
        std::cout << "O key pressed - Load file" << std::endl;
        m_shouldLoadFile = true;
        break;
    case VK_SPACE:
        std::cout << "SPACE key pressed - Toggle playback" << std::endl;
        m_shouldTogglePlayback = true;
        break;
    case 'H':
    case 'h':
        std::cout << "H key pressed - Toggle help" << std::endl;
        m_showHelp = !m_showHelp;
        break;
    case VK_ESCAPE:
        std::cout << "ESC key pressed - Exit" << std::endl;
        m_shouldExit = true;
        break;
    }
}

void GUIManager::DrawText(HDC hdc, const std::string& text, int x, int y, COLORREF color)
{
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);

    int wideSize = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (wideSize > 0)
    {
        std::wstring wideText(wideSize - 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wideText[0], wideSize);
        TextOutW(hdc, x, y, wideText.c_str(), (int)wideText.length());
    }
}

void GUIManager::DrawRect(HDC hdc, int x, int y, int width, int height, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    RECT rect = { x, y, x + width, y + height };
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

void GUIManager::SetAudioInfo(const std::string& filename, bool isPlaying, float duration, float currentTime)
{
    m_currentFile = filename;
    m_isPlaying = isPlaying;
    m_duration = duration;
    m_currentTime = currentTime;
}

void GUIManager::SetFFTInfo(int fftSize, float maxFrequency)
{
    m_fftSize = fftSize;
    m_maxFrequency = maxFrequency;
}

void GUIManager::ResetFlags()
{
    m_shouldLoadFile = false;
    m_shouldTogglePlayback = false;
    m_shouldExit = false;
}