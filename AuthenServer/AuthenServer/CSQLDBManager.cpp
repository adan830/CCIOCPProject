/**************************************************************************************
@author: 陈昌
@content: 数据库操作管理器，多线程处理
**************************************************************************************/
#include "stdafx.h"
#include "CSQLDBManager.h"
#include "CAuthFailLog.h"
#include "CDBServerSocket.h"
#include "CProtectChildManager.h"

const std::string SQL_LOGINLOG_PROC = "PW_GS_Passport_CreateLoginLog";
const std::string SQL_SAFECARD_AUTHEN_PROC = "P_GS_Usr_Securitycard_Verify";
const std::string SQL_REGIST_PROC = "***";

const int MAX_THREAD_COUNT = 3;
const int MAX_POOL_COUNT = 3000;

#ifdef TEST
const std::string SQL_AUTHEN_PROC = "P_GS_ForTest";
#else
const std::string SQL_AUTHEN_PROC = "PR_GS_Passport_Login";
#endif

#ifdef TEST
	const std::string DB_MAIN_USER = DB_USERNAME;
	const std::string DB_MAIN_PWD = DB_PASSWORD;
	#ifdef LOCAL_PASSPORT
		const std::string DB_MAIN_HOST = DB_HOSTNAME;
	#else	
		const std::string DB_MAIN_HOST = "192.168.1.2";
	#endif
#else
	const std::string DB_MAIN_HOST = "192.168.123.8";
	const std::string DB_MAIN_USER = "writeuser";
	const std::string DB_MAIN_PWD = "YsPmH_HpIOUzTRA]";
#endif

CSQLDBManager* pG_SQLDBManager;
std::string G_PWD_MD5_KEY;

/************************Start Of CSingleSQLWorker******************************************/
CSingleSQLWorker::CSingleSQLWorker(CSQLWorkerUnit* owner, unsigned short usIdx, const std::string &sConnectStr) : m_pOwner(owner), m_usThreadIdx(usIdx), m_sConnectStr(sConnectStr)
{}

CSingleSQLWorker::~CSingleSQLWorker()
{
	WaitThreadExecuteOver();
}

