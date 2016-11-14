#include <Windows.h>
#include <stdint.h>

#define MAX_NUM_ARGVS 50
int32_t argc = 1;
uint8_t *largv[MAX_NUM_ARGVS+1];

// String comparison (0 if equal, -1 if s1 < s2, 1 if s1 > s2)
uint32_t Q_strcmp(uint8_t *s1, uint8_t *s2)
{
    // Skip part where values are equal
    while (*s1++ == *s2++)
    {
        // Both strings ended at terminator so they are equal
        if (*s1 == 0)
        {
            return 0;
        }
    }

    // Reached two chars that are different. Find out which comes first.
    return ((*s1 < *s2) ? -1 : 1);
}

// Convert a string with a decimal or hexadecimal signed/unsigned number to an integer value
uint32_t Q_atoi(uint8_t *str)
{
    uint32_t sign = 1;
    uint32_t val = 0;
    
    if (*str == '-')
    {
        sign = -1;
        str++;
    }

    // hexadecimal (prefixed by 0x or 0X)
    if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')))
    {
        str += 2;
        while (1)
        {
            uint8_t c = *str;
            str++;
            // Reached a non-numerical char, so return the value found so far
            if ((c >= '0') && (c <= '9'))
            {
                val = (val * 16) + (c - '0');
            }
            else if ((c >= 'a') && (c <= 'f'))
            {
                val = (val * 16) + (c - 'a' + 10);
            }
            else if ((c >= 'A') && (c <= 'F'))
            {
                val = (val * 16) + (c - 'F' + 10);
            }
            else
            {
                return sign * val;
            }
        }
    }

    // decimal
    while (1)
    {
        uint8_t c = *str;
        str++;
        // Reached a non-numerical char, so return the value found so far
        if ((c < '0') || (c > '9'))
        {
            return sign * val;
        }
        val = (val * 10) + (c - '0');
    }    
}

// Check if parameter was given at command line and return it's position if found or 0 if not found
int32_t COM_CheckParm(uint8_t *Parm)
{
    for (int32_t i = 1; i < argc; i++)
    {
        if (!Q_strcmp(Parm, largv[i]))
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

    // Just testing parameter parsing
    int32_t test = COM_CheckParm("-setalpha");
    int32_t value = 0;
    if (test > 0)
    {
        value = Q_atoi(largv[test + 1]);
    }

    return 0;
}