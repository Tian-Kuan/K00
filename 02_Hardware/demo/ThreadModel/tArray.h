//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                          tArray.h
//������ �������������ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��10��                  ģ�鴴��                whl / hepeng
//

#ifndef __HH_TARRY_H_HH__
#define __HH_TARRY_H_HH__

#include "tTraversableObject.h"
#include "CollectionPackage.h"


#define  ARRAY_OBJECT_MAX_LENGTH               (64)    // 1024->64


typedef struct tagStructArrayObj  tArrayObj,*tPArrayObj;                               // ��������

//typedef struct _tagStructtfnOnCollectionFull tfnOnCollectionFull,*tPfnOnCollectionFull;

typedef INT32S   (*tfnCollectionArrayGetCount)(tPArrayObj);                            // ��ȡArray����
typedef tPObject* (*tfnCollectionArrayGetBuf)(tPArrayObj);                              // ��ȡArray�е�����
//typedef void     (*tfnCollectionRemoveAll)(tPArrayObj);                                // ���Array
//typedef void     (*tfnCollectionArrayRegisterCallBack)(tPArrayObj,tfnOnCollectionFull);// ע����Ӧ�Ļص�����
typedef void     (*tfnCollectionArrayAdd)(tPArrayObj,tPObject);                        // ���Ԫ��  
typedef tPObject (*tfnCollectionArrayGetAt)(tPArrayObj,INT16U);
 

typedef void (*tfnArrayNotify)(tPObject pListenner,tPObject pCollectionObj);

/*	
_tagStructtfnOnCollectionFull
-------------------
����������ص�����
NOTE��
REF��
------------------
Author��whl 
Date��2013��1��11��
*/
//struct _tagStructtfnOnCollectionFull
//{
//	tPObject  pListenner;                              //�������߶���
//	tfnArrayNotify fnNotify;                           //���¼�֪ͨ����
//};




/*	
tagStructArrayObj
-------------------
�������������Ͷ���
NOTE��
REF��
------------------
Author��whl / hepeng
Date��2012��12��17��
*/
struct tagStructArrayObj
{
	tTraversableObj                              baseObj;                            // ������������ 
//	tfnOnCollectionFull                          fnOnCollectionFull;                 // �������ص�������������
	INT32U                                       u16CurrentDataLength;               // ���ڱ�����ǰ����Ԫ�صĸ���
	tPObject                                     pArrayData[ARRAY_OBJECT_MAX_LENGTH];// ���ں��Ĵ�����ݵ�Data��(���ݴ�ŵ���tPobject������)
	tfnCollectionArrayGetCount                   fnGetCount;                         // Array�Ĵ�С
	tfnCollectionArrayGetBuf                     fnGetBuf;                           // ��ȡArray�д�ȡ������
//	tfnCollectionRemoveAll                       fnRemoveAll;                        // ɾ��Array�е�����Ԫ��
//	tfnCollectionArrayRegisterCallBack           fnCollectionArrayRegisterCallBack;  // ע��������ռ�ڴ�����������µĻص�����
	tfnCollectionArrayAdd                        fnAdd;                              // ���Ԫ��
	tfnCollectionArrayGetAt                      fnGetAtIndex;                       // ��ȡָ��λ���ϵ�Ԫ�� 
};


/*	
Array_Create
-------------------
������
NOTE��
REF��
------------------
Author��whl / hepeng
Date��2012��12��18��
 */
void Array_Create(tPArrayObj * ppArrayObj);

#endif    
