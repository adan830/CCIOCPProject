/**************************************************************************************
@author: 陈昌
@content: 生成日志文件类
**************************************************************************************/

#include "CCFileLog.h"
#include "CCUtils.h"

namespace CC_UTILS{

	const int MAX_FILE_CACHE_SIZE = 8 * 1024;
	const int WRITE_INTERVAL = 300 * 1000;

/************************Start Of CCFileLog******************************************/

	CFileLogManager::CFileLogManager(const std::string &sLogName) : m_sName(sLogName), m_iCacheLen(0), m_uiLastWriteTick(0)
	{
		m_uiLastDay = GetTodayNum();
		if ("" == sLogName)
			m_sPath = "" + "logs\\";
		else
			m_sPath = "" + "logs\\" + sLogName + "\\";
		m_pCache = (char*)malloc(MAX_FILE_CACHE_SIZE);
		try
		{
			ForceCreateDirectories(m_sPath);
		}
		catch (...)
		{
		}
	}

	CFileLogManager::~CFileLogManager()
	{
		WriteCache();
		free(m_pCache);
	}

	void CFileLogManager::WriteLog(const std::string &sLogStr)
	{
	
	}

	void CFileLogManager::WriteCache()
	{
		if (m_iCacheLen > 0)
		{
			
		}
		/*
var
  FileName          : ansistring;
  fiHandle          : integer;
begin
  if m_CacheLen > 0 then
  begin
    if not DirectoryExists(m_Path) then
      ForceDirectories(m_Path);
    FileName := m_Path + FormatDateTime('yyyymmdd', Now()) + '.txt';
    EnterCriticalSection(m_LogCS);
    try
      if FileExists(FileName) then
        fiHandle := FileOpen(FileName, fmOpenWrite)
      else
        fiHandle := FileCreate(FileName);
      if fiHandle > 0 then
      begin
        try
          if FileSeek(fiHandle, 0, FILE_END) <> -1 then
          begin
            FileWrite(fiHandle, m_Cache^, m_CacheLen);
          end;
        finally
          FileClose(fiHandle);
        end;
      end;
    finally
      m_CacheLen := 0;
      LeaveCriticalSection(m_LogCS);
    end;
    m_LastWriteTick := GetTickCount;
  end;
end;
		*/
	}

/************************End Of CCFileLog******************************************/

}