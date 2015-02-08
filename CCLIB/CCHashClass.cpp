/**************************************************************************************
@author: �²�
@content: �Լ�ʹ�õ�Hash��
**************************************************************************************/

#include "CCHashClass.h"

namespace CC_UTILS{

/************************Start Of CIntegerHash**************************************************/

	CIntegerHash::CIntegerHash(unsigned int uiSize) :m_RemoveEvent(nullptr), m_iTotalCount(0), m_uiBucketSize(uiSize), m_pFirstListNode(nullptr),
		m_pLastListNode(nullptr), m_pCurrentQueueNode(nullptr)
	{
		m_TopBuckets = new PIntHashItem[uiSize];
	}	

	CIntegerHash::~CIntegerHash()
	{
		Clear();
		delete[] m_TopBuckets;
	}

	bool CIntegerHash::Add(const int iKey, void* pValue)
	{
		bool retFlag = false;
		PIntHashItem pBucket = *(Find(iKey));
		if (nullptr == pBucket)
		{
			unsigned int uiHash = HashOf(iKey) % m_uiBucketSize;
			pBucket = new TIntHashItem;
			pBucket->Key = iKey;
			pBucket->Value = pValue;
			pBucket->BPrev = nullptr;
			pBucket->BNext = m_TopBuckets[uiHash];
			pBucket->LPrev = m_pLastListNode;
			pBucket->LNext = nullptr;

			if (pBucket->BNext != nullptr)
				pBucket->BNext->BPrev = pBucket;
			if (pBucket->LPrev != nullptr)
				pBucket->LPrev->LNext = pBucket;
			else
				m_pFirstListNode = pBucket;

			m_TopBuckets[uiHash] = pBucket;
			m_pLastListNode = pBucket;
			++m_iTotalCount;
			retFlag = true;
		}
		return retFlag;
	}

	void CIntegerHash::Clear()
	{
		PIntHashItem pCurr, pNext, pLast;
		pCurr = m_pFirstListNode;
		while (pCurr != nullptr)
		{
			pNext = pCurr->LNext;
			pLast = pCurr;
			pCurr = pNext;

			if (m_RemoveEvent != nullptr)
				m_RemoveEvent(pLast->Value, pLast->Key);
			delete pLast;
		}
		m_pFirstListNode = nullptr;
		m_pLastListNode = nullptr;
		m_pCurrentQueueNode = nullptr;
		for (int i = 0; i < (int)m_uiBucketSize; i++)
			m_TopBuckets[i] = nullptr;
		m_iTotalCount = 0;
	}

	void CIntegerHash::DoRemoveItem(PIntHashItem pItem)
	{
		if (pItem != nullptr)
		{
			if (pItem->BNext != nullptr)
				pItem->BNext->BPrev = pItem->BPrev;

			if (pItem->BPrev != nullptr)
				pItem->BPrev->BNext = pItem->BNext;
			else
			{
				unsigned int uiHash = HashOf(pItem->Key) % m_uiBucketSize;
				m_TopBuckets[uiHash] = pItem->BNext;
			}

			if (pItem->LNext != nullptr)
				pItem->LNext->LPrev = pItem->LPrev;
			else
				m_pLastListNode = pItem->LPrev;

			if (pItem->LPrev != nullptr)
				pItem->LPrev->LNext = pItem->LNext;
			else
				m_pFirstListNode = pItem->LNext;

			try
			{
				if (m_RemoveEvent != nullptr)
					m_RemoveEvent(pItem->Value, pItem->Key);
			}
			catch (...)
			{
				//����remove�������쳣
			}

			if (pItem == m_pCurrentQueueNode)
				m_pCurrentQueueNode = pItem->LNext;

			delete pItem;
			--m_iTotalCount;
		}
	}

	bool CIntegerHash::Remove(const int iKey)
	{
		bool retFlag = false;
		unsigned int uiHash = HashOf(iKey) % m_uiBucketSize;
		PIntHashItem pCurr = m_TopBuckets[uiHash];
		while (pCurr != nullptr)
		{
			if (iKey == pCurr->Key)
			{
				DoRemoveItem(pCurr);
				retFlag = true;
				break;
			}
			pCurr = pCurr->BNext;
		}
		return retFlag;
	}

	void* CIntegerHash::ValueOf(const int iKey)
	{
		PIntHashItem pItem = *(Find(iKey));
		if (pItem != nullptr)
			return pItem->Value;
		else
			return nullptr;
	}

