/**************************************************************************************
@author: 陈昌
@content:
**************************************************************************************/

#include "CCMySQLEngn.h"
#include "CCUtils.h"
#include "mysql_com.h"
#include "mysql_version.h"

#pragma comment(lib, "libmysql.lib")

namespace CC_UTILS{

	/************************Start Of _TMySQLField******************************************/
	std::string _TMySQLField::AsString()
	{
		return _OnGetValue(Field_Idx);
	}

	double _TMySQLField::AsDateTime()
	{
		/*
var
  FormatStr         : TFormatSettings;
begin
  GetLocaleFormatSettings(0, FormatStr);
  FormatStr.DateSeparator := '-';
  FormatStr.ShortDateFormat := 'yyyy-mm-dd';
  FormatStr.ShortTimeFormat := 'hh:nn:ss';
  Result := StrToDateTime(AsString, FormatStr);
end;
		*/
		return 0.0;
	}

	int _TMySQLField::AsInteger()
	{
		return (int)StrToInt64Def(AsString(), 0);
	}

	int64 _TMySQLField::AsInt64()
	{
		return StrToInt64Def(AsString(), 0);
	}
	/************************End Of _TMySQLField******************************************/



	/************************Start Of CMySQLRecord******************************************/
	CMySQLRecord::CMySQLRecord(MYSQL_RES* Res, TOnMySQLErrorEvent ErrorCallBack) : m_bEof(false), m_Fields(0), m_Values(0)
	{
		m_OnError = ErrorCallBack;
		m_iFieldsCount = mysql_num_fields(Res);
		m_Fields.reserve(m_iFieldsCount);
		MYSQL_FIELD* pField;

		std::vector<TMySQLField>::iterator vIter;
		int i = 0;
		for (vIter = m_Fields.begin(); vIter != m_Fields.end(); ++vIter)
		{
			pField = mysql_fetch_field(Res);
			(*vIter).Field_Idx = i;
			if (pField != nullptr)
			{
				(*vIter).Field_Name = pField->name;
				(*vIter).Field_Type = pField->type;
				(*vIter).Field_Length = pField->length;
				(*vIter)._OnGetValue = std::bind(&CMySQLRecord::GetValue, this, std::placeholders::_1);
			}
			i++;
		}

		m_Values.reserve(m_iFieldsCount);
		MYSQL_ROW pRow = mysql_fetch_row(Res);
		unsigned long* pLen = mysql_fetch_lengths(Res);
		std::vector<std::string>::iterator vIter1;
		i = 0;
		for (vIter1 = m_Values.begin(); vIter1 != m_Values.end(); ++vIter1)
		{
			if (pRow != nullptr)
			{
				//----------------------------------------
				//----------------------------------------
				//这里的长度是否正确？？？？
				//----------------------------------------
				//----------------------------------------
				(*vIter1).resize(pLen[i] + 1);
				memcpy_s((void*)(*vIter1).c_str(), pLen[i] + 1, pRow[i], pLen[i] + 1);
			}
			i++;
		}
		mysql_free_result(Res);
	}

	CMySQLRecord::~CMySQLRecord()
	{
		m_Fields.reserve(0);
		m_Values.reserve(0);
	}

	int CMySQLRecord::FieldCount()
	{
		return m_iFieldsCount;
	}

	int CMySQLRecord::RecordCount()
	{
		return 1;
	}

	PMySQLField CMySQLRecord::FieldByName(const std::string &sFieldName)
	{
		std::vector<TMySQLField>::iterator vIter;
		for (vIter = m_Fields.begin(); vIter != m_Fields.end(); ++vIter)
		{
			if (0 == sFieldName.compare((*vIter).Field_Name))
				return &(*vIter);
		}

		WriteLog("Field Not Exist: " + sFieldName);
		return nullptr;
	}

	PMySQLField CMySQLRecord::FieldByIdx(int iFieldIdx)
	{
		if ((iFieldIdx >= 0) && (iFieldIdx < m_iFieldsCount))
			return &m_Fields[iFieldIdx];

		WriteLog("Field Not Exist: " + std::to_string(iFieldIdx));
		return nullptr;
	}

	bool CMySQLRecord::Eof()
	{
		return m_bEof;
	}

