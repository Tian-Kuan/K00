//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                          tArray.c
//������ ������б�����ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��17��                  ģ�鴴��                whl / hepeng
//

#include <string.h>
#include "tArray.h"

/***************************************---�� Array���������� BEGIN---********************************/
static void _Add(tPArrayObj,tPObject);                                                   // ���Ӳ���
static tPObject _GetAtIndex(tPArrayObj,INT16U);                                          // ��ȡָ������λ���ϵ�����
//static void _RegisterCallBack(tPArrayObj,tfnOnCollectionFull);                           // ע����Ӧ�Ļص�����
static INT32S _GetCount(tPArrayObj);                                                     // ��ȡArray��С
static void _Free (tPObject);                                                            // �ͷŴ洢�ռ�
//static void _RemoveAll(tPArrayObj);                                                      // ���Array
static tPObject* _GetBuf(tPArrayObj);                                                    // ��ȡArray�е�����

static void _ForEach(tPTraversableObj pTraversedObj,tfnTraverseObj fnTraverseAction);    // ����ForEach����

/***************************************---�� Array���������� End---**********************************/



/***************************************---�� Array������ʵ�� BEGIN---********************************/

/*	
Array_Create
-------------------
������Array���󴴽�
NOTE��
REF��
------------------
Author��whl
Date��2012��12��18��
*/
void Array_Create(
	_IN______ tPArrayObj * ppArrayObj 
	)
{
	tPTraversableObj pBaseObj = NULL;
	tPObject pObject = NULL;

	OBJECT_MALLOC_DECLARE();
	if ( NULL == ppArrayObj)
	{
		return ;
	}
	/* S1:ִ�б���ڵ�Ĵ������� */
	*ppArrayObj = OBJECT_MALLOC(tArrayObj,*ppArrayObj);
	if (*ppArrayObj == NULL)
	{
		return;
	}
	/* S2:ִ�и���ڵ�Ĵ������� */
	pBaseObj = (tPTraversableObj) (*ppArrayObj);
	TraversableObject_Create(&pBaseObj);
	pObject= (tPObject)(*ppArrayObj);
	if ((*ppArrayObj)->pArrayData == NULL)
	{
		OBJECT_MALLOC_FREE(pObject);
		return ;
	}
	/* S3: �����ͺ�����ʼ�� */
	(*ppArrayObj)->u16CurrentDataLength = 0;
//	(*ppArrayObj)->fnOnCollectionFull.fnNotify = NULL;
//	(*ppArrayObj)->fnOnCollectionFull.pListenner = NULL;
	(*ppArrayObj)->fnGetAtIndex = _GetAtIndex;
	(*ppArrayObj)->fnGetCount = _GetCount;
	(*ppArrayObj)->fnGetBuf = _GetBuf;
	(*ppArrayObj)->fnAdd = _Add;
//	(*ppArrayObj)->fnRemoveAll = _RemoveAll;
//	(*ppArrayObj)->fnCollectionArrayRegisterCallBack = _RegisterCallBack;
	/*S4:��������������*/
	pBaseObj->fnForEach = _ForEach;
#if 0
	//pBaseObj->baseObj.fnFree = _Free;
#else
	pBaseObj->baseObj.fnOverideFree((tPObject)pBaseObj,_Free);
#endif
	pBaseObj->baseObj.Size = sizeof(tArrayObj);
}


/*	
_Free
-------------------
������Array�ͷź���
NOTE��
REF��
------------------
Author��whl / hepeng
Date��2012��12��18��
*/
void _Free (
	_IN______ tPObject pObject
	)
{
	tPArrayObj pArrayObj = NULL;

	if ( pObject == NULL)
	{
		return;
	}

	/* S1: �ͷ�Array�е�Ԫ�� */
	pArrayObj = (tPArrayObj)pObject;	
	if (pArrayObj->pArrayData != NULL)
	{
		OBJECT_FREE(pArrayObj->pArrayData);
	}
#if 0
	/* S2: �ͷ��Լ���ռ���ڴ�ռ� */
	//free(pArrayObj);
	//pArrayObj = NULL;
#endif
}


/*	
_RemoveAll
-------------------
������ɾ������������Ԫ������
NOTE��
REF��
------------------
Author��whl / hepeng
Date��2012��12��18��
*/
//void _RemoveAll(
//	_IN______ tPArrayObj pArrayObj
//	)
//{
//	INT16U u16Index = 0;
//	if (NULL == pArrayObj || pArrayObj->pArrayData == NULL)
//	{
//		return ;
//	}

