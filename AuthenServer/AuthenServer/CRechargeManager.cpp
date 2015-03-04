/**************************************************************************************
@author: 陈昌
@content: 充值付费管理器
**************************************************************************************/
#include "stdafx.h"
#include "CRechargeManager.h"

const std::string Area_Recharge_Proc = "P_GS_Order_ChargeQueue";
const std::string Single_Recharge_Proc = "P_GS_Order_Charge";
const std::string Recharge_CallBack_Proc = "P_GS_Order_ChargeCallBack";
const int MAX_CACHE_JOB = 600;

/************************Start Of CSQLWorkThread******************************************/
CSQLWorkThread::CSQLWorkThread(void* owner, const std::string &sConnectStr) : m_Owner(owner), m_sConnectStr(sConnectStr), m_bEnabled(false), m_pMySQLProc(nullptr)
{}

CSQLWorkThread::~CSQLWorkThread()
{
	WaitThreadExecuteOver();
}

bool CSQLWorkThread::IsEnabled()
{
	return m_bEnabled;
}

void CSQLWorkThread::DoExecute()
{
	m_pMySQLProc = new CMySQLManager();
	m_pMySQLProc->SetConnectString(m_sConnectStr);
	m_pMySQLProc->m_OnError = std::bind(&CSQLWorkThread::OnMySQLError, this, std::placeholders::_1, std::placeholders::_2);
	PJsonJobNode pWorkNode = nullptr;
	PJsonJobNode pTempNode = nullptr;
	try
	{
		while (!m_pMySQLProc->IsConnected())
		{
			if (!IsTerminated())
				return;
			if (m_pMySQLProc->Open())
				break;
			WaitForSingleObject(m_Event, 5000);
		}
		CheckProcExists(nullptr);
		if (!m_bEnabled)
			Terminate();

		int iErrorCount = 0;
		while (!IsTerminated())
		{
			try
			{
				if (nullptr == pWorkNode)
				{
					pWorkNode = ((CRechargeManager*)m_Owner)->PopRechargeJob();
					iErrorCount = 0;
				}
				if (pWorkNode != nullptr)
				{
					bool bSuccess = false;
					switch (pWorkNode->iCmd)
					{
					case SM_RECHARGE_AREA_QUERY:
						//定时查询本区的充值
						bSuccess = QueryAreaRecharge(pWorkNode);
						break;
					case SM_RECHARGE_DB_ACK:
						bSuccess = DBRechargeAck(pWorkNode);
						break;
					default:
						Log("[充值接口]:未知协议：" + std::to_string(pWorkNode->iCmd), lmtWarning);
						bSuccess = true;
						break;
					}

					if (!bSuccess)
					{
						m_pMySQLProc->Close();
						WaitForSingleObject(m_Event, 1000);
						m_pMySQLProc->Open();
						++iErrorCount;
						if (iErrorCount >= 10)
						{
							OnMySQLError(2, "[充值接口]:多次SQL执行错误！");
							bSuccess = true;
						}
					}
					if (bSuccess)
					{
						pTempNode = pWorkNode;
						pWorkNode = nullptr;
						delete pTempNode;
					}				
				}
			}
			catch (...)
			{
				Log("[充值接口]:异常:", lmtException);
			}
			WaitForSingleObject(m_Event, 10);
		}
		if (pWorkNode != nullptr)
			delete pWorkNode;
		m_pMySQLProc->Close();
		delete m_pMySQLProc;
	}
	catch (...)
	{
		m_pMySQLProc->Close();
		delete m_pMySQLProc;
	}
}

void CSQLWorkThread::OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg)
{
	if (0 == uiErrorCode)
		Log(sErrMsg, lmtMessage);
	else
		Log(sErrMsg, lmtError);
}

void CSQLWorkThread::CheckProcExists(void* Sender)
{
	m_bEnabled = false;
	std::string sSql("SHOW PROCEDURE STATUS LIKE \"" + Area_Recharge_Proc + "\"");
	int iAffected;
	IMySQLFields* pDataSet;
	if ((m_pMySQLProc->Exec(sSql, pDataSet, iAffected)) && (1 == iAffected))
	{
		sSql = "SHOW PROCEDURE STATUS LIKE \"" + Recharge_CallBack_Proc + "\"";
		m_bEnabled = (m_pMySQLProc->Exec(sSql, pDataSet, iAffected)) && (1 == iAffected);
	}
	if (!m_bEnabled)
		Log("充值接口不存在", lmtWarning);
}

