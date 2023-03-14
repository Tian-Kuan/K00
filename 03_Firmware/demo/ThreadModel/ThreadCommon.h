//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                    ThreadCommon.h
//描述： 威瑞达线程模块公共头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月13日                  模块创建                zsq
//

#ifndef __HH_THREADCOMMON_HH__
#define __HH_THREADCOMMON_HH__

#include "BaseType.h"
#include "tObject.h"
#include "ModuleCommon.h"


#define THREAD_TIMEOUT_FOREVER  (~(0))				/*线程一直等待*/	
#define THREAD_TIMEOUT_NOWAIT   (0)					/*线程不等待*/
#define THREAD_THREAD_PRIORITY_NORMAL 15			/*线程普通优先级*/

#define  THREAD_ERROR_ILLEGAL_PARAM					ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x01)	/*参数非法*/
#define  THREAD_ERROR_CANNOT_MALLOC					ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x02)	/*无法申请内存*/
#define  THREAD_ERROR_CANNOT_CREATE_MSG_BUF    		ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x03)	/*无法创建消息缓存*/
#define  THREAD_ERROR_CANNOT_CREATE_EVENT      		ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x04)	/*无法创建事件对象*/
#define  THREAD_ERROR_CANNOT_CREATE_THREAD      	ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x05)	/*无法创建线程*/
#define  THREAD_ERROR_CANNOT_CREATE_BLOCKER      	ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x06)	/*无法创建阻塞器*/

#define THREAD_THREAD_MAX_MSG_NUM		20   //←线程可缓存的最大的消息数目 若Thread中MSG缓存满的场合，新的消息无法写入以导致等待.
#define THREAD_WIPRIORITY_NORMAL		3
#define MESSAGEPUMPTIMEOUTINMICSEC      (INT32U)300000  //←线程接收消息/事件等待时间数,单位:MS


/*
OSA_EVENT_ID_x
------------------
描述：系统支持的事件ID的宏定义
Mark：目前只能在如下定义的ID中使用。
REF：
------------------
Author：Martin
Date：  2011年8月29日
*/
typedef		INT8U				tThreadEventID;				/*线程事件类数据类型*/
typedef		INT8U*				tPThreadEventID;			/*线程事件类数据类型*/
#define THREAD_EVENT_ID_NONE	(tThreadEventID)0x00
#define THREAD_EVENT_ID_00		(tThreadEventID)0x01
#define THREAD_EVENT_ID_01		(tThreadEventID)0x02
#define THREAD_EVENT_ID_02		(tThreadEventID)0x04
#define THREAD_EVENT_ID_03		(tThreadEventID)0x08
#define THREAD_EVENT_ID_04		(tThreadEventID)0x10
#define THREAD_EVENT_ID_05		(tThreadEventID)0x20

#define THREAD_EVENT_MAX_NUM		6
#define THREAD_EVENT_ID_MAX			(THREAD_EVENT_ID_00|THREAD_EVENT_ID_01|THREAD_EVENT_ID_02|THREAD_EVENT_ID_03|THREAD_EVENT_ID_04|THREAD_EVENT_ID_05)
#define THREAD_EVENT_ID_FAILED		~(THREAD_EVENT_ID_MAX)  
   
#define THREAD_EVNT_VERIFY(evtID)	(evtID==THREAD_EVENT_ID_00||evtID==THREAD_EVENT_ID_01		\
									||evtID==THREAD_EVENT_ID_02||evtID==THREAD_EVENT_ID_03		\
									||evtID==THREAD_EVENT_ID_04||evtID==THREAD_EVENT_ID_05)


/*
OSA_VERIFY_PRIORITY
------------
描述：线程优先级正确性验证
*/
#define THREAD_VERIFY_PRIORITY(prio)  ((prio)>=1 && (prio)<= THREAD_THREAD_PRIORITY_NORMAL)



/*
tEnumThreadWaiteMsgReturn
------------
描述：线程消息等待结果返回类型
*/
typedef enum
{
	ENUM_THREAD_WAITMSG_RETURN_SUCCESS,
	ENUM_THREAD_WAITMSG_RETURN_TIMEOUT,
	ENUM_THREAD_WAITMSG_RETURN_THREADTERMINATE,
	ENUM_THREAD_WAITMSG_RETURN_FAILED
}tEnumThreadWaiteMsgReturn;

/*
tEnumThreadWaiteMsgReturn
------------
描述：线程事件等待结果返回类型
*/
typedef enum
{
	ENUM_THREAD_WAITEVT_RETURN_EVT,
	ENUM_THREAD_WAITEVT_RETURN_TIMEOUT=0xf0,
	ENUM_THREAD_WAITEVT_RETURN_THREADTERMINATE=0x0f,
	ENUM_THREAD_WAITEVT_RETURN_FAILED=0xff
}tEnumThreadWaitEvtReturn;


/*
tEnumThreadWaiteMsgReturn
------------
描述：线程等待事件OR消息结果返回类型
*/
typedef enum
{
	ENUM_THREAD_WAITMSGEVT_RETURN_EVT,
	ENUM_THREAD_WAITMSGEVT_RETURN_MSG,
	ENUM_THREAD_WAITMSGEVT_RETURN_TIMEOUT=0xf0,
	ENUM_THREAD_WAITMSGEVT_RETURN_THREADTERMINATE=0x0f,
	ENUM_THREAD_WAITMSGEVT_RETURN_FAILED=0xff
}tEnumThreadWaitMsgEvtReturn;

/*
tEnumOSCallResult
------------
描述：线程接收，发送消息或事件执行结果返回值
*/
typedef enum
{
	ENUM_SUCCESS,
	ENUM_TIMEOUT,
	ENUM_FAILD
}tEnumOSCallResult;


typedef struct  _tagMsgObjStruct tMsgObj,*tPMsgObj;
typedef void (*tfnMSGDEF_Free)(tPMsgObj pMsgObj);

/*
_tagMsgObjStruct
------------
描述：OS任务消息数据类型
*/
struct  _tagMsgObjStruct
{
	INT8U  u8MsgID;
	union
	{
		INT32U  u32Data;
		FLOAT   fltData;
		void*   pData;
	}msgData;
	//tfnMSGDEF_Free fnFree;
};

/*
tOSTimeout
------------
描述：OS任务的超时时间类型
*/
typedef INT32U tOSTimeout;

/*
tStackSize
------------
描述：OS任务的栈大小类型
*/
typedef INT16U tStackSize;

typedef INT32S tPriority;
typedef unsigned int tTaskFuncArg;
typedef void* tPThreadStack;
typedef void* tThreadFuncArg;

typedef struct _tag_structThread tThread,*tPThread;

typedef struct _tag_structThreadParam tThreadParam,*tPThreadParam;
typedef void (*tfnThreadMain)(tPThread pThis,tThreadFuncArg arg);

/*
struct _tag_structThreadParam
------------
描述：线程参数
*/
struct _tag_structThreadParam
{
	tThreadFuncArg arg;					     /*线程函数的参数*/
	tPriority      priority;			     /*线程的优先级 none-nullable*/
	tPThreadStack  pStack;				     /*线程使用的栈地址 nullable*/
	tStackSize     stackSize;			     /*线程使用的栈大小 nullable*/
	BOOL           bAutoRun;				 /*是否创建后即自动运行*/
	INT32U         u32MessagePumpTimeout;    /*[Private virtual] 消息泵的超时*/
	tfnThreadMain  fnThreadMain;		     /*线程主方法*/	
};

#endif