	//func����true��ʱ�򣬴�hash��ɾ���ý��
	int CIntegerHash::Touch(TTouchFunc func, unsigned int uiParam)
	{
		int iRetCode = 0;
		if (func != nullptr)
		{
			PIntHashItem pCurr = m_pFirstListNode;
			PIntHashItem pNext = nullptr;
			while (pCurr != nullptr)
			{
				pNext = pCurr->LNext;
				if (func(pCurr->Value, uiParam, iRetCode))
					DoRemoveItem(pCurr);
				pCurr = pNext;
			}
		}
		return iRetCode;		
	}

	void CIntegerHash::First()
	{
		m_pCurrentQueueNode = m_pFirstListNode;
	}

	bool CIntegerHash::Eof()
	{
		return (m_pCurrentQueueNode == nullptr);
	}

	void* CIntegerHash::GetNextNode()
	{
		if (nullptr == m_pCurrentQueueNode)
			First();
		if (m_pCurrentQueueNode != nullptr)
		{
			void* p = m_pCurrentQueueNode->Value;
			m_pCurrentQueueNode = m_pCurrentQueueNode->LNext;
			return p;
		}
		else
			return nullptr;
	}

	unsigned int CIntegerHash::HashOf(const int iKey)
	{
		return iKey;
	}

	PPIntHashItem CIntegerHash::Find(const int iKey)
	{
		unsigned int uiHash = HashOf(iKey) % m_uiBucketSize;
		PPIntHashItem ppItem = &m_TopBuckets[uiHash];
		while (*ppItem != nullptr)
		{
			if (iKey == (*ppItem)->Key)
				break;
			else
				ppItem = &((*ppItem)->BNext);
		}
		return ppItem;
	}

/************************End Of CIntegerHash****************************************************/



/************************Start Of CStringHash**************************************************/

	CStringHash::CStringHash(unsigned int uiSize) : m_RemoveEvent(nullptr), m_iTotalCount(0), m_uiBucketSize(uiSize), m_pFirstListNode(nullptr),
		m_pLastListNode(nullptr), m_pCurrentQueueNode(nullptr)
	{
		m_TopBuckets = new PStrHashItem[uiSize];
	}

	CStringHash::~CStringHash()
	{
		Clear();
		delete[] m_TopBuckets;
	}

	bool CStringHash::Add(const std::string &sKey, void* pValue)
	{
		bool retFlag = false;
		PStrHashItem pBucket = *(Find(sKey));
		if (nullptr == pBucket)
		{
			unsigned int uiHash = HashOf(sKey) % m_uiBucketSize;
			pBucket = new TStrHashItem;
			pBucket->Key = sKey;
			pBucket->Value = pValue;
			pBucket->BPrev = nullptr;
			pBucket->BNext = m_TopBuckets[uiHash];
			pBucket->LPrev = m_pLastListNode;
			pBucket->LNext = nullptr;

			if (pBucket->BNext != nullptr)
				pBucket->BNext->BPrev = pBucket;
			if (pBucket->LPrev != nullptr)
				pBucket->LPrev->LNext = pBucket;
			else
				m_pFirstListNode = pBucket;

			m_TopBuckets[uiHash] = pBucket;
			m_pLastListNode = pBucket;
			++m_iTotalCount;
			retFlag = true;
		}
		return retFlag;
	}

	void CStringHash::Clear()
	{
		PStrHashItem pCurr, pNext, pLast;
		pCurr = m_pFirstListNode;
		while (pCurr != nullptr)
		{
			pNext = pCurr->LNext;
			pLast = pCurr;
			pCurr = pNext;

			if (m_RemoveEvent != nullptr)
				m_RemoveEvent(pLast->Value, pLast->Key);
			delete pLast;
		}
		m_pFirstListNode = nullptr;
		m_pLastListNode = nullptr;
		m_pCurrentQueueNode = nullptr;
		for (int i = 0; i < (int)m_uiBucketSize; i++)
			m_TopBuckets[i] = nullptr;
		m_iTotalCount = 0;
	}

