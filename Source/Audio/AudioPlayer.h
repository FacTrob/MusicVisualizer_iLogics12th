#pragma once
#include <Windows.h>
#include <vector>
#include <string>

class AudioPlayer
{
public:
    AudioPlayer();
    ~AudioPlayer();

    bool LoadWAVFile(const std::string& filename);
    bool Play();
    void Pause();
    void Stop();
    void SetPosition(float seconds);

    bool IsPlaying() const { return m_isPlaying; }
    float GetPosition() const;
    float GetDuration() const { return m_duration; }

private:
    bool m_isPlaying;
    float m_duration;
    std::string m_currentFile;
};