/**************************************************************************************
@author: 陈昌
@content: DBServer作为客户端方连接AuthenServer服务器的端口
**************************************************************************************/
#ifndef __CC_AUTHENSERVER_CLIENT_SOCKET_H__
#define __CC_AUTHENSERVER_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer作为客户端方连接AuthenServer服务器的端口
*
*/

const int MAX_AUTHENSERVER_NUM = 3;

class CASClientSocket : public CIOCPClientSocketManager
{
public:
	CASClientSocket();
	virtual ~CASClientSocket();
	void DoHeartbeat();
	void LoadConfig(CWgtIniFile* pIniFileParser);
	bool Closed();
	bool SendToServerPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen);
	void ProcAuthenServerMessage(PInnerMsgNode pNode);
	void OnYBCrushRsp(const std::string &sOrderID, const std::string &sRoleName, int iRetCode);
	void OnGiveItemRsp(const std::string &sOrderID, const std::string &sRoleName, int iRetCode);
protected:
	virtual void ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen);
private:
	void OnSocketConnect(void* Sender);
	void OnSocketDisconnect(void* Sender);
	void OnSocketRead(void* Sender, const char* pBuf, int iCount);
	void OnSocketError(void* Sender, int& iErrorCode);
	void Reconnect();  
	void SendHeartbeat();
	void SendRegisterServer();   
	void YBRechargeResponse(int iParam, char* pBuf, unsigned short usBufLen);
	void GiveItemResponse(char* pBuf, unsigned short usBufLen);
	void GameActCodeResponse(char* pBuf, unsigned short usBufLen);
	void ClearEncodeBuf();
	void ClearDecodeBuf();
private:
	unsigned int m_uiCheckTick;
	int m_iPingCount;
	PCodingFunc m_EnCodeFunc;
	PCodingFunc m_DeCodeFunc;
	char* m_pEnBuf;
	char* m_pDeBuf;
	unsigned short m_usEnBufLen;
	unsigned short m_usDeBufLen;
	int m_iWorkIndex;
	TServerAddress m_ServerArray[MAX_AUTHENSERVER_NUM];   //支持多认证服务器
};

extern CASClientSocket* pG_AuthenServerSocket;

#endif //__CC_AUTHENSERVER_CLIENT_SOCKET_H__