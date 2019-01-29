#include "stdafx.h"
#include "SerialPPIPduClass.h"
#include "stdio.h"

#define SHORTSENDBUFLENGTH 32
#define SHORTRECBUFLENGTH  32
#define SENDBUFLENGTH      512
#define RECBUFLENGTH	   512
#define PDUEXTRALEN	   6

#define DEBUG

CSerialPPIPduClass::CSerialPPIPduClass()
{

}

int CSerialPPIPduClass::InitCSerialPPIPduClass(SInterfaceComMaster *p_ParaFunc)
{
    ParaFunc=p_ParaFunc;
    return 1;
}

int CSerialPPIPduClass::RecDataFromSerial(unsigned char *RecBuf,int MaxRecLen,int &RealRecLen)
{
    RealRecLen=ParaFunc->ReadCom(ParaFunc->PortId,(char *)RecBuf,MaxRecLen,ParaFunc->MidTimeout);
    RealRecLen=RealRecLen<0?0:RealRecLen;

    return RealRecLen;
}

int CSerialPPIPduClass::RecDataFromSerial(unsigned char *RecBuf,int MaxRecLen)
{
    int RealRecLen=ParaFunc->ReadCom(ParaFunc->PortId,(char *)RecBuf,MaxRecLen,ParaFunc->MidTimeout);
    RealRecLen=RealRecLen<0?0:RealRecLen;

    return RealRecLen;
}

int CSerialPPIPduClass::SendDatatoSerial(unsigned char *SendBuf,int SendLen)
{
    return this->ParaFunc->WriteCom(ParaFunc->PortId,(char *)SendBuf,SendLen);
}

int CSerialPPIPduClass::SendPagingData(unsigned char DA)
{//下发寻呼指令
    unsigned char sendbuf[SHORTSENDBUFLENGTH];
    int sendbuflen=0;
    sendbuf[sendbuflen++]=0x10;
    sendbuf[sendbuflen++]=DA;
    sendbuf[sendbuflen++]=0x00;
    sendbuf[sendbuflen++]=0x49;

    sendbuf[sendbuflen]=PduCheckClass.SumCheck(sendbuf+1,sendbuflen-1);
    sendbuflen++;

    sendbuf[sendbuflen++]=0x16;

    return SendDatatoSerial(sendbuf,sendbuflen);
}

int CSerialPPIPduClass::RecPagingData(unsigned char SA)
{//分析接收的指令
    unsigned char recbuf[SHORTRECBUFLENGTH];
    int recbuflen = this->RecDataFromSerial(recbuf,SHORTRECBUFLENGTH);
    //校验数据
    int i_recbufpos=0;
    int i_state=0;
    int i_startpos=0;
    while(i_recbufpos < recbuflen)
    {
		switch(i_state)
		{
		case 0://找起始符
			if(recbuf[i_recbufpos]==0x10)
			{//找到起始符
				i_state = 1;
				i_startpos=i_recbufpos;

				//如果长度不够则重读
				if(recbuflen - i_startpos < 6)
				{//重读数据
					recbuflen+=this->RecDataFromSerial(recbuf+recbuflen,SHORTRECBUFLENGTH-recbuflen);
				}

			}
			i_recbufpos++;
			break;
		case 1://对比目的地址，目的地址一定为0
			if(recbuf[i_recbufpos] != 0x00)
			{
				goto failed;
			}
			i_state = 2;
			i_recbufpos++;
			break;
		case 2://对比源地址
			if(recbuf[i_recbufpos] != SA)
			{
				goto failed;
			}

			i_state = 3;
			i_recbufpos++;

			break;
		case 3://回应的错误码
			if(recbuf[i_recbufpos] != 0x00)
			{
				goto failed;
			}
			i_state = 4;
			i_recbufpos++;

			break;
		case 4://对比校验和
	    {
			unsigned char recsum=PduCheckClass.SumCheck(recbuf+i_startpos+1,3);
			if(recbuf[i_recbufpos] != recsum)
			{//校验和不正确
				goto failed;
			}

			i_state = 5;
			i_recbufpos++;
	    }
	    break;
		case 5://对比结束符
			if(recbuf[i_recbufpos] == 0x16)
				goto success;
			else
				goto failed;
			break;
		}
    }

failed:
    return 0;
success:
    return 1;
}

