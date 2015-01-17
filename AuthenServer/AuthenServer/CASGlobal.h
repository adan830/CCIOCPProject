/**************************************************************************************
@author: 陈昌
@content: AuthenServer的常量和变量的  定义
**************************************************************************************/
#ifndef __CC_AUTHEN_SERVER_GLOBAL_H__
#define __CC_AUTHEN_SERVER_GLOBAL_H__

#include "stdafx.h"

using namespace std;

const string MSG_SERVER_BUSY = "非常抱歉！服务器正忙，请稍候再试";
const string MSG_AUTHEN_ERROR = "很抱歉，您的登录认证失败了";
const string MSG_ACCOUNT_DENY = "该账号存在较高被盗风险\n\n请访问官网修改密码后登录";
const string MSG_PWD_EASY = "您的游戏密码过于简单\n\n请访问官网修改密码后登录";

const string MSG_REGISTER_ERROR = "很抱歉，服务器繁忙，\r\n注册失败，请稍候再试";
const string MSG_ACCOUNT_EXIST = "对不起，注册失败，该帐号已经存在";
const string MSG_REGISTER_SECURE_FAILED = "您的注册次数超出限制，\r\n请等待一小时后再试";
const string MSG_LOGIN_SECURE_FAILED = "您的账号登录错误次数超出限制，\r\n请等待一小时后再试";
const string MSG_DENY_CHILD = "您已受到防沉迷限制\n请离线满5小时后重新登录";
const string MSG_EMAIL_EXIST = "注册失败，这个邮箱已经被注册过了";   // 取消，不可能出现了
const string MSG_SAFECARD_AUTHEN_ERROR = "很抱歉，您的密保卡认证失败了";
const string MSG_SAFECARD_SECURE_FAILED = "您的密保卡认证次数超出限制，\r\n请等待一小时后再试";

#endif //__CC_AUTHEN_SERVER_GLOBAL_H__