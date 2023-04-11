//////////////////////////////////////////////////////////////////////////
//�ļ���                 DBG_Utility_Malloc.c
//���������Թ���ģ��֮���ڴ�����ģ��ʵ���ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2011��7��26��	ģ�鴴��               Martin
//#include <Windows.h>
//#include <time.h>
#include "ModuleCommon.h"
#include "DBG_Utility_Malloc_Internal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//HANDLE sghLock = NULL;
//INT32U u32TmpCount = 0;//cl_del
void* malloc_x(
   INT32U u32DesiredDataSize,
   CHAR* strFileName,         /*[IN] ���뷢�������ļ�*/
	INT32U u32LineNO           /*[IN] ���뷢�������ļ��е��к�*/
	)
{
	void* rtn;
	
	rtn = malloc(u32DesiredDataSize);	
//	if(NULL != rtn)
//	{
//		u32TmpCount++;
//	}
	//printf("%s[L%d]: malloc (0x%x,%d) \r\n",strFileName,u32LineNO,(INT32U)rtn,u32DesiredDataSize);	
	return rtn;
}
void free_x(
   void* pMem,
   CHAR* strFileName,         /*[IN] ���뷢�������ļ�*/
	 INT32U u32LineNO           /*[IN] ���뷢�������ļ��е��к�*/
	)
{	
	free(pMem);		
//	u32TmpCount--;
	//printf("%s[L%d]: free (0x%x) \r\n",strFileName,u32LineNO,(INT32U)pMem);	
}

//INT32U GetSP(void)
//{
//return __current_sp();

//}
#if 0
static BOOL  _Lock()
{
	DWORD dwWaitRtn = 0;
	if (sghLock == NULL)
	{
		return FALSE;
	}
	/*S1:�ȴ��������*/
	dwWaitRtn = WaitForSingleObject(sghLock,INFINITE);
	if (dwWaitRtn == WAIT_OBJECT_0)
	{
		return TRUE;
	}
	return FALSE;
}
static BOOL  _Unlock()
{
	DWORD dwWaitRtn = 0;
	if (sghLock == NULL)
	{
		return FALSE;
	}
	ReleaseMutex(sghLock);
	return TRUE;
}


void _Init(void)
{
	INT32U u32Index =0;	

	//sghLock = CreateMutex(NULL,FALSE,NULL);

	/*S1:��ʼ���ڴ�Ԫ��Ϣ��*/
	/*S1-1:ͷ����ʼ��*/
	sgDBG_U_MemManager.azMemMetaTable[0].azFileName[0]='\0';
	sgDBG_U_MemManager.azMemMetaTable[0].hdl = 0;
	sgDBG_U_MemManager.azMemMetaTable[0].pAddress = NULL;
	sgDBG_U_MemManager.azMemMetaTable[0].u32LineNO = 0;
	sgDBG_U_MemManager.azMemMetaTable[0].u32MemSize = 0;
	sgDBG_U_MemManager.azMemMetaTable[0].listNode.preNode  = DBG_U_M_HANDLE_NULL;
	sgDBG_U_MemManager.azMemMetaTable[0].listNode.NextNode = 1;

	/*S1-2:��ȥͷβ�ڵ��Ľڵ��ʼ��*/
	for ( u32Index=1;u32Index<MAX_MALOCC_ITEM_CNT-1;u32Index++ )
	{
		sgDBG_U_MemManager.azMemMetaTable[u32Index].azFileName[0]='\0';
		sgDBG_U_MemManager.azMemMetaTable[u32Index].hdl = u32Index;
		sgDBG_U_MemManager.azMemMetaTable[u32Index].pAddress = NULL;
		sgDBG_U_MemManager.azMemMetaTable[u32Index].u32LineNO = 0;
		sgDBG_U_MemManager.azMemMetaTable[u32Index].u32MemSize = 0;
		sgDBG_U_MemManager.azMemMetaTable[u32Index].listNode.preNode  = u32Index-1;
		sgDBG_U_MemManager.azMemMetaTable[u32Index].listNode.NextNode = u32Index+1;
	}

	/*S1-3:β����ʼ��*/
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].azFileName[0]='\0';
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].hdl = MAX_MALOCC_ITEM_CNT-1;
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].pAddress = NULL;
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].u32LineNO = 0;
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].u32MemSize = 0;
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].listNode.preNode  = MAX_MALOCC_ITEM_CNT-2;
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].listNode.NextNode = DBG_U_M_HANDLE_NULL;


	/*S2:��ʼ��"ʹ��"��"δʹ��"�б�����*/
	sgDBG_U_MemManager.hdlRegistedListHeader = DBG_U_M_HANDLE_NULL;//�� ʹ���б�ָ���
	sgDBG_U_MemManager.RegistedMemItemCnt = 0;
	sgDBG_U_MemManager.hdlEmptyListHeader = 0;//�� δʹ���б�ָ���������0���ڵ�
	/*S3:ͳ�����ݳ�ʼ��*/
	sgDBG_U_MemManager.statisticInfor.u32CurrentMemItemCnt = 0;
	sgDBG_U_MemManager.statisticInfor.u32CurrentUsingMemSize = 0;
	
}
tPMemHeader _GetMemHeaderByDataZoneAddress(void* pDataZoneAddress)
{
	tPMemHeader pMemHeader = NULL;
	if (pDataZoneAddress)
	{		
		pMemHeader = (tPMemHeader)((INT32U)(pDataZoneAddress)-sizeof(tMemHeader));
	}

	return pMemHeader;
}
void _EmbedMemHeader2Mem(void* pAddress,tPMemHeader pHeader)
{
	if (pAddress)
	{		
		*((tPMemHeader)pAddress) = *pHeader;
	}
}

