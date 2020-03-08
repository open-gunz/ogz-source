#include "stdafx.h"
#include "RMath.h"
#include <cmath>

_NAMESPACE_REALSPACE2_BEGIN

float GetDistance(const rboundingbox &bb, const rvector &point)
{
	rvector closest;

	if (bb.minx <= point.x && point.x <= bb.maxx) closest.x = point.x;
	else closest.x = fabs(point.x - bb.minx) < fabs(point.x - bb.maxx) ? bb.minx : bb.maxx;

	if (bb.miny <= point.y && point.y <= bb.maxy) closest.y = point.y;
	else closest.y = fabs(point.y - bb.miny) < fabs(point.y - bb.maxy) ? bb.miny : bb.maxy;

	if (bb.minz <= point.z && point.z <= bb.maxz) closest.z = point.z;
	else closest.z = fabs(point.z - bb.minz) < fabs(point.z - bb.maxz) ? bb.minz : bb.maxz;

	return Magnitude(closest - point);
}

float GetDistance(const rvector &position, const rplane &plane)
{
	return abs(DotProduct(plane, position));
}

rvector GetNearestPoint(const rvector &a, const rvector &aa, const rplane &plane)
{
	rvector b = aa - a;

	float fDot = DotPlaneNormal(plane, b);
	if (IS_ZERO(fDot))
		return a;

	float t = -(plane.d + DotPlaneNormal(plane, a)) / fDot;
	if (t<0) t = 0;
	if (t>1) t = 1;

	return a + t*b;
}

float GetDistance(const rvector &a, const rvector &aa, const rplane &plane)
{
	return GetDistance(GetNearestPoint(a, aa, plane), plane);
}

float GetDistance(const rvector &position, const rvector &line1, const rvector &line2)
{
	rvector a = position - line1;
	rvector b = line2 - line1;
	float asq = DotProduct(a, a);
	float bsq = DotProduct(b, b);
	return sqrtf(asq - powf(DotProduct(a, b), 2) / bsq);
}

rvector GetNearestPoint(const rvector &position, const rvector &a, const rvector &b)
{
	rvector dir = b - a;

	float d = -DotProduct(position, dir);

	float mdir = MagnitudeSq(dir);
	if (mdir<0.001f) return a;

	float t = -(DotProduct(a, dir) + d) / mdir;

	if (t<0) t = 0;
	if (t>1) t = 1;

	return a + t*dir;
}

float GetDistanceLineSegment(const rvector &position, const rvector &a, const rvector &b)
{
	return Magnitude(GetNearestPoint(position, a, b) - position);
}

float GetDistanceBetweenLineSegment(const rvector &a, const rvector &aa,
	const rvector &c, const rvector &cc, rvector *ap, rvector *cp)
{
	rvector b = aa - a;
	rvector d = cc - c;

	rvector cm;
	CrossProduct(&cm, b, d);

	float fMagnitude = Magnitude(cm);

	if (fMagnitude<1)
	{

		rvector x;

		rvector edge = a - c;
		float temp = DotProduct(edge, d) / Magnitude(d);
		rvector dir = d;
		Normalize(dir);
		x = c + temp*dir - a;

		float st0, st1;

		st0 = DotProduct(a + x - c, d) / DotProduct(d, d);
		st1 = DotProduct(aa + x - c, d) / DotProduct(d, d);

		if (st0<0 && st1<0)
		{
			*cp = c;
			if (fabs(st0)>fabs(st1))
				*ap = aa;
			else
				*ap = a;
		}
		else
			if (st0>1 && st1>1)
			{
				*cp = cc;
				if (fabs(st0)>fabs(st1))
					*ap = aa;
				else
					*ap = a;
			}
			else
			{
				if (st0 >= 0 && st0 <= 1)
				{
					*ap = a;
					*cp = c + st0*d;
				}
				else
					if (st1 >= 0 && st1 <= 1)
					{
						*ap = aa;
						*cp = c + st1*d;
					}
					else
					{
						*cp = c;
						*ap = *cp - x;
					}
			}
		return Magnitude(*ap - *cp);
	}

	cm /= fMagnitude;

	rvector r;
	CrossProduct(&r, d, cm);
	Normalize(r);

	float t = (DotProduct(r, c) - DotProduct(r, a)) / DotProduct(r, b);

	CrossProduct(&r, b, cm);
	Normalize(r);

	float s = (DotProduct(r, a) - DotProduct(r, c)) / DotProduct(r, d);

	if (t<0) t = 0;
	if (t>1) t = 1;

	if (s<0) s = 0;
	if (s>1) s = 1;

	*ap = a + t*b;
	*cp = c + s*d;
	return Magnitude(*ap - *cp);
}



