/**************************************************************************************
@author: 陈昌
@content: 数据库操作管理器，多线程处理
**************************************************************************************/
#include "stdafx.h"
#include "CSQLDBManager.h"

const std::string SQL_LOGINLOG_PROC = "PW_GS_Passport_CreateLoginLog";
const std::string SQL_SAFECARD_AUTHEN_PROC = "P_GS_Usr_Securitycard_Verify";
const std::string SQL_REGIST_PROC = "***";

#ifdef TEST
const std::string SQL_AUTHEN_PROC = 'P_GS_ForTest';
#else
const std::string SQL_AUTHEN_PROC = "PR_GS_Passport_Login";
#endif

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

void CSingleSQLWorker::MySQLAuthenRes(Json::Value js, PJsonJobNode pNode, IMySQLFields* pDataSet, TAccountFlagInfo AccountFlag)
{

}

void CSingleSQLWorker::OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg)
{
	if (0 == uiErrorCode)
		Log(sErrMsg, lmtMessage);
	else
		Log(sErrMsg, lmtError);
}

bool CSingleSQLWorker::SQLDB_Authen(PJsonJobNode pNode)
{
#ifdef TEST2
	Log("[SQLDB_Authen]:" + pNode->sJsonText);
#endif
	bool retFlag = false;
	int iRetCode = 0;
	TAccountFlagInfo info;
	info.iFlag = 0;
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(pNode->sJsonText, root))
	{
		std::string sAccount = root.get("AuthenID", "").asString();
		std::string sIP = root.get("ClientIP", "").asString();
		std::string sMac = root.get("Mac", "").asString();
		std::string sAreaID = root.get("AreaID", "").asString();
		std::string sPwd = ""; //------------------------------------- MD5Print(MD5String(LowerCase(GetStringValue(Field['Pwd'])) + G_PWD_MD5_KEY), True);
		std::string sSql = "call " + SQL_AUTHEN_PROC + "(\"" + _EscapeString(sAccount) + "\", \"" + _EscapeString(sPwd) + "\", " + root.get("AppID", "").asString 
			+ ", " + sAreaID + ", 0, \"" + sIP + "\");";
		int iAffected = 0;
		IMySQLFields* pDataSet = nullptr;
		if (m_pMySQLProc->Exec(sSql, pDataSet, iAffected))
		{
			retFlag = true;
			if (pDataSet != nullptr)
			{
				iRetCode = pDataSet->FieldByName("ReturnCode")->AsInteger();
				pNode->iRes = iRetCode;
			}
		}
		switch (iRetCode)
		{
		case 0:
			root["Message"] = MSG_AUTHEN_ERROR;
			break;
		case 1:
		case 2:
		case 3:
			break;
		case -1:
			break;
		case -6:
			break;
		case -2:
		case -3:
		case -4:
		case -5:
		case -9:
			break;
		case -10:
			break;
		default:
			root["Message"] = MSG_AUTHEN_ERROR;
			break;
		}
		/*
      case nResult of
        0: Add('Message', MSG_AUTHEN_ERROR);                // 'SQL Execute Error.'
        1, 2, 3:
          begin
            if nResult = 2 then                             //需要修改密码
            begin
              Include(AccountFlag.FlagSet, accNeedModifyPwd);
              nResult := 1;
            end
            else if nResult = 3 then                        //非安全账号
            begin
              Include(AccountFlag.FlagSet, accNoSafeAccount);
              nResult := 1;
            end;
            MyAuthenRes(js, nNode, DataSet, AccountFlag);
          end;
        -1: Add('Message', MSG_AUTHEN_ERROR);               // 'Account does not exist.'
        -6: Add('Message', MSG_ACCOUNT_DENY);
        -2, -3, -4, -5, -9: Add('Message', MSG_AUTHEN_ERROR);
        -10: Add('Message', MSG_PWD_EASY);
      else
        Add('Message', MSG_AUTHEN_ERROR);                   // 'Unknown SQL procedure result.'
      end;
		*/

#ifdef TEST
		if (iRetCode != 1)
		{
			Json::FastWriter writer;
			std::string sLogStr = writer.write(root);
			Log("Login Fail: " + std::to_string(iRetCode) + " " + sLogStr);
		}			
#endif
	}

	/*
var
  nResult           : integer;
  SQL, Account, IP, Mac, AreaID, Pwd: AnsiString;
  DataSet           : IMySQLFields;
  Affected          : integer;
  js                : TlkJSONobject;
  AccountFlag       : TAccountFlagInfo;
begin
  {$IFDEF TEST2}
  LOG('[SQLDB_Authen]:' + nNode^.JsonText);
  {$ENDIF}
  Result := False;
  nResult := 0;
  AccountFlag.nFlag := 0;
  js := TlkJSON.ParseText(nNode^.JsonText) as TlkJSONobject;
  if Assigned(js) then
  try
    with js, FMySQLProc do
    begin
      Account := GetStringValue(Field['AuthenID']);
      IP := GetStringValue(Field['ClientIP']);
      Mac := GetStringValue(Field['Mac']);
      AreaID := GetStringValue(Field['AreaID']);
      Pwd := MD5Print(MD5String(LowerCase(GetStringValue(Field['Pwd'])) + G_PWD_MD5_KEY), True);
      SQL := Format('call %s("%s", "%s", %s, %s, %d, "%s");', [
        SQL_AUTHEN_PROC,
          _Escape_string(Account),
          _Escape_string(Pwd),
          GetStringValue(Field['AppID']),
          AreaID,
          0,
          IP
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
      case nResult of
        0: Add('Message', MSG_AUTHEN_ERROR);                // 'SQL Execute Error.'
        1, 2, 3:
          begin
            if nResult = 2 then                             //需要修改密码
            begin
              Include(AccountFlag.FlagSet, accNeedModifyPwd);
              nResult := 1;
            end
            else if nResult = 3 then                        //非安全账号
            begin
              Include(AccountFlag.FlagSet, accNoSafeAccount);
              nResult := 1;
            end;
            MyAuthenRes(js, nNode, DataSet, AccountFlag);
          end;
        -1: Add('Message', MSG_AUTHEN_ERROR);               // 'Account does not exist.'
        -6: Add('Message', MSG_ACCOUNT_DENY);
        -2, -3, -4, -5, -9: Add('Message', MSG_AUTHEN_ERROR);
        -10: Add('Message', MSG_PWD_EASY);
      else
        Add('Message', MSG_AUTHEN_ERROR);                   // 'Unknown SQL procedure result.'
      end;
      if nResult <> 1 then
      begin
        G_AuthFailLog.WriteLog(Format('%s,%s,%s,%s,%s,%s,%d'#13#10, [FormatDateTime('yyyy-mm-dd hh:nn:ss', Now()), AreaID, Account, IP, Mac, Pwd, nResult]));
        nResult := -9;
      end;
      Add('Result', nResult);
      if Assigned(G_ServerSocket) then
      begin
        Field['Pwd'].Value := '';
        with nNode^ do
          G_ServerSocket.SQLJobResponse(Cmd, Handle, nParam, nRes, TlkJSON.GenerateText(js));
      end;
    end;
    {$IFDEF TEST}
    if nResult <> 1 then
      Log('Login Fail: ' + IntToStr(nResult) + ' ' + TlkJSON.GenerateText(js));
    {$ENDIF}
  finally
    js.Free;
  end;
end;

	*/
}

