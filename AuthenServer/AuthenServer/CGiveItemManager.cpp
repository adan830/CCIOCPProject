/**************************************************************************************
@author: 陈昌
@content: 道具推送管理器
**************************************************************************************/
#include "stdafx.h"
#include "CGiveItemManager.h"

const std::string Area_GiveItem_Proc = "P_GS_UsrItem_Queue";
const std::string GiveItem_CallBack_Proc = "P_GS_UsrItem_CallBack";
const int MAX_CACHE_JOB = 600;
const std::string GAME_GIVEITEM_DB_HOST = "192.168.1.2";
const std::string GAME_GIVEITEM_DB_USER = DB_USERNAME;
const std::string GAME_GIVEITEM_DB_PWD = DB_PASSWORD;
const std::string GAME_GIVEITEM_DB_NAME = "longget_gameitem_svr";

/************************Start Of CGiveItemSQLWorkThread******************************************/
CGiveItemSQLWorkThread::CGiveItemSQLWorkThread(void* owner) : m_Owner(owner), m_bEnabled(false), m_pMySQLProc(nullptr)
{}

CGiveItemSQLWorkThread::~CGiveItemSQLWorkThread()
{
	WaitThreadExecuteOver();
}

bool CGiveItemSQLWorkThread::IsEnabled()
{
	return m_bEnabled;
}

void CGiveItemSQLWorkThread::InitConnectString(const std::string &sHostName, const std::string &sDBName, const std::string &sUserName, 
	const std::string &sPassword, const std::string &sCharSet, const int iPort)
{
	m_pMySQLProc->m_sHostName = sHostName;
	m_pMySQLProc->m_sDBName = sDBName;
	m_pMySQLProc->m_sUser = sUserName;
	m_pMySQLProc->m_sPassword = sPassword;
	m_pMySQLProc->m_sCharSet = sCharSet;
	m_pMySQLProc->m_iPort = iPort;
}

