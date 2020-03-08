// dpoint class ( double precision version of point3 class from max sdk ) by dubble.

#ifndef _DMATH_H 
#define _DMATH_H

#include <math.h>
#include <io.h>

#include <crtdbg.h>

class dpoint {
public:
	double x,y,z;

	// Constructors
	dpoint(){}
	dpoint(float X, float Y, float Z)  { x = (double)X; y = (double)Y; z = (double)Z; }
	dpoint(double X, double Y, double Z) { x = X; y = Y; z = Z; }
	dpoint(int X, int Y, int Z) { x = (double)X; y = (double)Y; z = (double)Z; }
	dpoint(const dpoint& a) { x = a.x; y = a.y; z = a.z; } 
	dpoint(double af[3]) { x = af[0]; y = af[1]; z = af[2]; }

	// Access operators
	double& operator[](int i) { return (&x)[i]; }     
	const double& operator[](int i) const { return (&x)[i]; }  

	// Conversion function
	operator double*() { return(&x); }

	// Unary operators
	dpoint operator-() const { return(dpoint(-x,-y,-z)); } 
	dpoint operator+() const { return *this; }

	// Property functions
	double Length() const;
	double LengthSquared() const;

	// Assignment operators
	inline dpoint& operator-=(const dpoint&);
	inline dpoint& operator+=(const dpoint&);
	inline dpoint& operator*=(double); 
	inline dpoint& operator/=(double);

	// Test for equality
	int operator==(const dpoint& p) const { return ((p.x==x)&&(p.y==y)&&(p.z==z)); }
	int operator!=(const dpoint& p) const { return ((p.x!=x)||(p.y!=y)||(p.z!=z)); }
	int Equals(const dpoint& p, float epsilon = 1E-6f) const;

	// Binary operators
	inline  dpoint operator-(const dpoint&) const;
	inline  dpoint operator+(const dpoint&) const;


	void Normalize();

	size_t SaveFloat(FILE *file) { 
		float p[3]= { (float)x,(float)y,(float)z };
//		_ASSERT(fabs(p[0]-1655.4756)>1.f);
		return fwrite(p,sizeof(p),1,file);
	}
};

inline double dpoint::Length() const {	
	return sqrt(x*x+y*y+z*z);
}

inline double dpoint::LengthSquared() const {	
	return (x*x+y*y+z*z);
}

inline double Length(const dpoint& v) {	
	return v.Length();
}

inline double LengthSquared(const dpoint& v) {	
	return v.LengthSquared();
}

inline dpoint& dpoint::operator-=(const dpoint& a) {	
	x -= a.x;	y -= a.y;	z -= a.z;
	return *this;
}

inline dpoint& dpoint::operator+=(const dpoint& a) {
	x += a.x;	y += a.y;	z += a.z;
	return *this;
}

inline dpoint& dpoint::operator*=(double f) {
	x *= f;   y *= f;	z *= f;
	return *this;
}

inline dpoint& dpoint::operator/=(double f) { 
	x /= f;	y /= f;	z /= f;	
	return *this; 
}

inline void dpoint::Normalize() {
	double length=Length();
	x /= length;
	y /= length;
	z /= length;
}

inline dpoint dpoint::operator-(const dpoint& b) const {
	return(dpoint(x-b.x,y-b.y,z-b.z));
}

inline dpoint dpoint::operator+(const dpoint& b) const {
	return(dpoint(x+b.x,y+b.y,z+b.z));
}

inline dpoint operator*(double f, const dpoint& a) {
	return(dpoint(a.x*f, a.y*f, a.z*f));
}

inline dpoint operator*(const dpoint& a, double f) {
	return(dpoint(a.x*f, a.y*f, a.z*f));
}

inline dpoint operator/(const dpoint& a, double f) {
	return(dpoint(a.x/f, a.y/f, a.z/f));
}

inline dpoint operator+(const dpoint& a, double f) {
	return(dpoint(a.x+f, a.y+f, a.z+f));
}

inline dpoint Normalize(const dpoint& v) { 
	return v/Length(v);
}

inline double DotProduct(const dpoint& a, const dpoint& b) { 
	return(a.x*b.x+a.y*b.y+a.z*b.z);	
}

inline dpoint CrossProduct(const dpoint& v1, const dpoint& v2) { 
	return dpoint(v1.y * v2.z - v1.z * v2.y ,
					v1.z * v2.x - v1.x * v2.z,
					v1.x * v2.y - v1.y * v2.x);
}

