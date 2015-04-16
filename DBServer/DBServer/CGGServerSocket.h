/**************************************************************************************
@author: �²�
@content: DBServer��Ϊ����������GameGate������
**************************************************************************************/
#ifndef __CC_GAMEGATE_SERVER_SOCKET_H__
#define __CC_GAMEGATE_SERVER_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer�����ĵ���GameGate�����Ӷ���
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
* DBServer��GameGate�ļ���������
*
*/
class CGGServerSocket : public CIOCPServerSocketManager
{
public:
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
    function GetAllowIPs: ansistring;
    procedure smPlayerConnect(nNode: PInterMsgNode);
  protected
    procedure OnCreate; override;
    procedure OnDestroy; override;
  public
    procedure LoadConfig(IniFile: TIniFile);
    procedure GetComfyGate(var nAddr: integer; var nPort: integer; nettype: Byte);
    procedure ProcGameGateMessage(nNode: PInterMsgNode);
    procedure KickOutClient(Idx: Byte; handle: Word; iReason: Integer);
    procedure AddOnlineCount(GGIdx: Byte; iCount: Integer = 1);
    procedure SendToClient(GGIdx: Byte; wHandle: Word; Buf: PAnsiChar; BufLen: Word);
    procedure SendToGameGate(GGIdx: Byte; Ident: Word; Param: Integer; Buf: PAnsiChar; BufLen: Word);
    procedure SetGameGateNet(GGIdx: Byte; const NetType: AnsiString);
    procedure ReSendFilterWords;
    property AllowGateIP: ansistring read GetAllowIPs;
*/

extern CGGServerSocket* pG_GameGateSocket;

#endif //__CC_GAMEGATE_SERVER_SOCKET_H__