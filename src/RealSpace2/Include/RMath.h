#pragma once

#include "MMath.h"
#include "RTypes.h"
#include "MUtil.h"
#include "MDebug.h"
#include <cassert>
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <mmintrin.h>
#include <xmmintrin.h>
using std::min;
using std::max;
using std::sin;
using std::cos;
using std::tan;
using std::asin;
using std::acos;
using std::atan;
using std::atan2;
using std::abs;
using std::pow;
using std::log;

#ifndef TOLER
#define TOLER 0.001
#endif
#define IS_ZERO(a) ((fabs((double)(a)) < (double) TOLER))
inline bool IS_EQ(float a, float b)
{
	auto diff = a - b;
	return diff < TOLER && diff > -TOLER;
}
#define IS_EQ3(a,b) (IS_EQ((a).x,(b).x)&&IS_EQ((a).y,(b).y)&&IS_EQ((a).z,(b).z))
#define SIGNOF(a) ( (a)<-TOLER ? NEGATIVE : (a)>TOLER ? POSITIVE : ZERO )
#define RANDOMFLOAT ((float)rand()/(float)RAND_MAX)

#define FLOAT2RGB24(r, g, b) ( ( ((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define VECTOR2RGB24(v)		FLOAT2RGB24((v).x,(v).y,(v).z)
#define BYTE2RGB24(r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)))
#define BYTE2RGB32(a,r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)|(((DWORD) (BYTE) (a)) << 24)))
#define DWORD2VECTOR(x)		rvector(float(((x)& 0xff0000) >> 16)/255.f, float(((x) & 0xff00) >> 8)/255.f,float(((x) & 0xff))/255.f))

_NAMESPACE_REALSPACE2_BEGIN

inline bool Equals(float a, float b)
{
	return IS_EQ(a, b);
}

inline bool Equals(const v3& a, const v3& b)
{
	for (size_t i{}; i < 3; ++i)
		if (!Equals(a[i], b[i]))
			return false;

	return true;
}

inline bool Equals(const rmatrix& a, const rmatrix& b)
{
	for (size_t i{}; i < 4; ++i)
		for (size_t j{}; j < 4; ++j)
			if (!Equals(a(i, j), b(i, j)))
				return false;

	return true;
}

inline bool Equals(const rquaternion& a, const rquaternion& b)
{
	for (size_t i{}; i < 4; ++i)
		if (!Equals(a[i], b[i]))
			return false;

	return true;
}

inline void GetIdentityMatrix(rmatrix& m)
{
	m._11 = 1; m._12 = 0; m._13 = 0; m._14 = 0;
	m._21 = 0; m._22 = 1; m._23 = 0; m._24 = 0;
	m._31 = 0; m._32 = 0; m._33 = 1; m._34 = 0;
	m._41 = 0; m._42 = 0; m._43 = 0; m._44 = 1;
}

inline rmatrix GetIdentityMatrix() {
	rmatrix m;
	GetIdentityMatrix(m);
	return m;
}

inline rmatrix IdentityMatrix() { return GetIdentityMatrix(); }
inline rquaternion IdentityQuaternion() { return{ 0, 0, 0, 1 }; }

inline rmatrix ScalingMatrix(const v4& scale)
{
	rmatrix m;
	m._11 = scale.x; m._12 = 0;       m._13 = 0;       m._14 = 0;
	m._21 = 0;       m._22 = scale.y; m._23 = 0;       m._24 = 0;
	m._31 = 0;       m._32 = 0;       m._33 = scale.z; m._34 = 0;
	m._41 = 0;       m._42 = 0;       m._43 = 0;       m._44 = scale.w;
	return m;
}

inline rmatrix ScalingMatrix(const v3& scale)
{
	return ScalingMatrix({ EXPAND_VECTOR(scale), 1 });
}

inline rmatrix ScalingMatrix(float scale)
{
	return ScalingMatrix({ scale, scale, scale });
}

// Returns the translation a matrix applies as a vector.
inline rvector GetTransPos(const rmatrix& m) {
	return{ m._41, m._42, m._43 };
}

