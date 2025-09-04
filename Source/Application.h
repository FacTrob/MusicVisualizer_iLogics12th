#pragma once
#include <Windows.h>
#include <memory>
#include <vector>
#include <string>

// ���� ���� ��� (include ��ȯ ���� ����)
class WindowManager;
class Renderer;
class AudioLoader;
class AudioPlayer;
class FFTProcessor;
class FrequencyAnalyzer;
class VisualizationEngine;
class Timer;
class GUIManager;

class Application
{
public:
    Application();
    ~Application();

    bool Initialize(HINSTANCE hInstance, int width, int height);
    int Run();
    void Shutdown();

    // ���� Ű �Է� �ڵ鷯
    static void HandleKeyInput(WPARAM key);

private:
    void Update(float deltaTime);
    void Render();
    void HandleInput();
    void HandleGUI();
    bool LoadAudioFile();
    void UpdateAudioPlayback(float deltaTime);

    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<AudioLoader> m_audioLoader;
    std::unique_ptr<AudioPlayer> m_audioPlayer;
    std::unique_ptr<FFTProcessor> m_fftProcessor;
    std::unique_ptr<FrequencyAnalyzer> m_frequencyAnalyzer;
    std::unique_ptr<VisualizationEngine> m_visualizationEngine;
    std::unique_ptr<Timer> m_timer;
    std::unique_ptr<GUIManager> m_guiManager;

    bool m_isRunning;
    bool m_isPlaying;
    std::vector<float> m_audioData;
    size_t m_currentSample;
    int m_sampleRate;
    float m_audioDuration;
    std::string m_currentFilename;

    // ���� �ν��Ͻ� ������
    static Application* s_instance;
};