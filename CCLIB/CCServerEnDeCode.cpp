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
		/*
procedure initialize_Code();
var
  i, enDivisor, deDivisor: integer;
  wEnLen, wDeLen    : Word;
  Pen, pde          : PAnsiChar;
begin
  enDivisor := 0;
  deDivisor := 0;
  wEnLen := integer(@_EnCode_End_) - integer(@EnCode);
  wDeLen := integer(@_DeCode_End_) - integer(@DeCode);
  Pen := @EnCode;
  for i := 0 to wEnLen - 1 do
  begin
    if PByte(Pen)^ = ENCODE_DIVISOR then
    begin
      enDivisor := i;
      Break;
    end;
    inc(Pen);
  end;

  pde := @DeCode;
  for i := 0 to wDeLen - 1 do
  begin
    if PByte(pde)^ = ENCODE_DIVISOR then
    begin
      deDivisor := i;
      Break;
    end;
    inc(pde);
  end;
  Pen := @EnCode;
  pde := @DeCode;

  for i := Low(CodeBuffer) to High(CodeBuffer) do
    with CodeBuffer[i] do
    begin
      wEnBufferLen := wEnLen;
      wDeBufferLen := wDeLen;
      pEnBuffer := VirtualAlloc(nil, wEnBufferLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
      pDeBuffer := VirtualAlloc(nil, wDeBufferLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
      Move(Pen^, pEnBuffer^, wEnBufferLen);
      Move(pde^, pDeBuffer^, wDeBufferLen);
      pEnBuffer[enDivisor] := Char(i);                      // 加密因子 (ENCODE_DIVISOR)的位置 需要反编译找出位置
      pDeBuffer[deDivisor] := Char(i);                      // 解密因子
    end;
end;
		*/
	}

	void Finalize_Code()
	{
		/*
procedure finalize_Code();
var
  i                 : integer;
begin
  for i := Low(CodeBuffer) to High(CodeBuffer) do
    with CodeBuffer[i] do
    begin
      VirtualFree(pEnBuffer, wEnBufferLen, MEM_DECOMMIT);
      VirtualFree(pDeBuffer, wDeBufferLen, MEM_DECOMMIT);
    end;
end;
		*/
	}
}