int CSerialPPIPduClass::SendConfirmData(unsigned char DA)
{//下发确认指令
    unsigned char sendbuf[SHORTSENDBUFLENGTH];
    int sendbuflen=0;
    sendbuf[sendbuflen++]=0x10;
    sendbuf[sendbuflen++]=DA;
    sendbuf[sendbuflen++]=0x0;
    sendbuf[sendbuflen++]=0x5C;

    sendbuf[sendbuflen]=PduCheckClass.SumCheck(sendbuf+1,sendbuflen-1);
    sendbuflen++;

    sendbuf[sendbuflen++]=0x16;

    return SendDatatoSerial(sendbuf,sendbuflen);
}

int CSerialPPIPduClass::SendReadData(unsigned char DA,int ReadLen,int DataNum,int MemType,int OffSet)
{//发送读取命令指令
    unsigned char sendbuf[SENDBUFLENGTH];
    int sendbuflen = 0;
    sendbuf[sendbuflen++]=0x68;//开始符
    sendbuf[sendbuflen++]=0x00;//长度，回填
    sendbuf[sendbuflen++]=0x00;//确认长度，回填
    sendbuf[sendbuflen++]=0x68;//开始符
    sendbuf[sendbuflen++]=DA;//目的地址
    sendbuf[sendbuflen++]=0x00;//源地址
    sendbuf[sendbuflen++]=0x6C;//功能码，0x6C为读功能码
    sendbuf[sendbuflen++]=0x32;//协议识别
    sendbuf[sendbuflen++]=0x01;//远程控制
    sendbuf[sendbuflen++]=0x00;//冗余识别
    sendbuf[sendbuflen++]=0x00;//冗余识别
    sendbuf[sendbuflen++]=0x00;//协议数据
    sendbuf[sendbuflen++]=0x00;//单元参考
    sendbuf[sendbuflen++]=0x00;//参数长度,回填
    sendbuf[sendbuflen++]=0x0E;//参数长度,回填

    sendbuf[sendbuflen++]=0x00;//数据长度
    sendbuf[sendbuflen++]=0x00;//数据长度

    sendbuf[sendbuflen++]=0x04;//
    sendbuf[sendbuflen++]=0x01;
    sendbuf[sendbuflen++]=0x12;
    sendbuf[sendbuflen++]=0x0A;
    sendbuf[sendbuflen++]=0x10;
    //读取长度
    sendbuf[sendbuflen++]=ReadLen&0xff;//读取长度
    sendbuf[sendbuflen++]=0x00;
    //数据个数
    sendbuf[sendbuflen++]=DataNum&0xff;
    sendbuf[sendbuflen++]=0;
    //存储器类型
    if(MemType == 0x01||MemType == 0x84||MemType==0x0184)
    {//V存储器
		sendbuf[sendbuflen++]=0x01;
		sendbuf[sendbuflen++]=0x84;
    }
    else
    {//其它存储器
		sendbuf[sendbuflen++]=0x00;
		sendbuf[sendbuflen++]=MemType&0xff;
    }
    //偏移量
    sendbuf[sendbuflen++]=(OffSet>>16)&0xff;
    sendbuf[sendbuflen++]=(OffSet>>8)&0xff;
    sendbuf[sendbuflen++]=OffSet&0xff;
    //回填长度
    sendbuf[1] = sendbuflen - 4;
    sendbuf[2] = sendbuf[1];
    //校验码
    sendbuf[sendbuflen]=PduCheckClass.SumCheck(sendbuf+4,sendbuflen-4);
    sendbuflen++;
    //结束符
    sendbuf[sendbuflen++]=0x16;

    return SendDatatoSerial(sendbuf,sendbuflen);
}

int CSerialPPIPduClass::RecReadConfirmData()
{//接收发送指令的确认,只有接收到 0xE5 和 0xF9
    unsigned char recbuf[SHORTRECBUFLENGTH];
    int recbuflen=this->RecDataFromSerial(recbuf,SHORTRECBUFLENGTH);

    if(recbuflen ==0)
		return 0;

    if((recbuf[0]==0xE5)||(recbuf[0]==0xF9))
		return 1;

    return 0;
}

