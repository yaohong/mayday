#include "DefaultLog.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "Timestamp.h"
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
        Timestamp t = Timestamp::now();
        std::string st = t.toFormattedString();
        fprintf( stderr, "[DEBUG] [%s]: %s\n", st.c_str(), szBuf );
        fflush( stderr );
    }

    void LogWarning( const char *fmt, ... )
    {
        ZB_LOG_FMT();
        Timestamp t = Timestamp::now();
        std::string st = t.toFormattedString();
        fprintf( stderr, "[WARNING] [%s]: %s\n", st.c_str(), szBuf );
        fflush( stderr );
    }

    void LogError( const char *fmt, ... )
    {
        ZB_LOG_FMT();
        Timestamp t = Timestamp::now();
        std::string st = t.toFormattedString();
        fprintf( stderr, "[ERROR] [%s]: %s\n", st.c_str(), szBuf );
        fflush( stderr );
    }

    void LogInfo( const char *fmt, ... )
    {
        ZB_LOG_FMT();
        Timestamp t = Timestamp::now();
        std::string st = t.toFormattedString();
        fprintf( stderr, "[INFO] [%s]: %s\n", st.c_str(), szBuf );
        fflush( stderr );
    }

    void LogFatal( const char *fmt, ... )
    {
        ZB_LOG_FMT();
        Timestamp t = Timestamp::now();
        std::string st = t.toFormattedString();
        fprintf( stderr, "[FATAL] [%s]: %s\n", st.c_str(), szBuf );
        fflush( stderr );
        abort();
    }
}