void CSingleSQLWorker::DoExecute()
{
	m_pMySQLProc = new CC_UTILS::CMySQLManager();
	try
	{
		m_pMySQLProc->SetConnectString(m_sConnectStr);
		m_pMySQLProc->m_OnError = std::bind(&CSingleSQLWorker::OnMySQLError, this, std::placeholders::_1, std::placeholders::_2);
		PJsonJobNode pWorkNode = nullptr;
		int iErrorCount = 0;
		while (!IsTerminated())
		{
			try
			{
				if (nullptr == pWorkNode)
				{
					iErrorCount = 0;
					pWorkNode = m_pOwner->PopWorkJob();
				}

				if (pWorkNode != nullptr)
				{
					if (!m_pMySQLProc->IsMySQLConnected())
					{
						m_pMySQLProc->Open();
						WaitForSingleObject(m_Event, 1000);
					}

					bool bSuccess = false;
					switch (pWorkNode->iCmd)
					{
					case SM_USER_AUTHEN_REQ:
						bSuccess = SQLDB_Authen(pWorkNode);
						break;
					case SM_USER_REGIST_REQ:
						bSuccess = SQLDB_Regist(pWorkNode);
						break;
					case SM_USER_AUTHEN_LOG:
						bSuccess = SQLDB_AuthenLog(pWorkNode);
						break;
					case SM_SAFECARD_AUTHEN_REQ:
						bSuccess = SQLDB_SafeCardAuthen(pWorkNode);
						break;
					default:
						Log("[TSQLWorker未知协议]：" + std::to_string(pWorkNode->iCmd), lmtWarning);
						bSuccess = true;
						break;
					}

					if (!bSuccess)
					{
						m_pMySQLProc->Open();
						WaitForSingleObject(m_Event, 1000);
						++iErrorCount;
						if (iErrorCount >= 10)
							OnMySQLError(3, "[认证接口]:多次SQL执行错误！");
					}
					PJsonJobNode pTempNode = pWorkNode;
					pWorkNode->sJsonText = "";
					pWorkNode = nullptr;
					delete pWorkNode;
				}
				else
					WaitForSingleObject(m_Event, 10);
			}
			catch (...)
			{
				Log("TSQLWorker.Execute,ErrCode=" + std::to_string(GetLastError()) + " [ErrorMsg]", lmtException);
				WaitForSingleObject(m_Event, 10);
			}
		}

		if (pWorkNode != nullptr)
		{
			pWorkNode->sJsonText = "";
			delete pWorkNode;
		}
		delete m_pMySQLProc;
	}
	catch (...)
	{
		delete m_pMySQLProc;
	}	
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

		Json::FastWriter writerTest;
		std::string sTempTest;
		sTempTest = writerTest.write(js);

		//账号
		pMyField = pDataSet->FieldByName("Account");
		if (pMyField != nullptr)
			js["UniqueID"] = pMyField->AsString();

		sTempTest = writerTest.write(js);

		pMyField = pDataSet->FieldByName("CreateTime");
		//创建时间
		if (pMyField != nullptr)
			js["CreateTime"] = pMyField->AsDateTime();

		sTempTest = writerTest.write(js);

		//上次登陆ip
		pMyField = pDataSet->FieldByName("Usr_LastIP");
		if (pMyField != nullptr)
			js["LastLoginIP"] = pMyField->AsString();

		sTempTest = writerTest.write(js);

		//上次登陆时间
		pMyField = pDataSet->FieldByName("Usr_LastTime");
		if (pMyField != nullptr)
			js["LastLoginTime"] = pMyField->AsDateTime();

		sTempTest = writerTest.write(js);

		//帐号等级
		pMyField = pDataSet->FieldByName("Usr_Level");
		if (pMyField != nullptr)
			js["AccountLevel"] = pMyField->AsString();

		sTempTest = writerTest.write(js);

		//安全等级
		pMyField = pDataSet->FieldByName("Usr_SecurityLevel");
		if (pMyField != nullptr)
			js["SecurityLevel"] = pMyField->AsString();

		sTempTest = writerTest.write(js);

		//通过防沉迷
		pMyField = pDataSet->FieldByName("Usr_IsSecurityGame");
		if (pMyField != nullptr)
			js["IsSecurityGame"] = pMyField->AsString();

		sTempTest = writerTest.write(js);

		//通过邮件认证
		pMyField = pDataSet->FieldByName("Usr_IsSecurityEmail");
		if (pMyField != nullptr)
			js["IsSecurityEmail"] = pMyField->AsString();

		sTempTest = writerTest.write(js);

		//通过手机认证
		pMyField = pDataSet->FieldByName("Usr_IsSecurityMobile");
		if (pMyField != nullptr)
			js["IsSecurityMobile"] = pMyField->AsString();

		sTempTest = writerTest.write(js);

		//通行证激活等级
		pMyField = pDataSet->FieldByName("c_KeyLevel");
		if (pMyField != nullptr)
			js["ActivityFlag"] = pMyField->AsString();

		sTempTest = writerTest.write(js);

		//启用密保卡
		pMyField = pDataSet->FieldByName("Usr_IsSecurityCard");
		if (pMyField != nullptr)
			js["SafeCard"] = pMyField->AsString();

		sTempTest = writerTest.write(js);

		//支付密码认证 0: 不开启 1: 开启
		pMyField = pDataSet->FieldByName("Usr_IsSecurityPay");
		if (pMyField != nullptr)
			js["NeedPayPwd"] = pMyField->AsString();

		sTempTest = writerTest.write(js);

		//支付密码 格式: md5(密码小写)
		pMyField = pDataSet->FieldByName("Usr_PaySecurityKey");
		if (pMyField != nullptr)
			js["PayPwd"] = pMyField->AsString();

		sTempTest = writerTest.write(js);
		Log("test " + sTempTest, lmtError);

		//名人认证级别 0: 未认证
		pMyField = pDataSet->FieldByName("Usr_Certification");
		if (pMyField != nullptr)
			js["Certification"] = pMyField->AsString();

		//密保卡号
		pMyField = pDataSet->FieldByName("Usr_SecurityCardNo");
		if (pMyField != nullptr)
			js["Usr_SecurityCardNo"] = pMyField->AsString();
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
			std::string sCardId = pMyField->AsString();
			js["CardID"] = sCardId;
			bool bIsDeny = false;
			bIsChild = pG_ProtectChildManager->IsChild(sCardId, bIsDeny);
			if (bIsChild)
			{
#ifdef WALLOW_ALLOW
				if (bIsDeny)
#endif				
				{
					js["Result"] = 10;
					js["Message"] = MSG_DENY_CHILD;
				}			
			}
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
			Log("test End " + sTemp, lmtError);
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
		std::string sPwd = "ED781DB068B6309828465421E725CFDA"; //--------------------------------- MD5Print(MD5String(LowerCase(GetStringValue(Field['Pwd'])) + G_PWD_MD5_KEY), True);
		std::string sSql = "call " + SQL_AUTHEN_PROC + "(\"" + _EscapeString(sAccount) + "\", \"" + _EscapeString(sPwd) + "\", " + root.get("AppID", "").asString()
			+ ", " + sAreaID + ", 0, \"" + sIP + "\");";
		int iAffected = 0;
		IMySQLFields* pDataSet = nullptr;
		IMySQLFields** ppDataSet = &pDataSet;
		if (m_pMySQLProc->Exec(sSql, ppDataSet, iAffected))
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
		if (pG_DBSocket != nullptr)
		{
			//返回的时候将密码去掉
			root["Pwd"] = "";
			Json::FastWriter writer;
			std::string sRetStr = writer.write(root);
			pG_DBSocket->SQLJobResponse(pNode->iCmd, pNode->iHandle, pNode->iParam, pNode->iRes, sRetStr);
		}
#ifdef TEST
		if (iRetCode != 1)
		{
			Json::FastWriter writer;
			std::string sLogStr = writer.write(root);
			Log("Login Fail: " + std::to_string(iRetCode) + " " + sLogStr);
		}			
#endif
	}
	return retFlag;
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
	bool bRetFlag = false;
	return bRetFlag;
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
		IMySQLFields** ppDataSet = &pDataSet;
		retFlag = m_pMySQLProc->Exec(sLogSQL, ppDataSet, iAffected);
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
		IMySQLFields** ppDataSet = &pDataSet;
		if (m_pMySQLProc->Exec(sSql, ppDataSet, iAffected))
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
		Json::FastWriter writer;
		std::string sRetStr = writer.write(root);
		pG_DBSocket->SQLJobResponse(pNode->iCmd, pNode->iHandle, pNode->iParam, pNode->iRes, sRetStr);
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
		m_pWorkThreads[i]->InitialWorkThread();
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
	bool retFlag = m_iCount < MAX_POOL_COUNT;
	if (retFlag)
	{
		PJsonJobNode pNode = new TJsonJobNode();
		pNode->iCmd = iCmd;
		pNode->iHandle = iHandle;
		pNode->iParam = iParam;
		pNode->iRes = 0;
		pNode->sJsonText = s;
		pNode->pNext = nullptr;

		{
			std::lock_guard<std::mutex> guard(m_LockCS);
			if (m_pLast != nullptr)
				m_pLast->pNext = pNode;
			else
				m_pFirst = pNode;
			m_pLast = pNode;
			++m_iCount;
		}
	}
	return retFlag;
}

