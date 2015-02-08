/**************************************************************************************
@author: 陈昌
@content: 游戏内使用的通用常量和类型定义 以及游戏内的一些通用函数处理
**************************************************************************************/
#include "CCGameCommon.h"

const int MIN_PASSPORT_LENGTH = 4;
const int MAX_PASSPORT_LENGTH = 16;

bool VerifyPassport(const std::string &sPassport)
{
	bool bRetFlag = false;
	int len = sPassport.length();
	if ((len >= MIN_PASSPORT_LENGTH) && (len <= MAX_PASSPORT_LENGTH))
	{
		char tempc = sPassport[0];
		//检查首字符
		if (_isupper_l(tempc, nullptr) || _islower_l(tempc, nullptr))
		{
			//检查其它字符
			for (int i = 1; i < len; i++)
			{
				if (!(_isupper_l(tempc, nullptr) || _islower_l(tempc, nullptr) || _isdigit_l(tempc, nullptr)))
					return bRetFlag;
			}			
			bRetFlag = true;
		}
	}
	return bRetFlag;
}