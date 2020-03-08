#pragma once

#include <vector>
#include <list>
#include <queue>
#include <functional>

class MPageArrayCursor {
protected:
	int		m_nIndex;
public:
	MPageArrayCursor(int nIndex)	{ m_nIndex = nIndex; }
	virtual ~MPageArrayCursor();
	int GetIndex()					{ return m_nIndex; }
	void SetIndex(int nIndex)		{ m_nIndex = nIndex; }
};


template<class T>
class MPageArray {
public:
	int				m_nPageSize;
	int				m_nMaxIndex;
	std::vector<T>		m_Array;
	std::priority_queue<int, std::vector<int>, std::greater<int>> m_UnusedQueue;

public:
	MPageArray() 
	{
		m_nPageSize = 0;
		SetMaxIndex(-1);
	}
	virtual ~MPageArray()
	{
		m_Array.clear();
	}

	void Reserve(int nPageSize, int nTotalSize)
	{
		m_nPageSize = nPageSize;
		
		int nReserveSize = nTotalSize;;
		int nMod = nTotalSize % nPageSize;
		if (nMod > 0)
			nReserveSize = nTotalSize - nMod + nPageSize;

		m_Array.resize(nReserveSize);
		SetMaxIndex(nTotalSize-1);

		int i;
		for (i=0; i<nTotalSize; i++) {
			m_Array[i] = 0;
			m_UnusedQueue.push(i);
		}
		for(int j=i; j<nReserveSize; j++) {
			m_Array[j] = 0;
		}
	}

	int GetMaxIndex()				{ return m_nMaxIndex; }
	void SetMaxIndex(int nIndex)	{ m_nMaxIndex = nIndex; }
	int GetPageSize()				{ return m_nPageSize; }

	void Set(int i, T pData, MPageArrayCursor* pCursor=NULL)
	{
		_ASSERT(GetMaxIndex() >= i);
		_ASSERT((size_t)i<m_Array.capacity());

		if (pCursor && pCursor->GetIndex()>=0) {
			m_Array[pCursor->GetIndex()] = NULL;
			pCursor->SetIndex(i);
		}

		m_Array[i] = pData;
	}
	T Get(int i)
	{
		return m_Array[i];
	}
	T Get(int nPage, int i)
	{
		int nIndex = GetPageSize()*nPage+i;
		if (nIndex > GetMaxIndex())
			return 0;
		return m_Array[nIndex];
	}

	void Add(T pData, MPageArrayCursor* pCursor=NULL)
	{
		int nFreeSlot = m_UnusedQueue.top();
		m_UnusedQueue.pop();

		Set(nFreeSlot, pData, pCursor);
	}
	void Remove(int i, MPageArrayCursor* pCursor=NULL)
	{
		m_Array[i] = NULL;
		m_UnusedQueue.push(i);

		if (pCursor && pCursor->GetIndex()>=0) {
			pCursor->SetIndex(-1);
		}
	}
	void Remove(T pData) 
	{
		if (pData == NULL) return;

		for (int i=0; i<=GetMaxIndex(); i++) {
			if (pData == m_Array[i]) {
				Remove(i);
				return;
			}
		}
	}
};