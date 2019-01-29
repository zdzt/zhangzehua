#ifndef _NET_BASE_GATHER_SLAVE_H_
#define _NET_BASE_GATHER_SLAVE_H_

#include "NetBaseSlave.h"

template<typename T>
class CNetBaseGatherSlave:public CNetBaseSlave<T>
{//串口采集基类,所有主串口采集类都从此类派生
protected://保护成员
	virtual int StartWork() = 0;
public:
	virtual int StartService();
};

template<typename T>
int CNetBaseGatherSlave<T>::StartService()
{
	CBaseProtocol<T>::PrintLog(LOGTIP,"%s从规约开始在端口 %d 上运行 ",CBaseProtocol<T>::ParaFunc.ProtocolLibName,CBaseProtocol<T>::ParaFunc.PortId);

	this->StartWork();

	CBaseProtocol<T>::PrintLog(LOGTIP,"%s主规约在端口%d上运行结束",CBaseProtocol<T>::ParaFunc.ProtocolLibName,CBaseProtocol<T>::ParaFunc.PortId);

	return 1;
}


#endif