#pragma once

#include "Constant.h"
#include "PubStruct.h"
#include "InterfaceDevReg.h"
#include "InterfaceSlaveStruct.h"

class __declspec(novtable) ICallBackFunc//网络型规约回调函数接口
{
public:
	virtual void OnConnect(int para) = 0;
	virtual void OnDisconnect(int para) = 0;
	virtual void OnReceive(int para, char *buf, int len) = 0;
};

#define INTERFACE_NET_MASTER_TYPE		1
struct SInterfaceNetMaster
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int		(*GetDevNum)(int portId);
	void	(*WantConnectToDev)(int portId, int devId, char *ip, USHORT ipport);
	BOOL	(*IsDevConnected)(int portId, int devId);
	void	(*CloseConnectToDev)(int portId, int devId);
	BOOL	(*RegCallBackFuncOnDev)(int portId, int devId, ICallBackFunc *cbf, int cbp);
	int		(*SendToDev)(int portId, int devId, char *buf, int len);

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL 	(*WriteDevRegData)(SDevRegData &regData);
	BOOL	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_COM_MASTER_TYPE		2
struct SInterfaceComMaster
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int		(*OpenCom)(int portId);//返回0表示完全失败 1表示成功 -1表示可能失败
	int		(*ReadCom)(int portId, char *recBuf, int wantLen, int waitTime);
	int		(*WriteCom)(int portId, char *sendBuf, int sendLen);
	void	(*CloseCom)(int portId);

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL 	(*WriteDevRegData)(SDevRegData &regData);
	BOOL 	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_FREENET_MASTER_TYPE		3
struct SInterfaceFreeNetMaster
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	USHORT	LocalNetPort;//TCP服和UDP服的本地IP端口

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL	(*WriteDevRegData)(SDevRegData &regData);
	BOOL 	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_FREECOM_MASTER_TYPE		4
struct SInterfaceFreeComMaster
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	char	ComName[PORT_NAME_LEN];//串口名称，比如COM1，COM10
	int		BaudRate;
	int		DataBits;
	int		Parity;
	int		StopBits;

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL 	(*WriteDevRegData)(SDevRegData &regData);
	BOOL	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_COM_WITH_MYSQL_ADDR_TYPE		5
struct SInterfaceComWithMySqlAddr
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	char*	MySqlAddr;//MYSQL数据库地址，可以填IP,域名地址
	char*	MySqlUser;//MYSQL数据库用户名
	char*	MySqlPassWord;//MYSQL数据库密码
	char*	HistDbName;//MYSQL历史库库名

	char*	ConfigDbPathName;//SQLITE3配置库路径+文件名

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int		(*OpenCom)(int portId);//返回0表示完全失败 1表示成功 -1表示可能失败
	int		(*ReadCom)(int portId, char *recBuf, int wantLen, int waitTime);
	int		(*WriteCom)(int portId, char *sendBuf, int sendLen);
	void	(*CloseCom)(int portId);

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL 	(*WriteDevRegData)(SDevRegData &regData);
	BOOL 	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_NET_WITH_MYSQL_ADDR_TYPE		6
struct SInterfaceNetWithMySqlAddr
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	char*	MySqlAddr;//MYSQL数据库地址，可以填IP,域名地址
	char*	MySqlUser;//MYSQL数据库用户名
	char*	MySqlPassWord;//MYSQL数据库密码
	char*	HistDbName;//MYSQL历史库库名

	char*	ConfigDbPathName;//SQLITE3配置库路径+文件名

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int		(*GetDevNum)(int portId);
	void	(*WantConnectToDev)(int portId, int devId, char *ip, USHORT ipport);
	BOOL	(*IsDevConnected)(int portId, int devId);
	void	(*CloseConnectToDev)(int portId, int devId);
	BOOL	(*RegCallBackFuncOnDev)(int portId, int devId, ICallBackFunc *cbf, int cbp);
	int		(*SendToDev)(int portId, int devId, char *buf, int len);

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL 	(*WriteDevRegData)(SDevRegData &regData);
	BOOL	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_FREENET_WITH_MYSQL_ADDR_TYPE		7
