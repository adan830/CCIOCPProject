/**************************************************************************************
@author: �²�
@content: ��Ϸ��ʹ�õ�ͨ�ó��������Ͷ��� �Լ���Ϸ�ڵ�һЩͨ�ú�������
**************************************************************************************/
#ifndef __CC_GAME_COMMON_H__
#define __CC_GAME_COMMON_H__

#include <string>

const int MAX_GAMEGATE_COUNT = 24;						  //�����������
/*
ACCOUNT_MAX_LEN   = 50;                                   //�ʻ���󳤶�
ACTOR_NAME_MAX_LEN = 32;                                  //��ɫ��󳤶�
MON_NAME_MAX_LEN  = 32;                                   //��������󳤶�
MAP_NAME_MAX_LEN  = 20;                                   //��ͼ����󳤶�
ITEM_NAME_MAX_LEN = 20;                                   //��Ʒ������󳤶�
MAX_QUEST_INFO_LEN = 256;                                 //������Ϣ�У���Ϣ��󳤶�
SKILL_NAME_MAX_LEN = 20;                                  //��������󳤶�
SCRIPT_FILE_NAME_MAX_LEN = 32;                            //�ļ�����󳤶�
SCRIPT_FUN_NAME_MAX_LEN = 32;                             //��������󳤶�
SHOP_NAME_MAX_LEN = 32;                                   //�̵�����󳤶�
POINT_LIST_MAX_LEN = 255;
ORDER_ID_MAX_LEN  = 33;                                   //������󳤶�
OBJECT_NAME_MAX_LEN = 32;                                 //����������󳤶�
PASSWORD_MAX_LEN  = 33;                                   //������󳤶�
*/
const int ACCOUNT_MAX_LEN = 50;                           //�ʻ���󳤶�
const int ACTOR_NAME_MAX_LEN = 32;                        //��ɫ��󳤶�
const int MON_NAME_MAX_LEN = 32;                          //��������󳤶�
const int MAP_NAME_MAX_LEN = 20;                          //��ͼ����󳤶�
const int SKILL_NAME_MAX_LEN = 20;                        //��������󳤶�
const int ORDER_ID_MAX_LEN = 33;                          //������󳤶�

const int IP_ADDRESS_MAX_LEN = 15;		   				  //ip��ַ����
const int SERVER_NAME_MAX_LEN = 50;                       //����������󳤶�

const int GUILD_OP_MAX_LEN = ACTOR_NAME_MAX_LEN;

//У���¼�˺�
bool VerifyPassport(const std::string &sPassport); 

#endif //__CC_GAME_COMMON_H__