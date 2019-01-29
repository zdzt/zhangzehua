#ifndef SERIALPPIMASTER_H
#define SERIALPPIMASTER_H

#define RULESTATUTENAME	"串口PPI主规约"

#include "ProtocolInit.h"
#include "SerialBaseGatherMaster.h"
#include "SerialPPIPduClass.h"

struct SQueryRuleNode//查询和应答规则结构体，此规则表完全是MODBUS适用，非通用规则，不适合其他规约，但可以类推
{
	//采集控制
	int IsStop;//是否不再采集
	//下面成员用于构建查询报文
	int FunCode;//存储器类型
	int RegStartAddr;//起始偏移量
	int RegCount;//偏移量个数

	int DataType;//数据类型

	int PagingState;//寻呼指令状态
	//下面成员用于报文应答时直接定位到DevRegList的具体位置
	int RegStartSn;//寄存器开始序号
	int RegStopSn;//寄存器结束序号

	//采集相关的成员
	int	DataUpdateCycle;//数据更新周期，该变量必须在DataUpdateCycle毫秒内完成采集，用于某些优先级高的数据采集，值为-1时表示该参数无效，默认值为-1
	int LastingTimeAfterSample;//采集后的持续时间
	int SampleErrTryCount;//采集数据错误重试次数
};

struct SDevQueryNode
{//设备查询结点
	//设备信息
	int DevAddr;
	//查询计数
	int ErrorQueryRuleCount;//错误查询规则计数
	int PollingSn;//轮循序号
	//查询规则
	int QueryRuleNum;
	SQueryRuleNode *QueryRuleList;
	//紧急规则
	int UrgentRuleNum;
	int *UrgentRuleSnList;
	//特殊规则
	int SpecialRuleNum;
	SQueryRuleNode * SpecialRuleList;
};

class CSerialPPIMaster:public CSerialBaseGatherMaster<SProtocolInit>
{
public:
	CSerialPPIMaster();
	~CSerialPPIMaster();
public:
	SInterfaceComMaster ParaFunc;//参数和函数指针
private:
	int DevNum;//该端口下挂的设备数目
	SDevAddrRegInfo *DevRegList;//该端口下的设备和寄存器列表
	SDevQueryNode *DevQueryList;//设备查询规则结点
private:
	int FreeDevQueryList();//释放查询结点
private:
	int DataType2RegCount(int dt);//从数据类型推出占用寄存器个数
	int DataTypeAndRegCount2ReadLenAndReadNum(int dt,int regcount,int &readlen,int &readcount);
	int RegType2FunCode(int rt);//从寄存器类型中取出功能码
	int RegAddr2Offset(int rd);
	int RegAddr2WriteOffset(int rd);

	int SetDevCommState(int devsn,int commstate);
	int ChangeDevCommState(int devsn,int commstate);

	int ResponseAnalysis(int devsn,int rulesn, unsigned char *data, int datalen);//应答报文正确时，分析数据区并写入寄存器，data指向的不包含报文头和CRC
	void CommError(int devsn,int rulesn);//应答报文错误时，将rulesn序号的规则下面的所有寄存器都置为通信错误

	int GetBit(unsigned char *DataBuf,int DataPlace,int BitSn);
private:
	CSerialPPIPduClass m_SerialPPIPduClass;
public:
	int GetRuleNum(int devsn);//返回规则数目
	int FindUrgentRule(int devsn);//寻找到迫切需要采集的规则位置

	int OutPut(SOutputRecord &ror);//构造遥控遥调报文,并发送，返回期望的应答报文长度

	int Query(int devsn,int rulesn);// 根据规则构建查询报文，并发送，返回期望的应答报文长度

	int InitRule(SProtocolInit &pf);
private:
	int StartWork();
};

#endif // SERIALPPIMASTER_H
