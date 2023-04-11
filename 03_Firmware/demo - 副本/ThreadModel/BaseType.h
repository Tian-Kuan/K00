//////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ���                    BaseType.h
//������ ����������������ͷ�ļ�
//��ǰ�汾��v1.0.0.0
//������
//-------------------------
// 2012��12��10��                  ģ�鴴��                whl
//
#ifndef __HH_BASE_TYPE_HH__
#define __HH_BASE_TYPE_HH__

#include "stdlib.h"

#define _IN______   /*�������*/
#define _OUT____  /*�������*/
#define _INOUT_

#define _MODULE_OP_   /*ģ�鼶����*/
#define _CLASS_OP_    /*�෽��*/
#define _OBJECT_OP_   /*���󷽷�*/

#define X86 1
#define OMAPL138_ARM 2
#define OMAPL138_DSP 3

#define __TI__
#define _OMAP_DSP_
#define __TARGET_PLAT_DSP__
#define TARGETOS OMAPL138_DSP


typedef unsigned char			BOOLEAN;                /* ����                                 */
typedef unsigned char			INT8U;                  /* �޷���8λ����                        */
typedef INT8U*					PINT8U;					/* �޷���8λ����ָ��                    */
typedef signed   char			INT8S;                  /* �з���8λ����                        */
typedef INT8S*					PINT8S;					/* �з���8λ����ָ��                    */
typedef unsigned short			INT16U;                 /* �޷���16λ����                       */
typedef INT16U*					PINT16U;                /* �޷���16λ����ָ��                   */
typedef signed   short			INT16S;                 /* �з���16λ����                       */
typedef INT16S*     			PINT16S;                /* �з���16λ����ָ��                   */
typedef unsigned int 			INT32U;                 /* �޷���32λ����                       */
typedef unsigned int*			PINT32U;                /* �޷���32λ����ָ��                   */
typedef signed   int			INT32S;                 /* �з���32λ����                       */
typedef signed   int*			PINT32S;                 /* �з���32λ����ָ��                       */
typedef unsigned long long		INT64U;					/* �޷���32λ����                       */
typedef signed   long long		INT64S;					/* �з���32λ����                       */
typedef float					FP32;                   /* �����ȸ�������32λ���ȣ�             */
typedef double					FP64;                   /* ˫���ȸ�������64λ���ȣ�             */
typedef void*					PVOID;                  /* ��ָ�� */



#define INT32 INT32S      /*�з���32λ�����͵ı���*/
#define INT16 INT16S      /*�з���16λ�����͵ı���*/
#define INT8  INT8S       /*�з���8λ�����͵ı���*/

#ifndef BYTE
#define BYTE           INT8U		/*���ֽ�����*/
#endif
typedef BYTE*          PBYTE;		/*���ֽ�ָ������*/

#ifndef WORD
#define WORD           INT16S		/*�з��ŵ�������*/
#endif

#ifndef UWORD
#define UWORD           INT16U		/*�з��ŵ�������*/
#endif
#define UWORD          INT16U		/*�޷��ŵ�������*/

#ifndef LONG
#define LONG           INT32S		/*�з��ŵ�������*/
#endif

#ifndef ULONG
#define ULONG           INT32U		/*�޷��ŵ�������*/
#endif

#ifndef FLOAT
#define FLOAT           FP32		/*32λ�������͵ı���*/
#endif


typedef FLOAT*	       PFLOAT;      /*32λ����ָ������*/

#ifndef DOUBLE
#define DOUBLE           FP64		/*32λ�������͵ı���*/
#endif

typedef DOUBLE*	       PDOUBLE;     /*64λ����ָ������*/

#ifndef CHAR
#define CHAR           char         /*���ַ�����*/
#endif

typedef CHAR*		   PCHAR;        /*���ַ�ָ������*/

#ifndef UCHAR
#define UCHAR          unsigned  char         /*���޷����ַ�����*/
#endif
typedef UCHAR*		   PUCHAR;        /*���޷����ַ�ָ������*/

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