inline void SetTransPos(rmatrix& m, const v3& trans) {
	m._41 = trans.x;
	m._42 = trans.y;
	m._43 = trans.z;
}

template <typename T>
constexpr auto Square(const T& x) {
	return x * x;
}

// Returns (v.x, v.y, v.z, 0) * mat
inline v3 TransformNormal(const v3& v, const rmatrix& mat)
{
	v3 ret;

	ret.x = v.x * mat._11 + v.y * mat._21 + v.z * mat._31;
	ret.y = v.x * mat._12 + v.y * mat._22 + v.z * mat._32;
	ret.z = v.x * mat._13 + v.y * mat._23 + v.z * mat._33;

	return ret;
}

// Returns (v.x, v.y, v.z, 1) * mat
inline v3 Transform(const v3& v, const rmatrix& mat)
{
	return TransformNormal(v, mat) + GetTransPos(mat);
}

// Returns (v.x, v.y, v.z, 1) * mat projected back into w = 1
inline v3 TransformCoord(const v3& v, const rmatrix& mat)
{
	v3 ret = Transform(v, mat);
	auto w = v.x * mat._14 + v.y * mat._24 + v.z * mat._34 + mat._44;
	if (w)
		return ret / w;

	return{ 0, 0, 0 };
}

inline rmatrix Transpose(const rmatrix& src)
{
	rmatrix m;
	for (size_t i = 0; i < 4; i++)
		for (size_t j = 0; j < 4; j++)
			m(i, j) = src(j, i);
	return m;
}

inline v4 Transform(const v4& v, const rmatrix& mat)
{
	v4 ret;

	ret.x = v.x * mat._11 + v.y * mat._21 + v.z * mat._31 + v.w * mat._41;
	ret.y = v.x * mat._12 + v.y * mat._22 + v.z * mat._32 + v.w * mat._42;
	ret.z = v.x * mat._13 + v.y * mat._23 + v.z * mat._33 + v.w * mat._43;
	ret.w = v.x * mat._14 + v.y * mat._24 + v.z * mat._34 + v.w * mat._44;

	return ret;
}

// Transforms plane by a matrix of which mat is the inverse transpose.
inline rplane Transform(const rplane& plane, const rmatrix& mat)
{
	v4 v{ plane.a, plane.b, plane.c, plane.d };
	v = Transform(v, mat);
	return rplane{ v.x, v.y, v.z, v.w };
}

inline float Trace(const rmatrix& mat)
{
	return mat._11 + mat._22 + mat._33 + mat._44;
}

// Leave the namespace so that the operators are also visible through normal lookup.
_NAMESPACE_REALSPACE2_END

inline v3 operator *(const v3& v, const rmatrix &mat)
{
	return _REALSPACE2::TransformCoord(v, mat);
}

inline v3& operator *=(v3& v, const rmatrix &mat)
{
	v = v * mat;
	return v;
}

inline rplane operator *(const rplane& a, const rmatrix& b)
{
	return _REALSPACE2::Transform(a, b);
}

inline rplane& operator *=(rplane& a, const rmatrix& b)
{
	a = a * b;
	return a;
}

_NAMESPACE_REALSPACE2_BEGIN

inline rvector GetPoint(const rboundingbox& bb, int i) {
	return rvector{
		(i & 1) ? bb.vmin.x : bb.vmax.x,
		(i & 2) ? bb.vmin.y : bb.vmax.y,
		(i & 4) ? bb.vmin.z : bb.vmax.z };
}

// Returns the bounding box that contains both a and b.
inline rboundingbox Union(const rboundingbox& a, const rboundingbox& b)
{
	rboundingbox ret;
	for (size_t i = 0; i < 3; ++i)
		ret.vmin[i] = (std::min)(a.vmin[i], b.vmin[i]);
	for (size_t i = 0; i < 3; ++i)
		ret.vmax[i] = (std::max)(a.vmax[i], b.vmax[i]);
	return ret;
}

inline rboundingbox Union(const rboundingbox& a, const rvector& b)
{
	rboundingbox ret;
	for (size_t i = 0; i < 3; ++i)
		ret.vmin[i] = (std::min)(a.vmin[i], b[i]);
	for (size_t i = 0; i < 3; ++i)
		ret.vmax[i] = (std::max)(a.vmax[i], b[i]);
	return ret;
}

