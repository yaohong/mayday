#ifndef _MacroDefine_h__
#define _MacroDefine_h__

template<typename To, typename From>
inline To implicit_cast( From const &f )
{
    return f;
}

#ifdef WIN32
#define NETWORK_ERROR WSAGetLastError()
#else 
#define NETWORK_ERROR errno
#define SOCKET int
#endif

#ifdef WIN32
#define STRING_FMT sprintf_s
#else 
#define STRING_FMT snprintf
#endif

#endif