#ifndef SERIALBASEGATHERMASTER_H
#define SERIALBASEGATHERMASTER_H

#include "SerialBaseMaster.h"

template<typename T>
class CSerialBaseGatherMaster:public CSerialBaseMaster<T>
{//串口采集基类,所有主串口采集类都从此类派生
protected://保护成员
	virtual int StartWork() = 0;
public:
	virtual int StartService();
};

template<typename T>
int CSerialBaseGatherMaster<T>::StartService()
{
	CBaseProtocol<T>::PrintLog(LOGTIP,"%s主规约开始在端口 %d 上运行 ",CBaseProtocol<T>::ParaFunc.ProtocolLibName,CBaseProtocol<T>::ParaFunc.PortId);

	unsigned int scount = 0;
	while(!this->NeedQuit())
	{//不需要退出，要采集
		if(!this->NeedColdStandbyStop())
		{
			CBaseProtocol<T>::PrintLog(LOGWARNING,"冷备停止，采集己启动");
			this->StartWork();
		}
		else
		{//休眠
			if(scount%(60*15) == 0)
				CBaseProtocol<T>::PrintLog(LOGWARNING,"冷备启动，采集己停止");
			Sleep(1000);
		}
	}

	CBaseProtocol<T>::PrintLog(LOGTIP,"%s主规约在端口%d上运行结束",CBaseProtocol<T>::ParaFunc.ProtocolLibName,CBaseProtocol<T>::ParaFunc.PortId);

	return 1;
}


#endif
