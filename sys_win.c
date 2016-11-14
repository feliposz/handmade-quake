#include "winquake.h"
#include "quakedefs.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    COM_ParseCmdLine(lpCmdLine);

    // Just testing parameter parsing
    int32_t test = COM_CheckParm("-setalpha");
    int32_t value = 0;
    if (test > 0)
    {
        value = Q_atoi(com_argv[test + 1]);
    }

    // A few tests for the string copying
    uint8_t s1[] = "Handmade Quake";
    uint8_t s2[20] = {0};
    uint8_t s3[20] = {0};
    uint8_t s4[20] = {0};

    int32_t i1 = Q_strlen(s1);

    Q_strncpy(s2, s1, 15);
    int32_t i2 = Q_strlen(s2);

    Q_strncpy(s3, s1, 8);
    int32_t i3 = Q_strlen(s3);

    Q_strcpy(s4, s3);
    int32_t i4 = Q_strlen(s4);

    int32_t i5 = Q_strcmp(s1, s4);
    int32_t i6 = Q_strcmp(s4, s1);

    return 0;
}