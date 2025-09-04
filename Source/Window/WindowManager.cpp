#include "WindowManager.h"

// 키 입력 콜백 함수 포인터
static void (*g_keyCallback)(WPARAM) = nullptr;

void SetGlobalGUIManager(GUIManager* gui)
{
    // 실제로는 키 콜백만 설정
}

void SetKeyCallback(void (*callback)(WPARAM))
{
    g_keyCallback = callback;
}

WindowManager::WindowManager() : m_hWnd(nullptr), m_hInstance(nullptr), m_width(0), m_height(0)
{
}

WindowManager::~WindowManager()
{
    Shutdown();
}

bool WindowManager::Initialize(HINSTANCE hInstance, int width, int height, const wchar_t* title)
{
    m_hInstance = hInstance;
    m_width = width;
    m_height = height;

    // Register window class
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"MusicVisualizerWindow";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        return false;
    }

    // Calculate window size including borders
    RECT windowRect = { 0, 0, width, height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // Center window on screen
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenWidth - windowWidth) / 2;
    int posY = (screenHeight - windowHeight) / 2;

    // Create window
    m_hWnd = CreateWindow(
        L"MusicVisualizerWindow",
        title,
        WS_OVERLAPPEDWINDOW,
        posX, posY,
        windowWidth, windowHeight,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!m_hWnd)
    {
        return false;
    }

    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);

    return true;
}

void WindowManager::Shutdown()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }

    if (m_hInstance)
    {
        UnregisterClass(L"MusicVisualizerWindow", m_hInstance);
        m_hInstance = nullptr;
    }
}

LRESULT CALLBACK WindowManager::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        // 키 콜백 함수 호출
        if (g_keyCallback)
        {
            g_keyCallback(wParam);
        }

        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // DirectX가 렌더링하므로 여기서는 아무것도 하지 않음
        EndPaint(hWnd, &ps);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}