bool CSingleSQLWorker::SQLDB_Regist(PJsonJobNode pNode)
{
	/*
var
  nResult           : Integer;
  SQL               : AnsiString;
  DataSet           : IMySQLFields;
  Affected          : integer;
  js                : TlkJSONobject;
  AccountFlag       : TAccountFlagInfo;
begin
  {$IFDEF TEST2}
  LOG('[SQLDB_Regist]:' + nNode^.JsonText);
  {$ENDIF}
  nResult := 0;
  Result := False;
  AccountFlag.nFlag := 0;
  js := TlkJSON.ParseText(nNode^.JsonText) as TlkJSONobject;
  if Assigned(js) then
  try
    with js, FMySQLProc do
    begin
      SQL := Format('call %s("%s", "%s", "%s", "%s", "%s", "%s", %s, %s,"%s");', [
        SQL_REGIST_PROC,
          _Escape_string(GetStringValue(Field['AuthenID'])),
          _Escape_string(MD5Print(MD5String(LowerCase(GetStringValue(Field['Pwd'])) + G_PWD_MD5_KEY), True)),
          _Escape_string(GetStringValue(Field['UserName'])),
          _Escape_string(GetStringValue(Field['UserID'])),
          _Escape_string(GetStringValue(Field['EMail'])),
          GetStringValue(Field['ClientIP']),
          GetStringValue(Field['IsAdult']),
          GetStringValue(Field['AppID']),
          GetStringValue(Field['Mac'])
          ]);
      if Exec(SQL, DataSet, Affected) then
      begin
        Result := True;
        if Assigned(DataSet) then
        begin
          nResult := DataSet.FieldbyName('ReturnCode').AsInteger;
          nNode^.nRes := nResult;
        end;
      end;
    end;
    if Assigned(G_ServerSocket) then
    begin
      with js, nNode^ do
      begin
        case nResult of
          0: Add('Message', MSG_REGISTER_ERROR);            // 'SQL Execute Error.'
          1:
            begin
              nNode^.Cmd := SM_USER_AUTHEN_REQ;             // 当成认证返回
              MYAuthenRes(js, nNode, DataSet, AccountFlag); // 回调主服务器
            end;
          -1: Add('Message', MSG_ACCOUNT_EXIST);            // 'Account is exist.'
          -2: Add('Message', MSG_EMAIL_EXIST);              // 'EMail is exist.');
          -9: Add('Message', MSG_REGISTER_ERROR);           //  'interface Error.'
        else
          Add('Message', MSG_REGISTER_ERROR);               // 'Unknown SQL procedure result.'
        end;
        Field['Pwd'].Value := '';
        Add('Result', nResult);
        G_ServerSocket.SQLJobResponse(Cmd, Handle, nParam, nRes, TlkJSON.GenerateText(js));
      end;
    end;
    {$IFDEF TEST}
    if nResult <> 1 then
      Log('Regist Fail: ' + IntToStr(nResult) + ' ' + TlkJSON.GenerateText(js));
    {$ENDIF}
  finally
    js.Free;
  end;
end;
	*/
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
	int iRetCode = 0;
	bool retFlag = false;
#ifdef TEST
	Log("[SQLDB_SafeCardAuthen]:" + pNode->sJsonText);
#endif
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(pNode->sJsonText, root))
	{
		std::string sSql = "call " + SQL_SAFECARD_AUTHEN_PROC + "(" + root.get("UniqueID", "").asString() + ", \"" + root.get("ClientIP", "").asString() + "\", \"" 
			+ root.get("Position", "").asString() + "\", \"" + root.get("PosValue", "").asString() + "\", " + std::to_string(StrToIntDef(root.get("VerifyStyle", "").asString(), 2)) + ");";
		int iAffected = 0;
		IMySQLFields* pDataSet = nullptr;
		if (m_pMySQLProc->Exec(sSql, pDataSet, iAffected))
		{
			retFlag = true;
			if (pDataSet != nullptr)
			{
				iRetCode = pDataSet->FieldByName("ReturnCode")->AsInteger();
				pNode->iRes = iRetCode;
			}
		}
		root["Result"] = iRetCode;
		switch (iRetCode)
		{
		case 0:
			//sql执行错误
			root["Message"] = MSG_SAFECARD_AUTHEN_ERROR;
			break;
		case 1:
			//成功
			break;
		case -1:
		case -2:
		case -3:
		case -4:
		case -5:
		case -9:
			//各种认证错误 
			root["Message"] = MSG_SAFECARD_AUTHEN_ERROR;
			break;
		default:
			//其它异常未知sql执行结果
			root["Message"] = MSG_SAFECARD_AUTHEN_ERROR;
			break;
		}
		/*
		//--------------------------------
		//--------------------------------
		//--------------------------------
		  if Assigned(G_ServerSocket) then
			with nNode^ do
			  G_ServerSocket.SQLJobResponse(Cmd, Handle, nParam, nRes, TlkJSON.GenerateText(js));
		*/
	}
#ifdef TEST
	if (iRetCode != 1)
		Log("SafeCardAuth Fail: " + pNode->sJsonText);
#endif
	return retFlag;
}
/************************End Of CSingleSQLWorker********************************************/