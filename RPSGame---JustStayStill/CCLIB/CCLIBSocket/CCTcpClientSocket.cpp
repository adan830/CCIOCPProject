/**************************************************************************************
@author: 陈昌
@content: 
**************************************************************************************/
#include "CCTcpClientSocket.h"
#pragma comment(lib, "ws2_32.lib")

const int MAX_CACHE_SIZE    = 16 * 1024;							        // 每个节点的发送区大小

/************************Start Of CIOCPClientSocketManager******************************************/
CIOCPClientSocketManager::CIOCPClientSocketManager() : m_OnRead(nullptr), m_OnError(nullptr), m_OnConnect(nullptr), m_OnDisConnect(nullptr),
	m_LocalAddress(""), m_Address(""), m_Port(), m_TotalBufferlen(0), m_BoActive(false), m_BoConnected(false), m_Sending(false),
	m_hIOCP(0), m_CSocket(INVALID_SOCKET), m_RecvLock(0), m_SendLock(0), m_Reconnect_Interval(10*1000)
{
	memset(&m_SendBlock, 0, sizeof(m_SendBlock));
	memset(&m_RecvBlock, 0, sizeof(m_RecvBlock));
	memset(&m_SocketAddr, 0, sizeof(m_SocketAddr));
	m_SendList.DoInitial(MAX_CACHE_SIZE);
	m_pReceiveBuffer = new CC_UTILS::TBufferStream;
	m_pReceiveBuffer->Initialize();
}

CIOCPClientSocketManager :: ~CIOCPClientSocketManager()
{
	DoClose();
	WaitThreadExecuteOver();
	m_pReceiveBuffer->Finalize();
	delete(m_pReceiveBuffer);
}

int CIOCPClientSocketManager :: SendBuf(const char* pBuf, int iCount)
{
	unsigned int sendLen = 0;
	if ((iCount > 0) && (m_TotalBufferlen < MAX_CLIENT_SEND_BUFFER_SIZE))
	{
		{
		std::lock_guard<std::mutex> guard(m_LockCS); 
		sendLen = iCount;
		m_TotalBufferlen += iCount;
		m_SendList.AddBufferToList(pBuf, iCount);
		}
		if (m_BoConnected && (!m_Sending))
			PrepareSend(0, 0);
	}
	return sendLen;
}

int CIOCPClientSocketManager :: ParseSocketReadData(int iType, const char* pBuf, int iCount)
{
	m_pReceiveBuffer->Write(pBuf, iCount);
	char* pTempBuf = (char*)m_pReceiveBuffer->GetMemPoint();
	int iTempBufLen = m_pReceiveBuffer->GetPosition();
	int iErrorCode = 0;
	int iOffset = 0;
	int iPackageLen = 0;
	PServerSocketHeader pHeader = nullptr;
	while (iTempBufLen - iOffset >= sizeof(TServerSocketHeader))
	{
		pHeader = (PServerSocketHeader)pTempBuf;
		if (SS_SEGMENTATION_SIGN == pHeader->uiSign)
		{
			iPackageLen = sizeof(TServerSocketHeader) + pHeader->usBehindLen;
			//单个数据包超长后扔掉
			if (iPackageLen >= MAXWORD)
			{
				iOffset = iTempBufLen;
				iErrorCode = 1;
				break;
			}
			//加载m_pReceiveBuffer数据时，解析最新的包长度iPackageLen在当前位移iOffset上超出iTempBufLen
			if (iOffset + iPackageLen > iTempBufLen)
				break;
			//处理收到的数据包，子类实现
			ProcessReceiveMsg(pHeader, pTempBuf + sizeof(TServerSocketHeader), pHeader->usBehindLen);
			//移动指针，继续加载socket读入的数据
			iOffset += iPackageLen;
			pTempBuf += iPackageLen;
		}
		else
		{	//向下寻找包头
			iErrorCode = 2;
			iOffset += 1;
			pTempBuf += 1;
		}
	}
	m_pReceiveBuffer->Reset(iOffset);
	return iErrorCode;
}

bool CIOCPClientSocketManager :: DoInitialize()
{
	bool retflag = false;
	if (m_CSocket == INVALID_SOCKET)
	{
		m_CSocket = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		retflag = (m_CSocket != INVALID_SOCKET);
		if (retflag)		
		{
			if (m_LocalAddress.length() > 0)
			{
				SOCKADDR_IN Addr; 
				memset(&Addr, 0, sizeof(Addr));
				Addr.sin_family = PF_INET;
				Addr.sin_port = htons(INADDR_ANY);           
				Addr.sin_addr.s_addr = inet_addr(m_LocalAddress.c_str()); 
				retflag = ((bind(m_CSocket, (sockaddr *)&Addr, sizeof(Addr)) == 0));
			}
		}
		if (!retflag) 
			DoError(seConnect);
	}
	return retflag;
}

