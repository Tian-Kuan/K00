//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                   OsAdapterBuilder.c
//������ �����OsAdapterBuilderͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��14��                  ģ�鴴��                zsq
//




#include "OsAdapterBuilder.h"

/***************************************---��OsAdptBuilder���������� BEGIN---********************************/
static void OsAdptBuilder_Free(tPObject pObject);
static tPOSAdapter _OsAdptBuilder_CreatDumy(tPAdapterParam pAdapterParam);
/***************************************---��OsAdptBuilder���������� END---********************************/


/***************************************---��OsAdptBuilder������ʵ�� BEGIN---********************************/

/*
OsAdptBldObject_Create
-------------------
������OsAdptBuilder����Create����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��14��
*/	
INT32S OsAdptBldObject_Create(tPOsAdptBuilder* ppOsAdptBuilder)
{
//	OBJECT_MALLOC_DECLARE();
	tPObject pBaseObj = NULL;
	if (ppOsAdptBuilder == NULL)
	{
		return -1;
	}
	/*S1:Ϊ���������ڴ�ռ�*/
	(*ppOsAdptBuilder) =  OBJECT_MALLOC(tOsAdptBuilder,(*ppOsAdptBuilder));
	if(*ppOsAdptBuilder == NULL)
	{
		//���ش�����
		return -1;
	}
	pBaseObj = (tPObject)*ppOsAdptBuilder;
	/*S2:��ʼ������*/
	Object_Create((tPObject*)ppOsAdptBuilder);
	/*S3:��ʼ��������*/
	(*ppOsAdptBuilder)->fnCreatOSA = _OsAdptBuilder_CreatDumy;


	/*S4:ע�������free����*/
	//if(pBaseObj->fnOverideFree != NULL)
	//{
	pBaseObj->fnOverideFree(pBaseObj,OsAdptBuilder_Free);
	//}
	pBaseObj->Size =sizeof(tOsAdptBuilder);
	return 0;
}

/*
_OsAdptBuilder_CreatDumy
-------------------
������_OsAdptBuilder������Ԫ����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��14��
*/	
static tPOSAdapter _OsAdptBuilder_CreatDumy(tPAdapterParam pAdapterParam)
{
	//do nothing
	return NULL;
}


/*
OsAdptBuilder_Free
-------------------
������OsAdptBuilder_Free����Free����
NOTE��
REF��
------------------
Author��zsq
Date��2012��12��14��
*/	
static void OsAdptBuilder_Free(tPObject pObject)
{
	//do nothing
}

/***************************************---��OsAdptBuilder������ʵ�� END---********************************/
