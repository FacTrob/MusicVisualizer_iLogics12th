#include "Application.h"
#include "Window/WindowManager.h"
#include "Graphics/Renderer.h"
#include "Audio/AudioLoader.h"
#include "Audio/AudioPlayer.h"
#include "Audio/FFTProcessor.h"
#include "Audio/FrequencyAnalyzer.h"
#include "Visualization/VisualizationEngine.h"
#include "Utils/Timer.h"
#include "GUI/GUIManager.h"
#include <commdlg.h>
#include <iostream>
#include <iomanip>
#include <io.h>
#include <fcntl.h>
#include <algorithm>
#include <filesystem>

// C++14 호환성을 위한 make_unique 구현
#if _MSC_VER < 1900  // Visual Studio 2015 이전
namespace std {
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif

// 정적 인스턴스 포인터 초기화
Application* Application::s_instance = nullptr;

Application::Application()
    : m_isRunning(false), m_isPlaying(false), m_currentSample(0), m_sampleRate(44100), m_audioDuration(0.0f)
{
    s_instance = this;
}

Application::~Application()
{
    Shutdown();
}

bool Application::Initialize(HINSTANCE hInstance, int width, int height)
{
    // 콘솔 창 강제 생성 (디버깅용)
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

    std::cout << "=== Music Visualizer Debug Console ===" << std::endl;
    std::cout << "Application initializing..." << std::endl;

    // Initialize window
    m_windowManager = std::make_unique<WindowManager>();
    if (!m_windowManager->Initialize(hInstance, width, height, L"Music Visualizer"))
    {
        std::cout << "Failed to initialize window manager!" << std::endl;
        return false;
    }
    std::cout << "Window manager initialized successfully" << std::endl;

    // Initialize renderer
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Initialize(m_windowManager->GetHWND(), width, height))
    {
        std::cout << "Failed to initialize renderer!" << std::endl;
        return false;
    }
    std::cout << "Renderer initialized successfully" << std::endl;

    // Initialize audio components
    m_audioLoader = std::make_unique<AudioLoader>();
    m_audioPlayer = std::make_unique<AudioPlayer>();
    m_fftProcessor = std::make_unique<FFTProcessor>(4096); // 4096 sample window
    m_frequencyAnalyzer = std::make_unique<FrequencyAnalyzer>();
    std::cout << "Audio components created" << std::endl;

    // Initialize visualization engine
    m_visualizationEngine = std::make_unique<VisualizationEngine>();
    if (!m_visualizationEngine->Initialize(m_renderer.get()))
    {
        std::cout << "Failed to initialize visualization engine!" << std::endl;
        return false;
    }
    std::cout << "Visualization engine initialized successfully" << std::endl;

    // Initialize GUI
    m_guiManager = std::make_unique<GUIManager>();
    if (!m_guiManager->Initialize(m_windowManager->GetHWND()))
    {
        std::cout << "Failed to initialize GUI manager!" << std::endl;
        return false;
    }
    std::cout << "GUI manager initialized successfully" << std::endl;

    // 전역 키 콜백 설정
    extern void SetKeyCallback(void (*callback)(WPARAM));
    SetKeyCallback(Application::HandleKeyInput);
    std::cout << "Key callback set" << std::endl;

    // Initialize timer
    m_timer = std::make_unique<Timer>();
    std::cout << "Timer initialized" << std::endl;

    m_isRunning = true;
    std::cout << "Application initialization completed successfully!" << std::endl;
    return true;
}

int Application::Run()
{
    MSG msg = {};
    m_timer->Start();

    while (m_isRunning)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                m_isRunning = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!m_isRunning) break;

        m_timer->Tick();
        HandleInput();
        HandleGUI();
        Update(m_timer->GetDeltaTime());
        Render();
    }

    return static_cast<int>(msg.wParam);
}

void Application::HandleGUI()
{
    if (m_guiManager->ShouldExit())
    {
        std::cout << "Exit requested from GUI" << std::endl;
        m_isRunning = false;
    }

    if (m_guiManager->ShouldLoadFile())
    {
        std::cout << "File load requested from GUI" << std::endl;
        if (LoadAudioFile())
        {
            std::cout << "Audio file loaded successfully!" << std::endl;
        }
        else
        {
            std::cout << "Failed to load audio file!" << std::endl;
        }
    }

    if (m_guiManager->ShouldTogglePlayback())
    {
        if (!m_audioData.empty())
        {
            m_isPlaying = !m_isPlaying;

            // 실제 오디오 재생 제어
            if (m_isPlaying)
            {
                m_audioPlayer->Play();
            }
            else
            {
                m_audioPlayer->Pause();
            }

            std::cout << "Playback toggled: " << (m_isPlaying ? "Playing" : "Paused") << std::endl;
        }
        else
        {
            std::cout << "Cannot toggle playback: No audio data loaded" << std::endl;
        }
    }

    // GUI 상태 업데이트
    float currentTime = m_sampleRate > 0 ? (float)m_currentSample / m_sampleRate : 0.0f;
    m_guiManager->SetAudioInfo(m_currentFilename, m_isPlaying, m_audioDuration, currentTime);
    m_guiManager->SetFFTInfo(4096, (float)m_sampleRate / 2.0f);

    m_guiManager->ResetFlags();
}

