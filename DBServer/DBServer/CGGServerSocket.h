/**************************************************************************************
@author: 陈昌
@content: DBServer作为服务器监听GameGate的连接
**************************************************************************************/
#ifndef __CC_GAMEGATE_SERVER_SOCKET_H__
#define __CC_GAMEGATE_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer监听的单个GameGate的连接对象
*
*/
class CGGConnector : public CClientConnector
{
public:
	CGGConnector();
	virtual ~CGGConnector();
	void SendToClientPeer(unsigned short usIdent, int iParam, char* pBuf, unsigned short usBufLen);
	int GetServerIdx(){ return m_iServerIdx; }
	int GetOnlineCount(){ return m_iOnlineCount; }
	bool IsEnable(){ return m_bEnable; }
protected:
	virtual void SocketRead(const char* pBuf, int iCount);
	virtual void ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen);
private:
	void Msg_Register(int iParam, char* pBuf, unsigned short usBufLen);
	void Msg_Ping(int iParam, char* pBuf, unsigned short usBufLen);
private:
	std::string m_sNetType;
	int m_iServerIdx;
	int m_iOnlineCount;
	bool m_bEnable;
	CC_UTILS::CIntegerHash m_GamePlayerHash;
};

/**
*
* DBServer对GameGate的监听管理器
*
*/
class CGGServerSocket : public CIOCPServerSocketManager
{
public:
	CGGServerSocket();
	virtual ~CGGServerSocket();
	void LoadConfig(CWgtIniFile* pIniFileParser);
	void GetComfyGate(int &iAddr, int &iPort, unsigned char ucNetType);
	void ProcGameGateMessage(PInnerMsgNode pNode);
	void KickOutClient(unsigned char ucIdx, unsigned short usHandle, int iReason);
	void AddOnlineCount(unsigned char ucGGIdx, int iCount = 1);

	procedure SendToClient(GGIdx: Byte; wHandle: Word; Buf: PAnsiChar; BufLen: Word);
	procedure SendToGameGate(GGIdx: Byte; Ident: Word; Param: Integer; Buf: PAnsiChar; BufLen: Word);
	procedure SetGameGateNet(GGIdx: Byte; const NetType : AnsiString);
	procedure ReSendFilterWords;

	std::string GetAllowIPs();
protected:
private:
private:
};

/*
    FAllowIPs: ansistring;
    FServerArray: array[1..MAX_GAMEGATE_COUNT] of TServerAddressEx;
    function CheckConnectIP(const ConnectIP: ansistring): Boolean;
    procedure SocketError(Sender: TObject; var ErrorCode: integer);
    function CreateCustomSocket(const IP: ansistring): TCustomClient;
    procedure GGConnect(Sender: TObject);
    procedure GGDisConnect(Sender: TObject);
    function RegisterGameGate(Sender: TGameGate; const sAddr: ansistring; nPort: integer): Boolean;
    
    procedure smPlayerConnect(nNode: PInterMsgNode);
  protected

  public

*/

extern CGGServerSocket* pG_GameGateSocket;

#endif //__CC_GAMEGATE_SERVER_SOCKET_H__