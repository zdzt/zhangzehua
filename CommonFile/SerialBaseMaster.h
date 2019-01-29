#ifndef SERIALBASEMASTER_H
#define SERIALBASEMASTER_H

#include "BaseProtocol.h"

#ifndef RULESTATUTENAME
#define RULESTATUTENAME "串口主规约"
#endif

template <typename T>
class CSerialBaseMaster:public CBaseMasterProtocol<T>
{//串口采集类基类
protected://串口操作
	virtual int OpenCom();
	virtual int WriteCom(char *sendbuf,int sendbuflen);
	virtual int WriteCom(unsigned char *sendbuf,int sendbuflen);
	virtual int ReadCom(char *recbuf,int wantlen,int waittime);
	virtual int ReadCom(unsigned char *recbuf,int wantlen,int waittime);
	virtual void CloseCom();
};

template<typename T>
int CSerialBaseMaster<T>::OpenCom()
{
	if(CBaseProtocol<T>::ParaFunc.OpenCom == NULL)
		return 0;

	return CBaseProtocol<T>::ParaFunc.OpenCom(CBaseProtocol<T>::ParaFunc.PortId);
}

template<typename T>
int CSerialBaseMaster<T>::WriteCom(char *sendbuf,int sendbuflen)
{
	if(CBaseProtocol<T>::ParaFunc.WriteCom == NULL)
	{
		return 0;
	}

	return CBaseProtocol<T>::ParaFunc.WriteCom(CBaseProtocol<T>::ParaFunc.PortId,sendbuf,sendbuflen);
}

template<typename T>
int CSerialBaseMaster<T>::WriteCom(unsigned char *sendbuf,int sendbuflen)
{
	return WriteCom((char *)sendbuf,sendbuflen);
}

template<typename T>
int CSerialBaseMaster<T>::ReadCom(char *recbuf,int wantlen,int waittime)
{
	if(CBaseProtocol<T>::ParaFunc.ReadCom == NULL)
		return 0;

	return CBaseProtocol<T>::ParaFunc.ReadCom(CBaseProtocol<T>::ParaFunc.PortId,recbuf,wantlen,waittime);

}

template<typename T>
int CSerialBaseMaster<T>::ReadCom(unsigned char *recbuf,int wantlen,int waittime)
{
	return ReadCom((char *)recbuf,wantlen,waittime);
}

template<typename T>
void CSerialBaseMaster<T>::CloseCom()
{
	if(CBaseProtocol<T>::ParaFunc.CloseCom == NULL)
		return ;

	CBaseProtocol<T>::ParaFunc.CloseCom(CBaseProtocol<T>::ParaFunc.PortId);
}


#endif
