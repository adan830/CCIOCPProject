/**************************************************************************************
@author: �²�
@content: tcp�������ӵĵײ��-����˶˿ڹ���ͷ�ļ�
**************************************************************************************/
#ifndef __CC_TCP_SERVER_SOCKET_H__
#define __CC_TCP_SERVER_SOCKET_H__

#include <mutex>
#include <list>
#include "CCUtils.h"
#include "CCTcpSocketCommon.h"
#include "CCProtocol_Server.h"

//���ڹ�����ʱ�ͷſͻ��˵�����ṹ
typedef struct _TDelayFreeNode
{
	unsigned short usHandle;
	unsigned int uiAddTick;
	void* pObj;
	_TDelayFreeNode* Next;	
}TDelayFreeNode, *PDelayFreeNode;

/**
*  
* �ͻ������ӵ��������ϵ�ÿһ�����Ӷ���
*
*/
class CClientConnector
{
public:
	CClientConnector();
	virtual ~CClientConnector();
	void Close();
	void SafeClose();
	int SendBuf(const char* pBuf, int iCount);
	int SendText(const std::string& s);
	void IocpSendback(int iTransfered);
	bool IocpReadback(int iTransfered);
	std::string& GetRemoteAddress();
	unsigned short GetSocketHandle(){ return m_SocketHandle; }
protected:
	virtual void Execute(unsigned int uiTick){}
	virtual void SocketRead(const char* pBuf, int iCount){}
	int ParseSocketReadData(int iType, const char* pBuf, int iCount);                       //�����า�ǵ�SocketRead��������
	virtual void ProcessReceiveMsg(char* pHeader, char* pData, int iDataLen){};             //����������Ϣ�����ݣ�����ʵ��
protected:
	CC_UTILS::PBufferStream m_pReceiveBuffer;      // ����socket���ݽ��յ�buffer
private:
	void UpdateActive();
	void Clear();
	void PrepareSend(int iUntreated, int iTransfered);
	bool PrepareRecv();
	void DoActive(unsigned int uiTick);
	bool IsCorpse(unsigned int uiTick, unsigned int uiMaxCorpseTime);
	bool IsBlock(int iMaxBlockSize);
private:
    SOCKET m_Socket;
	std::string m_sRemoteAddress;
	int m_iRemotePort;
	unsigned short m_SocketHandle;      // ��ӦCIOCPServerSocketManager::m_HandleBuckets��handleֵ
	TSendBufferLinkedList m_SendList;   // ���ͻ����б�
	bool m_bSending;
	bool m_bSafeClose;
	TBlock m_SendBlock;
	TBlock m_RecvBlock;
	std::mutex m_LockCS;                // ���в���ʹ�õĻ�����
	TOnSocketError m_OnSocketError;
	int m_iTotalBufferLen;
	unsigned int m_uiActiveTick;
	unsigned int m_uiLastSendTick;
	unsigned int m_uiBufferFullTick;	// �ͻ��˷��ͻ������ѻ������ݶ�ú��ߵ�
friend class CIOCPServerSocketManager;
};

/**
*  
* ������ͻ������ݽ��պͷ��͵��ӹ����߳�---�������̳�,Ҳ�Ͳ����麯���ˣ���������
*
*/
class CSubIOCPWorker : public CExecutableBase
{
public:
	CSubIOCPWorker(PHANDLE ph, TNotifyEvent evt, std::string& sName);
	~CSubIOCPWorker();
	void DoExecute();					
    void ShutDown();
private:			
	//noncopy����
	CSubIOCPWorker();
	CSubIOCPWorker(CSubIOCPWorker& rhs);
	CSubIOCPWorker& operator=(const CSubIOCPWorker& worker);
private:
	PHANDLE m_pHIOCP;
    TNotifyEvent m_OnSocketClose;
};

/**
*  
* ������ά�������߳�---�����̳�---�������̳�,Ҳ�Ͳ����麯���ˣ���������
* ���ڴ���Accept��Ϣ�����ͻ������Ӷ��󣬲������������Ľ��շ��͵�CSubIOCPWorker�����߳�
*
*/
class CMainIOCPWorker : public CExecutableBase
{
public:
	CMainIOCPWorker(void* parent);
	~CMainIOCPWorker();				
	void DoExecute();
private:
    void MakeWorkers();
    void Close();
    bool Start(const std::string& sIP, int iPort);
    
