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
