/**************************************************************************************
@author: 陈昌
@content: DBServer作为客户端方连接DispatchGate服务器的端口
**************************************************************************************/
#include "stdafx.h"
#include "CDGClientSocket.h"

using namespace CC_UTILS;

CDGClientSocket* pG_DispatchGate;

/************************Start Of CDGClientSocket******************************************/

CDGClientSocket::CDGClientSocket()
{}

CDGClientSocket::~CDGClientSocket()
{}

void CDGClientSocket::DoHeartbeat()
{}

void CDGClientSocket::LoadConfig(CWgtIniFile* pIniFileParser)
{}

bool CDGClientSocket::Closed()
{}

bool CDGClientSocket::SendToServerPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)
{}

void CDGClientSocket::ProcDispatchMessage(PInnerMsgNode pNode)
{}

bool CDGClientSocket::IsTraceRole(const std::string &sRoleName)
{}

bool CDGClientSocket::GetSession(int iSessionID, PSessionInfo pResult)
{}

bool CDGClientSocket::ProcGMCmd(int iSessionID, const std::string &sParam1, const std::string &sParam2, const std::string &sParam3)
{}

void CDGClientSocket::ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen)
{}

void CDGClientSocket::OnSocketConnect(void* Sender)
{}

void CDGClientSocket::OnSocketDisconnect(void* Sender)
{}

void CDGClientSocket::OnSocketRead(void* Sender, const char* pBuf, int iCount)
{}

void CDGClientSocket::OnSocketError(void* Sender, int& iErrorCode)
{}

void CDGClientSocket::Reconnect()
{}

void CDGClientSocket::SendHeartbeat()
{}

void CDGClientSocket::OnRemoveSession(void* pValue, int iKey)
{}

void CDGClientSocket::LoadIpConfigFile()
{}

void CDGClientSocket::SendConfig(const std::string &sKey, std::string &sValue)
{}

bool CDGClientSocket::SetConfig(const std::string &sKey, std::string &sValue, bool bDel)
{}

std::string CDGClientSocket::GetConfigInfo(const std::string &sKey)
{}

void CDGClientSocket::SendRegisterServer()
{}

void CDGClientSocket::MsgSelectServer(int iParam, char* pBuf, unsigned short usBufLen)
{}

/************************End Of CDGClientSocket********************************************/