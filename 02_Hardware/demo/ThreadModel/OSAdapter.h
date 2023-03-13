//////////////////////////////////////////////////////////////////////////
//文件名                 OSAdapter.h
//描述：操作系统适配器模块头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月11日	模块创建               zsq



#ifndef __HH_OSADAPTER_H_HH__
#define __HH_OSADAPTER_H_HH__



#include "ThreadCommon.h"


#if TARGETOS==X86
	#ifndef OSADAPTER_WINAPI 
		#ifdef  _OSAdapter_DLL_
			#define OSADAPTER_WINAPI  _declspec(dllexport) 
		#else
			#define OSADAPTER_WINAPI  _declspec(dllimport)
		#endif
	#endif
#else
	#ifndef OSADAPTER_WINAPI
		#define OSADAPTER_WINAPI
	#endif
#endif



//////////////////////////////////////////////////////////////////////////
typedef struct _tagStructOSAdpt tOSAdapter,*tPOSAdapter;

/*
tAdapterParam
------------
描述：系统适配器参数
*/
typedef struct _tagAdapterParam
{
	tThreadParam threadParam;        /*线程参数*/
	tPThread     pBelongingThread;   /*属于的线程对象*/
}tAdapterParam,*tPAdapterParam;

#define ADAPTER_PARAM_INIT(param)  {\
	param.threadParam.arg = NULL;\
	param.threadParam.priority = THREAD_WIPRIORITY_NORMAL;\
	param.threadParam.pStack = NULL;\
    param.threadParam.stackSize = 0;\
    param.threadParam.bAutoRun = FALSE;\
	param.threadParam.fnThreadMain = NULL;\
}\

//←tPOSAdapter系统适配对象函数操作类型
typedef tEnumOSCallResult (*tfnOSAMailboxPend)(tPOSAdapter pOSAdapter,tPMsgObj pMsg,tOSTimeout timeoutInMicsec);
typedef tEnumOSCallResult (*tfnOSAMailboxPost)(tPOSAdapter pOSAdapter,tPMsgObj pMsg,tOSTimeout timeoutInMicsec);
typedef tEnumOSCallResult (*tfnOSASemaphorePend)(tPOSAdapter pOSAdapter,tOSTimeout timeoutInMicsec);
typedef tEnumOSCallResult (*tfnOSASemaphorePost)(tPOSAdapter pOSAdapter,INT8U u8PostCnt,tOSTimeout timeoutInMicsec);

typedef BOOL (*tfnOSALock)(tPOSAdapter pOSAdapter);
typedef BOOL (*tfnOSAUnLock)(tPOSAdapter pOSAdapter);

typedef void (*tfnOSASleep)(INT32U u32MicSec);
typedef void (*tfnOSATerminate)(tPOSAdapter pOSAdapter);
typedef void (*tfnOSAStart)(tPOSAdapter pOSAdapter);
typedef void (*tfnOSAPostEvent)(tPOSAdapter pOSAdapter,tThreadEventID u8Event);
//typedef void (*tfnOSAFree)(tPOSAdapter pOSAdapter);



typedef tThreadEventID (*tfnOSAWaiteForMsg)(
			tPOSAdapter pOSAdapter,
			tThreadEventID u8AndEventIDMask,
			tThreadEventID u8OrEventIDMask,
			tOSTimeout timeOut
			);
/*
tEnumOSAState
------------
描述：线程运行状态		
*/
typedef enum
{
	ENUM_OSA_INITIAL,
	ENUM_OSA_IDLE,
	ENUM_OSA_RUNNING,
	ENUM_OSA_TERMINATED
}tEnumOSAState;

/*
struct _tagStructOSAdpt
------------
描述：系统适配器基本数据对象
*/
struct _tagStructOSAdpt
{
	tObject             baseObj;			 /*基本数据类型*/
//	tAdapterParam       adapterParam;        /*适配器参数*/
	tPThreadStack       pStack;				     /*线程使用的栈地址 nullable*/
	tPThread            pBelongingThread;   /*属于的线程对象*/
	tfnThreadMain       fnThreadMain;		     /*线程主方法*/	
	tfnOSAMailboxPend   fnMailboxPend;		 /*邮箱信息接收*/
	tfnOSAMailboxPost   fnMailboxPost;		 /*邮箱信息发送*/
	//tfnOSASemaphorePend fnSemaphorePend;	 /*旗语接收*/
	//tfnOSASemaphorePost fnSemaphorePost;	 /*旗语发送*/
	tfnOSALock          fnLock;				 /*线程块锁定*/
	tfnOSAUnLock        fnUnLock;			 /*线程块锁定解除*/
	//tfnOSASleep         fnSleep;			 /*线程休眠*/
	//tfnOSATerminate     fnTerminate;		 /*线程终止*/	
	tfnOSAStart         fnStart;			 /*线程运行*/
	tfnOSAPostEvent     fnPostEvent;         /*发送事件*/
	tfnOSAWaiteForMsg   fnOSAWaiteForMsg;    /*等待消息OR事件*/
};


/*
AdapterParam_Init
-------------------
描述：AdapterParam的初始化方法
NOTE：所有使用该参数的场合必须首先使用本方法进行对Param的初始化
REF：
------------------
Author：Martin。Lee
Date：2013年5月22日
*/
void AdapterParam_Init(tPAdapterParam pParam);

/*
OSAdapterObject_Create
-------------------
描述：操作系统适配器模对象Create函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
*/	
OSADAPTER_WINAPI INT32S OSAdapterObject_Create(tPOSAdapter* ppOSAdapterObject);

#endif