void CGiveItemSQLWorkThread::DoExecute()
{
	m_pMySQLProc = new CMySQLManager();
	m_pMySQLProc->SetConnectString(m_sConnectStr);
	m_pMySQLProc->m_OnError = std::bind(&CGiveItemSQLWorkThread::OnMySQLError, this, std::placeholders::_1, std::placeholders::_2);
	PJsonJobNode pWorkNode = nullptr;
	PJsonJobNode pTempNode = nullptr;
	try
	{
		while (!m_pMySQLProc->IsMySQLConnected())
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
					pWorkNode = ((CGiveItemManager*)m_Owner)->PopGiveItemJob();
					iErrorCount = 0;
				}
				if (pWorkNode != nullptr)
				{
					bool bSuccess = false;
					switch (pWorkNode->iCmd)
					{
					case SM_GIVEITEM_QUERY:
						//定时查询本区的道具推送
						bSuccess = QueryAreaGiveItem(pWorkNode);
						break;
					case SM_GIVEITEM_DB_ACK:
						bSuccess = DBGiveItemAck(pWorkNode);
						break;
					default:
						Log("[推送道具]:未知协议：" + std::to_string(pWorkNode->iCmd), lmtWarning);
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
							OnMySQLError(2, "[推送道具]:多次SQL执行错误！");
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
				Log("[推送道具]:异常:", lmtException);
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

void CGiveItemSQLWorkThread::OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg)
{
	if (0 == uiErrorCode)
		Log(sErrMsg, lmtMessage);
	else
		Log(sErrMsg, lmtError);
}

void CGiveItemSQLWorkThread::CheckProcExists(void* Sender)
{
	m_bEnabled = false;
	std::string sSql("SHOW PROCEDURE STATUS LIKE \"" + Area_GiveItem_Proc + "\"");
	int iAffected = 0;
	IMySQLFields* pDataSet = nullptr;
	if ((m_pMySQLProc->Exec(sSql, pDataSet, iAffected)) && (1 == iAffected))
	{
		sSql = "SHOW PROCEDURE STATUS LIKE \"" + GiveItem_CallBack_Proc + "\"";
		m_bEnabled = (m_pMySQLProc->Exec(sSql, pDataSet, iAffected)) && (1 == iAffected);
	}
	if (!m_bEnabled)
		Log("推送道具接口不存在", lmtWarning);
}

std::string CGiveItemSQLWorkThread::BuildJsonResult(IMySQLFields* pDataSet)
{
	std::string sTemp("");
	if (pDataSet != nullptr)
	{
		try
		{
			Json::Value root;
			root["OrderId"] = pDataSet->FieldByName("transid")->AsString();
			root["ToAccount"] = pDataSet->FieldByName("ToAccount")->AsInt64();
			root["GameId"] = pDataSet->FieldByName("ServerId")->AsInteger();    
			root["AreaId"] = pDataSet->FieldByName("AreaId")->AsInteger();
			root["GroupId"] = pDataSet->FieldByName("GroupId")->AsInteger();
			root["ItemCount"] = pDataSet->FieldByName("ItemCount")->AsInteger();
			root["ItemName"] = pDataSet->FieldByName("ItemName")->AsString();
			root["Description"] = pDataSet->FieldByName("Description")->AsString();
			PMySQLField pTempField = pDataSet->FieldByName("ItemType");
			if (pTempField != nullptr)
				root["BindFlag"] = pDataSet->FieldByName("BindFlag")->AsInteger();

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

bool CGiveItemSQLWorkThread::QueryAreaGiveItem(PJsonJobNode pNode)
{
	bool retFlag = true;
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(pNode->sJsonText, root))
	{
		int iAffected = 0;
		IMySQLFields* pDataSet = nullptr;
		std::string sSql = "call " + Area_GiveItem_Proc + "(" + root.get("GameId", "0").asString() + ", " + root.get("AreaId", "0").asString() + ");";
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
					G_ServerSocket.SQLJobResponse(SM_GIVEITEM_DB_REQ, Handle, nParam, 0, BuildJsonResult(DataSet));
					****************************/
					pDataSet->Next();
				}
			}
		}
	}
	return retFlag;
}

bool CGiveItemSQLWorkThread::DBGiveItemAck(PJsonJobNode pNode)
{
	bool retFlag = false;
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(pNode->sJsonText, root))
	{
		int iAffected = 0;
		IMySQLFields* pDataSet = nullptr;
		std::string sSql = "call " + GiveItem_CallBack_Proc + "(\"" + root.get("OrderId", "").asString() + "\", \"" + root.get("IP", "").asString() + "\", " + root.get("State", "0").asString() + ");";
		retFlag = m_pMySQLProc->Exec(sSql, pDataSet, iAffected);
	}
	return retFlag;
}
/************************End Of CGiveItemSQLWorkThread********************************************/


/************************Start Of CGiveItemManager******************************************/
CGiveItemManager::CGiveItemManager() : m_pFirst(nullptr), m_pLast(nullptr), m_iCount(0)
{
	m_pWorkThread = new CGiveItemSQLWorkThread(this);
}

CGiveItemManager::~CGiveItemManager()
{
	Clear();
	if (m_pWorkThread != nullptr)
		delete m_pWorkThread;
}

bool CGiveItemManager::AddGiveItemJob(int iCmd, int iHandle, int iParam, const std::string &sTxt, const bool bForce)
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

PJsonJobNode CGiveItemManager::PopGiveItemJob()
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

bool CGiveItemManager::IsEnable()
{
	return m_pWorkThread->IsEnabled();
}

void CGiveItemManager::LoadSQLConfig()
{
	std::string sConfigFileName(G_CurrentExeDir + "config.ini");
	CWgtIniFile* pIniFileParser = new CWgtIniFile();
	pIniFileParser->loadFromFile(sConfigFileName);
	m_pWorkThread->InitConnectString(
		pIniFileParser->getString("GiveItemDB", "Host", GAME_GIVEITEM_DB_HOST),
		pIniFileParser->getString("GiveItemDB", "DBName", GAME_GIVEITEM_DB_NAME),
		pIniFileParser->getString("GiveItemDB", "User", GAME_GIVEITEM_DB_USER),
		CC_UTILS::DecodeString(pIniFileParser->getString("GiveItemDB", "Password", GAME_GIVEITEM_DB_PWD)),
		"gbk", 3306
		);
	delete pIniFileParser;
}

void CGiveItemManager::Clear()
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
/************************End Of CGiveItemManager******************************************/

