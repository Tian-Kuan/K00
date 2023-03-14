//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                       tObject.c
//描述： 威锐达基本数据类型实现文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月10日                  模块创建                whl
//

#include "tObject.h"
#include "led.h"

//static BOOL _Equal (tPObject pRootObj,tPObject pDstObject);                          //判断根对象哈希码是否相同
//static INT32S _GetHashCode(tPObject pRootObj);  
static void _Free (tPObject pRootObj);
//static INT32U _GetSize(tPObject pRootObj);
static BOOL _OverideFree (tPObject pRootObj,tfnFree fnfnFree);                       //←对象Free函数重载方法

/*-------------------------------内部方法宣称区--------------------------BEGIN------------*/
static BOOL _InnerMethod_RequireFree(tPObject pObject);
static BOOL _InnerMethod_CanFree(tPObject pObject);
static void _InnerMethod_DoDelayFree(tPObject pObject);
/*-------------------------------内部方法宣称区---------------------------END------------*/

/***************************************---↑Object根对象函数实现区 BEGIN  ---******************************/
 /*
 Object_Create
 -------------------
 描述：基本对象创建函数
 NOTE：
 REF：
 ------------------
 Author：whl
 Date：2012年12月10日
 */
  void Object_Create(
	 _IN______ tPObject* ppObject
	 )
 {
	 
	(*ppObject) = OBJECT_MALLOC(tObject,*ppObject);
	if (*ppObject == NULL)
	{
		return;
	}
	
	(*ppObject)->Size = sizeof(tObject);
//	(*ppObject)->HashCode = 0;	
	(*ppObject)->u8CurntInheritObjCount = 0;
	(*ppObject)->bNeedDelayFree = FALSE;
	(*ppObject)->fnoDelayFree.pFreeObj = NULL;
	//(*ppObject)->fnGetSize = _GetSize;
//	(*ppObject)->fnEqual = _Equal;
	//(*ppObject)->fnGetHashCode = _GetHashCode;
	(*ppObject)->fnFree = _Free;
	(*ppObject)->fnOverideFree = _OverideFree;
	(*ppObject)->fnRequireFree = _InnerMethod_RequireFree;
	(*ppObject)->fnDoDelayFree = _InnerMethod_DoDelayFree;

 }


 /*
 Equal
 -------------------
 描述：对象比较函数
 NOTE：相同返回True，否则Else
 REF：
 ------------------
 Author：whl
 Date：2012年12月10日
 */
// BOOL _Equal (
//	 _IN______ tPObject pObject,
//	 _IN______ tPObject pDstObject
//	 )
// {
//	 if (pObject == NULL || pDstObject == NULL)
//	 {
//		 return FALSE;
//	 }
//	 return (pObject->HashCode == pDstObject->HashCode) ? TRUE:FALSE;
// }

 /*
 GetHashCode
 -------------------
 描述：获取对象所在的内存码
 NOTE：
 REF：
 ------------------
 Author：whl
 Date：2012年12月10日
 */
// INT32S _GetHashCode(
//	 _IN______ tPObject pObject
//	 )
// {
//	 if (pObject == NULL)
//	 {
//		 return -1;
//	 }
//	 return (INT32S)pObject; //←暂时返回该对象在内存中的地址，应该返回该对象对应的哈希吗
//	 //return pObject->HashCode;

// }

 /*
 GetHashCode
 -------------------
 描述：获取对象所在的内存码
 NOTE：
 REF：
 ------------------
 Author：whl
 Date：2012年12月10日
 */
