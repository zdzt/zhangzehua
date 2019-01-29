#include "stdafx.h"
#include "SerialPPIMasterI2.h"
#include "stdlib.h"
#include "stdio.h"
#include <string.h>
#include <stdarg.h>

#define RECBUFLEN 512
#define MAXREADLEN 64

#define DEBUG

CSerialPPIMaster::CSerialPPIMaster()
{
	DevNum = 0;
	DevRegList = NULL;
	DevQueryList = NULL;
}

CSerialPPIMaster::~CSerialPPIMaster()
{
	//删除查询结点
	this->FreeDevQueryList();
	//删除设备结点
	if(DevRegList != NULL)
		ParaFunc.FreeDevAddrRegList(DevRegList,DevNum);
}

//释放查询规则列表
int CSerialPPIMaster::FreeDevQueryList()
{
	if(DevQueryList != NULL)
	{
		for(int i=0;i<DevNum;i++)
		{
			if(DevQueryList[i].QueryRuleList != NULL)
			{
				DevQueryList[i].QueryRuleNum = 0;
				delete DevQueryList[i].QueryRuleList;
				DevQueryList[i].QueryRuleList = NULL;
			}

			if(DevQueryList[i].UrgentRuleSnList != NULL)
			{
				DevQueryList[i].UrgentRuleNum = 0;
				delete [] DevQueryList[i].UrgentRuleSnList;
				DevQueryList[i].UrgentRuleSnList = NULL;
			}

			if(DevQueryList[i].SpecialRuleList != NULL)
			{
				DevQueryList[i].SpecialRuleNum = 0;
				delete [] DevQueryList[i].SpecialRuleList;
				DevQueryList[i].SpecialRuleList = NULL;
			}
		}

		delete [] DevQueryList;
		DevQueryList = NULL;
	}

	return 1;
}

int CSerialPPIMaster::DataType2RegCount(int dt)
{//从数据类型推出占用寄存器个数
	switch(dt)
	{
	case DEVREGCHAR:
	case DEVREGUCHAR:
		return 1;
	case DEVREGUSHORT:
	case DEVREGSHORT:
	case DEVREGBITSTRING://bit 串，在规约中当成ushort一样处理
	case DEVREGBITFIELD:
		return 2;
	case DEVREGUINT:
	case DEVREGINT:
	case DEVREGFLOAT:
	case DEVREGBITFIELD2:
		return 4;
	case DEVREGBIT:
		return 1;
	case DEVREGSTRING:
		return 100;
	default://错误都用日志方式提醒，因为配置错误，规约自身不可能解决错误，必须通过修改配置来更正
		ParaFunc.PrintLog("*CSerialPPIMaster, find wrong reg datatype on port %d!", ParaFunc.PortId);
		return 0;
	}
}

int CSerialPPIMaster::DataTypeAndRegCount2ReadLenAndReadNum(int dt,int regcount,int &readlen,int &readcount)
{
	switch(dt)
	{
	case DEVREGUSHORT:
	case DEVREGSHORT:
	case DEVREGUINT:
	case DEVREGINT:
	case DEVREGFLOAT:
	case DEVREGCHAR:
	case DEVREGUCHAR:
	case DEVREGBITSTRING:
	case DEVREGSTRING:
	case DEVREGBITFIELD:
	case DEVREGBITFIELD2:
		readlen = 2;
		readcount = regcount;
		break;
	case DEVREGBIT:
		readlen = 2;
		readcount = (regcount%8==0)?(regcount>>3):((regcount>>3)+1);
		break;
	}

	return 1;
}

int CSerialPPIMaster::RegType2FunCode(int rt)
{//从寄存器类型中取出功能码
	return rt & 0xffff;//取最低两位字节为功能码
}

int CSerialPPIMaster::RegAddr2Offset(int rd)
{//
	unsigned int offsethight=rd/10;
	unsigned int offsetlow=0;

	return ((offsethight<<3)+offsetlow);
}

int CSerialPPIMaster::RegAddr2WriteOffset(int rd)
{
	unsigned int offsethight=rd/10;
	unsigned int offsetlow=(rd%10)&0x7;

	return ((offsethight<<3)+offsetlow);
}

