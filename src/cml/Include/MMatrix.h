#ifndef _MMATRIX_H
#define _MMATRIX_H


#include "MVector3.h"
#include <string.h>

class MMatrix
{
public:
	union {
		struct {
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		} ;
		float m[16];
	};

	MMatrix() {  }
	MMatrix(float *f);
	MMatrix(float (*f)[4]);

	MMatrix& operator=(const MMatrix& other);
	bool operator==(const MMatrix& other) const;
	bool operator!=(const MMatrix& other) const;
	MMatrix& operator*=(const MMatrix& other);
	MMatrix operator*(const MMatrix& other) const;

	float& operator()(int row, int col) { return m[col * 4 + row]; }
	const float& operator()(int row, int col) const {  return m[col * 4 + row]; }


	void MakeIdentity();
	void SetTranslation(const MVector3& trans);
	void SetInverseTranslation( const MVector3& trans);
	MVector3 GetTranslation() const;
	void SetRotationRadians( const MVector3& rotation);
	void SetRotationDegrees( const MVector3& rotation);
	void SetScale( const MVector3& scale);
	void TransformVect( MVector3& vect) const;
	void TransformVect( const MVector3& in, MVector3& out) const;
	void SetProjectionMatrixFovRH(float fFOVy, float fAspectRatio, float zNear, float zFar);
	void SetProjectionMatrixFovLH(float fFOVy, float fAspectRatio, float zNear, float zFar);
	void SetLookAtMatrixLH(const MVector3& position, const MVector3& target, const MVector3& upVector);
	void SetLookAtMatrixRH(const MVector3& position, const MVector3& target, const MVector3& upVector);

	bool GetInverse( MMatrix *pOut, float *fDet );

	static const MMatrix IDENTITY;
};



inline MMatrix::MMatrix(float *f)
{
	memcpy(&_11, f, sizeof(float)*4*4);
}

inline MMatrix::MMatrix(float (*f)[4])
{
	for(int i=0; i<4; i++)
		memcpy(&_11 + 4*i, f[i], sizeof(float)*4);
}

inline MMatrix& MMatrix::operator=(const MMatrix& other)
{
	for (int i=0; i<16; i++)
	{
		MMatrix::m[i] = other.m[i];
	}
	return *this;
}

inline bool MMatrix::operator==(const MMatrix& other) const
{
	for (int i=0; i<16; i++)
	{
		if (MMatrix::m[i] != other.m[i]) return false;
	}
	return true;
}

inline bool MMatrix::operator!=(const MMatrix& other) const
{
	return !(*this == other);
}

