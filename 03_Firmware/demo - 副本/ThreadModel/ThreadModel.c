//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                    ThreadModel.c
//������ ������߳�ģ��ʵ���ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��11��                  ģ�鴴��                zsq
//
#include <stdio.h>
#include <string.h>
#include "ThreadModel.h"
#include "RS232.h"
#include "led.h"
#include "IWDG.h"


/***************************************---��tThread���������� BEGIN---********************************/
static tEnumThreadWaiteMsgReturn _Thread_WaitForMsg(tPThread pThis,tPMsgObj pMsg,tOSTimeout timeoutInMicrosec);
//static void _Thread_Sleep(tPThread pThis,INT32U u32MicSec);
static void _Thread_Lock(tPThread pThis);
static BOOL _Thread_SendMessage(tPThread pThis,tPMsgObj pMsg,tOSTimeout timeoutInMicrosec);
//static void _Thread_Terminate(tPThread pThis);

static void _Thread_ThreadMain(tPThread pThis,tThreadFuncArg arg);

static void _Thread_Unlock(tPThread pThis);



static BOOL _ThreadOnTimeout(tPThread pThis);
static BOOL _ThreadOnEvent(tPThread pThis,tThreadEventID u8EventID);
static BOOL _ThreadOnMessage(tPThread pThis,tPMsgObj pMsg);
//static void _ThreadQuited(tPObject pObject,tPThread pThis);
static void _ThreadFree(tPObject pObject);
static void _Thread_PostEvent(tPThread pThis,tThreadEventID u8Event);
static void _Thread_Start(tPThread pThis);

static void _Thread_Stop(tPThread pThis);

static tEnumThreadWaitMsgEvtReturn _Thread_WaitForMsgOrEvent(
	tPThread pThis,
	tThreadEventID u8AndEventIDMask,
	tThreadEventID u8OrEventIDMask,
	tPThreadEventID pu8RtnEvent,
	tPMsgObj pMsg,
	tOSTimeout timeoutInMicrosec
);

static void _InnerThreadFree(tPObject pObject);
static void _ThreadChangeState(tPThread pThis,tEnumTHREADState threadState);
static tEnumTHREADState _ThreadGetState(tPThread pThis);
static BOOL _ThreadRequireFree(tPObject pObject);
static void _InnerMethod_ThreadQuitHandle(tPThread pThis);
static void _InnerMethod_ThreadDelayFreeOccur(tPObject pObject,INT32U u32Param);
/***************************************---��tThread���������� End---**********************************/



/***************************************---��tThread������ʵ��BEGIN---********************************/


/*
_Thread_WaitForMsg
-------------------
�������̵߳ȴ���Ϣ������
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��20��
*/
static tEnumThreadWaiteMsgReturn _Thread_WaitForMsg(tPThread pThis,tPMsgObj pMsg,tOSTimeout timeoutInMicrosec)
{
	
	INT32U u32EventID = 0;
	tEnumThreadWaiteMsgReturn retValue = ENUM_THREAD_WAITMSG_RETURN_FAILED ;

	/*S1:������֤*/
	if (pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnMailboxPend ==NULL)
	{
		return retValue;
	}
	/*S2:�ȴ���Ϣ����*/ 
	u32EventID = pThis->pOSAdpt->fnOSAWaiteForMsg(pThis->pOSAdpt,0,THREAD_EVENT_ID_00+THREAD_EVENT_ID_01,timeoutInMicrosec);
	if (u32EventID & THREAD_EVENT_ID_00)
	{
		pThis->pOSAdpt->fnMailboxPend(pThis->pOSAdpt,pMsg,0);
		retValue = ENUM_THREAD_WAITMSG_RETURN_SUCCESS;
	}
	else if (u32EventID & THREAD_EVENT_ID_01)
	{
		retValue = ENUM_THREAD_WAITMSG_RETURN_THREADTERMINATE;
	}
	else if (u32EventID == 0)
	{
		retValue = ENUM_THREAD_WAITMSG_RETURN_TIMEOUT ;
	}
	else
	{
		retValue = ENUM_THREAD_WAITMSG_RETURN_FAILED ;
	}

	return retValue;
}