struct SInterfaceFreeNetWithMySqlAddr
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	USHORT	LocalNetPort;//TCP服和UDP服的本地IP端口

	char*	MySqlAddr;//MYSQL数据库地址，可以填IP,域名地址
	char*	MySqlUser;//MYSQL数据库用户名
	char*	MySqlPassWord;//MYSQL数据库密码
	char*	HistDbName;//MYSQL历史库库名

	char*	ConfigDbPathName;//SQLITE3配置库路径+文件名

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL	(*WriteDevRegData)(SDevRegData &regData);
	BOOL 	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_CJK_MASTER_TYPE		8
struct SInterfaceCjkMaster
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL 	(*WriteDevRegData)(SDevRegData &regData);
	BOOL 	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_COM_WITH_SQLITE_ADDR_TYPE		9
struct SInterfaceComWithSqliteAddr
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	char*	ConfigDbPathName;//SQLITE3配置库路径+文件名
	char*	HistDbPath;//SQLITE3历史库路径

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int		(*OpenCom)(int portId);//返回0表示完全失败 1表示成功 -1表示可能失败
	int		(*ReadCom)(int portId, char *recBuf, int wantLen, int waitTime);
	int		(*WriteCom)(int portId, char *sendBuf, int sendLen);
	void	(*CloseCom)(int portId);

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL 	(*WriteDevRegData)(SDevRegData &regData);
	BOOL 	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_NET_WITH_SQLITE_ADDR_TYPE		10
struct SInterfaceNetWithSqliteAddr
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	char*	ConfigDbPathName;//SQLITE3配置库路径+文件名
	char*	HistDbPath;//SQLITE3历史库路径

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int		(*GetDevNum)(int portId);
	void	(*WantConnectToDev)(int portId, int devId, char *ip, USHORT ipport);
	BOOL	(*IsDevConnected)(int portId, int devId);
	void	(*CloseConnectToDev)(int portId, int devId);
	BOOL	(*RegCallBackFuncOnDev)(int portId, int devId, ICallBackFunc *cbf, int cbp);
	int		(*SendToDev)(int portId, int devId, char *buf, int len);

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL 	(*WriteDevRegData)(SDevRegData &regData);
	BOOL	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_FREENET_WITH_SQLITE_ADDR_TYPE		11
struct SInterfaceFreeNetWithSqliteAddr
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	USHORT	LocalNetPort;//TCP服和UDP服的本地IP端口

	char*	ConfigDbPathName;//SQLITE3配置库路径+文件名
	char*	HistDbPath;//SQLITE3历史库路径

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int 	(*GetDevAddrRegList)(int portId, SDevAddrRegInfo *(&addrRegList));//得到portId接口上的所有设备站址列表，以及每个站址下的寄存器列表
	void	(*FreeDevAddrRegList)(SDevAddrRegInfo *addrRegList, int devNum);

	BOOL	(*SampleEnable)();//允许采集，当返回FALSE时，规约不需要采集数据，用于双机冷备
	BOOL	(*WriteDevRegData)(SDevRegData &regData);
	BOOL 	(*WriteDevRegSoe)(SDevRegSoe &devRegSoe);

	BOOL	(*HasOutput)(int portId, int devId);//判断是否有输出记录
	int 	(*ReadOutput)(int portId, int devId, SOutputRecord &ror);//取一条输出记录，并返回记录位置
	void 	(*FreeMString)(MString str);//释放ReadOutput返回的str内存(如果ReadOutput读到的是字符串遥控)
	BOOL	(*DelOutput)(int portId, int place, BOOL sucess);//根据应答报文成功与否，设置place位置记录的标志，并修改输出记录读指针。sucess值：TRUE正确返回 FALSE错误返回或无返回超时到

	BOOL	(*NeedQuit)(int portId);//退出允许
};


