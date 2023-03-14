//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                   OsAdapterBuilder.c
//描述： 威瑞达OsAdapterBuilder头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月14日                  模块创建                zsq
//




#include "OsAdapterBuilder.h"

/***************************************---↑OsAdptBuilder对象函数声明 BEGIN---********************************/
static void OsAdptBuilder_Free(tPObject pObject);
static tPOSAdapter _OsAdptBuilder_CreatDumy(tPAdapterParam pAdapterParam);
/***************************************---↑OsAdptBuilder对象函数声明 END---********************************/


/***************************************---↑OsAdptBuilder对象函数实现 BEGIN---********************************/

/*
OsAdptBldObject_Create
-------------------
描述：OsAdptBuilder对象Create函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月14日
*/	
INT32S OsAdptBldObject_Create(tPOsAdptBuilder* ppOsAdptBuilder)
{
//	OBJECT_MALLOC_DECLARE();
	tPObject pBaseObj = NULL;
	if (ppOsAdptBuilder == NULL)
	{
		return -1;
	}
	/*S1:为自身点分配内存空间*/
	(*ppOsAdptBuilder) =  OBJECT_MALLOC(tOsAdptBuilder,(*ppOsAdptBuilder));
	if(*ppOsAdptBuilder == NULL)
	{
		//返回错误码
		return -1;
	}
	pBaseObj = (tPObject)*ppOsAdptBuilder;
	/*S2:初始化父类*/
	Object_Create((tPObject*)ppOsAdptBuilder);
	/*S3:初始化自身方法*/
	(*ppOsAdptBuilder)->fnCreatOSA = _OsAdptBuilder_CreatDumy;


	/*S4:注册根对象free方法*/
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
描述：_OsAdptBuilder创建哑元函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月14日
*/	
static tPOSAdapter _OsAdptBuilder_CreatDumy(tPAdapterParam pAdapterParam)
{
	//do nothing
	return NULL;
}


/*
OsAdptBuilder_Free
-------------------
描述：OsAdptBuilder_Free对象Free函数
NOTE：
REF：
------------------
Author：zsq
Date：2012年12月14日
*/	
static void OsAdptBuilder_Free(tPObject pObject)
{
	//do nothing
}

/***************************************---↑OsAdptBuilder对象函数实现 END---********************************/
