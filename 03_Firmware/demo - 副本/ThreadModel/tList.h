//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                          tList.h
//������ ������б�����ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��10��                  ģ�鴴��                whl
//

#ifndef __HH_TLIST_H_HH__
#define __HH_TLIST_H_HH__


#include "CollectionPackage.h"
#include "tTraversableObject.h"
#include "tArray.h"
#include "BaseType.h"


typedef struct _tagStructListObj  tListObj,*tPListObj;
typedef struct _tagStructListElementObj tListElementObj,*tPListElementObj;


typedef BOOL (*tfnListElementMatch)(tPObject);    //��Ԫ��ƥ�亯��


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
	enPrev = 1,                /*ǰ������*/
	enNext = 2                 /*�������� as default*/
}EnumListGrownDirc;






/*	
tListElementObj
-------------------
�������б�Ԫ�����Ͷ���
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
struct _tagStructListElementObj
{
	tPObject                       pValue;           //������������ 
	//struct _tagStructListElementObj *prev;         //ǰ��ڵ�
	struct _tagStructListElementObj *next;           //����ڵ�
};




/*	
tListObj
-------------------
�������б����Ͷ���
NOTE��
REF��
------------------
Author��whl
Date��2012��12��10��
*/
struct _tagStructListObj
{
	tTraversableObj                     baseObj;             //[Private] ����Ļ����������� 
	BOOL                                bNeedFreeElement;    //[Private] �Ƿ�ɾ��Ԫ�������ڴ�
	tPListElementObj                    pHeadObj;            //[Private] ָ���б�ڵ��ͷָ��
    tPListElementObj                    pCurrentObj;         //[Private] ָ���б�ڵ�ĵ�ǰָ��
	tPListElementObj                    pEndObj;             //[Private] ָ���б�ڵ��βָ��
	EnumListGrownDirc                   enumListGrownDirc;   //[Private] ��ʾ��ǰ�б������
	tfnCollectionListGetCount           fnGetCount;          //[Public]  ��ȡ��ǰ�б�Ľڵ��������ָ�� 
	tfnCollectionListHasNext            fnHasNext;           //[Public]  ��ѯ�б��Ƿ������һ���ڵ㺯��ָ��
	tfnCollectionListGetNext            fnGetNext;           //[Public]  ��ȡ�б���һ���ڵ㺯��ָ��
	tfnCollectionListAdd                fnAdd;               //[Public]  ����б�ڵ㺯��ָ��
	tfnCollectionListRemove             fnRemove;            //[Public]  ɾ���б�ڵ㺯��ָ��
	tfnCollectionListClear              fnClear;	         //[Public]  ����б���ָ��
//	tfnCollectionListFind               fnFind;              //[Public]  ����ָ��ƥ�������ڵ�ĺ���ָ��
	tfnCollectionListGetHead            fnGetHead;           //[Public]  ��ȡ�б�ͷ��㺯��ָ��
	tfnToArray                          fnToArray;           //[Public]  ���б��нڵ�ת��Ϊ�ֽ�����
};



/*	
List_Create
-------------------
�������б�������
NOTE������ӿں���
REF��
------------------
Author��whl
Date��2012��12��17��
*/
void List_Create(tPListObj *,EnumListGrownDirc listGrownDirc);


#endif

