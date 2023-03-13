//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                          tList.c
//������ ������б�����ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��10��                  ģ�鴴��                whl
//

#include "tList.h"
#include "utlist.h"
#include "BaseType.h"


/***************************************---��List���������� BEGIN---********************************/

static INT32S _GetCount(tPListObj);
static BOOL _HasNext(tPListObj);
static tPObject _GetNext(tPListObj);
static tPListElementObj _Find(tPListObj,tPObject);
static void _Add(tPListObj,tPObject);
static void _Remove(tPListObj,tPObject);
static void _Clear(tPListObj,BOOL);
static tPArrayObj _ToArray(tPListObj);
static void _Free (tPObject); 
static void _ForEach(tPTraversableObj pTraversedObj,tfnTraverseObj fnTraverseAction);
//static tPListObj _Element_Find(tPListObj pListObj,tfnListElementMatch);
static tPObject _GetHead(tPListObj);
static tPListElementObj _SearchPrevElement(tPListObj pListObj,tPListElementObj pCurrntElement);
//static INT32U _GetSize(tPObject pObject);


/***************************************---��List���������� End---**********************************/

/***************************************---��List������ʵ�� BEGIN---********************************/

/*	
ListCreate
-------------------
�������б�Ԫ�����Ͷ���
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
void List_Create(
	_IN______ tPListObj * ppListObj ,
	_IN______ EnumListGrownDirc listGrownDirc
	)
{
	tPTraversableObj pbaseObj = NULL;
//	OBJECT_MALLOC_DECLARE();
	/*S1:ִ�б���ڵ�Ĵ�������*/
	*ppListObj =OBJECT_MALLOC(tListObj,*ppListObj);
	if (*ppListObj == NULL)
	{
		return;
	}
    pbaseObj = (tPTraversableObj) (*ppListObj);
	/*S2:ִ�и���ڵ�Ĵ�����������ȡ��ʼ��*/
	TraversableObject_Create(&pbaseObj);
	(*ppListObj)->enumListGrownDirc = listGrownDirc;
    (*ppListObj)->bNeedFreeElement  = FALSE;
	(*ppListObj)->pEndObj           = NULL;
	(*ppListObj)->pHeadObj          = NULL;             /* important- initialize to NULL! */
	(*ppListObj)->pCurrentObj       = NULL;
	(*ppListObj)->fnAdd             = _Add;
	(*ppListObj)->fnClear           = _Clear;
	(*ppListObj)->fnGetCount        = _GetCount;
	(*ppListObj)->fnGetNext         = _GetNext;
	(*ppListObj)->fnHasNext         = _HasNext;
	(*ppListObj)->fnRemove          = _Remove;
//	(*ppListObj)->fnFind            = _Element_Find;
	(*ppListObj)->fnGetHead         = _GetHead;
	(*ppListObj)->fnToArray         = _ToArray;
	/*S3:��������������*/
	pbaseObj->fnForEach             = _ForEach;
#if 0
	//pbaseObj->baseObj.fnFree = _Free;
#else
	pbaseObj->baseObj.fnOverideFree((tPObject)pbaseObj,_Free);
#endif
	pbaseObj->baseObj.Size = sizeof(tListObj);
	//pbaseObj->baseObj.fnGetSize     = _GetSize;
}

/*	
_Free
-------------------
�������б�����ͷź���
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
void _Free (
	_IN______ tPObject pObject
	)
{
	tPListObj pListObj = NULL;
	if ( pObject == NULL)
	{
		return;
	}
	pListObj = (tPListObj)pObject;	

    pListObj->fnClear(pListObj,pListObj->bNeedFreeElement); //����Ҫ�ͷ��б��нڵ���ռ���ڴ�����
#if 0
	/*S4:�ͷ��Լ���ռ���ڴ�ռ�*/
	//free(pListObj);
	//pListObj = NULL;
#endif
}

