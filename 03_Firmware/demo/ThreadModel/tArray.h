//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                          tArray.h
//描述： 威锐达数组类型头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月10日                  模块创建                whl / hepeng
//

#ifndef __HH_TARRY_H_HH__
#define __HH_TARRY_H_HH__

#include "tTraversableObject.h"
#include "CollectionPackage.h"


#define  ARRAY_OBJECT_MAX_LENGTH               (64)    // 1024->64


typedef struct tagStructArrayObj  tArrayObj,*tPArrayObj;                               // 变量声明

//typedef struct _tagStructtfnOnCollectionFull tfnOnCollectionFull,*tPfnOnCollectionFull;

typedef INT32S   (*tfnCollectionArrayGetCount)(tPArrayObj);                            // 获取Array长度
typedef tPObject* (*tfnCollectionArrayGetBuf)(tPArrayObj);                              // 读取Array中的数据
//typedef void     (*tfnCollectionRemoveAll)(tPArrayObj);                                // 清空Array
//typedef void     (*tfnCollectionArrayRegisterCallBack)(tPArrayObj,tfnOnCollectionFull);// 注册相应的回调函数
typedef void     (*tfnCollectionArrayAdd)(tPArrayObj,tPObject);                        // 添加元素  
typedef tPObject (*tfnCollectionArrayGetAt)(tPArrayObj,INT16U);
 

typedef void (*tfnArrayNotify)(tPObject pListenner,tPObject pCollectionObj);

/*	
_tagStructtfnOnCollectionFull
-------------------
描述：数组回调对象
NOTE：
REF：
------------------
Author：whl 
Date：2013年1月11日
*/
//struct _tagStructtfnOnCollectionFull
//{
//	tPObject  pListenner;                              //←监听者对象
//	tfnArrayNotify fnNotify;                           //←事件通知函数
//};




/*	
tagStructArrayObj
-------------------
描述：数组类型对象
NOTE：
REF：
------------------
Author：whl / hepeng
Date：2012年12月17日
*/
struct tagStructArrayObj
{
	tTraversableObj                              baseObj;                            // 基本数据类型 
//	tfnOnCollectionFull                          fnOnCollectionFull;                 // 集合满回调函数所需类型
	INT32U                                       u16CurrentDataLength;               // 用于表明当前数组元素的个数
	tPObject                                     pArrayData[ARRAY_OBJECT_MAX_LENGTH];// 用于核心存放数据的Data区(数据存放的是tPobject型数据)
	tfnCollectionArrayGetCount                   fnGetCount;                         // Array的大小
	tfnCollectionArrayGetBuf                     fnGetBuf;                           // 获取Array中存取的数据
//	tfnCollectionRemoveAll                       fnRemoveAll;                        // 删除Array中的所有元素
//	tfnCollectionArrayRegisterCallBack           fnCollectionArrayRegisterCallBack;  // 注册数组所占内存已满的情况下的回调函数
	tfnCollectionArrayAdd                        fnAdd;                              // 添加元素
	tfnCollectionArrayGetAt                      fnGetAtIndex;                       // 获取指定位置上的元素 
};


/*	
Array_Create
-------------------
描述：
NOTE：
REF：
------------------
Author：whl / hepeng
Date：2012年12月18日
 */
void Array_Create(tPArrayObj * ppArrayObj);

#endif    
