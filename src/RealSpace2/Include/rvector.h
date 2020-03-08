#pragma once

struct v2
{
	float x, y;

	static constexpr auto size = 2;

	v2() = default;
	v2(float x, float y) : x{ x }, y{ y } {}

	operator float* () { return reinterpret_cast<float*>(this); }
	operator const float* () const { return reinterpret_cast<const float*>(this); }

	v2 operator +() const { return *this; }
	v2 operator -() const { return{ -x, -y }; }

	v2& operator +=(const v2& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	v2& operator -=(const v2& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	v2& operator *=(float rhs) {
		x *= rhs;
		y *= rhs;
		return *this;
	}

	v2& operator /=(float rhs) {
		x /= rhs;
		y /= rhs;
		return *this;
	}
};

typedef struct _D3DVECTOR D3DVECTOR;
typedef struct D3DXVECTOR3 D3DXVECTOR3;

struct v3_pod
{
	float x, y, z;

	static constexpr auto size = 3;

	explicit operator float* () { return reinterpret_cast<float*>(this); }
	explicit operator const float* () const { return reinterpret_cast<const float*>(this); }

	float& operator[](size_t i) { assert(i < 3); return static_cast<float*>(*this)[i]; }
	const float& operator[](size_t i) const { assert(i < 3); return static_cast<const float*>(*this)[i]; }

	v3_pod operator +() const { return *this; }
	v3_pod operator -() const { return{ -x, -y, -z }; }

	v3_pod& operator +=(const v3_pod& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	v3_pod& operator -=(const v3_pod& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	v3_pod& operator *=(float rhs) {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}

	v3_pod& operator /=(float rhs) {
		x /= rhs;
		y /= rhs;
		z /= rhs;
		return *this;
	}
};

struct v3
{
	v3() = default;
	v3(float x, float y, float z) : x{ x }, y{ y }, z{ z } {}
	v3(const float(&arr)[3]) : x{ arr[0] }, y{ arr[1] }, z{ arr[2] } {}
	v3(const v3_pod& v) : x{v.x}, y{v.y}, z{v.z} {}

	v3(const D3DVECTOR& vec);
	v3(const D3DXVECTOR3& vec);

	float x, y, z;

	static constexpr auto size = 3;

	explicit operator float* () { return reinterpret_cast<float*>(this); }
	explicit operator const float* () const { return reinterpret_cast<const float*>(this); }
	operator v3_pod() const { return v3_pod{x, y, z}; }

	float& operator[](size_t i) { assert(i < 3); return static_cast<float*>(*this)[i]; }
	const float& operator[](size_t i) const { assert(i < 3); return static_cast<const float*>(*this)[i]; }

	operator D3DVECTOR () const;
	operator D3DXVECTOR3 () const;

	v3 operator +() const { return *this; }
	v3 operator -() const { return{ -x, -y, -z }; }

	v3& operator +=(const v3& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	v3& operator -=(const v3& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	v3& operator *=(float rhs) {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}

	v3& operator /=(float rhs) {
		x /= rhs;
		y /= rhs;
		z /= rhs;
		return *this;
	}
};

struct v4
{
	float x, y, z, w;
	
	static constexpr auto size = 4;

	v4() = default;
	v4(float x, float y, float z, float w) : x{ x }, y{ y }, z{ z }, w{ w } {}

	operator float* () { return reinterpret_cast<float*>(this); }
	operator const float* () const { return reinterpret_cast<const float*>(this); }

	v4 operator +() const { return *this; }
	v4 operator -() const { return{ -x, -y, -z, -w }; }

	v4& operator +=(const v4& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;
		return *this;
	}

	v4& operator -=(const v4& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		w -= rhs.w;
		return *this;
	}

	v4& operator *=(float rhs) {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		w *= rhs;
		return *this;
	}

	v4& operator /=(float rhs) {
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;
		return *this;
	}
};

#define NONMEMBER_OP(op, lhs_type, rhs_type) \
inline lhs_type operator op(const lhs_type& a, rhs_type b) { \
	auto ret = a; ret op##= b; return ret; }

#define NONMEMBER_OP_INVERTED(op, lhs_type, rhs_type) \
inline lhs_type operator op(rhs_type b, const lhs_type& a) { \
	auto ret = a; for (size_t i{}; i < lhs_type::size; ++i) ret[i] = b op ret[i]; return ret; }

#define NONMEMBER_OPS(type) \
NONMEMBER_OP(+, type, const type&) \
NONMEMBER_OP(-, type, const type&) \
NONMEMBER_OP(*, type, float) \
NONMEMBER_OP(/, type, float) \
NONMEMBER_OP_INVERTED(*, type, float) \
NONMEMBER_OP_INVERTED(/, type, float) \
inline bool operator ==(const type& a, const type& b) { \
	for (size_t i{}; i < type::size; ++i) if (a[i] != b[i]) return false; return true; }\
inline bool operator !=(const type& a, const type& b) { return !(a == b); }

NONMEMBER_OPS(v2)
NONMEMBER_OPS(v3)
NONMEMBER_OPS(v3_pod)
NONMEMBER_OPS(v4)

NONMEMBER_OP(+, v3, const v3_pod&)
NONMEMBER_OP(-, v3, const v3_pod&)
NONMEMBER_OP(+, v3_pod, v3)
NONMEMBER_OP(-, v3_pod, v3)

#undef NONMEMBER_OP
#undef NONMEMBER_OP_INVERTED
#undef NONMEMBER_OPS

using rvector = v3;
using rvector2 = v2;