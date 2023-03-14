//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                    ThreadModel.c
//描述： 威瑞达线程模块实现文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月11日                  模块创建                zsq
//
#include <stdio.h>
#include <string.h>
#include "ThreadModel.h"
#include "RS232.h"
#include "led.h"
#include "IWDG.h"


/***************************************---↑tThread对象函数声明 BEGIN---********************************/
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
/***************************************---↑tThread对象函数声明 End---**********************************/



/***************************************---↑tThread对象函数实现BEGIN---********************************/


/*
_Thread_WaitForMsg
-------------------
描述：线程等待消息处理函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月20日
*/
static tEnumThreadWaiteMsgReturn _Thread_WaitForMsg(tPThread pThis,tPMsgObj pMsg,tOSTimeout timeoutInMicrosec)
{
	
	INT32U u32EventID = 0;
	tEnumThreadWaiteMsgReturn retValue = ENUM_THREAD_WAITMSG_RETURN_FAILED ;

	/*S1:参数验证*/
	if (pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnMailboxPend ==NULL)
	{
		return retValue;
	}
	/*S2:等待消息接收*/ 
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
描述：线程休眠处理函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月20日
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
描述：线程内存块锁定处理函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月20日
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
描述：线程内存块锁定解除处理函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月20日
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
描述：线程发消息处理函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月20日
*/
static BOOL _Thread_SendMessage(tPThread pThis,tPMsgObj pMsg,tOSTimeout timeoutInMicrosec)
{
	BOOL retValue = FALSE;
	tEnumOSCallResult enumOsCallRslt = ENUM_SUCCESS;
	/*S0:参数验证*/
	if(pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnMailboxPost == NULL)
	{
		//参数出错处理
		return FALSE;
	}
	else
	{
		/*S1:执行发送消息命令*/
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
描述：线程终结处理函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月20日
*/
//static void _Thread_Terminate(tPThread pThis)
//{
//	/*S0:参数判断*/
////	if(pThis == NULL ||pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnTerminate == NULL)
////	{
////		//参数出错处理
////	}
////	else
////	{
////		/*S1:执行线程终止命令*/
////		if(pThis->state == ENUM_THRD_RUNNING)
////		{
////			pThis->pOSAdpt->fnTerminate(pThis->pOSAdpt);
////		}
////	}
//}


/*
  _Thread_ThreadMain
   -----------------
   描述： 线程主方法。构建线程模型的消息泵机制。
          线程接收发送给自己的消息/事件/退出线程/及超时信号
		  并调用线程对象的处理方法。
		  Thread的子类通过重载消息处理方法以获得对信号处理的不同
		  行为的扩展。
  NOTE：
  REF：
  ------------------
  Author：Martin.lee,zsq
  Date：2012年12月20日
*/
static void _Thread_ThreadMain(tPThread pThis,tThreadFuncArg arg)
{
	BOOL bThreadQuitNeeded = FALSE;
	INT8U u8OrEventIDMask = THREAD_EVENT_ID_MAX;
	INT8U u8AndEventIDMask = 0;
	INT8U u8RtnEvent = 0;
	tMsgObj Message;
	//int  Index = 1;
	
	/*S1:参数验证*/
    if ( pThis == NULL || pThis->fnWaitForMsgOrEvent == NULL)
    {
		return;
    }
	
	//DEBUG_PRINTF(MESSAGE_DEBUG," *[_Thread_ThreadMain] Entry In   \r\n");
	
	/*S2:在消息及事件上等待*/
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
					/*S2-1:若等到的是[消息]，则处理[消息]*/
					if ( pThis->fnOnMessage != NULL )
					{
						pThis->fnOnMessage(pThis,&Message);
					}

					break;
				}
			case ENUM_THREAD_WAITMSGEVT_RETURN_EVT:
				{
					/*S2-2:若等到的是【事件】，则处理【事件】*/
					if ( pThis->fnOnEvent != NULL )
					{
						pThis->fnOnEvent(pThis,u8RtnEvent);
					}
					break;
				}
			case ENUM_THREAD_WAITMSGEVT_RETURN_THREADTERMINATE:
				{
					/*S2-3:若等到的是【退出】，则处理【退出】*/
					bThreadQuitNeeded = TRUE;
					/*S1:切换状态到退出态*/	
					_ThreadChangeState(pThis,ENUM_THRD_TERMINATING);					
					/*S3:接收到【线程退出】，则处理【线程退出】*/
					if(pThis->fnOnQuited.fnNotify != NULL
					&& pThis->fnOnQuited.pLisennerObj != NULL)
					{
						/*S3-1: 发送线程退出通知给对此感兴趣的对象*/
						pThis->fnOnQuited.fnNotify(pThis->fnOnQuited.pLisennerObj,pThis);
					}
					/*S4:退出前状态变更及判断执行[推迟Free]*/
					/*S4-1:延迟销毁线程*/
					_InnerMethod_ThreadQuitHandle(pThis);
					break;
				}
			case ENUM_THREAD_WAITMSGEVT_RETURN_TIMEOUT:
				{
					/*S2-4:若消息泵等待【超时】，则处理【消息泵超时】*/
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
描述：消息泵事件处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月20日
*/	
static BOOL _ThreadOnEvent(tPThread pThis,tThreadEventID u8EventID)
{
	// do nothing
	return TRUE;
}



/*
_ThreadOnEvent
-------------------
描述：消息泵消息处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月20日
*/	
static BOOL _ThreadOnMessage(tPThread pThis,tPMsgObj pMsg)
{
	//do nothing
	return TRUE;
}



/*
_ThreadOnTimeout
-------------------
描述：消息泵超时处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月20日
*/	
static BOOL _ThreadOnTimeout(tPThread pThis)
{
	return TRUE;
}


/*
_ThreadFree
-------------------
描述：线程退出处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月20日
*/	
static void _ThreadFree(tPObject pObject)
{
//	tPThread pThis = (tPThread)pObject;
	
	///*S1:判断线程状态后执行推迟Free*/
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
描述：线程延迟销毁处理机制
NOTE：
REF：
------------------
Author：Martin.lee
Date：2013年1月14日
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
描述：线程内部销毁处理机制
NOTE：
REF：
------------------
Author：Martin.lee
Date：2013年1月14日
*/	
static void _InnerThreadFree(tPObject pObject)
{
	tPThread pThis = (tPThread)pObject;

	/*S1:判断并销毁适配器对象*/
	if (pThis != NULL && pThis->pOSAdpt != NULL)
	{
		pThis->pOSAdpt->baseObj.fnFree((tPObject)pThis->pOSAdpt);
	}
}

/*
_Thread_WaitForEvent
-------------------
描述：线程事件处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月11日
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
描述：线程等待事件OR消息处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月11日
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

	/*S1:参数验证*/
	if (pThis == NULL ||pThis->pOSAdpt == NULL ||pThis->pOSAdpt->fnMailboxPend == NULL)
	{
		return retValue;
	}
	/*S2:等待消息接收*/ 
	u32EventID = pThis->pOSAdpt->fnOSAWaiteForMsg(pThis->pOSAdpt,u8AndEventIDMask,u8OrEventIDMask,timeoutInMicrosec);

	if((u32EventID & THREAD_EVENT_ID_00) != 0)  //← 收到消息
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
描述：线程退出处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2013年1月4日
*/

//static void _ThreadQuited(tPObject pObject,tPThread pThis)
//{
//	//do nothing
//}

/*
_Thread_PostEvent
-------------------
描述：线程事件发送处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2013年1月4日
*/
static void _Thread_PostEvent(tPThread pThis,tThreadEventID u8Event)
{
	if(pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnPostEvent == NULL)
	{
		//参数出错处理
	}
	else
	{
		/*S1:执行线程终止命令*/
		//if(THREAD_STAT_IS_RUNNING(pThis))
		{
			pThis->pOSAdpt->fnPostEvent(pThis->pOSAdpt,u8Event);
		}
	}
}




/*
_Thread_Start
-------------------
描述：线程启动处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2013年1月4日
*/
static void _Thread_Start(tPThread pThis)
{
	/*S0:参数判断及出错处理*/
	if(pThis == NULL || pThis->pOSAdpt == NULL || pThis->pOSAdpt->fnStart == NULL)
	{
		//参数出错处理
		return;
	}
	else
	{
		/*S2:运行线程，实际上是执行UnLock动作*/
		pThis->pOSAdpt->fnStart(pThis->pOSAdpt);
		/*S1:设置线程的运行状态为ENUM_THRD_RUNNING*/
		_ThreadChangeState(pThis,ENUM_THRD_RUNNING);
	}
}

/*
_Thread_Stop
-------------------
描述：线程停止处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2013年1月7日
*/
static void _Thread_Stop(tPThread pThis)
{
	/*S0:参数判断及出错处理*/
	if(pThis == NULL || pThis->fnPostEvent == NULL)
	{
		//参数出错处理
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
描述：线程退出绑定处理机制
NOTE：
REF：
------------------
Author：zsq
Date：2013年1月7日
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
描述：线程对象要求退出处理
NOTE：
REF：
------------------
Author：Martin.lee
Date：2013年1月14日
*/	
static BOOL _ThreadRequireFree(tPObject pObject)
{
	THREAD_CAST(pObject,pThis);
	/*S0:参数验证*/
	if(pObject == NULL)
	{
		return FALSE;
	}
	
	/*S1:加锁*/
	pThis->fnLock(pThis);
	/*S2:判断是否处于运行状态，若是则推迟销毁*/
	if(THREAD_STAT_IS_RUNNING(pThis))
	{
		/*S1-1:处于运行状态，停止该线程并标识为推迟销毁*/
		pThis->fnStop(pThis);
		pThis->baseObj.bNeedDelayFree = TRUE;
		/*S1-1-1:定义用户扩展的销毁方法，由本对象定制推迟销毁方法*/
		pThis->baseObj.fnoDelayFree.pFreeObj = pObject;
		pThis->baseObj.fnoDelayFree.fnFree = _InnerMethod_ThreadDelayFreeOccur;
	}
	else
	{
		/*S1-2:标识为不需要推迟销毁*/
		pThis->baseObj.bNeedDelayFree = FALSE;
	}
	/*S3:解锁*/
	pThis->fnUnlock(pThis);
	return (pThis->baseObj.bNeedDelayFree == TRUE)?FALSE:TRUE;
}

/*
_ThreadChangeState
-------------------
描述：设置线程运行状态
NOTE：
REF：
------------------
Author：zsq
Date：2013年1月14日
*/
static void _ThreadChangeState(tPThread pThis,tEnumTHREADState threadState)
{
	/*S0:参数判断*/
	if(pThis != NULL)
	{
		/*S1:加锁*/
		pThis->fnLock(pThis);
		/*S2:设置线程的运行状态*/
		pThis->state = threadState;
		/*S3:解锁*/
		pThis->fnUnlock(pThis);
	}

}

/*
_ThreadGetState
-------------------
描述：获取线程运行状态
NOTE：
REF：
------------------
Author：zsq
Date：2013年1月14日
*/	
static tEnumTHREADState _ThreadGetState(tPThread pThis)
{
	tEnumTHREADState threadState = ENUM_THRD_INITIAL;
	/*S0:参数判断*/
	if(pThis != NULL)
	{
		/*S1:加锁*/
		pThis->fnLock(pThis);
		/*S2:获取线程的运行状态*/
		threadState = pThis->state;
		/*S3:解锁*/
		pThis->fnUnlock(pThis);
	}
	return threadState;
}

/*-------------------------------内部私有方法---------------------------BEGIN------------*/
/*
_InnerMethod_ThreadQuitHandle
-------------------
描述：线程对象内部退出处理
NOTE：
REF：
------------------
Author：Martin.lee
Date：2013年1月14日
*/	
static void _InnerMethod_ThreadQuitHandle(tPThread pThis)
{
	/*S0:参数判断*/
	if(pThis == NULL)
	{
		return;
	}
	/*S1:切换状态到退出态*/	
	_ThreadChangeState(pThis,ENUM_THRD_TERMINATED);
	/*S2:判断执行推迟Free*/
	((tPObject)pThis)->fnDoDelayFree((tPObject)pThis) ;	
}


/*
_InnerMethod_ThreadDelayFreeOccur
-------------------
描述：线程对象内部退出处理
NOTE：
REF：
------------------
Author：Martin.lee
Date：2013年1月14日
*/	
static void _InnerMethod_ThreadDelayFreeOccur(tPObject pObject,INT32U u32Param)
{
	THREAD_CAST(pObject,pThread);
//	OBJECT_CAST(tPThread,pObject);
	BOOL bNeedFree = FALSE;
	/*S1:参数验证*/
	if (pObject == NULL)
	{
		return ;
	}
	/*S2:判断状态为退出态场合，执行销毁*/
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

/*-------------------------------内部私有方法---------------------------END------------*/

/*
ThreadObj_Creat
-------------------
描述：Thread 对象Create函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
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
	
	/*S1:为自身点分配内存空间*/
	(*ppThread) =  OBJECT_MALLOC(tThread,(*ppThread));
	if(*ppThread == NULL)
	{
//		printf("ThreadObj malloc failed! \n");
		DEBUG_PRINTF(MESSAGE_ERR,"**Err: [ThreadObj_Creat] ThreadObj malloc failed!  \r\n \r\n");
		_SET_ERROR_CODE(pu16ErrorCode,THREADMODEL_ERROR_MALLOC_MEM_FAILED);
		return;
	}

	/*S2:初始化父类*/
	Object_Create((tPObject *)ppThread);
	/*S3:初始化对象*/
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
	/*S4:创建用户线程*/
	/*S4.1 : 付于线程主运行函数以消息泵的入口地址*/
	if (pAdapterParam->threadParam.fnThreadMain == NULL)
	{
		pAdapterParam->threadParam.fnThreadMain = (*ppThread)->fnMain;
	}
	/*S4.2 : 付于线程主运行函数首参数，以实现其操作方法。函数原型为(*tfnThreadMain)(tPThread pThis,tThreadFuncArg arg)*/
	pAdapterParam->pBelongingThread = (*ppThread);
	/*S4.3 : 创建线程*/
	
	(*ppThread)->pOSAdpt = pOsAdptBuilder->fnCreatOSA(pAdapterParam);
	
	
	if(NULL == (*ppThread)->pOSAdpt)
	{
//		printf("OSAdptBuilder create OSA fail\n ");
		DEBUG_PRINTF(MESSAGE_ERR,"**Err: [ThreadObj_Creat] OSAdptBuilder create OSA fail \r\n \r\n");
	}
		
	_ThreadChangeState(*ppThread,ENUM_THRD_IDLE);
	/*S5:重载虚方法*/
	/*S5-1:重载根对象Size方法*/
	(*ppThread)->baseObj.Size = sizeof(tThread);
	/*S5-2:注册根对象free方法*/
	(*ppThread)->baseObj.fnOverideFree(&((*ppThread)->baseObj),_ThreadFree);
	/*S5-3:重载RequireFree*/
	(*ppThread)->baseObj.fnRequireFree = _ThreadRequireFree;
 }


/***************************************---↑tThread对象函数实现End---**********************************/
