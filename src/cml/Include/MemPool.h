#pragma once

#include "MDebug.h"
#include "assert.h"
#include <mutex>

#define InitMemPool(T)
#define UninitMemPool(T)
#define ReleaseMemPool(T)	CMemPool<T>::Release();

template< typename T >
class CMemPool
{
protected:

protected:
	static T*	m_list;
	T*			m_next;

	static std::mutex Mutex;

public:
	static void	Release();

public:
	static void* operator new( size_t size_ );
	static void  operator delete( void* deadObject_, size_t size_ );
public:
};

// new
template<typename T>
void* CMemPool<T>::operator new( size_t size_ )
{
	T* instance;

	std::lock_guard<std::mutex> lock(Mutex);

	if( m_list != NULL )
	{
		instance	= m_list;
		m_list	= m_list->m_next;
	}
	else
	{
		instance = (T*)::operator new(size_);
	}

#ifdef _DEBUG
	if(size_ != sizeof(*instance))
		assert(0);
#endif

	return instance;
}

// delete
template<typename T>
void CMemPool<T>::operator delete( void* deadObject_, size_t size_ )
{
	std::lock_guard<std::mutex> lock(Mutex);

	((T*)deadObject_)->m_next	= m_list;
	m_list	= (T*)deadObject_;
}

template<typename T>
void CMemPool<T>::Release()
{
	if( NULL != m_list ) 
	{
		std::lock_guard<std::mutex> lock(Mutex);

		T* pInstace		= m_list;
		while( pInstace != NULL )
		{
			pInstace	= m_list->m_next;
			::operator delete( m_list );
			m_list	= pInstace;
		}
	}
}

template<typename T> std::mutex CMemPool<T>::Mutex;
template<typename T> T* CMemPool<T>::m_list;

template < typename T >
class CMemPoolSm
{
protected:
	static T*	m_list;
	T*			m_next;

public:
	static void* operator new( size_t size_ );
	static void  operator delete( void* deadObject_, size_t size_ );

	static void Release();

public:
	CMemPoolSm() {};
	~CMemPoolSm() {};
};


template<typename T>
void* CMemPoolSm<T>::operator new( size_t size_ )
{
	T* instance;

	if( m_list != NULL ) {
		instance = m_list;
		m_list = m_list->m_next;
	} else {
		instance = (T*)::operator new(size_);
	}

#ifdef _DEBUG
	if(size_ != sizeof(*instance))
		assert(0);
#endif

	return instance;
}

template<typename T>
void CMemPoolSm<T>::operator delete( void* deadObject_, size_t size_ )
{
	((T*)deadObject_)->m_next = m_list;
	m_list	= (T*)deadObject_;
}

template<typename T>
void CMemPoolSm<T>::Release()
{
	T* pInstace	= m_list;
	while( pInstace != NULL ) {
		pInstace = m_list->m_next;
		::operator delete( m_list );
		m_list = pInstace;
	}
}

template<typename T> T* CMemPoolSm<T>::m_list;