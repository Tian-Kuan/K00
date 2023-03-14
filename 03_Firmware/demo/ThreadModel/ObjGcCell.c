//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                    ObjGcCell.c
//描述： 威锐达可遍历类型实现文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2013年1月7日                  模块创建                Martin.lee

#include "ObjGcCell.h"



static void _ObjGcFree(tPObjGcCell pGcObj);
static void _ObjGcRegist(tPObjGcCell pGcObj,tPObject pNeedGcObj);

/*	
_SET_ERROR_CODE
-------------------
描述：错误码设置宏定义函数
NOTE：
REF：
------------------
Author：whl
Date：2013年1月7日
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
描述：垃圾回收单元创建函数
NOTE：
REF：
------------------
Author：whl
Date：2013年1月7日
*/
void ObjGcCell_Create(
    _IN______ tPObjGcCell *ppGcCell,
    _OUT____ PINT16U pu16ErrorCode
    )
{
	//OBJECT_MALLOC_DECLARE();
	/*S0:参数验证*/
	if (pu16ErrorCode == NULL)
	{
		return;
	}
	/*S1:为自身分配内存*/
	*ppGcCell = OBJECT_MALLOC(tObjGcCell,*ppGcCell);
	if (*ppGcCell == NULL)
	{
		_SET_ERROR_CODE(pu16ErrorCode,OBJGCCELL_MALLOC_FAILED);
		return;
	}
	/*S2:父类初始化*/
	List_Create((tPListObj*)ppGcCell,enNext);
	/*S3:自身初始化*/
	(*ppGcCell)->fnFree   = _ObjGcFree;
	(*ppGcCell)->fnRegist = _ObjGcRegist;
	((tPObject)((*ppGcCell)))->Size   = sizeof(tObjGcCell);
	_SET_ERROR_CODE(pu16ErrorCode,ERROR_WI_NORMAL);
}

/*	
_ObjGcFree
-------------------
描述：垃圾回收单元Free函数
NOTE：
REF：
------------------
Author：whl
Date：2013年1月7日
*/
void _ObjGcFree(tPObjGcCell pGcObj)
{
	tPListObj pListObj = NULL;
	tPObject pTemObj = NULL;
	/*S1:参数验证*/
	if (pGcObj == NULL)
	{
		return;
	}
	/*S2:删除列表中存储的对象*/
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
	/*S3:删除自身*/
	pListObj->fnClear(pListObj,FALSE); //←删除列表中节点类型
	OBJECT_FREE(pGcObj);

}


/*	
_ObjGcFree
-------------------
描述：垃圾回收单元Free函数
NOTE：
REF：
------------------
Author：whl
Date：2013年1月7日
*/
void _ObjGcRegist(
		tPObjGcCell pGcObj,
		tPObject pNeedGcObj
		)
{
	tPListObj pListObj = NULL;
	/*S1:参数验证*/
	if (pGcObj == NULL || pNeedGcObj == NULL)
	{
		return;
	}
	/*S2:将对象添加至列表中*/
	pListObj = (tPListObj)pGcObj;
	pListObj->fnAdd(pListObj,pNeedGcObj);
}