inline rboundingbox Union(const v3& a, const v3& b)
{
	rboundingbox ret;
	for (size_t i = 0; i < 3; ++i)
		ret.vmin[i] = (std::min)(a[i], b[i]);
	for (size_t i = 0; i < 3; ++i)
		ret.vmax[i] = (std::max)(a[i], b[i]);
	return ret;
}

inline bool Intersects(const rboundingbox& a, const rboundingbox &b)
{
	return
		a.vmin.x < b.vmax.x &&
		a.vmax.x > b.vmin.x &&
		a.vmin.y < b.vmax.y &&
		a.vmax.y > b.vmin.y &&
		a.vmin.z < b.vmax.z &&
		a.vmax.z > b.vmin.z;
}

// Returns the length squared of the input vector.
inline float MagnitudeSq(const v2& x) {
	return x.x * x.x + x.y * x.y;
}
inline float MagnitudeSq(const v3& x) {
	return x.x * x.x + x.y * x.y + x.z * x.z;
}
inline float MagnitudeSq(const v4& x) {
	return x.x * x.x + x.y * x.y + x.z * x.z + x.w * x.w;
}
inline float MagnitudeSq(const rquaternion& x) {
	return x.x * x.x + x.y * x.y + x.z * x.z + x.w * x.w;
}
// Returns the length of the input vector.
inline float Magnitude(const v2& x) {
	return sqrt(MagnitudeSq(x));
}
inline float Magnitude(const v3& x) {
	return sqrt(MagnitudeSq(x));
}
inline float Magnitude(const v4& x) {
	return sqrt(MagnitudeSq(x));
}
inline float Magnitude(const rquaternion& x) {
	return sqrt(MagnitudeSq(x));
}

// If the input vector has a nonzero length,
// this function sets it to a unit vector in the same direction.
// Otherwise, it does nothing.
template <typename T>
void Normalize(T& x)
{
	auto MagSq = MagnitudeSq(x);
	if (MagSq == 0 || Equals(MagSq, 1))
		return;
	x *= 1.f / sqrt(MagSq);
}

inline void Normalize(rplane& plane) {
	v3 Normal{ plane.a, plane.b, plane.c };
	Normalize(Normal);
	plane = { EXPAND_VECTOR(Normal), plane.d };
}

// If the input object has a nonzero length,
// this function returns a unit object in the same direction.
// Otherwise, it returns the null object.
template <typename T>
WARN_UNUSED_RESULT T Normalized(const T& x)
{
	T ret = x;
	Normalize(ret);
	return ret;
}

