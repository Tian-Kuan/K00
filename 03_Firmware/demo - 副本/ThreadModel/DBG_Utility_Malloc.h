//////////////////////////////////////////////////////////////////////////
//�ļ���                 DBG_Utility_Malloc.h
//���������Թ���ģ��֮���ڴ�����ģ��ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2011��7��26��	ģ�鴴��               Martin
// 2013��1��18��    ������ֲ               ������



#ifndef __HH_DBG_UTILITY_MALLOC_H_HH__
#define __HH_DBG_UTILITY_MALLOC_H_HH__


#include "BaseType.h"
#include "stdlib.h"

#if (0) //TARGETOS==X86
#ifndef DBG_UTILITY_WINAPI 
#ifdef  DBG_UTILITY_DLL
#define DBG_UTILITY_WINAPI  _declspec(dllexport) 
#else
#define DBG_UTILITY_WINAPI  _declspec(dllimport)
#endif
#endif
#else
#ifndef DBG_UTILITY_WINAPI
#define DBG_UTILITY_WINAPI
#endif
#endif


#define MALLOC(size)   malloc(size)						//malloc(size)     malloc_x(size,__FILE__,__LINE__)

#define FREE(pData)    free(pData)						//free(pData)   free_x(pData,__FILE__,__LINE__)



//////////////////////////////////////////////////////////////////////////
//Macros declaration begin from here..
/*
MAX_MALOCC_ITEM_CNT
------------------
�����������������Ŀ��
Mark��ϵͳ�����Ѵ��ڸú궨������Ŀ���ٴ����뽫ʧ��
NOTE��none
REF�� 
------------------
Author��Martin
Date��  2011��7��26��
*/
#define MAX_MALOCC_ITEM_CNT    10
#define MAX_REPORT_MSG_LEN     1

//Macros declaration ended here.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Types declaration begin from here..

/*
tIReportTargetOp
------------------
����������Ŀ�������
Mark��ָ�����������󼴿ɴ�ӡ��Ϣ��ָ���豸
NOTE��none
REF�� 
------------------
Author��Martin
Date��  2011��7��26��
*/
typedef void (*tfnPrint) (CHAR* strMsg);
typedef struct  
{
	tfnPrint fnPrint;
}tIReportTargetOp;


//Types declaration ended here.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Operations declaration begin from here..

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
DBG_UTILITY_WINAPI void DBG_UTILITY_M_Init(
	PINT16U pu16ErrorCode           /*[OUT] ���صĲ���ִ�н��������*/
);

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
DBG_UTILITY_WINAPI void* DBG_UTILITY_Malloc(
	INT32U u32DesiredDataSize, /*[IN] ��Ҫ������ڴ��С*/
	CHAR* strFileName,         /*[IN] ���뷢�������ļ�*/
	INT32U u32LineNO           /*[IN] ���뷢�������ļ��е��к�*/
);
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
DBG_UTILITY_WINAPI void* DBG_UTILITY_Realloc(
	void* pOrigMem,            /*[IN] ��Ҫ������ڴ��ַ*/   
	INT32U u32DesiredDataSize, /*[IN] ��Ҫ������ڴ��С*/
	CHAR* strFileName,         /*[IN] ���뷢�������ļ�*/
	INT32U u32LineNO           /*[IN] ���뷢�������ļ��е��к�*/
);
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
DBG_UTILITY_WINAPI void DBG_UTILITY_Free(
	void* pNeedFreeMemAddress           /*[IN] ��Ҫ���ͷŵ��ڴ�����ַ*/
);


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
DBG_UTILITY_WINAPI void DBG_UTILITY_ReportStatistics(
	tIReportTargetOp    reportTargetOp       /*[IN] ����Ŀ��������ӿ�*/
);


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
DBG_UTILITY_WINAPI void DBG_UTILITY_Clear(
	void
);

//Operations declaration ended here.
//////////////////////////////////////////////////////////////////////////
//INT32U GetSP(void);
void* malloc_x(
   INT32U u32DesiredDataSize,
   CHAR* strFileName,         /*[IN] ���뷢�������ļ�*/
	INT32U u32LineNO           /*[IN] ���뷢�������ļ��е��к�*/
	);
void free_x(
   void* pMem,
   CHAR* strFileName,         /*[IN] ���뷢�������ļ�*/
	 INT32U u32LineNO           /*[IN] ���뷢�������ļ��е��к�*/
	);


   
#define REALLOC(pOrigMem,size)  realloc(pOrigMem,size)
#endif