/*	
_Clear
-------------------
����������ڵ���պ���
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
void _Clear(
	_IN______ tPListObj pListObj,
	_IN______ BOOL bNeedFreeElement
	)
{
	tPListElementObj pTmpObj = NULL;
	tPListElementObj pElementObj = NULL;
	if (pListObj == NULL)
	{
		return;
	}
	if ( pListObj->fnGetCount(pListObj) != 0 )  
	{	
		pListObj->bNeedFreeElement = bNeedFreeElement;
		LL_FOREACH_SAFE(pListObj->pHeadObj,pElementObj,pTmpObj)    
		{
			LL_DELETE(pListObj->pHeadObj,pElementObj);
			//ɾ���洢�ڵ�ʱ��������ڴ�ռ�
			if ( pElementObj!= NULL)
			{
				if (pListObj->bNeedFreeElement == TRUE)
				{
					if (pElementObj->pValue != NULL)
					{
						pElementObj->pValue->fnFree(pElementObj->pValue);
					}
				}
				OBJECT_FREE(pElementObj);
			}			
		}
	}
	pListObj->pHeadObj = NULL;                  /* important- initialize to NULL! */
	pListObj->pEndObj = NULL;
	pListObj->pCurrentObj = NULL;
}



/*	
FUNCTION NAME: _GetCount
-------------------
DESCRIPTION����ȡָ���б���Ԫ�ظ���
NOTE��
REF��
------------------
Author��hepeng / whl
Date��2012-12-12
*/
INT32S _GetCount(  
	_IN______ tPListObj pListObj
	  )
{
	tPListElementObj pTmp = NULL;
	INT32U iCount = 0;
    if ((NULL == pListObj))
    {
		return -1;
    }
	/* S0: ѭ����ȡ�б���Ԫ�صĸ��� */
	pTmp = pListObj->pHeadObj;
	while (NULL != pTmp) 
	{
		iCount++;
		pTmp = pTmp->next;
	}

	pTmp = NULL;
    return (INT32S)iCount;
}


/*	
FUNCTION NAME: _HasNext
-------------------
DESCRIPTION��ָ���б��е�ǰԪ���Ƿ������һ��Ԫ��
NOTE��
REF��
------------------
Author��hepeng / whl
Date��2012-12-13
*/
BOOL _HasNext(
	_IN______ tPListObj pObj
	)
{
	tPListElementObj pTmpObj = NULL;
	if ((NULL == pObj) || (NULL == pObj->pCurrentObj))
	{
		return FALSE;
	}
	/* S1: ��ǰָ��ָ����һ��Ԫ�� */
	if (pObj->enumListGrownDirc == enNext)
	{
		pTmpObj = pObj->pCurrentObj->next;
		if (NULL == pTmpObj)
		{
			return FALSE;
		}
		pObj->pCurrentObj = pObj->pCurrentObj->next;                  //��pCurrentObj�����ƶ�
	}
	else if (pObj->enumListGrownDirc == enPrev)
	{
		pTmpObj = _SearchPrevElement(pObj,pObj->pCurrentObj);
		if (NULL == pTmpObj)
		{
			return FALSE;
		}
		pObj->pCurrentObj = pTmpObj;                                  //��pCurrentObjǰ���ƶ�
	}
	else
	{
		//do nothing
	}
	return TRUE;
}


/*	
_SearchPrevElement
-------------------
���������ҵ�ǰ�ڵ��ǰ��ڵ�
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
tPListElementObj _SearchPrevElement(
	_IN______ tPListObj pListObj,
	_IN______ tPListElementObj pCurrntElement
	)
{
	tPListElementObj pRtnElementObj = NULL;
	if (pCurrntElement == NULL || pListObj == NULL || pListObj->pHeadObj == NULL)
	{
		return NULL;
	}
	if (pCurrntElement == pListObj->pHeadObj)
	{
		return NULL;
	}
	pRtnElementObj = pListObj->pHeadObj;
	while(pRtnElementObj != NULL)
	{
		if (pRtnElementObj->next == pCurrntElement)
		{
			break;
		}
		pRtnElementObj = pRtnElementObj->next;
	}
	return pRtnElementObj;
}


/*	
FUNCTION NAME: _GetNext
-------------------
DESCRIPTION����ȡ��ǰԪ�ص���һ��Ԫ��
NOTE��
REF��
------------------
Author��hepeng / whl
Date��2012-12-13 
*/
tPObject _GetNext(
	_IN______ tPListObj pObj
	)
{
	tPListElementObj pTmp = NULL;
	if ((NULL == pObj) || (NULL == pObj->pCurrentObj))
	{
		return NULL;
	}
	/* S0: ͨ����ǰԪ�ض�ȡ��һ��Ԫ�� */
	pTmp = pObj->pCurrentObj;
    return pTmp->pValue;
}

