#ifndef __RUTILS_MAX_H
#define __RUTILS_MAX_H

#include "max.h"

#define Infinite 99999999999999999.0f
#define TOLER 0.0000076
#define IS_EQ(a,b) ((fabs((double)(a)-(b)) >= (double) TOLER) ? 0 : 1)
#define IS_EQ3(a,b) (IS_EQ((a).x,(b).x)&&IS_EQ((a).y,(b).y)&&IS_EQ((a).z,(b).z))

typedef union {
	struct {
		float Minx,Maxx,Miny,Maxy,Minz,Maxz;
	};
	float m[3][2];

	inline Reset() { m[0][0]=m[1][0]=m[2][0]=Infinite;m[0][1]=m[1][1]=m[2][1]=-Infinite; }

} rboundingbox;

struct rvector {
public:
	rvector() {}
	rvector(Point3 p) {x=-p.x;y=p.y;z=p.z;};
	void Normalize(){float mag=(float)sqrt(x*x+y*y+z*z);x=x/mag;y=y/mag;z=z/mag;}
	union{
		struct { float x,y,z; };
		float v[3];
	};
	inline friend rvector operator - (const rvector &v) { return rvector(-v.x,-v.y,-v.z); }
	inline float GetMagnitude() { return (float)sqrt(x*x+y*y+z*z); }
	inline friend rvector Normalize(const rvector & v){
		rvector r;
		float t;

		t = ((rvector*)&v)->GetMagnitude();
		r.x = v.x/t;
		r.y = v.y/t;
		r.z = v.z/t;
		return r;
	};
	inline friend float DotProduct (const rvector& v1, const rvector& v2) { return v1.x*v2.x + v1.y * v2.y + v1.z*v2.z; }
	inline friend rvector CrossProduct (const rvector& v1, const rvector& v2)
	{
		rvector result;
		result.x = v1.y * v2.z - v1.z * v2.y;
		result.y = v1.z * v2.x - v1.x * v2.z;
		result.z = v1.x * v2.y - v1.y * v2.x;
		return result;
	};
	rvector(float _x,float _y,float _z) { x=_x;y=_y;z=_z;}
	inline friend rvector operator - (const rvector& v1, const rvector& v2) { return rvector(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);	}
};

inline void operator +=(rvector &v,Point3 &p) {v.x+=-p.x;v.y+=p.y;v.z+=p.z;}
inline void operator +=(rvector &v,rvector &p) {v.x+=p.x;v.y+=p.y;v.z+=p.z;}

struct rvertex {
	rvector coord,normal;
	float u,v;

public:
	bool isEqual(rvertex &);
};

struct rmatrix {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
	};
			rmatrix(){_11=_12=_13=_14=_21=_22=_23=_24=
				_31=_32=_33=_34=_41=_42=_43=_44=0.0f;}
   float& operator()(int iRow, int iColumn) { return m[iRow][iColumn]; }
    const float& operator()(int iRow, int iColumn) const { return m[iRow][iColumn]; }
};

rmatrix IdentityMatrix44();
rmatrix MatrixInverse(const rmatrix & m);
rmatrix ScaleMatrix44(const float size);
rmatrix TranslateMatrix44(const float dx, const float dy, const float dz);
rmatrix ViewMatrix44(const rvector& from,const rvector& viewdir,const rvector& world_up);
rmatrix MatrixMult(rmatrix &m1,rmatrix &m2);
inline rmatrix operator * (rmatrix& m1,rmatrix& m2) { return MatrixMult(m1,m2); }
rvector	TransformVector(rvector &v,rmatrix &m);

#endif