void* _GetDataZoneOfMem(void* pAddress)
{
	return (void*)((INT32U)(pAddress)+sizeof(tMemHeader));
}


DBG_U_M_HANDLE _FindNodeByAddress(void* pDataZoneAddress)
{
	tPMemHeader pMemHeader = NULL;
	DBG_U_M_HANDLE rtnHdl = DBG_U_M_HANDLE_NULL;
	/*S1:����ڴ�ͷ������Ϣ*/
	pMemHeader = _GetMemHeaderByDataZoneAddress(pDataZoneAddress);
	if (pMemHeader)
	{
		rtnHdl = (pMemHeader->memHdl);
	}

	return rtnHdl;
}

DBG_U_M_HANDLE _ApplyNewRegistMetaNode()
{
	DBG_U_M_HANDLE rtnHdl = DBG_U_M_HANDLE_NULL;
	if (sgDBG_U_MemManager.hdlRegistedListHeader>=MAX_MALOCC_ITEM_CNT)
	{
		rtnHdl = DBG_U_M_HANDLE_NULL;
	}
	else
	{
		/*S1:��"δʹ��"���л�ȡһ���ڵ�*/
		rtnHdl = sgDBG_U_MemManager.hdlEmptyListHeader;
		/*S2:����"δʹ��"����ͷ�������*/
		sgDBG_U_MemManager.hdlEmptyListHeader = sgDBG_U_MemManager.azMemMetaTable[rtnHdl].listNode.NextNode;

		if (sgDBG_U_MemManager.hdlEmptyListHeader != DBG_U_M_HANDLE_NULL)
		{
			sgDBG_U_MemManager.azMemMetaTable[sgDBG_U_MemManager.hdlEmptyListHeader].listNode.preNode = DBG_U_M_HANDLE_NULL;
		}
	}

	return rtnHdl;
}