int CSerialPPIMaster::SetDevCommState(int devsn,int commstate)
{//设置设备通讯状态
	for(int i=0;i<DevQueryList[devsn].SpecialRuleNum;i++)
	{
		int regpos = DevQueryList[devsn].SpecialRuleList[i].RegStartSn;
		if(DevRegList[devsn].DevRegList[regpos].RegAddr == 1)
		{
			DevRegList[devsn].DevRegList[regpos].CommState = 1;
			DevRegList[devsn].DevRegList[regpos].Value.b = commstate;

			ParaFunc.WriteDevRegData(DevRegList[devsn].DevRegList[regpos]);
		}
	}

	return 1;
}

int CSerialPPIMaster::ChangeDevCommState(int devsn,int commstate)
{//改变设备通讯状态
	if(commstate == 0)
	{//设备通讯状态为0
		PrintLog(LOGWARNING,"设备%s的通讯状态转为通讯异常",DevRegList[devsn].DevName);
		this->SetDevCommState(devsn,0);
		DevQueryList[devsn].ErrorQueryRuleCount ++;
	}
	else
	{
		if(DevQueryList[devsn].ErrorQueryRuleCount == 1)
		{
			PrintLog(LOGWARNING,"设备%s的通讯状态转为通讯正常",DevRegList[devsn].DevName);
			this->SetDevCommState(devsn,1);
		}

		DevQueryList[devsn].ErrorQueryRuleCount --;
	}

	return 1;
}

int CSerialPPIMaster::GetRuleNum(int devsn)
{//返回规则数目
	return DevQueryList[devsn].QueryRuleNum;
}

int CSerialPPIMaster::FindUrgentRule(int devsn)
{//寻找到迫切需要采集的规则位置
	for(int i = 0; i < DevQueryList[devsn].UrgentRuleNum; i++)
	{
		if(DevQueryList[devsn].QueryRuleList[DevQueryList[devsn].UrgentRuleSnList[i]].DataUpdateCycle < DevQueryList[devsn].QueryRuleList[DevQueryList[devsn].UrgentRuleSnList[i]].LastingTimeAfterSample)
			return DevQueryList[devsn].UrgentRuleSnList[i];//距离上次采集过去的时间已经超过数据更新周期了，返回规则序号，表示该规则中的寄存器数据需要紧急更新
	}
	return -1;
}

