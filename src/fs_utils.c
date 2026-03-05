#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "flight_system.h"

int fs_printf(const char *fmt, ...)
{
    va_list args;
    va_list args_copy;
    int needed = 0;
    char *buf = NULL;
    int rc = 0;

    va_start(args, fmt);
    va_copy(args_copy, args);
    needed = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    if (needed < 0)
    {
        va_end(args);
        return needed;
    }

    buf = (char *)malloc((size_t)needed + 1U);
    if (!buf)
    {
        va_end(args);
        return -1;
    }

    vsnprintf(buf, (size_t)needed + 1U, fmt, args);
    va_end(args);

#ifdef _WIN32
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode))
        {
            int wide_len = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
            if (wide_len > 0)
            {
                wchar_t *wbuf = (wchar_t *)malloc(sizeof(wchar_t) * (size_t)wide_len);
                if (wbuf)
                {
                    DWORD written = 0;
                    MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, wide_len);
                    WriteConsoleW(hOut, wbuf, (DWORD)(wide_len - 1), &written, NULL);
                    free(wbuf);
                    free(buf);
                    return needed;
                }
            }
        }
    }
#endif

    rc = fprintf(stdout, "%s", buf);
    free(buf);
    return rc;
}

void fs_copy_text(char *dst, int dst_size, const char *src)
{
    if (dst_size <= 0)
    {
        return;
    }
    snprintf(dst, (size_t)dst_size, "%s", src ? src : "");
}

void fs_trim_newline(char *s)
{
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == '\n')
    {
        s[n - 1] = '\0';
    }
}

void fs_read_line(const char *prompt, char *buf, int len)
{
    printf("%s", prompt);
    if (fgets(buf, len, stdin) == NULL)
    {
        buf[0] = '\0';
        return;
    }
    fs_trim_newline(buf);
}

int fs_read_int(const char *prompt)
{
    char buf[64];
    fs_read_line(prompt, buf, (int)sizeof(buf));
    return atoi(buf);
}

double fs_read_double(const char *prompt)
{
    char buf[64];
    fs_read_line(prompt, buf, (int)sizeof(buf));
    return atof(buf);
}

int fs_parse_hhmm(const char *time_str)
{
    int h = -1;
    int m = -1;
    if (sscanf(time_str, "%d:%d", &h, &m) != 2)
    {
        return -1;
    }
    if (h < 0 || h > 23 || m < 0 || m > 59)
    {
        return -1;
    }
    return h * 60 + m;
}

double fs_group_discount(const char *group_name)
{
    if (strcmp(group_name, "student") == 0)
        return 0.90;
    if (strcmp(group_name, "soldier") == 0)
        return 0.85;
    if (strcmp(group_name, "elder") == 0)
        return 0.80;
    return 1.00;
}