/*
_Thread_Sleep
-------------------
�������߳����ߴ�����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��20��
*/
//static void _Thread_Sleep(tPThread pThis,INT32U u32MicSec)
//{
//	
////	if(pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnSleep == NULL)
////	{
////		//do nothing
////	}
////	else
////	{
////		if(pThis->state == ENUM_THRD_RUNNING)
////		{
////			pThis->pOSAdpt->fnSleep(u32MicSec);
////		}
////	}
//}


/*
_Thread_Lock
-------------------
�������߳��ڴ������������
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��20��
*/
static void _Thread_Lock(tPThread pThis)
{
	if(pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnLock == NULL)
	{
		// do nothing
	}
	else
	{
		pThis->pOSAdpt->fnLock(pThis->pOSAdpt);
	}
}

/*
_Thread_Unlock
-------------------
�������߳��ڴ���������������
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��20��
*/
static void _Thread_Unlock(tPThread pThis)
{
	if(pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnUnLock == NULL)
	{
		// do nothing
	}
	else
	{
		pThis->pOSAdpt->fnUnLock(pThis->pOSAdpt);
	}
}

/*
_Thread_SendMessage
-------------------
�������̷߳���Ϣ������
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��20��
*/
static BOOL _Thread_SendMessage(tPThread pThis,tPMsgObj pMsg,tOSTimeout timeoutInMicrosec)
{
	BOOL retValue = FALSE;
	tEnumOSCallResult enumOsCallRslt = ENUM_SUCCESS;
	/*S0:������֤*/
	if(pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnMailboxPost == NULL)
	{
		//����������
		return FALSE;
	}
	else
	{
		/*S1:ִ�з�����Ϣ����*/
		((tPThread)pThis)->fnLock(((tPThread)pThis));
		if(pThis->state == ENUM_THRD_RUNNING)
		{
			enumOsCallRslt =  pThis->pOSAdpt->fnMailboxPost(pThis->pOSAdpt,pMsg,timeoutInMicrosec);
			retValue = (enumOsCallRslt == ENUM_SUCCESS)?TRUE:FALSE;
		}
		((tPThread)pThis)->fnUnlock(((tPThread)pThis));
	}
	return retValue;
}


/*
_Thread_Terminate
-------------------
�������߳��սᴦ����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��20��
*/
//static void _Thread_Terminate(tPThread pThis)
//{
//	/*S0:�����ж�*/
////	if(pThis == NULL ||pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnTerminate == NULL)
////	{
////		//����������
////	}
////	else
////	{
////		/*S1:ִ���߳���ֹ����*/
////		if(pThis->state == ENUM_THRD_RUNNING)
////		{
////			pThis->pOSAdpt->fnTerminate(pThis->pOSAdpt);
////		}
////	}
//}


