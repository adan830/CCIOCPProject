/**************************************************************************************
@author: 陈昌
@content: 数据库操作管理器，多线程处理
**************************************************************************************/
#include "stdafx.h"
#include "CSQLDBManager.h"

const std::string SQL_LOGINLOG_PROC = "PW_GS_Passport_CreateLoginLog";

/************************Start Of CSingleSQLWorker******************************************/
CSingleSQLWorker::CSingleSQLWorker(CSQLWorkerUnit* owner, unsigned short usIdx, const std::string &sConnectStr) : m_pOwner(owner), m_usThreadIdx(usIdx), m_sConnectStr(sConnectStr)
{}

CSingleSQLWorker::~CSingleSQLWorker()
{
	WaitThreadExecuteOver();
}

void CSingleSQLWorker::DoExecute()
{}

std::string CSingleSQLWorker::_EscapeString(const std::string &str)
{
	return m_pMySQLProc->EscapeString(const_cast<char*>(str.c_str()), str.length());
}

void CSingleSQLWorker::MySQLAuthenRes(Json::Value js, PJsonJobNode pNode, IMySQLFields* pDataSet, AccountFlag: TAccountFlagInfo)
{}

void CSingleSQLWorker::OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg)
{
	if (0 == uiErrorCode)
		Log(sErrMsg, lmtMessage);
	else
		Log(sErrMsg, lmtError);
}

bool CSingleSQLWorker::SQLDB_Authen(PJsonJobNode pNode)
{

}

bool CSingleSQLWorker::SQLDB_Regist(PJsonJobNode pNode)
{

}

//认证成功后记录
bool CSingleSQLWorker::SQLDB_AuthenLog(PJsonJobNode pNode)
{
#ifdef TEST2
	Log("[SQLDB_AuthenLog]:" + pNode->sJsonText);
#endif
	bool retFlag = true;
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(pNode->sJsonText, root))
	{
		std::string sLogSQL = "call " + SQL_LOGINLOG_PROC + "(" + root.get("UniqueID", "").asString() + ", " + root.get("AppID", "").asString() + ", " 
							+ root.get("AreaID", "").asString() + ", 0, \"" + root.get("ClientIP", "").asString() + "\",\"" + root.get("Mac", "").asString() + "\", 1);";
		int iAffected = 0;
		IMySQLFields* pDataSet = nullptr;
		retFlag = m_pMySQLProc->Exec(sLogSQL, pDataSet, iAffected);
	}
	return retFlag;
}

bool CSingleSQLWorker::SQLDB_SafeCardAuthen(PJsonJobNode pNode)
{
	/*
var
  nResult           : integer;
  SQL               : AnsiString;
  DataSet           : IMySQLFields;
  Affected          : integer;
  js                : TlkJSONobject;
begin
  nResult := 0;
  Result := False;
  {$IFDEF TEST}
  LOG('[SQLDB_SafeCardAuthen]:' + nNode^.JsonText);
  {$ENDIF}
  js := TlkJSON.ParseText(nNode^.JsonText) as TlkJSONobject;
  if Assigned(js) then
  try
    with js, FMySQLProc do
    begin
      SQL := Format('call %s(%s, "%s", "%s", "%s", %d);', [
        SQL_SAFECARD_AUTHEN_PROC,
          GetStringValue(Field['UniqueID']),
          GetStringValue(Field['ClientIP']),
          GetStringValue(Field['Position']),
          GetStringValue(Field['PosValue']),
          StrToIntDef(GetStringValue(Field['VerifyStyle']), 2)
          ]);
      if Exec(SQL, DataSet, Affected) then
      begin
        Result := True;
        if Assigned(DataSet) then
        begin
          nResult := DataSet.FieldbyName('ReturnCode').AsInteger;
          nNode^.nRes := nResult;                           // 返回值
        end;
      end;
      Add('Result', nResult);
      case nResult of
        0: Add('Message', MSG_SAFECARD_AUTHEN_ERROR);       // 'SQL Execute Error.'
        1: ;
        -1, -2, -3, -4, -5, -9: Add('Message', MSG_SAFECARD_AUTHEN_ERROR);
      else
        Add('Message', MSG_SAFECARD_AUTHEN_ERROR);          // 'Unknown SQL procedure result.'
      end;
      if Assigned(G_ServerSocket) then
        with nNode^ do
          G_ServerSocket.SQLJobResponse(Cmd, Handle, nParam, nRes, TlkJSON.GenerateText(js));
    end;
    {$IFDEF TEST}
    if nResult <> 1 then
      Log('SafeCardAuth Fail: ' + TlkJSON.GenerateText(js));
    {$ENDIF}
  finally
    js.Free;
  end;
end;
	*/
}
/************************End Of CSingleSQLWorker********************************************/