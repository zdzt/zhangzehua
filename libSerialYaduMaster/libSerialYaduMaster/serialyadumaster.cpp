//****************************************************************************************
//此规约为标准MODBUS                                                                     *
//只支持的数据类型为：ushort,short,uint,int,float,bit,bitstr(按位读比特串)               *
//****************************************************************************************
//此规约只支持1,2,3,4,5,6,16功能码                                                       *
//    1,2,3,4是输入，5,6,16是输出                                                        *
//    1,2读bit                                                                           *
//    3,4读ushort,short,uint,int,float,bitstr(按位读比特串)                              *
//    5写bit                                                                             *
//    6写ushort,short                                                                    *
//    16写连续两个寄存器的uint,int,float                                                 *
//****************************************************************************************
#include "stdafx.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#include "serialyadumaster.h"

#define COMMNUNICATERETRY 10
#define COMMNUNICATEMAXRETRY 12

CSerialYaduMaster::CSerialYaduMaster()
{//构造函数
	DevRegList = NULL;
	DevNum = 0;

	DevQueryList = NULL;
}

CSerialYaduMaster::~CSerialYaduMaster()
{//析构函数
	//释放查询结点
	this->FreeDevQueryList();
	//释放DevRegList
	this->ParaFunc.FreeDevAddrRegList(DevRegList,DevNum);
}

//释放查询规则列表
int CSerialYaduMaster::FreeDevQueryList()
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

//此规约只支持1,2,3,4读功能码
int CSerialYaduMaster::RegType2FunCode(int rt)
{
	return rt & 0xff;//取最低字节的读功能码
}

int CSerialYaduMaster::SetDevCommState(int devsn,int commstate)
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

int CSerialYaduMaster::ChangeDevCommState(int devsn,int commstate)
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


int CSerialYaduMaster::GetRuleNum(int devsn)
{
	return DevQueryList[devsn].QueryRuleNum;
}

//寻找到迫切需要采集的规则位置
int CSerialYaduMaster::FindUrgentRule(int devsn)
{
	for(int i = 0; i < DevQueryList[devsn].UrgentRuleNum; i++)
	{
		if(DevQueryList[devsn].QueryRuleList[DevQueryList[devsn].UrgentRuleSnList[i]].DataUpdateCycle < DevQueryList[devsn].QueryRuleList[DevQueryList[devsn].UrgentRuleSnList[i]].LastingTimeAfterSample)
			return DevQueryList[devsn].UrgentRuleSnList[i];//距离上次采集过去的时间已经超过数据更新周期了，返回规则序号，表示该规则中的寄存器数据需要紧急更新
	}

	return -1;
}


//Query函数
//入口：规则序号
//返回：期望的应答报文长度
int CSerialYaduMaster::Query(int devsn,int rulesn)
{
	unsigned char txdbuf[32];//串口发送缓冲区
	int len = 0;
	int addr = 0;
	addr = DevQueryList[devsn].DevAddr;
	int funcode = DevQueryList[devsn].QueryRuleList[rulesn].FunCode;
	int wantlen = 0;
	switch(funcode)
	{
	case 1://0x01 查询系统数据
		txdbuf[len++] = 0x5A;//
		txdbuf[len++] = 0x06;//
		txdbuf[len++] = addr;
		txdbuf[len++] = 0x41;//
		txdbuf[len++] = (0x5A+0x06+addr+0x41)&0xff;//
		txdbuf[len++] = 0xA5;///
		txdbuf[len] = '\0';
		wantlen = 11;
		break;
	default:break;
	}
	ParaFunc.WriteCom(ParaFunc.PortId, (char *)txdbuf, len);//写query

	return wantlen;
}