/*
  _Thread_ThreadMain
   -----------------
   ������ �߳��������������߳�ģ�͵���Ϣ�û��ơ�
          �߳̽��շ��͸��Լ�����Ϣ/�¼�/�˳��߳�/����ʱ�ź�
		  �������̶߳���Ĵ�������
		  Thread������ͨ��������Ϣ�������Ի�ö��źŴ���Ĳ�ͬ
		  ��Ϊ����չ��
  NOTE��
  REF��
  ------------------
  Author��Martin.lee,zsq
  Date��2012��12��20��
*/
static void _Thread_ThreadMain(tPThread pThis,tThreadFuncArg arg)
{
	BOOL bThreadQuitNeeded = FALSE;
	INT8U u8OrEventIDMask = THREAD_EVENT_ID_MAX;
	INT8U u8AndEventIDMask = 0;
	INT8U u8RtnEvent = 0;
	tMsgObj Message;
	//int  Index = 1;
	
	/*S1:������֤*/
    if ( pThis == NULL || pThis->fnWaitForMsgOrEvent == NULL)
    {
		return;
    }
	
	//DEBUG_PRINTF(MESSAGE_DEBUG," *[_Thread_ThreadMain] Entry In   \r\n");
	
	/*S2:����Ϣ���¼��ϵȴ�*/
    while(!bThreadQuitNeeded)
	{			
		tEnumThreadWaitMsgEvtReturn enumRtnMsgOrEvent = pThis->fnWaitForMsgOrEvent(
															pThis,
															u8AndEventIDMask,
															u8OrEventIDMask,
															&u8RtnEvent,
															&Message,
															pThis->u32MessagePumpTimeout
															);
		switch (enumRtnMsgOrEvent)
		{
		    case ENUM_THREAD_WAITMSGEVT_RETURN_MSG:
				{
					/*S2-1:���ȵ�����[��Ϣ]������[��Ϣ]*/
					if ( pThis->fnOnMessage != NULL )
					{
						pThis->fnOnMessage(pThis,&Message);
					}

					break;
				}
			case ENUM_THREAD_WAITMSGEVT_RETURN_EVT:
				{
					/*S2-2:���ȵ����ǡ��¼����������¼���*/
					if ( pThis->fnOnEvent != NULL )
					{
						pThis->fnOnEvent(pThis,u8RtnEvent);
					}
					break;
				}
			case ENUM_THREAD_WAITMSGEVT_RETURN_THREADTERMINATE:
				{
					/*S2-3:���ȵ����ǡ��˳����������˳���*/
					bThreadQuitNeeded = TRUE;
					/*S1:�л�״̬���˳�̬*/	
					_ThreadChangeState(pThis,ENUM_THRD_TERMINATING);					
					/*S3:���յ����߳��˳����������߳��˳���*/
					if(pThis->fnOnQuited.fnNotify != NULL
					&& pThis->fnOnQuited.pLisennerObj != NULL)
					{
						/*S3-1: �����߳��˳�֪ͨ���Դ˸���Ȥ�Ķ���*/
						pThis->fnOnQuited.fnNotify(pThis->fnOnQuited.pLisennerObj,pThis);
					}
					/*S4:�˳�ǰ״̬������ж�ִ��[�Ƴ�Free]*/
					/*S4-1:�ӳ������߳�*/
					_InnerMethod_ThreadQuitHandle(pThis);
					break;
				}
			case ENUM_THREAD_WAITMSGEVT_RETURN_TIMEOUT:
				{
					/*S2-4:����Ϣ�õȴ�����ʱ����������Ϣ�ó�ʱ��*/
					if (pThis->fnOnTimeout != NULL)
					{
						pThis->fnOnTimeout(pThis);
					}
					break;
				}
			default:
				break;
		}
		//Index = !Index;
//	LED3(Index);
		IWDG_Feed();      
	}
}

/*
_ThreadOnEvent
-------------------
��������Ϣ���¼��������
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��20��
*/	
static BOOL _ThreadOnEvent(tPThread pThis,tThreadEventID u8EventID)
{
	// do nothing
	return TRUE;
}



/*
_ThreadOnEvent
-------------------
��������Ϣ����Ϣ�������
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��20��
*/	
static BOOL _ThreadOnMessage(tPThread pThis,tPMsgObj pMsg)
{
	//do nothing
	return TRUE;
}



/*
_ThreadOnTimeout
-------------------
��������Ϣ�ó�ʱ�������
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��20��
*/	
static BOOL _ThreadOnTimeout(tPThread pThis)
{
	return TRUE;
}


/*
_ThreadFree
-------------------
�������߳��˳��������
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��20��
*/	
static void _ThreadFree(tPObject pObject)
{
//	tPThread pThis = (tPThread)pObject;
	
	///*S1:�ж��߳�״̬��ִ���Ƴ�Free*/
	//if (pThis != NULL)
	//{
	//	pThis->fnLock(pThis);
	//	if(pThis->state != ENUM_THRD_TERMINATED)
	//	{
	//		_InnerDelayFree(pThis);
	//	}
	//	pThis->fnUnlock(pThis);
	//}
	//else
	//{
	//	_InnerThreadFree(pObject);

	_InnerThreadFree(pObject);
}

