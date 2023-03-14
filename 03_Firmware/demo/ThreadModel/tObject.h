//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                       tObject.h
//������ ������������������ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��10��                  ģ�鴴��                whl
//


#ifndef __HH_TOBJECT_HH__
#define __HH_TOBJECT_HH__

#include "BaseType.h"
#include "DBG_Utility_Malloc.h"


#if TARGETOS==X86
	#ifndef BASETYPE_WINAPI 
		#ifdef  BaseType_DLL_
			#define BASETYPE_WINAPI  _declspec(dllexport)	
		#else
	       #define BASETYPE_WINAPI  _declspec(dllimport)
		#endif
	#endif
#else
	#ifndef BASETYPE_WINAPI
		#define BASETYPE_WINAPI
	#endif
#endif

/***************************************---��Object�������� BEGIN---********************************/


#define TYPE_INHERIT_TREE_MAX_LEVELS                   ( 5 )                //�����ͼ̳���������    hepeng moddify 5->10 on 2013.5.3 11:22


typedef struct _tagStructObject tObject,*tPObject;


//typedef BOOL (*tfnEqual)(tPObject pRootObj,tPObject pCurrentObj);
typedef INT32S (*tfnGetHashCode)(tPObject pRootObj );
typedef void (*tfnFree)(tPObject pRootObj);
typedef INT32U (*tfnGetSize)(tPObject pRootObj);
typedef BOOL (*tfnOverideFree)(tPObject pRootObj ,tfnFree fnFree);   
typedef BOOL (*tfnRequireFree)(tPObject pObject); //��������Ҫ���� ������true��������ٷ����ӳ�
typedef void (*tfnDelayFreeOccur)(tPObject pObject,INT32U u32Param);  //�� �Ƴ����ٷ�������

typedef void (*tfnDoDelayFree)(tPObject pObject);   //�� ִ���Ƴ�����

/*
  tfnoDelayFree
  -----------------------
  ������ �Ƴ����ٶ���
  ���ߣ� Martin.lee
  ���ڣ� 2013-1-14
*/
typedef struct _tagDelayFree
{
	tPObject  pFreeObj;
	INT32U    u32Param;
	tfnDelayFreeOccur fnFree;
}tfnoDelayFree;

/*
tObject
-------------------
�����������������
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
struct _tagStructObject
{
	INT16U         Size;
//	INT32U         HashCode;
	BOOL           bNeedDelayFree;                                   //[Private]���Ƿ��Ƴ�Free�ı�ǣ�default is False                     
	INT8U          u8CurntInheritObjCount;                           //[Private]��ָ��ǰ�̳ж�����������������
	tfnFree        _InnerFreesFIFO[TYPE_INHERIT_TREE_MAX_LEVELS];    //[Private]���������ͼ̳��������������б�
//	tfnEqual       fnEqual;                                          //[Public] ���ж���������ʵ���Ƿ���ȵĺ������Ƚ϶������ڵĹ�ϣ�룩
	tfnFree        fnFree;                                           //[Public] ����������������(���ڸ�����)
	tfnOverideFree fnOverideFree;                                    //[Public] �����ڶ���̳�����������������
	tfnoDelayFree  fnoDelayFree;                                     /*[Protected] �Ƴ����ٷ�������*/ 
	tfnRequireFree fnRequireFree;                                    /*[Protected] ��������.����trueΪ������.*/	
	tfnDoDelayFree fnDoDelayFree;                                    /*[Protected] ִ���Ƴ�����*/
//	tfnGetSize fnGetSize;                                            //[Public] ����ȡ��������ռ���ڴ��С
//	tfnGetHashCode fnGetHashCode;	                                 //[Public] ����ȡ������Ĺ�ϣ�루��ʱ������������ڴ�ĵ�ַ��
};


#define OBJECT_DECLARE(Type,pObject)   {\
	(Type*)pObject;\
	pObject = NULL; \
}\


#define OBJECT_CAST(type,pCastedObj)  type pThis = (type)(pCastedObj)                                                                        //[Public]������ָ�����͵�pThis
#define OBJECT_MEMORY_MALLOC(type,memlength) ((memlength <= 0)? NULL:((type*)MALLOC(memlength)))                                             //[Public]������ָ����С���ڴ�
#define OBJECT_MEMORY_REALLOC(type,pObject,memlength) (((pObject == NULL)||(memlength <= 0)) ? pObject:((type*)REALLOC(pObject,memlength)))  //[Public]��׷������ָ����С���ڴ�
#define OBJECT_MALLOC(type,pObject)   ( pObject == NULL ? ((type*)MALLOC(sizeof(type))) : pObject)                                           //[Public]�����������ڴ�궨��
#define OBJECT_FREE(pObject)   FREE(pObject)
#define OBJECT_MALLOC_DECLARE()   BOOL bNeedMalloc = TRUE                                                                                    //[Public]�����������ڴ�ǰ�����궨��
#define OBJECT_MALLOC_IS_NEED()    (bNeedMalloc == TRUE)
#define OBJECT_MALLOC_FREE(pObject)   {\
	if(OBJECT_MALLOC_IS_NEED()) {\
	    FREE((pObject));\
		pObject=NULL;\
	}\
}\

/*
Object_Create
-------------------
�����������󴴽�����
NOTE��
REF��
------------------
Author��whl
Date��2012��12��21��
*/

void Object_Create(tPObject* ppObject);


#endif
