//////////////////////////////////////////////////////////////////////////
//文件名                 DBG_Utility_Malloc.h
//描述：调试工具模块之堆内存申请模块头文件
//当前版本：v1.0.0.0
//履历：
//-------------------------
// 2011年7月26日	模块创建               Martin
// 2013年1月18日    代码移植               李立川



#ifndef __HH_DBG_UTILITY_MALLOC_H_HH__
#define __HH_DBG_UTILITY_MALLOC_H_HH__


#include "BaseType.h"
#include "stdlib.h"

#if (0) //TARGETOS==X86
#ifndef DBG_UTILITY_WINAPI 
#ifdef  DBG_UTILITY_DLL
#define DBG_UTILITY_WINAPI  _declspec(dllexport) 
#else
#define DBG_UTILITY_WINAPI  _declspec(dllimport)
#endif
#endif
#else
#ifndef DBG_UTILITY_WINAPI
#define DBG_UTILITY_WINAPI
#endif
#endif


#define MALLOC(size)   malloc(size)						//malloc(size)     malloc_x(size,__FILE__,__LINE__)

#define FREE(pData)    free(pData)						//free(pData)   free_x(pData,__FILE__,__LINE__)



//////////////////////////////////////////////////////////////////////////
//Macros declaration begin from here..
/*
MAX_MALOCC_ITEM_CNT
------------------
描述：最大可申请的项目数
Mark：系统中若已存在该宏定义条项目后再次申请将失败
NOTE：none
REF： 
------------------
Author：Martin
Date：  2011年7月26日
*/
#define MAX_MALOCC_ITEM_CNT    10
#define MAX_REPORT_MSG_LEN     1

//Macros declaration ended here.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Types declaration begin from here..

/*
tIReportTargetOp
------------------
描述：报告目标机操作
Mark：指定操作方法后即可打印信息到指定设备
NOTE：none
REF： 
------------------
Author：Martin
Date：  2011年7月26日
*/
typedef void (*tfnPrint) (CHAR* strMsg);
typedef struct  
{
	tfnPrint fnPrint;
}tIReportTargetOp;


//Types declaration ended here.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Operations declaration begin from here..

/*
DBG_UTILITY_M_Init
------------------
描述：调试模块之内存管理模块的初始化
Mark：未经初始化本模块处于未定义的状态不可使用！
REF： 
------------------
Author：Martin
Date：  2011年7月26日
*/
DBG_UTILITY_WINAPI void DBG_UTILITY_M_Init(
	PINT16U pu16ErrorCode           /*[OUT] 返回的操作执行结果错误码*/
);

/*
DBG_UTILITY_Malloc
------------------
描述：申请获得指定大小的存储区域
Mark：申请内存后即在调试系统内部注册该申请的信息。
      信息为一个堆申请描述三元组{所获得的内存的地址,内存区域长度,
	  申请函数调用所在文件名,申请函数所在文件的行号}。
NOTE：通过C_Runtime Lib的堆管理模块实质性获取存储区域
REF： mallloc
------------------
Author：Martin
Date：  2011年7月26日
*/
DBG_UTILITY_WINAPI void* DBG_UTILITY_Malloc(
	INT32U u32DesiredDataSize, /*[IN] 需要申请的内存大小*/
	CHAR* strFileName,         /*[IN] 申请发生所在文件*/
	INT32U u32LineNO           /*[IN] 申请发生所在文件中的行号*/
);
/*
DBG_UTILITY_Realloc
------------------
描述：重新申请内存
Mark：
NOTE：通过C_Runtime Lib的堆管理模块实质性获取存储区域
REF： reallloc
------------------
Author：Martin
Date：  2011年7月26日
*/
DBG_UTILITY_WINAPI void* DBG_UTILITY_Realloc(
	void* pOrigMem,            /*[IN] 需要申请的内存地址*/   
	INT32U u32DesiredDataSize, /*[IN] 需要申请的内存大小*/
	CHAR* strFileName,         /*[IN] 申请发生所在文件*/
	INT32U u32LineNO           /*[IN] 申请发生所在文件中的行号*/
);
/*
DBG_UTILITY_Free
------------------
描述：释放指定存储区域
Mark：根据指定地址查询获得所释放内存的描述信息。通过C-Runtime Lib
      从堆中释放该区域后将该内存描述信息从系统中去除。
REF： free
------------------
Author：Martin
Date：  2011年7月26日
*/
DBG_UTILITY_WINAPI void DBG_UTILITY_Free(
	void* pNeedFreeMemAddress           /*[IN] 需要被释放的内存区地址*/
);


/*
DBG_UTILITY_ReportStatistics
------------------
描述：报告堆使用统计信息
Mark：将统计信息打印到指定设备。通常情况系您需要实现或指定缺省的报告设备接口
REF： 
------------------
Author：Martin
Date：  2011年7月26日
*/
DBG_UTILITY_WINAPI void DBG_UTILITY_ReportStatistics(
	tIReportTargetOp    reportTargetOp       /*[IN] 报告目标机操作接口*/
);


/*
DBG_UTILITY_Clear
------------------
描述：清除DBG_Malloc的所有统计信息。
Mark：所有申请过的内存单元从注册列表中清除。
CAUTION：调用本函数后Free发生于本函数前通过Malloc而得的内存单元将发生崩溃的危险！
REF： None
------------------
Author：Martin
Date：  2011年8月3日
*/
DBG_UTILITY_WINAPI void DBG_UTILITY_Clear(
	void
);

//Operations declaration ended here.
//////////////////////////////////////////////////////////////////////////
//INT32U GetSP(void);
void* malloc_x(
   INT32U u32DesiredDataSize,
   CHAR* strFileName,         /*[IN] 申请发生所在文件*/
	INT32U u32LineNO           /*[IN] 申请发生所在文件中的行号*/
	);
void free_x(
   void* pMem,
   CHAR* strFileName,         /*[IN] 申请发生所在文件*/
	 INT32U u32LineNO           /*[IN] 申请发生所在文件中的行号*/
	);


   
#define REALLOC(pOrigMem,size)  realloc(pOrigMem,size)
#endif

