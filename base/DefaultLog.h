#ifndef _DefaultLog_h__
#define _DefaultLog_h__



namespace mayday
{
    void LogDebug( const char *fmt, ... );
    void LogWarning( const char *fmt, ... );
    void LogError( const char *fmt, ... );
    void LogInfo( const char *fmt, ... );
    void LogFatal( const char *fmt, ... );
}

#define MDLog mayday::LogDebug
#define MDWarning mayday::LogWarning
#define MDError mayday::LogError
#define MDInfo mayday::LogInfo
#define MDFatal mayday::LogFatal
#define MDAssert assert

#endif