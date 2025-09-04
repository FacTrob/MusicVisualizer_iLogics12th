#pragma once
#include <Windows.h>
#include <string>

class GUIManager
{
public:
    GUIManager();
    ~GUIManager();

    bool Initialize(HWND hwnd);
    void Shutdown();

    void Update(float deltaTime);
    void Render(HDC hdc);

    // GUI ���� ��ȯ
    bool ShouldLoadFile() const { return m_shouldLoadFile; }
    bool ShouldTogglePlayback() const { return m_shouldTogglePlayback; }
    bool ShouldExit() const { return m_shouldExit; }

    // ���� ������Ʈ
    void SetAudioInfo(const std::string& filename, bool isPlaying, float duration, float currentTime);
    void SetFFTInfo(int fftSize, float maxFrequency);
    void ResetFlags();

    // Ű �Է� ó��
    void OnKeyDown(WPARAM key);

private:
    void DrawText(HDC hdc, const std::string& text, int x, int y, COLORREF color = RGB(255, 255, 255));
    void DrawRect(HDC hdc, int x, int y, int width, int height, COLORREF color);

    HWND m_hwnd;

    // GUI ����
    bool m_shouldLoadFile;
    bool m_shouldTogglePlayback;
    bool m_shouldExit;

    // ����� ����
    std::string m_currentFile;
    bool m_isPlaying;
    float m_duration;
    float m_currentTime;
    int m_fftSize;
    float m_maxFrequency;

    // �ִϸ��̼� �� Ű ó��
    float m_time;
    bool m_showHelp;
    float m_lastKeyTime;
    WPARAM m_lastKey;

    bool m_initialized;

    // Ű ��ٿ �ð�
    static const float KEY_DEBOUNCE_TIME;
};