// INT32U _GetSize(
//	 _IN______ tPObject pObject
//	 )
// {
//	 if (pObject == NULL)
//	 {
//		 return 0;
//	 }
//	 return pObject->Size; 
// }

 /*
 Object_Free
 -------------------
 描述：释放对象所在的内存
 NOTE：
 REF：
 ------------------
 Author：whl
 Date：2012年12月10日
 */
 void _Free(
	 _IN______ tPObject pObject
	 )
 {
	 tfnFree _InheritObjFree = NULL;
	 /*S1:参数验证*/
//	 if (pObject == NULL)
//	 {
//		 return;
//	 }
	 /*S2:判断是否可以销毁*/
	 if (pObject->fnRequireFree != NULL && _InnerMethod_CanFree(pObject)==TRUE)
	 {
		 /*S3:倒序调用继承树上的析构函数*/
		 if (/*pObject->_InnerFreesFIFO != NULL &&*/ pObject->u8CurntInheritObjCount > 0)
		 {
				 while(pObject->u8CurntInheritObjCount > 0)
				 {
					 _InheritObjFree = pObject->_InnerFreesFIFO[pObject->u8CurntInheritObjCount-1];
					 _InheritObjFree(pObject);
					 pObject->u8CurntInheritObjCount--;
				 }
		 }
		 /*S4:释放自身*/
		 OBJECT_FREE(pObject);
	 }	 
 }

/*
 _OverideFree
 -------------------
 描述：重载对象Free函数
 NOTE：重载成功返回TRUE，否则返回FALSE
 REF：
 ------------------
 Author：whl
 Date：2013年1月7日
 */
BOOL _OverideFree(
	    _IN______ tPObject pObject,tfnFree fnFree
		)
{
//	if (pObject == NULL 
//		|| fnFree == NULL
//		|| pObject->_InnerFreesFIFO == NULL)
//	{
//		return FALSE;
//	}
	if (pObject->u8CurntInheritObjCount > TYPE_INHERIT_TREE_MAX_LEVELS) //←超过继承树规定的最大深度
	{
		return FALSE;
	}
	pObject->_InnerFreesFIFO[pObject->u8CurntInheritObjCount] = fnFree;
	pObject->u8CurntInheritObjCount++;
	return TRUE;
}



/*-------------------------------内部私有方法---------------------------BEGIN------------*/

/*
  _InnerMethod_DoDelayFree
  -----------------------
  描述： 执行推迟销毁方法。该方法将判断是否推迟销毁，若是则实际执行free
  作者： Martin.lee
  日期： 2013-1-14
*/
void _InnerMethod_DoDelayFree(tPObject pObject)
{
	/*S1:参数验证*/
	if (pObject == NULL ||  pObject->bNeedDelayFree == FALSE)
	{
		return;
	}

	/*S2:执行实际的销毁*/	
	if (pObject->fnoDelayFree.pFreeObj != NULL)
	{
		/*S2-1:若存在用户定义的free则调用之*/
		pObject->fnoDelayFree.fnFree(pObject->fnoDelayFree.pFreeObj,pObject->fnoDelayFree.u32Param);
	}
	else
	{
		/*S2-2:不存在用户定义的free则直接调用对象的销毁方法*/
		pObject->bNeedDelayFree = FALSE;
		pObject->fnFree(pObject);
	}	
}

/*
  _InnerMethod_RequireFree
  -----------------------
  描述： 返回是否可以销毁。返回True则表示可以销毁
  作者： Martin.lee
  日期： 2013-1-14
*/
BOOL _InnerMethod_RequireFree(tPObject pObject)
{
	/*S1:参数验证*/
	if (pObject == NULL)
	{
		return FALSE;
	}

	/*S2:直接返回[是否需要推迟Free]标识本身(取反）*/
	return pObject->bNeedDelayFree==TRUE?FALSE:TRUE;
}

/*
  _InnerMethod_CanFree
  -----------------------
  描述： 遍历对象继承体系Free链已判定是否可以Free
  作者： Martin.lee
  日期： 2013-1-14
*/
BOOL _InnerMethod_CanFree(tPObject pObject)
{
	/*S1:参数验证*/
	if (pObject != NULL && pObject->fnRequireFree!= NULL)
	{
		/*S2:调用请求free方法已获得可否销毁信息*/
		return pObject->fnRequireFree(pObject);
	}
	return FALSE;
}
/*-------------------------------内部私有方法---------------------------END------------*/

/***************************************---↑Object根对象定义 END---********************************/
