/**************************************************************************************
@author: 陈昌
@content: 数据库操作管理器，多线程处理
**************************************************************************************/
#include "stdafx.h"
#include "CSQLDBManager.h"
#include "CAuthFailLog.h"

const std::string SQL_LOGINLOG_PROC = "PW_GS_Passport_CreateLoginLog";
const std::string SQL_SAFECARD_AUTHEN_PROC = "P_GS_Usr_Securitycard_Verify";
const std::string SQL_REGIST_PROC = "***";

#ifdef TEST
const std::string SQL_AUTHEN_PROC = 'P_GS_ForTest';
#else
const std::string SQL_AUTHEN_PROC = "PR_GS_Passport_Login";
#endif

CSQLDBManager* pG_SQLDBManager;

/************************Start Of CSingleSQLWorker******************************************/
CSingleSQLWorker::CSingleSQLWorker(CSQLWorkerUnit* owner, unsigned short usIdx, const std::string &sConnectStr) : m_pOwner(owner), m_usThreadIdx(usIdx), m_sConnectStr(sConnectStr)
{}

CSingleSQLWorker::~CSingleSQLWorker()
{
	WaitThreadExecuteOver();
}

void CSingleSQLWorker::DoExecute()
{

}

void CSingleSQLWorker::SetWorkType(TSQLWorkDB worktype)
{
	m_WorkType = worktype;
}

std::string CSingleSQLWorker::_EscapeString(const std::string &str)
{
	return m_pMySQLProc->EscapeString(const_cast<char*>(str.c_str()), str.length());
}