//Response函数
//判断应答报文是否正确，如果正确，则写寄存器，否则置通信错误
//入口：规则序号，期望的应答报文长度
//返回：成功 or 失败
int CSerialYaduMaster::Response(int devsn,int rulesn, int wantretlen)//校验报文
{
	unsigned char rxdbuf[256];//串口接收缓冲区

	int reclen = ParaFunc.ReadCom(ParaFunc.PortId, (char *)rxdbuf, wantretlen, ParaFunc.MidTimeout);//普通的采集信息，一般用最小延时
	rxdbuf[reclen]='\0';

	int state=0;
	int i_pos=0;
	int startpos=0;
	int i_sum=0;
	int i_jiaoyan=0;
	int datalen=0;

	while(i_pos<reclen)
	{
		switch(state)
		{
		case 0://查找头
			if(rxdbuf[i_pos]==0x5A)
			{//查找到头
				startpos=i_pos;
				state=1;
			}
			else
			{
				i_pos++;
			}

			break;
		case 1://
			if((reclen-startpos)<wantretlen)
			{
				reclen+=ParaFunc.ReadCom(ParaFunc.PortId, (char *)(rxdbuf+reclen), wantretlen-(reclen-startpos), ParaFunc.MidTimeout);

				if(reclen - startpos>=wantretlen)
				{
					state=2;
				}
				else
				{
					state=4;
				}
			}
			else
			{
				state=2;
			}

			break;
		case 2://jiaoyan
			datalen=rxdbuf[startpos+1];
			i_jiaoyan=rxdbuf[startpos+9];
			for (int i = 0; i<(datalen-2);i++)
			{  
				i_sum=i_sum+rxdbuf[startpos+i];
			}
			i_sum=i_sum&0xFF;
			if (i_sum=i_jiaoyan)
			{
				state=3;
			}
			else
			{
				state=4;
			}
			break;
		case 3://结尾0xA5
			if(rxdbuf[startpos+10]==0xA5)
			{
				goto success;
			}
			else
			{
				goto fail;
			}
			break;
		case 4:
			goto fail;
			break;
		}
	}

fail:	
	if(DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount == 0)
		this->ChangeDevCommState(devsn,0);

	if(DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount >= COMMNUNICATERETRY && DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount <= COMMNUNICATEMAXRETRY)//连续3次以上通信错误
		CommError(devsn,rulesn);//将该规则里的寄存器通信状态设置为错误，保险起见，写3次
	DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount++;
	return false;
success:
	if(DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount != 0)
		this->ChangeDevCommState(devsn,1);
	DevQueryList[devsn].QueryRuleList[rulesn].LastingTimeAfterSample = 0;
	DevQueryList[devsn].QueryRuleList[rulesn].SampleErrTryCount = 0;
	return ResponseAnalysis(devsn,rulesn, rxdbuf+startpos+4,5 );	//成功
}

//ResponseAnalysis函数
//入口：规则序号，应答报文的数据区（去掉了MODBUS报文头和CRC部分）和数据区的长度
//返回：成功与否，一般如果数据区长度不够，会return rt & 0xff;//取最低字节的读功能码返回错
int CSerialYaduMaster::ResponseAnalysis(int devsn,int rulesn, unsigned char *data, int datalen)
{
	int regstartsn = DevQueryList[devsn].QueryRuleList[rulesn].RegStartSn;//寄存器开始序号
	int regstopsn =  DevQueryList[devsn].QueryRuleList[rulesn].RegStopSn;//寄存器结束序号


	for(int i = regstartsn; i <= regstopsn; i++)
	{
		int regaddr = DevRegList[devsn].DevRegList[i].RegAddr;
		switch(DevRegList[devsn].DevRegList[i].RegDataType)//根据数据类型判断
		{
		case DEVREGCHAR:

			DevRegList[devsn].DevRegList[i].CommState=1;
			DevRegList[devsn].DevRegList[i].Value.c = data[regaddr];
			ParaFunc.WriteDevRegData(DevRegList[devsn].DevRegList[i]);
			break;
		case DEVREGUSHORT:
			DevRegList[devsn].DevRegList[i].CommState=1;
			DevRegList[devsn].DevRegList[i].Value.us = data[regaddr+1]*256+data[regaddr];
			ParaFunc.WriteDevRegData(DevRegList[devsn].DevRegList[i]);
			break;
			break;
		default:
			break;
		}
	}
	return true;
}