PJsonJobNode CSQLWorkerUnit::PopWorkJob()
{
	PJsonJobNode pRetNode = nullptr;
	std::lock_guard<std::mutex> guard(m_LockCS);
	if (m_pFirst != nullptr)
	{
		pRetNode = m_pFirst;
		m_pFirst = pRetNode->pNext;
		if (nullptr == m_pFirst)
			m_pLast = nullptr;
		--m_iCount;
	}
	return pRetNode;
}

void CSQLWorkerUnit::Clear()
{
	PJsonJobNode pNode = nullptr;
	std::lock_guard<std::mutex> guard(m_LockCS);
	while (m_pFirst != nullptr)
	{
		pNode = m_pFirst;
		m_pFirst = pNode->pNext;
		delete(pNode);
	}
	m_pFirst = nullptr;
	m_pLast = nullptr;
	m_iCount = 0;
}

/************************End Of CSQLWorkerUnit********************************************/




/************************Start Of CSQLDBManager******************************************/
CSQLDBManager::CSQLDBManager()
{
	m_pWorkUnits[0] = nullptr;
	m_pWorkUnits[1] = nullptr;
	LoadSQLConfig();

	std::string sPwdMd5Key = "X?ECJOPrM@UdHnH"; //---p2G956DFh3#---
	G_PWD_MD5_KEY = CC_UTILS::DecodeString(sPwdMd5Key);
}

