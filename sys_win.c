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
        case WM_KEYUP:
            // Debug
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
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = MainWindowProc;
    wc.lpszClassName = "HMQ_WindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    // Register Class so windows knows what kind of a window we'll be creating
    if (!RegisterClassEx(&wc))
    {
        exit(EXIT_FAILURE);
    }

    int BufferWidth = 640;
    int BufferHeight = 480;
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    DWORD dwExStyle = 0;

    BOOL Fullscreen = FALSE;

    if (Fullscreen)
    {
        DEVMODE dmScreenSettings = { 0 };
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = BufferWidth;
        dmScreenSettings.dmPelsHeight = BufferHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
        {
            dwExStyle = WS_EX_APPWINDOW;
            dwStyle = WS_POPUP;
        }
        else
        {
            Fullscreen = FALSE;
        }
    }

    // Adjust the size of the window so that the client area is 800x600
    RECT r;
    r.top = r.left = 0;
    r.right = BufferWidth;
    r.bottom = BufferHeight;
    AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle);

    // Create the window based on the registered class
    HWND hWnd = CreateWindowEx(dwExStyle,
                               wc.lpszClassName,
                               "Handmade Quake",
                               dwStyle,
                               CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
                               NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        exit(EXIT_FAILURE);
    }

    if (Fullscreen)
    {
        SetWindowLong(hWnd, GWL_STYLE, 0);
    }

    ShowWindow(hWnd, nShowCmd);

    /*
    // Paint the client area to black
    HDC hdc = GetDC(hWnd);
    PatBlt(hdc, 0, 0, BufferWidth, BufferHeight, BLACKNESS);
    ReleaseDC(hWnd, hdc);
    */

    float TimeAccumulated = 0;
    float TargetTime = 1.0f / 60.0f;
    float oldtime = Sys_InitFloatTime();

    BITMAPINFO BitMapInfo = { 0 };
    BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
    BitMapInfo.bmiHeader.biWidth = BufferWidth;
    BitMapInfo.bmiHeader.biHeight = -BufferHeight;
    BitMapInfo.bmiHeader.biCompression = BI_RGB;
    BitMapInfo.bmiHeader.biPlanes = 1;
    BitMapInfo.bmiHeader.biBitCount = 32;

    void *BackBuffer = malloc(BufferWidth * BufferHeight * 4);

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

        // Draw some random pattern to the buffer before displaying it to screen
        uint32_t *MemoryWalker = (uint32_t *) BackBuffer;
        for (int y = 0; y < BufferHeight; y++)
        {
            for (int x = 0; x < BufferWidth; x++)
            {
                uint8_t Red = rand() % 256;
                uint8_t Green = x % 256;
                uint8_t Blue = y % 256;

                if (x == BufferWidth / 2)
                {
                    Red = 255;
                    Green = 0;
                    Blue = 255;
                }
                else if (y == BufferHeight / 2)
                {
                    Red = 0;
                    Green = 255;
                    Blue = 255;
                }
                else if (x == 0)
                {
                    Red = 255;
                    Green = 0;
                    Blue = 0;
                }
                else if (x == BufferWidth-1)
                {
                    Red = 0;
                    Green = 255;
                    Blue = 0;
                }
                else if (y == 0)
                {
                    Red = 0;
                    Green = 0;
                    Blue = 255;
                }
                else if (y == BufferHeight - 1)
                {
                    Red = 255;
                    Green = 255;
                    Blue = 0;
                }

                *MemoryWalker++ = (Red << 16)| (Green << 8) | Blue;
            }
        }

        // Flip buffer to screen
        HDC dc = GetDC(hWnd);
        StretchDIBits(dc,
                      0, 0, BufferWidth, BufferHeight,
                      0, 0, BufferWidth, BufferHeight,
                      BackBuffer, &BitMapInfo, DIB_RGB_COLORS, SRCCOPY);
        ReleaseDC(hWnd, dc);

        float newtime = Sys_FloatTime();
        Host_Frame(newtime - oldtime);
        oldtime = newtime;
    }

    Host_Shutdown();

    return 0;
}
