#pragma once

#include <climits>
#include <map>
#include <vector>
#include "GlobalTypes.h"

/// MAIET Unique ID
struct MUID{
	u32	High{};
	u32	Low{};

	MUID() = default;
	MUID(u32 h, u32 l){
		High = h;
		Low = l;
	}
	explicit MUID(u64 x) {
		High = u32(x >> 32);
		Low = u32(x & 0xFFFF'FFFF);
	}

	void SetZero(){
		High = Low = 0;
	}
	void SetInvalid(){
		SetZero();
	}

	MUID Increase(u32 nSize=1){
		if(Low+nSize>UINT_MAX){
			_ASSERT(High<UINT_MAX);
			Low = nSize-(UINT_MAX-Low);
			High++;
		}
		else{
			Low+=nSize;
		}
		return *this;
	}

	bool IsInvalid() const {
		if(High==Low && Low==0) return true;
		return false;
	}
	bool IsValid() const {
		if(High==Low && Low==0) return false;
		return true;
	}

	inline friend bool operator > (const MUID& a, const MUID& b){
		if(a.High>b.High) return true;
		if(a.High==b.High){
			if(a.Low>b.Low) return true;
		}
		return false;
	}
	inline friend bool operator >= (const MUID& a, const MUID& b){
		if(a.High>b.High) return true;
		if(a.High==b.High){
			if(a.Low>=b.Low) return true;
		}
		return false;
	}
	inline friend bool operator < (const MUID& a, const MUID& b){
		if(a.High<b.High) return true;
		if(a.High==b.High){
			if(a.Low<b.Low) return true;
		}
		return false;
	}
	inline friend bool operator <= (const MUID& a, const MUID& b){
		if(a.High<b.High) return true;
		if(a.High==b.High){
			if(a.Low<=b.Low) return true;
		}
		return false;
	}

	inline MUID& operator=(int v){
		High = 0;
		Low = v;
		return *this;
	}
	inline friend bool operator==(const MUID& a, const MUID& b){
		if(a.High==b.High){
			if(a.Low==b.Low) return true;
		}
		return false;
	}
	inline friend bool operator!=(const MUID& a, const MUID& b){
		if(a.Low!=b.Low) return true;
		if(a.High!=b.High) return true;
		return false;
	}
	inline friend MUID& operator++(MUID& a){
		a.Increase();
		return a;
	}
	u64 AsU64() const {
		return (u64(High) << 32) | Low;
	}

	static MUID Invalid();

	// string representation of MUID
	std::string ToString() const
	{
		std::string low = std::to_string(Low);
		std::string high = std::to_string(High);
		std::string msg = "MUID.low = ";
		std::string HighText = "MUID.high = ";
		std::string space = " ";
		return (msg + low + space + HighText + high);
	}
};

struct MUIDRANGE{
	MUID Start;
	MUID End;

	bool IsEmpty() const {
		return Start == End;
	}
	void Empty(){
		SetZero();
	}
	void SetZero(){
		Start.SetZero();
		End.SetZero();
	}
};

#define MAKEMUID(_high, _low)	MUID(_high, _low)

class MUIDRefMap final : protected std::map<MUID, void*> {
	MUID m_CurrentMUID;
public:
	MUIDRefMap();
	~MUIDRefMap();

	MUID Generate(void* pRef);

	void* GetRef(const MUID& uid);

	void* Remove(const MUID& uid);

	MUIDRANGE Reserve(int nSize);

	MUIDRANGE GetReservedCount();
};

template <typename T>
class MUIDRefCache : public std::map<MUID, T*> {
public:
	using Base = std::map<MUID, T*>;
	using Base::insert;
	using Base::find;
	using Base::end;
	using Base::erase;

	void Insert(const MUID& uid, T* pRef)
	{
#ifdef _DEBUG
		if (GetRef(uid)) {
			_ASSERT(0);
			MLog("MUIDRefCache DUPLICATED Data. \n");
		}
#endif
		insert({uid, pRef});
	}
	T* GetRef(const MUID& uid)
	{
		auto i = find(uid);
		if (i == end()) return nullptr;
		return i->second;
	}
	T* Remove(const MUID& uid)
	{
		auto i = find(uid);
		if (i == end()) return nullptr;
		auto pRef = i->second;
		erase(i);
		return pRef;
	}
};

using MMatchObjectMap = MUIDRefCache<class MMatchObject>;

namespace std
{
	template <>
	class hash<MUID> : public hash<uint64_t>
	{
	public:
		size_t operator()(const MUID &UID) const
		{
			static_assert(sizeof(u64) == sizeof(MUID), "MUID is not 64 bits wide");

			u64 uid64;
			memcpy(&uid64, &UID, sizeof(UID));
			return hash<uint64_t>::operator()(uid64);
		}
	};
}
