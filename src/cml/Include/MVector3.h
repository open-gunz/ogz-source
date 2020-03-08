#ifndef _MVECTOR3_H
#define _MVECTOR3_H

#include <math.h>

class MVector3
{
public:
	union {
		struct {
			float x, y, z;
		} ;
		float v[3];
	};

	MVector3() { }
	MVector3(float xx, float yy, float zz): x(xx), y(yy), z(zz) { }

	// operator
	MVector3 operator-() const { return MVector3(-x, -y, -z);   }
	MVector3& operator=(const MVector3& other)	{ x = other.x; y = other.y; z = other.z; return *this; }

	MVector3 operator+(const MVector3& other) const { return MVector3(x + other.x, y + other.y, z + other.z);	}
	MVector3& operator+=(const MVector3& other)	{ x+=other.x; y+=other.y; z+=other.z; return *this; }

	MVector3 operator-(const MVector3& other) const { return MVector3(x - other.x, y - other.y, z - other.z);	}
	MVector3& operator-=(const MVector3& other)	{ x-=other.x; y-=other.y; z-=other.z; return *this; }

	MVector3 operator*(const MVector3& other) const { return MVector3(x * other.x, y * other.y, z * other.z);	}
	MVector3& operator*=(const MVector3& other)	{ x*=other.x; y*=other.y; z*=other.z; return *this; }
	MVector3 operator*(const float v) const { return MVector3(x * v, y * v, z * v);	}
	MVector3& operator*=(const float v) { x*=v; y*=v; z*=v; return *this; }

	MVector3 operator/(const MVector3& other) const { return MVector3(x / other.x, y / other.y, z / other.z);	}
	MVector3& operator/=(const MVector3& other)	{ x/=other.x; y/=other.y; z/=other.z; return *this; }
	MVector3 operator/(const float v) const { float i=(float)1.0/v; return MVector3(x * i, y * i, z * i);	}
	MVector3& operator/=(const float v) { float i=(float)1.0/v; x*=i; y*=i; z*=i; return *this; }

	bool operator<=(const MVector3&other) const { return x<=other.x && y<=other.y && z<=other.z;};
	bool operator>=(const MVector3&other) const { return x>=other.x && y>=other.y && z>=other.z;};
	bool operator==(const MVector3& other) const { return other.x==x && other.y==y && other.z==z; }
	bool operator!=(const MVector3& other) const { return other.x!=x || other.y!=y || other.z!=z; }

	friend MVector3 operator * ( float f, const class MVector3& v) { return MVector3( f*v.x , f*v.y , f*v.z ); }

	// function
	void Set(float x, float y, float z) { MVector3::x = x; MVector3::y = y; MVector3::z = z; }
	void Set(MVector3& p)				{ MVector3::x = p.x; MVector3::y = p.y; MVector3::z = p.z; }
	float Magnitude();
	float MagnitudeSQ();
	float DotProduct(const MVector3& other) const;
	MVector3 CrossProduct(const MVector3& p) const;
	MVector3& Normalize();
	void SetLength(float newlength);
	void Invert();
	MVector3 GetInterpolated(const MVector3& other, float d) const;

	static const MVector3 IDENTITY;
	static const MVector3 AXISX;
	static const MVector3 AXISY;
	static const MVector3 AXISZ;
};



// inline functions ////////////////////////////////////////////////////////////////////
inline float MVector3::Magnitude()
{
	return (float)sqrt(x*x+y*y+z*z);
}

inline float MVector3::MagnitudeSQ()
{
	return (x*x+y*y+z*z);
}

inline float MVector3::DotProduct(const MVector3& other) const
{
	return x*other.x + y*other.y + z*other.z;
}

inline MVector3 MVector3::CrossProduct(const MVector3& p) const
{
	return MVector3(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x);
}

inline MVector3& MVector3::Normalize()
{
	float scale = (float)Magnitude();

	if (scale == 0)
		return *this;

	scale = (float)1.0f / scale;
	x *= scale;
	y *= scale;
	z *= scale;
	return *this;
}

inline void MVector3::SetLength(float newlength)
{
	Normalize();
	*this *= newlength;
}

inline void MVector3::Invert()
{
	x *= -1.0f;
	y *= -1.0f;
	z *= -1.0f;
}

inline MVector3 MVector3::GetInterpolated(const MVector3& other, float d) const
{
	float inv = 1.0f - d;
	return MVector3(other.x*inv + x*d,
						other.y*inv + y*d,
						other.z*inv + z*d);
}

#endif