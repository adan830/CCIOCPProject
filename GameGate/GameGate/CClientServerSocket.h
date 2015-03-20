/**************************************************************************************
@author: 陈昌
@content: GameGate对客户端连接的监听socket管理
**************************************************************************************/
#ifndef __CC_CLIENT_SERVER_SOCKET_H__
#define __CC_CLIENT_SERVER_SOCKET_H__

#include "stdafx.h"
#include "CDBClientSocket.h"
#include "CGSClientSocket.h"
#include "CIMClientSocket.h"

/**
*
* GameGate监听的单个PlayerClient的连接对象
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
    m_DisConnected: Boolean;                                //是否断开
    m_BoDeath: Boolean;                                     //是否死亡
    m_BoNormalClose: Boolean;                               //是否正常关闭
    m_MapID: Integer;
    m_HitSpeed: Word;
    procedure InitDynCode(EdIdx: Byte = 0);
  private
    m_GPS_Request: Boolean;                                 // 已经
    m_GPS_Request_Start: Cardinal;
    m_GPS_Action_Count: integer;
    m_CSAuthObject: TCSAuthObject;                          // 反外挂，检测对象
    procedure GPSCheck();                                   // 需要检测的动作
  private
    m_LastActionTick: Cardinal;
    m_StiffTime: Word;                                      // 动作的硬直时间
    function AcceptNextAction: Boolean;
    procedure Stiffen(aType: TActionType);                  // 开始一个硬直时间
    procedure IncStiffen(IncValue: Cardinal);               // 增加一个硬直时间
    function AnalyseIdent(Buf: PAnsiChar; BufLen: Word;     // 分析上行的数据
      var bCDType: Byte; var cdTime: Cardinal): TActionType;
    procedure AddToDelayQueue(nNode: PClientActionNode);    // 增加到延迟队列
    procedure ProcDelayQueue;                               // 处理
    function CoolDelayPass(nNode: PClientActionNode): Boolean; // CD能否通过
    procedure scmSkillList(Buf: PAnsiChar; BufLen: Word);   // 接收技能表
    procedure scmAddSkill(Buf: PAnsiChar; BufLen: Word);    // 新增的技能
    procedure scmUpdateCDTime(Buf: PAnsiChar; BufLen: Word); // 更新CoolDelay
    function NeedQueueCount(bCDType: Byte): Boolean;
    procedure OpenWindow(wtype: TClientWindowType; nParam: integer; const msg: ansistring = '');
  private
    function CheckGuildWords(Buf: PAnsiChar; BufLen: Word): Boolean;
    function CheckInputWords(Buf: PAnsiChar; BufLen: Word): Boolean;
    procedure MsgProcess(Buf: PAnsiChar; BufLen: Word);     // 处理协议
    procedure ReceiveServerMsg(Buf: PansiChar; BufLen: Word);
    function CheckServerPkg(Ident: Word; Buf: PAnsiChar; BufLen: integer): Boolean;
    procedure SendMsg(const sMsg: AnsiString; MsgType: TMesssageType = msHint; Color: Byte = Byte(-1); BackColor: Byte = Byte(-1));
  protected
    procedure SendActGood(Act, ActParam: Word);             // 没用上
    procedure UpdateCDTime(CDType: Byte; CDTime: Cardinal); //没用上更新CD
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
* GameGate对所有PlayerClient的监听管理器
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
	procedure OnClientDisConnect(SockHandle: Word);   ----- 函数不用独立出来
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