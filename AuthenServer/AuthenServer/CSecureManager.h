/**************************************************************************************
@author: 陈昌
@content: 安全策略 用于 登录防暴力破解、防恶意注册、防安全卡穷举等
**************************************************************************************/
#ifndef __CC_SECURE_MANAGER_H__
#define __CC_SECURE_MANAGER_H__

#include "stdafx.h"

enum TSecureType { scAuthen, scRegister, scSafeCard };

/**
*
* 安全策略管理器
*
*/
class CSecureManager
{
public:
	CSecureManager();
	virtual ~CSecureManager();

	bool LimitOfRegister(const std::string &sIP);		// 检测注册限制
	bool LimitOfLogin(const std::string &sAuthenID, const std::string &sIP, const std::string &sMac);    // 检测登录限制
	void ExecuteCancel(const std::string &sAuthenID, std::string &sIP, TSecureType type, std::string &sMac);
	void ExecuteCancel(const std::string &sAuthenID, std::string &sIP, TSecureType type);
	void LoginSuccess(const std::string &sAuthenID, const std::string &sIP, const std::string &sMac);    // 处理登录成功
	void RegisterFailed(const std::string &sIP);								 // 处理注册失败的情况
	void SafeCardSuccess(const std::string &sCardNo, const std::string &sIP);    // 密保卡认证成功
	bool LimitOfSafeCard(const std::string &sCardNo, const std::string &sIP);    // 检测密保卡认证限制	
	int GetLoginFailCount();    // 登录失败限制的帐号+IP个数
	int GetRegistFailCount();   // 注册个数限制的IP数
	int GetMacFailCount();      // 登录失败限制Mac个数
	void Execute();
private:
	void RemoveIntList(void* p, int iKey);
	void RemoveStrList(void* p, const std::string& sKey);
	void LoadConfig();
private:
	int m_MaxRegisterCount;                         // 同IP每小时的最大注册次数
	int m_MaxLoginFailureCount;                     // 同帐号同IP每小时的最大登录次数
	int m_MaxMacLoginFailureCount;                  // 同Mac每小时的最大登录次数
	int m_CurrentHour;                              // 当前的小时数
	CC_UTILS::CIntegerHash m_RegisterList;          // 注册列表
	CC_UTILS::CStringHash m_LoginList;              // 登录列表
	CC_UTILS::CStringHash m_SafeCardList;           // 密保卡认证列表
	CC_UTILS::CStringHash m_MacLoginList;           // 账号+Mac登录列表
};

extern CSecureManager* pG_SecureManager;

#endif //__CC_SECURE_MANAGER_H__