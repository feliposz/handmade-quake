#include "quakedefs.h"
#include "winquake.h"

int WindowWidth = 0;
int WindowHeight = 0;
int BytesPerPixel = 0;
int BufferWidth = 0;
int BufferHeight = 0;

HINSTANCE GlobalInstance;
HWND MainWindow;
void *BackBuffer = NULL;
BOOL SettingMode = FALSE;

typedef struct
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[256];
} SAFEBITMAPINFO;

SAFEBITMAPINFO BitMapInfo = { 0 };

typedef enum { MT_WINDOWED, MT_FULLSCREEN } modetype_t;

typedef struct
{
    modetype_t Type;
    int32_t Width;
    int32_t Height;
    uint32_t Hz;
    uint32_t BPP;
} videomode_t;

#define MODE_LIST_MAX 40
videomode_t ModeList[MODE_LIST_MAX];
int ModeCount;
int FirstFullScreenMode = -1;

// Test assets and drawing functions
// TODO: Remove these!!!

uint32_t DiscWidth, DiscHeight;
uint8_t *DiscData = NULL;
uint32_t PauseWidth, PauseHeight;
uint8_t *PauseData = NULL;

void DrawRectangle(int x, int y, int Width, int Height, uint8_t Red, uint8_t Green, uint8_t Blue, void *Buffer)
{
    uint32_t Color = (Red << 16) | (Green << 8) | Blue;

    if (x < 0)
    {
        x = 0;
    }

    if (y < 0)
    {
        y = 0;
    }

    if ((x + Width) > BufferWidth)
    {
        Width = BufferWidth - x;
    }

    if ((y + Height) > BufferHeight)
    {
        Height = BufferHeight - y;
    }

    for (int Row = 0; Row < Height; Row++)
    {
        uint32_t *Pixel = (uint32_t *) Buffer + (y + Row) * BufferWidth + x;
        for (int Col = 0; Col < Width; Col++)
        {
            *Pixel++ = Color;
        }
    }
}

void DrawRectangle8(int x, int y, int Width, int Height, uint8_t Color, void *Buffer)
{
    if (x < 0)
    {
        x = 0;
    }

    if (y < 0)
    {
        y = 0;
    }

    if ((x + Width) > BufferWidth)
    {
        Width = BufferWidth - x;
    }

    if ((y + Height) > BufferHeight)
    {
        Height = BufferHeight - y;
    }

    for (int Row = 0; Row < Height; Row++)
    {
        uint8_t *Pixel = (uint8_t *) Buffer + (y + Row) * BufferWidth + x;
        for (int Col = 0; Col < Width; Col++)
        {
            *Pixel++ = Color;
        }
    }
}

void DrawPic8(int x, int y, int Width, int Height, uint8_t *Data, void *Buffer)
{
    int DataOffsetX = 0;
    int DataOffsetY = 0;
    if (x < 0)
    {
        DataOffsetX = -x;
        x = 0;
    }

    if (y < 0)
    {
        DataOffsetY = -y;
        y = 0;
    }

    if ((x + Width) > BufferWidth)
    {
        Width = BufferWidth - x;
    }

    if ((y + Height) > BufferHeight)
    {
        Height = BufferHeight - y;
    }

    for (int Row = 0; Row < Height - DataOffsetY; Row++)
    {
        uint8_t *Pixel = (uint8_t *) Buffer + (y + Row) * BufferWidth + x;
        uint8_t *Source = Data + (DataOffsetY + Row) * Width + DataOffsetX;
        for (int Col = 0; Col < Width - DataOffsetX; Col++)
        {
            *Pixel++ = *Source++;
        }
    }
}