void _RtnNode2EmptyList(DBG_U_M_HANDLE hdl)
{
	if (hdl!=DBG_U_M_HANDLE_NULL)
	{
		/*S1:�����黹�ڵ���Ϣ*/
		sgDBG_U_MemManager.azMemMetaTable[hdl].azFileName[0]='\0';
		sgDBG_U_MemManager.azMemMetaTable[hdl].pAddress = NULL;
		sgDBG_U_MemManager.azMemMetaTable[hdl].u32LineNO = 0;
		sgDBG_U_MemManager.azMemMetaTable[hdl].u32MemSize = 0;
		sgDBG_U_MemManager.azMemMetaTable[hdl].listNode.NextNode = sgDBG_U_MemManager.hdlEmptyListHeader;
		if (sgDBG_U_MemManager.hdlEmptyListHeader != DBG_U_M_HANDLE_NULL)
		{
			sgDBG_U_MemManager.azMemMetaTable[sgDBG_U_MemManager.hdlEmptyListHeader].listNode.preNode = hdl;
		}
		sgDBG_U_MemManager.azMemMetaTable[hdl].listNode.preNode = DBG_U_M_HANDLE_NULL;
		/*S2:����"δʹ��"�б�������Ϣ*/
		sgDBG_U_MemManager.hdlEmptyListHeader = hdl;
	}
}
void _AddNode2RegestedList(DBG_U_M_HANDLE hdlMetaNode)
{
	if (hdlMetaNode!=DBG_U_M_HANDLE_NULL)
	{
		/*S1:�����ڵ���Ϣ*/
		if (sgDBG_U_MemManager.hdlRegistedListHeader != DBG_U_M_HANDLE_NULL)
		{
			sgDBG_U_MemManager.azMemMetaTable[sgDBG_U_MemManager.hdlRegistedListHeader].listNode.preNode = hdlMetaNode;
		}
		
		sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.NextNode = sgDBG_U_MemManager.hdlRegistedListHeader;
		sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.preNode = DBG_U_M_HANDLE_NULL;
		/*S2:����"δʹ��"�б�������Ϣ*/
		sgDBG_U_MemManager.hdlRegistedListHeader = hdlMetaNode;
		sgDBG_U_MemManager.RegistedMemItemCnt ++;
	}
}
#if TARGETOS==X86
#pragma warning(disable:4996)
#endif

#define STR_CPY(dest,src)  strcpy(dest,src)

DBG_U_M_HANDLE _RegistMem(CHAR* strFileName,void* pDataZoneAddress,INT32U u32LineNO,INT32U u32MemSize)
{
	DBG_U_M_HANDLE hdlMetaNode = DBG_U_M_HANDLE_NULL;
	/*S1:���һ��Ԫ�����ڵ�*/
	hdlMetaNode = _ApplyNewRegistMetaNode();
	if (hdlMetaNode!=DBG_U_M_HANDLE_NULL)
	{
		/*S2:��дԪ������Ϣ���ڵ�*/
		STR_CPY(sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].azFileName,strFileName);
		sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].pAddress = pDataZoneAddress;
		sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].u32LineNO = u32LineNO;
		sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].u32MemSize = u32MemSize;
		/*S3:�������ڵ�Ͷ��"ʹ��"�б�*/
		_AddNode2RegestedList(hdlMetaNode);
	}
	return hdlMetaNode;
}

void _RemoveNodeFromRegistedList(DBG_U_M_HANDLE hdlMetaNode)
{
	tListNode tmpNodeForFakePre,tmpNodeForFakeNext;
	tPListNode pPreNode,pNextNode;

	if (hdlMetaNode != DBG_U_M_HANDLE_NULL)
	{
		
		/*S1:��Ϊͷ���������б�������Ϣ*/
		if (hdlMetaNode == sgDBG_U_MemManager.hdlRegistedListHeader)
		{
			sgDBG_U_MemManager.hdlRegistedListHeader = sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.NextNode;
		}
		/*S2:�����ڵ�ǰ����ڵ���Ϣ*/
		/*S2-1:��ýڵ��ǰ����ڵ�*/
		pPreNode = (sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.preNode==DBG_U_M_HANDLE_NULL)?&tmpNodeForFakePre:
			(tPListNode)&sgDBG_U_MemManager.azMemMetaTable[sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.preNode];
		pNextNode = (sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.NextNode==DBG_U_M_HANDLE_NULL)?&tmpNodeForFakeNext:
			(tPListNode)&sgDBG_U_MemManager.azMemMetaTable[sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.NextNode];
		/*S2-2:����ǰ����ڵ���Ϣ*/
		pPreNode->NextNode = sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.NextNode;
		pNextNode->preNode = sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.preNode;
		sgDBG_U_MemManager.RegistedMemItemCnt -- ;
	}
}
void _UnregistMem(void* pDataZoneAddress)
{
	DBG_U_M_HANDLE hdlMetaNode = DBG_U_M_HANDLE_NULL;
	if (pDataZoneAddress)
	{
		/*S1:��ȡԪ�����ڵ�*/
		hdlMetaNode = _FindNodeByAddress(pDataZoneAddress);

		if (hdlMetaNode != DBG_U_M_HANDLE_NULL)
		{
			/*S2:���ýڵ��"ʹ��"�б���ȥ��*/
			_RemoveNodeFromRegistedList(hdlMetaNode);
			/*S3:���ýڵ�Ͷ��"δʹ��"�б�*/
			_RtnNode2EmptyList(hdlMetaNode);
		}
	}
}

