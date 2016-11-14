#include "quakedefs.h"

// Copy src string to dst
void Q_strcpy(char *dst, const char *src)
{
    while (*src)
    {
        *dst++ = *src++;
    }
    // Ensures destination is null terminated
    *dst = 0;
}

// Copy a number of characters from src string to dst padding with zeroes if needed
void Q_strncpy(char *dst, const char *src, int32_t count)
{
    if (count < 0)
    {
        return;
    }

    while (*src && count)
    {
        *dst++ = *src++;
        --count;
    }

    while (count > 0)
    {
        *dst = 0;
        --count;
    }
}

int32_t Q_strlen(const char *str)
{
    int32_t count = 0;

    while (str[count])
    {
        ++count;
    }

    return count;
}

// String comparison (0 if equal, -1 if s1 < s2, 1 if s1 > s2)
int32_t Q_strcmp(const char *s1, const char *s2)
{
    // Skip part where values are equal
    while (*s1 == *s2)
    {
        // Both strings ended at terminator so they are equal
        if (*s1 == 0)
        {
            return 0;
        }
        ++s1;
        ++s2;
    }

    // Reached two chars that are different. Find out which comes first.
    return ((*s1 < *s2) ? -1 : 1);
}

// Convert a string with a decimal or hexadecimal signed/unsigned number to an integer value
int32_t Q_atoi(const char *str)
{
    int32_t sign = 1;
    int32_t val = 0;

    if (*str == '-')
    {
        sign = -1;
        str++;
    }

    // hexadecimal (prefixed by 0x or 0X)
    if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')))
    {
        str += 2;
        for (;;)
        {
            const char c = *str;
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
    for (;;)
    {
        const char c = *str;
        str++;
        // Reached a non-numerical char, so return the value found so far
        if ((c < '0') || (c > '9'))
        {
            return sign * val;
        }
        val = (val * 10) + (c - '0');
    }
}