std::string CSQLWorkThread::BuildJsonResult(IMySQLFields* pDataSet)
{
	std::string sTemp("");
	if (pDataSet != nullptr)
	{
		try
		{
			Json::Value root;
			root["OrderId"] = pDataSet->FieldByName("OrderId")->AsString();
			root["AppId"] = pDataSet->FieldByName("AddId")->AsInteger();
			root["ToAccount"] = pDataSet->FieldByName("ToAccount")->AsInt64();
			root["GameId"] = pDataSet->FieldByName("ServerId")->AsInteger();    //db使用的是ServerId对应游戏编号
			root["AreaId"] = pDataSet->FieldByName("AreaId")->AsInteger();
			root["GroupId"] = pDataSet->FieldByName("GroupId")->AsInteger();
			root["ChargeCurrency"] = pDataSet->FieldByName("ChargeCurrency")->AsInteger();
			root["ChargeAmount"] = pDataSet->FieldByName("ChargeAmount")->AsInteger();
			root["BindAmount"] = pDataSet->FieldByName("LockAmount")->AsInteger();
			root["ServerBatch"] = pDataSet->FieldByName("ServerBatch")->AsString();
			Json::FastWriter writer;
			sTemp = writer.write(root);
		}
		catch (...)
		{
			Log("BuildJsonResult: ");
		}
	}
	return sTemp;
}

bool CSQLWorkThread::QueryAreaRecharge(PJsonJobNode pNode)
{
	bool retFlag = true;
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(pNode->sJsonText, root))
	{
		int iAffected;
		IMySQLFields* pDataSet;
		std::string sSql = "call " + Area_Recharge_Proc + "(" + root.get("GameId", "0").asString() + ", " + root.get("AreaId", "0").asString() + ");";
		if (m_pMySQLProc->Exec(sSql, pDataSet, iAffected))
		{
			if ((iAffected > 0) && (pDataSet != nullptr))
			{
				pDataSet->First();
				while (!pDataSet->Eof())
				{
					/****************************
					/****************************
					/****************************
					?????????????????????????????
					with nNode^ do
						G_ServerSocket.SQLJobResponse(SM_RECHARGE_DB_REQ, Handle, nParam, 0, BuildJsonResult(DataSet));
					****************************/
					pDataSet->Next();
				}
			}
		}
	}
	return retFlag;
}

bool CSQLWorkThread::DBRechargeAck(PJsonJobNode pNode)
{
	bool retFlag = false;
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(pNode->sJsonText, root))
	{
		int iAffected;
		IMySQLFields* pDataSet;
		std::string sSql = "call " + Recharge_CallBack_Proc + "(\"" + root.get("OrderId", "").asString() + "\", \"" + root.get("IP", "").asString() + "\", " + root.get("State", "0").asString() + ");";
		retFlag = m_pMySQLProc->Exec(sSql, pDataSet, iAffected);
	}
	return retFlag;
}
/************************End Of CSQLWorkThread********************************************/


/************************Start Of CRechargeManager******************************************/
CRechargeManager::CRechargeManager() : m_pFirst(nullptr), m_pLast(nullptr), m_iCount(0)
{
	std::string sTemp = LoadSQLConfig();
	m_pWorkThread = new CSQLWorkThread(this, sTemp);
}

CRechargeManager::~CRechargeManager()
{
	Clear();
	if (m_pWorkThread != nullptr)
		delete m_pWorkThread;
}

bool CRechargeManager::AddRechargeJob(int iCmd, int iHandle, int iParam, const std::string &sTxt, const bool bForce)
{
	bool retFlag = m_iCount < MAX_CACHE_JOB;
	if (retFlag || bForce)
	{
		PJsonJobNode pNode = new TJsonJobNode();
		pNode->iCmd = iCmd;
		pNode->iHandle = iHandle;
		pNode->iParam = iParam;
		pNode->iRes = 0;
		pNode->sJsonText = sTxt;
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

PJsonJobNode CRechargeManager::PopRechargeJob()
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

bool CRechargeManager::IsEnable()
{
	return m_pWorkThread->IsEnabled();
}

std::string CRechargeManager::LoadSQLConfig()
{
	/*
var
  FileName          : ansistring;
  iniFile           : TIniFile;
begin
  FileName := ExtractFilePath(ParamStr(0)) + CONFIG_FILENAME;
  iniFile := TIniFile.Create(FileName);
  Result := Format(RECHARGE_CONNECTSTRING, [
    IniFile.ReadString('RechargeDB', 'Host', GAME_PAY_DB_HOST),
      IniFile.ReadString('RechargeDB', 'User', GAME_PAY_DB_USER),
      DeCodeString(IniFile.ReadString('RechargeDB', 'Password', GAME_PAY_DB_PWD))
      ]
      );
  iniFile.Free;
end;
	*/
}

void CRechargeManager::Clear()
{
	std::lock_guard<std::mutex> guard(m_LockCS);
	PJsonJobNode pWorkNode = nullptr;
	while (m_pFirst != nullptr)
	{
		pWorkNode = m_pFirst;
		m_pFirst = pWorkNode->pNext;
		delete pWorkNode;
	}
	m_pFirst = nullptr;
	m_pLast = nullptr;
	m_iCount = 0;
}
/************************End Of CRechargeManager******************************************/

