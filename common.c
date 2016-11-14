#include "quakedefs.h"

int32_t com_argc = 1;
uint8_t *com_argv[MAX_NUM_ARGVS + 1];

void COM_ParseCmdLine(uint8_t *lpCmdLine)
{
    // Keep first argument empty for now
    com_argv[0] = "";

    while (*lpCmdLine && (com_argc < sizeof(com_argv)))
    {
        // Skip spaces and non-printable chars
        while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
        {
            lpCmdLine++;
        }

        // Capture argument
        if (*lpCmdLine)
        {
            com_argv[com_argc++] = lpCmdLine;

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
}

// Check if parameter was given at command line and return it's position if found or 0 if not found
int32_t COM_CheckParm(uint8_t *Parm)
{
    for (int32_t i = 1; i < com_argc; i++)
    {
        if (!Q_strcmp(Parm, com_argv[i]))
        {
            return i;
        }
    }
    return 0;
}