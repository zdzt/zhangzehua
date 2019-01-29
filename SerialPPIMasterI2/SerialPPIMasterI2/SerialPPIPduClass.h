#ifndef SERIALPPIPDUCLASS_H
#define SERIALPPIPDUCLASS_H

#include "Constant.h"
#include "ProtocolInterface.h"
#include "PduCheckClass.h"

class CSerialPPIPduClass
{
public:
    CSerialPPIPduClass();
private:
    CPduCheckClass PduCheckClass;
    SInterfaceComMaster * ParaFunc;
private:
    int RecDataFromSerial(unsigned char *RecBuf,int MaxRecLen,int &RealRecLen);
    int RecDataFromSerial(unsigned char *RecBuf,int MaxRecLen);

    int SendDatatoSerial(unsigned char *SendBuf,int SendLen);
public:
    int InitCSerialPPIPduClass(SInterfaceComMaster *p_ParaFunc);
    int SendPagingData(unsigned char DA);//�·�Ѱ��ָ��
    int RecPagingData(unsigned char SA);
    int SendConfirmData(unsigned char DA);

    int SendReadData(unsigned char DA,int ReadLen,int DataNum,int MemType,int OffSet);
    int RecReadConfirmData();//���շ���ָ���ȷ��

    int ResponseAnalysis(int SA,unsigned char *RecBuf,int RecbufLen,unsigned char *&DataBuf,int &DataBufLen);

    int WriteData(int DA,int MemType,int Offset,int DataLen,int DataNum,unsigned char *DataBuf,int DataBufLen);
    int WriteBitData(int DA,int MemType,int Offset,int value);
    int WriteShortData(int DA,int MemType,int Offset,int value);
    int WriteUShortData(int DA,int MemType,int Offset,int value);
    int WriteIntData(int DA,int MemType,int Offset,int value);
    int WriteUIntData(int DA,int MemType,int Offset,int value);
    int WriteFloatData(int DA,int MemType,int Offset,float value);
    int WriteStringData(int DA,int MemType,int Offset,MString value);
    int WriteCharData(int DA,int MemType,int Offset,int value);

    int RecWriteReturn(int SA);
};

#endif // SERIALPPIPDUCLASS_H
