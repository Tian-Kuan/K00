//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                    ThreadCommon.h
//������ ������߳�ģ�鹫��ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��13��                  ģ�鴴��                zsq
//

#ifndef __HH_THREADCOMMON_HH__
#define __HH_THREADCOMMON_HH__

#include "BaseType.h"
#include "tObject.h"
#include "ModuleCommon.h"


#define THREAD_TIMEOUT_FOREVER  (~(0))				/*�߳�һֱ�ȴ�*/	
#define THREAD_TIMEOUT_NOWAIT   (0)					/*�̲߳��ȴ�*/
#define THREAD_THREAD_PRIORITY_NORMAL 15			/*�߳���ͨ���ȼ�*/

#define  THREAD_ERROR_ILLEGAL_PARAM					ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x01)	/*�����Ƿ�*/
#define  THREAD_ERROR_CANNOT_MALLOC					ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x02)	/*�޷������ڴ�*/
#define  THREAD_ERROR_CANNOT_CREATE_MSG_BUF    		ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x03)	/*�޷�������Ϣ����*/
#define  THREAD_ERROR_CANNOT_CREATE_EVENT      		ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x04)	/*�޷������¼�����*/
#define  THREAD_ERROR_CANNOT_CREATE_THREAD      	ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x05)	/*�޷������߳�*/
#define  THREAD_ERROR_CANNOT_CREATE_BLOCKER      	ERROR_DEFINE(THREAD_MODEL_MODULE_CODE, 0x06)	/*�޷�����������*/

#define THREAD_THREAD_MAX_MSG_NUM		20   //���߳̿ɻ����������Ϣ��Ŀ ��Thread��MSG�������ĳ��ϣ��µ���Ϣ�޷�д���Ե��µȴ�.
#define THREAD_WIPRIORITY_NORMAL		3
#define MESSAGEPUMPTIMEOUTINMICSEC      (INT32U)300000  //���߳̽�����Ϣ/�¼��ȴ�ʱ����,��λ:MS


/*
OSA_EVENT_ID_x
------------------
������ϵͳ֧�ֵ��¼�ID�ĺ궨��
Mark��Ŀǰֻ�������¶����ID��ʹ�á�
REF��
------------------
Author��Martin
Date��  2011��8��29��
*/
typedef		INT8U				tThreadEventID;				/*�߳��¼�����������*/
typedef		INT8U*				tPThreadEventID;			/*�߳��¼�����������*/
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
�������߳����ȼ���ȷ����֤
*/
#define THREAD_VERIFY_PRIORITY(prio)  ((prio)>=1 && (prio)<= THREAD_THREAD_PRIORITY_NORMAL)



/*
tEnumThreadWaiteMsgReturn
------------
�������߳���Ϣ�ȴ������������
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
�������߳��¼��ȴ������������
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
�������̵߳ȴ��¼�OR��Ϣ�����������
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
�������߳̽��գ�������Ϣ���¼�ִ�н������ֵ
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
������OS������Ϣ��������
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
������OS����ĳ�ʱʱ������
*/
typedef INT32U tOSTimeout;

/*
tStackSize
------------
������OS�����ջ��С����
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
�������̲߳���
*/
struct _tag_structThreadParam
{
	tThreadFuncArg arg;					     /*�̺߳����Ĳ���*/
	tPriority      priority;			     /*�̵߳����ȼ� none-nullable*/
	tPThreadStack  pStack;				     /*�߳�ʹ�õ�ջ��ַ nullable*/
	tStackSize     stackSize;			     /*�߳�ʹ�õ�ջ��С nullable*/
	BOOL           bAutoRun;				 /*�Ƿ񴴽����Զ�����*/
	INT32U         u32MessagePumpTimeout;    /*[Private virtual] ��Ϣ�õĳ�ʱ*/
	tfnThreadMain  fnThreadMain;		     /*�߳�������*/	
};

#endif