void DrawPic32(int x, int y, int Width, int Height, uint8_t *Data, void *Buffer)
{
    int DataOffsetX = 0;
    int DataOffsetY = 0;
    if (x < 0)
    {
        DataOffsetX = -x;
        x = 0;
    }

    if (y < 0)
    {
        DataOffsetY = -y;
        y = 0;
    }

    if ((x + Width) > BufferWidth)
    {
        Width = BufferWidth - x;
    }

    if ((y + Height) > BufferHeight)
    {
        Height = BufferHeight - y;
    }

    for (int Row = 0; Row < Height - DataOffsetY; Row++)
    {
        uint32_t *Pixel = (uint32_t *) Buffer + (y + Row) * BufferWidth + x;
        uint8_t *Source = Data + (DataOffsetY + Row) * Width + DataOffsetX;
        for (int Col = 0; Col < Width - DataOffsetX; Col++)
        {
            // NOTE: Palette is organized in the same order we need to draw colors in the screen, so taking advantage of that!
            uint32_t *PaletteColor = (uint32_t *) &BitMapInfo.bmiColors[*Source++];
            *Pixel++ = *PaletteColor;
        }
    }
}

void TestAssetsFree()
{
    if (DiscData)
    {
        free(DiscData);
        DiscData = NULL;
    }
    if (PauseData)
    {
        free(PauseData);
        PauseData = NULL;
    }
}

void TestAssetsLoad()
{
    if (DiscData || PauseData)
    {
        TestAssetsFree();
    }

    FILE * DiscFile = fopen("DISC.lmp", "rb");
    fread(&DiscWidth, sizeof(DiscWidth), 1, DiscFile);
    fread(&DiscHeight, sizeof(DiscHeight), 1, DiscFile);
    DiscData = malloc(DiscWidth * DiscHeight);
    fread(DiscData, DiscWidth * DiscHeight, 1, DiscFile);
    fclose(DiscFile);

    FILE * PauseFile = fopen("pause.lmp", "rb");
    fread(&PauseWidth, sizeof(PauseWidth), 1, PauseFile);
    fread(&PauseHeight, sizeof(PauseHeight), 1, PauseFile);
    PauseData = malloc(PauseWidth * PauseHeight);
    fread(PauseData, PauseWidth * PauseHeight, 1, PauseFile);
    fclose(PauseFile);
}

void TestAssetsDraw(void)
{

    if (BytesPerPixel == 4)
    {
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
                else if (x == BufferWidth - 1)
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

                *MemoryWalker++ = (Red << 16) | (Green << 8) | Blue;
            }
        }

        DrawRectangle(50, 30, 250, 120, 255, 0, 0, BackBuffer);

        DrawRectangle(100, 80, 100, 150, 0, 255, 128, BackBuffer);

        DrawPic32(10, 10, DiscWidth, DiscHeight, DiscData, BackBuffer);

        DrawPic32(100, 100, PauseWidth, PauseHeight, PauseData, BackBuffer);

    }
    else
    {
        // 8-bit palette
        uint8_t *MemoryWalker = (uint8_t *) BackBuffer;
        for (int y = 0; y < BufferHeight; y++)
        {
            for (int x = 0; x < BufferWidth; x++)
            {
                uint8_t Color = rand() % 256;

                if (x == BufferWidth / 2)
                {
                    Color = 1;
                }
                else if (y == BufferHeight / 2)
                {
                    Color = 2;
                }
                else if (x == 0)
                {
                    Color = 3;
                }
                else if (x == BufferWidth - 1)
                {
                    Color = 4;
                }
                else if (y == 0)
                {
                    Color = 5;
                }
                else if (y == BufferHeight - 1)
                {
                    Color = 6;
                }

                *MemoryWalker++ = Color;
            }
        }

        DrawRectangle8(50, 30, 400, 200, 1, BackBuffer);

        DrawRectangle8(100, 150, 100, 250, 2, BackBuffer);

        DrawPic8(10, 10, DiscWidth, DiscHeight, DiscData, BackBuffer);

        DrawPic8(100, 100, PauseWidth, PauseHeight, PauseData, BackBuffer);

    }
}

