/**************************************************************************************
@author: 陈昌
@content: DBServer作为客户端方连接DispatchGate服务器的端口
**************************************************************************************/
#ifndef __CC_DISPATCHGATE_CLIENT_SOCKET_H__
#define __CC_DISPATCHGATE_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer作为客户端方连接DispatchGate服务器的端口
*
*/

class CDGClientSocket : public CIOCPClientSocketManager
{
public:
	procedure OnCreate; override;
	procedure OnDestroy; override;
    procedure DoHeartbest;                                  // 主线程调用
    procedure LoadConfig(IniFile: TiniFile);
    function Closed: Boolean;
    function SendToServer(Ident: Word; Param: integer;
      Buf: PAnsiChar; BufLen: Word): Boolean;
    procedure ProcDispatchMessage(nNode: PInterMsgNode);
    function IsTraceRole(const RoleName: AnsiString): Boolean;
    function GetSession(SessionID: integer; pResult: PSessionInfo): Boolean;
    function ProcGMCmd(SessionID: Integer; const Param1, Param2, Param3: AnsiString): Boolean;
    property BoDenyAll: Boolean read FBoDenyAll;
protected:
	procedure SocketConnect(Sender : TObject);
	procedure SocketDisConnect(Sender: TObject);
	procedure SocketRead(Sender: TObject; const Buf; Count: integer);
	procedure SocketError(Sender: TObject; var ErrorCode : integer);
private:
    procedure ReConnect;
    procedure OnRemoveSession(Pvalue: Pointer; Key: integer);
    procedure LoadIpConfigFile;
    procedure SendConfig(const Key: AnsiString; Value: AnsiString);
    function SetConfig(const Key: AnsiString; Value: AnsiString; bDel: Boolean): Boolean;
    function GetConfigInfo(const Key: AnsiString): AnsiString;
    procedure SendHeartBeat;                                // 发送心跳
    procedure SendRegisterServer;
    procedure MsgProcess(PHeader: PSocketHeader; Buf: PAnsiChar; BufLen: Word); // 处理协议
    procedure MsgSelectServer(Param: integer; Buf: PansiChar; BufLen: Word);
private:
    FCheckTick: Cardinal;                                   // 检测连接时间
    FPingCount: integer;
    FReceiveBuffer: TBufferStream;                          // 接收缓冲
    FServerArray: array[1..3] of TServerAddress;            // 可配置多个Dispatch Gate
    FWorkIndex: integer;                                    // 当前使用的Dispatch顺序号
    FSessionList: TIntegerHash;                             // 会话列表
    FConfigFileAge: integer;
    FTraceList: TStringList;
    FBoDenyAll: Boolean;

};

extern CDGClientSocket* pG_DispatchGateSocket;

#endif //__CC_DISPATCHGATE_CLIENT_SOCKET_H__