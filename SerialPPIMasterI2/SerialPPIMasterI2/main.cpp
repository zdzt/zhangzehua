#include "stdafx.h"

//*******************************************************************************************
//规约初始化
//*******************************************************************************************
#include "SerialPPIMasterI2.h"

#define LIBVER_INT 0x05000000
#define LIBMODIFYSTR "2014-12-17"

struct SThreadEntry
{
	bool RuningFlag;
	SProtocolInit*	ProtocolEntry;
};


//GetInterfaceType函数
//入口：SCADA版本号，四个字节的整数，比如：4.0.2.1, 其中4是主版本号，表示平台系统，0是接口版本，2和1是规约版本
//出口：返回接口类型给SCADA，SCADA根据该接口类型提供一套功能函数的指针集合给ProtocolInit
extern "C" __declspec(dllexport) int GetInterfaceType(int scadaversion)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//备注：采用GetInterfaceType机制后，旧的接口会一直保留，除非发生重大变化
	//当接口发生重大变化时，SCADA版本号会提升第2个字节，所以此处判断版本号是否在5.0和5.1之间
	//因为版本为5.1时，说明接口已经发生重大变化了，不再支持了
	if((scadaversion >= VERSION_INT) && ((LIBVER_INT & 0xFFFF0000) <= (scadaversion & 0xFFFF0000)) && (scadaversion < VERSION_NOTSUPORT_INT))//无数据表规约判断
		return InterFaceNum;//返回接口号2，下面的ProtocolInit函数将收到一个SInterfaceComMaster结构体

	return 0;
}

#ifndef _WINDOWS
void * ServiceThread(LPVOID lpParam)
#else
UINT __stdcall ServiceThread(LPVOID lpParam)
#endif
{
	SThreadEntry *ThreadEntry = (SThreadEntry *)lpParam;
	PrintLog(ThreadEntry->ProtocolEntry,LOGTIP,"%s规约开始在端口 %s 上运行 ",ThreadEntry->ProtocolEntry->ProtocolLibName,ThreadEntry->ProtocolEntry->PortName);

	CSerialPPIMaster serviceobj;
	if(serviceobj.InitRule(*(ThreadEntry->ProtocolEntry))== -1)
	{
		PrintLog(ThreadEntry->ProtocolEntry,LOGTIP,"%s规约初始化失败",ThreadEntry->ProtocolEntry->ProtocolLibName);
		ThreadEntry->RuningFlag = TRUE;//置位该标志告诉ProtocolInit函数可以返回了
		PrintLog(ThreadEntry->ProtocolEntry,LOGTIP,"%s规约在端口 %s 上运行结束 ",ThreadEntry->ProtocolEntry->ProtocolLibName,ThreadEntry->ProtocolEntry->PortName);
		return 0;
	}

	//如果从Run中退出，说明规约要退出
	//释放规约中分配的内存
	ThreadEntry->RuningFlag = TRUE;//置位该标志告诉ProtocolInit函数可以返回了
	serviceobj.StartService();//开始运行规约了

#ifndef _WINDOWS
	return NULL;
#else
	return 1;
#endif
}

//ProtocolInit函数
//功能：规约入口函数，SCADA核心调用，完成规约线程初始化
//要求：该函数里完成规约线程建立工作，并立刻返回（如果函数不返回，SCADA核心不会继续运行）
extern "C" __declspec(dllexport) HANDLE ProtocolInit(SProtocolInit *it)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	PrintLog(it,LOGTIP,"%s规约的接口号是:%d,版本是:%d.%d.%d.%d@%s,编译时间是:%s %s ,PortId=%d",it->ProtocolLibName,InterFaceNum,(LIBVER_INT >> 24)&0xFF,(LIBVER_INT >> 16)&0xFF,(LIBVER_INT >> 8)&0xFF,LIBVER_INT&0xFF,LIBMODIFYSTR,__DATE__,__TIME__,it->PortId);

	SThreadEntry te;
	te.RuningFlag = FALSE;
	te.ProtocolEntry = it;//保存入口结构体指针，传递给规约线程

#ifndef _WINDOWS
	HANDLE hservice;
	pthread_create(&hservice,NULL,ServiceThread,&te);

	while(te.RuningFlag == 0)
	{
		Sleep(50);
	}

	return hservice;
#else
	UINT id;
	HANDLE hservice = (HANDLE)_beginthreadex(NULL, 0,  ServiceThread, &te, 0, &id);

	while(te.RuningFlag == 0)
	{
		Sleep(50);
	}

	return hservice;
#endif
}
