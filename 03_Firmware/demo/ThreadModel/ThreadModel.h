//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                    ThreadModel.h
//描述： 威瑞达线程模块头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月11日                  模块创建                zsq
//

#ifndef __HH_THREADMODEL_HH__
#define __HH_THREADMODEL_HH__

//#include <memory.h>
#include "ThreadCommon.h"
#include "OSAdapter.h"
#include "OsAdapterBuilder.h"

#if TARGETOS==X86
	#ifndef THREADMODELPACKAGE_WINAPI 
		#ifdef  _ThreadModelPackage_DLL_
			#define THREADMODELPACKAGE_WINAPI  _declspec(dllexport)	
		#else
			#define THREADMODELPACKAGE_WINAPI  _declspec(dllimport)
		#endif
	#endif
#else
		#ifndef THREADMODELPACKAGE_WINAPI
			#define THREADMODELPACKAGE_WINAPI
		#endif
#endif


/*	
tEnumTHREADState Thread状态枚举类型
-------------------
描述：tEnumTHREADState类型对象
NOTE：
REF：
------------------
Author：zsq
Date：2013年1月14日
*/
typedef enum
{
	ENUM_THRD_INITIAL,
	ENUM_THRD_IDLE,
	ENUM_THRD_RUNNING,
	ENUM_THRD_TERMINATING,
	ENUM_THRD_TERMINATED
}tEnumTHREADState;

#define DTHREADMODEL_ERROR_ILLEGAL_PARAM         ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x01) //参数错误
#define THREADMODEL_ERROR_MALLOC_MEM_FAILED      ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x02) //申请内存失败

#define THREAD_STAT_IS(pThread,comparingStat) (_ThreadGetState( (tPThread)pThread ) ==(comparingStat))
#define THREAD_STAT_IS_TERMINATE(pThread) THREAD_STAT_IS(pThread, ENUM_THRD_TERMINATED)
#define THREAD_STAT_IS_RUNNING(pThread)  THREAD_STAT_IS(pThread,ENUM_THRD_RUNNING)
#define THREAD_CAST(pObj,pThread)  tPThread pThread =( (tPThread)(pObj))

typedef struct _tag_structThreadMessage tThreadMessage,*tPThreadMessage;
typedef  struct _tagtfnOnThreadQuitedObj tfnOnThreadQuited,*tPfnOnThreadQuited;
//←tPThread对象函数操作类型
typedef void (*tfnNotify)(tPObject pLisennerObj,tPThread pThis);
typedef void (*tfnTerminateThread)(tPThread pThis);
typedef void (*tfnFreeThread) (tPThread pThis);
typedef tEnumThreadWaiteMsgReturn (*tfnWaiteForMsg)(tPThread pThis,tPMsgObj pMsg,tOSTimeout timeoutInMicrosec);
typedef BOOL (*tfnThreadSendMessage)(tPThread pThis,tPMsgObj pMsg,tOSTimeout timeoutInMicrosec);
typedef void (*tfnThreadLock)(tPThread pThis);
typedef void (*tfnThreadUnlock)(tPThread pThis);
typedef void (*tfnThreadSleep)(tPThread pThis,INT32U u32MicSec);

typedef BOOL (*tfnOnThreadMessage) (tPThread pThis,tPMsgObj pMsg);
typedef BOOL (*tfnOnThreadEvent) (tPThread pThis,tThreadEventID u8EventID);
typedef BOOL (*tfnOnThreadTimeout) (tPThread pThis);
typedef void (*tfnThreadStart)(tPThread pThis);
typedef void (*tfnThreadStop)(tPThread pThis);
typedef tEnumThreadWaitEvtReturn (*tfnThreadWaitForEvent)(
	tPThread pThis,
	tThreadEventID u8OrEventIDMask,
	tThreadEventID u8AndEventIDMask,
	tPThreadEventID pu8RtnEvent,
	tOSTimeout timeoutInMicrosec
	);

typedef void (*tfnThreadPostEvent)(tPThread pThis,tThreadEventID u8Event);

