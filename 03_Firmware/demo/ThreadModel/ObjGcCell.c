//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                    ObjGcCell.c
//������ �����ɱ�������ʵ���ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2013��1��7��                  ģ�鴴��                Martin.lee

#include "ObjGcCell.h"



static void _ObjGcFree(tPObjGcCell pGcObj);
static void _ObjGcRegist(tPObjGcCell pGcObj,tPObject pNeedGcObj);

/*	
_SET_ERROR_CODE
-------------------
���������������ú궨�庯��
NOTE��
REF��
------------------
Author��whl
Date��2013��1��7��
*/
#define _SET_ERROR_CODE(pu16ErrorCode,u16Code)   {\
	if ((pu16ErrorCode)!=NULL)\
		{\
			*(pu16ErrorCode) = u16Code;\
		}\
}\

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
    _IN______ tPObjGcCell *ppGcCell,
    _OUT____ PINT16U pu16ErrorCode
    )
{
	//OBJECT_MALLOC_DECLARE();
	/*S0:������֤*/
	if (pu16ErrorCode == NULL)
	{
		return;
	}
	/*S1:Ϊ��������ڴ�*/
	*ppGcCell = OBJECT_MALLOC(tObjGcCell,*ppGcCell);
	if (*ppGcCell == NULL)
	{
		_SET_ERROR_CODE(pu16ErrorCode,OBJGCCELL_MALLOC_FAILED);
		return;
	}
	/*S2:�����ʼ��*/
	List_Create((tPListObj*)ppGcCell,enNext);
	/*S3:�����ʼ��*/
	(*ppGcCell)->fnFree   = _ObjGcFree;
	(*ppGcCell)->fnRegist = _ObjGcRegist;
	((tPObject)((*ppGcCell)))->Size   = sizeof(tObjGcCell);
	_SET_ERROR_CODE(pu16ErrorCode,ERROR_WI_NORMAL);
}

/*	
_ObjGcFree
-------------------
�������������յ�ԪFree����
NOTE��
REF��
------------------
Author��whl
Date��2013��1��7��
*/
void _ObjGcFree(tPObjGcCell pGcObj)
{
	tPListObj pListObj = NULL;
	tPObject pTemObj = NULL;
	/*S1:������֤*/
	if (pGcObj == NULL)
	{
		return;
	}
	/*S2:ɾ���б��д洢�Ķ���*/
	pListObj = (tPListObj)pGcObj;
	pTemObj = pListObj->fnGetHead(pListObj);
	if (pTemObj != NULL)
	{
		pTemObj->fnFree(pTemObj);
	}
	while(pListObj->fnHasNext(pListObj))
	{
		pTemObj = pListObj->fnGetNext(pListObj);
		if (pTemObj != NULL)
		{
			pTemObj->fnFree(pTemObj);
		}
	}
	/*S3:ɾ������*/
	pListObj->fnClear(pListObj,FALSE); //��ɾ���б��нڵ�����
	OBJECT_FREE(pGcObj);

}


/*	
_ObjGcFree
-------------------
�������������յ�ԪFree����
NOTE��
REF��
------------------
Author��whl
Date��2013��1��7��
*/
void _ObjGcRegist(
		tPObjGcCell pGcObj,
		tPObject pNeedGcObj
		)
{
	tPListObj pListObj = NULL;
	/*S1:������֤*/
	if (pGcObj == NULL || pNeedGcObj == NULL)
	{
		return;
	}
	/*S2:������������б���*/
	pListObj = (tPListObj)pGcObj;
	pListObj->fnAdd(pListObj,pNeedGcObj);
}