void* _Malloc(INT32U memSize)
{
	void* rtnMem = NULL;

	rtnMem = malloc(memSize+sizeof(tMemHeader));

	return rtnMem;
}

void* _Realloc(void* pOrigMem,INT32U memSize)
{
	void* rtnMem = NULL;

	rtnMem = realloc((void*)_GetMemHeaderByDataZoneAddress(pOrigMem),memSize+sizeof(tMemHeader));

	return rtnMem;
}

void _Statistic(void)
{
	DBG_U_M_HANDLE hdlMemMeta = DBG_U_M_HANDLE_NULL;
	sgDBG_U_MemManager.statisticInfor.u32CurrentMemItemCnt = 0;
	sgDBG_U_MemManager.statisticInfor.u32CurrentUsingMemSize = 0;
	/*S1:����"ʹ��"�б��ȡ��ǰʹ���ڴ��С���ڴ���Ŀ��*/
	hdlMemMeta = sgDBG_U_MemManager.hdlRegistedListHeader;
	while (hdlMemMeta!= DBG_U_M_HANDLE_NULL)
	{
		sgDBG_U_MemManager.statisticInfor.u32CurrentMemItemCnt ++;
		sgDBG_U_MemManager.statisticInfor.u32CurrentUsingMemSize += sgDBG_U_MemManager.azMemMetaTable[hdlMemMeta].u32MemSize;
		hdlMemMeta = sgDBG_U_MemManager.azMemMetaTable[hdlMemMeta].listNode.NextNode;
	}
}


/*
DBG_UTILITY_M_Init
------------------
����������ģ��֮�ڴ����ģ��ĳ�ʼ��
Mark��δ����ʼ����ģ�鴦��δ�����״̬����ʹ�ã�
REF�� 
------------------
Author��Martin
Date��  2011��7��26��
*/
void DBG_UTILITY_M_Init(
	PINT16U pu16ErrorCode           /*[OUT] ���صĲ���ִ�н��������*/
	)
{
	*pu16ErrorCode = ERROR_WI_NORMAL;
	_Init();
}