float GetDistance(const rboundingbox& bb, const rplane& plane)
{
	float a, b, c;
	a = (plane.a > 0) ? bb.m[1][0] : bb.m[0][0];
	b = (plane.b > 0) ? bb.m[1][1] : bb.m[0][1];
	c = (plane.c > 0) ? bb.m[1][2] : bb.m[0][2];
	return plane.a*a + plane.b*b + plane.c*c + plane.d;
}

void GetDistanceMinMax(rboundingbox &bb, rplane &plane, float *MinDist, float *MaxDist)
{
	float a, b, c, a2, b2, c2;
	if (plane.a>0) { a = bb.m[1][0]; a2 = bb.m[0][0]; }
	else { a = bb.m[0][0]; a2 = bb.m[1][0]; }
	if (plane.b>0) { b = bb.m[1][1]; b2 = bb.m[0][1]; }
	else { b = bb.m[0][1]; b2 = bb.m[1][1]; }
	if (plane.c>0) { c = bb.m[1][2]; c2 = bb.m[0][2]; }
	else { c = bb.m[0][2]; c2 = bb.m[1][2]; }
	*MaxDist = plane.a*a + plane.b*b + plane.c*c + plane.d;
	*MinDist = plane.a*a2 + plane.b*b2 + plane.c*c2 + plane.d;
}

float GetArea(rvector &v1, rvector &v2, rvector &v3)
{
	float a, b, c;
	a = Magnitude(v1 - v2);
	b = Magnitude(v2 - v3);
	c = Magnitude(v3 - v1);

	float p = (a + b + c) / 2;
	return sqrt(p*(p - a)*(p - b)*(p - c));
}

bool isInPlane(const rboundingbox& bb, const rplane& plane)
{
	return GetDistance(bb, plane) >= 0;
}

template <typename T>
static auto SlerpImpl(const T& from, const T& to, float t)
{
	auto clamp = [&](auto&& val, auto&& minval, auto&& maxval) {
		return max(minval, min(maxval, val));
	};

	auto costheta = clamp(DotProduct(from, to), -1.f, 1.f);

	if (costheta > 0.999999f)
		return Lerp(from, to, t);

	auto angle = acos(costheta);

	return (sin((1 - t) * angle) * from + sin(t * angle) * to) / sin(angle);
}

v3 Slerp(const v3& from, const v3& to, float t) { return SlerpImpl(from, to, t); }
v2 Slerp(const v2& from, const v2& to, float t) { return SlerpImpl(from, to, t); }
v4 Slerp(const v4& from, const v4& to, float t) { return SlerpImpl(from, to, t); }

rquaternion Slerp(const rquaternion& from, const rquaternion& to, float t)
{
	auto temp = 1.0f - t;
	auto costheta = DotProduct(from, to);

	if (costheta < 0.0f)
	{
		t = -t;
		costheta = -costheta;
	}

	if (costheta < 0.999f)
	{
		auto theta = acos(costheta);

		temp = sin(theta * temp) / sinf(theta);
		t = sin(theta * t) / sinf(theta);
	}

	return{
		temp * from.x + t * to.x,
		temp * from.y + t * to.y,
		temp * from.z + t * to.z,
		temp * from.w + t * to.w,
	};
}

bool IsIntersect(const rboundingbox& bb1, const rboundingbox& bb2)
{
	if (bb1.minx > bb2.maxx) return false;
	if (bb1.miny > bb2.maxy) return false;
	if (bb1.minz > bb2.maxz) return false;
	if (bb2.minx > bb1.maxx) return false;
	if (bb2.miny > bb1.maxy) return false;
	if (bb2.minz > bb1.maxz) return false;

	return true;
}