/*	
FUNCTION NAME: _Find
-------------------
DESCRIPTION������ָ����Ԫ���Ƿ���ָ�����б���
NOTE��
REF��
------------------
Author��hepeng / whl
Date��2012-12-13
*/
tPListElementObj _Find(
	_IN______ tPListObj pListObj,
	_IN______ tPObject pObj
	)
{
	tPListElementObj pTmpObj = NULL;
	if ((NULL == pListObj) || (NULL == pObj))
	{
		return NULL;
	}	
	/* S0: ��ָ���б��б�������ָ��Ԫ�� */
	pTmpObj = pListObj->pHeadObj;
	while (pTmpObj != NULL && pTmpObj->pValue != pObj)
	{
		if (NULL != pTmpObj)
		{
			pTmpObj = pTmpObj->next;
		}
		else
		{
			return NULL;
		}
	}

	/* S1: ����ָ��Ԫ����ָ���б��е�ָ�� */
	return pTmpObj;
}


/*	
FUNCTION NAME: _Add
-------------------
DESCRIPTION����ָ���б������ָ����Ԫ��
NOTE��
REF��
------------------
Author��hepeng / whl
Date��2012-12-13
*/
void _Add(
	_IN______ tPListObj pListObj, 
	_IN______ tPObject pObj
	)
{
	tPListElementObj pNode = NULL;
	if ((NULL == pListObj)  || (NULL == pObj))
	{
		return ;
	}
	///*S0:�жϵ�ǰ��ӵ�Ԫ�����б����Ƿ����*/
	////pNode = _Find(pListObj, pObj);
	////if (pNode != NULL )
	////{
	////	//��ʱ��ӵ�Ԫ�����б����Ѵ��ڣ��������
	////	return;
	////}
	/* S1: Ϊָ����ӵ�Ԫ������ռ� */
	pNode = OBJECT_MALLOC(tListElementObj,pNode);
	if (pNode == NULL)
	{
		return;
	}
	/* S2: ��ָ��Ԫ��׷�ӵ�ָ���б��� */
	pNode->pValue = pObj;
	LL_APPEND(pListObj->pHeadObj, pNode);
	/* S2-1: ��pEnd���г�ʼ�� */
	pListObj->pEndObj = pNode;
	/* S2-2: ����List���͸�pCurrentObj���г�ʼ��  */
	if (pListObj->enumListGrownDirc == enNext)
	{
		if (pListObj->pCurrentObj == NULL)   
		{
			pListObj->pCurrentObj = pListObj->pHeadObj;  //ֻ���pCurrentһ�θ�ֵ����
		}
		else
		{
			//do nothing
		}
	}
	else if (pListObj->enumListGrownDirc == enPrev)
	{
		pListObj->pCurrentObj = pListObj->pEndObj;      //��pEndָ���ڱ仯������pCurrent��ʼҲ����ű仯
	}
	else
	{
		//do nothing
	}
}


/*	
FUNCTION NAME: _ForEach
-------------------
DESCRIPTION��
NOTE��
REF��
------------------
Author��
Date��
*/
void _ForEach(
	_IN______ tPTraversableObj pTraversedObj,
	_IN______ tfnTraverseObj fnTraverseAction
	)
{
	tPListObj pListObj = (tPListObj)pTraversedObj;
	tPListElementObj pListElement = NULL;

	if ( pListObj->fnGetCount(pListObj) != 0 )
	{		
		LL_FOREACH(pListObj->pHeadObj,pListElement)
		{
			fnTraverseAction.fnTraverse((tPTraversableObj)pListObj,pListElement->pValue,fnTraverseAction.pTraversingObject,fnTraverseAction.u32Param);
		}
	}	
}