/*
_InnerDelayFree
-------------------
�������߳��ӳ����ٴ������
NOTE��
REF��
------------------
Author��Martin.lee
Date��2013��1��14��
*/	
//static void _InnerDelayFree(tPThread pThis)
//{
//	if (pThis != NULL && pThis->state != ENUM_THRD_TERMINATED)
//	{
//		pThis->bNeedFree = TRUE;
//	}
//}

/*
_InnerThreadFree
-------------------
�������߳��ڲ����ٴ������
NOTE��
REF��
------------------
Author��Martin.lee
Date��2013��1��14��
*/	
static void _InnerThreadFree(tPObject pObject)
{
	tPThread pThis = (tPThread)pObject;

	/*S1:�жϲ���������������*/
	if (pThis != NULL && pThis->pOSAdpt != NULL)
	{
		pThis->pOSAdpt->baseObj.fnFree((tPObject)pThis->pOSAdpt);
	}
}

/*
_Thread_WaitForEvent
-------------------
�������߳��¼��������
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��11��
*/
//static tEnumThreadWaitEvtReturn _Thread_WaitForEvent(
//		tPThread pThis,
//		tThreadEventID u8OrEventIDMask,
//		tThreadEventID u8AndEventIDMask,
//		tPThreadEventID pu8RtnEvent,
//		tOSTimeout timeoutInMicrosec
//		)
//{
//	return ENUM_THREAD_WAITEVT_RETURN_EVT;
//}
/*
_Thread_WaitForMsgOrEvent
-------------------
�������̵߳ȴ��¼�OR��Ϣ�������
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��11��
*/
static tEnumThreadWaitMsgEvtReturn _Thread_WaitForMsgOrEvent(
	tPThread pThis,
	tThreadEventID u8AndEventIDMask,
	tThreadEventID u8OrEventIDMask,
	tPThreadEventID pu8RtnEvent,
	tPMsgObj pMsg,
	tOSTimeout timeoutInMicrosec
	)
{
	INT32U u32EventID = 0;
	tEnumThreadWaitMsgEvtReturn retValue = ENUM_THREAD_WAITMSGEVT_RETURN_FAILED;

	/*S1:������֤*/
	if (pThis == NULL ||pThis->pOSAdpt == NULL ||pThis->pOSAdpt->fnMailboxPend == NULL)
	{
		return retValue;
	}
	/*S2:�ȴ���Ϣ����*/ 
	u32EventID = pThis->pOSAdpt->fnOSAWaiteForMsg(pThis->pOSAdpt,u8AndEventIDMask,u8OrEventIDMask,timeoutInMicrosec);

	if((u32EventID & THREAD_EVENT_ID_00) != 0)  //�� �յ���Ϣ
	{
		pThis->pOSAdpt->fnMailboxPend(pThis->pOSAdpt,pMsg,timeoutInMicrosec);
		retValue = ENUM_THREAD_WAITMSGEVT_RETURN_MSG;
		*pu8RtnEvent = THREAD_EVENT_ID_NONE;
		
	}
	else if ((u32EventID & THREAD_EVENT_ID_01) != 0)
	{
		retValue = ENUM_THREAD_WAITMSGEVT_RETURN_THREADTERMINATE;
		*pu8RtnEvent = THREAD_EVENT_ID_NONE;
	}
	else if (u32EventID == 0)
	{
		retValue = ENUM_THREAD_WAITMSGEVT_RETURN_TIMEOUT ;
		*pu8RtnEvent = THREAD_EVENT_ID_NONE;
	}
	else if (u32EventID <= THREAD_EVENT_ID_MAX)
	{
		retValue = ENUM_THREAD_WAITMSGEVT_RETURN_EVT;
		//*pu8RtnEvent = THREAD_EVENT_ID_NONE;
		*pu8RtnEvent = (INT8U)u32EventID;
	}
	else
	{
		retValue = ENUM_THREAD_WAITMSGEVT_RETURN_FAILED ;
		*pu8RtnEvent = THREAD_EVENT_ID_NONE;
	}
	return retValue;
}

/*
_ThreadQuited
-------------------
�������߳��˳��������
NOTE��
REF��
------------------
Author��zsq
Date��2013��1��4��
*/

//static void _ThreadQuited(tPObject pObject,tPThread pThis)
//{
//	//do nothing
//}

