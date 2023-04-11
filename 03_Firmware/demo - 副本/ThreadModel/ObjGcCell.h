//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                    ObjGcCell.h
//������ �����ɱ�������ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2013��1��7��                  ģ�鴴��                Martin.lee

#ifndef __HH_ObjGCCELL_H_HH__
#define __HH_ObjGCCELL_H_HH__


#include "BaseType.h"
#include "ModuleCommon.h"
#include "tObject.h"
#include "tList.h"


#define  OBJGCCELL_MALLOC_FAILED      1       //���ڴ�����ʧ��
#define  OBJGCCELL_PARAM_ILLEGAL      2       //�������Ƿ�
#define  OBJGCCELL_PARAM_NULL         3       //������Ϊ��


typedef struct _tagObjGcCell tObjGcCell,*tPObjGcCell;


typedef void (*tfnObjGcFree)(tPObjGcCell pGcObj);
typedef void (*tfnObjGcRegist)(tPObjGcCell pGcObj,tPObject pNeedGcObj);


/*	
tObjGcCell
-------------------
�������������յ�Ԫ���Ͷ���
NOTE��
REF��
------------------
Author��whl
Date��2013��1��7��
*/
struct _tagObjGcCell
{
	tListObj       innerObjList;   //�� �Ǽ���Ҫ���յĶ����б�
	tfnObjGcRegist fnRegist;    //���Ǽ���Ҫ���յĶ���
	tfnObjGcFree   fnFree;        //�� ���ղ���
};


/*	
ObjGcCell_Create
-------------------
�������������յ�Ԫ��������
NOTE��
REF��
------------------
Author��whl
Date��2013��1��7��
*/
void ObjGcCell_Create(
    _IN______ tPObjGcCell* ppGcCell,
    _OUT____  PINT16U pu16ErrorCode
    );



#endif

