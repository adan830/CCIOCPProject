/**************************************************************************************
@author: �²�
@content: ��ȫ���� ���� ��¼�������ƽ⡢������ע�ᡢ����ȫ����ٵ�
**************************************************************************************/
#ifndef __CC_SECURE_MANAGER_H__
#define __CC_SECURE_MANAGER_H__

#include "stdafx.h"

enum TSecureType { scAuthen, scRegister, scSafeCard };

/**
*
* ��ȫ���Թ�����
*
*/
class CSecureManager
{
public:
	CSecureManager();
	virtual ~CSecureManager();

	bool LimitOfRegister(const std::string &sIP);		// ���ע������
	bool LimitOfLogin(const std::string &sAuthenID, const std::string &sIP, const std::string &sMac);    // ����¼����
	void ExecuteCancel(const std::string &sAuthenID, std::string &sIP, TSecureType type, std::string &sMac);
	void ExecuteCancel(const std::string &sAuthenID, std::string &sIP, TSecureType type);
	void LoginSuccess(const std::string &sAuthenID, const std::string &sIP, const std::string &sMac);    // �����¼�ɹ�
	void RegisterFailed(const std::string &sIP);								 // ����ע��ʧ�ܵ����
	void SafeCardSuccess(const std::string &sCardNo, const std::string &sIP);    // �ܱ�����֤�ɹ�
	bool LimitOfSafeCard(const std::string &sCardNo, const std::string &sIP);    // ����ܱ�����֤����	
	int GetLoginFailCount();    // ��¼ʧ�����Ƶ��ʺ�+IP����
	int GetRegistFailCount();   // ע��������Ƶ�IP��
	int GetMacFailCount();      // ��¼ʧ������Mac����
	void Execute();
private:
	void RemoveIntList(void* p, int iKey);
	void RemoveStrList(void* p, const std::string& sKey);
	void LoadConfig();
private:
	int m_MaxRegisterCount;                         // ͬIPÿСʱ�����ע�����
	int m_MaxLoginFailureCount;                     // ͬ�ʺ�ͬIPÿСʱ������¼����
	int m_MaxMacLoginFailureCount;                  // ͬMacÿСʱ������¼����
	int m_CurrentHour;                              // ��ǰ��Сʱ��
	CC_UTILS::CIntegerHash m_RegisterList;          // ע���б�
	CC_UTILS::CStringHash m_LoginList;              // ��¼�б�
	CC_UTILS::CStringHash m_SafeCardList;           // �ܱ�����֤�б�
	CC_UTILS::CStringHash m_MacLoginList;           // �˺�+Mac��¼�б�
};

extern CSecureManager* pG_SecureManager;

#endif //__CC_SECURE_MANAGER_H__