	//noncopy����
	CMainIOCPWorker();
	CMainIOCPWorker(CMainIOCPWorker& rhs);
	CMainIOCPWorker& operator=(const CMainIOCPWorker& worker);
private:
	TOnSocketError m_OnSocketError;
	TNotifyEvent m_OnSocketClose;
	TNotifyEvent m_OnReady;
	TOnSocketAccept m_OnSocketAccept;
	void* m_Parent;
	HANDLE m_hIOCP;
	SOCKET m_Socket;
	int m_iSubThreadCount;
	CSubIOCPWorker** m_SubWorkers;
friend class CIOCPServerSocketManager;
};

/**
*  
* �������˵�Socket����������IOCPģ��ʵ��
* ����Ҫ��������Accept�̣߳�������ͻ��˵����Ӷ����б�
*
*/
class CIOCPServerSocketManager : public CExecutableBase
{
public:
	CIOCPServerSocketManager();
	virtual ~CIOCPServerSocketManager();
	void Open();
	void Close();
	bool IsActive(){ return (m_MainWorker != nullptr); }
    void DoExecute();							 //���಻���ش˷���
	bool DoCheckConnect(const std::string& sRemoteAddress);
	int GetClientCount(){ return m_QueryClientHash.GetItemCount(); }
public:
	std::string m_sLocalIP;					 // ����IP
	int m_iListenPort;	     				 // �����˿�
    TNotifyEvent m_OnConnect;
    TNotifyEvent m_OnDisConnect;  
	TNotifyEvent m_OnListenReady; 
	TOnSocketError m_OnClientError;
	TOnCreateClient m_OnCreateClient;
	TOnCheckAddress m_OnCheckAddress;
	std::list<void*> m_ActiveConnects;		 // ά����ǰ���ӿͻ��˵Ķ����б��ó�Ա����������Ҫ���⿪�ţ������ⲿ������ʹ�ã�����
protected:
	virtual void DoActive(){}         // Open ����Execute�е���
    void* ValueOf(const int iKey);  
	void SetMaxCorpseTime(const int iTime){ m_iMaxCorpseTime = iTime; }
	void SetMaxBlockSize(const int iSize){ m_iMaxBlockSize = iSize; }
protected:
	std::mutex m_LockCS;                     // �ٽ�������ʹ�õĻ���������������������ʹ��
	int m_iDelayFreeHandleCount;			 // ��ǰ������ʱ�ͷŵĿͻ��˾������
private: 
	void DoReady(void* Sender);
	void DoSocketClose(void* Sender);
	void DoSocketAccept(HANDLE hIOCP, SOCKET hSocket, const std::string& sRemoteAddress, int iRemotePort);
	void DoAcceptError(void* Sender, int& iErrorCode);
	void DoClientError(void* Sender, int& iErrorCode);

	bool DelayFreeClient(unsigned int uiTick);
	unsigned short AllocHandle();
	void AddClient(int iHandle, void* pClient);
private:
	CMainIOCPWorker* m_MainWorker;           // ���������󣬸���Accept���͹����շ����߳���
	int m_iMaxCorpseTime;				     //	�ͻ��˺ͷ�������ͨ�ŵ��ά��ʱ��--�������
	int m_iMaxBlockSize;					 // �ͻ��������󣬷�����Ϊ�ÿͻ��˵��������������ֵ--�������
	bool m_bDelayFree;                       // ������ʱ�ͷſͻ������ӵı��
	PDelayFreeNode m_DFNFirst;               // ��ʱ�ͷŵ�ͷ�ڵ�
	PDelayFreeNode m_DFNLast;				 // ��ʱ�ͷŵ�β�ڵ�
	unsigned short m_usNewCreateHandle;		 // ���´�����Handle��ţ�����
	CC_UTILS::TSimpleHash m_QueryClientHash; // ���ڲ�ѯ�ͻ������ӵļ���hash��ֻ��Ŷ���ָ�룬�����𴴽��ͷŶ���
};

#endif //__CC_TCP_SERVER_SOCKET_H__