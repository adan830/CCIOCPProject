/**************************************************************************************
@author: �²�
@content: ��Ϸ��ʹ�õ�ͨ�ó��������Ͷ��� �Լ���Ϸ�ڵ�һЩͨ�ú�������
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
		//������ַ�
		if (_isupper_l(tempc, nullptr) || _islower_l(tempc, nullptr))
		{
			//��������ַ�
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