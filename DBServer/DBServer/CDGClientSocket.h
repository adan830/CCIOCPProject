/**************************************************************************************
@author: �²�
@content: DBServer��Ϊ�ͻ��˷�����DispatchGate�������Ķ˿�
**************************************************************************************/
#ifndef __CC_DISPATCHGATE_CLIENT_SOCKET_H__
#define __CC_DISPATCHGATE_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer��Ϊ�ͻ��˷�����DispatchGate�������Ķ˿�
*
*/

const int MAX_DISPATCHGATE_NUM = 3;

class CDGClientSocket : public CIOCPClientSocketManager
{
public:
	CDGClientSocket();
	virtual ~CDGClientSocket();
	void DoHeartbeat();
	void LoadConfig(CWgtIniFile* pIniFileParser);
	bool Closed();
	bool SendToServerPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen);
	void ProcDispatchMessage(PInnerMsgNode pNode);
	bool IsTraceRole(const std::string &sRoleName);
	bool GetSession(int iSessionID, PSessionInfo pResult);
	bool ProcGMCmd(int iSessionID, const std::string &sParam1, const std::string &sParam2, const std::string &sParam3);
	bool IsDenyAll(){ return m_bDenyAll; }
protected:
	virtual void ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen);
private:
	void OnSocketConnect(void* Sender);
	void OnSocketDisconnect(void* Sender);
	void OnSocketRead(void* Sender, const char* pBuf, int iCount);
	void OnSocketError(void* Sender, int& iErrorCode);
	void Reconnect();
	void SendHeartbeat();
	void OnRemoveSession(void* pValue, int iKey);
	void LoadIpConfigFile();
	void SendConfig(const std::string &sKey, std::string &sValue);
	bool SetConfig(const std::string &sKey, std::string &sValue, bool bDel);
	std::string GetConfigInfo(const std::string &sKey);
	void SendRegisterServer();
	void MsgSelectServer(int iParam, char* pBuf, unsigned short usBufLen);
private:
	unsigned int m_uiCheckTick;								// �������ʱ��
    int m_iPingCount;
	bool m_bDenyAll;
	int m_iConfigFileAge;
	std::mutex m_SessionCS;
	CC_UTILS::CIntegerHash m_SessionHash;					// �Ự�б�
	std::vector<std::string> m_TraceList;
	int m_iWorkIndex;										// ��ǰʹ�õ�Dispatch˳���
	TServerAddress m_ServerArray[MAX_DISPATCHGATE_NUM];     // ֧�ֶ�Dispatch Gate
};

extern CDGClientSocket* pG_DispatchGateSocket;

#endif //__CC_DISPATCHGATE_CLIENT_SOCKET_H__