/*	
FUNCTION NAME: _Remove
-------------------
DESCRIPTION����ָ���б����Ƴ�ָ����Ԫ��
NOTE��
REF��
------------------
Author��hepeng / whl
Date��2012-12-13
*/
void _Remove(
	_IN______ tPListObj pListObj, 
	_IN______ tPObject pObj
	)
{
	tPListElementObj pRtnElementObj = NULL;
	if ((NULL == pListObj) || (NULL == pListObj->pHeadObj) || (NULL == pObj))
	{
		return ;
	}
	pRtnElementObj = _Find(pListObj, pObj);
	/* S1: ���б��в��ң���Ҫɾ����Ԫ��*/
	if ( pRtnElementObj != NULL)  
	{
		if (pListObj->enumListGrownDirc == enNext)
		{
			//do nothing
		}
		else if (pListObj->enumListGrownDirc == enPrev)
		{
			if (pRtnElementObj == pListObj->pEndObj)
			{
				pListObj->pEndObj = _SearchPrevElement(pListObj,pRtnElementObj); //��pEndָ��ǰ��
			}
		}
		else
		{
			//do nothing
		}
		/* S3: ���б���ɾ��Ԫ��*/
		LL_DELETE(pListObj->pHeadObj, pRtnElementObj);
		/* S4: ���б���ɾ��Ԫ��*/
		OBJECT_FREE(pRtnElementObj);
	}
	else
	{
		// ���б���û��pObjԪ�أ���ӵ�����Ϣ
	}
}


/*	
_Element_Find
-------------------
����������Ԫ��ִ�����������ط����������б�
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
//tPListObj _Element_Find(
//	_IN______ tPListObj pListObj,
//	_IN______ tfnListElementMatch fnListElementMatch
//	)
//{
//	tPListObj pRtnListObj = NULL;
//	tPListElementObj pListElement = NULL;
//	if (pListObj == NULL || fnListElementMatch == NULL)
//	{
//		return NULL;
//	}
//	List_Create(&pRtnListObj,enNext);
//	if (pRtnListObj == NULL)
//	{
//		return NULL;
//	}
//	if ( pListObj->fnGetCount(pListObj) != 0 )
//	{		
//		LL_FOREACH(pListObj->pHeadObj,pListElement)
//		{
//			if (fnListElementMatch(pListElement->pValue))
//			{
//				pRtnListObj->fnAdd(pRtnListObj,(tPObject)pListElement->pValue);
//			}
//		}
//	}	
//	return pRtnListObj;
//}

/*	
_GetHead
-------------------
��������ȡ�б�ͷ���Ԫ��
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
tPObject _GetHead(
	_IN______ tPListObj pListObj
	)
{
	tPObject pRtnObj = NULL;
	if (pListObj == NULL || pListObj->pHeadObj == NULL)
	{
		return NULL;
	}
	if (pListObj->enumListGrownDirc == enNext)
	{
		pListObj->pCurrentObj = pListObj->pHeadObj;    //���б�����ΪenNextʱ��pCurrentObjָ��������ΪpHead;
		pRtnObj = pListObj->pHeadObj->pValue;
	}
	else if (pListObj->enumListGrownDirc == enPrev)
	{
		if (pListObj->pEndObj == NULL)
		{
			return NULL;
		}
		pListObj->pCurrentObj = pListObj->pEndObj;     //���б�����ΪenPrevʱ��pCurrentObjָ��������ΪpEnd;
		pRtnObj = pListObj->pEndObj->pValue;
	}
	return pRtnObj;

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
// INT32U _GetSize(
//	_IN______ tPObject pObject
//	)
// {
//	 tPListObj pListObj = NULL;
//	 if (pObject == NULL)
//	 {
//		 return  0;
//	 }
//	 pListObj = (tPListObj)pObject;
//	 return pListObj->baseObj.baseObj.Size;
// }


 /*	
 _ToArray
 -------------------
 ���������б��нڵ�����תΪ�ֽ�����
 NOTE��
 REF��
 ------------------
 Author��whl
 Date��2012��12��10��
 */
 tPArrayObj _ToArray(
	_IN______ tPListObj pListObj
	)
 {
	 tPObject pTempData = NULL;
	 INT32S ListCount = 0;
	 tPArrayObj pRtnArrayObj = NULL;
	 if (pListObj == NULL)
	 {
		 return NULL;
	 }
	 ListCount = pListObj->fnGetCount(pListObj);
	 if ( ListCount<= 0)
	 {
		 return NULL;
	 }
	//����List�нڵ����ݴ����Array��
     Array_Create(&pRtnArrayObj);
	 pTempData =  pListObj->fnGetHead(pListObj);
	 pRtnArrayObj->fnAdd(pRtnArrayObj,pTempData);
	 while(pListObj->fnHasNext(pListObj))
	 {
		 pTempData = pListObj->fnGetNext(pListObj);
		 pRtnArrayObj->fnAdd(pRtnArrayObj,pTempData);
	 }
	 return pRtnArrayObj;

 }
/***************************************---��List������ʵ�� End---**********************************/
