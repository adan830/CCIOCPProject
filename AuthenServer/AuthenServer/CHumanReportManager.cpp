/**************************************************************************************
@author: 陈昌
@content: 人数上报管理器
**************************************************************************************/
#include "stdafx.h"
#include "CHumanReportManager.h"
#include "CDBServerSocket.h"

const std::string CREATE_HUMCOUNT_TABLE = "Create Table if not exists HumanCount(" \
"Idx bigint(20) unsigned PRIMARY KEY AUTO_INCREMENT," \
"ServerID smallint(5) unsigned default 0," \
"HumanCount smallint(5) unsigned default 0," \
"ReportDate DateTime default \"2011 - 01 - 01\"," \
"Index Idx_SrvID_Date(Idx, ServerID, ReportDate)," \
"Index SrvID_Date(ServerID, ReportDate));";

#ifdef TEST
	const std::string REPORT_DB_HOST = "localhost";
#else
	const std::string REPORT_DB_HOST = "192.168.111.111";
#endif

CHumanReportManager* pG_HumanReportManager;

/************************Start Of CHumanReportManager******************************************/

CHumanReportManager::CHumanReportManager() :m_uiLastRunTick(0), m_usRunInterval(30), m_bInitial(false)
{
	m_pMySQLProc = new CMySQLManager();
	m_pMySQLProc->m_OnError = std::bind(&CHumanReportManager::OnMySQLError, this, std::placeholders::_1, std::placeholders::_2);
	m_uiLastRunTick = GetTickCount() - 3000;
	LoadConfig();
}

CHumanReportManager::~CHumanReportManager()
{
	delete m_pMySQLProc;
}

void CHumanReportManager::Initialize()
{
	m_pMySQLProc->Connect(m_sDBHost, m_sDBName, m_sDBUserName, m_sUserPwd);
	if (m_pMySQLProc->IsMySQLConnected())
	{
		try
		{
			Log("Reoprt 数据库连接成功！");
			int iAffected = 0;
			IMySQLFields* pDataSet = nullptr;
			if (m_pMySQLProc->Exec(CREATE_HUMCOUNT_TABLE, pDataSet, iAffected))
				m_bInitial = true;
			else
				Log("Report 数据库创建表失败！", lmtError);
		}
		catch (...)
		{
		}
	}
	else
		Log("Report 数据库初始化失败！", lmtError);
}

void CHumanReportManager::LoadConfig()
{
	std::string sConfigFileName(G_CurrentExeDir + "config.ini");
	CWgtIniFile* pIniFileParser = new CWgtIniFile();
	pIniFileParser->loadFromFile(sConfigFileName);
	m_sDBHost = pIniFileParser->getString("Report", "Host", REPORT_DB_HOST);
	m_sDBName = pIniFileParser->getString("Report", "DBName", "Report");
	m_sDBUserName = pIniFileParser->getString("Report", "DBUser", DB_USERNAME);
	m_sUserPwd = DecodeString(pIniFileParser->getString("Report", "Password", DB_PASSWORD));
	m_usRunInterval = pIniFileParser->getInteger("Report", "SaveTick", 30);
	delete pIniFileParser;
}

void CHumanReportManager::SaveHumanCount()
{
	std::string sSql;
	int iAffected = 0;
	IMySQLFields* pDataSet = nullptr;
	std::list<void*>::iterator vIter;
	CDBConnector* pDBClient;
	for (vIter = pG_DBSocket->m_ActiveConnects.begin(); vIter != pG_DBSocket->m_ActiveConnects.end(); ++vIter)
	{
		pDBClient = (CDBConnector*)*vIter;
		if (pDBClient != nullptr)
		{
			sSql = "Insert into Humancount(ServerID, HumanCount, ReportDate) values(" + std::to_string(pDBClient->GetServerID()) + ", "
				+ std::to_string(pDBClient->GetHumanCount()) + ", Now());";
			m_pMySQLProc->Exec(sSql, pDataSet, iAffected);
		}
	}
}

void CHumanReportManager::Run(unsigned int uiTick)
{
	if (uiTick - m_uiLastRunTick >= m_usRunInterval * 1000)
	{
		m_uiLastRunTick = uiTick;
		if (!m_bInitial)
		{
			Initialize();
			return;
		}
		if (m_pMySQLProc->IsMySQLConnected())
			SaveHumanCount();
		else
			m_pMySQLProc->Open();
	}
}

void CHumanReportManager::OnMySQLError(unsigned int uiErrorCode, const std::string &sErrMsg)
{
	if (0 == uiErrorCode)
		Log(sErrMsg, lmtMessage);
	else
		Log(sErrMsg, lmtError);
}

/************************End Of CHumanReportManager******************************************/

