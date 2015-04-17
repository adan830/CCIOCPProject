/**************************************************************************************
@author: �²�
@content: DBServer��Ϊ����������GameGate������
**************************************************************************************/
#include "stdafx.h"
#include "CGGServerSocket.h"
using namespace CC_UTILS;

CGGServerSocket* pG_GameGateSocket;

/************************Start Of CGGConnector******************************************/

CGGConnector::CGGConnector() :m_sNetType(""), m_iServerIdx(0), m_iOnlineCount(0), m_bEnable(false), m_GamePlayerHash(1023)
{
}

CGGConnector::~CGGConnector()
{
	m_GamePlayerHash.Clear();
}

void CGGConnector::SendToClientPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen)
{
	int iDataLen = sizeof(TServerSocketHeader)+usBufLen;
	char* pData = (char*)malloc(iDataLen);
	if (pData != nullptr)
	{
		try
		{
			((PServerSocketHeader)pData)->uiSign = SS_SEGMENTATION_SIGN;
			((PServerSocketHeader)pData)->usIdent = usIdent;
			((PServerSocketHeader)pData)->iParam = iParam;
			((PServerSocketHeader)pData)->usBehindLen = usBufLen;
			if (usBufLen > 0)
				memcpy(pData + sizeof(TServerSocketHeader), pBuf, usBufLen);

			SendBuf(pData, iDataLen);
			free(pData);
		}
		catch (...)
		{
			free(pData);
		}
	}
}

void CGGConnector::SocketRead(const char* pBuf, int iCount)
{
	//�ڻ������������ݰ���������ProcessReceiveMsg����߼���Ϣ����
	int iErrorCode = ParseSocketReadData(1, pBuf, iCount);
	if (iErrorCode > 0)
		Log("CGGConnector Socket Read Error, Code = " + std::to_string(iErrorCode), lmtError);
}

void CGGConnector::ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen)
{
	PInnerMsgNode pNode;
	PServerSocketHeader pH = (PServerSocketHeader)pHeader;
	switch (pH->usIdent)
	{
	case SM_REGISTER:
		Msg_Register(pH->iParam, pData, iDataLen);
		break;
	case SM_PING:
		Msg_Ping(pH->iParam, pData, iDataLen);
		break;
	default:
		//������Ϣ�����»ص����̵߳Ķ����д�����֤�̰߳�ȫ
		pNode = new TInnerMsgNode();
		pNode->MsgFrom = fromGameGate;
		pNode->iIdx = m_iServerIdx;
		pNode->iParam = pH->iParam;
		pNode->usIdent = pH->usIdent;
		if ((pData != nullptr) && (iDataLen > 0))
		{
			pNode->usBufLen = iDataLen;
			pNode->pBuf = (char*)malloc(iDataLen);
			memcpy_s(pNode->pBuf, iDataLen, pData, iDataLen);
		}
		else
		{
			pNode->pBuf = nullptr;
			pNode->usBufLen = 0;
		}
		pG_MainThread->ReceiveMessage(pNode);
		break;
	}
}

void CGGConnector::Msg_Register(int iParam, char* pBuf, unsigned short usBufLen)
{
	/*
var
  P                 : PServerAddress;
begin
  if BufLen = sizeof(TServerAddress) then
  begin
    P := PServerAddress(Buf);
    if G_GGSocket.RegisterGameGate(Self, P^.IPAddress, P^.nPort) then
    begin
      if FNetType <> '' then
        Log(Format('GameGate %d %s:%d Enabled. NetType=%s', [FServerIdx, p^.IPAddress, p^.nPort, FNetType]), lmtMessage)
      else
        Log(Format('GameGate %d %s:%d Enabled.', [FServerIdx, p^.IPAddress, p^.nPort]), lmtMessage);
      P := G_GSSocket.GetGameServerInfo;
      SendBuffer(SM_SERVER_CONFIG, FServerIdx, PAnsiChar(P), sizeof(TServerAddress));
      G_MainThread.SendFilterWords(SendBuffer);
    end
    else
      Log(Format('Invalid GameGate %s:%d', [P^.IPAddress, P^.nPort]), lmtError);
  end
  else
    Log('Invalid GameGate(' + RemoteAddress + ') Data. BufLen = ' + IntToStr(BufLen), lmtError);
end;
	*/
}

void CGGConnector::Msg_Ping(int iParam, char* pBuf, unsigned short usBufLen)
{
	if (iParam >= 0)
	{
		m_bEnable = true;
		m_iOnlineCount = iParam;
	}
	SendToClientPeer(SM_PING, 0, nullptr, 0);
}

/************************End Of CGGConnector******************************************/


/************************Start Of CGGServerSocket******************************************/

/************************End Of CGGServerSocket******************************************/