/*
_Thread_PostEvent
-------------------
�������߳��¼����ʹ������
NOTE��
REF��
------------------
Author��zsq
Date��2013��1��4��
*/
static void _Thread_PostEvent(tPThread pThis,tThreadEventID u8Event)
{
	if(pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnPostEvent == NULL)
	{
		//����������
	}
	else
	{
		/*S1:ִ���߳���ֹ����*/
		//if(THREAD_STAT_IS_RUNNING(pThis))
		{
			pThis->pOSAdpt->fnPostEvent(pThis->pOSAdpt,u8Event);
		}
	}
}




/*
_Thread_Start
-------------------
�������߳������������
NOTE��
REF��
------------------
Author��zsq
Date��2013��1��4��
*/
static void _Thread_Start(tPThread pThis)
{
	/*S0:�����жϼ�������*/
	if(pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnStart == NULL)
	{
		//����������
		return;
	}
	else
	{
		/*S2:�����̣߳�ʵ������ִ��UnLock����*/
		pThis->pOSAdpt->fnStart(pThis->pOSAdpt);
		/*S1:�����̵߳�����״̬ΪENUM_THRD_RUNNING*/
		_ThreadChangeState(pThis,ENUM_THRD_RUNNING);
	}
}

/*
_Thread_Stop
-------------------
�������߳�ֹͣ�������
NOTE��
REF��
------------------
Author��zsq
Date��2013��1��7��
*/
static void _Thread_Stop(tPThread pThis)
{
	/*S0:�����жϼ�������*/
	if(pThis == NULL || pThis->fnPostEvent == NULL)
	{
		//����������
	}
	else
	{
		((tPThread)pThis)->fnLock(((tPThread)pThis));
		if(THREAD_STAT_IS_RUNNING(pThis))
		{			
			pThis->fnPostEvent(pThis,THREAD_EVENT_ID_01);			
			_ThreadChangeState(pThis,ENUM_THRD_TERMINATING);
		}
		((tPThread)pThis)->fnUnlock(((tPThread)pThis));
	}
}



/*
_ThreadQuitedBind
-------------------
�������߳��˳��󶨴������
NOTE��
REF��
------------------
Author��zsq
Date��2013��1��7��
*/
//static void _ThreadQuitedBind(tPThread pThis,tfnOnThreadQuited fnOnThreadQuited)
//{
//	if(pThis == NULL 
//		|| fnOnThreadQuited.fnNotify == NULL
//		|| fnOnThreadQuited.pLisennerObj == NULL
//	)
//	{
//		return;
//	}
//	memcpy(&(pThis->fnOnQuited),&fnOnThreadQuited,sizeof(tfnOnThreadQuited));
//}

/*
_ThreadRequireFree
-------------------
�������̶߳���Ҫ���˳�����
NOTE��
REF��
------------------
Author��Martin.lee
Date��2013��1��14��
*/	
static BOOL _ThreadRequireFree(tPObject pObject)
{
	THREAD_CAST(pObject,pThis);
	/*S0:������֤*/
	if(pObject == NULL)
	{
		return FALSE;
	}
	
	/*S1:����*/
	pThis->fnLock(pThis);
	/*S2:�ж��Ƿ�������״̬���������Ƴ�����*/
	if(THREAD_STAT_IS_RUNNING(pThis))
	{
		/*S1-1:��������״̬��ֹͣ���̲߳���ʶΪ�Ƴ�����*/
		pThis->fnStop(pThis);
		pThis->baseObj.bNeedDelayFree = TRUE;
		/*S1-1-1:�����û���չ�����ٷ������ɱ��������Ƴ����ٷ���*/
		pThis->baseObj.fnoDelayFree.pFreeObj = pObject;
		pThis->baseObj.fnoDelayFree.fnFree = _InnerMethod_ThreadDelayFreeOccur;
	}
	else
	{
		/*S1-2:��ʶΪ����Ҫ�Ƴ�����*/
		pThis->baseObj.bNeedDelayFree = FALSE;
	}
	/*S3:����*/
	pThis->fnUnlock(pThis);
	return (pThis->baseObj.bNeedDelayFree == TRUE)?FALSE:TRUE;
}

