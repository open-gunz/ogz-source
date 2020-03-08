#pragma once

struct D3DXQUATERNION;

struct rquaternion
{
	float x, y, z, w;

	rquaternion() {}
	rquaternion(float x, float y, float z, float w) : x{ x }, y{ y }, z{ z }, w{ w } {}
	rquaternion(const D3DXQUATERNION& quat);

	operator float* () { return reinterpret_cast<float*>(this); }
	operator const float* () const { return reinterpret_cast<const float*>(this); }

	operator D3DXQUATERNION () const;

	const rquaternion& operator +() const {
		return *this;
	}
	rquaternion operator -() const {
		return{ -x, -y, -z, -w };
	}

	rquaternion& operator +=(const rquaternion& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;
		return *this;
	}

	rquaternion& operator *=(const rquaternion& rhs) {
		auto p = *this;
		auto q = rhs;

		x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
		y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
		z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
		w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;

		return *this;
	}

	rquaternion& operator *=(float rhs) {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		w *= rhs;
		return *this;
	}
	rquaternion& operator /=(float rhs) {
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;
		return *this;
	}
};

inline rquaternion operator +(const rquaternion& a, const rquaternion& b) {
	auto ret = a; return ret += b;
}

inline rquaternion operator *(const rquaternion& a, const rquaternion& b) {
	auto ret = a; return ret *= b;
}

inline rquaternion operator *(const rquaternion& a, float b) {
	auto ret = a; return ret *= b;
}

inline rquaternion operator *(float b, const rquaternion& a) {
	auto ret = a; return ret *= b;
}

inline rquaternion operator /(const rquaternion& a, float b) {
	auto ret = a; return ret /= b;
}