bool IsInSphere(const rboundingbox &bb, const rvector &point, float radius)
{
	for (int i = 0; i<3; i++)
	{
		rvector nearest;

		if (fabs(bb.m[0][i] - point[i])<fabs(bb.m[1][i] - point[i]))
			nearest[i] = bb.m[0][i];
		else
			nearest[i] = bb.m[1][i];

		int au = (i + 1) % 3, av = (i + 2) % 3;

		nearest[au] = min(max(point[au], bb.m[0][au]), bb.m[1][au]);
		nearest[av] = min(max(point[av], bb.m[0][av]), bb.m[1][av]);

		if (Magnitude(nearest - point) < radius)
			return true;
	}

	return (bb.minx <= point.x && bb.miny <= point.y && bb.minz <= point.z &&
		bb.maxx >= point.x && bb.maxy >= point.y && bb.maxz >= point.z);
}

bool isInViewFrustum(const rvector &point, const rfrustum& frustum)
{
#define FN(i) frustum[i].a * point.x + frustum[i].b * point.y + frustum[i].c * point.z+frustum[i].d >= 0
	return FN(0) && FN(1) && FN(2) && FN(3);
}

bool isInViewFrustum(const rvector &point, float radius, const rfrustum& frustum)
{
	return
		DotProduct(frustum[0], point) > -radius &&
		DotProduct(frustum[1], point) > -radius &&
		DotProduct(frustum[2], point) > -radius &&
		DotProduct(frustum[3], point) > -radius &&
		DotProduct(frustum[5], point) > -radius;
}

bool isInViewFrustum(const rboundingbox& bb, const rfrustum& frustum)
{
	return
		isInPlane(bb, frustum[0]) &&
		isInPlane(bb, frustum[1]) &&
		isInPlane(bb, frustum[2]) &&
		isInPlane(bb, frustum[3]);
}

bool isInViewFrustum(const rvector &point1, const rvector &point2, const rfrustum& frustum)
{
	rvector p1 = point1, p2 = point2;

	for (int i = 0; i < 6; i++) {
		auto&& plane = frustum[i];
		float d1 = DotProduct(plane, p1);
		float d2 = DotProduct(plane, p2);
		rsign s1 = SIGNOF(d1);
		rsign s2 = SIGNOF(d2);

		if (s1 == NEGATIVE && s2 == NEGATIVE) return false;

		if (s1*s2 == NEGATIVE) {
			float t = d1 / (d1 - d2);
			rvector inter = p1 + t * (p2 - p1);
			if (s1 == NEGATIVE)
				p1 = inter;
			else
			{
				assert(s2 == NEGATIVE);
				p2 = inter;
			}
		}
	}

	if (MagnitudeSq(p2 - p1) > .01f)
		return true;

	return false;
}

bool isInViewFrustumWithZ(const rboundingbox& bb, const rfrustum& frustum)
{
	return
		isInPlane(bb, frustum[0]) &&
		isInPlane(bb, frustum[1]) &&
		isInPlane(bb, frustum[2]) &&
		isInPlane(bb, frustum[3]) &&
		isInPlane(bb, frustum[4]) &&
		isInPlane(bb, frustum[5]);
}

bool isInViewFrustumWithFarZ(const rboundingbox& bb, const rfrustum& frustum)
{
	return
		isInPlane(bb, frustum[0]) &&
		isInPlane(bb, frustum[1]) &&
		isInPlane(bb, frustum[2]) &&
		isInPlane(bb, frustum[3]) &&
		isInPlane(bb, frustum[5]);
}

bool isInViewFrustumwrtnPlanes(const rboundingbox& bb, const rfrustum& frustum, int nplane)
{
	for (int i = 0; i < nplane; i++)
		if (!isInPlane(bb, frustum[i]))
			return false;

	return true;
}

