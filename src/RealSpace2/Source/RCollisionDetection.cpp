#include "stdafx.h"
#include "RCollisionDetection.h"

_NAMESPACE_REALSPACE2_BEGIN

float SweepTest(const rsphere& sphere, const rvector& vec, const rplane &pln, rplane* out)
{
	float d1, d2;
	rvector pln_normal = rvector(pln.a, pln.b, pln.c);

	d1 = DotProduct(vec, pln_normal);

	if (d1 < 0.0f) {
		d2 = GetDistance(sphere.Pos(), pln) - sphere.radius;

		if (d2 >= 0 && d2 < -d1) {
			float t = d2 / (-d1);

			if (out) {
				*out = rplane(pln.a, pln.b, pln.c, pln.d-sphere.radius);
			}
			return t;
		}
	}

	return 1.0f;
}

static float _RayIntersect(const rvector& pivot, const rvector& pos, const rvector& vec, float radius)
{
	rvector dv;
	float dvl;

	dv = pivot - pos;
	dvl = Magnitude(dv);

	if (dvl < radius)
		return 1.0f;

	float dotp = DotProduct(dv, vec);

	if (dotp / dvl < dvl-radius)
		return 1.0f;

	float vecl = Magnitude(vec);
	float l2 = dotp / vecl;
	float db = (float) sqrt(dvl*dvl - l2*l2);

	if (db > radius)
		return 1.0f;

	float dc = (float) sqrt(radius*radius - db*db);
	float dd = l2 - dc;

	if (dd < vecl)
		return dd / vecl;

	return 1.0f;
}


float SweepTest(const rsphere & sphere, const rvector& vec, const rsphere& body, rplane* out)
{
	float radius = sphere.radius + body.radius, t;
	int i;

	for(i=0; i<3; i++) {
		if (((float*)&vec)[i] > 0.0f) {
			if (((float*)&sphere.x)[i] - radius > ((float*)&body.x)[i] || ((float*)&sphere.x)[i] + ((float*)&vec)[i] + radius < ((float*)&body.x)[i])
				return 1.0f;
		} else {
			if (((float*)&sphere.x)[i] + radius < ((float*)&body.x)[i] || ((float*)&sphere.x)[i] + ((float*)&vec)[i] - radius > ((float*)&body.x)[i])
				return 1.0f;
		}
	}

	t = _RayIntersect(body.Pos(), sphere.Pos(), vec, radius);

	if (t < 1.0f) {
		if (out) {
			rvector norm = (sphere.Pos() + vec * t) - body.Pos();
			Normalize(norm);
			rvector plane_pos = body.Pos() + (norm * body.radius);
			float d = -DotProduct(norm, plane_pos);

			*out = rplane(norm.x, norm.y, norm.z, d);
		}
		return t;
	}

	return 1.0f;
}

float SweepTest(const rsphere& sphere, const rvector& vec, const rcapsule &capsule, rplane* out)
{
	float d1 = DotProduct(capsule.lerp, vec);
	float d2 = DotProduct(capsule.lerp, sphere.Pos() - capsule.pivot);

	if (d1 < 0) {
		if (d2+d1 >= capsule.length+(sphere.radius+capsule.radius))
			return 1.0f;
	}	else {
		if (d2 < -(sphere.radius+capsule.radius))
			return 1.0f;
	}

	rvector p1 = sphere.Pos() - d2 * capsule.lerp;
	rvector v1 = vec - d1 * capsule.lerp;

	float t = _RayIntersect(capsule.pivot, p1, v1, sphere.radius + capsule.radius);

	if (t < 1.0f) {
		rvector p = sphere.Pos() + vec * t;
		float v = DotProduct(capsule.lerp, p - capsule.pivot);

		if (v > 0 && v < capsule.length) {
			if (out) {
				rvector norm = p1 + v1 * t - capsule.pivot;
				Normalize(norm);
				rvector plane_pos = p - norm * sphere.radius;
				float d = -DotProduct(norm, plane_pos);
				*out = rplane(norm.x, norm.y, norm.z, d);
			}

			return t;
		}
	}

	if (d1 > 0) {
		if (d2 < 0) {
			t = _RayIntersect(capsule.pivot, sphere.Pos(), vec, sphere.radius + capsule.radius);

			if (t < 1.0f) {
				if (out) {
					rvector norm = sphere.Pos() + vec * t - capsule.pivot;
					Normalize(norm);
					rvector plane_pos = capsule.pivot + norm * capsule.radius;
					float d = -DotProduct(norm, plane_pos);
					*out = rplane(norm.x, norm.y, norm.z, d);
				}
				return t;
			}
		}
	}	else {
		if (d2 > capsule.length) {
			rvector v2 = capsule.pivot + capsule.lerp;
			t = _RayIntersect(v2, sphere.Pos(), vec, sphere.radius + capsule.radius);

			if (t < 1.0f) {
				if (out) {
					rvector norm = sphere.Pos() + vec * t - v2;
					Normalize(norm);
					rvector plane_pos = v2 + norm * capsule.radius;
					float d = -DotProduct(norm, plane_pos);
					*out = rplane(norm.x, norm.y, norm.z, d);
				}
				return t;
			}
		}	}

	return 1.0f;
}



_NAMESPACE_REALSPACE2_END