int CSerialPPIMaster::ResponseAnalysis(int devsn,int rulesn, unsigned char *data, int datalen)
{//应答报文正确时，分析数据区并写入寄存器，data指向的不包含报文头和CRC
	int i_start=DevQueryList[devsn].QueryRuleList[rulesn].RegStartSn;
	int i_dataplace=0;
	unsigned char *p_data=NULL;
	int bitpos=0;
	for(;i_start <= DevQueryList[devsn].QueryRuleList[rulesn].RegStopSn;i_start++)
	{//轮询派发数据
		DevRegList[devsn].DevRegList[i_start].CommState=1;
		switch(DevRegList[devsn].DevRegList[i_start].RegDataType)
		{
		case DEVREGUSHORT:
		case DEVREGBITFIELD:
			p_data=(unsigned char *)&DevRegList[devsn].DevRegList[i_start].Value.us;
			//赋值
			i_dataplace = (DevRegList[devsn].DevRegList[i_start].RegAddr-DevQueryList[devsn].QueryRuleList[rulesn].RegStartAddr)/10;
			*(p_data+0)=data[i_dataplace++];
			*(p_data+1)=data[i_dataplace++];
			break;
		case DEVREGSHORT:
			p_data=(unsigned char *)&DevRegList[devsn].DevRegList[i_start].Value.s;
			//赋值
			i_dataplace = (DevRegList[devsn].DevRegList[i_start].RegAddr-DevQueryList[devsn].QueryRuleList[rulesn].RegStartAddr)/10;
			*(p_data+0)=data[i_dataplace++];
			*(p_data+1)=data[i_dataplace++];
			break;
		case DEVREGUINT:
		case DEVREGBITFIELD2:
			p_data=(unsigned char *)&DevRegList[devsn].DevRegList[i_start].Value.ui;
			//赋值
			i_dataplace = (DevRegList[devsn].DevRegList[i_start].RegAddr-DevQueryList[devsn].QueryRuleList[rulesn].RegStartAddr)/10;
			*(p_data+0)=data[i_dataplace++];
			*(p_data+1)=data[i_dataplace++];
			*(p_data+2)=data[i_dataplace++];
			*(p_data+3)=data[i_dataplace++];
			break;
		case DEVREGINT:
			p_data=(unsigned char *)&DevRegList[devsn].DevRegList[i_start].Value.i;
			//赋值
			i_dataplace = (DevRegList[devsn].DevRegList[i_start].RegAddr-DevQueryList[devsn].QueryRuleList[rulesn].RegStartAddr)/10;

			*(p_data+0)=data[i_dataplace++];
			*(p_data+1)=data[i_dataplace++];
			*(p_data+2)=data[i_dataplace++];
			*(p_data+3)=data[i_dataplace++];
			break;
		case DEVREGFLOAT:
			p_data=(unsigned char *)&DevRegList[devsn].DevRegList[i_start].Value.i;
			//赋值
			i_dataplace = (DevRegList[devsn].DevRegList[i_start].RegAddr-DevQueryList[devsn].QueryRuleList[rulesn].RegStartAddr)/10;

			*(p_data+0)=data[i_dataplace++];
			*(p_data+1)=data[i_dataplace++];
			*(p_data+2)=data[i_dataplace++];
			*(p_data+3)=data[i_dataplace++];
			break;
		case DEVREGCHAR:
			i_dataplace = (DevRegList[devsn].DevRegList[i_start].RegAddr-DevQueryList[devsn].QueryRuleList[rulesn].RegStartAddr)/10;
			DevRegList[devsn].DevRegList[i_start].Value.c=data[i_dataplace++];
			break;
		case DEVREGUCHAR:
			i_dataplace = (DevRegList[devsn].DevRegList[i_start].RegAddr-DevQueryList[devsn].QueryRuleList[rulesn].RegStartAddr)/10;
			DevRegList[devsn].DevRegList[i_start].Value.uc=data[i_dataplace++];
			break;
		case DEVREGBIT:
			i_dataplace = (DevRegList[devsn].DevRegList[i_start].RegAddr-DevQueryList[devsn].QueryRuleList[rulesn].RegStartAddr)/10;
			bitpos = DevRegList[devsn].DevRegList[i_start].RegAddr%10;
			DevRegList[devsn].DevRegList[i_start].Value.b=GetBit(data,i_dataplace,bitpos);

			break;
		case DEVREGBITSTRING:
			p_data=(unsigned char *)&DevRegList[devsn].DevRegList[i_start].Value.bitStr;
			i_dataplace = (DevRegList[devsn].DevRegList[i_start].RegAddr-DevQueryList[devsn].QueryRuleList[rulesn].RegStartAddr)/10;
			//赋值
			*(p_data+0)=data[i_dataplace++];
			*(p_data+1)=data[i_dataplace++];
			break;
		case DEVREGSTRING:
			DevRegList[devsn].DevRegList[i_start].Value.str.buf=(char *)data;
			DevRegList[devsn].DevRegList[i_start].Value.str.len=datalen;
			break;
		}

		ParaFunc.WriteDevRegData(DevRegList[devsn].DevRegList[i_start]);
	}

	return 1;
}

void CSerialPPIMaster::CommError(int devsn,int rulesn)
{//应答报文错误时，将rulesn序号的规则下面的所有寄存器都置为通信错误
	int i_start=DevQueryList[devsn].QueryRuleList[rulesn].RegStartSn;
	int i_endpos = DevQueryList[devsn].QueryRuleList[rulesn].RegStopSn;
	for(;i_start <=i_endpos;i_start++)
	{
		DevRegList[devsn].DevRegList[i_start].CommState = 0;
		ParaFunc.WriteDevRegData(DevRegList[devsn].DevRegList[i_start]);
	}
}

int CSerialPPIMaster::GetBit(unsigned char *DataBuf,int DataPlace,int BitSn)
{
	return ((DataBuf[DataPlace]>>BitSn)&0x01)?1:0;
}

