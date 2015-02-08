/**************************************************************************************
@author: 陈昌
@content: 服务器间通信的常量和结构定义
**************************************************************************************/

#ifndef __CC_PROTOCOL_SERVER_H__
#define __CC_PROTOCOL_SERVER_H__

#include "CCGameCommon.h"

// Server间通讯的协议头
typedef struct _TServerSocketHeader
{
	unsigned int uiSign;				// 分隔符 SS_SEGMENTATION_SIGN
	int iParam;							// 扩展参数
	unsigned short usIdent;				// 协议号
	unsigned short usBehindLen;			// 后续数据长度
}TServerSocketHeader, *PServerSocketHeader;

// Dispatch Gate -> DBServer 转发玩家选区信息
typedef struct _TClientSelectServerInfo
{
	int iSessionID;
	int iSelectServerID;
	int iEnCodeIdx;
	int iClientType;
	bool bMasterIP;
	unsigned char ucNetType;
}TClientSelectServerInfo, *PClientSelectServerInfo;

// ip地址的字符串类型
typedef char TIPAddress[IP_ADDRESS_MAX_LEN+1];
typedef TIPAddress* PIPAddress;

// 服务器地址类型
typedef struct _TServerAddress
{
	TIPAddress IPAddress;
	int iPort;
	unsigned char ucNetType;
}TServerAddress, *PServerAddress;

// 服务器当前的连接信息
typedef struct _TServerConnectInfo
{
	TServerAddress Addr;
	int iConnectCount;
}TServerConnectInfo, *PServerConnectInfo;

// Pig查询区组信息
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

// 防沉迷的用户信息
typedef struct _TGameChildLogin
{
	char szCard_ID[ACTOR_NAME_MAX_LEN];  //身份证
	char szRoleName[ACTOR_NAME_MAX_LEN];  //角色名
}TGameChildLogin, *PGameChildLogin;

// 防沉迷，回应在线时长
typedef struct _TGameChildInfo
{
	TGameChildLogin Child;
	int iOnLineSecond;
}TGameChildInfo, *PGameChildInfo;

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

const int SS_SEGMENTATION_SIGN = 0XFFEEDDCC;                        // 服务器之间通信协议起始标志

// 服务器间协议
const int SM_REGISTER = 0x1000;			                            // 注册服务器
const int SM_UNREGISTER = 0x1001;							        // 注销服务器
const int SM_PING = 0x1002;								            // 心跳检测
const int SM_USER_AUTHEN_REQ = 0x1003;                              // 请求认证
const int SM_USER_AUTHEN_RES = 0x1004;                              // 认证返回
const int SM_SERVER_CONFIG = 0x1005;                                // 配置信息
const int SM_USER_REGIST_REQ = 0x1008;                              // 用户注册
const int SM_USER_REGIST_RES = 0x1009;								// 用户注册返回
const int SM_KICKOUT_ACCOUNT = 0x1010;                              // 踢掉账号
const int SM_RECHARGE_AREA_QUERY = 0x1011;                          // 按虚拟区号自动轮询充值信息
const int SM_RECHARGE_DB_REQ = 0x1012;                              // 认证服务器向db发送充值请求
const int SM_RECHARGE_DB_ACK = 0x1013;								// db返回给认证服务器充值结果
const int SM_CHILD_ONLINE_TIME = 0x1015;                            // 防沉迷，在线的时间
const int SM_CHILD_LOGON = 0x1016;									// 防沉迷，上线
const int SM_CHILD_LOGOUT = 0x1017;									// 防沉迷，下线
const int SM_SAFECARD_AUTHEN_REQ = 0x1019;							// 密保卡认证
const int SM_SAFECARD_AUTHEN_RES = 0x1020;                          // 密保卡认证返回

const int SM_ENCODE_BUFFER = 0x1101;                                // 启动加密
const int SM_DECODE_BUFFER = 0x1102;                                // 解密

const int SM_SELECT_SERVER = 0x2001;                                // 选服请求
const int SM_PLAYER_CONNECT = 0x2002;                               // 玩家连接
const int SM_PLAYER_DISCONNECT = 0x2003;                            // 玩家断线
const int SM_PLAYER_MSG = 0x2004;                                   // 玩家的消息
const int SM_MULPLAYER_MSG = 0x2005;                                // 群发的消息

const int SM_GIVEITEM_QUERY = 0x2044;                               // 按照虚拟区号自动轮询推送道具信息
const int SM_GIVEITEM_DB_REQ = 0x2045;                              // 认证服务器向db发送送道具请求
const int SM_GIVEITEM_DB_ACK = 0x2046;								// db返回给认证服务器送道具结果
const int SM_REFRESH_RECHARGE = 0x2056;								// 通知认证服务立即查询充值

//PigServer相关协议
const int SM_PIG_MSG = 0x3001;		     						    //中转Pig消息
const int SM_PIG_QUERY_AREA = 0x3002;                               //查询区组信息

//游戏ID
const int G_GAME_ID = 1;                                    

// 默认的侦听端口
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


#endif //__CC_PROTOCOL_SERVER_H__