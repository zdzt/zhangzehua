#pragma once

#ifdef _WINDOWS
#ifndef _UTF8_
#define _UTF8_
#pragma execution_character_set("utf-8")	//使用UTF8字符集
#endif
#endif

//版本号
#define VERSION_STRING	"5.0.0.0"
#define VERSION_INT		0x05000000
#define VERSION_NOTSUPORT_INT	0x06000000

//各种名称长度
#define VAR_DESCR_LEN		24//DI量0和1状态字串描述长度，AI量分段文字描述长度
#define VAR_NAME_LEN		128//变量名长度
#define PORT_NAME_LEN		16//端口名长度
#define DEV_NAME_LEN		64//设备名长度
#define LIB_NAME_LEN		256//规约库名长度
#define REG_DESCR_LEN		1024*3//寄存器描述字段长度,适应UTF8字符一个汉字是3字节
#define MVAR_SCRIP_LEN		1024*3//M型寄存器上挂的脚本

//延时方面
#define MIN_TIME_RESOLUTION	100//最小时间分辨率，单位：毫秒

//设备寄存器数据类型
#define DEVREGUSHORT	 1 //ushort
#define DEVREGSHORT	 2 //short
#define DEVREGUINT	 3 //uint
#define DEVREGINT	 4 //int
#define DEVREGFLOAT	 5 //float
#define DEVREGCHAR	 6 //char
#define DEVREGUCHAR	 7 //uchar
#define DEVREGBIT	 8 //bit
#define DEVREGSTRING	 9 //string
#define DEVREGBITSTRING	 10 //比特串，映射到逻辑寄存器类型6，RegExtDataType中表示某比特在比特串中的位置
#define DEVREGBITFIELD	 11//短整型位域，一个USHORT中若干个比特组成一个数据，映射到逻辑寄存器类型LOGICREGUINT
  //RegExtDataType中存放(字节顺序 + 起始BIT位置<<8 + 结束BIT位置<<16)
  //字节顺序  1:高字节在前  2:低字节在前
  //起始BIT位置：0~15    结束BIT位置 0~15
#define DEVREGBITFIELD2	 12//整型位域，一个UINT中若干个比特组成一个数据，映射到逻辑寄存器类型LOGICREGUINT
  //RegExtDataType中存放(字节顺序 + 起始BIT位置<<8 + 结束BIT位置<<16)
  //字节顺序  1:ABCD  2:BADC 3:CDAB 4:DCBA
  //起始BIT位置：0~31    结束BIT位置 0~31

//逻辑寄存器数据类型
#define LOGICREGUINT		3 //uint
#define LOGICREGINT			4 //int
#define LOGICREGFLOAT		5 //float
#define LOGICREGBIT			6 //bit
#define LOGICREGSTRING		7 //string

//逻辑数据类型
#define LOGIC_REG_UINT				3 //uint
#define LOGIC_REG_INT				4 //int
#define LOGIC_REG_FLOAT				5 //float


//寄存器读写模式
#define REG_READ_ONLY		1//只读
#define REG_WRITE_ONLY		2//只写
#define REG_READ_WRITE		3//读写

//逻辑寄存器读写模式
#define REG_MODE_READ				1//可读
#define REG_MODE_WRITE				2//可写
#define REG_MODE_READ_WRITE			3//读写

//日志缓冲区长度
#define LOGBUFLEN	1024*4
//日志级别
#define LOGTIP		1	//提示
#define LOGWARNING	2	//警告
#define LOGERROR	3	//错误

//特殊寄存器类型
#define SPECIALREGTYPE	((int)0x80000000)

//配置软件的默认端口
#define IPNETCONFIGDEFAULT	0	//