int CSerialPPIMaster::InitRule(SProtocolInit &pf)
{
	//成员初始化
	//复制超时
	this->ParaFunc=pf;
	if(this->ParaFunc.AutoTimeout)
	{//自动处理超时
		this->ParaFunc.MinTimeout=100;
		this->ParaFunc.MinTimeout=400;
		this->ParaFunc.MaxTimeout=1000;
	}
	//成员类初始化
	m_SerialPPIPduClass.InitCSerialPPIPduClass(&ParaFunc);
	//读设备和寄存器列表，返回的是设备个数
	DevNum = ParaFunc.GetDevAddrRegList(ParaFunc.PortId, DevRegList);
	if(DevNum == 0)
		return -1;


	//生成和设备相等的规则
	DevQueryList = new SDevQueryNode[DevNum];
	memset(DevQueryList,0,DevNum*sizeof(SDevQueryNode));

	if(DevQueryList == NULL)
	{
		PrintLog(LOGERROR,"分配设备查询规则结点失败，不能采集任何量");
		return 0;
	}

	//挨个设备扫描轮循规则
	for(int i=0;i<DevNum;i++)
	{
		if(DevRegList[i].DevRegNum == 0)
		{
			PrintLog(LOGWARNING,"在设备%s下没有任何寄存器，不能采集该设备的信息",DevRegList[i].DevName);
			continue;
		}
		//拷贝设备信息 
		DevQueryList[i].DevAddr = DevRegList[i].DevAddr;
		//先预扫描一遍，得到规则数目
		DevQueryList[i].QueryRuleList = new SQueryRuleNode[DevRegList[i].DevRegNum];
		if(DevQueryList[i].QueryRuleList == NULL)
		{
			PrintLog(LOGWARNING,"设备%s在分配采集空间时失败，不能采集设备信息",DevRegList[i].DevName);
			continue;
		}
		int rulesn = -1;
		int specialrulesn = 0;
		//遍历设备下的寄存器
		for(int j=0;j<DevRegList[i].DevRegNum;j++)
		{
			if(DevRegList[i].DevRegList[j].RegRWMode == REG_WRITE_ONLY)
				continue;//只写寄存器不记入规则中
			if(DevRegList[i].DevRegList[j].RegType == SPECIALREGTYPE)
			{//特殊规则
				specialrulesn ++;
				continue;
			}

			if(rulesn == -1||DevQueryList[i].QueryRuleList[rulesn].FunCode != RegType2FunCode(DevRegList[i].DevRegList[j].RegType)
				|| DevRegList[i].DevRegList[j].RegAddr/10 - DevQueryList[i].QueryRuleList[rulesn].RegStartAddr/10 + DataType2RegCount(DevRegList[i].DevRegList[j].RegDataType) <= MAXREADLEN)
			{
				rulesn ++;

				DevQueryList[i].QueryRuleList[rulesn].FunCode = RegType2FunCode(DevRegList[i].DevRegList[j].RegType);
				DevQueryList[i].QueryRuleList[rulesn].RegStartAddr = DevRegList[i].DevRegList[j].RegAddr;
				DevQueryList[i].QueryRuleList[rulesn].RegCount = DataType2RegCount(DevRegList[i].DevRegList[j].RegDataType);
			}
			else
			{
				int regcount = DevRegList[i].DevRegList[j].RegAddr/10 - DevQueryList[i].QueryRuleList[rulesn].RegStartAddr/10 + DataType2RegCount(DevRegList[i].DevRegList[j].RegDataType);
				DevQueryList[i].QueryRuleList[rulesn].RegCount = DevQueryList[i].QueryRuleList[rulesn].RegCount >=regcount?DevQueryList[i].QueryRuleList[rulesn].RegCount:regcount;
			}
		}

		//重新生成规则空间
		DevQueryList[i].QueryRuleNum = rulesn + 1;
		if(DevQueryList[i].QueryRuleNum != 0 && DevQueryList[i].QueryRuleNum != DevRegList[i].DevRegNum)
		{
			delete [] DevQueryList[i].QueryRuleList;
			DevQueryList[i].QueryRuleList = new SQueryRuleNode[DevQueryList[i].QueryRuleNum];
		}

		if(DevQueryList[i].QueryRuleList == NULL)
		{
			PrintLog(LOGWARNING,"设备%s在重分配采集空间时失败，不能采集设备信息",DevRegList[i].DevName);
			continue;
		}

		//正式生成规则
		rulesn = -1;
		//遍历设备下的寄存器
		for(int j=0;j<DevRegList[i].DevRegNum;j++)
		{
			if(DevRegList[i].DevRegList[j].RegRWMode == REG_WRITE_ONLY || DevRegList[i].DevRegList[j].RegType == SPECIALREGTYPE)
				continue;//只写寄存器不记入规则中

			if(rulesn == -1||DevQueryList[i].QueryRuleList[rulesn].FunCode != RegType2FunCode(DevRegList[i].DevRegList[j].RegType)
				|| DevRegList[i].DevRegList[j].RegAddr/10 - DevQueryList[i].QueryRuleList[rulesn].RegStartAddr/10 + DataType2RegCount(DevRegList[i].DevRegList[j].RegDataType) <= MAXREADLEN)
			{
				rulesn ++;

				DevQueryList[i].QueryRuleList[rulesn].FunCode = RegType2FunCode(DevRegList[i].DevRegList[j].RegType);
				DevQueryList[i].QueryRuleList[rulesn].RegStartAddr = DevRegList[i].DevRegList[j].RegAddr;
				DevQueryList[i].QueryRuleList[rulesn].RegCount = DataType2RegCount(DevRegList[i].DevRegList[j].RegDataType);
				DevQueryList[i].QueryRuleList[rulesn].PagingState = 0;

				DevQueryList[i].QueryRuleList[rulesn].RegStartSn = j;
				DevQueryList[i].QueryRuleList[rulesn].RegStopSn  = j;

				DevQueryList[i].QueryRuleList[rulesn].DataUpdateCycle = DevRegList[i].DevRegList[j].DataUpdateCycle;
				DevQueryList[i].QueryRuleList[rulesn].LastingTimeAfterSample = 0;
				DevQueryList[i].QueryRuleList[rulesn].SampleErrTryCount = 0;
			}
			else
			{
				DevQueryList[i].QueryRuleList[rulesn].RegStopSn ++;
				//修改寄存器计数
				int regcount = DevRegList[i].DevRegList[j].RegAddr/10 - DevQueryList[i].QueryRuleList[rulesn].RegStartAddr/10 + DataType2RegCount(DevRegList[i].DevRegList[j].RegDataType);
				if(DevQueryList[i].QueryRuleList[rulesn].RegCount < regcount)
					DevQueryList[i].QueryRuleList[rulesn].RegCount = regcount;
				//修改数据周期
				if(DevRegList[i].DevRegList[j].DataUpdateCycle != -1)
				{
					if(DevQueryList[i].QueryRuleList[rulesn].DataUpdateCycle < DevRegList[i].DevRegList[j].DataUpdateCycle)
						DevQueryList[i].QueryRuleList[rulesn].DataUpdateCycle = DevRegList[i].DevRegList[j].DataUpdateCycle;
				}
			}
		}

		//生成紧急规则
		DevQueryList[i].UrgentRuleSnList = new int[DevQueryList[i].QueryRuleNum];
		for(int j = 0;j<DevQueryList[i].QueryRuleNum;j++)
		{//
			if(DevQueryList[i].QueryRuleList[j].DataUpdateCycle != -1)
			{
				DevQueryList[i].UrgentRuleSnList[DevQueryList[i].UrgentRuleNum] = j;
				DevQueryList[i].UrgentRuleNum ++;

				//转换计时时间 
				DevQueryList[i].QueryRuleList[j].DataUpdateCycle = (DevQueryList[i].QueryRuleList[j].DataUpdateCycle + MIN_TIME_RESOLUTION - 1)/MIN_TIME_RESOLUTION;
				//注册计时器
				ParaFunc.RegSoftTimer(DevQueryList[i].QueryRuleList[j].SampleErrTryCount);
			}
		}


		//生成特殊规则
		if(specialrulesn + 1 > 0)
		{
			DevQueryList[i].SpecialRuleList = new SQueryRuleNode[specialrulesn + 1];
			for(int j=0;j<DevRegList[i].DevRegNum;j++)
			{
				if(DevRegList[i].DevRegList[j].RegType == SPECIALREGTYPE)
				{

					DevQueryList[i].SpecialRuleList[DevQueryList[i].SpecialRuleNum].FunCode = RegType2FunCode(DevRegList[i].DevRegList[j].RegType);
					DevQueryList[i].SpecialRuleList[DevQueryList[i].SpecialRuleNum].RegStartAddr = DevRegList[i].DevRegList[j].RegAddr;

					DevQueryList[i].SpecialRuleList[DevQueryList[i].SpecialRuleNum].RegStartSn = j;
					DevQueryList[i].SpecialRuleList[DevQueryList[i].SpecialRuleNum].RegStopSn  = j;

					DevQueryList[i].SpecialRuleNum ++;
				}		
			}
		}

		//打印各种规则的条数
		PrintLog(LOGTIP,"在设备:(%s)上有%d条普通规则",DevRegList[i].DevName,DevQueryList[i].QueryRuleNum);
		PrintLog(LOGTIP,"在设备:(%s)上有%d条紧急规则",DevRegList[i].DevName,DevQueryList[i].UrgentRuleNum);
		PrintLog(LOGTIP,"在设备:(%s)上有%d条特殊规则",DevRegList[i].DevName,DevQueryList[i].SpecialRuleNum);

	}

	//初始化设备通信状态
	for(int i=0;i<DevNum;i++)
		this->SetDevCommState(i,1);


	return 1;
}

