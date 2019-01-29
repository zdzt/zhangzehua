#ifndef _NET_BASE_GATHER_SLAVE_H_
#define _NET_BASE_GATHER_SLAVE_H_

#include "NetBaseSlave.h"

template<typename T>
class CNetBaseGatherSlave:public CNetBaseSlave<T>
{//���ڲɼ�����,���������ڲɼ��඼�Ӵ�������
protected://������Ա
	virtual int StartWork() = 0;
public:
	virtual int StartService();
};

template<typename T>
int CNetBaseGatherSlave<T>::StartService()
{
	CBaseProtocol<T>::PrintLog(LOGTIP,"%s�ӹ�Լ��ʼ�ڶ˿� %d ������ ",CBaseProtocol<T>::ParaFunc.ProtocolLibName,CBaseProtocol<T>::ParaFunc.PortId);

	this->StartWork();

	CBaseProtocol<T>::PrintLog(LOGTIP,"%s����Լ�ڶ˿�%d�����н���",CBaseProtocol<T>::ParaFunc.ProtocolLibName,CBaseProtocol<T>::ParaFunc.PortId);

	return 1;
}


#endif