	void CMySQLRecord::First()
	{
		m_bEof = false;
	}

	void CMySQLRecord::Next()
	{
		m_bEof = true;
	}

	void CMySQLRecord::Seek(int iRow)
	{}

	//????????????????????????????????
	//????????????????????????????????
	//这个函数是否ok
	std::string CMySQLRecord::GetValue(int iFieldIdx)
	{
		std::string sRet("");
		try
		{
			if ((iFieldIdx >= 0) && (iFieldIdx < m_iFieldsCount))
			{
				if (m_Fields[iFieldIdx].Field_Type = MYSQL_TYPE_BIT)
				{
					//---------------------------------
					//---------------------------------
					//---------------------------------
					//if m_Values[FieldIdx] = #1 then
					char sTemp[5];					
					sprintf_s(sTemp, 5, "%c", 1);
					if (0 == m_Values[iFieldIdx].compare(sTemp))     
						sRet = "1";
					else
						sRet = "0";
				}
				else
					sRet = m_Values[iFieldIdx];
			}
		}
		catch (...)
		{
			WriteLog("GetValue Exception: " + std::to_string(iFieldIdx));
		}
		return sRet;
	}

	void CMySQLRecord::WriteLog(const std::string sErrMsg)
	{
		if (m_OnError != nullptr)
			m_OnError((unsigned int)-1, sErrMsg);
	}
	/************************End Of CMySQLRecord******************************************/


	/************************Start Of CMySQLRecords******************************************/

	CMySQLRecords::CMySQLRecords(MYSQL_RES* Res, TOnMySQLErrorEvent ErrorCallBack) : m_bEof(false), m_iFieldsCount(0), m_iRecordCount(0), m_Res(Res), m_CurrentRows(nullptr), m_Fields(0)
	{
		m_OnError = ErrorCallBack;
		m_iFieldsCount = mysql_num_fields(Res);
		m_Fields.reserve(m_iFieldsCount);
		MYSQL_FIELD* pField;
		std::vector<TMySQLField>::iterator vIter;
		int i = 0;
		for (vIter = m_Fields.begin(); vIter != m_Fields.end(); ++vIter)
		{
			pField = mysql_fetch_field(Res);
			(*vIter).Field_Idx = i;
			if (pField != nullptr)
			{
				(*vIter).Field_Name = pField->name;
				(*vIter).Field_Type = pField->type;
				(*vIter).Field_Length = pField->length;
				(*vIter)._OnGetValue = std::bind(&CMySQLRecords::GetValue, this, std::placeholders::_1);
			}
			i++;
		}

		m_iRecordCount = (int)mysql_num_rows(Res);
		if (m_iRecordCount > 0)
			First();
	}

	CMySQLRecords::~CMySQLRecords()
	{
		m_Fields.reserve(0);
		MYSQL_RES* pTemp = nullptr;
		if (m_Res != nullptr)
		{
			pTemp = m_Res;
			m_Res = nullptr;
			mysql_free_result(pTemp);
		}
	}

	int CMySQLRecords::FieldCount()
	{
		return m_iFieldsCount;
	}

	int CMySQLRecords::RecordCount()
	{
		return m_iRecordCount;
	}

	PMySQLField CMySQLRecords::FieldByName(const std::string &sFieldName)
	{
		std::vector<TMySQLField>::iterator vIter;
		for (vIter = m_Fields.begin(); vIter != m_Fields.end(); ++vIter)
		{
			if (0 == sFieldName.compare((*vIter).Field_Name))
				return &(*vIter);
		}

		WriteLog("Field Not Exist: " + sFieldName);
		return nullptr;
	}

	PMySQLField CMySQLRecords::FieldByIdx(int iFieldIdx)
	{
		if ((iFieldIdx >= 0) && (iFieldIdx < m_iFieldsCount))
			return &m_Fields[iFieldIdx];

		WriteLog("Field Not Exist: " + std::to_string(iFieldIdx));
		return nullptr;
	}

	bool CMySQLRecords::Eof()
	{
		return m_bEof;
	}

	void CMySQLRecords::First()
	{
		m_bEof = true;
		mysql_data_seek(m_Res, 0);
		m_CurrentRows = mysql_row_seek(m_Res, mysql_row_tell(m_Res));
		if (m_CurrentRows != nullptr)
			m_bEof = false;
	}