typedef tEnumThreadWaitMsgEvtReturn (*tfnThreadWaitMsgOrEvent)(
	tPThread pThis,
	tThreadEventID u8AndEventIDMask,
	tThreadEventID u8OrEventIDMask,
	tPThreadEventID pu8RtnEvent,
	tPMsgObj pMsg,
	tOSTimeout timeoutInMicrosec
	);
//typedef void (*tfnThreadQuitedBind)(tPThread pThis,tfnOnThreadQuited fnOnThreadQuited);
typedef void (*tfnChangeState)(tPThread pThis,tEnumTHREADState threadState);
typedef tEnumTHREADState (*tfnGetState)(tPThread pThis);
/*	
tfnOnThreadQuited匹配器类型定义
-------------------
描述：tfnOnThreadQuited类型对象
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月17日
*/
struct _tagtfnOnThreadQuitedObj
{
	tPObject    pLisennerObj;
	tfnNotify   fnNotify;
};

/*
tThread
-------------------
描述：基本线程对象
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月11日
*/
struct _tag_structThread
{
	tObject                 baseObj;                  /*[Public] 基本数据类型*/
	tPOSAdapter             pOSAdpt;				  /*[Private virtual] 适配器OSAdpt地址*/
	BOOL                    bNeedFree;				  /*[Private] 用于推迟free的标识*/
	INT32U                  u32MessagePumpTimeout;    /*[Private virtual] 消息泵的超时*/
	tEnumTHREADState        state;				      /*[Private] 线程状态*/
	tfnChangeState          fnChangeState;		      /*[Private]设置线程状态*/
	//tfnGetState             fnGetState;               /*[Private]获取线程状态*/
	tfnOnThreadQuited       fnOnQuited;		          /*[private] 线程退出参数对象*/   
	tfnThreadMain           fnMain;				      /*[Private virtual] 线程主运行处理方法*/
	tfnWaiteForMsg          fnWaitForMsg;		      /*[Public] 线程等待消息处理方法*/
	//tfnThreadSleep          fnSleep;			      /*[Public] 线程休眠,延迟执行线程主任务处理方法*/
	//tfnThreadWaitForEvent   fnWaitForEvent;           /*[Public] 线程等待事件处理方法*/
	//tfnTerminateThread      fnTerminate;		      /*[Public] 线程终结处理方法*/
	tfnThreadSendMessage    fnSendMessage;		      /*[Public] 线程发送消息处理方法*/
	tfnThreadLock           fnLock;				      /*[Public] 线程块锁定处理方法*/
	tfnThreadUnlock         fnUnlock;			      /*[Public] 线程块锁定解除处理方法*/
	tfnThreadPostEvent      fnPostEvent;		      /*[Public] 线程发送事件处理方法*/
	tfnThreadWaitMsgOrEvent fnWaitForMsgOrEvent;      /*[Public] 线程等待事件OR消息处理方法*/
	tfnOnThreadMessage      fnOnMessage;		      /*[private virtual] 消息处理方法*/
	tfnOnThreadEvent        fnOnEvent;                /*[private virtual] 事件处理方法*/
	tfnOnThreadTimeout      fnOnTimeout;              /*[private virtual] 消息泵等待超时处理方法*/
	tfnThreadStart          fnStart;                  /*[Public] 线程启动处理方法*/
	tfnThreadStop           fnStop;                   /*[Public] 线程停止处理方法*/
//	tfnThreadQuitedBind     fnQuitedBind;             /*[Public] 线程退出通知参数绑定方法*/ 
};


/*
tThreadMessage
-------------------
描述：线程消息
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月11日
*/
struct _tag_structThreadMessage
{
	INT8U u8MessageID;
	tPObject pMessageData;
};

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
THREADMODELPACKAGE_WINAPI void ThreadObj_Creat(
    _IN______ tPThread *ppThread,
    _IN______ tPOsAdptBuilder pOsAdptBuilder,
    _IN______ tPAdapterParam pAdapterParam,
    _OUT____  PINT16U pu16ErrorCode
    );
#endif


