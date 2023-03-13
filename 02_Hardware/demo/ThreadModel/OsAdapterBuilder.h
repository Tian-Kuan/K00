//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                   OsAdapterBuilder.h
//������ �����OsAdapterBuilderͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��14��                  ģ�鴴��                zsq
//

#ifndef __HH_OSADAPERBUILDER_HH__
#define __HH_OSADAPERBUILDER_HH__





#include "ThreadCommon.h"
#include "OSAdapter.h"

#if TARGETOS==X86
	#ifndef OSADAPTERBUILDER_WINAPI 
		#ifdef  _OSAdapterBuilder_DLL_
			#define OSADAPTERBUILDER_WINAPI  _declspec(dllexport) 
		#else
			#define OSADAPTERBUILDER_WINAPI  _declspec(dllimport)
		#endif
	#endif
#else
	#ifndef OSADAPTERBUILDER_WINAPI
		#define OSADAPTERBUILDER_WINAPI
	#endif
#endif


typedef struct _tagStructOsAdptBuilder tOsAdptBuilder,*tPOsAdptBuilder;

typedef tPOSAdapter (*tfnCreatOSA)(tPAdapterParam pAdapterParam);


struct _tagStructOsAdptBuilder
{
	tObject baseObj;
	tfnCreatOSA fnCreatOSA;
};

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
OSADAPTERBUILDER_WINAPI INT32S OsAdptBldObject_Create(tPOsAdptBuilder* ppOsAdptBuilder);

#endif

