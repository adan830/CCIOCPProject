/**************************************************************************************
@author: ³Â²ý
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
	CMySQLRecord::CMySQLRecord(MYSQL_RES* Res, TOnMySQLErrorEvent ErrorCallBack)
	{}

	CMySQLRecord::~CMySQLRecord()
	{}

	int CMySQLRecord::FieldCount()
	{}

	int CMySQLRecord::RecordCount()
	{}

	PMySQLField CMySQLRecord::FieldByName(const std::string &sFieldName)
	{}

	PMySQLField CMySQLRecord::FieldByIdx(int iFieldIdx)
	{}

	bool CMySQLRecord::Eof()
	{}

	void CMySQLRecord::First()
	{}

	void CMySQLRecord::Next()
	{}

	void CMySQLRecord::Seek(int iRow)
	{}

	std::string CMySQLRecord::GetValue(int iFieldIdx)
	{}

	void CMySQLRecord::WriteLog(const std::string sErrMsg)
	{}
	/************************End Of CMySQLRecord******************************************/

}