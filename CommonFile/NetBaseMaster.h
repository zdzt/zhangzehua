#ifndef NETBASEMASTER_H_
#define NETBASEMASTER_H_

#include "BaseProtocol.h"

template<typename T>
class CNetBaseMaster:public CBaseMasterProtocol<T>
{
public:
	void WantConnectToDev(int devId, char *ip, USHORT ipport);
	BOOL IsDevConnected(int devId);
	void CloseConnectToDev(int devId);
	BOOL RegCallBackFuncOnDev(int devId, ICallBackFunc *cbf, int cbp);
	int	SendToDev(int devId, char *buf, int len);
};

template<typename T>
void CNetBaseMaster<T>::WantConnectToDev(int devId, char *ip, USHORT ipport)
{
	if(CBaseProtocol<T>::ParaFunc.WantConnectToDev == NULL)
		return;

	CBaseProtocol<T>::ParaFunc.WantConnectToDev(CBaseProtocol<T>::ParaFunc.PortId,devId,ip,ipport);
}

template<typename T>
BOOL CNetBaseMaster<T>::IsDevConnected(int devId)
{
	if(CBaseProtocol<T>::ParaFunc.IsDevConnected == NULL)
		return 0;

	return CBaseProtocol<T>::ParaFunc.IsDevConnected(CBaseProtocol<T>::ParaFunc.PortId,devId);
}

template<typename T>
void CNetBaseMaster<T>::CloseConnectToDev(int devId)
{
	if(CBaseProtocol<T>::ParaFunc.CloseConnectToDev == NULL)
		return;

	CBaseProtocol<T>::ParaFunc.CloseConnectToDev(CBaseProtocol<T>::ParaFunc.PortId,devId);
}

template<typename T>
BOOL CNetBaseMaster<T>::RegCallBackFuncOnDev(int devId, ICallBackFunc *cbf, int cbp)
{
	if(CBaseProtocol<T>::ParaFunc.RegCallBackFuncOnDev == NULL)
		return 0;
	return CBaseProtocol<T>::ParaFunc.RegCallBackFuncOnDev(CBaseProtocol<T>::ParaFunc.PortId,devId,cbf,cbp);
}

template<typename T>
int CNetBaseMaster<T>::SendToDev(int devId, char *buf, int len)
{
	if(CBaseProtocol<T>::ParaFunc.SendToDev == NULL)
		return 0;

	return CBaseProtocol<T>::ParaFunc.SendToDev(CBaseProtocol<T>::ParaFunc.PortId,devId,buf,len);
}


#endif