/*
_ThreadChangeState
-------------------
�����������߳�����״̬
NOTE��
REF��
------------------
Author��zsq
Date��2013��1��14��
*/
static void _ThreadChangeState(tPThread pThis,tEnumTHREADState threadState)
{
	/*S0:�����ж�*/
	if(pThis != NULL)
	{
		/*S1:����*/
		pThis->fnLock(pThis);
		/*S2:�����̵߳�����״̬*/
		pThis->state = threadState;
		/*S3:����*/
		pThis->fnUnlock(pThis);
	}

}

/*
_ThreadGetState
-------------------
��������ȡ�߳�����״̬
NOTE��
REF��
------------------
Author��zsq
Date��2013��1��14��
*/	
static tEnumTHREADState _ThreadGetState(tPThread pThis)
{
	tEnumTHREADState threadState = ENUM_THRD_INITIAL;
	/*S0:�����ж�*/
	if(pThis != NULL)
	{
		/*S1:����*/
		pThis->fnLock(pThis);
		/*S2:��ȡ�̵߳�����״̬*/
		threadState = pThis->state;
		/*S3:����*/
		pThis->fnUnlock(pThis);
	}
	return threadState;
}

/*-------------------------------�ڲ�˽�з���---------------------------BEGIN------------*/
/*
_InnerMethod_ThreadQuitHandle
-------------------
�������̶߳����ڲ��˳�����
NOTE��
REF��
------------------
Author��Martin.lee
Date��2013��1��14��
*/	
static void _InnerMethod_ThreadQuitHandle(tPThread pThis)
{
	/*S0:�����ж�*/
	if(pThis == NULL)
	{
		return;
	}
	/*S1:�л�״̬���˳�̬*/	
	_ThreadChangeState(pThis,ENUM_THRD_TERMINATED);
	/*S2:�ж�ִ���Ƴ�Free*/
	((tPObject)pThis)->fnDoDelayFree((tPObject)pThis) ;	
}


/*
_InnerMethod_ThreadDelayFreeOccur
-------------------
�������̶߳����ڲ��˳�����
NOTE��
REF��
------------------
Author��Martin.lee
Date��2013��1��14��
*/	
static void _InnerMethod_ThreadDelayFreeOccur(tPObject pObject,INT32U u32Param)
{
	THREAD_CAST(pObject,pThread);
//	OBJECT_CAST(tPThread,pObject);
	BOOL bNeedFree = FALSE;
	/*S1:������֤*/
	if (pObject == NULL)
	{
		return ;
	}
	/*S2:�ж�״̬Ϊ�˳�̬���ϣ�ִ������*/
	pThread->fnLock(pThread);
	if (THREAD_STAT_IS_TERMINATE(pThread) && pObject->bNeedDelayFree == TRUE)
	{
		bNeedFree = TRUE;
	}
	pThread->fnUnlock(pThread);
	if (bNeedFree == TRUE)
	{
		pObject->fnFree(pObject);
	}
}

/*-------------------------------�ڲ�˽�з���---------------------------END------------*/

