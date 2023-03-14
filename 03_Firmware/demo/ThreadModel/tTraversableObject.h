//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                    tTraversableObject.h
//描述： 威锐达可遍历类型头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月10日                  模块创建                whl
//

#ifndef _HH_TTRAVERSABLEOBJECT_HH_
#define  _HH_TTRAVERSABLEOBJECT_HH_


#include "BaseType.h"
#include "tObject.h"



typedef struct _tagstructTraversableObject tTraversableObj,*tPTraversableObj;

typedef struct _tagFnTraverse tfnTraverseObj,*tPfnTraverseObj;

typedef void (*tfnxTraverse)(tPTraversableObj pTraversableObj ,tPObject pElementObj,tPObject pTraversingObj,INT32U u32Param);


struct _tagFnTraverse
{
    INT32U  u32Param;
	tPObject pTraversingObject;
	tfnxTraverse fnTraverse;
};



typedef void (*tfnForEach)(tPTraversableObj,tfnTraverseObj);    //遍历函数←此时执行具体对象的遍历方法







/*
tTraversableObject
-------------------
描述：可遍历的数据对象
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
*/	
struct _tagstructTraversableObject
{
	tObject    baseObj;       //基本数据类型  
	tfnForEach fnForEach; 
};


INT32S TraversableObject_Create(tPTraversableObj* ppTraversableObject); 

#endif