bool CIOCPClientSocketManager :: Open()
{
	bool retflag = false;
	if ((!m_BoActive) && (m_Address.length() > 0) && (m_Port > 0))
	{
		if (DoInitialize()) 
		{
			memset(&m_SocketAddr, 0, sizeof(m_SocketAddr));
			m_SocketAddr.sin_family = AF_INET;
			m_SocketAddr.sin_addr.s_addr = inet_addr(m_Address.c_str());
			if (m_SocketAddr.sin_addr.s_addr == u_long(INADDR_NONE))
			{
				PHOSTENT HostEnt = gethostbyname(m_Address.c_str());
				if (HostEnt != nullptr)
					m_SocketAddr.sin_addr.s_addr = ((PIN_ADDR)(HostEnt->h_addr))->s_addr;
			}
			retflag = ((m_SocketAddr.sin_addr.s_addr != u_long(INADDR_NONE)) && (WSAHtons(m_CSocket, m_Port, &(m_SocketAddr.sin_port)) == 0));
			if (retflag) 
				m_BoActive = true;		
		}
	}
	return retflag;
}

//显式地调用Close才清空Send Buffer
void CIOCPClientSocketManager :: Close(bool BoClearBuffer)
{	
	DoClose();
	if (BoClearBuffer)
		Clear();
}

void CIOCPClientSocketManager :: DoExecute()
{
	SetThreadLocale(0X804);
	unsigned int LastconnectTick = 0;
	while (!IsTerminated())  
	{
		try
		{
			if (m_BoConnected)
			{
				DoQueued();
				if (m_CSocket == INVALID_SOCKET)
				{
					m_BoConnected = false;
					if (nullptr != m_OnDisConnect)
						m_OnDisConnect(this);
				}
			}
			else
			{
				if (m_BoActive)
				{
					if (DoConnect())
						continue;
				}
				else
				{
					if (m_Reconnect_Interval > 0)
					{
						unsigned int Tick = GetTickCount();
						if (Tick - LastconnectTick >= m_Reconnect_Interval)
						{
							LastconnectTick = Tick;
							Open();
						}
					}
				}
				WaitForSingleObject(m_Event, 100);
			}
		}
		catch(...)
		{
			SendDebugString("CIOCPClientSocketManager :: Execute Exception");
		}
	}
	Clear();
}

bool CIOCPClientSocketManager :: DoError(TSocketErrorType seType)
{
	bool retflag = false;
	int ErrorCode = WSAGetLastError();
	if ((((seType == seRead) || (seType == seSend)) && (ErrorCode == ERROR_IO_PENDING)) ||
		((seType == seConnect) && (ErrorCode == WSAEWOULDBLOCK)))
	{
		return retflag;
	}

	retflag = true;
	if ((seType == seConnect) && (ErrorCode == WSAECONNREFUSED))
	{
		if (nullptr != m_OnError)
			m_OnError(this, ErrorCode);
	}
	if (seType != seClose)
	{
		DoClose();
	}
	return retflag;
}

bool CIOCPClientSocketManager :: DoConnect()
{
	bool retflag = false;
	if ((m_hIOCP == 0) && (m_CSocket != INVALID_SOCKET))
	{
		//创建完成端口 
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
		if (m_hIOCP > 0)
		{
			//关联完成端口
			if (CreateIoCompletionPort(HANDLE(m_CSocket), m_hIOCP, unsigned int(this), 0) == 0)
			{
				DoClose();
			}
			else
			{
				if (WSAConnect(m_CSocket, (PSOCKADDR)&m_SocketAddr, sizeof(m_SocketAddr), nullptr, nullptr, nullptr, nullptr) == SOCKET_ERROR)
				{
					if (DoError(seConnect))
						return retflag;
				}
				m_BoConnected = true;
				if (nullptr != m_OnConnect)
				{
					m_OnConnect(this);
				}
				if (m_CSocket != INVALID_SOCKET)
				{
					retflag = PrepareRecv();
					if (retflag && (!m_Sending))
					{
						PrepareSend(0, 0);
					}
				}
				else if (nullptr != m_OnDisConnect)
				{
					m_OnDisConnect(this);
				}
			}
		}
	}
	return retflag;
}

