//////////////////////////////////////////////////////////////////////////
//�ļ���                 OSAdapter.c
//����������ϵͳ������ģ��ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��11��	ģ�鴴��               zsq


#include "OSAdapter.h"


/***************************************---��tOSAdapter���������� BEGIN---********************************/

static tEnumOSCallResult  _OSAMailboxPend_Dumy(tPOSAdapter pOSAdapter,tPMsgObj pMsg,tOSTimeout timeoutInMicsec);
static tEnumOSCallResult  _OSAMailboxPost_Dumy(tPOSAdapter pOSAdapter,tPMsgObj pMsg,tOSTimeout timeoutInMicsec);


static BOOL _OSALock_Dumy(tPOSAdapter pOSAdapter);
static BOOL _OSAUnLock_Dumy(tPOSAdapter pOSAdapter);

//static void _OSASleep_Dumy(INT32U u32MicSec);
//static void _OSATerminate_Dumy(tPOSAdapter pOSAdapter);
static void _OSAStart_Dumy(tPOSAdapter pOSAdapter);
//static void _OSAPostEvent_Dumy(tPOSAdapter pOSAdapter,tThreadEventID u8Event);
static tThreadEventID  _OSAWaiteForMsg_Dumy(
					tPOSAdapter pOSAdapter,
					tThreadEventID u8AndEventIDMask,
					tThreadEventID u8OrEventIDMask,
					tOSTimeout timeOut
					);

static void _OSAFree_Dumy(tPObject  pObject);
/***************************************---��tOSAdapter���������� End---**********************************/





/***************************************---��tOSAdapter������ʵ�� BEGIN---********************************/
/*
AdapterParam_Init
-------------------
������AdapterParam�ĳ�ʼ������
NOTE������ʹ�øò����ĳ��ϱ�������ʹ�ñ��������ж�Param�ĳ�ʼ��
REF��
------------------
Author��Martin��Lee
Date��2013��5��22��
*/
void AdapterParam_Init(tPAdapterParam pParam)
{
	if (pParam != NULL)
	{
		pParam->pBelongingThread = NULL;
		pParam->threadParam.arg = NULL;
		pParam->threadParam.bAutoRun = FALSE;
		pParam->threadParam.fnThreadMain = NULL;
		pParam->threadParam.priority = -1;
		pParam->threadParam.pStack = NULL;
		pParam->threadParam.stackSize = 0;
		pParam->threadParam.u32MessagePumpTimeout = 0;
	}
}
/*
OSAdapterObject_Create
-------------------
����������ϵͳ������ģ����Create����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��10��
*/	

INT32S OSAdapterObject_Create(tPOSAdapter* ppOSAdapterObject)
{
//	OBJECT_MALLOC_DECLARE();
	if (ppOSAdapterObject == NULL)
	{
		return -1;
	}
	
	/*S1:Ϊ���������ڴ�ռ�*/
	(*ppOSAdapterObject) =  OBJECT_MALLOC(tOSAdapter,(*ppOSAdapterObject));
	if(*ppOSAdapterObject == NULL)
	{
		//���ش�����
		return -1;
	}
	
	/*S2:ִ�и��๹�������ѻ�öԸ���Ĵ�������ʼ�������ļ̳�*/
	Object_Create((tPObject*)ppOSAdapterObject);	
	/*S3:��ʼ�������Լ��ķ���*/
	(*ppOSAdapterObject)->fnStart = _OSAStart_Dumy;
//	(*ppOSAdapterObject)->fnTerminate = _OSATerminate_Dumy;
//	(*ppOSAdapterObject)->fnSleep = _OSASleep_Dumy;
	(*ppOSAdapterObject)->fnMailboxPend = _OSAMailboxPend_Dumy;
	(*ppOSAdapterObject)->fnMailboxPost = _OSAMailboxPost_Dumy;
//	(*ppOSAdapterObject)->fnSemaphorePend = _OSASemaphorePend_Dumy;
//	(*ppOSAdapterObject)->fnSemaphorePost = _OSASemaphorePost_Dumy;
	(*ppOSAdapterObject)->fnLock = _OSALock_Dumy;
	(*ppOSAdapterObject)->fnUnLock = _OSAUnLock_Dumy;
	(*ppOSAdapterObject)->fnOSAWaiteForMsg = _OSAWaiteForMsg_Dumy;
	/*S4:ע�������free����*/
	(*ppOSAdapterObject)->baseObj.Size = sizeof(tOSAdapter);
	(*ppOSAdapterObject)->baseObj.fnOverideFree(&((*ppOSAdapterObject)->baseObj),_OSAFree_Dumy);
	return 0;
}