inline dpoint InterpolatedVector(dpoint &a,dpoint &b,double x)
{
	double ab,theta,theta1,theta2,costheta1,costheta2,u,v;

	ab=min(max(DotProduct(a,b),-1.),1.);
	if(ab==1.0f) return b;	// 각도가 0이면 그냥 리턴

	//	else if(ab==-1.0f) return TransformVector(a, RotateZMatrix(pi*x));		// 각도가 180이면 Z축으로 회전한다.

	theta=acos(ab);
	//if(theta==0.0f) return a;	// 0도이면 a 리턴

	theta1=theta*x;
	theta2=theta*(1.0f-x);
	costheta1=cos(theta1);
	costheta2=cos(theta2);
	u=costheta1-ab*costheta2;
	v=costheta2-ab*costheta1;
	double D = (1.0f-ab*ab);
	//	_ASSERT(D!=0.0f);	// 앞에서 ab를 검사하므로 0이 나올 수 없다.
	if(D==0) return a;

	dpoint vReturn=(1.0f/D*(u*a+v*b));
	//	_ASSERT(!_isnan(vReturn.z));
	return vReturn;
}


class dplane {
public:
	double a,b,c,d;

	dplane(){}
	dplane(double A,double B,double C,double D) {
		a=A;b=B;c=C;d=D;
	}

	operator double*() { return(&a); }

	dplane operator-() const { return(dplane(-a,-b,-c,-d)); } 

	size_t SaveFloat(FILE *file) { 
		float p[4]= { (float)a,(float)b,(float)c,(float)d };
		return fwrite(p,sizeof(p),1,file);
	}

};

inline double DPlaneDotCoord(const dplane &plane,const dpoint &point) {
	return plane.a*point.x+plane.b*point.y+plane.c*point.z+plane.d;
}

inline double DPlaneDotNormal(const dplane &plane,const dpoint &point) {
	return plane.a*point.x+plane.b*point.y+plane.c*point.z;
}

inline dplane *DPlaneFromPointNormal(dplane *pOut,const dpoint &apoint,const dpoint &normal) {
	pOut->a=normal.x;
	pOut->b=normal.y;
	pOut->c=normal.z;
	pOut->d=-DPlaneDotNormal(*pOut,apoint);
	return pOut;
}

inline dplane *DPlaneFromPoints(dplane *pOut,const dpoint &p1,const dpoint &p2,const dpoint &p3) {
	dpoint normal=CrossProduct(p2-p1,p3-p2);
	normal.Normalize();
	return DPlaneFromPointNormal(pOut,p1,normal);
}


typedef union {
	struct {
		double minx,miny,minz,maxx,maxy,maxz;
	};
	struct {
		dpoint vmin,vmax;
	};
	double m[2][3];

	size_t SaveFloat(FILE *file) { 
		float p[6]= { (float)minx,(float)miny,(float)minz,(float)maxx,(float)maxy,(float)maxz };
		return fwrite(p,sizeof(p),1,file);
	}

} dboundingbox;

inline void MergeBoundingBox(dboundingbox *dest,dboundingbox *src)
{
	for(int i=0;i<3;i++)
	{
		dest->vmin[i]=min(dest->vmin[i],src->vmin[i]);
		dest->vmax[i]=max(dest->vmax[i],src->vmax[i]);
	}
}


inline double GetArea(dpoint &v1,dpoint &v2,dpoint &v3) {
	double a,b,c;	// 삼각형의 세변의 길이.
	a=Length(v1-v2);
	b=Length(v2-v3);
	c=Length(v3-v1);

	double p=(a+b+c)/2;
	return sqrt(p*(p-a)*(p-b)*(p-c));
}

inline dpoint GetPlaneIntersectLine(dplane &plane,dpoint &v1,dpoint &v2,double *t) {
	double d[3]= { v2.x-v1.x, v2.y-v1.y, v2.z-v1.z };
	*t=-(double)(v1.x*plane.a+v1.y*plane.b+v1.z*plane.c+plane.d)/
		(double)(d[0]*plane.a+d[1]*plane.b+d[2]*plane.c);

	return dpoint(v1.x+d[0]**t,v1.y+d[1]**t,v1.z+d[2]**t);
}

#endif