inline float DotProduct(const v2& a, const v2& b) {
	return a.x * b.x + a.y * b.y;
}
inline float DotProduct(const rvector& a, const rvector& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline float DotProduct(const v4& a, const v4& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
// Returns a dot (b.x, b.y, b.z, 1).
inline float DotProduct(const rplane& a, const rvector& b) {
	return a.a * b.x + a.b * b.y + a.c * b.z + a.d;
}
// Returns a dot (b.x, b.y, b.z, 0).
inline float DotPlaneNormal(const rplane& a, const rvector& b) {
	return a.a * b.x + a.b * b.y + a.c * b.z;
}
inline v3 CrossProduct(const v3& u, const v3& v) {
	return{ u.y * v.z - u.z * v.y,
		u.z * v.x - u.x * v.z,
		u.x * v.y - u.y * v.x };
}
inline void CrossProduct(rvector *result, const rvector &a, const rvector &b) {
	*result = CrossProduct(a, b);
}

inline float DotProduct(const rquaternion& a, const rquaternion& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

void MakeWorldMatrix(rmatrix *pOut, const rvector& pos, rvector dir, rvector up);

float GetDistance(const rvector &position, const rvector &line1, const rvector &line2);
rvector GetNearestPoint(const rvector &position, const rvector &a, const rvector &b);
float GetDistanceLineSegment(const rvector &position, const rvector &a, const rvector &b);
float GetDistanceBetweenLineSegment(const rvector &a, const rvector &aa, const rvector &c,
	const rvector &cc, rvector *ap, rvector *cp);
float GetDistance(const rvector &position, const rplane &plane);
rvector GetNearestPoint(const rvector &a, const rvector &aa, const rplane &plane);
float GetDistance(const rvector &a, const rvector &aa, const rplane &plane);
float GetDistance(const rboundingbox& bb, const rplane& plane);
void GetDistanceMinMax(rboundingbox &bb, rplane &plane, float *MinDist, float *MaxDist);
float GetDistance(const rboundingbox &bb, const rvector &point);
float GetArea(rvector &v1, rvector &v2, rvector &v3);

// Returns the clockwise rotation of ta such that tb aligns with ta on xy
float GetAngleOfVectors(const rvector &ta, const rvector &tb);

template <typename T>
auto Lerp(T src, T dest, float t) {
	return src * (1 - t) + dest * t;
}

// Returns from spherically interpolated towards to by t.
// from and to should be unit length, t should be in the range (0, 1).
v3 Slerp(const v3& from, const v3& to, float t);
v2 Slerp(const v2& from, const v2& to, float t);
v4 Slerp(const v4& from, const v4& to, float t);
rquaternion Slerp(const rquaternion& from, const rquaternion& to, float t);

inline rquaternion HadamardProduct(const rquaternion& a, const rquaternion& b) {
	return{ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

inline v3 HadamardProduct(const v3& a, const v3& b) {
	return{ a.x * b.x, a.y * b.y, a.z * b.z };
}

bool IsIntersect(const rboundingbox& bb1, const rboundingbox& bb2);
bool isInPlane(const rboundingbox& bb, const rplane& plane);
bool IsInSphere(const rboundingbox& bb, const rvector& point, float radius);

// Frustum intersection checks for the forward planes (ignores near and far).

// Point
bool isInViewFrustum(const rvector &point, const rfrustum& frustum);
// Ball
bool isInViewFrustum(const rvector &point, float radius, const rfrustum& frustum);
// Bounding box
bool isInViewFrustum(const rboundingbox& bb, const rfrustum& frustum);
// Line segment
bool isInViewFrustum(const rvector &point1, const rvector &point2, const rfrustum& frustum);

// Bounding box, all planes
bool isInViewFrustumWithZ(const rboundingbox& bb, const rfrustum& frustum);
// Bounding box, forward and far plane
bool isInViewFrustumWithFarZ(const rboundingbox& bb, const rfrustum& frustum);
// Bounding box, N planes (starting from the first)
bool isInViewFrustumwrtnPlanes(const rboundingbox& bb, const rfrustum& frustum, int nplane);

bool isLineIntersectBoundingBox(rvector &origin, rvector &dir, rboundingbox &bb);
bool IsIntersect(rvector& line_begin_, rvector& line_end_, rboundingbox& box_);
bool IsIntersect(rvector& line_begin_, rvector& line_dir_, rvector& center_, float radius_,
	float* dist = nullptr, rvector* p = nullptr);
bool IsIntersect(const rvector& orig, const rvector& dir, const rvector& center,
	const float radius, rvector* p = nullptr);
bool GetIntersectionOfTwoPlanes(rvector *pOutDir, rvector *pOutAPoint, const rplane &plane1, const rplane &plane2);

void MergeBoundingBox(rboundingbox *dest, rboundingbox *src);
void TransformBox(rboundingbox* result, const rboundingbox& src, const rmatrix& matrix);

inline rvector GetReflectionVector(const rvector& v, const rvector& n)
{
	auto neg = -v;
	float dot = DotProduct(neg, n);

	return (2 * dot) * n + v;
}

// Intel's reference 4x4 matrix inversion code
// from ftp://download.intel.com/design/pentiumiii/sml/24504301.pdf
inline void Inverse(rmatrix& result, const rmatrix& mat)
{
	auto* src = static_cast<const float*>(mat);
	auto* dest = static_cast<float*>(result);

	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1{}, row2, row3{};
	__m128 det, tmp1{};

	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)), (__m64*)(src + 4));
	row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src + 8)), (__m64*)(src + 12));
	row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
	row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src + 2)), (__m64*)(src + 6));
	row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src + 10)), (__m64*)(src + 14));
	row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
	row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);

	tmp1 = _mm_mul_ps(row2, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_mul_ps(row1, tmp1);
	minor1 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
	minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
	minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);

	tmp1 = _mm_mul_ps(row1, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
	minor3 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
	minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);

	tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	row2 = _mm_shuffle_ps(row2, row2, 0x4E);
	minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
	minor2 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
	minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);

	tmp1 = _mm_mul_ps(row0, row1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

	minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));

	tmp1 = _mm_mul_ps(row0, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
	minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));

	tmp1 = _mm_mul_ps(row0, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);

	det = _mm_mul_ps(row0, minor0);
	det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
	det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
	tmp1 = _mm_rcp_ss(det);
	det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
	det = _mm_shuffle_ps(det, det, 0x00);
	minor0 = _mm_mul_ps(det, minor0);
	_mm_storel_pi((__m64*)(dest), minor0);
	_mm_storeh_pi((__m64*)(dest + 2), minor0);
	minor1 = _mm_mul_ps(det, minor1);
	_mm_storel_pi((__m64*)(dest + 4), minor1);
	_mm_storeh_pi((__m64*)(dest + 6), minor1);
	minor2 = _mm_mul_ps(det, minor2);
	_mm_storel_pi((__m64*)(dest + 8), minor2);
	_mm_storeh_pi((__m64*)(dest + 10), minor2);
	minor3 = _mm_mul_ps(det, minor3);
	_mm_storel_pi((__m64*)(dest + 12), minor3);
	_mm_storeh_pi((__m64*)(dest + 14), minor3);
}

