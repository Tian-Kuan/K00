//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                          tList.h
//描述： 威锐达列表类型头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月10日                  模块创建                whl
//

#ifndef __HH_TLIST_H_HH__
#define __HH_TLIST_H_HH__


#include "CollectionPackage.h"
#include "tTraversableObject.h"
#include "tArray.h"
#include "BaseType.h"


typedef struct _tagStructListObj  tListObj,*tPListObj;
typedef struct _tagStructListElementObj tListElementObj,*tPListElementObj;


typedef BOOL (*tfnListElementMatch)(tPObject);    //←元素匹配函数


typedef tPObject (*tfnCollectionListGetHead)(tPListObj);
typedef INT32S (*tfnCollectionListGetCount)(tPListObj);
typedef BOOL   (*tfnCollectionListHasNext)(tPListObj);
typedef tPObject (*tfnCollectionListGetNext)(tPListObj);
typedef void (*tfnCollectionListAdd) (tPListObj,tPObject);
typedef void (*tfnCollectionListRemove)(tPListObj,tPObject);
typedef void (*tfnCollectionListClear)(tPListObj,BOOL);
//typedef tPListObj (*tfnCollectionListFind)(tPListObj,tfnListElementMatch);
typedef tPArrayObj (*tfnToArray)(tPListObj);



typedef enum _tagEnumListGrownDirc
{
	enPrev = 1,                /*前向增长*/
	enNext = 2                 /*后向增长 as default*/
}EnumListGrownDirc;






/*	
tListElementObj
-------------------
描述：列表元素类型对象
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
*/
struct _tagStructListElementObj
{
	tPObject                       pValue;           //基本数据类型 
	//struct _tagStructListElementObj *prev;         //前向节点
	struct _tagStructListElementObj *next;           //后向节点
};




/*	
tListObj
-------------------
描述：列表类型对象
NOTE：
REF：
------------------
Author：whl
Date：2012年12月10日
*/
struct _tagStructListObj
{
	tTraversableObj                     baseObj;             //[Private] 父类的基本数据类型 
	BOOL                                bNeedFreeElement;    //[Private] 是否删除元素所在内存
	tPListElementObj                    pHeadObj;            //[Private] 指向列表节点的头指针
    tPListElementObj                    pCurrentObj;         //[Private] 指向列表节点的当前指针
	tPListElementObj                    pEndObj;             //[Private] 指向列表节点的尾指针
	EnumListGrownDirc                   enumListGrownDirc;   //[Private] 表示当前列表的类型
	tfnCollectionListGetCount           fnGetCount;          //[Public]  获取当前列表的节点个数函数指针 
	tfnCollectionListHasNext            fnHasNext;           //[Public]  查询列表是否存在下一个节点函数指针
	tfnCollectionListGetNext            fnGetNext;           //[Public]  获取列表下一个节点函数指针
	tfnCollectionListAdd                fnAdd;               //[Public]  添加列表节点函数指针
	tfnCollectionListRemove             fnRemove;            //[Public]  删除列表节点函数指针
	tfnCollectionListClear              fnClear;	         //[Public]  清除列表函数指针
//	tfnCollectionListFind               fnFind;              //[Public]  查找指定匹配条件节点的函数指针
	tfnCollectionListGetHead            fnGetHead;           //[Public]  获取列表头结点函数指针
	tfnToArray                          fnToArray;           //[Public]  将列表中节点转化为字节数组
};



/*	
List_Create
-------------------
描述：列表创建函数
NOTE：对外接口函数
REF：
------------------
Author：whl
Date：2012年12月17日
*/
void List_Create(tPListObj *,EnumListGrownDirc listGrownDirc);


#endif

