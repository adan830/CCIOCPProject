/**************************************************************************************
@author: 陈昌
@content: 自己使用的一个常用结构函数库
**************************************************************************************/
#ifndef __CC_UTILS_H__
#define __CC_UTILS_H__

#include <string>
#include <vector>
#include <cstdint>

#define _WINSOCKAPI_
#include <Windows.h>

namespace CC_UTILS{

	extern unsigned int _ExGetTickCount;           

	//简易的Hash结点
	typedef struct _THashPortItem
	{
		int iHandle;
		void* pItem;
		_THashPortItem* Next;
	}THashPortItem, *PHashPortItem, **PPHashPortItem;

	/**
	* 一个简易的hash，数组冲突开链表
	*/
	typedef struct _TSimpleHash
	{
	public:
		void DoInitial(int iSize);
		void AddPortItem(const int iKey, void* pClient);
		void RemovePortItem(const int iKey);
		void ClearAllPortItems();
		PPHashPortItem FindPortItemPointer(const int iKey);     //这里需要返回的是PortItem的指针
		int GetItemCount();
	private:
		PPHashPortItem m_ppItemBuckets;							//用于存储客户端对象的简易hash
		int m_iBucketSize;                                      //固定的数组长度
		int m_iHashItemCount;  				     			    //当前连接中的客户端句柄数量
	}TSimpleHash, *PSimpleHash;

	/**
	* 一个简易的buffer的写入管理对象，主要用在一个长期维护的接受buffer对象
	*/
	typedef struct _TBufferStream
	{
	public:
		void Initialize();
		void Finalize();
		bool Write(const char* pBuf, const int iCount);
		bool Reset(int iUsedLength);
		void* GetMemPoint();
		int GetPosition();
	private:
		void* m_pMemory;
		int m_iMemorySize;
		int m_iMemoryPosition;
	}TBufferStream, *PBufferStream;

	/**
	* 微软多媒体API构造的时钟对象
	*/
	typedef struct _TMMTimer
	{
	public:
		void Initialize(const int time_event_ms);
		void Finalize();
	private:
		unsigned int m_uiHandle;
		unsigned int m_uiAccuracy;
	}TMMTimer;

	//返回文件的最后修改时间转化成的整数，用以检测文件是否修改
	int GetFileAge(const std::string &sFileName);

	//返回文件的版本信息  
	//注意：需要工程中加入version.lib
	std::string GetFileVersion(const std::string &sFileName);

	//字符串分割
	void SplitStr(const std::string& s, const std::string& delim, std::vector<std::string>* ret);

	//将有换行符号的文本字符串,切割存放进vector中
	void SplitStrByLine(std::string& s, std::vector<std::string>* ret);

	//字符串转整数，带默认值，不抛出异常
	int StrToIntDef(const std::string& sTemp, const int iDef);

	//字符串转64位整数，带默认值，不抛出异常
	int64_t StrToInt64Def(const std::string& sTemp, const int64_t iDef);

	//返回当前对应的天数数字
	int GetTodayNum();

	//循环创建目录
	int ForceCreateDirectories(std::string& sDir);

	//从完整路径中取出文件夹路径
	inline std::wstring ExtractFilePathW(const std::wstring& strFullPath);
	inline std::string ExtractFilePathA(const std::string& strFullPath);

	//从完整路径中取出文件名
	std::wstring ExtractFileNameW(const std::wstring& strFullPath);
	std::string ExtractFileNameA(const std::string& strFullPath);

	//取得程序当前的路径 包括反斜杠"\"
	std::wstring GetAppPathW();
	std::string GetAppPathA();

	//取应用程序全路径
	std::wstring GetAppFullNameW();
	std::string GetAppFullNameA();

	//取应用程序名字
	std::wstring GetAppNameW();
	std::string GetAppNameA();

	//返回文件是否存在
	bool IsFileExistsA(LPCSTR szFileName);
	bool IsFileExistsW(LPCWSTR szFileName);

	//字符串加解密功能块
	std::string EncodeString(std::string& str);
	std::string DecodeString(std::string& str);
	void Decode6BitBuf(char* pSource, char* pBuf, int iSrcLen, int iBufLen);
	void Encode6BitBuf(char* pSource, char* pDest, int iSrcLen, int iDestLen);

	//格式化字符串,返回unicode
	std::wstring FormatWStr(LPCWSTR szFormat, ...);

	//格式化字符串,返回普通字符串
	std::string FormatStr(LPCSTR szFormat, ...);

	//宽字符串转化为大写
	std::wstring WStrUpper(const std::wstring& str);

	//普通字符串转化为大写
	std::string StrUpper(const std::string& str);

	//宽字符串转化为小写
	std::wstring WStrLower(const std::wstring& str);

	//普通字符串转化为小写
	std::string StrLower(const std::string& str);

	
}

/*
安全删除对象指针
*/
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)	{ if (p) { delete (p); (p)=NULL; }}
#endif

/*
增加标志
*/
#ifndef ADD_FLAG
#define ADD_FLAG(body, flag) body|=(flag)
#endif
/*
删除标志
*/
#ifndef DEL_FLAG
#define DEL_FLAG(body, flag) body&=~(flag)
#endif

/*
是否包含标志
*/
#ifndef HAS_FLAG
#define HAS_FLAG(body, flag) ((body & (flag)) != 0)
#endif

#endif //__CC_UTILS_H__