inline MMatrix& MMatrix::operator*=(const MMatrix& other)
{
	float newMatrix[16];
	const float* m1 = m, *m2 = other.m;

	newMatrix[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
	newMatrix[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
	newMatrix[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
	newMatrix[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];
	
	newMatrix[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
	newMatrix[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
	newMatrix[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
	newMatrix[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];
	
	newMatrix[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
	newMatrix[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
	newMatrix[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
	newMatrix[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];
	
	newMatrix[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
	newMatrix[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
	newMatrix[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
	newMatrix[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];

	for (int i=0; i<16;i++)
	{
		MMatrix::m[i] = newMatrix[i];
	}
	return *this;
}

inline MMatrix MMatrix::operator*(const MMatrix& other) const
{
	MMatrix tmtrx;
	const float *m1 = m, *m2 = other.m;
	float *m3 = tmtrx.m;

	m3[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
	m3[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
	m3[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
	m3[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];
	
	m3[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
	m3[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
	m3[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
	m3[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];
	
	m3[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
	m3[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
	m3[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
	m3[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];
	
	m3[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
	m3[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
	m3[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
	m3[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];

	return tmtrx;

}

inline void MMatrix::MakeIdentity()
{
	for (int i = 0; i < 16; i++)
	{
		m[i] = 0.0f;
	}
	m[0] = m[5] = m[10] = m[15] = 1.0f;
}

inline void MMatrix::SetTranslation(const MVector3& trans)
{
	_41 = trans.x;
	_42 = trans.y;
	_43 = trans.z;
}

inline void MMatrix::SetInverseTranslation( const MVector3& trans)
{
	_41 = -trans.x;
	_42 = -trans.y;
	_43 = -trans.z;

}

inline MVector3 MMatrix::GetTranslation() const
{
	return MVector3(_41, _42, _43);
}

inline void MMatrix::SetRotationRadians( const MVector3& rotation)
{
	SetRotationDegrees( rotation * (float)3.1415926535897932384626433832795 / 180.0 );
}

inline void MMatrix::SetRotationDegrees( const MVector3& rotation)
{
	double cr = cos( rotation.x );
	double sr = sin( rotation.x );
	double cp = cos( rotation.y );
	double sp = sin( rotation.y );
	double cy = cos( rotation.z );
	double sy = sin( rotation.z );

	m[0] = float(cp*cy);
	m[1] = float(cp*sy);
	m[2] = float( -sp );

	double srsp = sr*sp;
	double crsp = cr*sp;

	m[4] = (float)( srsp*cy-cr*sy );
	m[5] = (float)( srsp*sy+cr*cy );
	m[6] = (float)( sr*cp );

	m[8] = (float)( crsp*cy+sr*sy );
	m[9] = (float)( crsp*sy-sr*cy );
	m[10] = (float)( cr*cp );
}

inline void MMatrix::SetScale(const MVector3& scale)
{
	_11 = scale.x;
	_22 = scale.y;
	_33 = scale.z;
}

inline void MMatrix::TransformVect( MVector3& vect) const
{
	float vector[3];

	vector[0] = vect.x*_11 + vect.y*_21 + vect.z*_31 + _41;
	vector[1] = vect.x*_12 + vect.y*_22 + vect.z*_32 + _42;
	vector[2] = vect.x*_13 + vect.y*_23 + vect.z*_33 + _43;

	vect.x = vector[0];
	vect.y = vector[1];
	vect.z = vector[2];

}

inline void MMatrix::TransformVect( const MVector3& in, MVector3& out) const
{
	out.x = in.x*_11 + in.y*_21 + in.z*_31 + _41;
	out.y = in.x*_12 + in.y*_22 + in.z*_32 + _42;
	out.z = in.x*_13 + in.y*_23 + in.z*_33 + _43;
}


inline bool MMatrix::GetInverse(MMatrix *pOut, float *fDet)
{
	/// The inverse is calculated using Cramers rule.
	/// If no inverse exists then 'false' is returned.

	const MMatrix &m = *this;

	float d = (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) * (m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3))	- (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) * (m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3))
			+ (m(0, 0) * m(3, 1) - m(3, 0) * m(0, 1)) * (m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3))	+ (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) * (m(0, 2) * m(3, 3) - m(3, 2) * m(0, 3))
			- (m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1)) * (m(0, 2) * m(2, 3) - m(2, 2) * m(0, 3))	+ (m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1)) * (m(0, 2) * m(1, 3) - m(1, 2) * m(0, 3));
	
	if(fDet) *fDet = d;

	if (d == 0.f)
		return false;

	d = 1.f / d;

	MMatrix &out = *pOut;

	out(0, 0) = d * (m(1, 1) * (m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3)) + m(2, 1) * (m(3, 2) * m(1, 3) - m(1, 2) * m(3, 3)) + m(3, 1) * (m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3)));
	out(1, 0) = d * (m(1, 2) * (m(2, 0) * m(3, 3) - m(3, 0) * m(2, 3)) + m(2, 2) * (m(3, 0) * m(1, 3) - m(1, 0) * m(3, 3)) + m(3, 2) * (m(1, 0) * m(2, 3) - m(2, 0) * m(1, 3)));
	out(2, 0) = d * (m(1, 3) * (m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1)) + m(2, 3) * (m(3, 0) * m(1, 1) - m(1, 0) * m(3, 1)) + m(3, 3) * (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)));
	out(3, 0) = d * (m(1, 0) * (m(3, 1) * m(2, 2) - m(2, 1) * m(3, 2)) + m(2, 0) * (m(1, 1) * m(3, 2) - m(3, 1) * m(1, 2)) + m(3, 0) * (m(2, 1) * m(1, 2) - m(1, 1) * m(2, 2)));
	out(0, 1) = d * (m(2, 1) * (m(0, 2) * m(3, 3) - m(3, 2) * m(0, 3)) + m(3, 1) * (m(2, 2) * m(0, 3) - m(0, 2) * m(2, 3)) + m(0, 1) * (m(3, 2) * m(2, 3) - m(2, 2) * m(3, 3)));
	out(1, 1) = d * (m(2, 2) * (m(0, 0) * m(3, 3) - m(3, 0) * m(0, 3)) + m(3, 2) * (m(2, 0) * m(0, 3) - m(0, 0) * m(2, 3)) + m(0, 2) * (m(3, 0) * m(2, 3) - m(2, 0) * m(3, 3)));
	out(2, 1) = d * (m(2, 3) * (m(0, 0) * m(3, 1) - m(3, 0) * m(0, 1)) + m(3, 3) * (m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1)) + m(0, 3) * (m(3, 0) * m(2, 1) - m(2, 0) * m(3, 1)));
	out(3, 1) = d * (m(2, 0) * (m(3, 1) * m(0, 2) - m(0, 1) * m(3, 2)) + m(3, 0) * (m(0, 1) * m(2, 2) - m(2, 1) * m(0, 2)) + m(0, 0) * (m(2, 1) * m(3, 2) - m(3, 1) * m(2, 2)));
	out(0, 2) = d * (m(3, 1) * (m(0, 2) * m(1, 3) - m(1, 2) * m(0, 3)) + m(0, 1) * (m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3)) + m(1, 1) * (m(3, 2) * m(0, 3) - m(0, 2) * m(3, 3)));
	out(1, 2) = d * (m(3, 2) * (m(0, 0) * m(1, 3) - m(1, 0) * m(0, 3)) + m(0, 2) * (m(1, 0) * m(3, 3) - m(3, 0) * m(1, 3)) + m(1, 2) * (m(3, 0) * m(0, 3) - m(0, 0) * m(3, 3)));
	out(2, 2) = d * (m(3, 3) * (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) + m(0, 3) * (m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1)) + m(1, 3) * (m(3, 0) * m(0, 1) - m(0, 0) * m(3, 1)));
	out(3, 2) = d * (m(3, 0) * (m(1, 1) * m(0, 2) - m(0, 1) * m(1, 2)) + m(0, 0) * (m(3, 1) * m(1, 2) - m(1, 1) * m(3, 2)) + m(1, 0) * (m(0, 1) * m(3, 2) - m(3, 1) * m(0, 2)));
	out(0, 3) = d * (m(0, 1) * (m(2, 2) * m(1, 3) - m(1, 2) * m(2, 3)) + m(1, 1) * (m(0, 2) * m(2, 3) - m(2, 2) * m(0, 3)) + m(2, 1) * (m(1, 2) * m(0, 3) - m(0, 2) * m(1, 3)));
	out(1, 3) = d * (m(0, 2) * (m(2, 0) * m(1, 3) - m(1, 0) * m(2, 3)) + m(1, 2) * (m(0, 0) * m(2, 3) - m(2, 0) * m(0, 3)) + m(2, 2) * (m(1, 0) * m(0, 3) - m(0, 0) * m(1, 3)));
	out(2, 3) = d * (m(0, 3) * (m(2, 0) * m(1, 1) - m(1, 0) * m(2, 1)) + m(1, 3) * (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) + m(2, 3) * (m(1, 0) * m(0, 1) - m(0, 0) * m(1, 1)));
	out(3, 3) = d * (m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) + m(1, 0) * (m(2, 1) * m(0, 2) - m(0, 1) * m(2, 2)) + m(2, 0) * (m(0, 1) * m(1, 2) - m(1, 1) * m(0, 2)));

	return true;

}

inline void MMatrix::SetProjectionMatrixFovRH(float fFOVy, float fAspectRatio, float zNear, float zFar)
{
	float yScale = 1/tan(fFOVy/2);
	float xScale = fAspectRatio * yScale;

	(*this)(0,0) = xScale;
	(*this)(1,0) = 0;
	(*this)(2,0) = 0;
	(*this)(3,0) = 0;

	(*this)(0,1) = 0;
	(*this)(1,1) = yScale;
	(*this)(2,1) = 0;
	(*this)(3,1) = 0;

	(*this)(0,2) = 0;
	(*this)(1,2) = 0;
	(*this)(2,2) = zFar/(zFar-zNear);
	(*this)(3,2) = -1;

	(*this)(0,3) = 0;
	(*this)(1,3) = 0;
	(*this)(2,3) = zNear*zFar/(zNear-zFar);
	(*this)(3,3) = 0;
}

inline void MMatrix::SetProjectionMatrixFovLH(float fFOVy, float fAspectRatio, float zNear, float zFar)
{
	float yScale = 1/tan(fFOVy/2);
	float xScale = fAspectRatio * yScale;

	(*this)(0,0) = xScale;
	(*this)(1,0) = 0;
	(*this)(2,0) = 0;
	(*this)(3,0) = 0;

	(*this)(0,1) = 0;
	(*this)(1,1) = yScale;
	(*this)(2,1) = 0;
	(*this)(3,1) = 0;

	(*this)(0,2) = 0;
	(*this)(1,2) = 0;
	(*this)(2,2) = zFar/(zFar-zNear);
	(*this)(3,2) = 1;

	(*this)(0,3) = 0;
	(*this)(1,3) = 0;
	(*this)(2,3) = zNear*zFar/(zNear-zFar);
	(*this)(3,3) = 0;
}

inline void MMatrix::SetLookAtMatrixRH(const MVector3& eye, const MVector3& at, const MVector3& up)
{
	MVector3 zaxis = eye - at;
	zaxis.Normalize();

	MVector3 xaxis = up.CrossProduct(zaxis);
	xaxis.Normalize();

	MVector3 yaxis = zaxis.CrossProduct(xaxis);

	(*this)(0,0) = xaxis.x;
	(*this)(1,0) = yaxis.x;
	(*this)(2,0) = zaxis.x;
	(*this)(3,0) = 0;

	(*this)(0,1) = xaxis.y;
	(*this)(1,1) = yaxis.y;
	(*this)(2,1) = zaxis.y;
	(*this)(3,1) = 0;

	(*this)(0,2) = xaxis.z;
	(*this)(1,2) = yaxis.z;
	(*this)(2,2) = zaxis.z;
	(*this)(3,2) = 0;

	(*this)(0,3) = -xaxis.DotProduct(eye);
	(*this)(1,3) = -yaxis.DotProduct(eye);
	(*this)(2,3) = -zaxis.DotProduct(eye);
	(*this)(3,3) = 1.0f;
}

inline void MMatrix::SetLookAtMatrixLH(const MVector3& eye, const MVector3& at, const MVector3& up)
{
	MVector3 zaxis = at - eye;
	zaxis.Normalize();

	MVector3 xaxis = up.CrossProduct(zaxis);
	xaxis.Normalize();

	MVector3 yaxis = zaxis.CrossProduct(xaxis);

	(*this)(0,0) = xaxis.x;
	(*this)(1,0) = yaxis.x;
	(*this)(2,0) = zaxis.x;
	(*this)(3,0) = 0;

	(*this)(0,1) = xaxis.y;
	(*this)(1,1) = yaxis.y;
	(*this)(2,1) = zaxis.y;
	(*this)(3,1) = 0;

	(*this)(0,2) = xaxis.z;
	(*this)(1,2) = yaxis.z;
	(*this)(2,2) = zaxis.z;
	(*this)(3,2) = 0;

	(*this)(0,3) = -xaxis.DotProduct(eye);
	(*this)(1,3) = -yaxis.DotProduct(eye);
	(*this)(2,3) = -zaxis.DotProduct(eye);
	(*this)(3,3) = 1.0f;
}

#endif