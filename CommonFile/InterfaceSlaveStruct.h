#ifndef _INTERFACE_SLAVE_STRUCT_
#define _INTERFACE_SLAVE_STRUCT_

#pragma pack(push, 4)//���ֽڶ��룬��ӦX64��X86
union LAMVarValue//A,M��������3~5��ֵ
{
	UINT ui; //��������LOGIC_REG_UINT
	int i; //��������LOGIC_REG_INT
	float f; //��������LOGIC_REG_FLOAT
	int b;//��������LOGIC_REG_BIT
};
#pragma pack (pop)

#endif