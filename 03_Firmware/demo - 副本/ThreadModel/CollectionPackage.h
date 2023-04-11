//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                    CollectionPackage.h
//������ ����Ｏ������ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��10��                  ģ�鴴��                whl
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
