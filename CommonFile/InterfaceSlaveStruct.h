#ifndef _INTERFACE_SLAVE_STRUCT_
#define _INTERFACE_SLAVE_STRUCT_

#pragma pack(push, 4)//四字节对齐，适应X64和X86
union LAMVarValue//A,M数据类型3~5的值
{
	UINT ui; //数据类型LOGIC_REG_UINT
	int i; //数据类型LOGIC_REG_INT
	float f; //数据类型LOGIC_REG_FLOAT
	int b;//数据类型LOGIC_REG_BIT
};
#pragma pack (pop)

#endif