// Handle messages received by windows OS
LRESULT CALLBACK MainWindowProc(HWND MainWindow, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;
    switch (Msg)
    {
        case WM_KEYDOWN:
        {
            if (wParam == 'A')
            {
                VID_SetMode(0);
            }
            else if (wParam == 'S')
            {
                VID_SetMode(1);
            }
            else if (wParam == 'D')
            {
                VID_SetMode(2);
            }
            else if (wParam == 'F')
            {
                VID_SetMode(3);
            }
            else if (wParam == '1')
            {
                VID_SetMode(FirstFullScreenMode);
            }
            else if (wParam == '2')
            {
                VID_SetMode(FirstFullScreenMode + 1);
            }
            else if (wParam == 'Q')
            {
                Sys_Shutdown();
            }
        } break;

        case WM_DESTROY:
        {
            if (!SettingMode)
            {
                Sys_Shutdown();
            }
        } break;

        default:
        {   
            // Call the default window message handler for messages we don't care
            Result = DefWindowProc(MainWindow, Msg, wParam, lParam);
        } break;
    }
    return Result;
}

void VID_SetWindowedMode(int Mode)
{
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    DWORD dwExStyle = 0;

    // Adjust the size of the window so that the client area is 800x600
    RECT r;
    r.top = r.left = 0;
    r.right = WindowWidth;
    r.bottom = WindowHeight;
    AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle);

    // Create the window based on the registered class
    MainWindow = CreateWindowEx(dwExStyle,
                                "HMQ_WindowClass",
                                "Handmade Quake",
                                dwStyle,
                                CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
                                NULL, NULL, GlobalInstance, NULL);

    if (!MainWindow)
    {
        exit(EXIT_FAILURE);
    }
}

void VID_SetFullscreenMode(int Mode)
{
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    DWORD dwExStyle = 0;

    DEVMODE dmScreenSettings = { 0 };
    dmScreenSettings.dmSize = sizeof(dmScreenSettings);
    dmScreenSettings.dmPelsWidth = WindowWidth;
    dmScreenSettings.dmPelsHeight = WindowHeight;
    dmScreenSettings.dmBitsPerPel = BytesPerPixel * 8;
    dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
    {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;
    }
    else
    {
        exit(EXIT_FAILURE);
    }

    // Adjust the size of the window so that the client area is 800x600
    RECT r;
    r.top = r.left = 0;
    r.right = WindowWidth;
    r.bottom = WindowHeight;
    AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle);

    // Create the window based on the registered class
    MainWindow = CreateWindowEx(dwExStyle,
                                "HMQ_WindowClass",
                                "Handmade Quake",
                                dwStyle,
                                0, 0, r.right - r.left, r.bottom - r.top,
                                NULL, NULL, GlobalInstance, NULL);

    if (!MainWindow)
    {
        exit(EXIT_FAILURE);
    }

    SetWindowLong(MainWindow, GWL_STYLE, 0);
}

void VID_SetMode(int Mode)
{
    if (BackBuffer)
    {
        SettingMode = TRUE;
        VID_Shutdown();
        SettingMode = FALSE;
    }

    TestAssetsLoad();

    WindowWidth = ModeList[Mode].Width;
    WindowHeight = ModeList[Mode].Height;
    BufferWidth = WindowWidth;
    BufferHeight = WindowHeight;
    BytesPerPixel = ModeList[Mode].BPP;

    if (ModeList[Mode].Type == MT_WINDOWED)
    {
        VID_SetWindowedMode(Mode);
    }
    else
    {
        VID_SetFullscreenMode(Mode);
    }

    ShowWindow(MainWindow, SW_SHOWDEFAULT);

    BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
    BitMapInfo.bmiHeader.biWidth = BufferWidth;
    BitMapInfo.bmiHeader.biHeight = -BufferHeight;
    BitMapInfo.bmiHeader.biCompression = BI_RGB;
    BitMapInfo.bmiHeader.biPlanes = 1;
    BitMapInfo.bmiHeader.biBitCount = (WORD) BytesPerPixel * 8;

    FILE * PaletteFile = fopen("palette.lmp", "rb");
    void *RawPaletteData = malloc(256 * 3);
    fread(RawPaletteData, 256 * 3, 1, PaletteFile);
    fclose(PaletteFile);

    uint8_t *PaletteData = (uint8_t *) RawPaletteData;
    for (int i = 0; i < 256; i++)
    {
        BitMapInfo.bmiColors[i].rgbRed = *PaletteData++;
        BitMapInfo.bmiColors[i].rgbGreen = *PaletteData++;
        BitMapInfo.bmiColors[i].rgbBlue = *PaletteData++;
        BitMapInfo.bmiColors[i].rgbReserved = 0;
    }
    free(RawPaletteData);

    BackBuffer = malloc(BufferWidth * BufferHeight * BytesPerPixel);
}

