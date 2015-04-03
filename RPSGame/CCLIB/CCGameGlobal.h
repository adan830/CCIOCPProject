/**************************************************************************************
@author: 陈昌
@content: 游戏的的常量和结构定义
**************************************************************************************/

#ifndef __CC_GAME_GLOBAL_H__
#define __CC_GAME_GLOBAL_H__

// 通讯的协议头
typedef struct _TServerSocketHeader
{
	unsigned int uiSign;				// 分隔符 SS_SEGMENTATION_SIGN
	int iParam;							// 扩展参数
	unsigned short usIdent;				// 协议号
	unsigned short usBehindLen;			// 后续数据长度
}TServerSocketHeader, *PServerSocketHeader;

//服务器返回给客户端的每轮比较后的信息
typedef struct _TGamePlayAckPkg
{
	int iRoundCount;
	int iClientChoose;
	int iServerChoose;
	int iConclusion;
	int iTotalWins;
	int iTotalLooses;
	int iTotalTies;
}TGamePlayAckPkg, *PGamePlayerAckPkg;

const int SS_SEGMENTATION_SIGN = 0XFFEEDDCC;                        // 客户端与服务器之间通信协议起始标志

//游戏客户端发送给服务器的消息
const int CM_PING = 10;										 // 心跳
const int CM_PLAY_REQ = 11;								     // 发送游戏请求，客户端出一个
const int CM_QUIT = 20;		    							 // 正常退出


//服务器端发给游戏客户端的消息
const int SCM_PING = 100;									 // 心跳
const int SCM_PLAY_ACK = 11;								 // 游戏响应，返回客户端和服务器出的，以及当前结果和统计结果
const int SCM_QUIT = 101;                                    // 退出

// 服务器的侦听端口
const int DEFAULT_LISTENING_PORT = 8300;                        

//日志等级
const int lmtMessage = 0;
const int lmtWarning = 1;
const int lmtError = 2;
const int lmtException = 3;
const int lmtDebug = 255;

//游戏参数
enum RPS_ENUM
{
	rps_rock,
	rps_paper,
    rps_scissors
};
const std::string G_RPS_STRING[3] = { "Rock", "Paper", "Scissors" };

const std::string G_RPS_CONCLUSION[3] = { "TIE", "WIN", "LOOSE" };


#endif //__CC_GAME_GLOBAL_H__