//	/* S0: ��Array�е�������� */
//	for (u16Index = 0;u16Index < pArrayObj->u16CurrentDataLength;u16Index++)
//	{
//		pArrayObj->pArrayData[u16Index] = NULL;
//	}
//}



/*	
FUNCTION NAME: _GetCount
-------------------
DESCRIPTION����ȡָ���б���Ԫ�ظ���
NOTE��
REF��
------------------
Author��hepeng / whl
Date��2012-12-18
*/
INT32S _GetCount(  
	_IN______ tPArrayObj pArrayObj
	)
{
	if (pArrayObj == NULL || pArrayObj->pArrayData == NULL )
	{
		return -1;
	}

	/* S1: ���ص�ǰArray�Ĵ�С */
	return pArrayObj->u16CurrentDataLength;
}

/*	
FUNCTION NAME: _GetBuf
-------------------
DESCRIPTION����ȡArray�е�����
NOTE��
REF��
------------------
Author��hepeng / whl
Date��2012-12-18
*/
tPObject*  _GetBuf (
	_IN______ tPArrayObj pArrayObj
	)
{
	if ( NULL == pArrayObj || pArrayObj->pArrayData == NULL)
	{
		return NULL;
	}

	/* S0: ����Array�����ݵ��׵�ַ */
	return pArrayObj->pArrayData;
}


/*	
FUNCTION NAME: _ForEach
-------------------
DESCRIPTION������Array
NOTE��
REF��
------------------
Author��whl
Date��2012-12-18
*/
void _ForEach(
	_IN______ tPTraversableObj pTraversedObj,
	_IN______ tfnTraverseObj fnTraverseAction
	)
{
	INT32U Index = 0;

	tPArrayObj pArrayObj = (tPArrayObj)pTraversedObj;
	if ( pArrayObj->fnGetCount(pArrayObj) != 0 )
	{
		for (Index = 0;Index < pArrayObj->u16CurrentDataLength;Index++)
		{
				fnTraverseAction.fnTraverse((tPTraversableObj)pArrayObj,(tPObject)(pArrayObj->pArrayData+Index),fnTraverseAction.pTraversingObject,fnTraverseAction.u32Param);
		}
	}	
}

/*	
_RegisterCallBack
-------------------
������ע�����ݴ��������Ļص�����
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
//void _RegisterCallBack(
//	_IN______ tPArrayObj pArrayObj,
//	_IN______ tfnOnCollectionFull fnOnCollectionFull
//)
//{
//	/*S1:������֤*/
//	if (pArrayObj == NULL 
//		|| fnOnCollectionFull.fnNotify == NULL 
//		|| fnOnCollectionFull.pListenner == NULL
//	)
//	{
//		return;
//	}
//	/*S2:ע��ص�����*/
//	memcpy(&(pArrayObj->fnOnCollectionFull),&fnOnCollectionFull,sizeof(tfnOnCollectionFull));

//}


/*	
_Add
-------------------
���������Ԫ��
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
void _Add(
	_IN______ tPArrayObj pArrayObj,
	_IN______ tPObject pObj
	)
{
	if (pArrayObj == NULL || pObj == NULL)
	{
		return;
	}
	/*�ж��Ƿ����*/
	if (pArrayObj->u16CurrentDataLength == ARRAY_OBJECT_MAX_LENGTH)
	{
		//����ʱԪ���Ѽ����������ص�����
//		if (pArrayObj->fnOnCollectionFull.fnNotify != NULL 
//			&& pArrayObj->fnOnCollectionFull.pListenner != NULL)
//		{
//			//���ע����ص������������ص�����֪ͨ
//			pArrayObj->fnOnCollectionFull.fnNotify(
//				pArrayObj->fnOnCollectionFull.pListenner,
//				(tPObject)pArrayObj
//				);
//		}
		return;
		
	}
	pArrayObj->pArrayData[pArrayObj->u16CurrentDataLength] = pObj;
	pArrayObj->u16CurrentDataLength++;
}


/*	
_GetAtIndex
-------------------
���������Ԫ��
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
tPObject _GetAtIndex(
	_IN______ tPArrayObj pArrayObj,
	_IN______ INT16U u16DataIndex
	)
{
	/*S1:������֤*/
	if (pArrayObj == NULL
		|| u16DataIndex >= ARRAY_OBJECT_MAX_LENGTH)
	{
		return NULL;
	}
	/*S2:���������Ƿ����*/
	if (pArrayObj->u16CurrentDataLength > ARRAY_OBJECT_MAX_LENGTH)
	{
		return NULL;
	}
	/*S3:��ȡָ��λ�õ�ֵ*/
	return pArrayObj->pArrayData[u16DataIndex];
	
}

/***************************************---�� Array������ʵ�� End---**********************************/
