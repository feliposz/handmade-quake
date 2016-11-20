#include "quakedefs.h"
#include "winquake.h"

double realtime = 0;
double old_realtime = 0;
double host_frametime = 0;

qboolean Host_FilterTime(float timedelta)
{
    realtime += timedelta;

    if (realtime - old_realtime < 1.0 / 72.0)
        return false;

    host_frametime = realtime - old_realtime;
    old_realtime = realtime;

    return true;
}

void Host_Init(void)
{
    srand(time(NULL));
    VID_Init();
}

void Host_Frame(float timedelta)
{
    if (!Host_FilterTime(timedelta))
    {
        return;
    }

    Sys_SendKeyEvents();

    VID_Update();
}

void Host_Shutdown(void)
{
    VID_Shutdown();
}