/*
DBG_UTILITY_Malloc
------------------
������������ָ����С�Ĵ洢����
Mark�������ڴ���ڵ���ϵͳ�ڲ�ע����������Ϣ��
��ϢΪһ��������������Ԫ��{����õ��ڴ�ĵ�ַ,�ڴ����򳤶�,
���뺯�����������ļ���,���뺯�������ļ����к�}��
NOTE��ͨ��C_Runtime Lib�Ķѹ���ģ��ʵ���Ի�ȡ�洢����
REF�� mallloc
------------------
Author��Martin
Date��  2011��7��26��
*/
void* DBG_UTILITY_Malloc(
	INT32U u32DesiredDataSize, /*[IN] ��Ҫ������ڴ��С*/
	CHAR* strFileName,         /*[IN] ���뷢�������ļ�*/
	INT32U u32LineNO           /*[IN] ���뷢�������ļ��е��к�*/
	)
{
	void* pRtnAddress = NULL;
	DBG_U_M_HANDLE hdlMeta = DBG_U_M_HANDLE_NULL;
	tMemHeader memHeader;
	//_Lock();
	/*S1:�����ڴ�*/
	pRtnAddress = _Malloc(u32DesiredDataSize);

	if (pRtnAddress)
	{
		/*S2:ע���ڴ���Ϣ*/
		hdlMeta = _RegistMem(strFileName,_GetDataZoneOfMem(pRtnAddress),u32LineNO,u32DesiredDataSize);
		if (hdlMeta == DBG_U_M_HANDLE_NULL)
		{
			//_Unlock();
			return NULL;
		}
		/*S3:Ƕ��memͷ��Ϣ*/
		memHeader.memHdl = hdlMeta;
		_EmbedMemHeader2Mem(pRtnAddress,&memHeader);
	}
	//_Unlock();

	return pRtnAddress==NULL?NULL:_GetDataZoneOfMem(pRtnAddress);
}
/*
DBG_UTILITY_Realloc
------------------
���������������ڴ�
Mark��
NOTE��ͨ��C_Runtime Lib�Ķѹ���ģ��ʵ���Ի�ȡ�洢����
REF�� reallloc
------------------
Author��Martin
Date��  2011��7��26��
*/
void* DBG_UTILITY_Realloc(
	void* pOrigMem,            /*[IN] ��Ҫ������ڴ��ַ*/   
	INT32U u32DesiredDataSize, /*[IN] ��Ҫ������ڴ��С*/
	CHAR* strFileName,         /*[IN] ���뷢�������ļ�*/
	INT32U u32LineNO           /*[IN] ���뷢�������ļ��е��к�*/
	)
{
	void* pRtnAddress = NULL;
	DBG_U_M_HANDLE hdlMeta = DBG_U_M_HANDLE_NULL;
	tMemHeader memHeader;
	/*S1:��ȡԭʼ�洢����ͷ*/
	//_Lock();
	memHeader = *_GetMemHeaderByDataZoneAddress(pOrigMem);
	/*S2:ȥ�����ڴ�ע��*/
	_UnregistMem(pOrigMem);
	/*S3:�����ڴ�*/
	pRtnAddress = _Realloc(pOrigMem,u32DesiredDataSize);

	if (pRtnAddress)
	{
		/*S2:ע���ڴ���Ϣ*/
		hdlMeta = _RegistMem(strFileName,_GetDataZoneOfMem(pRtnAddress),u32LineNO,u32DesiredDataSize);
		if (hdlMeta == DBG_U_M_HANDLE_NULL)
		{
			//_Unlock();
			return NULL;
		}
		/*S3:Ƕ��memͷ��Ϣ*/
		memHeader.memHdl = hdlMeta;
		_EmbedMemHeader2Mem(pRtnAddress,&memHeader);
	}
	//_Unlock();

	return pRtnAddress==NULL?NULL:_GetDataZoneOfMem(pRtnAddress);
}

/*
DBG_UTILITY_Free
------------------
�������ͷ�ָ���洢����
Mark������ָ����ַ��ѯ������ͷ��ڴ��������Ϣ��ͨ��C-Runtime Lib
�Ӷ����ͷŸ�����󽫸��ڴ�������Ϣ��ϵͳ��ȥ����
REF�� free
------------------
Author��Martin
Date��  2011��7��26��
*/
void DBG_UTILITY_Free(
	void* pNeedFreeMemAddress           /*[IN] ��Ҫ���ͷŵ��ڴ�����ַ*/
	)
{
	void* pMem = NULL;
	//_Lock();
	if (pNeedFreeMemAddress)
	{
		pMem = (void*)_GetMemHeaderByDataZoneAddress(pNeedFreeMemAddress);
		/*S1:��ע���ڴ���Ϣ*/
		_UnregistMem(pNeedFreeMemAddress);
		/*S2:�ͷ��ڴ�*/
		free(pMem);
	}	
	//_Unlock();
}
CHAR azStrStatisticInformation[MAX_REPORT_MSG_LEN];

