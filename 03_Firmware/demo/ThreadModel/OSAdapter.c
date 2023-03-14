//////////////////////////////////////////////////////////////////////////
//文件名                 OSAdapter.c
//描述：操作系统适配器模块头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月11日	模块创建               zsq


#include "OSAdapter.h"


/***************************************---↑tOSAdapter对象函数声明 BEGIN---********************************/

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
/***************************************---↑tOSAdapter对象函数声明 End---**********************************/





/***************************************---↑tOSAdapter对象函数实现 BEGIN---********************************/
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
描述：操作系统适配器模对象Create函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
*/	

INT32S OSAdapterObject_Create(tPOSAdapter* ppOSAdapterObject)
{
//	OBJECT_MALLOC_DECLARE();
	if (ppOSAdapterObject == NULL)
	{
		return -1;
	}
	
	/*S1:为自身点分配内存空间*/
	(*ppOSAdapterObject) =  OBJECT_MALLOC(tOSAdapter,(*ppOSAdapterObject));
	if(*ppOSAdapterObject == NULL)
	{
		//返回错误码
		return -1;
	}
	
	/*S2:执行父类构建方法已获得对父类的创建及初始化方法的继承*/
	Object_Create((tPObject*)ppOSAdapterObject);	
	/*S3:初始化子类自己的方法*/
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
	/*S4:注册根对象free方法*/
	(*ppOSAdapterObject)->baseObj.Size = sizeof(tOSAdapter);
	(*ppOSAdapterObject)->baseObj.fnOverideFree(&((*ppOSAdapterObject)->baseObj),_OSAFree_Dumy);
	return 0;
}









/*
_OSAMailboxPend_Dumy
------------------
描述：系统适配器邮件消息接收哑元函数	
NOTE：用于初始化系统适配器邮件消息接收方法
REF： OSAdapterPakage.h的tOSAdapter定义
------------------
Author：zsq
Date：  2012年12月13
*/
static tEnumOSCallResult  _OSAMailboxPend_Dumy(tPOSAdapter pOSAdapter,tPMsgObj pMsg,tOSTimeout timeoutInMicsec)
{
	return ENUM_SUCCESS;
}



/*
_OSAMailboxPost_Dumy
-------------------
描述：系统适配器Mailbox发送操作哑元函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
*/	
static tEnumOSCallResult  _OSAMailboxPost_Dumy(tPOSAdapter pOSAdapter,tPMsgObj pMsg,tOSTimeout timeoutInMicsec)
{
	return ENUM_SUCCESS;
}

/*
_OSASemaphorePend_Dumy
-------------------
描述：系统适配器Semaphore接收操作哑元函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
*/	
//static tEnumOSCallResult  _OSASemaphorePend_Dumy(tPOSAdapter pOSAdapter,tOSTimeout timeoutInMicsec)
//{
//	return ENUM_SUCCESS;
//}

/*
_OSASemaphorePost_Dumy
-------------------
描述：系统适配器Semaphore发送操作哑元函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
*/	
//static tEnumOSCallResult  _OSASemaphorePost_Dumy(tPOSAdapter pOSAdapter,INT8U u8PostCnt,tOSTimeout timeoutInMicsec)
//{
//	return ENUM_SUCCESS;
//}


/*
_OSALock_Dumy
-------------------
描述：系统适配器块锁定操作哑元函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
*/	
static BOOL _OSALock_Dumy(tPOSAdapter pOSAdapter)
{
	return TRUE;
}

/*
_OSAUnLock_Dumy
-------------------
描述：系统适配器块接除锁定操作哑元函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
*/	
static BOOL _OSAUnLock_Dumy(tPOSAdapter pOSAdapter)
{
	return TRUE;
}


/*
_OSASleep_Dumy
-------------------
描述：系统适配器线程休眠操作哑元函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
*/
//static void _OSASleep_Dumy(INT32U u32MicSec)
//{
//	//do nothing
//}


/*
_OSATerminate_Dumy
-------------------
描述：系统适配器线程终结操作哑元函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
*/
//static void _OSATerminate_Dumy(tPOSAdapter pOSAdapter)
//{
//	//do nothing
//}


/*
_OSAStart_Dumy
-------------------
描述：系统适配器线程运行操作哑元函数
NOTE：对TIBIOS而言，_OSAStart只需执行一次，即创建完第一个Thread线程，
	  执行_OSAStart，此时整个BIOS就已经运行起来来，故创建后面的Thread
	  线程不需要再执行此类的操作
REF：
------------------
Author：zsq
Date：2012年12月10日
*/
static void _OSAStart_Dumy(tPOSAdapter pOSAdapter)
{
	//do nothing
}



/*
_OSAWaiteForMsg_Dumy
-------------------
描述：系统适配器线程等待消息OR事件操作哑元函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月10日
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
描述：系统适配器线程发送事件哑元函数
NOTE：
REF：
------------------
Author：zsq
Date：2013年1月6日
*/
//static void _OSAPostEvent_Dumy(tPOSAdapter pOSAdapter,tThreadEventID u8Event)
//{
//	//do nothing
//}

/***************************************---↑tOSAdapter对象函数实现 END---***********************************/
