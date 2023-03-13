//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                    ThreadModel.h
//������ ������߳�ģ��ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��11��                  ģ�鴴��                zsq
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
tEnumTHREADState Thread״̬ö������
-------------------
������tEnumTHREADState���Ͷ���
NOTE��
REF��
------------------
Author��zsq
Date��2013��1��14��
*/
typedef enum
{
	ENUM_THRD_INITIAL,
	ENUM_THRD_IDLE,
	ENUM_THRD_RUNNING,
	ENUM_THRD_TERMINATING,
	ENUM_THRD_TERMINATED
}tEnumTHREADState;

#define DTHREADMODEL_ERROR_ILLEGAL_PARAM         ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x01) //��������
#define THREADMODEL_ERROR_MALLOC_MEM_FAILED      ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x02) //�����ڴ�ʧ��

#define THREAD_STAT_IS(pThread,comparingStat) (_ThreadGetState( (tPThread)pThread ) ==(comparingStat))
#define THREAD_STAT_IS_TERMINATE(pThread) THREAD_STAT_IS(pThread, ENUM_THRD_TERMINATED)
#define THREAD_STAT_IS_RUNNING(pThread)  THREAD_STAT_IS(pThread,ENUM_THRD_RUNNING)
#define THREAD_CAST(pObj,pThread)  tPThread pThread =( (tPThread)(pObj))

typedef struct _tag_structThreadMessage tThreadMessage,*tPThreadMessage;
typedef  struct _tagtfnOnThreadQuitedObj tfnOnThreadQuited,*tPfnOnThreadQuited;
//��tPThread��������������
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
tfnOnThreadQuitedƥ�������Ͷ���
-------------------
������tfnOnThreadQuited���Ͷ���
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��17��
*/
struct _tagtfnOnThreadQuitedObj
{
	tPObject    pLisennerObj;
	tfnNotify   fnNotify;
};

/*
tThread
-------------------
�����������̶߳���
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��11��
*/
struct _tag_structThread
{
	tObject                 baseObj;                  /*[Public] ������������*/
	tPOSAdapter             pOSAdpt;				  /*[Private virtual] ������OSAdpt��ַ*/
	BOOL                    bNeedFree;				  /*[Private] �����Ƴ�free�ı�ʶ*/
	INT32U                  u32MessagePumpTimeout;    /*[Private virtual] ��Ϣ�õĳ�ʱ*/
	tEnumTHREADState        state;				      /*[Private] �߳�״̬*/
	tfnChangeState          fnChangeState;		      /*[Private]�����߳�״̬*/
	//tfnGetState             fnGetState;               /*[Private]��ȡ�߳�״̬*/
	tfnOnThreadQuited       fnOnQuited;		          /*[private] �߳��˳���������*/   
	tfnThreadMain           fnMain;				      /*[Private virtual] �߳������д�����*/
	tfnWaiteForMsg          fnWaitForMsg;		      /*[Public] �̵߳ȴ���Ϣ������*/
	//tfnThreadSleep          fnSleep;			      /*[Public] �߳�����,�ӳ�ִ���߳�����������*/
	//tfnThreadWaitForEvent   fnWaitForEvent;           /*[Public] �̵߳ȴ��¼�������*/
	//tfnTerminateThread      fnTerminate;		      /*[Public] �߳��սᴦ����*/
	tfnThreadSendMessage    fnSendMessage;		      /*[Public] �̷߳�����Ϣ������*/
	tfnThreadLock           fnLock;				      /*[Public] �߳̿�����������*/
	tfnThreadUnlock         fnUnlock;			      /*[Public] �߳̿��������������*/
	tfnThreadPostEvent      fnPostEvent;		      /*[Public] �̷߳����¼�������*/
	tfnThreadWaitMsgOrEvent fnWaitForMsgOrEvent;      /*[Public] �̵߳ȴ��¼�OR��Ϣ������*/
	tfnOnThreadMessage      fnOnMessage;		      /*[private virtual] ��Ϣ������*/
	tfnOnThreadEvent        fnOnEvent;                /*[private virtual] �¼�������*/
	tfnOnThreadTimeout      fnOnTimeout;              /*[private virtual] ��Ϣ�õȴ���ʱ������*/
	tfnThreadStart          fnStart;                  /*[Public] �߳�����������*/
	tfnThreadStop           fnStop;                   /*[Public] �߳�ֹͣ������*/
//	tfnThreadQuitedBind     fnQuitedBind;             /*[Public] �߳��˳�֪ͨ�����󶨷���*/ 
};


/*
tThreadMessage
-------------------
�������߳���Ϣ
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��11��
*/
struct _tag_structThreadMessage
{
	INT8U u8MessageID;
	tPObject pMessageData;
};

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
THREADMODELPACKAGE_WINAPI void ThreadObj_Creat(
    _IN______ tPThread *ppThread,
    _IN______ tPOsAdptBuilder pOsAdptBuilder,
    _IN______ tPAdapterParam pAdapterParam,
    _OUT____  PINT16U pu16ErrorCode
    );
#endif


