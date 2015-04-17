/**************************************************************************************
@author: �²�
@content: ��������ͨ�ŵĳ����ͽṹ����
**************************************************************************************/

#ifndef __CC_PROTOCOL_SERVER_H__
#define __CC_PROTOCOL_SERVER_H__

#include "CCGameCommon.h"
#include <string>

// Server��ͨѶ��Э��ͷ
typedef struct _TServerSocketHeader
{
	unsigned int uiSign;				// �ָ��� SS_SEGMENTATION_SIGN
	int iParam;							// ��չ����
	unsigned short usIdent;				// Э���
	unsigned short usBehindLen;			// �������ݳ���
}TServerSocketHeader, *PServerSocketHeader;

// Dispatch Gate -> DBServer ת�����ѡ����Ϣ
typedef struct _TClientSelectServerInfo
{
	int iSessionID;
	int iSelectServerID;
	int iEnCodeIdx;
	int iClientType;
	bool bMasterIP;
	unsigned char ucNetType;
}TClientSelectServerInfo, *PClientSelectServerInfo;

// DBServer -> DispatchGate
typedef struct _TSessionInfo
{
	int iEncodeIdx;					// �ӽ���Key
	int iAreaID;					// ѡ��������
	int iClientType;				// �ͻ�������
	unsigned int uiCreateTick;		// ������ʱ��
	bool bMasterIP;					// �Ƿ�ΪGM����
	unsigned char ucNetType;		// ��������

}TSessionInfo, *PSessionInfo;

// ip��ַ���ַ�������
typedef char TIPAddress[IP_ADDRESS_MAX_LEN+1];
typedef TIPAddress* PIPAddress;

// ��������ַ����
typedef struct _TServerAddress
{
	TIPAddress IPAddress;
	int iPort;
	unsigned char ucNetType;
}TServerAddress, *PServerAddress;

typedef struct _TServerAddressEx
{
	TServerAddress Addr;
	std::string sNetType;
}TServerAddressEx, *PServerAddressEx;

// �������ڲ�����Ϣ����
enum TInnerMsgType
{
	fromDispatchGate,
	fromDBServer,
	fromGameServer,
	fromGameGate,
	fromAuthenServer,
	fromDataBase,
	fromInternal,
	fromIMServer
}; 

// �������ڲ�����Ϣ�ṹ
typedef struct _TInnerMsgNode
{
	TInnerMsgType MsgFrom;
	int iIdx;
	int iParam;  // Client SocketHandle
	char* pBuf;
	unsigned short usIdent;
	unsigned short usBufLen;
	_TInnerMsgNode* pNext;
}TInnerMsgNode, *PInnerMsgNode;


// ��������ǰ��������Ϣ
typedef struct _TServerConnectInfo
{
	TServerAddress Addr;
	int iConnectCount;
}TServerConnectInfo, *PServerConnectInfo;

// ���������Ϣ
typedef struct _TPlayerConnectRec
{
	int iSessionID;
	int iIntAddr;
}TPlayerConnectRec, *PPlayerConnectRec;

// Pig��ѯ������Ϣ
typedef struct _TPigQueryServerInfo
{
	int iServerID;
	char szServerIP[IP_ADDRESS_MAX_LEN+1];
	char szServerName[SERVER_NAME_MAX_LEN+1];
}TPigQueryServerInfo, *PPigQueryServerInfo;

typedef struct _TPigMsgData
{
	unsigned char ucMsgType;
	unsigned short usAreaLen;
	unsigned short usMsgLen;
}TPigMsgData, *PPigMsgData;

// �����Ե��û���Ϣ
typedef struct _TGameChildLogin
{
	char szCard_ID[ACTOR_NAME_MAX_LEN];  //���֤
	char szRoleName[ACTOR_NAME_MAX_LEN];  //��ɫ��
}TGameChildLogin, *PGameChildLogin;

// �����ԣ���Ӧ����ʱ��
typedef struct _TGameChildInfo
{
	TGameChildLogin Child;
	int iOnLineSecond;
}TGameChildInfo, *PGameChildInfo;