inline rmatrix Inverse(const rmatrix& mat)
{
	rmatrix ret;
	Inverse(ret, mat);
	return ret;
}

inline void RMatInv(rmatrix& q, const rmatrix& a) {
	Inverse(q, a);
}

// Returns a matrix that rotates row vectors by
// the given angle in radians around the given axis.
//
// Appears counterclockwise in a right-handed coordinate system
// when the axis is pointing towards the observer.
// Axis should be a unit vector. Angle should be in radians.
inline rmatrix RotationMatrix(const v3& axis, float angle)
{
	rmatrix m;

	auto cosa = cos(angle);
	auto sina = sin(angle);

	m._11 = cosa + Square(axis.x) * (1 - cosa);
	m._12 = axis.y * axis.x * (1 - cosa) + axis.z * sina;
	m._13 = axis.z * axis.x * (1 - cosa) - axis.y * sina;
	m._14 = 0;

	m._21 = axis.x * axis.y * (1 - cosa) - axis.z * sina;
	m._22 = cosa + Square(axis.y) * (1 - cosa);
	m._23 = axis.z * axis.y * (1 - cosa) + axis.x * sina;
	m._24 = 0;

	m._31 = axis.x * axis.z * (1 - cosa) + axis.y * sina;
	m._32 = axis.y * axis.z * (1 - cosa) - axis.x * sina;
	m._33 = cosa + Square(axis.z) * (1 - cosa);
	m._34 = 0;

	m._41 = 0;
	m._42 = 0;
	m._43 = 0;
	m._44 = 1;

	return m;
}

// Returns a matrix that rotates row vectors
// around the x-axis by the given angle in degrees.
inline rmatrix RGetRotX(float angle) {
	auto rads = PI_FLOAT / 180.f * angle;
	return RotationMatrix({ 1, 0, 0 }, rads);
}

inline rmatrix RGetRotY(float angle) {
	auto rads = PI_FLOAT / 180.f * angle;
	return RotationMatrix({ 0, 1, 0 }, rads);
}

inline rmatrix RGetRotZ(float angle) {
	auto rads = PI_FLOAT / 180.f * angle;
	return RotationMatrix({ 0, 0, 1 }, rads);
}

// Same as above, but the argument is in radians instead.
inline rmatrix RGetRotXRad(float angle) {
	return RotationMatrix({ 1, 0, 0 }, angle);
}

