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

const int SS_SEGMENTATION_SIGN = 0XFFEEDDCC;                        // 客户端与服务器之间通信协议起始标志

// 客户端与服务器间协议
                               

// 默认的侦听端口
const int DEFAULT_GameGate_PORT = 8300;                             // GameGate     <- CLIENT

//日志等级
const int lmtMessage = 0;
const int lmtWarning = 1;
const int lmtError = 2;
const int lmtException = 3;
const int lmtDebug = 255;


#endif //__CC_GAME_GLOBAL_H__