int CSerialPPIPduClass::ResponseAnalysis(int SA,unsigned char *RecBuf,int RecbufLen,unsigned char *&DataBuf,int &DataBufLen)
{//读数据并返回数据缓冲区
    int reclen=this->RecDataFromSerial(RecBuf,RecbufLen);
    int recbufpos = 0;

    int state = 0;
    int startpos=0;
    int pdulen=0;
    int count = 0;
    int datastartpos=0;
    int countlen=0;

    while(recbufpos < reclen)
    {
		switch(state)
		{
		case 0://查找起始符0x68
			if(RecBuf[recbufpos] == 0x68)
			{
				startpos=recbufpos;
				state = 1;
				count = 0;
			}
			recbufpos++;
			break;
		case 1://读取长度
			pdulen=RecBuf[recbufpos];
			recbufpos++;
			state = 2;
			break;
		case 2://读取长度确认
			if(RecBuf[recbufpos]==pdulen)
			{//长度相同是要取的长度
				//判断长度是否够
				if((reclen - startpos) < (pdulen+PDUEXTRALEN))
				{//重读数据
					reclen+=RecDataFromSerial(RecBuf+reclen,RecbufLen-reclen);
					//reclen+=m_p_ParaFunc->ReadPort(m_p_ParaFunc->PortId,RecBuf+reclen,RecbufLen-reclen,m_p_ParaFunc->MidTimeout);
					if((reclen - startpos) < (pdulen+PDUEXTRALEN))
					{//重读后，长度还是不够
						goto failed;
					}
				}

				recbufpos++;
				state = 3;
			}
			else
			{//长度不同不是要取的长度
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 3://值为0x68H
			if(RecBuf[recbufpos] == 0x68)
			{
				recbufpos++;
				state=4;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 4://目的地址为00
			if(RecBuf[recbufpos] == 0x00)
			{
				recbufpos++;
				state = 5;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 5://源地址
			if(RecBuf[recbufpos] == (SA&0xff))
			{
				recbufpos++;
				state = 6;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 6://功能码
			if(RecBuf[recbufpos] == 0x08)
			{
				recbufpos++;
				state = 7;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 7://协议识别
			if(RecBuf[recbufpos] == 0x32)
			{
				recbufpos ++;
				state = 8;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 8://远程控制
			recbufpos++;
			state=9;
			count=0;
			break;
		case 9://冗余识别(2),协议数据(1),单元参考(1),参数长度(2)
			recbufpos+=6;
			state=10;
			break;
		case 10://数据长度
			if(count < 2)
			{
				count++;
				recbufpos++;
			}
			else
			{
				DataBufLen=((RecBuf[recbufpos-2]<<8)+RecBuf[recbufpos-1])-4;
				count = 0;
				state = 11;
			}
			break;
		case 11://未知数据2位
			recbufpos+=2;
			state = 12;
			break;
		case 12://参数长度
			countlen = RecBuf[recbufpos++];
			state = 13;
			recbufpos +=countlen;//越过参数信息
			break;
		case 13://数据长度
			if(((DataBufLen<<3)&0xFF) == RecBuf[recbufpos])
			{
				recbufpos++;
				datastartpos=recbufpos;
				state = 14;
				recbufpos+= DataBufLen;
			}
			else
			{
				goto failed;
			}
			break;
		case 14://计算校验
			if(RecBuf[recbufpos]==PduCheckClass.SumCheck(RecBuf+startpos+4,recbufpos- startpos - 4))
			{
				recbufpos ++;
				state = 15;
			}
			else
			{
				printf("CSerialPPIPduClass::ResponseAnalysis Check Failed\n");

				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 15://结束符
			if(RecBuf[recbufpos]==0x16)
			{//成功
				recbufpos++;
				goto success;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		}
    }
failed://取值失败
    DataBuf = NULL;
    DataBufLen = 0;
    return 0;
success://取值成功
    DataBuf = RecBuf+datastartpos;
    return 1;
}

int CSerialPPIPduClass::WriteData(int DA,int MemType,int Offset,int DataLen,int DataNum,unsigned char *DataBuf,int DataBufLen)
{//向PLC写入数据
    unsigned char sendbuf[SENDBUFLENGTH];
    int sendbuflen = 0;
    //开始符
    sendbuf[sendbuflen++] = 0x68;
    //报文长度
    sendbuf[sendbuflen++] = 0x00;
    //确认长度
    sendbuf[sendbuflen++] = 0x00;
    //起始符
    sendbuf[sendbuflen++] = 0x68;
    //目标地址
    sendbuf[sendbuflen++] = DA&0xff;
    //源地址
    sendbuf[sendbuflen++] = 0x00;
    //功能码
    sendbuf[sendbuflen++] = 0x7C;
    //协议识别
    sendbuf[sendbuflen++] = 0x32;
    //远程控制
    sendbuf[sendbuflen++] = 0x01;
    //冗余识别
    sendbuf[sendbuflen++] = 0x00;
    sendbuf[sendbuflen++] = 0x00;
    //协议数据
    sendbuf[sendbuflen++] = 0x00;
    //单元数据
    sendbuf[sendbuflen++] = 0x00;
    //参数长度
    sendbuf[sendbuflen++] = 0x00;
    sendbuf[sendbuflen++] = 0x0E;
    //数据长度
    sendbuf[sendbuflen++] = ((DataBufLen+4)>>8)&0xFF;
    sendbuf[sendbuflen++] = (DataBufLen+4)&0xFF;
    //04读，05写
    sendbuf[sendbuflen++] = 0x05;
    //变量地址数
    sendbuf[sendbuflen++] = 0x01;

    sendbuf[sendbuflen++] = 0x12;
    sendbuf[sendbuflen++] = 0x0A;
    sendbuf[sendbuflen++] = 0x10;
    //数据长度
    sendbuf[sendbuflen++] = DataLen&0xFF;
    sendbuf[sendbuflen++] = (DataLen>>8)&0xFF;
    //数据个数
    sendbuf[sendbuflen++] = DataNum&0xFF;
    sendbuf[sendbuflen++] = (DataNum>>8)&0xFF;
    //存储类型
    if(MemType==1||MemType == 0x84||MemType==0x0184)
    {
		sendbuf[sendbuflen++] = 0x01;
		sendbuf[sendbuflen++] = MemType&0xFF;
    }
    else
    {
		sendbuf[sendbuflen++] = 0x00;
		sendbuf[sendbuflen++] = MemType&0xFF;
    }

    //偏移量
    sendbuf[sendbuflen++] = (Offset>>16)&0xFF;
    sendbuf[sendbuflen++] = (Offset>>8)&0xFF;
    sendbuf[sendbuflen++] = Offset&0xFF;
    //数据形式
    if(DataLen==1)
    {
		sendbuf[sendbuflen++] = 0x00;
		sendbuf[sendbuflen++] = 0x03;
    }
    else
    {
		sendbuf[sendbuflen++] = 0x00;
		sendbuf[sendbuflen++] = 0x04;
    }
    //数据位数
    if(DataLen==1)
    {
		sendbuf[sendbuflen++] = (DataNum>>8)&0xFF;
		sendbuf[sendbuflen++] = DataNum&0xFF;
    }
    else
    {
		sendbuf[sendbuflen++] = ((DataBufLen<<3)>>8)&0xFF;
		sendbuf[sendbuflen++] = (DataBufLen<<3)&0xFF;
    }
    //写入值，随数据变化
    for(int i=0;i<DataBufLen;i++)
    {
		sendbuf[sendbuflen++] = DataBuf[i];
    }
    //回填长度
    sendbuf[2]=sendbuf[1]=sendbuflen-4;
    //校验
    sendbuf[sendbuflen]=PduCheckClass.SumCheck(sendbuf+4,sendbuflen-4);
    sendbuflen++;
    //结束符
    sendbuf[sendbuflen++] = 0x16;


    return SendDatatoSerial(sendbuf,sendbuflen);
}

int CSerialPPIPduClass::WriteBitData(int DA,int MemType,int Offset,int value)
{
    if(this->WriteData(DA,MemType,Offset,01,01,(unsigned char *)&value,1))
    {
		if(this->RecReadConfirmData())
		{
			if(this->SendConfirmData(DA))
			{
				if(this->RecWriteReturn(DA))
				{
					return 1;
				}
			}
		}
    }
    return 0;
}

int CSerialPPIPduClass::WriteCharData(int DA,int MemType,int Offset,int value)
{
    if(this->WriteData(DA,MemType,Offset,02,01,(unsigned char *)&value,1))
    {
		if(this->RecReadConfirmData())
		{
			if(this->SendConfirmData(DA))
			{
				if(this->RecWriteReturn(DA))
				{
					return 1;
				}
			}
		}
    }

    return 0;
}

int CSerialPPIPduClass::WriteShortData(int DA,int MemType,int Offset,int value)
{

    if(this->WriteData(DA,MemType,Offset,04,01,(unsigned char *)&value,2))
    {
		if(this->RecReadConfirmData())
		{
			if(this->SendConfirmData(DA))
			{
				if(this->RecWriteReturn(DA))
				{
					return 1;
				}
			}
		}
    }


    return 0;
}

int CSerialPPIPduClass::WriteUShortData(int DA,int MemType,int Offset,int value)
{

    if(this->WriteData(DA,MemType,Offset,04,01,(unsigned char *)&value,2))
    {
		if(this->RecReadConfirmData())
		{
			if(this->SendConfirmData(DA))
			{
				if(this->RecWriteReturn(DA))
				{
					return 1;
				}
			}
		}
    }
    return 0;
}

int CSerialPPIPduClass::WriteIntData(int DA,int MemType,int Offset,int value)
{

    if(this->WriteData(DA,MemType,Offset,06,01,(unsigned char *)&value,4))
    {
		if(this->RecReadConfirmData())
		{
			if(this->SendConfirmData(DA))
			{
				if(this->RecWriteReturn(DA))
				{
					return 1;
				}
			}
		}
    }

    return 0;
}

int CSerialPPIPduClass::WriteUIntData(int DA,int MemType,int Offset,int value)
{

    if(this->WriteData(DA,MemType,Offset,06,01,(unsigned char *)&value,4))
    {
		if(this->RecReadConfirmData())
		{
			if(this->SendConfirmData(DA))
			{
				if(this->RecWriteReturn(DA))
				{
					return 1;
				}
			}
		}
    }

    return 0;
}

int CSerialPPIPduClass::WriteFloatData(int DA,int MemType,int Offset,float value)
{
    if(this->WriteData(DA,MemType,Offset,02,04,(unsigned char *)&value,4))
    {
		if(this->RecReadConfirmData())
		{
			if(this->SendConfirmData(DA))
			{
				if(this->RecWriteReturn(DA))
				{
					return 1;
				}
			}
		}
    }

    return 0;
}

int CSerialPPIPduClass::WriteStringData(int DA,int MemType,int Offset,MString value)
{

    if(this->WriteData(DA,MemType,Offset,02,value.len,(unsigned char *)value.buf,value.len))
    {
		if(this->RecReadConfirmData())
		{
			if(this->SendConfirmData(DA))
			{
				if(this->RecWriteReturn(DA))
				{
					return 1;
				}
			}
		}
    }

    return 0;
}


int CSerialPPIPduClass::RecWriteReturn(int SA)
{
    unsigned char RecBuf[RECBUFLENGTH];
    int RecbufLen = RECBUFLENGTH;

    int reclen=this->RecDataFromSerial(RecBuf,RecbufLen);
    int recbufpos = 0;

    int state = 0;
    int startpos=0;
    int pdulen=0;
    int count = 0;
    while(recbufpos < reclen)
    {
		switch(state)
		{
		case 0://查找起始符0x68
			if(RecBuf[recbufpos] == 0x68)
			{
				startpos=recbufpos;
				state = 1;
				count = 0;
			}
			recbufpos++;
			break;
		case 1://读取长度
			pdulen=RecBuf[recbufpos];
			recbufpos++;
			state = 2;
			break;
		case 2://读取长度确认
			if(RecBuf[recbufpos]==pdulen)
			{//长度相同是要取的长度
				//判断长度是否够
				if((reclen - startpos) < (pdulen+PDUEXTRALEN))
				{//重读数据
					reclen+=RecDataFromSerial(RecBuf+reclen,RecbufLen-reclen);
					//reclen+=m_p_ParaFunc->ReadPort(m_p_ParaFunc->PortId,RecBuf+reclen,RecbufLen-reclen,m_p_ParaFunc->MidTimeout);
					if((reclen - startpos) < (pdulen+PDUEXTRALEN))
					{//重读后，长度还是不够
						goto failed;
					}
				}

				recbufpos++;
				state = 3;
			}
			else
			{//长度不同不是要取的长度
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 3://值为0x68H
			if(RecBuf[recbufpos] == 0x68)
			{
				recbufpos++;
				state=4;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 4://目的地址为00
			if(RecBuf[recbufpos] == 0x00)
			{
				recbufpos++;
				state = 5;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 5://源地址
			if(RecBuf[recbufpos] == (SA&0xff))
			{
				recbufpos++;
				state = 6;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 6://功能码
			if(RecBuf[recbufpos] == 0x08)
			{
				recbufpos++;
				state = 7;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 7://协议识别
			if(RecBuf[recbufpos] == 0x32)
			{
				recbufpos ++;
				state = 8;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 8://远程控制,//冗余识别(2),协议数据(1),单元参考(1),参数长度(2),//数据长度,//未知数据2位,参数和数据
			recbufpos+=14;
			state=9;
			count=0;
			break;
		case 9://计算校验
			if(RecBuf[recbufpos]==PduCheckClass.SumCheck(RecBuf+startpos+4,recbufpos- startpos - 4))
			{
				recbufpos ++;
				state = 10;
			}
			else
			{
				printf("CSerialPPIPduClass::ResponseAnalysis Check Failed\n");

				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		case 10://结束符
			if(RecBuf[recbufpos]==0x16)
			{//成功
				recbufpos++;
				goto success;
			}
			else
			{
				recbufpos = startpos + 1;
				state = 0;
			}
			break;
		}
    }
failed://取值失败
    return 0;
success://取值成功
    return 1;
}