	void CMySQLRecords::Next()
	{
		if (m_CurrentRows != nullptr)
		{
			m_CurrentRows = m_CurrentRows->next;
			if (m_CurrentRows == nullptr)
				m_bEof = true;
		}
	}

	void CMySQLRecords::Seek(int iRow)
	{
		mysql_data_seek(m_Res, iRow);
		m_CurrentRows = mysql_row_seek(m_Res, mysql_row_tell(m_Res));
		m_bEof = (m_CurrentRows != nullptr);	
	}

	//????????????????????????????????
	//????????????????????????????????
	//这个函数是否ok
	std::string CMySQLRecords::GetValue(int iFieldIdx)
	{
		std::string sRet("");
		if ((m_CurrentRows != nullptr) && (iFieldIdx >= 0) && (iFieldIdx < m_iFieldsCount))
		{
			//-----------------------------------------
			//-----------------------------------------
			//---------这里的取值----------------------
			sRet = *(m_CurrentRows->data[iFieldIdx]);
			if (m_Fields[iFieldIdx].Field_Type = MYSQL_TYPE_BIT)
			{
				/*************
				  if Result = #1 then
					Result := '1'
				  else if Result = #0 then
					Result := '0';
				*************/
				char sTemp[5];
				sprintf_s(sTemp, 5, "%c", 1);
				if (0 == sRet.compare(sTemp))
					sRet = "1";
				else
				{
					sprintf_s(sTemp, 5, "%c", 0);
					if (0 == sRet.compare(sTemp))
						sRet = "0";
				}					
			}
		}
		return sRet;
	}

	void CMySQLRecords::WriteLog(const std::string &sErrMsg)
	{
		if (m_OnError != nullptr)
			m_OnError((unsigned int)-1, sErrMsg);
	}

	/************************End Of CMySQLRecords******************************************/


	/************************Start Of CMySQLManager******************************************/
	CMySQLManager::CMySQLManager() :m_sHostName(""), m_sDBName(""), m_iPort(MYSQL_DEFAULT_PORT), m_sUser(""), m_sPassword(""), m_sCharSet(MYSQL_DEFAULT_CHARSET), m_bAutoReconnect(true), m_bConnected(false),
		m_sConnectString(""), m_bAutoCreateDB(true), m_bFirstConnect(true), m_LibHandle(nullptr), m_bReady(false), m_uiErrorCode(0), m_sErrorMsg("")
	{}

	CMySQLManager::~CMySQLManager()
	{
		Close();
	}

	bool CMySQLManager::Connect(const std::string &sHost, const std::string &sDBName, const std::string &sUser, const std::string &sPwd, std::string sCharSet, int iPort)
	{
		DoSetConnectString(sHost, sDBName, sUser, sPwd, sCharSet, iPort);
		return Open();
	}

	void CMySQLManager::Close()
	{
		if (m_LibHandle != nullptr)
		{
			mysql_close(m_LibHandle);
			m_LibHandle = nullptr;
		}	
	}

	bool CMySQLManager::Open()
	{
		bool retFlag = false;
		m_bConnected = false;
		try
		{
			Close();
			//----------------------------------------
			//----------------------------------------
			//----------------------------------------
			//if libmysql_status = LIBMYSQL_READY then
				m_LibHandle = mysql_init(nullptr);
			Sleep(100);
			if (m_LibHandle != nullptr)
			{
				retFlag = (mysql_real_connect(m_LibHandle, m_sHostName.c_str(), m_sUser.c_str(), m_sPassword.c_str(), m_sDBName.c_str(), m_iPort, nullptr, UNIQUE_FLAG) != nullptr);
				if (retFlag)
				{
					m_bConnected = true;
					// 影响 mysql_real_escape_string，不能用set names
					if (mysql_set_character_set(m_LibHandle, m_sCharSet.c_str()) == 0)
					{
						std::string sSql("set wait_timeout=2124000");
						mysql_real_query(m_LibHandle, sSql.c_str(), sSql.length());
						MYSQL_RES* pRes = mysql_store_result(m_LibHandle);
						if (pRes != nullptr)
							mysql_free_result(pRes);
					}
					if (m_bFirstConnect)
					{
						m_bFirstConnect = false;
						m_bReady = true;
						CheckTables();
					}
					if (m_OnError != nullptr)
						m_OnError(0, "数据库[" + m_sHostName + "." + m_sDBName + "]连接成功");
					if (m_OnConnect != nullptr)
						m_OnConnect(this);
				}
				else
				{
					DoError("[Open]:");
					if ((m_uiErrorCode == ER_BAD_DB_ERROR) && m_bAutoCreateDB)
					{
						retFlag = CreateDataBase(m_sDBName);
						if (retFlag)
							Open();
					}
				}
			}
		}
		catch (...)
		{
			//只是捕获，不处理
		}
		return retFlag;
	}

