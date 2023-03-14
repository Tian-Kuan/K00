//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                       tObject.h
//描述： 威锐达根对象数据类型头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月10日                  模块创建                whl
//


#ifndef __HH_TOBJECT_HH__
#define __HH_TOBJECT_HH__

#include "BaseType.h"
#include "DBG_Utility_Malloc.h"


#if TARGETOS==X86
	#ifndef BASETYPE_WINAPI 
		#ifdef  BaseType_DLL_
			#define BASETYPE_WINAPI  _declspec(dllexport)	
		#else
	       #define BASETYPE_WINAPI  _declspec(dllimport)
		#endif
	#endif
#else
	#ifndef BASETYPE_WINAPI
		#define BASETYPE_WINAPI
	#endif
#endif

/***************************************---↑Object根对象定义 BEGIN---********************************/


#define TYPE_INHERIT_TREE_MAX_LEVELS                   ( 5 )                //←类型继承树最大深度    hepeng moddify 5->10 on 2013.5.3 11:22


typedef struct _tagStructObject tObject,*tPObject;


//typedef BOOL (*tfnEqual)(tPObject pRootObj,tPObject pCurrentObj);
typedef INT32S (*tfnGetHashCode)(tPObject pRootObj );
typedef void (*tfnFree)(tPObject pRootObj);
typedef INT32U (*tfnGetSize)(tPObject pRootObj);
typedef BOOL (*tfnOverideFree)(tPObject pRootObj ,tfnFree fnFree);   
typedef BOOL (*tfnRequireFree)(tPObject pObject); //←请求需要销毁 若返回true则继续销毁否则被延迟
typedef void (*tfnDelayFreeOccur)(tPObject pObject,INT32U u32Param);  //← 推迟销毁发生方法

typedef void (*tfnDoDelayFree)(tPObject pObject);   //← 执行推迟销毁

/*
  tfnoDelayFree
  -----------------------
  描述： 推迟销毁对象
  作者： Martin.lee
  日期： 2013-1-14
*/
typedef struct _tagDelayFree
{
	tPObject  pFreeObj;
	INT32U    u32Param;
	tfnDelayFreeOccur fnFree;
}tfnoDelayFree;

/*
tObject
-------------------
描述：基本对象根类
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
*/
struct _tagStructObject
{
	INT16U         Size;
//	INT32U         HashCode;
	BOOL           bNeedDelayFree;                                   //[Private]←是否推迟Free的标记，default is False                     
	INT8U          u8CurntInheritObjCount;                           //[Private]←指向当前继承对象析构函数的索引
	tfnFree        _InnerFreesFIFO[TYPE_INHERIT_TREE_MAX_LEVELS];    //[Private]←对象类型继承树上析构函数列表
//	tfnEqual       fnEqual;                                          //[Public] ←判断两个对象实例是否相等的函数（比较对象所在的哈希码）
	tfnFree        fnFree;                                           //[Public] ←根对象析构函数(用于根对象)
	tfnOverideFree fnOverideFree;                                    //[Public] ←用于对象继承树上析构函数重载
	tfnoDelayFree  fnoDelayFree;                                     /*[Protected] 推迟销毁方法对象*/ 
	tfnRequireFree fnRequireFree;                                    /*[Protected] 请求销毁.返回true为可销毁.*/	
	tfnDoDelayFree fnDoDelayFree;                                    /*[Protected] 执行推迟销毁*/
//	tfnGetSize fnGetSize;                                            //[Public] ←获取根对象所占的内存大小
//	tfnGetHashCode fnGetHashCode;	                                 //[Public] ←获取根对象的哈希码（暂时返回其对象在内存的地址）
};


#define OBJECT_DECLARE(Type,pObject)   {\
	(Type*)pObject;\
	pObject = NULL; \
}\


#define OBJECT_CAST(type,pCastedObj)  type pThis = (type)(pCastedObj)                                                                        //[Public]←创建指定类型的pThis
#define OBJECT_MEMORY_MALLOC(type,memlength) ((memlength <= 0)? NULL:((type*)MALLOC(memlength)))                                             //[Public]←申请指定大小的内存
#define OBJECT_MEMORY_REALLOC(type,pObject,memlength) (((pObject == NULL)||(memlength <= 0)) ? pObject:((type*)REALLOC(pObject,memlength)))  //[Public]←追加申请指定大小的内存
#define OBJECT_MALLOC(type,pObject)   ( pObject == NULL ? ((type*)MALLOC(sizeof(type))) : pObject)                                           //[Public]←对象申请内存宏定义
#define OBJECT_FREE(pObject)   FREE(pObject)
#define OBJECT_MALLOC_DECLARE()   BOOL bNeedMalloc = TRUE                                                                                    //[Public]←对象申请内存前声明宏定义
#define OBJECT_MALLOC_IS_NEED()    (bNeedMalloc == TRUE)
#define OBJECT_MALLOC_FREE(pObject)   {\
	if(OBJECT_MALLOC_IS_NEED()) {\
	    FREE((pObject));\
		pObject=NULL;\
	}\
}\

/*
Object_Create
-------------------
描述：根对象创建函数
NOTE：
REF：
------------------
Author：whl
Date：2012年12月21日
*/

void Object_Create(tPObject* ppObject);


#endif
