/**************************************************************************************
@author: �²�
@content: GameGate�Կͻ������ӵļ���socket����
**************************************************************************************/
#ifndef __CC_CLIENT_SERVER_SOCKET_H__
#define __CC_CLIENT_SERVER_SOCKET_H__

#include "stdafx.h"
#include "CDBClientSocket.h"
#include "CGSClientSocket.h"
#include "CIMClientSocket.h"

/**
*
* GameGate�����ĵ���PlayerClient�����Ӷ���
*
*/
class CPlayerClientConnector : public CClientConnector
{
public:

protected:

private:

private:

};

/*
  TConnecter = class(TCustomClient)
  private
    m_ReceiveBuffer: TBufferStream;
    m_DelayCS: TRTLCriticalSection;
    m_OnSendToServer: TOnSendToServer;
    m_PackageIdx: Cardinal;
    m_ObjectID: Integer;
    m_RoleName: AnsiString;
    m_Account: AnsiString;
    m_BoTrace: Boolean;
    m_BoGM: Boolean;
    m_LastPackageTick: Cardinal;
    m_LastCMCmd: Word;
    m_LastSCMCmd: Word;
    m_CloseTick: Cardinal;
    m_SkillTable: TList;
    m_First, m_Last: PClientActionNode;
    m_QueueCount: integer;
    m_LastCD_Ticks: array[1..MAX_CD_ID] of Cardinal;
    m_EnCode, m_DeCode: TCodingProc;
    m_DisConnected: Boolean;                                //�Ƿ�Ͽ�
    m_BoDeath: Boolean;                                     //�Ƿ�����
    m_BoNormalClose: Boolean;                               //�Ƿ������ر�
    m_MapID: Integer;
    m_HitSpeed: Word;
    procedure InitDynCode(EdIdx: Byte = 0);
  private
    m_GPS_Request: Boolean;                                 // �Ѿ�
    m_GPS_Request_Start: Cardinal;
    m_GPS_Action_Count: integer;
    m_CSAuthObject: TCSAuthObject;                          // ����ң�������
    procedure GPSCheck();                                   // ��Ҫ���Ķ���
  private
    m_LastActionTick: Cardinal;
    m_StiffTime: Word;                                      // ������Ӳֱʱ��
    function AcceptNextAction: Boolean;
    procedure Stiffen(aType: TActionType);                  // ��ʼһ��Ӳֱʱ��
    procedure IncStiffen(IncValue: Cardinal);               // ����һ��Ӳֱʱ��
    function AnalyseIdent(Buf: PAnsiChar; BufLen: Word;     // �������е�����
      var bCDType: Byte; var cdTime: Cardinal): TActionType;
    procedure AddToDelayQueue(nNode: PClientActionNode);    // ���ӵ��ӳٶ���
    procedure ProcDelayQueue;                               // ����
    function CoolDelayPass(nNode: PClientActionNode): Boolean; // CD�ܷ�ͨ��
    procedure scmSkillList(Buf: PAnsiChar; BufLen: Word);   // ���ռ��ܱ�
    procedure scmAddSkill(Buf: PAnsiChar; BufLen: Word);    // �����ļ���
    procedure scmUpdateCDTime(Buf: PAnsiChar; BufLen: Word); // ����CoolDelay
    function NeedQueueCount(bCDType: Byte): Boolean;
    procedure OpenWindow(wtype: TClientWindowType; nParam: integer; const msg: ansistring = '');
  private
    function CheckGuildWords(Buf: PAnsiChar; BufLen: Word): Boolean;
    function CheckInputWords(Buf: PAnsiChar; BufLen: Word): Boolean;
    procedure MsgProcess(Buf: PAnsiChar; BufLen: Word);     // ����Э��
    procedure ReceiveServerMsg(Buf: PansiChar; BufLen: Word);
    function CheckServerPkg(Ident: Word; Buf: PAnsiChar; BufLen: integer): Boolean;
    procedure SendMsg(const sMsg: AnsiString; MsgType: TMesssageType = msHint; Color: Byte = Byte(-1); BackColor: Byte = Byte(-1));
  protected
    procedure SendActGood(Act, ActParam: Word);             // û����
    procedure UpdateCDTime(CDType: Byte; CDTime: Cardinal); //û���ϸ���CD
  protected
    procedure SocketRead(const Buf; Count: integer); override;
    procedure Execute(Tick: cardinal); override;
    procedure OnCreate; override;
    procedure OnDestroy; override;
  public
    procedure SendToClient(wIdent: Word; dwidx: DWord; Buf: PAnsiChar; BufLen: Word);
    procedure DelayClose(nReason: Integer = -1);
    procedure OnDisconnect;
  end;
*/

/**
*
* GameGate������PlayerClient�ļ���������
*
*/
class CClientServerSocket : public CIOCPServerSocketManager
{
public:
	CClientServerSocket();
	virtual ~CClientServerSocket();
	void SMServerConfig(int iParam, char* pBuf, unsigned short usBufLen);
	void ProcServerMessage(unsigned short usIdent, unsigned short usHandle, char* pBuf, unsigned short usBufLen);
	void ClientManage(unsigned short usIdent, unsigned short usHandle, char* pBuf, unsigned short usBufLen, bool bInGame);
	void GameServerShutDown();
public:
	CGSClientSocket* m_pGameServer;
	CDBClientSocket* m_pDBServer;
	std::string m_InternetIP;
protected:
	virtual void DoActive();
private:
	/*
	procedure OnClientDisConnect(SockHandle: Word);   ----- �������ö�������
	*/
	void LoadConfig();
	CClientConnector* OnCreateClientSocket(const std::string &sIP);
	void OnClientError(void* Sender, int& iErrorCode);
	void OnListenReady(void* Sender);
	void OnClientConnect(void* Sender);
	void OnClientDisconnect(void* Sender);
	void NotifyNotExistClient(unsigned short usHandle, int iReason);
private:
	bool m_bListenOK;
	CIMClientSocket* m_pIMServer;
	std::string m_sDBAddr;
	int m_iDBPort;
	int m_iLoopCount;
	unsigned int m_uiSlowerTick;
	std::string m_sGSIP;
};

extern CClientServerSocket* pG_ClientServerSocket;

#endif //__CC_CLIENT_SERVER_SOCKET_H__