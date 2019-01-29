#pragma once

struct MString
{
	int len;
	char *buf;
};

#ifndef _WINDOWS
//Linux
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <time.h>

#ifndef UINT
#define UINT unsigned int
#endif

#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef ULONG
#define ULONG	unsigned long
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef LPVOID
#define LPVOID void *
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define __declspec(novtable)
#define AFX_MANAGE_STATE(AfxGetStaticModuleState)
#define __stdcall

typedef pthread_mutex_t HThreadLock;

#define InitThreadLock(threadlock)	pthread_mutex_init(threadlock,NULL)
#define LockThread(threadlock)	pthread_mutex_lock(threadlock)
#define UnLockThread(threadlock)	pthread_mutex_unlock(threadlock)
#define DestroyThreadLock(threadlock) pthread_mutex_destroy(threadlock)

#define HANDLE pthread_t

#else


#pragma warning( disable : 4996)

typedef CRITICAL_SECTION HThreadLock;
#define InitThreadLock(threadlock) InitializeCriticalSection(threadlock);
#define LockThread(threadlock)		EnterCriticalSection(threadlock);
#define UnLockThread(threadlock)	LeaveCriticalSection(threadlock)
#define DestroyThreadLock(threadlock) DeleteCriticalSection(threadlock)

#endif