inline rmatrix RGetRotYRad(float angle) {
	return RotationMatrix({ 0, 1, 0 }, angle);
}

inline rmatrix RGetRotZRad(float angle) {
	return RotationMatrix({ 0, 0, 1 }, angle);
}

inline rmatrix QuaternionToMatrix(const rquaternion& q)
{
	return {
		1.0f - 2.0f*q.y*q.y - 2.0f*q.z*q.z, 2.0f*q.x*q.y + 2.0f*q.z*q.w, 2.0f*q.x*q.z - 2.0f*q.y*q.w, 0.0f,
		2.0f*q.x*q.y - 2.0f*q.z*q.w, 1.0f - 2.0f*q.x*q.x - 2.0f*q.z*q.z, 2.0f*q.y*q.z + 2.0f*q.x*q.w, 0.0f,
		2.0f*q.x*q.z + 2.0f*q.y*q.w, 2.0f*q.y*q.z - 2.0f*q.x*q.w, 1.0f - 2.0f*q.x*q.x - 2.0f*q.y*q.y, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };
}

inline rquaternion MatrixToQuaternion(const rmatrix& mat)
{
	auto fourXSquaredMinus1 = mat(0, 0) - mat(1, 1) - mat(2, 2);
	auto fourYSquaredMinus1 = mat(1, 1) - mat(0, 0) - mat(2, 2);
	auto fourZSquaredMinus1 = mat(2, 2) - mat(0, 0) - mat(1, 1);
	auto fourWSquaredMinus1 = mat(0, 0) + mat(1, 1) + mat(2, 2);

	int biggestIndex = 0;
	auto fourBiggestSquaredMinus1 = fourWSquaredMinus1;
	if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourXSquaredMinus1;
		biggestIndex = 1;
	}
	if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourYSquaredMinus1;
		biggestIndex = 2;
	}
	if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourZSquaredMinus1;
		biggestIndex = 3;
	}

	auto biggestVal = sqrt(fourBiggestSquaredMinus1 + 1) * 0.5f;
	auto mult = 0.25f / biggestVal;

	rquaternion Result;
	switch (biggestIndex)
	{
	case 0:
		Result.w = biggestVal;
		Result.x = (mat(1, 2) - mat(2, 1)) * mult;
		Result.y = (mat(2, 0) - mat(0, 2)) * mult;
		Result.z = (mat(0, 1) - mat(1, 0)) * mult;
		break;
	case 1:
		Result.w = (mat(1, 2) - mat(2, 1)) * mult;
		Result.x = biggestVal;
		Result.y = (mat(0, 1) + mat(1, 0)) * mult;
		Result.z = (mat(2, 0) + mat(0, 2)) * mult;
		break;
	case 2:
		Result.w = (mat(2, 0) - mat(0, 2)) * mult;
		Result.x = (mat(0, 1) + mat(1, 0)) * mult;
		Result.y = biggestVal;
		Result.z = (mat(1, 2) + mat(2, 1)) * mult;
		break;
	case 3:
		Result.w = (mat(0, 1) - mat(1, 0)) * mult;
		Result.x = (mat(2, 0) + mat(0, 2)) * mult;
		Result.y = (mat(1, 2) + mat(2, 1)) * mult;
		Result.z = biggestVal;
		break;
	}
	return Result;
}

inline rquaternion AngleAxisToQuaternion(const v3& Axis, float Angle)
{
	auto xyz = Axis * sin(Angle / 2);
	return{ EXPAND_VECTOR(xyz), cos(Angle / 2) };
}

inline float sgn(float a)
{
	if (a > 0.0F) return (1.0F);
	if (a < 0.0F) return (-1.0F);
	return (0.0F);
}

inline v3 GetPlaneNormal(const rplane& plane) {
	return{ plane.a, plane.b, plane.c };
}

inline bool IntersectLineSegmentPlane(v3* hit, const rplane& plane, const v3& l0, const v3& l1)
{
	auto dot1 = DotProduct(plane, l0);
	auto dot2 = DotProduct(plane, l1);
	if (sgn(dot1) == sgn(dot2))
		return false;
	else if (!hit)
		return true;

	auto l = l1 - l0;
	Normalize(l);

	auto n = GetPlaneNormal(plane);
	auto p0 = n * -plane.d;
	auto d = DotProduct(p0 - l0, n) / DotProduct(l, n);

	*hit = d * l + l0;
	return true;
}

