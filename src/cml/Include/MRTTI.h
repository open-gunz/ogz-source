#ifndef _MRTTI_H
#define _MRTTI_H

#include <cstddef>
#include <string>
using namespace std;

class MRTTI
{
public:
	MRTTI(const char* szName, const MRTTI* pBaseRTTI)
	{
		m_szName = std::string(szName);
		m_pBaseRTTI = pBaseRTTI;
	}

	const MRTTI* GetBaseRTTI() const
	{
		return m_pBaseRTTI;
	}

	const char* GetName() const
	{
		return m_szName.c_str();
	}

private:
	const MRTTI* m_pBaseRTTI;
	std::string m_szName;
};

// insert in Object class declaration
#define MDeclareRootRTTI \
public: \
	static const MRTTI m_stRTTI; \
	virtual const MRTTI* GetRTTI() const; \
	bool IsExactlyClass (const MRTTI* pQueryRTTI) const; \
	bool IsDerivedFromClass(const MRTTI* pQueryRTTI) const; \
	void* DynamicCast(const MRTTI* pQueryRTTI) const

// insert in source file of Object class
#define MImplementRootRTTI(classname) \
const MRTTI classname::m_stRTTI(#classname, NULL); \
const MRTTI* classname::GetRTTI() const \
{ \
	return &m_stRTTI; \
} \
\
bool classname::IsExactlyClass(const MRTTI* pQueryRTTI) const \
{ \
	return (GetRTTI() == pQueryRTTI); \
} \
\
bool classname::IsDerivedFromClass(const MRTTI* pQueryRTTI) const \
{ \
	const MRTTI* pRTTI = GetRTTI(); \
	while (pRTTI) \
	{ \
		if (pRTTI == pQueryRTTI) \
			return true; \
		pRTTI = pRTTI->GetBaseRTTI(); \
	} \
	return false; \
} \
\
void* classname::DynamicCast(const MRTTI* pQueryRTTI) const \
{ \
	return (void*)(IsDerivedFromClass(pQueryRTTI) ? this : NULL); \
}

// insert in derived class declaration
#define MDeclareRTTI \
public: \
	static const MRTTI m_stRTTI; \
	virtual const MRTTI* GetRTTI() const override { return &m_stRTTI; }

// insert in source file of derived class of inheritance tree
#define MImplementRTTI(classname, baseclassname) \
const MRTTI classname::m_stRTTI(#classname, &baseclassname::m_stRTTI)

// runtime type testing and casting
#define MIsExactlyClass(classname, pObj) \
( (pObj) ? (pObj)->IsExactlyClass(&classname::m_stRTTI) : false )

#define MIsDerivedFromClass(classname, pObj) \
( (pObj) ? (pObj)->IsDerivedFromClass(&classname::m_stRTTI) : false )

#define MStaticCast(classname, pObj) \
((classname*)(void*)(pObj))

#define MDynamicCast(classname, pObj) \
( (pObj) ? (classname*)(pObj)->DynamicCast(&classname::m_stRTTI) : NULL)


#endif