CSQLDBManager::~CSQLDBManager()
{
	delete m_pWorkUnits[0];
	delete m_pWorkUnits[1];
	m_pWorkUnits[0] = nullptr;
	m_pWorkUnits[1] = nullptr;
}

bool CSQLDBManager::AddWorkJob(int iCmd, int iHandle, int iParam, const std::string &s)
{
	bool retFlag = false;
	CSQLWorkerUnit* pWorker = nullptr;
	if (nullptr == pG_DBSocket)
		return retFlag;

	Json::Reader reader;
	Json::Value root;
	std::string sAccount;
	switch (iCmd)
	{
	case SM_USER_AUTHEN_REQ:
	case SM_SAFECARD_AUTHEN_REQ:
		pWorker = m_pWorkUnits[swSlave];
		break;
	case SM_USER_REGIST_REQ:
	case SM_USER_AUTHEN_LOG: 
		pWorker = m_pWorkUnits[swMain];
		break;
	case SM_IN_CREDIT_NOW:
		pG_DBSocket->InCreditNow();
		retFlag = true;
		pWorker = nullptr;
		break;
    case SM_SENDITEM_NOW:
		pG_DBSocket->InSendItemNow();
		retFlag = true;
		pWorker = nullptr;
		break;
    case SM_KICKOUT_NOW:
		if (reader.parse(s, root))
		{
			sAccount = root.get("Account", "").asString();
			pG_DBSocket->BroadCastKickOutNow(sAccount, iParam);
			retFlag = true;
		}
		pWorker = nullptr;
		break;
	default:
		pWorker = m_pWorkUnits[swSlave];
		break;
	}
	if (pWorker != nullptr)
		retFlag = pWorker->AddWorkJob(iCmd, iHandle, iParam, s);
	return retFlag;
}

int CSQLDBManager::GetPoolCount()
{
	return m_pWorkUnits[0]->m_iCount + m_pWorkUnits[1]->m_iCount;
}

void CSQLDBManager::LoadSQLConfig()
{
	std::string sConfigFileName(G_CurrentExeDir + "config.ini");
	CWgtIniFile* pIniFileParser = new CWgtIniFile();
	pIniFileParser->loadFromFile(sConfigFileName);
	std::string sHost = pIniFileParser->getString("MainDB", "Host", DB_MAIN_HOST);
	std::string sUser = pIniFileParser->getString("MainDB", "User", DB_MAIN_USER);
	std::string sPwd = CC_UTILS::DecodeString(pIniFileParser->getString("MainDB", "Pwd", DB_MAIN_PWD));
	std::string sConnectStr = "Host=" + sHost + ";User=" + sUser + ";Pwd=" + sPwd + ";Database=LongGet_Passport_SVR;";
	m_pWorkUnits[swMain] = new CSQLWorkerUnit(sConnectStr, swMain);
	sHost = pIniFileParser->getString("SlaveDB", "Host", DB_MAIN_HOST);;
	sUser = pIniFileParser->getString("SlaveDB", "User", DB_MAIN_USER);;
	sPwd = CC_UTILS::DecodeString(pIniFileParser->getString("SlaveDB", "Pwd", DB_MAIN_PWD));
	sConnectStr = "Host=" + sHost + ";User=" + sUser + ";Pwd=" + sPwd + ";Database=LongGet_Passport_SVR;";
	m_pWorkUnits[swSlave] = new CSQLWorkerUnit(sConnectStr, swSlave);
	delete pIniFileParser;
}
/************************End Of CSQLDBManager********************************************/