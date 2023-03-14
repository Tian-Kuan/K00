//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                    CollectionPackage.c
//描述： 威锐达集合类型实现文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月10日                  模块创建                whl
//
#include "tTraversableObject.h"


/***************************************---↑Traversable对象函数声明 BEGIN---********************************/

static void TraversableObject_Free(tPObject  pTraversableObject); 
static void ForEach(tPTraversableObj pTraversableObject,tfnTraverseObj fnAction);
//static INT32U _GetSize(tPObject pObject);

/***************************************---↑Traversable对象函数声明 End  ---********************************/





/***************************************---↑Traversable对象函数实现 BEGIN  ---*******************************/

/*
CollectionPackage_Init
-------------------
描述：模块级初始化函数
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
*/	
void CollectionPackage_Init(void)
{
	//do nothing
}


/*
TraversableObject_Create
-------------------
描述：可遍历的数据对象创建函数
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
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
	/*S1:为自身点分配内存空间*/
    pBaseObj = (tPObject)*ppTraversableObject;
	(*ppTraversableObject) =  OBJECT_MALLOC(tTraversableObj,(*ppTraversableObject));
	if (*ppTraversableObject == NULL)
	{
		return -1;
	}
	/*S2:执行父类构建方法已获得对父类的创建及初始化方法的继承*/
	Object_Create(&pBaseObj);		
	(*ppTraversableObject)->fnForEach = ForEach;
	/*S3:重载free函数*/
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
描述：获取对象所占内存大小
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
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
描述：可遍历的数据对象free函数
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
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
	///*S2:先释放子节点*/
	//free(pTraversableObject);
#endif
}

/*
ForEach
-------------------
描述：可遍历的数据对象遍历函数
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
*/	
void ForEach(
	_IN______ tPTraversableObj  pTraversableObject,
	_IN______ tfnTraverseObj fnAction
	)
{
	//do nothing
}



/***************************************---↑Traversable对象函数实现   End  ---********************************/