inline bool IntersectTriangle(const v3& V1, const v3& V2, const v3& V3, // Triangle points
	const v3& Origin, const v3& Dir, // Ray origin and direction
	float* out = nullptr, // Output: Distance from origin to intersection point
	float* u_out = nullptr, float* v_out = nullptr)
{
	// Möller–Trumbore triangle intersection algorithm

	constexpr auto EPSILON = 0.000001;

	v3 e1, e2;  //Edge1, Edge2
	v3 P, Q, T;
	float det, inv_det, u, v;
	float t;

	//Find vectors for two edges sharing V1
	e1 = V2 - V1;
	e2 = V3 - V1;
	//Begin calculating determinant - also used to calculate u parameter
	CrossProduct(&P, Dir, e2);
	//if determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
	det = DotProduct(e1, P);
	//NOT CULLING
	if (det > -EPSILON && det < EPSILON) return false;
	inv_det = 1.f / det;

	//calculate distance from V1 to ray origin
	T = Origin - V1;

	//Calculate u parameter and test bound
	u = DotProduct(T, P) * inv_det;
	//The intersection lies outside of the triangle
	if (u < 0.f || u > 1.f) return false;

	//Prepare to test v parameter
	CrossProduct(&Q, T, e1);

	//Calculate V parameter and test bound
	v = DotProduct(Dir, Q) * inv_det;
	//The intersection lies outside of the triangle
	if (v < 0.f || u + v  > 1.f) return false;

	t = DotProduct(e2, Q) * inv_det;

	if (t > EPSILON) { //ray intersection
		if (out)
			*out = t;
		if (u_out)
			*u_out = u;
		if (v_out)
			*v_out = v;
		return true;
	}

	// No hit, no win
	return false;
}

inline v3 NoninfiniteReciprocal(const v3& Vec)
{
	v3 NonzeroVec{
		Vec.x == 0 ? 0.000001f : Vec.x,
		Vec.y == 0 ? 0.000001f : Vec.y,
		Vec.z == 0 ? 0.000001f : Vec.z,
	};
	return 1.f / NonzeroVec;
}

inline bool IntersectLineAABB(
	const v3& origin, const v3& dir,
	const rboundingbox& bbox,
	const v3& dirfrac,
	float* t = nullptr)
{
	float t1 = (bbox.vmin.x - origin.x)*dirfrac.x;
	float t2 = (bbox.vmax.x - origin.x)*dirfrac.x;
	float t3 = (bbox.vmin.y - origin.y)*dirfrac.y;
	float t4 = (bbox.vmax.y - origin.y)*dirfrac.y;
	float t5 = (bbox.vmin.z - origin.z)*dirfrac.z;
	float t6 = (bbox.vmax.z - origin.z)*dirfrac.z;

	float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behind us
	if (tmax < 0)
	{
		if (t) *t = tmax;
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
		if (t) *t = tmax;
		return false;
	}

	if (t) *t = tmin;
	return true;
}

inline bool IntersectLineAABB(
	const v3& origin, const v3& dir,
	const rboundingbox& bbox,
	float* t = nullptr)
{
	return IntersectLineAABB(origin, dir, bbox, NoninfiniteReciprocal(dir), t);
}

inline bool IntersectLineSegmentAABB(
	const v3& l0, const v3& l1,
	const rboundingbox& bbox,
	const v3& InverseDir,
	float* t = nullptr)
{
	float lt;
	auto ret = IntersectLineAABB(l0, Normalized(l1 - l0), bbox, InverseDir, &lt);
	if (!ret)
		return false;

	if (t) *t = lt;

	if (Square(lt) > MagnitudeSq(l1 - l0))
		return false;

	return true;
}

inline bool IntersectLineSegmentAABB(
	const v3& l0, const v3& l1,
	const rboundingbox& bbox,
	float* t = nullptr)
{
	return IntersectLineSegmentAABB(l0, l1, bbox, NoninfiniteReciprocal(Normalized(l1 - l0)), t);
}