int CSerialPPIMaster::StartWork()
{//启动服务
	//打开串口
	if(ParaFunc.OpenCom(ParaFunc.PortId) == 0)
	{//打开硬串口失败，退出采集
		PrintLog(LOGERROR,"打开串口%d失败，结束采集",ParaFunc.PortId);
		if(DevQueryList != NULL)
		{
			//将所有的规则均置为错误状态
			for(int i=0;i<DevNum;i++)
			{
				this->SetDevCommState(i,0);
				for(int j=0;j<DevQueryList[i].QueryRuleNum;j++)
					this->CommError(i,j);
			}
		}
	}
	else
	{
		while(!ParaFunc.NeedQuit(ParaFunc.PortId) && !this->NeedColdStandbyStop())
		{
			for(int i=0;i<DevNum;i++)
			{
				if(ParaFunc.NeedQuit(ParaFunc.PortId) || this->NeedColdStandbyStop())
					break;

				//判断是否有遥控遥调，遥控遥调报文优先级高，插入轮询队列处理
				if(ParaFunc.HasOutput(ParaFunc.PortId,0))
				{
					SOutputRecord ror;
					PrintLog(LOGTIP,"CSerialPPIMaster Have Value To OutPut On Port,PortId=%d",ParaFunc.PortId);
					int place = ParaFunc.ReadOutput(ParaFunc.PortId,0,ror);//读遥控记录
					int returnvalue = OutPut(ror);//输出遥控记录
					if(ror.RegDataType == DEVREGSTRING)
						ParaFunc.FreeMString(ror.Value.str);
					if(returnvalue)//判断返回报文
					{
						PrintLog(LOGTIP,"CSerialPPIMaster OutPut Value On Port Success,PortId=%d",ParaFunc.PortId);
						ParaFunc.DelOutput(ParaFunc.PortId, place, true);//成功
					}
					else
					{
						PrintLog(LOGWARNING,"CSerialPPIMaster OutPut Value On Port Failed,PortId=%d",ParaFunc.PortId);
						ParaFunc.DelOutput(ParaFunc.PortId, place, false);//失败
					}
				}

				if(GetRuleNum(i))
				{
					//扫描所有规则里的DataUpdateCycle和LastingTimeAfterSample，看看有无紧急任务
					int urgentrulesn;
					if((urgentrulesn = FindUrgentRule(i)) != -1)
					{
						Query(i,urgentrulesn);
					}

					//正常轮询
					if(DevQueryList[i].PollingSn != urgentrulesn)
					{
						Query(i,DevQueryList[i].PollingSn);
					}

					DevQueryList[i].PollingSn = (DevQueryList[i].PollingSn + 1) % GetRuleNum(i);
				}
			}

			Sleep(ParaFunc.MinTimeout);
		}
		//关闭串口
		ParaFunc.CloseCom(ParaFunc.PortId);
	}

	return 1;
}

