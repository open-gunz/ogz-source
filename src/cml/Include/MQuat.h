#ifndef _MQUAT_H
#define _MQUAT_H

#include "MMatrix.h"
#include "MVector3.h"

class MQuat
{
public:
	float x, y, z, w;

	MQuat() { }
	MQuat(float xx, float yy, float zz, float ww) {
		x=xx;y=yy;z=zz;w=ww;
	}
	MQuat(const MMatrix& mat);

	bool operator==(const MQuat& other) const;
	MQuat& operator=(const MQuat& other);
	MQuat& operator=(const MMatrix& other);
	MQuat operator+(const MQuat& other) const;
	MQuat operator*(const MQuat& other) const;
	MQuat operator*(float s) const;
	MQuat& operator*=(float s);
	MVector3 MQuat::operator* (const MVector3& v) const;
	MQuat& operator*=(const MQuat& other);

	MMatrix Conv2Matrix() const;
	float DotProduct(const MQuat& other) const;
	MQuat Slerp(MQuat q1, MQuat q2, float time);
};


inline MQuat::MQuat(const MMatrix& mat)
{
	(*this) = mat;
}

inline MMatrix MQuat::Conv2Matrix() const
{
	MMatrix m;

	float _xx, _yy, _zz, _xy, _xz, _yz, _wx, _wy, _wz;

	_xx = 2.0f * x * x;
	_yy = 2.0f * y * y;
	_zz = 2.0f * z * z;
	_xy = 2.0f * x * y;
	_xz = 2.0f * x * z;
	_yz = 2.0f * y * z;
	_wx = 2.0f * w * x;
	_wy = 2.0f * w * y;
	_wz = 2.0f * w * z;

	m._11 = 1 - (_yy + _zz); 
	m._12 = (_xy - _wz);
	m._13 = (_xz + _wy);
	m._14 = 0.0f;

	m._21 = (_xy + _wz);
	m._22 = 1 - (_xx + _zz);
	m._23 = (_yz - _wx);
	m._24 = 0.0f;

	m._31 = (_xz - _wy);
	m._32 = (_yz + _wx);
	m._33 = 1 - (_xx + _yy);
	m._34 = 0.0f;

	m._41 = 0.0f;
	m._42 = 0.0f;
	m._43 = 0.0f;
	m._44 = 1.0f;

	return m;
}

inline bool MQuat::operator==(const MQuat& other) const
{
	if(x != other.x)
		return false;
	if(y != other.y)
		return false;
	if(z != other.z)
		return false;
	if(w != other.w)
		return false;

	return true;
}

inline MQuat& MQuat::operator=(const MQuat& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
	return *this;
}

inline MQuat& MQuat::operator=(const MMatrix& other)
{
	float t, l, s;

	t = other._11 + other._22+ other._33;

	if (t > 0.0f)
	{
		l = sqrt(t + 1.0f);
		s = 0.5f / l;
		w = l * 0.5f;
		x = (other._32 - other._23) * s;
		y = (other._13 - other._31) * s;
		z = (other._21 - other._12) * s;
	}	else
	{
		switch((other._11 > other._22) ? (other._11 > other._33 ? 0 : 2) : (other._22 > other._33 ? 1 : 2))
		{
			case 0 :
				l = sqrt(1.0f + other._11 - other._22 - other._33);
				s = 0.5f / l;
				x = l * 0.5f;
				y = (other._21 + other._12) * s;
				z = (other._13 + other._31) * s;
				w = (other._32 - other._23) * s;
				break;

			case 1 :
				l = sqrt(1.0f - other._11 + other._22 - other._33);
				s = 0.5f / l;
				x = (other._12 + other._21) * s;
				y = l * 0.5f;
				z = (other._23 + other._32) * s;
				w = (other._13 - other._31) * s;
				break;

			case 2 :
				l = sqrt(1.0f - other._11 - other._22 + other._33);
				s = 0.5f / l;
				x = (other._31 + other._13) * s;
				y = (other._23 + other._32) * s;
				z = l * 0.5f;
				w = (other._21 - other._12) * s;
				break;
		}
	}
}

inline MQuat MQuat::operator+(const MQuat& other) const
{
	return MQuat(x+other.x, y+other.y, z+other.z, w+other.w);
}

inline MQuat MQuat::operator*(const MQuat& other) const
{
	MQuat tmp;

	tmp.w = (other.w * w) - (other.x * x) - (other.y * y) - (other.z * z);
	tmp.x = (other.w * x) + (other.x * w) + (other.y * z) - (other.z * y);
	tmp.y = (other.w * y) + (other.y * w) + (other.z * x) - (other.x * z);
	tmp.z = (other.w * z) + (other.z * w) + (other.x * y) - (other.y * x);

	return tmp;
}

inline MQuat MQuat::operator*(float s) const
{
	return MQuat(s*x, s*y, s*z, s*w);
}

inline MQuat& MQuat::operator*=(float s)
{
	x *= s; y*=s; z*=s; w*=s;
	return *this;
}

inline MVector3 MQuat::operator*(const MVector3& v) const
{
	MVector3 uv, uuv; 
	MVector3 qvec(x, y, z); 
	uv = qvec.CrossProduct(v); 
	uuv = qvec.CrossProduct(uv); 
	uv *= (2.0f * w); 
	uuv *= 2.0f; 

	return v + uv + uuv; 
}

inline MQuat& MQuat::operator*=(const MQuat& other)
{
	*this = other * (*this);
	return *this;
}


inline float MQuat::DotProduct(const MQuat& other) const
{
	return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
}

inline MQuat MQuat::Slerp(MQuat q1, MQuat q2, float time)
{
    float angle = q1.DotProduct(q2);

    if (angle < 0.0f) 
    {
		q1 *= -1.0f;
		angle *= -1.0f;
    }

	float scale;
	float invscale;

    if ((angle + 1.0f) > 0.05f) 
    {
		if ((1.0f - angle) >= 0.05f)  // spherical interpolation
		{
			float theta = (float)acos(angle);
			float invsintheta = 1.0f / (float)sin(theta);
			scale = (float)sin(theta * (1.0f-time)) * invsintheta;
			invscale = (float)sin(theta * time) * invsintheta;
		}
		else // linear interploation
		{
			scale = 1.0f - time;
			invscale = time;
		}
    }
    else 
    {
        q2 = MQuat(-q1.y, q1.x, -q1.w, q1.z);
        scale = (float)sin(3.14159f * (0.5f - time));
        invscale = (float)sin(3.14159f * time);
    }

	*this = (q1*scale) + (q2*invscale);
	return *this;
}

#endif