/*
DBG_UTILITY_ReportStatistics
------------------
�����������ʹ��ͳ����Ϣ
Mark����ͳ����Ϣ��ӡ��ָ���豸��ͨ�����ϵ����Ҫʵ�ֻ�ָ��ȱʡ�ı����豸�ӿ�
REF�� 
------------------
Author��Martin
Date��  2011��7��26��
*/
void DBG_UTILITY_ReportStatistics(
	tIReportTargetOp    reportTargetOp       /*[IN] ����Ŀ��������ӿ�*/
	)
{
	DBG_U_M_HANDLE hdlMeta = DBG_U_M_HANDLE_NULL;
	//SYSTEMTIME  timeinfo;
	INT32U u32Index = 0;
	//GetSystemTime(&timeinfo);
	/*S1:ͳ����Ϣ*/
	_Statistic();
	/*S2:��֯��ӡ��Ϣ*/
	//sprintf(azStrStatisticInformation,"==============%4d-%02d-%02d %02d:%02d:%02d\n=======Statistics\r\n",1900+timeinfo->tm_year, 1+timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    //sprintf(azStrStatisticInformation,"==============%4d-%02d-%02d %02d:%02d:%02d=======\r\n",
	//	timeinfo.wYear, timeinfo.wMonth,timeinfo.wDay,timeinfo.wHour+8,timeinfo.wMinute,timeinfo.wSecond);
	sprintf(&azStrStatisticInformation[strlen(azStrStatisticInformation)],"\t Current using memory size:%d\r\n",
		sgDBG_U_MemManager.statisticInfor.u32CurrentUsingMemSize);

	sprintf(&azStrStatisticInformation[strlen(azStrStatisticInformation)],"\t Current mem item cnt:%d\r\n",
		sgDBG_U_MemManager.statisticInfor.u32CurrentMemItemCnt);

	/*S2-2:��ӡ��ϸ��Ϣ*/
	if (sgDBG_U_MemManager.RegistedMemItemCnt !=0 && sgDBG_U_MemManager.hdlRegistedListHeader !=DBG_U_M_HANDLE_NULL)
	{
		hdlMeta = sgDBG_U_MemManager.hdlRegistedListHeader;
		while (hdlMeta != DBG_U_M_HANDLE_NULL)
		{
			sprintf(&azStrStatisticInformation[strlen(azStrStatisticInformation)],"====Item %d====\r\n",
				u32Index++);

			if (strlen(azStrStatisticInformation)>=MAX_REPORT_MSG_LEN)
			{
				return;
			}

			sprintf(&azStrStatisticInformation[strlen(azStrStatisticInformation)],"[.FileName:%s]\r\n[.LineNO:%d]\r\n[.Size:%d]\r\n[.Address:%x]\r\n",
				sgDBG_U_MemManager.azMemMetaTable[hdlMeta].azFileName,
				sgDBG_U_MemManager.azMemMetaTable[hdlMeta].u32LineNO,
				sgDBG_U_MemManager.azMemMetaTable[hdlMeta].u32MemSize,
				sgDBG_U_MemManager.azMemMetaTable[hdlMeta].pAddress);
			if (strlen(azStrStatisticInformation)>=MAX_REPORT_MSG_LEN)
			{
				return;
			}

			hdlMeta = sgDBG_U_MemManager.azMemMetaTable[hdlMeta].listNode.NextNode;
		}
	}

	reportTargetOp.fnPrint(azStrStatisticInformation);

}


/*
DBG_UTILITY_Clear
------------------
���������DBG_Malloc������ͳ����Ϣ��
Mark��������������ڴ浥Ԫ��ע���б��������
CAUTION�����ñ�������Free�����ڱ�����ǰͨ��Malloc���õ��ڴ浥Ԫ������������Σ�գ�
REF�� None
------------------
Author��Martin
Date��  2011��8��3��
*/
void DBG_UTILITY_Clear()
{
	_Init();//�� ��ʱʹ�ú���init������������������в�ǡ�������ķ��գ���	
}

#endif
