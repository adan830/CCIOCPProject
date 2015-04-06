/**************************************************************************************
@author: 陈昌
@content: 生成日志文件类
**************************************************************************************/
#ifndef __CC_FILE_LOG_H__
#define __CC_FILE_LOG_H__

#include <string>
#include <mutex>

namespace CC_UTILS{

	class CFileLogManager
	{
	public:
		CFileLogManager(const std::string &sLogName);
		virtual ~CFileLogManager();
		void WriteLog(const std::string &sLogStr);
	private:
		void WriteCache();
	private:
		std::mutex m_LogCS;
		char* m_pCache;
		int m_iCacheLen;
		std::string m_sName;
		std::string m_sPath;
		unsigned int m_uiLastWriteTick;
		unsigned int m_uiLastDay;
	};
}

#endif //__CC_FILE_LOG_H__