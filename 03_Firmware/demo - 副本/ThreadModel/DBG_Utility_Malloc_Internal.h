//////////////////////////////////////////////////////////////////////////
//�ļ���                 DBG_Utility_Malloc_Internal.h
//���������Թ���ģ��֮���ڴ�����ģ��ͷ�ļ�(ģ���ڲ��ɼ�)
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2011��7��26��	ģ�鴴��               Martin

#ifndef __HH_DBG_UTILITY_MALLOC_INTERNAL_H_HH__
#define __HH_DBG_UTILITY_MALLOC_INTERNAL_H_HH__

#include "DBG_Utility_Malloc.h"  //�� import public header file of debug module

#define MAX_META_FILENAME_LEN   200

//////////////////////////////////////////////////////////////////////////
//Types declaration begin from here..
#define DBG_U_M_HANDLE  INT32
#define DBG_U_M_HANDLE_NULL  -1

typedef struct
{
	DBG_U_M_HANDLE memHdl;
}tMemHeader,*tPMemHeader;

typedef struct
{
	DBG_U_M_HANDLE preNode;
	DBG_U_M_HANDLE NextNode;
}tListNode,*tPListNode;



typedef struct
{
	tListNode   listNode;
	DBG_U_M_HANDLE  hdl;
	void* pAddress;
	CHAR azFileName[MAX_META_FILENAME_LEN];
	INT32U u32LineNO;
	INT32U u32MemSize;	
	
}tDBG_U_MemMetaInfor,*tPDBG_U_MemMetaInfor;



typedef struct  
{
	INT32U u32CurrentUsingMemSize;
	INT32  u32CurrentMemItemCnt;	
}tStatisticInfor,*tPStatisticInfor;

typedef struct  
{
	tDBG_U_MemMetaInfor azMemMetaTable[MAX_MALOCC_ITEM_CNT];
	DBG_U_M_HANDLE RegistedMemItemCnt;

	DBG_U_M_HANDLE  hdlRegistedListHeader;
	DBG_U_M_HANDLE  hdlEmptyListHeader;
	tStatisticInfor statisticInfor;
}tDBG_U_MemManager,*tPDBG_U_MemManager;

//Types declaration ended here.
//////////////////////////////////////////////////////////////////////////
static tDBG_U_MemManager sgDBG_U_MemManager;

void _Init(void);
DBG_U_M_HANDLE _FindNodeByAddress(void* pAddress);
DBG_U_M_HANDLE _RegistMem(CHAR* strFileName,void* pAddress,INT32U u32LineNO,INT32U u32MemSize);
void _UnregistMem(void* pAddress);
void _Statistic(void);

#endif
