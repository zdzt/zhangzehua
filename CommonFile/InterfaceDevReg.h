#pragma once
#include "Constant.h"
#include "PubStruct.h"

struct SRegAddData//寄存器附加数据
{
	int	 DataType;//附加数据类型 UINT 3   INT 4   FLOAT 5  CHAR* 9  （定义和RegDataType相同）
	int	 DataNum;//附加数据数量，代表RegDescription 字符串长度，UIntData,IntData,FloatData数组中数据个数
	union{
		char*	Description;//寄存器描述信息字符串
		UINT* 	UIntData;
		int*	IntData;
		float*	FloatData;
	};
};

struct SOutputRecord
{
	int	 RegId;//寄存器唯一ID，系统自动产生
	int	 VarId;//变量ID
	int	 DevId;//设备ID
	int	 DevAddr;//设备站址
	int	 WaitTime;//毫秒

	int	 RegType;//寄存器类型
	int	 RegAddr;//寄存器地址
	int	 RegDataType;//设备库数据类型1~10
	int	 RegExtDataType;//数据类型扩展类型
	SRegAddData*	RegAddData;//寄存器附加数据

	union
	{
		USHORT us;
		short s;
		UINT ui;
		int i;
		float f;
		char c;
		unsigned char uc;
		int b;
		MString str;//字符串
	}Value;
};

#pragma pack(push, 4)
union SDevRegValue
{
	USHORT us;//设备库数据类型1
	short s;//设备库数据类型2
	UINT ui;//设备库数据类型3
	int i;//设备库数据类型4
	float f;//设备库数据类型5
	char c;//设备库数据类型6
	unsigned char uc;//设备库数据类型7
	int b;//设备库数据类型8,bit型
	MString str;//设备库数据类型9,字符串类型
	USHORT bitStr;//设备库数据类型10,比特串，包含16个BIT
	USHORT bitField;//设备库数据类型11,位域
	UINT bitField2;//设备库数据类型12,位域
};//寄存器数据值
#pragma pack (pop)

struct SDevRegData
{
	int	 RegId;//寄存器唯一ID，系统自动产生
	int	 VarId;//变量ID
	int	 RegType;//寄存器类型
	int	 RegAddr;//寄存器地址
	int	 RegDataType;//设备库数据类型1~10
	int	 RegExtDataType;//数据类型扩展类型
	int	 RegRWMode;//寄存器读写模式：1读，2写，3读写
	SRegAddData*	RegAddData;
	int	 DataUpdateCycle;//数据更新周期，该变量必须在DataUpdateCycle毫秒内完成采集
	//用于某些优先级高的数据采集，值为-1时表示该参数无效，默认值为-1
	//寄存器数据区
	int 	CommState;//通信状态,-1:未采集 0:通信错误 1:通信正常
	SDevRegValue Value;//数据值
	//信息挂载点
	union{
		void*	ptr;
		UINT	ui;
		int	 i;
		float	f;
	}Mount;//此区域给规约使用，可以在此处挂载任何和该寄存器相关的信息，方便规约编程
};

struct SDevAddrRegInfo//设备寄存器信息列表
{
	int	 DevId;//设备ID，在同一端口下的设备，DevId从1开始连续递增编号
	int	 DevAddr;//设备站址，串口规约靠站址识别不同设备，网络型规约靠DevId识别不同设备，因为网络型规约中可能存在相同的站址
	char*	 DevName;//设备名，只读
	char*	 DevDesc;//设备描述，只读

	ULONG	 NetIp;//设备IP，网络型规约使用，保存的是通过inet_addr("192.168.1.10")函数转换出来的长整数
	int	 NetPort;//设备IP端口, 0表示需要规约替换端口，-1表示端口无效，1~65535是正常端口，不运行规约替换
	int	 DevRegNum;//该设备包含的寄存器个数
	SDevRegData	*DevRegList;//该设备包含的“可读”或“可写”寄存器列表数组，内存在接口函数里分配
	union{
		void*	ptr;
		UINT	ui;
		int	 i;
		float	f;
	}Mount;//信息挂载点；此区域给规约使用，可以在此处挂载任何和该设备相关的信息，方便规约编程
};

struct SDevRegSoe
{
	int	 RegId;//寄存器唯一ID，系统自动产生
	int	 EventType;//0:经典SOE类型 1:uint 2:int 3:float 4:string  
	union
	{
		UINT ui;//EventType=1
		int i;//EventType=2
		struct//EventType=3
		{
			float f;
			int decPlace;//浮点数小数点位置
		}Float;
		MString str;//EventType=4
	}Value;
	time_t Time;//事件发生时间，秒
};

//extern int GetDevAddrRegList(int portId, SDevAddrRegInfo *(&addrRegList));//读端口下的所有设备信息和设备下的寄存器信息列表
//extern void FreeDevAddrRegList(SDevAddrRegInfo *addrRegList, int devNum);
//extern BOOL WriteDevRegData(SDevRegData &regData);//设置设备寄存器的值
//extern BOOL WriteDevRegSoe(SDevRegSoe &devRegSoe);//写设备寄存器事件
//extern BOOL SampleEnable();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
//extern BOOL HasOutput(int portId, int devId);//判断端口下是否有遥控
//extern int ReadOutput(int portId, int devId, SOutputRecord &ror);//从遥控队列中读一条遥控记录
//extern void FreeMString(MString str);//释放由ReadOutput读到的字符串（如果ReadOutput读到的不是字符串，不要调用该函数）
//extern BOOL	DelOutput(int portId, int place, BOOL sucess); //从遥控队列中删除遥控记录
//extern BOOL NeedQuit(int portId);//返回TRUE，表示SCADA提示规约要退出任务，规约释放所有内存后，退出线程