	bool CMySQLManager::SelectDB(const std::string &sDBName)
	{
		bool retFlag = (mysql_select_db(m_LibHandle, sDBName.c_str()) == 0);
		if (retFlag)
			m_sDBName = sDBName;
		else
			DoError("[Select_DB(" + sDBName + ")]:");
		return retFlag;
	}

	bool CMySQLManager::Exec(const std::string &sSQL, IMySQLFields* pDataSet, int &iAffected)
	{
		bool retFlag = false;
		pDataSet = nullptr;
		iAffected = -1;
		try
		{
			MYSQL_RES* pRes;
			MYSQL_RES* pNextRes;
			if (m_bReady)
			{
				while (mysql_next_result(m_LibHandle) == 0)
				{
					pNextRes = mysql_store_result(m_LibHandle);
					if (pNextRes != nullptr)
						mysql_free_result(pNextRes);
				}

				retFlag = (mysql_real_query(m_LibHandle, sSQL.c_str(), sSQL.length()) == 0);
				if (retFlag)
				{
					pRes = mysql_store_result(m_LibHandle);
					if (pRes != nullptr)
					{
						iAffected = mysql_num_rows(pRes);
						if (iAffected > 0)
						{
							if (1 == iAffected)
								pDataSet = new CMySQLRecord(pRes, m_OnError);
							else
								pDataSet = new CMySQLRecords(pRes, m_OnError);
						}
						else
							mysql_free_result(pRes);
					}
					else
						iAffected = 0;
				}
			}
		}
		catch (...)
		{
			retFlag = false;
			iAffected = -1;
		}

		if (!retFlag)
		{
			DoError("[" + sSQL + "]:");
			CheckConnected();
		}
		return retFlag;
	}

	unsigned int CMySQLManager::EscapeString(char* pSource, unsigned int uiSize, char* pDest)
	{
		unsigned int uiRetCode = 0;
		if (m_LibHandle != nullptr)
			uiRetCode = mysql_real_escape_string(m_LibHandle, pDest, pSource, uiSize);
		return uiRetCode;
	}

	std::string CMySQLManager::EscapeString(char* pSource, unsigned int uiSize)
	{
		std::string sRetStr("");
		if (m_LibHandle != nullptr)
		{
			char* pDest = (char*)malloc(uiSize*2+1);
			if (pDest != nullptr)
			{
				try
				{
					int iDataLen = mysql_real_escape_string(m_LibHandle, pDest, pSource, uiSize);
					if (iDataLen > 0)
						sRetStr.assign(pDest, iDataLen);
					free(pDest);
				}
				catch (...)
				{
					free(pDest);
				}
			}
		}
		return sRetStr;
	}

	std::string CMySQLManager::GetLastError(unsigned int &uiErrorCode)
	{
		uiErrorCode = m_uiErrorCode;
		return m_sErrorMsg;
	}

	bool CMySQLManager::CheckTables()
	{
		return true;
	}

	void CMySQLManager::DoSetConnectString(const std::string &sHost, const std::string &sDBName, const std::string &sUser, const std::string &sPwd, std::string &sCharSet, int iPort)
	{
		m_sHostName = sHost;
		m_sDBName = sDBName;
		m_sUser = sUser;
		m_sPassword = sPwd;
		m_iPort = iPort;
		m_sCharSet = sCharSet;
		m_sConnectString = "Host=" + m_sHostName + ";User=" + m_sUser + ";Pwd=" + m_sPassword + ";Port=" + std::to_string(m_iPort) + ";Database=" + m_sDBName + ";CharSet=" + m_sCharSet + ";";
	}

