#ifndef _NET_BASE_SLAVE_H_
#define _NET_BASE_SLAVE_H_

#include "BaseProtocol.h"

template<typename T>
class CNetBaseSlave:public CBaseSlaveProtocol<T>
{
public:
	int		GetLinkNum();
	void	WantConnect(int linkNo);
	BOOL	IsLinkConnected(int linkNo);
	void	CloseConnect( int linkNo);
	BOOL	RegCallBackFuncOnPort( ICallBackFunc *cbf);
	int		SendToLink(int linkNo, char *buf, int len);
};

template<typename T>
int CNetBaseSlave<T>::GetLinkNum()
{
	if(CBaseSlaveProtocol<T>::ParaFunc.GetLinkNum == NULL)
		return 0;//最小值0，不能连接

	return CBaseSlaveProtocol<T>::ParaFunc.GetLinkNum(CBaseSlaveProtocol<T>::ParaFunc.PortId);
}

template<typename T>
void CNetBaseSlave<T>::WantConnect(int linkNo)
{
	if(CBaseSlaveProtocol<T>::ParaFunc.WantConnect == NULL)
		return;

	CBaseSlaveProtocol<T>::ParaFunc.WantConnect(CBaseSlaveProtocol<T>::ParaFunc.PortId,linkNo);
}

template<typename T>
BOOL CNetBaseSlave<T>::IsLinkConnected(int linkNo)
{	
	if(CBaseSlaveProtocol<T>::ParaFunc.IsLinkConnected == NULL)
		return FALSE;

	return CBaseSlaveProtocol<T>::ParaFunc.IsLinkConnected(CBaseSlaveProtocol<T>::ParaFunc.PortId,linkNo);
}

template<typename T>
void CNetBaseSlave<T>::CloseConnect( int linkNo)
{
	if(CBaseSlaveProtocol<T>::ParaFunc.CloseConnect == NULL)
		return;

	CBaseSlaveProtocol<T>::ParaFunc.CloseConnect(CBaseSlaveProtocol<T>::ParaFunc.PortId);
}

template<typename T>
BOOL CNetBaseSlave<T>::RegCallBackFuncOnPort( ICallBackFunc *cbf)
{
	if(CBaseSlaveProtocol<T>::ParaFunc.RegCallBackFuncOnPort == NULL)
		return FALSE;

	return CBaseSlaveProtocol<T>::ParaFunc.RegCallBackFuncOnPort(CBaseSlaveProtocol<T>::ParaFunc.PortId,cbf);
}

template<typename T>
int CNetBaseSlave<T>::SendToLink(int linkNo, char *buf, int len)
{
	if(CBaseSlaveProtocol<T>::ParaFunc.SendToLink == NULL)
		return 0;

	return CBaseSlaveProtocol<T>::ParaFunc.SendToLink(CBaseSlaveProtocol<T>::ParaFunc.PortId,linkNo,buf,len);
}

#endif