/*
ThreadObj_Creat
-------------------
������Thread ����Create����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��10��
*/
 void ThreadObj_Creat(
		tPThread *ppThread,
		tPOsAdptBuilder pOsAdptBuilder,
		tPAdapterParam pAdapterParam,
		PINT16U pu16ErrorCode
		)
{
//	OBJECT_MALLOC_DECLARE();

	_SET_ERROR_CODE(pu16ErrorCode,ERROR_WI_NORMAL);
	if (ppThread == NULL || pOsAdptBuilder == NULL || pAdapterParam == NULL
		|| pOsAdptBuilder->fnCreatOSA == NULL || pOsAdptBuilder == NULL
	)
	{
		_SET_ERROR_CODE(pu16ErrorCode,DTHREADMODEL_ERROR_ILLEGAL_PARAM);
		return;
	}
	
	/*S1:Ϊ���������ڴ�ռ�*/
	(*ppThread) =  OBJECT_MALLOC(tThread,(*ppThread));
	if(*ppThread == NULL)
	{
//		printf("ThreadObj malloc failed! \n");
		DEBUG_PRINTF(MESSAGE_ERR,"**Err: [ThreadObj_Creat] ThreadObj malloc failed!  \r\n \r\n");
		_SET_ERROR_CODE(pu16ErrorCode,THREADMODEL_ERROR_MALLOC_MEM_FAILED);
		return;
	}

	/*S2:��ʼ������*/
	Object_Create((tPObject *)ppThread);
	/*S3:��ʼ������*/
	(*ppThread)->fnWaitForMsg = _Thread_WaitForMsg;
	//(*ppThread)->fnSleep = _Thread_Sleep;
	//(*ppThread)->fnWaitForEvent = _Thread_WaitForEvent;
	//(*ppThread)->fnTerminate = _Thread_Terminate;
	(*ppThread)->fnSendMessage = _Thread_SendMessage;
	(*ppThread)->fnLock = _Thread_Lock;
	(*ppThread)->fnUnlock = _Thread_Unlock;
	(*ppThread)->fnWaitForMsgOrEvent = _Thread_WaitForMsgOrEvent;
	(*ppThread)->fnMain = _Thread_ThreadMain;
	(*ppThread)->fnOnEvent = _ThreadOnEvent;
	(*ppThread)->fnOnMessage = _ThreadOnMessage;
	(*ppThread)->fnOnTimeout = _ThreadOnTimeout;
	if (0 == pAdapterParam->threadParam.u32MessagePumpTimeout)
	{
		(*ppThread)->u32MessagePumpTimeout = MESSAGEPUMPTIMEOUTINMICSEC;
	}
	else
	{
		(*ppThread)->u32MessagePumpTimeout = pAdapterParam->threadParam.u32MessagePumpTimeout;
	}
	(*ppThread)->fnPostEvent = _Thread_PostEvent;
	(*ppThread)->fnStart = _Thread_Start;
	(*ppThread)->fnStop = _Thread_Stop;
	(*ppThread)->fnOnQuited.fnNotify = NULL;
	(*ppThread)->fnOnQuited.pLisennerObj = NULL;
//	(*ppThread)->fnQuitedBind = _ThreadQuitedBind;
	(*ppThread)->bNeedFree = FALSE;
	(*ppThread)->state = ENUM_THRD_INITIAL;
	(*ppThread)->fnChangeState = _ThreadChangeState;
	//(*ppThread)->fnGetState = _ThreadGetState;
	/*S4:�����û��߳�*/
	/*S4.1 : �����߳������к�������Ϣ�õ���ڵ�ַ*/
	if (pAdapterParam->threadParam.fnThreadMain == NULL)
	{
		pAdapterParam->threadParam.fnThreadMain = (*ppThread)->fnMain;
	}
	/*S4.2 : �����߳������к����ײ�������ʵ�����������������ԭ��Ϊ(*tfnThreadMain)(tPThread pThis,tThreadFuncArg arg)*/
	pAdapterParam->pBelongingThread = (*ppThread);
	/*S4.3 : �����߳�*/
	
	(*ppThread)->pOSAdpt = pOsAdptBuilder->fnCreatOSA(pAdapterParam);
	
	
	if(NULL == (*ppThread)->pOSAdpt)
	{
//		printf("OSAdptBuilder create OSA fail\n ");
		DEBUG_PRINTF(MESSAGE_ERR,"**Err: [ThreadObj_Creat] OSAdptBuilder create OSA fail \r\n \r\n");
	}
		
	_ThreadChangeState(*ppThread,ENUM_THRD_IDLE);
	/*S5:�����鷽��*/
	/*S5-1:���ظ�����Size����*/
	(*ppThread)->baseObj.Size = sizeof(tThread);
	/*S5-2:ע�������free����*/
	(*ppThread)->baseObj.fnOverideFree(&((*ppThread)->baseObj),_ThreadFree);
	/*S5-3:����RequireFree*/
	(*ppThread)->baseObj.fnRequireFree = _ThreadRequireFree;
 }


/***************************************---��tThread������ʵ��End---**********************************/