	void CStringHash::DoRemoveItem(PStrHashItem pItem)
	{
		if (pItem != nullptr)
		{
			if (pItem->BNext != nullptr)
				pItem->BNext->BPrev = pItem->BPrev;

			if (pItem->BPrev != nullptr)
				pItem->BPrev->BNext = pItem->BNext;
			else
			{
				unsigned int uiHash = HashOf(pItem->Key) % m_uiBucketSize;
				m_TopBuckets[uiHash] = pItem->BNext;
			}

			if (pItem->LNext != nullptr)
				pItem->LNext->LPrev = pItem->LPrev;
			else
				m_pLastListNode = pItem->LPrev;

			if (pItem->LPrev != nullptr)
				pItem->LPrev->LNext = pItem->LNext;
			else
				m_pFirstListNode = pItem->LNext;

			try
			{
				if (m_RemoveEvent != nullptr)
					m_RemoveEvent(pItem->Value, pItem->Key);
			}
			catch (...)
			{
				//����remove�������쳣
			}

			if (pItem == m_pCurrentQueueNode)
				m_pCurrentQueueNode = pItem->LNext;

			delete pItem;
			--m_iTotalCount;
		}
	}

	bool CStringHash::Remove(const std::string &sKey)
	{
		bool retFlag = false;
		unsigned int uiHash = HashOf(sKey) % m_uiBucketSize;
		PStrHashItem pCurr = m_TopBuckets[uiHash];
		while (pCurr != nullptr)
		{
			if (0 == sKey.compare(pCurr->Key))
			{
				DoRemoveItem(pCurr);
				retFlag = true;
				break;
			}
			pCurr = pCurr->BNext;
		}
		return retFlag;
	}

	void* CStringHash::ValueOf(const std::string &sKey)
	{
		PStrHashItem pItem = *(Find(sKey));
		if (pItem != nullptr)
			return pItem->Value;
		else
			return nullptr;
	}

	int CStringHash::Touch(TTouchFunc func, unsigned int uiParam)
	{
		int iRetCode = 0;
		if (func != nullptr)
		{
			PStrHashItem pCurr = m_pFirstListNode;
			PStrHashItem pNext = nullptr;
			while (pCurr != nullptr)
			{
				pNext = pCurr->LNext;
				if (func(pCurr->Value, uiParam, iRetCode))
					DoRemoveItem(pCurr);
				pCurr = pNext;
			}
		}
		return iRetCode;
	}

	void CStringHash::First()
	{
		m_pCurrentQueueNode = m_pFirstListNode;
	}

	bool CStringHash::Eof()
	{
		return (m_pCurrentQueueNode == nullptr);
	}

	void* CStringHash::GetNextNode()
	{
		if (nullptr == m_pCurrentQueueNode)
			First();
		if (m_pCurrentQueueNode != nullptr)
		{
			void* p = m_pCurrentQueueNode->Value;
			m_pCurrentQueueNode = m_pCurrentQueueNode->LNext;
			return p;
		}
		else
			return nullptr;
	}

	//CStringHash��key�����ִ�Сд
	unsigned int CStringHash::HashOf(const std::string &sKey)
	{
		unsigned int uiRetCode = 0;
		bool bHead = false;
		bool bTail = false;
		unsigned char key;
		for (int i = 0; i < sKey.length(); i++)
		{
			key = (unsigned char)sKey[i];

			//�жϵ�˫�ֽ�
			//����ANSI�ַ���128��, ����, ANSI�ַ���bit���λΪ0, ��bit���λΪ1ʱ, 
			//�ͱ�ʾ�Ǹ�˫�ֽ��ַ��ˡ���char��Ҳ����signed char��������ǡ�������λ����������ֱ������char < 0 ���ж��Ƿ���˫�ֽ��ַ��ˡ�
			if (!bHead)
			{
				bTail = false;
				if (sKey[i] < 0)
					bHead = true;
			}
			else
			{
				bTail = true;
				bHead = false;
			}
			//���ֽڵĴ�д��ĸ��ת��Сд
			if ((!bHead) && (!bTail) && (key >= 0x41) && (key <= 0x5A))
				key = key | 0x20;

			uiRetCode = ((uiRetCode << 2) | (uiRetCode >> (sizeof(uiRetCode)* 8 - 2))) ^ key;
		}
		return uiRetCode;
	}

	PPStrHashItem CStringHash::Find(const std::string &sKey)
	{
		unsigned int uiHash = HashOf(sKey) % m_uiBucketSize;
		PPStrHashItem ppItem = &m_TopBuckets[uiHash];
		while (*ppItem != nullptr)
		{
			if (0 == sKey.compare((*ppItem)->Key))
				break;
			else
				ppItem = &((*ppItem)->BNext);
		}
		return ppItem;
	}

/************************End Of CStringHash****************************************************/

}