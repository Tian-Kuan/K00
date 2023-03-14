//////////////////////////////////////////////////////////////////////////
//文件名                 DBG_Utility_Malloc.c
//描述：调试工具模块之堆内存申请模块实现文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2011年7月26日	模块创建               Martin
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
   CHAR* strFileName,         /*[IN] 申请发生所在文件*/
	INT32U u32LineNO           /*[IN] 申请发生所在文件中的行号*/
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
   CHAR* strFileName,         /*[IN] 申请发生所在文件*/
	 INT32U u32LineNO           /*[IN] 申请发生所在文件中的行号*/
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
	/*S1:等待互斥对象*/
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

	/*S1:初始化内存元信息表*/
	/*S1-1:头结点初始化*/
	sgDBG_U_MemManager.azMemMetaTable[0].azFileName[0]='\0';
	sgDBG_U_MemManager.azMemMetaTable[0].hdl = 0;
	sgDBG_U_MemManager.azMemMetaTable[0].pAddress = NULL;
	sgDBG_U_MemManager.azMemMetaTable[0].u32LineNO = 0;
	sgDBG_U_MemManager.azMemMetaTable[0].u32MemSize = 0;
	sgDBG_U_MemManager.azMemMetaTable[0].listNode.preNode  = DBG_U_M_HANDLE_NULL;
	sgDBG_U_MemManager.azMemMetaTable[0].listNode.NextNode = 1;

	/*S1-2:除去头尾节点后的节点初始化*/
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

	/*S1-3:尾结点初始化*/
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].azFileName[0]='\0';
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].hdl = MAX_MALOCC_ITEM_CNT-1;
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].pAddress = NULL;
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].u32LineNO = 0;
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].u32MemSize = 0;
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].listNode.preNode  = MAX_MALOCC_ITEM_CNT-2;
	sgDBG_U_MemManager.azMemMetaTable[MAX_MALOCC_ITEM_CNT-1].listNode.NextNode = DBG_U_M_HANDLE_NULL;


	/*S2:初始化"使用"与"未使用"列表描述*/
	sgDBG_U_MemManager.hdlRegistedListHeader = DBG_U_M_HANDLE_NULL;//← 使用列表指向空
	sgDBG_U_MemManager.RegistedMemItemCnt = 0;
	sgDBG_U_MemManager.hdlEmptyListHeader = 0;//← 未使用列表指向表中索引0处节点
	/*S3:统计数据初始化*/
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
	/*S1:获得内存头描述信息*/
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
		/*S1:从"未使用"表中获取一个节点*/
		rtnHdl = sgDBG_U_MemManager.hdlEmptyListHeader;
		/*S2:调整"未使用"表中头结点描述*/
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
		/*S1:修正归还节点信息*/
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
		/*S2:调整"未使用"列表描述信息*/
		sgDBG_U_MemManager.hdlEmptyListHeader = hdl;
	}
}
void _AddNode2RegestedList(DBG_U_M_HANDLE hdlMetaNode)
{
	if (hdlMetaNode!=DBG_U_M_HANDLE_NULL)
	{
		/*S1:修正节点信息*/
		if (sgDBG_U_MemManager.hdlRegistedListHeader != DBG_U_M_HANDLE_NULL)
		{
			sgDBG_U_MemManager.azMemMetaTable[sgDBG_U_MemManager.hdlRegistedListHeader].listNode.preNode = hdlMetaNode;
		}
		
		sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.NextNode = sgDBG_U_MemManager.hdlRegistedListHeader;
		sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.preNode = DBG_U_M_HANDLE_NULL;
		/*S2:调整"未使用"列表描述信息*/
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
	/*S1:获得一个元描述节点*/
	hdlMetaNode = _ApplyNewRegistMetaNode();
	if (hdlMetaNode!=DBG_U_M_HANDLE_NULL)
	{
		/*S2:填写元描述信息到节点*/
		STR_CPY(sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].azFileName,strFileName);
		sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].pAddress = pDataZoneAddress;
		sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].u32LineNO = u32LineNO;
		sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].u32MemSize = u32MemSize;
		/*S3:将描述节点投入"使用"列表*/
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
		
		/*S1:若为头结点则调整列表描述信息*/
		if (hdlMetaNode == sgDBG_U_MemManager.hdlRegistedListHeader)
		{
			sgDBG_U_MemManager.hdlRegistedListHeader = sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.NextNode;
		}
		/*S2:调整节点前后向节点信息*/
		/*S2-1:获得节点的前后向节点*/
		pPreNode = (sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.preNode==DBG_U_M_HANDLE_NULL)?&tmpNodeForFakePre:
			(tPListNode)&sgDBG_U_MemManager.azMemMetaTable[sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.preNode];
		pNextNode = (sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.NextNode==DBG_U_M_HANDLE_NULL)?&tmpNodeForFakeNext:
			(tPListNode)&sgDBG_U_MemManager.azMemMetaTable[sgDBG_U_MemManager.azMemMetaTable[hdlMetaNode].listNode.NextNode];
		/*S2-2:调整前后向节点信息*/
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
		/*S1:获取元描述节点*/
		hdlMetaNode = _FindNodeByAddress(pDataZoneAddress);

		if (hdlMetaNode != DBG_U_M_HANDLE_NULL)
		{
			/*S2:将该节点从"使用"列表中去除*/
			_RemoveNodeFromRegistedList(hdlMetaNode);
			/*S3:将该节点投入"未使用"列表*/
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
	/*S1:遍历"使用"列表获取当前使用内存大小及内存条目数*/
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
描述：调试模块之内存管理模块的初始化
Mark：未经初始化本模块处于未定义的状态不可使用！
REF： 
------------------
Author：Martin
Date：  2011年7月26日
*/
void DBG_UTILITY_M_Init(
	PINT16U pu16ErrorCode           /*[OUT] 返回的操作执行结果错误码*/
	)
{
	*pu16ErrorCode = ERROR_WI_NORMAL;
	_Init();
}

/*
DBG_UTILITY_Malloc
------------------
描述：申请获得指定大小的存储区域
Mark：申请内存后即在调试系统内部注册该申请的信息。
信息为一个堆申请描述三元组{所获得的内存的地址,内存区域长度,
申请函数调用所在文件名,申请函数所在文件的行号}。
NOTE：通过C_Runtime Lib的堆管理模块实质性获取存储区域
REF： mallloc
------------------
Author：Martin
Date：  2011年7月26日
*/
void* DBG_UTILITY_Malloc(
	INT32U u32DesiredDataSize, /*[IN] 需要申请的内存大小*/
	CHAR* strFileName,         /*[IN] 申请发生所在文件*/
	INT32U u32LineNO           /*[IN] 申请发生所在文件中的行号*/
	)
{
	void* pRtnAddress = NULL;
	DBG_U_M_HANDLE hdlMeta = DBG_U_M_HANDLE_NULL;
	tMemHeader memHeader;
	//_Lock();
	/*S1:申请内存*/
	pRtnAddress = _Malloc(u32DesiredDataSize);

	if (pRtnAddress)
	{
		/*S2:注册内存信息*/
		hdlMeta = _RegistMem(strFileName,_GetDataZoneOfMem(pRtnAddress),u32LineNO,u32DesiredDataSize);
		if (hdlMeta == DBG_U_M_HANDLE_NULL)
		{
			//_Unlock();
			return NULL;
		}
		/*S3:嵌入mem头信息*/
		memHeader.memHdl = hdlMeta;
		_EmbedMemHeader2Mem(pRtnAddress,&memHeader);
	}
	//_Unlock();

	return pRtnAddress==NULL?NULL:_GetDataZoneOfMem(pRtnAddress);
}
/*
DBG_UTILITY_Realloc
------------------
描述：重新申请内存
Mark：
NOTE：通过C_Runtime Lib的堆管理模块实质性获取存储区域
REF： reallloc
------------------
Author：Martin
Date：  2011年7月26日
*/
void* DBG_UTILITY_Realloc(
	void* pOrigMem,            /*[IN] 需要申请的内存地址*/   
	INT32U u32DesiredDataSize, /*[IN] 需要申请的内存大小*/
	CHAR* strFileName,         /*[IN] 申请发生所在文件*/
	INT32U u32LineNO           /*[IN] 申请发生所在文件中的行号*/
	)
{
	void* pRtnAddress = NULL;
	DBG_U_M_HANDLE hdlMeta = DBG_U_M_HANDLE_NULL;
	tMemHeader memHeader;
	/*S1:获取原始存储区得头*/
	//_Lock();
	memHeader = *_GetMemHeaderByDataZoneAddress(pOrigMem);
	/*S2:去除该内存注册*/
	_UnregistMem(pOrigMem);
	/*S3:申请内存*/
	pRtnAddress = _Realloc(pOrigMem,u32DesiredDataSize);

	if (pRtnAddress)
	{
		/*S2:注册内存信息*/
		hdlMeta = _RegistMem(strFileName,_GetDataZoneOfMem(pRtnAddress),u32LineNO,u32DesiredDataSize);
		if (hdlMeta == DBG_U_M_HANDLE_NULL)
		{
			//_Unlock();
			return NULL;
		}
		/*S3:嵌入mem头信息*/
		memHeader.memHdl = hdlMeta;
		_EmbedMemHeader2Mem(pRtnAddress,&memHeader);
	}
	//_Unlock();

	return pRtnAddress==NULL?NULL:_GetDataZoneOfMem(pRtnAddress);
}

/*
DBG_UTILITY_Free
------------------
描述：释放指定存储区域
Mark：根据指定地址查询获得所释放内存的描述信息。通过C-Runtime Lib
从堆中释放该区域后将该内存描述信息从系统中去除。
REF： free
------------------
Author：Martin
Date：  2011年7月26日
*/
void DBG_UTILITY_Free(
	void* pNeedFreeMemAddress           /*[IN] 需要被释放的内存区地址*/
	)
{
	void* pMem = NULL;
	//_Lock();
	if (pNeedFreeMemAddress)
	{
		pMem = (void*)_GetMemHeaderByDataZoneAddress(pNeedFreeMemAddress);
		/*S1:反注册内存信息*/
		_UnregistMem(pNeedFreeMemAddress);
		/*S2:释放内存*/
		free(pMem);
	}	
	//_Unlock();
}
CHAR azStrStatisticInformation[MAX_REPORT_MSG_LEN];

/*
DBG_UTILITY_ReportStatistics
------------------
描述：报告堆使用统计信息
Mark：将统计信息打印到指定设备。通常情况系您需要实现或指定缺省的报告设备接口
REF： 
------------------
Author：Martin
Date：  2011年7月26日
*/
void DBG_UTILITY_ReportStatistics(
	tIReportTargetOp    reportTargetOp       /*[IN] 报告目标机操作接口*/
	)
{
	DBG_U_M_HANDLE hdlMeta = DBG_U_M_HANDLE_NULL;
	//SYSTEMTIME  timeinfo;
	INT32U u32Index = 0;
	//GetSystemTime(&timeinfo);
	/*S1:统计信息*/
	_Statistic();
	/*S2:组织打印信息*/
	//sprintf(azStrStatisticInformation,"==============%4d-%02d-%02d %02d:%02d:%02d\n=======Statistics\r\n",1900+timeinfo->tm_year, 1+timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    //sprintf(azStrStatisticInformation,"==============%4d-%02d-%02d %02d:%02d:%02d=======\r\n",
	//	timeinfo.wYear, timeinfo.wMonth,timeinfo.wDay,timeinfo.wHour+8,timeinfo.wMinute,timeinfo.wSecond);
	sprintf(&azStrStatisticInformation[strlen(azStrStatisticInformation)],"\t Current using memory size:%d\r\n",
		sgDBG_U_MemManager.statisticInfor.u32CurrentUsingMemSize);

	sprintf(&azStrStatisticInformation[strlen(azStrStatisticInformation)],"\t Current mem item cnt:%d\r\n",
		sgDBG_U_MemManager.statisticInfor.u32CurrentMemItemCnt);

	/*S2-2:打印详细信息*/
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
描述：清除DBG_Malloc的所有统计信息。
Mark：所有申请过的内存单元从注册列表中清除。
CAUTION：调用本函数后Free发生于本函数前通过Malloc而得的内存单元将发生崩溃的危险！
REF： None
------------------
Author：Martin
Date：  2011年8月3日
*/
void DBG_UTILITY_Clear()
{
	_Init();//← 暂时使用函数init代替清除动作。（具有不恰当依赖的风险！）	
}

#endif
