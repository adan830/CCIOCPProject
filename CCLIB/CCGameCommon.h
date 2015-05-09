/**************************************************************************************
@author: 陈昌
@content: 游戏内使用的通用常量和类型定义 以及游戏内的一些通用函数处理
**************************************************************************************/
#ifndef __CC_GAME_COMMON_H__
#define __CC_GAME_COMMON_H__

#include <string>

const int MAX_GAMEGATE_COUNT = 24;						  //最大网关数量
/*
ACCOUNT_MAX_LEN   = 50;                                   //帐户最大长度
ACTOR_NAME_MAX_LEN = 32;                                  //角色最大长度
MON_NAME_MAX_LEN  = 32;                                   //怪物名最大长度
MAP_NAME_MAX_LEN  = 20;                                   //地图名最大长度
ITEM_NAME_MAX_LEN = 20;                                   //物品名称最大长度
MAX_QUEST_INFO_LEN = 256;                                 //任务消息中，信息最大长度
SKILL_NAME_MAX_LEN = 20;                                  //技能名最大长度
SCRIPT_FILE_NAME_MAX_LEN = 32;                            //文件名最大长度
SCRIPT_FUN_NAME_MAX_LEN = 32;                             //函数名最大长度
SHOP_NAME_MAX_LEN = 32;                                   //商店名最大长度
POINT_LIST_MAX_LEN = 255;
ORDER_ID_MAX_LEN  = 33;                                   //单号最大长度
OBJECT_NAME_MAX_LEN = 32;                                 //对象名字最大长度
PASSWORD_MAX_LEN  = 33;                                   //密码最大长度
*/
const int ACCOUNT_MAX_LEN = 50;                           //帐户最大长度
const int ACTOR_NAME_MAX_LEN = 32;                        //角色最大长度
const int MON_NAME_MAX_LEN = 32;                          //怪物名最大长度
const int MAP_NAME_MAX_LEN = 20;                          //地图名最大长度
const int SKILL_NAME_MAX_LEN = 20;                        //技能名最大长度
const int ORDER_ID_MAX_LEN = 33;                          //单号最大长度

const int IP_ADDRESS_MAX_LEN = 15;		   				  //ip地址长度
const int SERVER_NAME_MAX_LEN = 50;                       //服务器名最大长度

const int GUILD_OP_MAX_LEN = ACTOR_NAME_MAX_LEN;

//校验登录账号
bool VerifyPassport(const std::string &sPassport); 

#endif //__CC_GAME_COMMON_H__