//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                    tTraversableObject.h
//������ �����ɱ�������ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��10��                  ģ�鴴��                whl
//

#ifndef _HH_TTRAVERSABLEOBJECT_HH_
#define  _HH_TTRAVERSABLEOBJECT_HH_


#include "BaseType.h"
#include "tObject.h"



typedef struct _tagstructTraversableObject tTraversableObj,*tPTraversableObj;

typedef struct _tagFnTraverse tfnTraverseObj,*tPfnTraverseObj;

typedef void (*tfnxTraverse)(tPTraversableObj pTraversableObj ,tPObject pElementObj,tPObject pTraversingObj,INT32U u32Param);


struct _tagFnTraverse
{
    INT32U  u32Param;
	tPObject pTraversingObject;
	tfnxTraverse fnTraverse;
};



typedef void (*tfnForEach)(tPTraversableObj,tfnTraverseObj);    //������������ʱִ�о������ı�������







/*
tTraversableObject
-------------------
�������ɱ��������ݶ���
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/	
struct _tagstructTraversableObject
{
	tObject    baseObj;       //������������  
	tfnForEach fnForEach; 
};


INT32S TraversableObject_Create(tPTraversableObj* ppTraversableObject); 

#endif


