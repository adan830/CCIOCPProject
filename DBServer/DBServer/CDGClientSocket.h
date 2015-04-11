/**************************************************************************************
@author: �²�
@content: DBServer��Ϊ�ͻ��˷�����DispatchGate�������Ķ˿�
**************************************************************************************/
#ifndef __CC_DISPATCHGATE_CLIENT_SOCKET_H__
#define __CC_DISPATCHGATE_CLIENT_SOCKET_H__

#include "stdafx.h"

/**
*
* DBServer��Ϊ�ͻ��˷�����DispatchGate�������Ķ˿�
*
*/

class CDGClientSocket : public CIOCPClientSocketManager
{
public:
	procedure OnCreate; override;
	procedure OnDestroy; override;
    procedure DoHeartbest;                                  // ���̵߳���
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
    procedure SendHeartBeat;                                // ��������
    procedure SendRegisterServer;
    procedure MsgProcess(PHeader: PSocketHeader; Buf: PAnsiChar; BufLen: Word); // ����Э��
    procedure MsgSelectServer(Param: integer; Buf: PansiChar; BufLen: Word);
private:
    FCheckTick: Cardinal;                                   // �������ʱ��
    FPingCount: integer;
    FReceiveBuffer: TBufferStream;                          // ���ջ���
    FServerArray: array[1..3] of TServerAddress;            // �����ö��Dispatch Gate
    FWorkIndex: integer;                                    // ��ǰʹ�õ�Dispatch˳���
    FSessionList: TIntegerHash;                             // �Ự�б�
    FConfigFileAge: integer;
    FTraceList: TStringList;
    FBoDenyAll: Boolean;

};

extern CDGClientSocket* pG_DispatchGateSocket;

#endif //__CC_DISPATCHGATE_CLIENT_SOCKET_H__