#define INTERFACE_NET_SLAVE_TYPE		101
struct SInterfaceNetSlave
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		StationAddr;//本机站址
	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int		(*GetLinkNum)(int portId);
	void	(*WantConnect)(int portId, int linkNo);
	BOOL	(*IsLinkConnected)(int portId, int linkNo);
	void	(*CloseConnect)(int portId, int linkNo);
	BOOL	(*RegCallBackFuncOnPort)(int portId, ICallBackFunc *cbf);
	int		(*SendToLink)(int portId, int linkNo, char *buf, int len);

	int		(*GetAI)(int logRegAddr, int &dataType, LAMVarValue &value, int &commState);
	int		(*GetAIOType)(int logRegAddr, int &dataType, int &rwMode);
	int		(*SetAO)(int logRegAddr, int waitTime, ...);
	int		(*GetDI)(int logRegAddr, int &value, int &commState);
	int		(*SetDO)(int logRegAddr, int waitTime, int b);
	int		(*GetSI)(int logRegAddr, MString &str, int &commState);
	int		(*SetSO)(int logRegAddr, int waitTime, MString str);
	int		(*GetMI)(int logRegAddr, int &dataType, LAMVarValue &value);
	int		(*GetMIOType)(int logRegAddr, int &dataType);
	int		(*SetMO)(int logRegAddr, int waitTime, ...);
	int		(*GetMSI)(int logRegAddr, MString &str);
	int		(*SetMSO)(int logRegAddr, int waitTime, MString str);
	void	(*FreeMString)(MString str);

	BOOL	(*NeedQuit)(int portId);//退出允许
};

#define INTERFACE_COM_SLAVE_TYPE		102
struct SInterfaceComSlave
{
	int     PortId;// 端口id号
	char*	PortName;//端口名
	char*	ProtocolLibName;//规约库名

	int		StationAddr;//本机站址
	int		AutoTimeout;//1:规约自动选择超时 0:规约使用下面三个超时参数
	int 	MinTimeout;//最短超时，单位毫秒
	int 	MidTimeout;//中间长度超时，单位毫秒
	int 	MaxTimeout;//最长超时，单位毫秒

	BOOL	(*RegSoftTimer)(int &timer);//注册软定时器，该定时器每100毫秒递增1
	int		(*PrintLog)(const char *fmt, ...);//将信息打印进日志缓冲区
	int		(*PrintLogToFile)(const char *fmt, ...);//将信息打印进日志文件
	BOOL	(*WriteSysInfoSoe)(const char *fmt, ...);//写挂载在“$系统信息”变量上的字符串事件

	int		(*OpenCom)(int portId);//返回0表示完全失败 1表示成功 -1表示可能失败
	int		(*ReadCom)(int portId, char *recBuf, int wantLen, int waitTime);
	int		(*WriteCom)(int portId, char *sendBuf, int sendLen);
	void	(*CloseCom)(int portId);

	int		(*GetAI)(int logRegAddr, int &dataType, LAMVarValue &value, int &commState);
	int		(*GetAIOType)(int logRegAddr, int &dataType, int &rwMode);
	int		(*SetAO)(int logRegAddr, int waitTime, ...);
	int		(*GetDI)(int logRegAddr, int &value, int &commState);
	int		(*SetDO)(int logRegAddr, int waitTime, int b);
	int		(*GetSI)(int logRegAddr, MString &str, int &commState);
	int		(*SetSO)(int logRegAddr, int waitTime, MString str);
	int		(*GetMI)(int logRegAddr, int &dataType, LAMVarValue &value);
	int		(*GetMIOType)(int logRegAddr, int &dataType);
	int		(*SetMO)(int logRegAddr, int waitTime, ...);
	int		(*GetMSI)(int logRegAddr, MString &str);
	int		(*SetMSO)(int logRegAddr, int waitTime, MString str);
	void	(*FreeMString)(MString str);

	BOOL	(*NeedQuit)(int portId);//退出允许
};
