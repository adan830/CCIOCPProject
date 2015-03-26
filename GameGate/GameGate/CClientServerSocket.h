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
	CPlayerClientConnector();
	virtual ~CPlayerClientConnector();
	void SendToClientPeer(unsigned short usIdent, unsigned int uiIdx, void* pBuf, unsigned short usBufLen);
	void DelayClose(int iReason = -1);
	void OnDisconnect();
protected:
	void SendActGood(unsigned short usAct, unsigned short usActParam);       // 没用上
	void UpdateCDTime(unsigned char bCDType, unsigned int uiCDTime);		 //没用上更新CD
	virtual void Execute(unsigned int uiTick);
	virtual void SocketRead(const char* pBuf, int iCount);
private:
	bool CheckGuildWords(char* pBuf, unsigned short usBufLen);
	bool CheckInputWords(char* pBuf, unsigned short usBufLen);
	void ProcessReceiveMsg(char* pHeader, char* pBuf, int iBufLen);
	void ReceiveServerMsg(char* pBuf, unsigned short usBufLen);
	bool CheckServerPkg(unsigned short usIdent, char* pBuf, unsigned short usBufLen);
	void SendMsg(const std::string &sMsg, TMesssageType msgType = msHint, unsigned char ucColor = 255, unsigned char ucBackColor = 255);
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

  protected

  public

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