/*
_OSAMailboxPend_Dumy
------------------
������ϵͳ�������ʼ���Ϣ������Ԫ����	
NOTE�����ڳ�ʼ��ϵͳ�������ʼ���Ϣ���շ���
REF�� OSAdapterPakage.h��tOSAdapter����
------------------
Author��zsq
Date��  2012��12��13
*/
static tEnumOSCallResult  _OSAMailboxPend_Dumy(tPOSAdapter pOSAdapter,tPMsgObj pMsg,tOSTimeout timeoutInMicsec)
{
	return ENUM_SUCCESS;
}



/*
_OSAMailboxPost_Dumy
-------------------
������ϵͳ������Mailbox���Ͳ�����Ԫ����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��10��
*/	
static tEnumOSCallResult  _OSAMailboxPost_Dumy(tPOSAdapter pOSAdapter,tPMsgObj pMsg,tOSTimeout timeoutInMicsec)
{
	return ENUM_SUCCESS;
}

/*
_OSASemaphorePend_Dumy
-------------------
������ϵͳ������Semaphore���ղ�����Ԫ����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��10��
*/	
//static tEnumOSCallResult  _OSASemaphorePend_Dumy(tPOSAdapter pOSAdapter,tOSTimeout timeoutInMicsec)
//{
//	return ENUM_SUCCESS;
//}

/*
_OSASemaphorePost_Dumy
-------------------
������ϵͳ������Semaphore���Ͳ�����Ԫ����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��10��
*/	
//static tEnumOSCallResult  _OSASemaphorePost_Dumy(tPOSAdapter pOSAdapter,INT8U u8PostCnt,tOSTimeout timeoutInMicsec)
//{
//	return ENUM_SUCCESS;
//}


/*
_OSALock_Dumy
-------------------
������ϵͳ������������������Ԫ����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��10��
*/	
static BOOL _OSALock_Dumy(tPOSAdapter pOSAdapter)
{
	return TRUE;
}

/*
_OSAUnLock_Dumy
-------------------
������ϵͳ��������ӳ�����������Ԫ����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��10��
*/	
static BOOL _OSAUnLock_Dumy(tPOSAdapter pOSAdapter)
{
	return TRUE;
}


/*
_OSASleep_Dumy
-------------------
������ϵͳ�������߳����߲�����Ԫ����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��10��
*/
//static void _OSASleep_Dumy(INT32U u32MicSec)
//{
//	//do nothing
//}


/*
_OSATerminate_Dumy
-------------------
������ϵͳ�������߳��ս������Ԫ����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��10��
*/
//static void _OSATerminate_Dumy(tPOSAdapter pOSAdapter)
//{
//	//do nothing
//}


/*
_OSAStart_Dumy
-------------------
������ϵͳ�������߳����в�����Ԫ����
NOTE����TIBIOS���ԣ�_OSAStartֻ��ִ��һ�Σ����������һ��Thread�̣߳�
	  ִ��_OSAStart����ʱ����BIOS���Ѿ��������������ʴ��������Thread
	  �̲߳���Ҫ��ִ�д���Ĳ���
REF��
------------------
Author��zsq
Date��2012��12��10��
*/
static void _OSAStart_Dumy(tPOSAdapter pOSAdapter)
{
	//do nothing
}



/*
_OSAWaiteForMsg_Dumy
-------------------
������ϵͳ�������̵߳ȴ���ϢOR�¼�������Ԫ����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��10��
*/
static tThreadEventID  _OSAWaiteForMsg_Dumy(
	tPOSAdapter pOSAdapter,
	tThreadEventID u8AndEventIDMask,
	tThreadEventID u8OrEventIDMask,
	tOSTimeout timeOut
	)
{
	return 0;
}

void  _OSAFree_Dumy(tPObject  pObject)
{
	//do nothing
}
/*
_OSAPostEvent_Dumy
-------------------
������ϵͳ�������̷߳����¼���Ԫ����
NOTE��
REF��
------------------
Author��zsq
Date��2013��1��6��
*/
//static void _OSAPostEvent_Dumy(tPOSAdapter pOSAdapter,tThreadEventID u8Event)
//{
//	//do nothing
//}

/***************************************---��tOSAdapter������ʵ�� END---***********************************/
