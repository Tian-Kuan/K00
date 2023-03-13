//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                    CollectionPackage.c
//������ ����Ｏ������ʵ���ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��10��                  ģ�鴴��                whl
//
#include "tTraversableObject.h"


/***************************************---��Traversable���������� BEGIN---********************************/

static void TraversableObject_Free(tPObject  pTraversableObject); 
static void ForEach(tPTraversableObj pTraversableObject,tfnTraverseObj fnAction);
//static INT32U _GetSize(tPObject pObject);

/***************************************---��Traversable���������� End  ---********************************/





/***************************************---��Traversable������ʵ�� BEGIN  ---*******************************/

/*
CollectionPackage_Init
-------------------
������ģ�鼶��ʼ������
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/	
void CollectionPackage_Init(void)
{
	//do nothing
}


/*
TraversableObject_Create
-------------------
�������ɱ��������ݶ��󴴽�����
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/	
INT32S TraversableObject_Create(
	_IN______ tPTraversableObj* ppTraversableObject
	)
{
	tPObject pBaseObj = NULL;
//	OBJECT_MALLOC_DECLARE();
	if (ppTraversableObject == NULL)
	{
		return -1;
	}
	/*S1:Ϊ���������ڴ�ռ�*/
    pBaseObj = (tPObject)*ppTraversableObject;
	(*ppTraversableObject) =  OBJECT_MALLOC(tTraversableObj,(*ppTraversableObject));
	if (*ppTraversableObject == NULL)
	{
		return -1;
	}
	/*S2:ִ�и��๹�������ѻ�öԸ���Ĵ�������ʼ�������ļ̳�*/
	Object_Create(&pBaseObj);		
	(*ppTraversableObject)->fnForEach = ForEach;
	/*S3:����free����*/
#if 0
	//pBaseObj->fnFree = TraversableObject_Free;
#else
	pBaseObj->fnOverideFree(pBaseObj,TraversableObject_Free);
#endif
	pBaseObj->Size      = sizeof(tTraversableObj);
	//pBaseObj->fnGetSize = _GetSize;
	return 0;
}


/*
_GetSize
-------------------
��������ȡ������ռ�ڴ��С
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
//INT32U _GetSize(
//	_IN______ tPObject pObject
//	)
//{
//	tPTraversableObj pTraversableObj = NULL;
//	if (pObject == NULL )
//	{
//		return 0;
//	}
//	pTraversableObj = (tPTraversableObj)pObject;
//	return pTraversableObj->baseObj.Size;
//}


/*
TraversableObject_Free
-------------------
�������ɱ��������ݶ���free����
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/	
void TraversableObject_Free(
	_IN______ tPObject  pObject
	)
{
#if 0
	//tPTraversableObj pTraversableObject =NULL;
	//if (pTraversableObject == NULL)
	//{
	//	return;
	//}
 //   pTraversableObject = (tPTraversableObj)pObject;
	///*S2:���ͷ��ӽڵ�*/
	//free(pTraversableObject);
#endif
}

/*
ForEach
-------------------
�������ɱ��������ݶ����������
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/	
void ForEach(
	_IN______ tPTraversableObj  pTraversableObject,
	_IN______ tfnTraverseObj fnAction
	)
{
	//do nothing
}



/***************************************---��Traversable������ʵ��   End  ---********************************/
