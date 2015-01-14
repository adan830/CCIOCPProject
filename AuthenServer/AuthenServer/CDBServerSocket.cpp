/**************************************************************************************
@author: 陈昌
@content: AuthenServer对DB服务器连接的监听socket管理
**************************************************************************************/
#include "stdafx.h"
#include "CDBServerSocket.h"

using namespace CC_UTILS;

CDBServerSocket* pG_DBSocket;

/************************Start Of CDBConnector******************************************/
CDBConnector::CDBConnector() :m_iServerID(0), m_iHumanCount(0), m_bCheckCredit(false), m_bCheckItem(false)  ///*FEnCode := nil;FDeCode := nil;*/
{}

CDBConnector::~CDBConnector()
{}

int CDBConnector::GetServerID()
{
	return m_iServerID;
}

int CDBConnector::GetHumanCount()
{
	return m_iHumanCount;
}

void CDBConnector::SendToClientPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)
{
	int iDataLen = sizeof(TServerSocketHeader) + usBufLen;
	char* pData = (char*)malloc(iDataLen);
	if (pData != nullptr)
	{
		try
		{
			((PServerSocketHeader)pData)->ulSign = SS_SEGMENTATION_SIGN;
			((PServerSocketHeader)pData)->usIdent = usIdent;
			((PServerSocketHeader)pData)->iParam = iParam;
			((PServerSocketHeader)pData)->usBehindLen = usBufLen;
			if (usBufLen > 0)
			{
				memcpy(pData + sizeof(TServerSocketHeader), pBuf, usBufLen);
				/*
				  if Assigned(FEnCode) then
					FEnCode(@Data[sizeof(TSocketHeader)], BufLen);
				*/
			}

			SendBuf(pData, iDataLen);
			free(pData);
		}
		catch (...)
		{
			free(pData);
		}
	}
}

void CDBConnector::SendToClientPeer(unsigned short usIdent, int iParam, const std::string &str)
{
	if (str.length() > 0)
	{
		SendToClientPeer(usIdent, iParam, const_cast<char*>(str.c_str()), str.length() + 1);
	}
	else
	{
		SendToClientPeer(usIdent, iParam, nullptr, 0);
	}
}

void CDBConnector::Execute(unsigned long ulTick)
{
	if (m_bCheckCredit)
	{
		m_bCheckCredit = false;
		//G_ServerSocket.AddRechargeQueryJob(FServerIdx, SocketHandle);
	}
	if (m_bCheckItem)
	{
		m_bCheckItem = false;
		//G_ServerSocket.AddQueryGiveItemJob(FServerIdx, SocketHandle);
	}
}

void CDBConnector::SocketRead(const char* pBuf, int iCount)
{}

void CDBConnector::ProcessReceiveMsg(PServerSocketHeader pHeader, char* pData, int iDataLen)
{}

void CDBConnector::InitDynCode()
{}

void CDBConnector::Msg_Ping(int iCount)
{}

void CDBConnector::Msg_RegisterServer(int iServerID)
{}

void CDBConnector::Msg_UserAuthenRequest(int iParam, char* pBuf, unsigned short usBufLen)
{}

void CDBConnector::Msg_NewAccountRequest(int iParam, char* pBuf, unsigned short usBufLen)
{}

void CDBConnector::Msg_DBResponse(int iIdent, int iParam, char* pBuf, unsigned short usBufLen)
{}

void CDBConnector::Msg_SafeCardAuthen(int iParam, char* pBuf, unsigned short usBufLen)
{}

void CDBConnector::SQLWorkCallBack(int iCmd, int iParam, const std::string &str)
{
	SendToClientPeer(iCmd, iParam, str);
}

void CDBConnector::OnAuthenFail(int iSessionID, int iRetCode, std::string &sMsg, int iAuthType, int iAuthenApp)
{}

/************************End Of CDBConnector******************************************/




/************************Start Of CDBServerSocket******************************************/

/************************End Of CDBServerSocket******************************************/