//CommError函数
//将rulesn序号的规则下面的所有寄存器都置为通信错误
void CSerialYaduMaster::CommError(int devsn,int rulesn)
{
	int regstartsn = DevQueryList[devsn].QueryRuleList[rulesn].RegStartSn;
	int regstopsn =  DevQueryList[devsn].QueryRuleList[rulesn].RegStopSn;

	for(int i = regstartsn; i <= regstopsn; i++)
	{
		DevRegList[devsn].DevRegList[i].CommState = 0;//通信错误
		ParaFunc.WriteDevRegData(DevRegList[devsn].DevRegList[i]);
	}
}

int CSerialYaduMaster::GetRegValue(unsigned char *Data,int DataLen,int RegAddr,unsigned char *ReturnBuf,int &ReturnValue)
{
	if(Data==NULL||ReturnBuf==NULL)
	{
		ReturnValue=0;
	}

	int state=0;
	int i_pos=0;
	int i_regaddr=0;
	ReturnValue=0;
	while(i_pos<DataLen)
	{
		switch(state)
		{
		case 0://找头
			if(RegAddr==0)
			{
				if(Data[i_pos]>='0'&&Data[i_pos]<='9')
				{
					state=1;
				}
				else
				{
					i_pos++;
				}
			}
			else
			{
				if(Data[i_pos]==0x20)//空格
				{
					i_regaddr++;
					i_pos++;
					if(Data[i_pos] != 0x20)
					{
						if(RegAddr == i_regaddr)
						{
							state=1;
						}
					}
					else
					{
						i_regaddr--;
					}
				}
				else
				{
					i_pos++;
				}
			}
			break;
		case 1://拷贝数据
			if(Data[i_pos]!=0x20&&Data[i_pos]!=0x0d)
			{
				ReturnBuf[ReturnValue++]=Data[i_pos++];
			}
			else
			{
				state=2;
			}
			break;
		case 2:
			goto success;
			break;
		}
	}
success:
	ReturnBuf[ReturnValue]='\0';

	return 1;
}

float CSerialYaduMaster::GetRegFloatValue(unsigned char *Data,int DataLen,int RegAddr)
{
	unsigned char recbuf[32];
	int  recbuflen=32;
	float returnvalue=0;
	if(GetRegValue(Data,DataLen,RegAddr,recbuf,recbuflen))
	{
		recbuf[recbuflen]='\0';
		returnvalue=(float)atof((char *)recbuf);
	}
	else
	{
		returnvalue=0;
	}

	return returnvalue;
}

unsigned short CSerialYaduMaster::GetRegBitValue(unsigned char *Data,int DataLen,int RegAddr)
{
	unsigned char recbuf[32];
	int  recbuflen=32;
	unsigned short returnvalue=0;
	if(GetRegValue(Data,DataLen,RegAddr,recbuf,recbuflen))
	{
		recbuflen=recbuflen>8?8:recbuflen;

		for(int i=0;i<recbuflen;i++)
		{
			returnvalue|=((recbuf[i]-'0')<<i);
		}

		returnvalue<<=8;
	}
	else
	{
		returnvalue=0;
	}

	return returnvalue;
}