void VID_InitWindowedMode(void)
{
    ModeList[ModeCount].Width = 320;
    ModeList[ModeCount].Height = 240;
    ModeList[ModeCount].BPP = 4;
    ModeList[ModeCount].Type = MT_WINDOWED;
    ModeList[ModeCount].Hz = 0;
    ModeCount++;

    ModeList[ModeCount].Width = 640;
    ModeList[ModeCount].Height = 480;
    ModeList[ModeCount].BPP = 4;
    ModeList[ModeCount].Type = MT_WINDOWED;
    ModeList[ModeCount].Hz = 0;
    ModeCount++;

    ModeList[ModeCount].Width = 800;
    ModeList[ModeCount].Height = 600;
    ModeList[ModeCount].BPP = 4;
    ModeList[ModeCount].Type = MT_WINDOWED;
    ModeList[ModeCount].Hz = 0;
    ModeCount++;

    ModeList[ModeCount].Width = 1024;
    ModeList[ModeCount].Height = 768;
    ModeList[ModeCount].BPP = 4;
    ModeList[ModeCount].Type = MT_WINDOWED;
    ModeList[ModeCount].Hz = 0;
    ModeCount++;
}

void VID_InitFullscreenMode()
{
    DEVMODE DeviceMode;
    DWORD ModeNum = 0;
    BOOL Success = TRUE;
    DWORD OldHeight = 0;
    DWORD OldWidth = 0;

    FirstFullScreenMode = ModeCount;

    do
    {
        Success = EnumDisplaySettings(NULL, ModeNum, &DeviceMode);

        if ((DeviceMode.dmPelsWidth == OldWidth) && (DeviceMode.dmPelsHeight == OldHeight))
        {
            if (ModeList[ModeCount - 1].Hz < DeviceMode.dmDisplayFrequency)
            {
                ModeList[ModeCount - 1].Hz = DeviceMode.dmDisplayFrequency;
            }
        }
        else
        {
            ModeList[ModeCount].Width = DeviceMode.dmPelsWidth;
            ModeList[ModeCount].Height = DeviceMode.dmPelsHeight;
            ModeList[ModeCount].BPP = 4;
            ModeList[ModeCount].Type = MT_FULLSCREEN;
            ModeList[ModeCount].Hz = DeviceMode.dmDisplayFrequency;
            ModeCount++;

            OldWidth = DeviceMode.dmPelsWidth;
            OldHeight = DeviceMode.dmPelsHeight;
        }

        ModeNum++;
    } while (Success && (ModeCount < MODE_LIST_MAX));
}


void VID_Init(void)
{
    // Define basic window class for RegisterClass
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.hInstance = GlobalInstance;
    wc.lpfnWndProc = MainWindowProc;
    wc.lpszClassName = "HMQ_WindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    // Register Class so windows knows what kind of a window we'll be creating
    if (!RegisterClassEx(&wc))
    {
        exit(EXIT_FAILURE);
    }

    VID_InitWindowedMode();
    VID_InitFullscreenMode();

    VID_SetMode(0);
}

void VID_Update(void)
{
    TestAssetsDraw();

    // Flip buffer to screen
    HDC dc = GetDC(MainWindow);
    StretchDIBits(dc,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, BufferWidth, BufferHeight,
                  BackBuffer, (BITMAPINFO *)&BitMapInfo, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(MainWindow, dc);
}

void VID_Shutdown(void)
{
    ChangeDisplaySettings(NULL, 0);
    DestroyWindow(MainWindow);
    free(BackBuffer);
    BackBuffer = NULL;
    TestAssetsFree();
}
