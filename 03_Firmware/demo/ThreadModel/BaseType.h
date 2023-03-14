//////////////////////////////////////////////////////////////////////////////////////////////////
//文件名                    BaseType.h
//描述： 威锐达基本数据类型头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2012年12月10日                  模块创建                whl
//
#ifndef __HH_BASE_TYPE_HH__
#define __HH_BASE_TYPE_HH__

#include "stdlib.h"

#define _IN______   /*输入参数*/
#define _OUT____  /*输出参数*/
#define _INOUT_

#define _MODULE_OP_   /*模块级方法*/
#define _CLASS_OP_    /*类方法*/
#define _OBJECT_OP_   /*对象方法*/

#define X86 1
#define OMAPL138_ARM 2
#define OMAPL138_DSP 3

#define __TI__
#define _OMAP_DSP_
#define __TARGET_PLAT_DSP__
#define TARGETOS OMAPL138_DSP


typedef unsigned char			BOOLEAN;                /* 布尔                                 */
typedef unsigned char			INT8U;                  /* 无符号8位整型                        */
typedef INT8U*					PINT8U;					/* 无符号8位整型指针                    */
typedef signed   char			INT8S;                  /* 有符号8位整型                        */
typedef INT8S*					PINT8S;					/* 有符号8位整型指针                    */
typedef unsigned short			INT16U;                 /* 无符号16位整型                       */
typedef INT16U*					PINT16U;                /* 无符号16位整型指针                   */
typedef signed   short			INT16S;                 /* 有符号16位整型                       */
typedef INT16S*     			PINT16S;                /* 有符号16位整型指针                   */
typedef unsigned int 			INT32U;                 /* 无符号32位整型                       */
typedef unsigned int*			PINT32U;                /* 无符号32位整型指针                   */
typedef signed   int			INT32S;                 /* 有符号32位整型                       */
typedef signed   int*			PINT32S;                 /* 有符号32位整型指针                       */
typedef unsigned long long		INT64U;					/* 无符号32位整型                       */
typedef signed   long long		INT64S;					/* 有符号32位整型                       */
typedef float					FP32;                   /* 单精度浮点数（32位长度）             */
typedef double					FP64;                   /* 双精度浮点数（64位长度）             */
typedef void*					PVOID;                  /* 空指针 */



#define INT32 INT32S      /*有符号32位整数型的别名*/
#define INT16 INT16S      /*有符号16位整数型的别名*/
#define INT8  INT8S       /*有符号8位整数型的别名*/

#ifndef BYTE
#define BYTE           INT8U		/*单字节类型*/
#endif
typedef BYTE*          PBYTE;		/*单字节指针类型*/

#ifndef WORD
#define WORD           INT16S		/*有符号单字类型*/
#endif

#ifndef UWORD
#define UWORD           INT16U		/*有符号单字类型*/
#endif
#define UWORD          INT16U		/*无符号单字类型*/

#ifndef LONG
#define LONG           INT32S		/*有符号单字类型*/
#endif

#ifndef ULONG
#define ULONG           INT32U		/*无符号单字类型*/
#endif

#ifndef FLOAT
#define FLOAT           FP32		/*32位浮点类型的别名*/
#endif


typedef FLOAT*	       PFLOAT;      /*32位浮点指针类型*/

#ifndef DOUBLE
#define DOUBLE           FP64		/*32位浮点类型的别名*/
#endif

typedef DOUBLE*	       PDOUBLE;     /*64位浮点指针类型*/

#ifndef CHAR
#define CHAR           char         /*单字符类型*/
#endif

typedef CHAR*		   PCHAR;        /*单字符指针类型*/

#ifndef UCHAR
#define UCHAR          unsigned  char         /*单无符号字符类型*/
#endif
typedef UCHAR*		   PUCHAR;        /*单无符号字符指针类型*/

#ifndef BOOL
#define BOOL           INT8U
#endif


#ifndef TRUE
#define TRUE           1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#endif
