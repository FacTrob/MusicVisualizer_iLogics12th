#include "AudioPlayer.h"
#include <mmsystem.h>
#include <iostream>

#pragma comment(lib, "winmm.lib")

AudioPlayer::AudioPlayer()
    : m_isPlaying(false), m_duration(0.0f)
{
}

AudioPlayer::~AudioPlayer()
{
    Stop();
}

bool AudioPlayer::LoadWAVFile(const std::string& filename)
{
    Stop(); // 이전 재생 정지

    // 파일명을 wide string으로 변환
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, nullptr, 0);
    if (wideSize <= 0) return false;

    std::wstring wideFilename(wideSize - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, &wideFilename[0], wideSize);

    m_currentFile = filename;

    // MCI를 사용해서 파일 길이 확인
    std::wstring command = L"open \"" + wideFilename + L"\" type waveaudio alias myWAV";
    MCIERROR result = mciSendString(command.c_str(), nullptr, 0, nullptr);

    if (result != 0)
    {
        wchar_t errorMsg[256];
        mciGetErrorString(result, errorMsg, 256);
        std::wcout << L"MCI open error: " << errorMsg << std::endl;
        return false;
    }

    // 길이 정보 가져오기
    wchar_t lengthStr[256];
    result = mciSendString(L"status myWAV length", lengthStr, 256, nullptr);
    if (result == 0)
    {
        m_duration = (float)_wtoi(lengthStr) / 1000.0f; // ms를 초로 변환
        std::wcout << L"Audio duration: " << m_duration << L" seconds" << std::endl;
    }

    std::cout << "Audio file loaded successfully for playback" << std::endl;
    return true;
}

bool AudioPlayer::Play()
{
    if (m_currentFile.empty()) return false;

    MCIERROR result = mciSendString(L"play myWAV from 0", nullptr, 0, nullptr);
    if (result != 0)
    {
        wchar_t errorMsg[256];
        mciGetErrorString(result, errorMsg, 256);
        std::wcout << L"MCI play error: " << errorMsg << std::endl;
        return false;
    }

    m_isPlaying = true;
    std::cout << "Audio playback started" << std::endl;
    return true;
}

void AudioPlayer::Pause()
{
    if (!m_isPlaying) return;

    mciSendString(L"pause myWAV", nullptr, 0, nullptr);
    m_isPlaying = false;
    std::cout << "Audio playback paused" << std::endl;
}

void AudioPlayer::Stop()
{
    if (!m_currentFile.empty())
    {
        mciSendString(L"stop myWAV", nullptr, 0, nullptr);
        mciSendString(L"close myWAV", nullptr, 0, nullptr);
        m_isPlaying = false;
        std::cout << "Audio playback stopped" << std::endl;
    }
}

void AudioPlayer::SetPosition(float seconds)
{
    if (m_currentFile.empty()) return;

    int positionMs = (int)(seconds * 1000);
    std::wstring command = L"seek myWAV to " + std::to_wstring(positionMs);
    mciSendString(command.c_str(), nullptr, 0, nullptr);

    if (m_isPlaying)
    {
        mciSendString(L"play myWAV", nullptr, 0, nullptr);
    }
}

float AudioPlayer::GetPosition() const
{
    if (m_currentFile.empty()) return 0.0f;

    wchar_t positionStr[256];
    MCIERROR result = mciSendString(L"status myWAV position", positionStr, 256, nullptr);
    if (result == 0)
    {
        return (float)_wtoi(positionStr) / 1000.0f; // ms를 초로 변환
    }

    return 0.0f;
}