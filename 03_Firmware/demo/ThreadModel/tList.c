//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                          tList.c
//描述： 威锐达列表类型头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月10日                  模块创建                whl
//

#include "tList.h"
#include "utlist.h"
#include "BaseType.h"


/***************************************---↑List对象函数声明 BEGIN---********************************/

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


/***************************************---↑List对象函数声明 End---**********************************/

/***************************************---↑List对象函数实现 BEGIN---********************************/

/*	
ListCreate
-------------------
描述：列表元素类型对象
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
*/
void List_Create(
	_IN______ tPListObj * ppListObj ,
	_IN______ EnumListGrownDirc listGrownDirc
	)
{
	tPTraversableObj pbaseObj = NULL;
//	OBJECT_MALLOC_DECLARE();
	/*S1:执行本身节点的创建函数*/
	*ppListObj =OBJECT_MALLOC(tListObj,*ppListObj);
	if (*ppListObj == NULL)
	{
		return;
	}
    pbaseObj = (tPTraversableObj) (*ppListObj);
	/*S2:执行父类节点的创建函数，获取初始化*/
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
	/*S3:重载型属性设置*/
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
描述：列表对象释放函数
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
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

    pListObj->fnClear(pListObj,pListObj->bNeedFreeElement); //←需要释放列表中节点所占的内存数据
#if 0
	/*S4:释放自己所占的内存空间*/
	//free(pListObj);
	//pListObj = NULL;
#endif
}

/*	
_Clear
-------------------
描述：链表节点清空函数
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
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
			//删除存储节点时所申请的内存空间
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
DESCRIPTION：获取指定列表中元素个数
NOTE：
REF：
------------------
Author：hepeng / whl
Date：2012-12-12
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
	/* S0: 循环读取列表中元素的个数 */
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
DESCRIPTION：指定列表中当前元素是否存在下一个元素
NOTE：
REF：
------------------
Author：hepeng / whl
Date：2012-12-13
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
	/* S1: 当前指针指向下一个元素 */
	if (pObj->enumListGrownDirc == enNext)
	{
		pTmpObj = pObj->pCurrentObj->next;
		if (NULL == pTmpObj)
		{
			return FALSE;
		}
		pObj->pCurrentObj = pObj->pCurrentObj->next;                  //将pCurrentObj后向移动
	}
	else if (pObj->enumListGrownDirc == enPrev)
	{
		pTmpObj = _SearchPrevElement(pObj,pObj->pCurrentObj);
		if (NULL == pTmpObj)
		{
			return FALSE;
		}
		pObj->pCurrentObj = pTmpObj;                                  //将pCurrentObj前向移动
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
描述：查找当前节点的前向节点
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
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
DESCRIPTION：获取当前元素的下一个元素
NOTE：
REF：
------------------
Author：hepeng / whl
Date：2012-12-13 
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
	/* S0: 通过当前元素读取下一个元素 */
	pTmp = pObj->pCurrentObj;
    return pTmp->pValue;
}

/*	
FUNCTION NAME: _Find
-------------------
DESCRIPTION：查找指定的元素是否在指定的列表中
NOTE：
REF：
------------------
Author：hepeng / whl
Date：2012-12-13
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
	/* S0: 在指定列表中遍历查找指定元素 */
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

	/* S1: 返回指定元素在指定列表中的指针 */
	return pTmpObj;
}


/*	
FUNCTION NAME: _Add
-------------------
DESCRIPTION：在指定列表中添加指定的元素
NOTE：
REF：
------------------
Author：hepeng / whl
Date：2012-12-13
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
	///*S0:判断当前添加的元素在列表中是否存在*/
	////pNode = _Find(pListObj, pObj);
	////if (pNode != NULL )
	////{
	////	//此时添加的元素在列表中已存在，无需添加
	////	return;
	////}
	/* S1: 为指定添加的元素申请空间 */
	pNode = OBJECT_MALLOC(tListElementObj,pNode);
	if (pNode == NULL)
	{
		return;
	}
	/* S2: 将指定元素追加到指定列表中 */
	pNode->pValue = pObj;
	LL_APPEND(pListObj->pHeadObj, pNode);
	/* S2-1: 对pEnd进行初始化 */
	pListObj->pEndObj = pNode;
	/* S2-2: 根据List类型给pCurrentObj进行初始化  */
	if (pListObj->enumListGrownDirc == enNext)
	{
		if (pListObj->pCurrentObj == NULL)   
		{
			pListObj->pCurrentObj = pListObj->pHeadObj;  //只需给pCurrent一次赋值即可
		}
		else
		{
			//do nothing
		}
	}
	else if (pListObj->enumListGrownDirc == enPrev)
	{
		pListObj->pCurrentObj = pListObj->pEndObj;      //因pEnd指针在变化，所以pCurrent初始也需跟着变化
	}
	else
	{
		//do nothing
	}
}


/*	
FUNCTION NAME: _ForEach
-------------------
DESCRIPTION：
NOTE：
REF：
------------------
Author：
Date：
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
DESCRIPTION：在指定列表中移除指定的元素
NOTE：
REF：
------------------
Author：hepeng / whl
Date：2012-12-13
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
	/* S1: 在列表中查找，需要删除的元素*/
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
				pListObj->pEndObj = _SearchPrevElement(pListObj,pRtnElementObj); //←pEnd指针前移
			}
		}
		else
		{
			//do nothing
		}
		/* S3: 在列表中删除元素*/
		LL_DELETE(pListObj->pHeadObj, pRtnElementObj);
		/* S4: 在列表中删除元素*/
		OBJECT_FREE(pRtnElementObj);
	}
	else
	{
		// 在列表中没有pObj元素，添加调试信息
	}
}


/*	
_Element_Find
-------------------
描述：按照元素执行条件，返回符合条件的列表
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
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
描述：获取列表头结点元素
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
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
		pListObj->pCurrentObj = pListObj->pHeadObj;    //当列表类型为enNext时，pCurrentObj指针重新置为pHead;
		pRtnObj = pListObj->pHeadObj->pValue;
	}
	else if (pListObj->enumListGrownDirc == enPrev)
	{
		if (pListObj->pEndObj == NULL)
		{
			return NULL;
		}
		pListObj->pCurrentObj = pListObj->pEndObj;     //当列表类型为enPrev时，pCurrentObj指针重新置为pEnd;
		pRtnObj = pListObj->pEndObj->pValue;
	}
	return pRtnObj;

}

/*	
_GetSize
-------------------
描述：获取对象所占内存大小
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
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
 描述：将列表中节点类型转为字节数组
 NOTE：
 REF：
 ------------------
 Author：whl
 Date：2012年12月10日
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
	//←将List中节点数据存放置Array中
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
/***************************************---↑List对象函数实现 End---**********************************/
