#pragma once

inline void cprint(...) {}

static constexpr int RG_SLASH_RADIUS = 320;
static constexpr int RG_MASSIVE_RADIUS = 280;

inline int RelativeWidth(int x)
{
	return int(float(x) / 1920 * RGetScreenWidth());
}

inline int RelativeHeight(int y)
{
	return int(float(y) / 1080 * RGetScreenHeight());
}

inline int RELWIDTH(int x) { return RelativeWidth(x); }
inline int RELHEIGHT(int y) { return RelativeHeight(y); }

inline unsigned long long QPC()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}

inline unsigned long long QPF()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	return li.QuadPart;
}

template <typename T>
inline void SafeDestroy(T& obj)
{
	obj.Destroy();
}

template <typename T>
inline void SafeDestroy(T* obj)
{
	if (obj)
		obj->Destroy();
}

template <typename T>
inline void SAFE_DESTROY(T *obj) { SafeDestroy(obj); }

class EOFException : public std::runtime_error
{
public:
	EOFException(int a) : std::runtime_error("End of file was reached"), Position(a) { }
	int GetPosition() const {
		return Position;
	}
private:
	int Position;
};

#define STACK_ALLOC(sa_type, sa_num) static_cast<std::add_pointer<sa_type>::type>((sa_num) * sizeof(sa_type) > 5012 ? new char[sa_num * sizeof(sa_type)] : alloca(sa_num * sizeof(sa_type)))
#define MAKE_STACK_ARRAY(msa_type, msa_num, ...) StackArray<msa_type>(STACK_ALLOC(msa_type, msa_num), msa_num, (msa_num) * sizeof(msa_type) > 5012, __VA_ARGS__)

template <typename T>
class StackArray
{
public:
	template <typename... ArgsType>
	StackArray(T* p, size_t s, bool h, ArgsType&&... Args) : ptr(p), Size(s), Heap(h)
	{
		for (size_t i = 0; i < s; i++)
		{
			new (p + i) T(std::forward<ArgsType>(Args)...);
		}
	}

	template <typename T_fn>
	StackArray(T* p, size_t s, bool h, T_fn fn) : ptr(p), Size(s), Heap(h)
	{
		for (size_t i = 0; i < s; i++)
		{
			fn(p + i, i);
		}
	}

	T& operator[](size_t Index)
	{
		return ptr[Index];
	}

	T* get()
	{
		return ptr;
	}

	size_t size()
	{
		return Size;
	}

	~StackArray()
	{
		for (size_t i = 0; i < Size; i++)
		{
			ptr[i].~T();
		}

		if (Heap)
			delete[] ptr;
	}

private:
	T* ptr;
	size_t Size;
	bool Heap;
};
