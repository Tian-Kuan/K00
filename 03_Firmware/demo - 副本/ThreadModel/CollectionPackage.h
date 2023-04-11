//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                    CollectionPackage.h
//描述： 威锐达集合类型头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月10日                  模块创建                whl
//
#ifndef __HH_COLLECTIONPACKAGE_HH__
#define __HH_COLLECTIONPACKAGE_HH__

#include <stdlib.h>
#include "ModuleCommon.h"
//#include "SystemConfig.h"


#if TARGETOS==X86
   #ifndef COLLECTIONPACKAGE_WINAPI 
		#ifdef  CollectionPackage_DLL_
			#define COLLECTIONPACKAGE_WINAPI  _declspec(dllexport)	
		#else
			#define COLLECTIONPACKAGE_WINAPI  _declspec(dllimport)
		#endif
	#endif
#else
	#ifndef COLLECTIONPACKAGE_WINAPI
		#define COLLECTIONPACKAGE_WINAPI
	#endif
#endif


void CollectionPackage_Init(void);


#endif
