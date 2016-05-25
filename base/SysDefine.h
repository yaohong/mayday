#ifndef _SysDefine_h__
#define _SysDefine_h__
#ifdef WIN32 
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")

#ifdef WIN32 
#define pid_t uint32
#define pthread_t uint32
#endif
#else
#include <sys/socket.h>    // socket     connect  write
#include <arpa/inet.h>      // inet_pton
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/eventfd.h>
#include <algorithm>
#endif











#endif