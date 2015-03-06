/**************************************************************************************
@author: 陈昌
@content: 数据库操作管理器，多线程处理
**************************************************************************************/
#include "stdafx.h"
#include "CSQLDBManager.h"

const std::string SQL_LOGINLOG_PROC = "PW_GS_Passport_CreateLoginLog";
const std::string SQL_SAFECARD_AUTHEN_PROC = "P_GS_Usr_Securitycard_Verify";

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