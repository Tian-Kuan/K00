//////////////////////////////////////////////////////////////////////////
//�ļ���                 OSAdapter.h
//����������ϵͳ������ģ��ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��11��	ģ�鴴��               zsq



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
������ϵͳ����������
*/
typedef struct _tagAdapterParam
{
	tThreadParam threadParam;        /*�̲߳���*/
	tPThread     pBelongingThread;   /*���ڵ��̶߳���*/
}tAdapterParam,*tPAdapterParam;

#define ADAPTER_PARAM_INIT(param)  {\
	param.threadParam.arg = NULL;\
	param.threadParam.priority = THREAD_WIPRIORITY_NORMAL;\
	param.threadParam.pStack = NULL;\
    param.threadParam.stackSize = 0;\
    param.threadParam.bAutoRun = FALSE;\
	param.threadParam.fnThreadMain = NULL;\
}\

//��tPOSAdapterϵͳ�����������������
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
�������߳�����״̬		
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
������ϵͳ�������������ݶ���
*/
struct _tagStructOSAdpt
{
	tObject             baseObj;			 /*������������*/
//	tAdapterParam       adapterParam;        /*����������*/
	tPThreadStack       pStack;				     /*�߳�ʹ�õ�ջ��ַ nullable*/
	tPThread            pBelongingThread;   /*���ڵ��̶߳���*/
	tfnThreadMain       fnThreadMain;		     /*�߳�������*/	
	tfnOSAMailboxPend   fnMailboxPend;		 /*������Ϣ����*/
	tfnOSAMailboxPost   fnMailboxPost;		 /*������Ϣ����*/
	//tfnOSASemaphorePend fnSemaphorePend;	 /*�������*/
	//tfnOSASemaphorePost fnSemaphorePost;	 /*���﷢��*/
	tfnOSALock          fnLock;				 /*�߳̿�����*/
	tfnOSAUnLock        fnUnLock;			 /*�߳̿��������*/
	//tfnOSASleep         fnSleep;			 /*�߳�����*/
	//tfnOSATerminate     fnTerminate;		 /*�߳���ֹ*/	
	tfnOSAStart         fnStart;			 /*�߳�����*/
	tfnOSAPostEvent     fnPostEvent;         /*�����¼�*/
	tfnOSAWaiteForMsg   fnOSAWaiteForMsg;    /*�ȴ���ϢOR�¼�*/
};


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
void AdapterParam_Init(tPAdapterParam pParam);

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
OSADAPTER_WINAPI INT32S OSAdapterObject_Create(tPOSAdapter* ppOSAdapterObject);

#endif