int CSerialPPIMaster::Query(int devsn,int rulesn)
{// 根据规则构建查询报文，并发送，返回期望的应答报文长度
	if(!DevQueryList[devsn].QueryRuleList[rulesn].PagingState)
	{
		m_SerialPPIPduClass.SendPagingData(DevQueryList[devsn].DevAddr);
		if(m_SerialPPIPduClass.RecPagingData(DevQueryList[devsn].DevAddr))
		{//成功
			DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount=0;
			DevQueryList[devsn].QueryRuleList[rulesn].PagingState=1;
			return 1;
		}
		else
		{//失败
			DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount++;
			if(DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount>=10&&DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount<=13)
				this->CommError(devsn,rulesn);

			return 0;
		}
	}

	unsigned char recbuf[RECBUFLEN];
	int recbuflen=RECBUFLEN;
	unsigned char *databuf=NULL;
	int databuflen=0;

	//下发寻呼指令成功，或已经下发过寻呼指令
	//1下发读指令
	//2接收读指令返回
	//3下发执行指令
	//4接收执行指令返回
	int readlen=0,readcount=0;
	this->DataTypeAndRegCount2ReadLenAndReadNum(DevQueryList[devsn].QueryRuleList[rulesn].DataType,DevQueryList[devsn].QueryRuleList[rulesn].RegCount,readlen,readcount);
	if(m_SerialPPIPduClass.SendReadData(DevQueryList[devsn].DevAddr,readlen,readcount,DevQueryList[devsn].QueryRuleList[rulesn].FunCode,RegAddr2Offset(DevQueryList[devsn].QueryRuleList[rulesn].RegStartAddr)))
	{//发送命令成功
		if(m_SerialPPIPduClass.RecReadConfirmData())
		{//接收到确认数据
			if(m_SerialPPIPduClass.SendConfirmData(DevQueryList[devsn].DevAddr))
			{//发送确认数据
				if(m_SerialPPIPduClass.ResponseAnalysis(DevQueryList[devsn].DevAddr,recbuf,recbuflen,databuf,databuflen))
				{//接收到数据
					DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount=0;
					//派发数据
					this->ResponseAnalysis(devsn,rulesn,databuf,databuflen);
					return 1;
				}
				else
				{
					DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount++;
				}
			}
			else
			{
				DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount++;
			}
		}
		else
		{
			DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount++;
		}
	}
	else
	{
		DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount++;
	}

	if(DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount>=10&&DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount<=15)
	{
		this->CommError(devsn,rulesn);
	}

	return 0;
}

