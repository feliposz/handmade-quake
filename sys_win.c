#include "quakedefs.h"
#include "winquake.h"

static BOOL IsRunning = TRUE;

// Timer functions

static double GTimePassed;
static double GSecondsPerTick;
static __int64 GTimeCounter;

float Sys_InitFloatTime(void)
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);

    GSecondsPerTick = 1.0 / (double) Frequency.QuadPart;

    LARGE_INTEGER Counter;
    QueryPerformanceCounter(&Counter);
    GTimeCounter = Counter.QuadPart;

    return 0;
}

float Sys_FloatTime(void)
{
    LARGE_INTEGER Counter;
    QueryPerformanceCounter(&Counter);

    __int64 Interval = Counter.QuadPart - GTimeCounter;
    GTimeCounter = Counter.QuadPart;

    double SecondsGoneBy = Interval * GSecondsPerTick;
    GTimePassed += SecondsGoneBy;

    return (float) GTimePassed;
}

// Main window functions

void Sys_Shutdown()
{
    IsRunning = FALSE;
}

// Handle messages received by windows OS
LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;
    switch (Msg)
    {
        case WM_DESTROY:
            Sys_Shutdown();
            break;
        default:
            // Call the default window message handler for messages we don't care
            Result = DefWindowProc(hWnd, Msg, wParam, lParam);
            break;
    }
    return Result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    COM_ParseCmdLine(lpCmdLine);

    // Define basic window class for RegisterClass
    WNDCLASS wc = { 0 };
    wc.hInstance = hInstance;
    wc.lpfnWndProc = MainWindowProc;
    wc.lpszClassName = "HMQ_WindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    // Register Class so windows knows what kind of a window we'll be creating
    if (!RegisterClass(&wc))
    {
        exit(EXIT_FAILURE);
    }

    // Adjust the size of the window so that the client area is 800x600
    RECT r;
    r.top = r.left = 0;
    r.right = 800;
    r.bottom = 600;
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window based on the registered class
    HWND hWnd = CreateWindow(wc.lpszClassName,
                             "Handmade Quake",
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
                             NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        exit(EXIT_FAILURE);
    }

    // Paint the client area to black
    HDC hdc = GetDC(hWnd);
    PatBlt(hdc, 0, 0, 800, 600, BLACKNESS);
    ReleaseDC(hWnd, hdc);

    float TimeAccumulated = 0;
    float TargetTime = 1.0f / 60.0f;
    float oldtime = Sys_InitFloatTime();

    Host_Init();

    // Main program loop.
    // Runs until flag is set to false.
    // Dispatches any messages received to MainWindowProc
    MSG Msg;
    while (IsRunning)
    {
        // Get messages from the queue without blocking if the queue is empty
        while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }

        float newtime = Sys_FloatTime();
        Host_Frame(newtime - oldtime);
        oldtime = newtime;
    }

    Host_Shutdown();

    return 0;
}
