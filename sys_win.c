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

// File functions

#define MAX_HANDLES 10

static FILE *FileHandles[MAX_HANDLES];

static int FindHandle()
{
    for (int i = 0; i < MAX_HANDLES; i++)
    {
        if (FileHandles[i] == NULL)
        {
            return i;
        }
    }
    return -1;
}

static int FileLength(int Handle)
{
    FILE *f = FileHandles[Handle];
    if (f == NULL)
    {
        return -1;
    }
    int Pos = ftell(f);
    fseek(f, 0, SEEK_END);
    int End = ftell(f);
    fseek(f, Pos, SEEK_SET);
    return End;
}

int Sys_FileOpenRead(char *Path, int *Size)
{
    int HandleIndex = FindHandle();
    FILE *FileHandle;
    fopen_s(&FileHandle, Path, "rb");
    if (!FileHandle)
    {
        if (Size != NULL)
        {
            *Size = -1;
        }
        return -1;
    }
    FileHandles[HandleIndex] = FileHandle;
    *Size = FileLength(HandleIndex);
    return HandleIndex;
}

int Sys_FileOpenWrite(char *Path)
{
    int HandleIndex = FindHandle();
    FILE *FileHandle;
    fopen_s(&FileHandle, Path, "wb");
    if (!FileHandle)
    {
        return -1;
    }
    FileHandles[HandleIndex] = FileHandle;
    return HandleIndex;
}

void Sys_FileClose(int Handle)
{
    if ((Handle < 0) || (Handle >= MAX_HANDLES))
    {
        return;
    }
    FILE *f = FileHandles[Handle];
    if (!f)
    {
        return;
    }
    fclose(f);
    FileHandles[Handle] = NULL;
}

void Sys_FileSeek(int Handle, int Position)
{
    if ((Handle < 0) || (Handle >= MAX_HANDLES))
    {
        return;
    }
    FILE *f = FileHandles[Handle];
    if (!f)
    {
        return;
    }
    fseek(f, Position, SEEK_SET);
}

int Sys_FileRead(int Handle, void *Dest, int Count)
{
    if ((Handle < 0) || (Handle >= MAX_HANDLES))
    {
        return -1;
    }
    FILE *f = FileHandles[Handle];
    if (!f)
    {
        return -1;
    }
    int BytesRead = (int) fread_s(Dest, Count, 1, Count, f);
    return BytesRead;
}

int Sys_FileWrite(int Handle, void *Source, int Count)
{
    if ((Handle < 0) || (Handle >= MAX_HANDLES))
    {
        return -1;
    }
    FILE *f = FileHandles[Handle];
    if (!f)
    {
        return -1;
    }
    int BytesWriten = (int)fwrite(Source, 1, Count, f);
    return BytesWriten;
}

// Main window functions

void Sys_Shutdown(void)
{
    IsRunning = FALSE;
}

void Sys_SendKeyEvents(void)
{
    MSG Msg;
    // Get messages from the queue without blocking if the queue is empty
    while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    GlobalInstance = hInstance;

    COM_ParseCmdLine(lpCmdLine);

#if 1
    // Test file functions
    int Size;
    int ReadHandle = Sys_FileOpenRead("..\\sys_win.c", &Size);
    if (ReadHandle == -1)
    {
        int b = 0; b;
    }
    uint8_t *Buffer = malloc(Size);
    Sys_FileRead(ReadHandle, Buffer, Size);

    int WriteHandle = Sys_FileOpenWrite("..\\temp\\sys_win.out");
    if (ReadHandle == -1)
    {
        int b = 0; b;
    }
    Sys_FileWrite(WriteHandle, Buffer, Size);

    Sys_FileClose(ReadHandle);
    Sys_FileClose(WriteHandle);

    free(Buffer);
#endif

    float TimeAccumulated = 0;
    float TargetTime = 1.0f / 60.0f;
    float oldtime = Sys_InitFloatTime();

    Host_Init();

    // Main program loop.
    // Runs until flag is set to false.
    while (IsRunning)
    {


        float newtime = Sys_FloatTime();
        Host_Frame(newtime - oldtime);
        oldtime = newtime;
    }

    Host_Shutdown();

    return 0;
}