void CSingleSQLWorker::MySQLAuthenRes(Json::Value js, PJsonJobNode pNode, IMySQLFields* pDataSet, TAccountFlagInfo AccountFlag)
{
	try
	{
		bool bIsChild = false;
		PMySQLField pMyField = nullptr;
		//账号
		pMyField = pDataSet->FieldByName("Account");
		if (pMyField != nullptr)
			js["UniqueID"] = pMyField->AsString();
		pMyField = pDataSet->FieldByName("CreateTime");
		//创建时间
		if (pMyField != nullptr)
			js["CreateTime"] = pMyField->AsDateTime();
		//上次登陆ip
		pMyField = pDataSet->FieldByName("Usr_LastIP");
		if (pMyField != nullptr)
			js["LastLoginIP"] = pMyField->AsString();
		//上次登陆时间
		pMyField = pDataSet->FieldByName("Usr_LastTime");
		if (pMyField != nullptr)
			js["LastLoginTime"] = pMyField->AsDateTime();
		//帐号等级
		pMyField = pDataSet->FieldByName("Usr_Level");
		if (pMyField != nullptr)
			js["AccountLevel"] = pMyField->AsString();
		//安全等级
		pMyField = pDataSet->FieldByName("Usr_SecurityLevel");
		if (pMyField != nullptr)
			js["SecurityLevel"] = pMyField->AsString();
		//通过防沉迷
		pMyField = pDataSet->FieldByName("Usr_IsSecurityGame");
		if (pMyField != nullptr)
			js["IsSecurityGame"] = pMyField->AsString();
		//通过邮件认证
		pMyField = pDataSet->FieldByName("Usr_IsSecurityEmail");
		if (pMyField != nullptr)
			js["IsSecurityEmail"] = pMyField->AsString();
		//通过手机认证
		pMyField = pDataSet->FieldByName("Usr_IsSecurityMobile");
		if (pMyField != nullptr)
			js["IsSecurityMobile"] = pMyField->AsString();
		//通行证激活等级
		pMyField = pDataSet->FieldByName("c_KeyLevel");
		if (pMyField != nullptr)
			js["ActivityFlag"] = pMyField->AsString();
		//启用密保卡
		pMyField = pDataSet->FieldByName("Usr_IsSecurityCard");
		if (pMyField != nullptr)
			js["SafeCard"] = pMyField->AsString();
		//支付密码认证 0: 不开启 1: 开启
		pMyField = pDataSet->FieldByName("Usr_IsSecurityPay");
		if (pMyField != nullptr)
			js["NeedPayPwd"] = pMyField->AsString();
		//支付密码 格式: md5(密码小写)
		pMyField = pDataSet->FieldByName("Usr_PaySecurityKey");
		if (pMyField != nullptr)
			js["PayPwd"] = pMyField->AsString();
		//名人认证级别 0: 未认证
		pMyField = pDataSet->FieldByName("Usr_Certification");
		if (pMyField != nullptr)
			js["Certification"] = pMyField->AsString();

		//密保卡号
		pMyField = pDataSet->FieldByName("Usr_Certification");
		if (pMyField != nullptr)
			js["Certification"] = pMyField->AsString();
		//用户标示
		pMyField = pDataSet->FieldByName("Usr_Flag");
		if (pMyField != nullptr)
		{
			/*
			Flag := myField.AsInteger;
			if (Flag and $1) <> 0 then
			Include(AccountFlag.FlagSet, accNeedModifyPwd);
			if (Flag and $2) <> 0 then
			Include(AccountFlag.FlagSet, accNoSafeAccount);
			*/
		}
		//身份证号
		pMyField = pDataSet->FieldByName("Usr_CardId");
		if (pMyField != nullptr)
		{
			/*
			CardId := myField.AsString;
			Add('CardID', CardId);                              // 身份证
			isChild := G_ChildManager.IsChild(CardID, isDeny);
			if isChild then
			begin
			{$IFDEF WALLOW_ALLOW}
			if isDeny then
			{$ENDIF}
			begin
			Add('Result', 10);
			Add('Message', MSG_DENY_CHILD);                 // 'Child Login is Deny!'
			end;
			end;
			*/
		}
		else
			Log("身份证号不存在！", lmtError);

		if (bIsChild)
			js["IsChild"] = "1";
		else
			js["IsChild"] = "0";
		js["AccountFlag"] = AccountFlag.iFlag;

		if (pG_SQLDBManager != nullptr)
		{
			Json::FastWriter writer;
			std::string sTemp = writer.write(js);
			pG_SQLDBManager->AddWorkJob(SM_USER_AUTHEN_LOG, pNode->iHandle, pNode->iParam, sTemp);
		}
	}
	catch (...)
	{
		Log("MySQLAuthenRes: ");
	}
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
		std::string sPwd = ""; //--------------------------------- MD5Print(MD5String(LowerCase(GetStringValue(Field['Pwd'])) + G_PWD_MD5_KEY), True);
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
			if (2 == iRetCode)
			{
				//Include(AccountFlag.FlagSet, accNeedModifyPwd);
				iRetCode = 1;
			}
			else if (3 == iRetCode)
			{
				//Include(AccountFlag.FlagSet, accNoSafeAccount);
				iRetCode = 1;
			}
			MySQLAuthenRes(root, pNode, pDataSet, info);
			break;
		case -1:
			root["Message"] = MSG_AUTHEN_ERROR;
			break;
		case -6:
			root["Message"] = MSG_ACCOUNT_DENY;
			break;
		case -2:
		case -3:
		case -4:
		case -5:
		case -9:
			root["Message"] = MSG_AUTHEN_ERROR;
			break;
		case -10:
			root["Message"] = MSG_PWD_EASY;
			break;
		default:
			root["Message"] = MSG_AUTHEN_ERROR;
			break;
		}
		if (iRetCode != 1)
		{
			//---------------------------------
			//---------------------------------
			//---------------------------------
			//G_AuthFailLog.WriteLog(Format('%s,%s,%s,%s,%s,%s,%d'#13#10, [FormatDateTime('yyyy-mm-dd hh:nn:ss', Now()), AreaID, Account, IP, Mac, Pwd, nResult]));
			iRetCode = -9;
		}
		root["Result"] = iRetCode;
		/*
		  if Assigned(G_ServerSocket) then
		  begin
			Field['Pwd'].Value := '';
			with nNode^ do
			  G_ServerSocket.SQLJobResponse(Cmd, Handle, nParam, nRes, TlkJSON.GenerateText(js));
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
}

bool CSingleSQLWorker::SQLDB_Regist(PJsonJobNode pNode)
{
//暂时不增加该功能
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



/************************Start Of CSQLWorkerUnit******************************************/

CSQLWorkerUnit::CSQLWorkerUnit(const std::string &s, TSQLWorkDB dbtype) : m_pFirst(nullptr), m_pLast(nullptr), m_iCount(0)
{
	for (int i = 0; i < MAX_SQL_WORK_COUNT; i++)
	{
		m_pWorkThreads[i] = new CSingleSQLWorker(this, i, s);
		m_pWorkThreads[i]->SetWorkType(dbtype);
	}
}

CSQLWorkerUnit::~CSQLWorkerUnit()
{
	for (int i = 0; i < MAX_SQL_WORK_COUNT; i++)
		delete m_pWorkThreads[i];

	Clear();
}

bool CSQLWorkerUnit::AddWorkJob(int iCmd, int iHandle, int iParam, const std::string &s)
{

}

PJsonJobNode CSQLWorkerUnit::PopWorkJob()
{

}

void CSQLWorkerUnit::Clear()
{
	/*
var
  nNode             : PJSONJobNode;
begin
  EnterCriticalSection(FCS);
  try
    while Assigned(FFirst) do
    begin
      nNode := FFirst;
      FFirst := nNode^.Next;
      DisPose(nNode);
    end;
    FFirst := nil;
    FLast := nil;
    FCount := 0;
  finally
    LeaveCriticalSection(FCS);
  end;
end;
	*/
}

/************************End Of CSQLWorkerUnit********************************************/