void MergeBoundingBox(rboundingbox *dest, rboundingbox *src)
{
	for (int i = 0; i<3; i++)
	{
		dest->vmin[i] = min(dest->vmin[i], src->vmin[i]);
		dest->vmax[i] = max(dest->vmax[i], src->vmax[i]);
	}
}

bool isLineIntersectBoundingBox(rvector &origin, rvector &dir, rboundingbox &bb)
{
	return IntersectLineAABB(origin, dir, bb);
}

void MakeWorldMatrix(rmatrix *pOut, const rvector& pos, rvector dir, rvector up)
{
	GetIdentityMatrix(*pOut);

	rvector right;

	Normalize(dir);

	CrossProduct(&right, dir, up);
	Normalize(right);

	CrossProduct(&up, right, dir);
	Normalize(up);

	pOut->_11 = right.x;
	pOut->_12 = right.y;
	pOut->_13 = right.z;

	pOut->_21 = up.x;
	pOut->_22 = up.y;
	pOut->_23 = up.z;

	pOut->_31 = -dir.x;
	pOut->_32 = -dir.y;
	pOut->_33 = -dir.z;

	pOut->_41 = pos.x;
	pOut->_42 = pos.y;
	pOut->_43 = pos.z;
}

bool IsIntersect(rvector& line_begin_, rvector& line_end_, rboundingbox& box_)
{
	// transform line to box space
	auto box_center = (box_.vmin + box_.vmax) * 0.5;
	auto begin = line_begin_ - box_center;
	auto end = line_end_ - box_center;
	float hx, hy, hz;
	hx = (box_.maxx - box_.minx) * 0.5f;
	hy = (box_.maxy - box_.miny) * 0.5f;
	hz = (box_.maxz - box_.minz) * 0.5f;

	// get center of line
	auto center = (begin + end) * 0.5f;

	float wx = begin.x - center.x;
	float lengthX = fabs(wx);
	if (fabs(center.x)  > lengthX)
		return false;

	float wy = begin.y - center.y;
	float lengthY = fabs(wy);
	if (fabs(center.y)  > lengthY)
		return false;

	float wz = begin.z - center.z;
	float lengthZ = fabs(wz);
	if (fabs(center.z)  > lengthZ)
		return false;

	if (fabs(center.y * wz - center.z * wy) > hy * lengthZ + hz * lengthY)
		return false;
	if (fabs(center.x * wz - center.z * wx) > hx * lengthZ + hz * lengthX)
		return false;
	if (fabs(center.x * wy - center.y * wx) > hx * lengthY + hy * lengthX)
		return false;

	return true;
}

bool IsIntersect(rvector& line_begin_, rvector& line_dir_, rvector& center_,
	float radius_, float* dist, rvector* p)
{
	rvector	l = center_ - line_begin_;
	float		s = DotProduct(l, line_dir_);
	float l_sq = DotProduct(l, l);
	float r_sq = (radius_ * radius_);
	if (s < 0 && l_sq > r_sq)
		return false;
	float m_sq = l_sq - s*s;
	if (m_sq > r_sq)
		return false;
	if (dist == NULL || p == NULL)
		return true;
	float q = sqrt(r_sq - m_sq);
	if (l_sq > r_sq)
		*dist = s - q;
	*p = line_begin_ + (*dist) * line_dir_;
	return true;
}

static float GetAngle(const rvector &a)
{
	if (a.x >= 1.0f) return 0.0f;
	if (a.x <= -1.0f) return -PI_FLOAT;
	if (a.y>0)
		return acos(a.x);
	else
		return -acos(a.x);
}

float GetAngleOfVectors(const rvector &ta, const rvector &tb)
{
	if ((ta.x == 0.0f) && (ta.y == 0.0f) && (ta.z == 0.0f))
		return 0;
	if ((tb.x == 0.0f) && (tb.y == 0.0f) && (tb.z == 0.0f))
		return 0;

	rvector a = ta, b = tb;
	a.z = 0; Normalize(a); b.z = 0; Normalize(b);

	float aa = GetAngle(a);
	float x, y;
	x = b.x*cos(aa) + b.y*sin(aa);
	y = b.x*(-sin(aa)) + b.y*cos(aa);

	float ret = GetAngle(rvector(x, y, 0));
	return ret;
}