void Application::UpdateAudioPlayback(float deltaTime)
{
    if (m_isPlaying && !m_audioData.empty())
    {
        // Calculate samples per frame for 60 FPS
        size_t samplesPerFrame = static_cast<size_t>(m_sampleRate / 60.0f);

        if (m_currentSample + samplesPerFrame < m_audioData.size())
        {
            // Extract current audio chunk
            std::vector<float> audioChunk(m_audioData.begin() + m_currentSample,
                m_audioData.begin() + m_currentSample + samplesPerFrame);

            // Process FFT
            auto fftResult = m_fftProcessor->ProcessFFT(audioChunk);

            // Analyze frequencies
            auto frequencyBands = m_frequencyAnalyzer->AnalyzeFrequencies(fftResult, m_sampleRate);

            // Update visualization
            m_visualizationEngine->Update(frequencyBands, deltaTime);

            m_currentSample += samplesPerFrame;
        }
        else
        {
            // End of audio, stop playing
            m_isPlaying = false;
            m_currentSample = 0;
        }
    }
}

void Application::Update(float deltaTime)
{
    UpdateAudioPlayback(deltaTime);
}

void Application::Render()
{
    m_renderer->BeginFrame();

    // 시각화 렌더링
    m_visualizationEngine->Render();

    m_renderer->EndFrame();

    // GUI 렌더링 (GDI 사용)
    HDC hdc = GetDC(m_windowManager->GetHWND());
    if (hdc)
    {
        m_guiManager->Update(m_timer->GetDeltaTime());
        m_guiManager->Render(hdc);
        ReleaseDC(m_windowManager->GetHWND(), hdc);
    }
}

void Application::HandleInput()
{
    static bool spacePressed = false;
    static bool oPressed = false;

    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        if (!spacePressed)
        {
            if (m_isPlaying)
            {
                m_isPlaying = false;
            }
            else if (!m_audioData.empty())
            {
                m_isPlaying = true;
            }
            spacePressed = true;
        }
    }
    else
    {
        spacePressed = false;
    }

    if (GetAsyncKeyState('O') & 0x8000)
    {
        if (!oPressed)
        {
            LoadAudioFile();
            oPressed = true;
        }
    }
    else
    {
        oPressed = false;
    }

    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
        m_isRunning = false;
    }
}

bool Application::LoadAudioFile()
{
    std::cout << "LoadAudioFile() called" << std::endl;

    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_windowManager->GetHWND();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Audio Files\0*.wav\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        std::wcout << L"Selected file: " << szFile << std::endl;

        std::wstring wfilePath(szFile);

        // wstring을 string으로 변환
        int size = WideCharToMultiByte(CP_UTF8, 0, wfilePath.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string filePath(size - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, wfilePath.c_str(), -1, &filePath[0], size, nullptr, nullptr);

        std::cout << "Converted file path: " << filePath << std::endl;

        if (m_audioLoader->LoadWAVFile(filePath, m_audioData, m_sampleRate))
        {
            // 시각화용 데이터 설정
            m_currentSample = 0;
            m_isPlaying = false;
            m_audioDuration = (float)m_audioData.size() / m_sampleRate;

            // 파일명만 추출
            std::filesystem::path path(filePath);
            m_currentFilename = path.filename().string();

            // 실제 오디오 재생용으로도 로드
            if (m_audioPlayer->LoadWAVFile(filePath))
            {
                std::cout << "Audio loaded for both visualization and playback!" << std::endl;
            }
            else
            {
                std::cout << "Audio loaded for visualization only (playback failed)" << std::endl;
            }

            std::cout << "Audio file loaded successfully!" << std::endl;
            std::cout << "Audio data size: " << m_audioData.size() << std::endl;
            std::cout << "Sample rate: " << m_sampleRate << std::endl;
            std::cout << "Duration: " << m_audioDuration << " seconds" << std::endl;

            return true;
        }
        else
        {
            std::cout << "Failed to load audio file!" << std::endl;
        }
    }
    else
    {
        std::cout << "File selection cancelled or failed" << std::endl;
    }
    return false;
}

void Application::HandleKeyInput(WPARAM key)
{
    if (s_instance && s_instance->m_guiManager)
    {
        char keyChar = (key >= 32 && key <= 126) ? (char)key : '?';
        std::cout << "Key pressed: '" << keyChar << "' (code: " << key << ")" << std::endl;
        s_instance->m_guiManager->OnKeyDown(key);
    }
    else
    {
        std::cout << "Key input received but no instance available" << std::endl;
    }
}

void Application::Shutdown()
{
    if (m_visualizationEngine)
        m_visualizationEngine.reset();
    if (m_renderer)
        m_renderer.reset();
    if (m_windowManager)
        m_windowManager.reset();
}