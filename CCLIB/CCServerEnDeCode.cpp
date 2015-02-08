/**************************************************************************************
@author: 陈昌
@content:
**************************************************************************************/

#include "CCServerEnDeCode.h"
#include <Windows.h>

namespace CC_UTILS{

	TEnDeRecord CodeBuffer[MAX_BUFFER_TYPE];

	PEnDeRecord GetCode(BYTE Idx)
	{
		if (0 == Idx)
			Idx = rand() % (MAX_BUFFER_TYPE - 1) + 1;
		return &CodeBuffer[Idx];
	}

	std::string MemToHex(PBYTE pb, unsigned short len)
	{
		return "";
		/*
function MemToHex(pb: PByte; len: word): ansistring;
const
  HexChar           : array[0..15] of Char = (
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F');
var
  b                 : Byte;
  pc                : PansiChar;
  i                 : integer;
begin
  SetLength(Result, len * 3);
  pc := PAnsiChar(Result);
  for i := 1 to len do
  begin
    b := pb^;
    pc^ := HexChar[(b shr 4)];
    inc(pc);
    pc^ := HexChar[b and $F];
    inc(pc);
    Pc^ := #32;
    inc(pc);
    inc(pb);
  end;
end;
		*/
	}

	void EnCode(PBYTE pb, unsigned short len)
	{
		BYTE a, b, c, d;
		a = ENCODE_DIVISOR;
		for (int i = 0; i < len; i++)
		{
			b = *pb ^ a;
			c = b >> 3;
			d = (b & 7) << 5;
			*pb = c | d;
			a = *pb;
			++pb;
		}
	}

	void _EnCode_End_(){}

	void DeCode(PBYTE pb, unsigned short len)
	{
		BYTE a, b, c, d;
		a = ENCODE_DIVISOR;
		for (int i = 0; i < len; i++)
		{
			b = *pb;
			c = b >> 5;
			d = b << 3;
			*pb = (c | d) ^ a;
			a = b;
			++pb;
		}
	}

	void _DeCode_End_(){}

	void Initialize_Code()
	{
		int enDivisor = 0;
		int deDivisor = 0;
		unsigned short usEnLen = int(&_EnCode_End_) - int(&EnCode);
		unsigned short usDeLen = int(&_DeCode_End_) - int(&DeCode);

		char* pEn = (char*)&EnCode;
		for (int i = 0; i < usEnLen; i++)
		{
			if (ENCODE_DIVISOR == *(pEn))
			{
				enDivisor = i;
				break;
			}
			++pEn;
		}

		char* pDe = (char*)&DeCode;
		for (int i = 0; i < usDeLen; i++)
		{
			if (ENCODE_DIVISOR == *(pDe))
			{
				deDivisor = i;
				break;
			}
			++pDe;
		}

		pEn = (char*)&EnCode;
		pDe = (char*)&DeCode;
		PEnDeRecord pBuf = nullptr;
		for (int i = 0; i < MAX_BUFFER_TYPE; i++)
		{
			pBuf = &CodeBuffer[i];
			pBuf->usEnBufferLen = usEnLen;
			pBuf->usDeBufferLen = usDeLen;
			pBuf->pEnBuffer = (char*)VirtualAlloc(nullptr, usEnLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			pBuf->pDeBuffer = (char*)VirtualAlloc(nullptr, usDeLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			memcpy(pBuf->pEnBuffer, pEn, usEnLen);
			memcpy(pBuf->pDeBuffer, pDe, usDeLen);
			pBuf->pEnBuffer[enDivisor] = (char)i;			// 加密因子 (ENCODE_DIVISOR)的位置 需要反编译找出位置
			pBuf->pDeBuffer[deDivisor] = (char)i;			// 解密因子
		}
	}

	void Finalize_Code()
	{
		for (int i = 0; i < MAX_BUFFER_TYPE; i++)
		{
			VirtualFree(CodeBuffer[i].pEnBuffer, CodeBuffer[i].usEnBufferLen, MEM_DECOMMIT);
			VirtualFree(CodeBuffer[i].pDeBuffer, CodeBuffer[i].usDeBufferLen, MEM_DECOMMIT);
		}
	}
}