inline rplane PlaneFromPointNormal(const v3& point, const v3& normal)
{
	return{ normal.x, normal.y, normal.z, -DotProduct(point, normal) };
}

inline rplane PlaneFromPoints(const v3& a, const v3& b, const v3& c)
{
	auto normal = Normalized(CrossProduct(b - a, c - a));
	return{ EXPAND_VECTOR(normal), -DotProduct(normal, a) };
}

inline rmatrix ViewMatrix(const rvector& Position, const rvector& Direction, const rvector& Up)
{
	auto z = Normalized(Direction);
	auto x = Normalized(CrossProduct(Up, z));
	auto y = CrossProduct(z, x);

#define VIEWMATRIX_ROW(e) x.e, y.e, z.e, 0

	return{
		VIEWMATRIX_ROW(x),
		VIEWMATRIX_ROW(y),
		VIEWMATRIX_ROW(z),
		-DotProduct(x, Position), -DotProduct(y, Position), -DotProduct(z, Position), 1 };

#undef VIEWMATRIX_ROW
}

inline rmatrix PerspectiveProjectionMatrix(float AspectRatio, float FovY, float Near, float Far)
{
	auto yScale = 1.0f / tan(FovY / 2);
	auto xScale = yScale / AspectRatio;

	return{
		xScale, 0, 0, 0,
		0, yScale, 0, 0,
		0, 0, Far / (Far - Near), 1,
		0, 0, -Near*Far / (Far - Near), 0 };
}

inline rmatrix PerspectiveProjectionMatrixViewport(float ScreenWidth, float ScreenHeight,
	float FOV, float Near, float Far)
{
	float Aspect = ScreenWidth / ScreenHeight;
	float FovY = atan(tan(FOV / 2.0f) / Aspect) * 2.0f;

	return PerspectiveProjectionMatrix(Aspect, FovY, Near, Far);
}

inline rmatrix TranslationMatrix(const v3& Trans)
{
	rmatrix mat = GetIdentityMatrix();
	SetTransPos(mat, Trans);
	return mat;
}

v3 CatmullRomSpline(const v3& v0, const v3& v1, const v3& v2, const v3& v3, float s);

bool isnan(const v3& vec);
bool isinf(const v3& vec);
bool isnan(const rmatrix& mat);
bool isinf(const rmatrix& mat);

inline rplane ComputeZPlane(float z, float sign, const v3& Pos, const v3& Dir)
{
	rplane ret;
	auto t = Pos + z * Dir;
	auto normal = Normalized(sign * Dir);

	return{ EXPAND_VECTOR(normal), -DotProduct(normal, t) };
}

inline float ComputeVerticalFOV(float HorizontalFOV, float Aspect) {
	return atan(tan(HorizontalFOV / 2.0f) / Aspect) * 2.0f;
}

inline rfrustum MakeViewFrustum(
	const rmatrix& View,
	const v3& Pos, const v3& Dir,
	float HorizontalFOV, float VerticalFOV,
	float Near, float Far)
{
	auto ComputeForwardPlane = [&](float x, float y, float z)
	{
		v3 normal{
			View._11*x + View._12*y + View._13*z,
			View._21*x + View._22*y + View._23*z,
			View._31*x + View._32*y + View._33*z
		};

		return rplane{ EXPAND_VECTOR(normal), -DotProduct(normal, Pos) };
	};

	float fovh2 = HorizontalFOV / 2.0f;
	float fovv2 = VerticalFOV / 2.0f;
	float ch = cos(fovh2), sh = sin(fovh2);
	float cv = cos(fovv2), sv = sin(fovv2);

	return{
		ComputeForwardPlane(-ch, 0, sh),
		ComputeForwardPlane(ch, 0, sh),
		ComputeForwardPlane(0, cv, sv),
		ComputeForwardPlane(0, -cv, sv),
		ComputeZPlane(Near, 1, Pos, Dir),
		ComputeZPlane(Far, -1, Pos, Dir),
	};
}

_NAMESPACE_REALSPACE2_END