void CIOCPClientSocketManager :: DoQueued()
{
	PBlock PRBlock = nullptr;
	unsigned int dwBytesXfered = 0;
	unsigned int dwCompletionKey = 0;
	bool retflag = (GetQueuedCompletionStatus(m_hIOCP, (LPDWORD)&dwBytesXfered, (LPDWORD)&dwCompletionKey, (LPOVERLAPPED*)&PRBlock, INFINITE) > 0);

	if ((IsTerminated()) || ((unsigned int)PRBlock == SHUTDOWN_FLAG))
		return;

	if (dwCompletionKey > 0)
	{
		//客户端断开连接
		if ((!retflag) || (dwBytesXfered == 0))
		{
			DoClose();
			return;
		}
		switch (PRBlock->Event)
		{
			case soWrite:
				PRBlock->Event = soIdle;
				IocpSendback(dwBytesXfered);
				break;
			case soRead:
				PRBlock->Event = soIdle;
				if (!IocpReadback(dwBytesXfered))
					DoClose();
				break;
			default:
				break;
		}
	}
	else
	{
		DoClose();
	}
}

void CIOCPClientSocketManager :: DoClose()
{
	try
	{
		SOCKET sock = m_CSocket;
		m_CSocket = INVALID_SOCKET;
		if (sock != INVALID_SOCKET)
			closesocket(sock);

		HANDLE hIOCP = m_hIOCP;
		m_hIOCP = 0;
		if (hIOCP > 0)
		{
			PostQueuedCompletionStatus(hIOCP, 0, 0, (LPOVERLAPPED)SHUTDOWN_FLAG);
			CloseHandle(hIOCP);
		}
	}
	catch(...)
	{
		DoError(seClose);
	}
	m_BoActive = false;
	m_Sending = false;
}

void CIOCPClientSocketManager :: Clear()
{
	std::lock_guard<std::mutex> guard(m_LockCS); 
	m_SendList.DoFinalize();
}

void CIOCPClientSocketManager :: PrepareSend(int iUntreated, int iTransfered)
{
	std::lock_guard<std::mutex> guard(m_LockCS);
	m_TotalBufferlen -= iTransfered;
	if (m_TotalBufferlen < 0)
		m_TotalBufferlen = 0;
	if (m_CSocket != INVALID_SOCKET)
	{
		iUntreated = m_SendList.GetBufferFromList(m_SendBlock.Buffer, MAX_IOCP_BUFFER_SIZE, iUntreated);

		m_Sending = false;
		if (iUntreated > 0)
		{
			//用户buffer转移到系统buffer中 
			m_SendBlock.Event = soWrite;
			m_SendBlock.wsaBuffer.len = iUntreated;
			m_SendBlock.wsaBuffer.buf = m_SendBlock.Buffer;
			memset(&m_SendBlock.Overlapped, 0, sizeof(m_SendBlock.Overlapped));
			if (WSASend(m_CSocket, &m_SendBlock.wsaBuffer, 1, (LPDWORD)&iTransfered, 0, &m_SendBlock.Overlapped, nullptr) == SOCKET_ERROR)
			{
				if (DoError(seSend))
					return;
			}
			m_Sending = true;
		}
	}
}

bool CIOCPClientSocketManager :: PrepareRecv()
{
	bool retflag = false;
	if (m_CSocket != INVALID_SOCKET)
	{
		m_RecvBlock.Event = soRead;
		m_RecvBlock.wsaBuffer.len = MAX_IOCP_BUFFER_SIZE;
		m_RecvBlock.wsaBuffer.buf = m_RecvBlock.Buffer;
		memset(&m_RecvBlock.Overlapped, 0, sizeof(m_RecvBlock.Overlapped));
		unsigned int Flags = 0;
		unsigned int Transfer = 0;
		retflag = (WSARecv(m_CSocket, &m_RecvBlock.wsaBuffer, 1, (LPDWORD)&Transfer, (LPDWORD)&Flags, &m_RecvBlock.Overlapped, nullptr) != SOCKET_ERROR);
		if (!retflag)
			retflag = (! DoError(seRead));
	}
	return retflag;
}

void CIOCPClientSocketManager :: IocpSendback(int Transfered)
{
	int RemainLen = m_SendBlock.wsaBuffer.len - Transfered;
	if (RemainLen > 0)
		memcpy(&(m_SendBlock.Buffer[0]), &(m_SendBlock.Buffer[Transfered]), RemainLen);
	else
		RemainLen = 0;

	PrepareSend(RemainLen, Transfered);
}

bool CIOCPClientSocketManager :: IocpReadback(int Transfered)
{
	bool retflag = false;
	if (Transfered > 0)
	{
		try
		{
			if (nullptr != m_OnRead)
				m_OnRead(this, m_RecvBlock.Buffer, Transfered);
			//顺利操作后尝试再次投递
			if (m_CSocket != INVALID_SOCKET)
				retflag = PrepareRecv();
		}
		catch(...)
		{
			//捕获异常后也尝试再次投递
			if (m_CSocket != INVALID_SOCKET)
				retflag = PrepareRecv();
		}
	}
	return retflag;
}

/************************End Of CIOCPClientSocketManager********************************************/