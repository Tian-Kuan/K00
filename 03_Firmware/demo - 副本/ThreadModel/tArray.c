//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                          tArray.c
//描述： 威锐达列表类型头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月17日                  模块创建                whl / hepeng
//

#include <string.h>
#include "tArray.h"

/***************************************---↑ Array对象函数声明 BEGIN---********************************/
static void _Add(tPArrayObj,tPObject);                                                   // 增加操作
static tPObject _GetAtIndex(tPArrayObj,INT16U);                                          // 获取指定索引位置上的数据
//static void _RegisterCallBack(tPArrayObj,tfnOnCollectionFull);                           // 注册相应的回调函数
static INT32S _GetCount(tPArrayObj);                                                     // 获取Array大小
static void _Free (tPObject);                                                            // 释放存储空间
//static void _RemoveAll(tPArrayObj);                                                      // 清空Array
static tPObject* _GetBuf(tPArrayObj);                                                    // 获取Array中的数据

static void _ForEach(tPTraversableObj pTraversedObj,tfnTraverseObj fnTraverseAction);    // 重载ForEach方法

/***************************************---↑ Array对象函数声明 End---**********************************/



/***************************************---↑ Array对象函数实现 BEGIN---********************************/

/*	
Array_Create
-------------------
描述：Array对象创建
NOTE：
REF：
------------------
Author：whl
Date：2012年12月18日
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
	/* S1:执行本身节点的创建函数 */
	*ppArrayObj = OBJECT_MALLOC(tArrayObj,*ppArrayObj);
	if (*ppArrayObj == NULL)
	{
		return;
	}
	/* S2:执行父类节点的创建函数 */
	pBaseObj = (tPTraversableObj) (*ppArrayObj);
	TraversableObject_Create(&pBaseObj);
	pObject= (tPObject)(*ppArrayObj);
	if ((*ppArrayObj)->pArrayData == NULL)
	{
		OBJECT_MALLOC_FREE(pObject);
		return ;
	}
	/* S3: 变量和函数初始化 */
	(*ppArrayObj)->u16CurrentDataLength = 0;
//	(*ppArrayObj)->fnOnCollectionFull.fnNotify = NULL;
//	(*ppArrayObj)->fnOnCollectionFull.pListenner = NULL;
	(*ppArrayObj)->fnGetAtIndex = _GetAtIndex;
	(*ppArrayObj)->fnGetCount = _GetCount;
	(*ppArrayObj)->fnGetBuf = _GetBuf;
	(*ppArrayObj)->fnAdd = _Add;
//	(*ppArrayObj)->fnRemoveAll = _RemoveAll;
//	(*ppArrayObj)->fnCollectionArrayRegisterCallBack = _RegisterCallBack;
	/*S4:重载型属性设置*/
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
描述：Array释放函数
NOTE：
REF：
------------------
Author：whl / hepeng
Date：2012年12月18日
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

	/* S1: 释放Array中的元素 */
	pArrayObj = (tPArrayObj)pObject;	
	if (pArrayObj->pArrayData != NULL)
	{
		OBJECT_FREE(pArrayObj->pArrayData);
	}
#if 0
	/* S2: 释放自己所占的内存空间 */
	//free(pArrayObj);
	//pArrayObj = NULL;
#endif
}


/*	
_RemoveAll
-------------------
描述：删除数组内所有元素数据
NOTE：
REF：
------------------
Author：whl / hepeng
Date：2012年12月18日
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

//	/* S0: 将Array中的数据清空 */
//	for (u16Index = 0;u16Index < pArrayObj->u16CurrentDataLength;u16Index++)
//	{
//		pArrayObj->pArrayData[u16Index] = NULL;
//	}
//}



/*	
FUNCTION NAME: _GetCount
-------------------
DESCRIPTION：获取指定列表中元素个数
NOTE：
REF：
------------------
Author：hepeng / whl
Date：2012-12-18
*/
INT32S _GetCount(  
	_IN______ tPArrayObj pArrayObj
	)
{
	if (pArrayObj == NULL || pArrayObj->pArrayData == NULL )
	{
		return -1;
	}

	/* S1: 返回当前Array的大小 */
	return pArrayObj->u16CurrentDataLength;
}

/*	
FUNCTION NAME: _GetBuf
-------------------
DESCRIPTION：获取Array中的数据
NOTE：
REF：
------------------
Author：hepeng / whl
Date：2012-12-18
*/
tPObject*  _GetBuf (
	_IN______ tPArrayObj pArrayObj
	)
{
	if ( NULL == pArrayObj || pArrayObj->pArrayData == NULL)
	{
		return NULL;
	}

	/* S0: 返回Array中数据的首地址 */
	return pArrayObj->pArrayData;
}


/*	
FUNCTION NAME: _ForEach
-------------------
DESCRIPTION：遍历Array
NOTE：
REF：
------------------
Author：whl
Date：2012-12-18
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
描述：注册数据存满产生的回调函数
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
*/
//void _RegisterCallBack(
//	_IN______ tPArrayObj pArrayObj,
//	_IN______ tfnOnCollectionFull fnOnCollectionFull
//)
//{
//	/*S1:参数验证*/
//	if (pArrayObj == NULL 
//		|| fnOnCollectionFull.fnNotify == NULL 
//		|| fnOnCollectionFull.pListenner == NULL
//	)
//	{
//		return;
//	}
//	/*S2:注册回调函数*/
//	memcpy(&(pArrayObj->fnOnCollectionFull),&fnOnCollectionFull,sizeof(tfnOnCollectionFull));

//}


/*	
_Add
-------------------
描述：添加元素
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
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
	/*判断是否加满*/
	if (pArrayObj->u16CurrentDataLength == ARRAY_OBJECT_MAX_LENGTH)
	{
		//←此时元素已加满，产生回调函数
//		if (pArrayObj->fnOnCollectionFull.fnNotify != NULL 
//			&& pArrayObj->fnOnCollectionFull.pListenner != NULL)
//		{
//			//如果注册过回调函数，产生回调进行通知
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
描述：添加元素
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
*/
tPObject _GetAtIndex(
	_IN______ tPArrayObj pArrayObj,
	_IN______ INT16U u16DataIndex
	)
{
	/*S1:参数验证*/
	if (pArrayObj == NULL
		|| u16DataIndex >= ARRAY_OBJECT_MAX_LENGTH)
	{
		return NULL;
	}
	/*S2:检查对象本身是否出错*/
	if (pArrayObj->u16CurrentDataLength > ARRAY_OBJECT_MAX_LENGTH)
	{
		return NULL;
	}
	/*S3:获取指定位置的值*/
	return pArrayObj->pArrayData[u16DataIndex];
	
}

/***************************************---↑ Array对象函数实现 End---**********************************/
