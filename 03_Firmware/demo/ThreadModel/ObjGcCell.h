//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                    ObjGcCell.h
//描述： 威锐达可遍历类型头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2013年1月7日                  模块创建                Martin.lee

#ifndef __HH_ObjGCCELL_H_HH__
#define __HH_ObjGCCELL_H_HH__


#include "BaseType.h"
#include "ModuleCommon.h"
#include "tObject.h"
#include "tList.h"


#define  OBJGCCELL_MALLOC_FAILED      1       //←内存申请失败
#define  OBJGCCELL_PARAM_ILLEGAL      2       //←参数非法
#define  OBJGCCELL_PARAM_NULL         3       //←参数为空


typedef struct _tagObjGcCell tObjGcCell,*tPObjGcCell;


typedef void (*tfnObjGcFree)(tPObjGcCell pGcObj);
typedef void (*tfnObjGcRegist)(tPObjGcCell pGcObj,tPObject pNeedGcObj);


/*	
tObjGcCell
-------------------
描述：垃圾回收单元类型定义
NOTE：
REF：
------------------
Author：whl
Date：2013年1月7日
*/
struct _tagObjGcCell
{
	tListObj       innerObjList;   //← 登记需要回收的对象列表
	tfnObjGcRegist fnRegist;    //←登记需要回收的对象
	tfnObjGcFree   fnFree;        //← 回收操作
};


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
    _IN______ tPObjGcCell* ppGcCell,
    _OUT____  PINT16U pu16ErrorCode
    );



#endif

