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
		m_OnError = std::bind(&ErrorCallBack, this, std::placeholders::_1, std::placeholders::_2);
		m_iFieldsCount = mysql_num_fields(Res);
		m_Fields.reserve(m_iFieldsCount);
		MYSQL_FIELD* pField;
		for (int i = 0; i < m_Fields.size(); i++)
		{
			pField = mysql_fetch_field(Res);
			m_Fields[i].Field_Idx = i;
			if (pField != nullptr)
			{
				m_Fields[i].Field_Name = pField->name;
				m_Fields[i].Field_Type = pField->type;
				m_Fields[i].Field_Length = pField->length;
				m_Fields[i]._OnGetValue = std::bind(&CMySQLRecord::GetValue, this, std::placeholders::_1);
			}
		}

		m_Values.reserve(m_iFieldsCount);
		MYSQL_ROW pRow = mysql_fetch_row(Res);
		unsigned long* pLen = mysql_fetch_lengths(Res);
		for (int i = 0; i < m_Fields.size(); i++)
		{
			if (pRow != nullptr)
			{
				//----------------------------------------
				//----------------------------------------
				//这里的长度是否正确？？？？
				//----------------------------------------
				//----------------------------------------
				m_Values[i].resize(pLen[i]+1);
				memcpy_s((void*)m_Values[i].c_str(), pLen[i] + 1, pRow[i], pLen[i] + 1);
			}
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
		for (int i = 0; i < m_Fields.size(); i++)
		{
			if (0 == sFieldName.compare(m_Fields[i].Field_Name))
				return &m_Fields[i];
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
					sprintf(sTemp, "%c", 1);
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
		m_OnError = std::bind(&ErrorCallBack, this, std::placeholders::_1, std::placeholders::_2);
		m_iFieldsCount = mysql_num_fields(Res);
		m_Fields.reserve(m_iFieldsCount);
		MYSQL_FIELD* pField;
		for (int i = 0; i < m_Fields.size(); i++)
		{
			pField = mysql_fetch_field(Res);
			m_Fields[i].Field_Idx = i;
			if (pField != nullptr)
			{
				m_Fields[i].Field_Name = pField->name;
				m_Fields[i].Field_Type = pField->type;
				m_Fields[i].Field_Length = pField->length;
				m_Fields[i]._OnGetValue = std::bind(&CMySQLRecords::GetValue, this, std::placeholders::_1);
			}
		}
		m_iRecordCount = mysql_num_rows(Res);
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
		for (int i = 0; i < m_Fields.size(); i++)
		{
			if (0 == sFieldName.compare(m_Fields[i].Field_Name))
				return &m_Fields[i];
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

	std::string CMySQLRecords::GetValue(int iFieldIdx)
	{
		/*
  Result := '';
  if Assigned(m_Current_Rows) and (FieldIdx >= 0) and (FieldIdx < m_FieldsCount) then
  begin
    Result := m_Current_Rows.Data^[FieldIdx];
    if m_Fields[FieldIdx].Field_Type = MYSQL_TYPE_BIT then
    begin
      if Result = #1 then
        Result := '1'
      else if Result = #0 then
        Result := '0';
    end;
  end;
		*/
	}

	void CMySQLRecords::WriteLog(const std::string &sErrMsg)
	{
		if (m_OnError != nullptr)
			m_OnError((unsigned int)-1, sErrMsg);
	}

	/************************End Of CMySQLRecords******************************************/

}