#include "DefaultLog.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32

#define _FMT_PRINF vsprintf_s(szBuf, 1024 * 4, fmt, va);
#else
#define _FMT_PRINF vsprintf(szBuf, fmt, va);
#endif
#define ZB_LOG_FMT() static char szBuf[1024 * 4]; szBuf[0] = 0; va_list va; va_start(va, fmt); _FMT_PRINF; va_end(va); 
namespace mayday
{

    void LogDebug(const char *fmt, ... )
    {
        ZB_LOG_FMT();
        printf("DEBUG: %s\n", szBuf);
    }

    void LogWarning( const char *fmt, ... )
    {
        ZB_LOG_FMT();
        printf( "WARNING: %s\n", szBuf );
    }

    void LogError( const char *fmt, ... )
    {
        ZB_LOG_FMT();
        printf( "ERROR: %s\n", szBuf );
    }

    void LogInfo( const char *fmt, ... )
    {
        ZB_LOG_FMT();
        printf( "INFO: %s\n", szBuf );
    }

    void LogFatal( const char *fmt, ... )
    {
        ZB_LOG_FMT();
        printf( "FATAL: %s\n", szBuf );
        abort();
    }
}