bool IsIntersect(const rvector& orig, const rvector& dir, const rvector& center,
	float radius, rvector* p)
{
	rvector center2orig = orig - center;
	float r2 = radius*radius;
	float tmp1 = DotProduct(center2orig, dir);
	float tmp2 = DotProduct(center2orig, center2orig);
	if (tmp1 < 0 && tmp2 > r2) return false;
	float tmp3 = tmp2 - tmp1;
	if (tmp3 > r2) return false;

	if (p == NULL) return true;

	float q = sqrtf(r2 - tmp3*tmp3);
	float t;
	if (tmp2>r2) t = tmp1 - q;
	else t = tmp1 + q;

	*p = orig + t*dir;
	return true;
}

bool GetIntersectionOfTwoPlanes(rvector *pOutDir, rvector *pOutAPoint, const rplane &plane1, const rplane &plane2)
{
	rvector n1 = rvector(plane1.a, plane1.b, plane1.c);
	rvector n2 = rvector(plane2.a, plane2.b, plane2.c);

	rvector dir;
	CrossProduct(&dir, n1, n2);

	if (IS_ZERO(DotProduct(dir, dir))) return false;

	float determinant = DotProduct(n1, n1)*DotProduct(n2, n2) - DotProduct(n1, n2)*DotProduct(n1, n2);
	float c1 = (-plane1.d*DotProduct(n2, n2) + plane2.d*DotProduct(n1, n2)) / determinant;
	float c2 = (-plane2.d*DotProduct(n1, n1) + plane1.d*DotProduct(n1, n2)) / determinant;

	*pOutAPoint = c1*n1 + c2*n2;
	*pOutDir = dir;

	return true;
}

void TransformBox(rboundingbox* result, const rboundingbox& src, const rmatrix& matrix)
{
	rvector pts[8];
	for (int i = 0; i < 8; i++)
		pts[i] = GetPoint(src, i);

	result->vmin = rvector(3.3e33f, 3.3e33f, 3.3e33f);
	result->vmax = rvector(-3.3e33f, -3.3e33f, -3.3e33f);

	for (int i = 0; i < 8; i++)
	{
		rvector tmp = pts[i];
		tmp *= matrix;
		*result = Union(*result, tmp);
	}
}

v3 CatmullRomSpline(const v3& V0, const v3& V1, const v3& V2, const v3& V3, float s)
{
	return{
		0.5f * (2.0f * V1.x + (V2.x - V0.x) * s +
		(2.0f *V0.x - 5.0f * V1.x + 4.0f * V2.x - V3.x) * s * s +
			(V3.x - 3.0f * V2.x + 3.0f * V1.x - V0.x) * s * s * s),

		0.5f * (2.0f * V1.y + (V2.y - V0.y) * s +
			(2.0f *V0.y - 5.0f * V1.y + 4.0f * V2.y - V3.y) * s * s +
			(V3.y - 3.0f * V2.y + 3.0f * V1.y - V0.y) * s * s * s),
		
		0.5f * (2.0f * V1.z + (V2.z - V0.z) * s +
			(2.0f *V0.z - 5.0f * V1.z + 4.0f * V2.z - V3.z) * s * s +
			(V3.z - 3.0f * V2.z + 3.0f * V1.z - V0.z) * s * s * s) };
}

bool isnan(const v3& vec) {
	return std::isnan(vec.x) || std::isnan(vec.y) || std::isnan(vec.z);
}
bool isinf(const v3& vec) {
	return std::isinf(vec.x) || std::isinf(vec.y) || std::isinf(vec.z);
}

bool isnan(const rmatrix & mat)
{
	for (size_t i{}; i < 4; ++i)
		for (size_t j{}; j < 4; ++j)
			if (std::isnan(mat(i, j)))
				return true;
	return false;
}

bool isinf(const rmatrix & mat)
{
	for (size_t i{}; i < 4; ++i)
		for (size_t j{}; j < 4; ++j)
			if (std::isinf(mat(i, j)))
				return true;
	return false;
}

_NAMESPACE_REALSPACE2_END