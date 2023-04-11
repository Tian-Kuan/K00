//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                       tObject.c
//������ ����������������ʵ���ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��10��                  ģ�鴴��                whl
//

#include "tObject.h"
#include "led.h"

//static BOOL _Equal (tPObject pRootObj,tPObject pDstObject);                          //�жϸ������ϣ���Ƿ���ͬ
//static INT32S _GetHashCode(tPObject pRootObj);  
static void _Free (tPObject pRootObj);
//static INT32U _GetSize(tPObject pRootObj);
static BOOL _OverideFree (tPObject pRootObj,tfnFree fnfnFree);                       //������Free�������ط���

/*-------------------------------�ڲ�����������--------------------------BEGIN------------*/
static BOOL _InnerMethod_RequireFree(tPObject pObject);
static BOOL _InnerMethod_CanFree(tPObject pObject);
static void _InnerMethod_DoDelayFree(tPObject pObject);
/*-------------------------------�ڲ�����������---------------------------END------------*/

/***************************************---��Object��������ʵ���� BEGIN  ---******************************/
 /*
 Object_Create
 -------------------
 �������������󴴽�����
 NOTE��
 REF��
 ------------------
 Author��whl
 Date��2012��12��10��
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
 ����������ȽϺ���
 NOTE����ͬ����True������Else
 REF��
 ------------------
 Author��whl
 Date��2012��12��10��
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
 ��������ȡ�������ڵ��ڴ���
 NOTE��
 REF��
 ------------------
 Author��whl
 Date��2012��12��10��
 */
// INT32S _GetHashCode(
//	 _IN______ tPObject pObject
//	 )
// {
//	 if (pObject == NULL)
//	 {
//		 return -1;
//	 }
//	 return (INT32S)pObject; //����ʱ���ظö������ڴ��еĵ�ַ��Ӧ�÷��ظö����Ӧ�Ĺ�ϣ��
//	 //return pObject->HashCode;

// }

 /*
 GetHashCode
 -------------------
 ��������ȡ�������ڵ��ڴ���
 NOTE��
 REF��
 ------------------
 Author��whl
 Date��2012��12��10��
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
 �������ͷŶ������ڵ��ڴ�
 NOTE��
 REF��
 ------------------
 Author��whl
 Date��2012��12��10��
 */
 void _Free(
	 _IN______ tPObject pObject
	 )
 {
	 tfnFree _InheritObjFree = NULL;
	 /*S1:������֤*/
//	 if (pObject == NULL)
//	 {
//		 return;
//	 }
	 /*S2:�ж��Ƿ��������*/
	 if (pObject->fnRequireFree != NULL && _InnerMethod_CanFree(pObject)==TRUE)
	 {
		 /*S3:������ü̳����ϵ���������*/
		 if (/*pObject->_InnerFreesFIFO != NULL &&*/ pObject->u8CurntInheritObjCount > 0)
		 {
				 while(pObject->u8CurntInheritObjCount > 0)
				 {
					 _InheritObjFree = pObject->_InnerFreesFIFO[pObject->u8CurntInheritObjCount-1];
					 _InheritObjFree(pObject);
					 pObject->u8CurntInheritObjCount--;
				 }
		 }
		 /*S4:�ͷ�����*/
		 OBJECT_FREE(pObject);
	 }	 
 }

/*
 _OverideFree
 -------------------
 ���������ض���Free����
 NOTE�����سɹ�����TRUE�����򷵻�FALSE
 REF��
 ------------------
 Author��whl
 Date��2013��1��7��
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
	if (pObject->u8CurntInheritObjCount > TYPE_INHERIT_TREE_MAX_LEVELS) //�������̳����涨��������
	{
		return FALSE;
	}
	pObject->_InnerFreesFIFO[pObject->u8CurntInheritObjCount] = fnFree;
	pObject->u8CurntInheritObjCount++;
	return TRUE;
}



/*-------------------------------�ڲ�˽�з���---------------------------BEGIN------------*/

/*
  _InnerMethod_DoDelayFree
  -----------------------
  ������ ִ���Ƴ����ٷ������÷������ж��Ƿ��Ƴ����٣�������ʵ��ִ��free
  ���ߣ� Martin.lee
  ���ڣ� 2013-1-14
*/
void _InnerMethod_DoDelayFree(tPObject pObject)
{
	/*S1:������֤*/
	if (pObject == NULL ||  pObject->bNeedDelayFree == FALSE)
	{
		return;
	}

	/*S2:ִ��ʵ�ʵ�����*/	
	if (pObject->fnoDelayFree.pFreeObj != NULL)
	{
		/*S2-1:�������û������free�����֮*/
		pObject->fnoDelayFree.fnFree(pObject->fnoDelayFree.pFreeObj,pObject->fnoDelayFree.u32Param);
	}
	else
	{
		/*S2-2:�������û������free��ֱ�ӵ��ö�������ٷ���*/
		pObject->bNeedDelayFree = FALSE;
		pObject->fnFree(pObject);
	}	
}

/*
  _InnerMethod_RequireFree
  -----------------------
  ������ �����Ƿ�������١�����True���ʾ��������
  ���ߣ� Martin.lee
  ���ڣ� 2013-1-14
*/
BOOL _InnerMethod_RequireFree(tPObject pObject)
{
	/*S1:������֤*/
	if (pObject == NULL)
	{
		return FALSE;
	}

	/*S2:ֱ�ӷ���[�Ƿ���Ҫ�Ƴ�Free]��ʶ����(ȡ����*/
	return pObject->bNeedDelayFree==TRUE?FALSE:TRUE;
}

/*
  _InnerMethod_CanFree
  -----------------------
  ������ ��������̳���ϵFree�����ж��Ƿ����Free
  ���ߣ� Martin.lee
  ���ڣ� 2013-1-14
*/
BOOL _InnerMethod_CanFree(tPObject pObject)
{
	/*S1:������֤*/
	if (pObject != NULL && pObject->fnRequireFree!= NULL)
	{
		/*S2:��������free�����ѻ�ÿɷ�������Ϣ*/
		return pObject->fnRequireFree(pObject);
	}
	return FALSE;
}
/*-------------------------------�ڲ�˽�з���---------------------------END------------*/

/***************************************---��Object�������� END---********************************/
