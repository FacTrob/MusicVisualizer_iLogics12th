#pragma once
#include <Windows.h>

// 전방 선언
class GUIManager;

// 전역 함수 선언
void SetGlobalGUIManager(GUIManager* gui);
void SetKeyCallback(void (*callback)(WPARAM));

class WindowManager
{
public:
    WindowManager();
    ~WindowManager();

    bool Initialize(HINSTANCE hInstance, int width, int height, const wchar_t* title);
    void Shutdown();

    HWND GetHWND() const { return m_hWnd; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd;
    HINSTANCE m_hInstance;
    int m_width;
    int m_height;
};