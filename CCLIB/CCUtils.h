/**************************************************************************************
@author: �²�
@content: �Լ�ʹ�õ�һ�����ýṹ������
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

	//���׵�Hash���
	typedef struct _THashPortItem
	{
		int iHandle;
		void* pItem;
		_THashPortItem* Next;
	}THashPortItem, *PHashPortItem, **PPHashPortItem;

	/**
	* һ�����׵�hash�������ͻ������
	*/
	typedef struct _TSimpleHash
	{
	public:
		void DoInitial(int iSize);
		void AddPortItem(const int iKey, void* pClient);
		void RemovePortItem(const int iKey);
		void ClearAllPortItems();
		PPHashPortItem FindPortItemPointer(const int iKey);     //������Ҫ���ص���PortItem��ָ��
		int GetItemCount();
	private:
		PPHashPortItem m_ppItemBuckets;							//���ڴ洢�ͻ��˶���ļ���hash
		int m_iBucketSize;                                      //�̶������鳤��
		int m_iHashItemCount;  				     			    //��ǰ�����еĿͻ��˾������
	}TSimpleHash, *PSimpleHash;

	/**
	* һ�����׵�buffer��д����������Ҫ����һ������ά���Ľ���buffer����
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
	* ΢���ý��API�����ʱ�Ӷ���
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

	//�����ļ�������޸�ʱ��ת���ɵ����������Լ���ļ��Ƿ��޸�
	int GetFileAge(const std::string &sFileName);

	//�����ļ��İ汾��Ϣ  
	//ע�⣺��Ҫ�����м���version.lib
	std::string GetFileVersion(const std::string &sFileName);

	//�ַ����ָ�
	void SplitStr(const std::string& s, const std::string& delim, std::vector<std::string>* ret);

	//���л��з��ŵ��ı��ַ���,�и��Ž�vector��
	void SplitStrByLine(std::string& s, std::vector<std::string>* ret);

	//�ַ���ת��������Ĭ��ֵ�����׳��쳣
	int StrToIntDef(const std::string& sTemp, const int iDef);

	//�ַ���ת64λ��������Ĭ��ֵ�����׳��쳣
	int64_t StrToInt64Def(const std::string& sTemp, const int64_t iDef);

	//���ص�ǰ��Ӧ����������
	int GetTodayNum();

	//ѭ������Ŀ¼
	int ForceCreateDirectories(std::string& sDir);

	//������·����ȡ���ļ���·��
	inline std::wstring ExtractFilePathW(const std::wstring& strFullPath);
	inline std::string ExtractFilePathA(const std::string& strFullPath);

	//������·����ȡ���ļ���
	std::wstring ExtractFileNameW(const std::wstring& strFullPath);
	std::string ExtractFileNameA(const std::string& strFullPath);

	//ȡ�ó���ǰ��·�� ������б��"\"
	std::wstring GetAppPathW();
	std::string GetAppPathA();

	//ȡӦ�ó���ȫ·��
	std::wstring GetAppFullNameW();
	std::string GetAppFullNameA();

	//ȡӦ�ó�������
	std::wstring GetAppNameW();
	std::string GetAppNameA();

	//�����ļ��Ƿ����
	bool IsFileExistsA(LPCSTR szFileName);
	bool IsFileExistsW(LPCWSTR szFileName);

	//�ַ����ӽ��ܹ��ܿ�
	std::string EncodeString(std::string& str);
	std::string DecodeString(std::string& str);
	void Decode6BitBuf(char* pSource, char* pBuf, int iSrcLen, int iBufLen);
	void Encode6BitBuf(char* pSource, char* pDest, int iSrcLen, int iDestLen);

	//��ʽ���ַ���,����unicode
	std::wstring FormatWStr(LPCWSTR szFormat, ...);

	//��ʽ���ַ���,������ͨ�ַ���
	std::string FormatStr(LPCSTR szFormat, ...);

	//���ַ���ת��Ϊ��д
	std::wstring WStrUpper(const std::wstring& str);

	//��ͨ�ַ���ת��Ϊ��д
	std::string StrUpper(const std::string& str);

	//���ַ���ת��ΪСд
	std::wstring WStrLower(const std::wstring& str);

	//��ͨ�ַ���ת��ΪСд
	std::string StrLower(const std::string& str);

	
}

/*
��ȫɾ������ָ��
*/
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)	{ if (p) { delete (p); (p)=NULL; }}
#endif

/*
���ӱ�־
*/
#ifndef ADD_FLAG
#define ADD_FLAG(body, flag) body|=(flag)
#endif
/*
ɾ����־
*/
#ifndef DEL_FLAG
#define DEL_FLAG(body, flag) body&=~(flag)
#endif

/*
�Ƿ������־
*/
#ifndef HAS_FLAG
#define HAS_FLAG(body, flag) ((body & (flag)) != 0)
#endif

#endif //__CC_UTILS_H__