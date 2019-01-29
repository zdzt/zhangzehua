#ifndef BASEPROTOCOL_H_
#define BASEPROTOCOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ProtocolInterface.h"

#ifndef _WINDOWS

#ifndef RULESTATUTENAME
#define RULESTATUTENAME "一体机规约"
#endif

#define VSNPRINTF vsnprintf
const char * const OCEAN_LFCR ="";
inline void Sleep(int SleepTime)
{
	struct timeval waittime;
	waittime.tv_sec = SleepTime/1000;
	waittime.tv_usec = (SleepTime%1000)*1000;

	select(0,NULL,NULL,NULL,&waittime);
}

inline int XSnprintf(char * const Dest,const int MaxLen,const char *fmt,...)
{
	va_list fmtparam;
	va_start(fmtparam,fmt);
	int retsize = VSNPRINTF(Dest,MaxLen-1,fmt,fmtparam);
	va_end(fmtparam);

	if(retsize == -1 && errno == EINVAL)
		retsize = 0;
	else if(retsize == -1)
		retsize = MaxLen -1;

	Dest[retsize] = 0;
	return retsize;
}

#else

#ifndef _UTF8_
#define _UTF8_
#pragma execution_character_set("utf-8")	//使用UTF8字符集
#endif


#ifndef RULESTATUTENAME
#define RULESTATUTENAME "平台规约"
#endif

#define VSNPRINTF _vsnprintf_s
const char * const OCEAN_LFCR ="\r\n";

inline int XSnprintf(char * const Dest,const int MaxLen,const char *fmt,...)
{
	va_list fmtparam;
	va_start(fmtparam,fmt);
	int retsize = VSNPRINTF(Dest,MaxLen,MaxLen-1,fmt,fmtparam);
	va_end(fmtparam);

	if(retsize == -1 && errno == EINVAL)
		retsize = 0;
	else if(retsize == -1)
		retsize = MaxLen -1;

	Dest[retsize] = 0;
	return retsize;
}

#endif

//函数模板

template <typename T>
static int PrintLOG (T ParaFunc,int level,const char *fmt,va_list fmtparam)
{
	if(ParaFunc.PrintLog == NULL)
		return 0;
	char logbuf[LOGBUFLEN];
	int len = VSNPRINTF(logbuf,LOGBUFLEN - 1,fmt,fmtparam);
	if(len == -1)
	{
		if(errno != EINVAL)
			logbuf[LOGBUFLEN - 1] = 0;
		else
			logbuf[0] = 0;
	}

#ifndef _WINDOWS
	time_t curtime = time(NULL);
	char *timestr  = ctime(&curtime);
#else
	char timestr[128];
	__time64_t curtime = _time64(NULL);
	_ctime64_s(timestr,&curtime);
#endif

	switch(level)
	{
	case LOGTIP:
		ParaFunc.PrintLog("%s提示:%s,PortName=%s @ %s %s",RULESTATUTENAME,logbuf,ParaFunc.PortName,timestr,OCEAN_LFCR);
		break;
	case LOGWARNING:
		ParaFunc.PrintLog("%s警告:%s,PortName=%s @ %s %s",RULESTATUTENAME,logbuf,ParaFunc.PortName,timestr,OCEAN_LFCR);
		break;
	case LOGERROR:
		ParaFunc.PrintLog("%s错误:%s,PortName=%s @ %s %s",RULESTATUTENAME,logbuf,ParaFunc.PortName,timestr,OCEAN_LFCR);
		break;
	default:
		ParaFunc.PrintLog("%s未知提示:%s,PortName=%s @ %s %s",RULESTATUTENAME,logbuf,ParaFunc.PortName,timestr,OCEAN_LFCR);
		break;
	}

	return 1;
}

template<typename T>
static int PrintLog(T *ParaFunc,int level,const char *fmt,...)
{

	va_list fmtparam;
	va_start(fmtparam,fmt);
	int retval = PrintLOG(*ParaFunc,level,fmt,fmtparam);
	va_end(fmtparam);

	return retval;
}



//类模板
template<typename T>
class CBaseProtocol
{
public:
	T ParaFunc;
protected://日志和信息处理
	virtual int PrintLog(int level,const char *fmt,...);//打印日志
	virtual int PrintSysSoe(int level,const char *fmt,...);//写入系统事
protected:
	virtual int NeedQuit();//是否需要退出
};

template<typename T>
int CBaseProtocol<T>::PrintLog(int level,const char *fmt,...)
{//打印日志
	if(ParaFunc.PrintLog == NULL)
		return 0;

	va_list fmtparam;
	va_start(fmtparam,fmt);
	int retval = PrintLOG(ParaFunc,level,fmt,fmtparam);
	va_end(fmtparam);

	return retval;
}

template<typename T>
int CBaseProtocol<T>::PrintSysSoe(int level,const char *fmt,...)
{//写入系统事件
	if(ParaFunc.WriteSysInfoSoe == NULL)
		return 0;

	char logbuf[LOGBUFLEN];
	va_list fmtparam;
	va_start(fmtparam,fmt);
	int len = VSNPRINTF(logbuf,LOGBUFLEN - 1,fmt,fmtparam);
	va_end(fmtparam);

	if(len == -1)
	{
		if(errno != EINVAL)
			logbuf[LOGBUFLEN - 1] = 0;
		else
			logbuf[0] = 0;
	}

	switch(level)
	{
	case LOGTIP:
		ParaFunc.WriteSysInfoSoe("%s提示:%s,PortName是%s ",RULESTATUTENAME,logbuf,ParaFunc.PortName);
		break;
	case LOGWARNING:
		ParaFunc.WriteSysInfoSoe("%s警告:%s,PortName是%s ",RULESTATUTENAME,logbuf,ParaFunc.PortName);
		break;
	case LOGERROR:
		ParaFunc.WriteSysInfoSoe("%s错误:%s,PortName是%s  ",RULESTATUTENAME,logbuf,ParaFunc.PortName);
		break;
	default:
		ParaFunc.WriteSysInfoSoe("%s未知提示:%s,PortName是%s ",RULESTATUTENAME,logbuf,ParaFunc.PortName);
		break;
	}

	return 1;
}

template<typename T>
int CBaseProtocol<T>::NeedQuit()
{
	if(ParaFunc.NeedQuit == NULL)
		return 1;
	return ParaFunc.NeedQuit(ParaFunc.PortId);
}

//类模板
template<typename T>
class CBaseMasterProtocol:public CBaseProtocol<T>
{
public:
	virtual int SampleEnable();//是否需要采集
	virtual int NeedColdStandbyStop();//是否要冷备停机
};

template<typename T>
int CBaseMasterProtocol<T>::SampleEnable()
{
	if(ParaFunc.SampleEnable == NULL)
		return 1;

	return ParaFunc.SampleEnable();
}

template<typename T>
int CBaseMasterProtocol<T>::NeedColdStandbyStop()
{//是否要冷备停机
	if(!SampleEnable())
	{
		return 1;
	}

	return 0;
}

//类模板
template<typename T>
class CBaseSlaveProtocol:public CBaseProtocol<T>
{

};


#endif
