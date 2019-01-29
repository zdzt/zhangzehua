#ifndef PDUCHECKCLASS_H
#define PDUCHECKCLASS_H

#define PDUCHECKCLASSVERSION 2.0.0

class CPduCheckClass
{
public:
    CPduCheckClass();
public:
	//计算modbus 校验和
	unsigned short CrcChecker(unsigned char *buf,int buflength);
	//计算char型累加和
	unsigned char SumCheck(unsigned char *checkbuf,int checkbuflength);
	//计算short型累加和
	unsigned short SumShortCheck(unsigned char *checkbuf,int checkbuflength);
	//计算YDT协议的长度
	unsigned short YDLengthCheck(unsigned short length);
	unsigned short YDLengthCheck(unsigned int length);
	unsigned short YDLengthCheck(int length);
	//计算YDT协议的总校验和
	unsigned short YDCumulativeCheck(unsigned char *info ,unsigned int infolength);
	//差位校验
	unsigned char DifCheck(unsigned char *checkbuf,int checkbuflen);
	//异或校验
	unsigned char XORCheck(unsigned char *checkbuf,int checkbuflen);
};

#endif // PDUCHECKCLASS_H