int CSerialPPIMaster::OutPut(SOutputRecord &ror)
{//构造遥控遥调报文,并发送，返回期望的应答报文长度

	switch(ror.RegDataType)
	{
	case DEVREGUSHORT:
		return m_SerialPPIPduClass.WriteUShortData(ror.DevAddr,ror.RegType,RegAddr2WriteOffset(ror.RegAddr),ror.Value.us);
		break;
	case DEVREGSHORT:
		return m_SerialPPIPduClass.WriteShortData(ror.DevAddr,ror.RegType,RegAddr2WriteOffset(ror.RegAddr),ror.Value.s);
		break;
	case DEVREGUINT:
		return m_SerialPPIPduClass.WriteUIntData(ror.DevAddr,ror.RegType,RegAddr2WriteOffset(ror.RegAddr),ror.Value.ui);
		break;
	case DEVREGINT:
		return m_SerialPPIPduClass.WriteUIntData(ror.DevAddr,ror.RegType,RegAddr2WriteOffset(ror.RegAddr),ror.Value.i);
		break;
	case DEVREGFLOAT:
		return m_SerialPPIPduClass.WriteFloatData(ror.DevAddr,ror.RegType,RegAddr2WriteOffset(ror.RegAddr),ror.Value.f);
		break;
	case DEVREGCHAR:
		return m_SerialPPIPduClass.WriteCharData(ror.DevAddr,ror.RegType,RegAddr2WriteOffset(ror.RegAddr),ror.Value.c);
		break;
	case DEVREGUCHAR:
		return m_SerialPPIPduClass.WriteCharData(ror.DevAddr,ror.RegType,RegAddr2WriteOffset(ror.RegAddr),ror.Value.uc);
		break;
	case DEVREGBIT:
		return m_SerialPPIPduClass.WriteBitData(ror.DevAddr,ror.RegType,RegAddr2WriteOffset(ror.RegAddr),ror.Value.b);
		break;
	case DEVREGBITSTRING:
		return m_SerialPPIPduClass.WriteShortData(ror.DevAddr,ror.RegType,RegAddr2WriteOffset(ror.RegAddr),ror.Value.s);
		break;
	case DEVREGSTRING:
		return m_SerialPPIPduClass.WriteStringData(ror.DevAddr,ror.RegType,RegAddr2WriteOffset(ror.RegAddr),ror.Value.str);
		break;
	}

	return 0;
}
