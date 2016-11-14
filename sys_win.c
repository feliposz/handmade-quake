#include <Windows.h>

#define MAX_NUM_ARGVS 50
int argc = 1;
char *largv[MAX_NUM_ARGVS];

// Check if parameter was given at command line and return it's position if found or 0 if not found
int COM_CheckParm(char *Parm)
{
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(Parm, largv[i]))
        {
            return i;
        }
    }
    return 0;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // Keep first argument empty for now
    largv[0] = "";

    while (*lpCmdLine && (argc < sizeof(largv)))
    {
        // Skip spaces and non-printable chars
        while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
        {
            lpCmdLine++;
        }

        // Capture argument
        if (*lpCmdLine)
        {
            largv[argc++] = lpCmdLine;

            // Keep printable chars together and place a string terminator after them
            while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine < 126)))
            {
                lpCmdLine++;
            }
            if (*lpCmdLine)
            {
                *lpCmdLine++ = '\0';
            }
        }
    }
    // Make sure last argument is empty (as a terminator?)
    largv[argc] = "";

    // Just testing parameter parsing
    int test = COM_CheckParm("-setalpha");
    int value = 0;
    if (test > 0)
    {
        value = atoi(largv[test + 1]);
    }

    return 0;
}