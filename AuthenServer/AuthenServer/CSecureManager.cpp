/**************************************************************************************
@author: 陈昌
@content: 安全策略 用于 登录防暴力破解、防恶意注册、防安全卡穷举等
**************************************************************************************/
#include "stdafx.h"
#include "CSecureManager.h"
#include "windows.h"

using namespace CC_UTILS;

CSecureManager* pG_SecureManager;

const int MAX_REGISTER_COUNT = 50;                                // 每小时同IP最大注册角色数
const int MAX_LOGIN_FAILURE_COUNT = 5;                            // 每小时同帐号同IP最大登录错误数
const int MAX_MACLOGIN_FAILURE_COUNT = 15;                        // Mac地址限制

/************************Start Of CSecureManager******************************************/

CSecureManager::CSecureManager() :m_MaxRegisterCount(0), m_MaxLoginFailureCount(0), m_MaxMacLoginFailureCount(0), m_CurrentHour(0), 
	m_RegisterList(11117), m_LoginList(11117), m_SafeCardList(11117), m_MacLoginList(11117)
{
	m_RegisterList.m_RemoveEvent = std::bind(&CSecureManager::RemoveIntList, this, std::placeholders::_1, std::placeholders::_2);
	m_LoginList.m_RemoveEvent = std::bind(&CSecureManager::RemoveStrList, this, std::placeholders::_1, std::placeholders::_2);
	m_SafeCardList.m_RemoveEvent = std::bind(&CSecureManager::RemoveStrList, this, std::placeholders::_1, std::placeholders::_2);
	m_MacLoginList.m_RemoveEvent = std::bind(&CSecureManager::RemoveStrList, this, std::placeholders::_1, std::placeholders::_2);
	LoadConfig();
}

CSecureManager::~CSecureManager()
{
}

bool CSecureManager::LimitOfRegister(const std::string &sIP)
{
	bool bRetFlag = true;
	int iAddr = inet_addr(sIP.c_str());
	//异常地址
	if ((iAddr != u_long(INADDR_NONE)) && (iAddr != INADDR_ANY))
	{
		int* pInt = (int*)m_RegisterList.ValueOf(iAddr);
		if (nullptr == pInt)
		{
			pInt = new int;
			*pInt = 1;
			m_RegisterList.Add(iAddr, pInt);
		}
		else
		{
			++(*pInt);
		}
		bRetFlag = (*pInt > m_MaxRegisterCount);
	}
	return bRetFlag;
}

bool CSecureManager::LimitOfLogin(const std::string &sAuthenID, const std::string &sIP, const std::string &sMac)
{
	bool bRetFlag = true;
	std::string sKey(sAuthenID + '_' + sIP);
	int* pInt = (int*)m_LoginList.ValueOf(sKey);
	if (nullptr == pInt)
	{
		pInt = new int;
		*pInt = 1;
		m_LoginList.Add(sKey, pInt);
	}
	else
	{
		++(*pInt);
	}
	bRetFlag = (*pInt > m_MaxLoginFailureCount);

#ifndef TEST
	if ((!bRetFlag) && (sMac != ""))
	{
		sKey = sMac;
		pInt = (int*)m_MacLoginList.ValueOf(sKey);
		if (nullptr == pInt)
		{
			pInt = new int;
			*pInt = 1;
			m_MacLoginList.Add(sKey, pInt);
		}
		else
		{
			++(*pInt);
		}
		bRetFlag = (*pInt > m_MaxMacLoginFailureCount);
	}	
#else
	Log("[LimitOfLogin]: " + sKey + ':' + std::to_string(*pInt));	
#endif
	return bRetFlag;
}

void CSecureManager::ExecuteCancel(const std::string &sAuthenID, std::string &sIP, TSecureType type)
{
	std::string sTemp("");
	ExecuteCancel(sAuthenID, sIP, type, sTemp);
}