	void CMySQLManager::DoError(const std::string &sCause)
	{
		m_sErrorMsg = "";
		m_uiErrorCode = mysql_errno(m_LibHandle);
		if (m_uiErrorCode > 0)
		{
			m_sErrorMsg = mysql_error(m_LibHandle);
			if (m_OnError != nullptr)
				m_OnError(m_uiErrorCode, sCause + m_sErrorMsg);
			if (m_bConnected && ((SERVER_GONE_ERROR == m_uiErrorCode) || (CONN_HOST_ERROR == m_uiErrorCode)))
			{
				m_bConnected = false;
				if (m_OnDisconnect != nullptr)
					m_OnDisconnect(this);
			}
		}
	}

	bool CMySQLManager::CreateDataBase(const std::string &sDBName)
	{
		bool retFlag = false;
		//----------------------------------------
		//----------------------------------------
		//----------------------------------------
		//if libmysql_status = LIBMYSQL_READY then
		{
			Close();
			m_LibHandle = mysql_init(nullptr);
			if (m_LibHandle != nullptr)
			{
				retFlag = (mysql_real_connect(m_LibHandle, m_sHostName.c_str(), m_sUser.c_str(), m_sPassword.c_str(), "", m_iPort, nullptr, 0) != nullptr);
				if (retFlag)
				{
					if (mysql_set_character_set(m_LibHandle, m_sCharSet.c_str()) == 0)
					{
						std::string sSql("set wait_timeout=2124000");
						mysql_real_query(m_LibHandle, sSql.c_str(), sSql.length());
						MYSQL_RES* pRes = mysql_store_result(m_LibHandle);
						if (pRes != nullptr)
							mysql_free_result(pRes);
					}
					std::string sSql = "CREATE DATABASE IF NOT EXISTS " + sDBName + " DEFAULT CHARACTER SET utf8;";
					retFlag = (mysql_real_query(m_LibHandle, sSql.c_str(), sSql.length()) == 0);
					MYSQL_RES* pRes = mysql_store_result(m_LibHandle);
					if (pRes != nullptr)
						mysql_free_result(pRes);
				}
			}
		}
		return retFlag;
	}

	void CMySQLManager::CheckConnected()
	{
		if (m_bAutoReconnect && (m_uiErrorCode > 0))
		{
			std::string sSql("Select High_Priority 1;");
			if (mysql_real_query(m_LibHandle, sSql.c_str(), sSql.length()) != 0)
				Open();
		}	
	}

	void CMySQLManager::SetConnectString(const std::string &sValue)
	{
		std::string sHostName, sDBName, sUser, sPwd;
		std::string sCharSet("gbk");
		int iPort = 3306;

		std::string sTempStr(sValue);
		std::vector<std::string> vec;
		SplitStr(sTempStr, ";", &vec);

		int iPos;
		std::string sKey1, sValue1;
		std::vector<std::string>::iterator vIter;
		for (vIter = vec.begin(); vIter != vec.end(); ++vIter)
		{
			//这里的字符串处理需要再检查---------------------
			//这里的字符串处理需要再检查---------------------
			//这里的字符串处理需要再检查---------------------
			iPos = (*vIter).find('=');
			if (iPos != std::string::npos)
			{
				sKey1 = (*vIter).substr(0, iPos - 1);
				sValue1 = (*vIter).substr(iPos + 1, (*vIter).length());

				if (sKey1.compare("Host") == 0)
					sHostName = sValue1;
				else if (sKey1.compare("User") == 0)
					sUser = sValue1;
				else if (sKey1.compare("Pwd") == 0)
					sPwd = sValue1;
				else if (sKey1.compare("Database") == 0)
					sDBName = sValue1;
				else if (sKey1.compare("CharSet") == 0)
					sCharSet = sValue1;
				else if (sKey1.compare("Port") == 0)
					iPort = StrToIntDef(sValue1, iPort);
			}
		}
		DoSetConnectString(sHostName, sDBName, sUser, sPwd, sCharSet, iPort);
	}

	/************************End Of CMySQLManager******************************************/

}