// ��ɫ��Ϣ
typedef struct _TRoleInfo
{
	char szRoleName[ACCOUNT_MAX_LEN];
	int iFlag;
}TRoleInfo, *PRoleInfo;

// �ʺű��ö������
typedef enum _TAccountFlag {
	//δ֪0x1���󶨰�ȫ��0x2��������0x4���ǰ�ȫ�˺�0x8, ���뱣��
	accKnown = 0, accSafeCard, accNeedModifyPwd, accNoSafeAccount, accPwdAccount, accResv5, accResv6, accResv7,
	accResv8, accResv9, accResv10, accResv11, accResv12, accResv13, accResv14, accResv15,
	accResv16, accResv17, accResv18, accResv19, accResv20, accResv21, accResv22, accResv23,
	accResv24, accResv25, accResv26, accResv27, accResv28, accResv29, accResv30, accResv31
} TAccountFlag;

//c++����enum���Ͷ��ڼ��ϲ�����δ�����������������������������������������
//c++����enum���Ͷ��ڼ��ϲ�����δ�����������������������������������������
//c++����enum���Ͷ��ڼ��ϲ�����δ�����������������������������������������
//---------------------------------------------
//---------------------------------------------
typedef union _TAccountFlagInfo
{
	int iFlag;
	TAccountFlag FlagSet;
}TAccountFlagInfo;

typedef struct _TJsonJobNode
{
	int iCmd;
	int iHandle;
	int iParam;
	int iRes;
	unsigned int uiAddTick;
	std::string sJsonText;
	_TJsonJobNode* pNext;
}TJsonJobNode, *PJsonJobNode;

const int SS_SEGMENTATION_SIGN = 0XFFEEDDCC;                        // ������֮��ͨ��Э����ʼ��־

// ��������Э��
const int SM_REGISTER = 0x1000;			                            // ע�������
const int SM_UNREGISTER = 0x1001;							        // ע��������
const int SM_PING = 0x1002;								            // �������
const int SM_USER_AUTHEN_REQ = 0x1003;                              // ������֤
const int SM_USER_AUTHEN_RES = 0x1004;                              // ��֤����
const int SM_SERVER_CONFIG = 0x1005;                                // ������Ϣ
const int SM_USER_REGIST_REQ = 0x1008;                              // �û�ע��
const int SM_USER_REGIST_RES = 0x1009;								// �û�ע�᷵��
const int SM_KICKOUT_ACCOUNT = 0x1010;                              // �ߵ��˺�
const int SM_RECHARGE_AREA_QUERY = 0x1011;                          // �����������Զ���ѯ��ֵ��Ϣ
const int SM_RECHARGE_DB_REQ = 0x1012;                              // ��֤��������db���ͳ�ֵ����
const int SM_RECHARGE_DB_ACK = 0x1013;								// db���ظ���֤��������ֵ���
const int SM_USER_AUTHEN_LOG = 0x1014;                              // �����֤�ɹ����һ���ص��������
const int SM_CHILD_ONLINE_TIME = 0x1015;                            // �����ԣ����ߵ�ʱ��
const int SM_CHILD_LOGON = 0x1016;									// �����ԣ�����
const int SM_CHILD_LOGOUT = 0x1017;									// �����ԣ�����
const int SM_SAFECARD_AUTHEN_REQ = 0x1019;							// �ܱ�����֤
const int SM_SAFECARD_AUTHEN_RES = 0x1020;                          // �ܱ�����֤����

const int SM_IN_CREDIT_NOW = 0x1020;                                // ��ֵ Web�ӿ�<->AuthenServer  ------  �ظ�
const int SM_SENDITEM_NOW = 0x1021;                                 // �͵��� Web�ӿ�<->AuthenServer
const int SM_KICKOUT_NOW = 0x1022;                                  // ������ Web�ӿ�<->AuthenServer

const int SM_ENCODE_BUFFER = 0x1101;                                // ��������
const int SM_DECODE_BUFFER = 0x1102;                                // ����

const int SM_SELECT_SERVER = 0x2001;                                // ѡ������
const int SM_PLAYER_CONNECT = 0x2002;                               // �������
const int SM_PLAYER_DISCONNECT = 0x2003;                            // ��Ҷ���
const int SM_PLAYER_MSG = 0x2004;                                   // ��ҵ���Ϣ
const int SM_MULPLAYER_MSG = 0x2005;                                // Ⱥ������Ϣ

const int SM_PLAYER_UPDATE_IDX = 0x2012;                            // �������������DB�е�����
const int SM_BROADCAST_MSG = 0x2013;                                // ȫ�����ݹ㲥

const int SM_FILTER_WORD = 0x2019;                                  // ��������Ϣ



const int SM_GIVEITEM_QUERY = 0x2044;                               // �������������Զ���ѯ���͵�����Ϣ
const int SM_GIVEITEM_DB_REQ = 0x2045;                              // ��֤��������db�����͵�������
const int SM_GIVEITEM_DB_ACK = 0x2046;								// db���ظ���֤�������͵��߽��
const int SM_SHUTDOWN = 0x2052;                                     // �رշ���
const int SM_REFRESH_RECHARGE = 0x2056;								// ֪ͨ��֤����������ѯ��ֵ

const int SM_GAME_ACT_CODE_REQ = 0x2069;                            // ��Ϸ�ڼ���CPS����ȡý������������
const int SM_GAME_ACT_CODE_RES = 0x206A;                            // ��Ϸ�ڼ���CPS����ȡý�������뷵��

//PigServer���Э��
const int SM_PIG_MSG = 0x3001;		     						    //��תPig��Ϣ
const int SM_PIG_QUERY_AREA = 0x3002;                               //��ѯ������Ϣ

//��ϷID
const int G_GAME_ID = 1;                                    

// Ĭ�ϵ������˿�
const int DEFAULT_AuthenServer_PORT = 2300;                         // AuthenServer <- DB
const int DEFAULT_DispatchGate_DB_PORT = 3300;                      // DispatchGate <- DB
const int DEFAULT_DispatchGate_CLIENT_PORT = 4300;                  // DispatchGate <- CLIENT
const int DEFAULT_DBServer_GS_PORT = 5300;                          // DBServer     <- GS
const int DEFAULT_DBServer_GG_PORT = 6300;                          // DBServer     <- GG
const int DEFAULT_GameServer_PORT = 7300;                           // GameServer   <- GG
const int DEFAULT_GameGate_PORT = 8300;                             // GameGate     <- CLIENT
const int DEFAULT_IMServer_GS_PORT = 9300;                          // IMServer  <- GS
const int DEFAULT_IMServer_GG_PORT = 7310;                          // IMServer <- GG
const int DEFAULT_RESCENTER_PORT = 9900;                            // ResourceServer <- Client
const int DEFAULT_RESCENTERCENTER_CR_PORT = 2900;                   // ResourceCenter <- ResourceServer
const int DEFAULT_RESCENTERCENTER_CD_PORT = 3900;                   // ResourceCenter <- DispatchGate
const int DEFAULT_WEBINTERFACE_PORT = 8010;                         // WEB <- WebClient
const int DEFAULT_WEBSERVER_PORT = 8011;                            // WEBInterface <- AuthenServer
const int DEFAULT_WEBQUERYSRV_PORT = 8012;                          // WEBQuery <- WebClient
const int DEFAULT_ACTLOG_SERVER_PORT = 8500;                        // GS <- ACTLog
const int DEFAULT_PIG_SERVER_PORT = 8600;                           // DispatchGate <- PigServer
const int DEFAULT_CONTROL_SERVER_PORT = 9800;                       // EventLog <- ControlClient

//mysql���
const std::string DB_HOSTNAME = "localhost";
const int DB_PORT = 3306;
const std::string DB_PROTOCOL = "mysql-5";
const std::string DB_USERNAME = "gamemaster";
const std::string DB_PASSWORD = "WByjUrYaYCt]HODoDl";                 //�����ģ���������Decodestring
const std::string DB_DATABASE = "";

const std::string DEFAULT_MQ_SERVER_IP = "222.73.123.112";


#endif //__CC_PROTOCOL_SERVER_H__