void CSecureManager::ExecuteCancel(const std::string &sAuthenID, std::string &sIP, TSecureType type, std::string &sMac)
{
	int* pInt = nullptr;
	std::string sKey;
	int iKey;
	switch (type)
	{		
	case scAuthen:
		sKey = sAuthenID + '_' + sIP;
		pInt = (int*)m_LoginList.ValueOf(sKey);
		if (pInt != nullptr)
		{
			if (*pInt > 0)
				--(*pInt);
		}
		if (sMac != "")
		{
			pInt = (int*)m_MacLoginList.ValueOf(sMac);
			if (pInt != nullptr)
			{
				if (*pInt > 0)
					--(*pInt);
				if (*pInt < 1)
					m_MacLoginList.Remove(sMac);
			}
		}
		return;
		break;
	case scRegister:
		iKey = inet_addr(sIP.c_str());
		if ((iKey == u_long(INADDR_NONE)) || (iKey == INADDR_ANY))
			return;
		pInt = (int*)m_RegisterList.ValueOf(iKey);
		break;
	case scSafeCard:
		sKey = sAuthenID + '_' + sIP;
		pInt = (int*)m_SafeCardList.ValueOf(sKey);
		break;
	default:
		break;
	}
	if (pInt != nullptr)
	{
		if (*pInt > 0)
			--(*pInt);
	}
}

void CSecureManager::LoginSuccess(const std::string &sAuthenID, const std::string &sIP, const std::string &sMac)
{
	std::string sKey(sAuthenID + '_' + sIP);
	m_LoginList.Remove(sKey);
	if (sMac != "")
	{
		int* pInt = (int*)m_MacLoginList.ValueOf(sMac);
		if (pInt != nullptr)
		{
			if (*pInt > 0)
				--(*pInt);
			if (*pInt < 1)
				m_MacLoginList.Remove(sMac);
		}
	}
}

void CSecureManager::RegisterFailed(const std::string &sIP)
{
	int iAddr = inet_addr(sIP.c_str());
	int* pInt = (int*)m_RegisterList.ValueOf(iAddr);
	if (pInt != nullptr)
	{
		--(*pInt);
		if (*pInt <= 0)
			m_RegisterList.Remove(iAddr);
	}
}

void CSecureManager::SafeCardSuccess(const std::string &sCardNo, const std::string &sIP)
{
	m_SafeCardList.Remove(sCardNo + '_' + sIP);
}

bool CSecureManager::LimitOfSafeCard(const std::string &sCardNo, const std::string &sIP)
{
	std::string sKey(sCardNo + '_' + sIP);
	int* pInt = (int*)m_SafeCardList.ValueOf(sKey);
	if (nullptr == pInt)
	{
		pInt = new int;
		*pInt = 1;
		m_SafeCardList.Add(sKey, pInt);
	}
	else
	{
		++(*pInt);
	}
	return (*pInt > m_MaxLoginFailureCount);
}

int CSecureManager::GetLoginFailCount()
{
	return m_LoginList.GetCount();
}

int CSecureManager::GetRegistFailCount()
{
	return m_RegisterList.GetCount();
}

int CSecureManager::GetMacFailCount()
{
	return m_MacLoginList.GetCount();
}

void CSecureManager::Execute()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	if (st.wHour != m_CurrentHour)
	{
		m_CurrentHour = st.wHour;
		m_LoginList.Clear();
		m_RegisterList.Clear();
		m_SafeCardList.Clear();
		m_MacLoginList.Clear();
	}
}

void CSecureManager::RemoveIntList(void* p, int iKey)
{
	delete ((int*)p);
}

void CSecureManager::RemoveStrList(void* p, const std::string& sKey)
{
	delete ((int*)p);
}

void CSecureManager::LoadConfig()
{
	std::string sConfigFileName(G_CurrentExeDir + "config.ini");
	CWgtIniFile* pIniFileParser = new CWgtIniFile();
	pIniFileParser->loadFromFile(sConfigFileName);
	m_MaxRegisterCount = pIniFileParser->getInteger("Secure", "RegisterPerHour", MAX_REGISTER_COUNT);
	m_MaxLoginFailureCount = pIniFileParser->getInteger("Secure", "LoginPerHour", MAX_LOGIN_FAILURE_COUNT);
	m_MaxMacLoginFailureCount = pIniFileParser->getInteger("Secure", "MacLoginPerHour", MAX_MACLOGIN_FAILURE_COUNT);
	delete pIniFileParser;
}
/************************End Of CSecureManager******************************************/