int CSerialYaduMaster::InitRule(SInterfaceComMaster &pf)
{//此函数规约只调用一次，效率可以不考虑
	//保存入口参数和函数指针
	ParaFunc = pf;
	if(ParaFunc.AutoTimeout)
	{//规约自动选择超时，由于考虑到网络转串口设备，SPI模拟串口设备，通过波特率计算超时不准确，所以在此预估一个超时值
		ParaFunc.MinTimeout = 200;
		ParaFunc.MidTimeout = 2000;
		ParaFunc.MaxTimeout = 8000;
	}

	//读设备和寄存器列表，返回的是设备个数
	DevNum = ParaFunc.GetDevAddrRegList(ParaFunc.PortId, DevRegList);
	if(DevNum == 0)
	{
		ParaFunc.PrintLog("CSerialYaduMaster, no device found under port %d, task terminated!", ParaFunc.PortId);
		return -1;
	}

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
		DevQueryList[i].IsFileterData = 1;
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

			if(rulesn == -1||DevQueryList[i].QueryRuleList[rulesn].FunCode != RegType2FunCode(DevRegList[i].DevRegList[j].RegType))
			{
				rulesn ++;
				DevQueryList[i].QueryRuleList[rulesn].FunCode = RegType2FunCode(DevRegList[i].DevRegList[j].RegType);
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

			if(rulesn == -1||DevQueryList[i].QueryRuleList[rulesn].FunCode != RegType2FunCode(DevRegList[i].DevRegList[j].RegType))
			{
				rulesn ++;

				DevQueryList[i].QueryRuleList[rulesn].FunCode = RegType2FunCode(DevRegList[i].DevRegList[j].RegType);
				DevQueryList[i].QueryRuleList[rulesn].RegStartAddr = DevRegList[i].DevRegList[j].RegAddr;

				DevQueryList[i].QueryRuleList[rulesn].RegStartSn = j;
				DevQueryList[i].QueryRuleList[rulesn].RegStopSn  = j;

				DevQueryList[i].QueryRuleList[rulesn].DataUpdateCycle = DevRegList[i].DevRegList[j].DataUpdateCycle;
				DevQueryList[i].QueryRuleList[rulesn].LastingTimeAfterSample = 0;
				DevQueryList[i].QueryRuleList[rulesn].SampleErrTryCount = 0;
			}
			else
			{
				DevQueryList[i].QueryRuleList[rulesn].RegStopSn ++;
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

int CSerialYaduMaster::StartWork()
{
	ParaFunc.PrintLog("YaduMasterThread, runing on port %d", ParaFunc.PortId);

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

		while(!ParaFunc.NeedQuit(ParaFunc.PortId))
		{
			for(int i=0;i<DevNum;i++)
			{
				if(ParaFunc.NeedQuit(ParaFunc.PortId))
					break;

				//判断是否有遥控遥调，遥控遥调报文优先级高，插入轮询队列处理
				if(ParaFunc.HasOutput(ParaFunc.PortId,0))
				{
					SOutputRecord ror;
					int place = ParaFunc.ReadOutput(ParaFunc.PortId,0,ror);//读遥控记录
					ParaFunc.DelOutput(ParaFunc.PortId, place, false);//失败
				}

				if(GetRuleNum(i))
				{
					//扫描所有规则里的DataUpdateCycle和LastingTimeAfterSample，看看有无紧急任务
					int urgentrulesn;
					if((urgentrulesn = FindUrgentRule(i)) != -1)
					{
						int wantretlen = Query(i,urgentrulesn);
						Response(i,urgentrulesn, wantretlen);
					}
					//正常轮询
					if(DevQueryList[i].PollingSn != urgentrulesn)
					{
						int wantretlen = Query(i,DevQueryList[i].PollingSn);
						Response(i,DevQueryList[i].PollingSn, wantretlen);
					}
					DevQueryList[i].PollingSn = (DevQueryList[i].PollingSn + 1) % GetRuleNum(i);
				}
			}
			Sleep(ParaFunc.MinTimeout);
		}
		ParaFunc.CloseCom(ParaFunc.PortId);
	}
	ParaFunc.PrintLog("*YaduMasterThread, abnormal terminated on port %d!", ParaFunc.PortId);

	return NULL;

}
