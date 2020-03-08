#include "rutils_max.h"

static void	lubksb(rmatrix & a, int *indx, float *b);
static void ludcmp(rmatrix & a, int *indx, float *d);

bool rvertex::isEqual(rvertex &target)
{
	if(!IS_EQ3(target.coord,coord)) return false;
	if(!IS_EQ3(target.normal,normal)) return false;
	if(!IS_EQ(target.u,u)) return false;
	if(!IS_EQ(target.v,v)) return false;
	return true;
}

rmatrix MatrixInverse(const rmatrix & m)
{
	rmatrix	n, y;
	int			i, j, indx[4];
	float		d, col[4];

	n = m;
	ludcmp(n, indx, &d);

	for (j=0; j<4; j++) {
		for (i=0; i<4; i++) {
			col[i] = 0.0f;
		}
		col[j] = 1.0f;
		lubksb(n, indx, col);
		for (i=0; i<4; i++) {
			y(i, j) = col[i];
		}
	}
	return y;
} // end MatrixInverse

/*
**-----------------------------------------------------------------------------
**  Name:       lubksb
**  Purpose:	backward subsitution
**-----------------------------------------------------------------------------
*/

static void 
lubksb(rmatrix & a, int *indx, float *b)
{
	int		i, j, ii=-1, ip;
	float	sum;

	for (i=0; i<4; i++) {
		ip = indx[i];
		sum = b[ip];
		b[ip] = b[i];
		if (ii>=0) {
			for (j=ii; j<=i-1; j++) {
				sum -= a(i, j) * b[j];
			}
		} else if (sum != 0.0) {
			ii = i;
		}
		b[i] = sum;
	}
	for (i=3; i>=0; i--) {
		sum = b[i];
		for (j=i+1; j<4; j++) {
			sum -= a(i, j) * b[j];
		}
		b[i] = sum/a(i, i);
	}
} // end lubksb

/*
**-----------------------------------------------------------------------------
**  Name:       ludcmp
**  Purpose:	LU decomposition
**-----------------------------------------------------------------------------
*/

static void 
ludcmp(rmatrix & a, int *indx, float *d)
{
	float	vv[4];               /* implicit scale for each row */
	float	big, dum, sum, tmp;
	int		i, imax, j, k;

	*d = 1.0f;
	for (i=0; i<4; i++) {
		big = 0.0f;
		for (j=0; j<4; j++) {
			if ((tmp = (float) fabs(a(i, j))) > big) {
				big = tmp;
			}
		}
		/*
		if (big == 0.0f) {
			printf("ludcmp(): singular matrix found...\n");
			exit(1);
		}
		*/
		vv[i] = 1.0f/big;
	}
	for (j=0; j<4; j++) {
		for (i=0; i<j; i++) {
			sum = a(i, j);
			for (k=0; k<i; k++) {
				sum -= a(i, k) * a(k, j);
			}
			a(i, j) = sum;
		}
		big = 0.0f;
		for (i=j; i<4; i++) {
			sum = a(i, j);
			for (k=0; k<j; k++) {
				sum -= a(i, k)*a(k, j);
			}
			a(i, j) = sum;
			if ((dum = vv[i] * (float)fabs(sum)) >= big) {
				big = dum;
				imax = i;
			}
		}
		if (j != imax) {
			for (k=0; k<4; k++) {
				dum = a(imax, k);
				a(imax, k) = a(j, k);
				a(j, k) = dum;
			}
			*d = -(*d);
			vv[imax] = vv[j];
		}
		indx[j] = imax;
		if (a(j, j) == 0.0f) {
			a(j, j) = 1.0e-20f;      /* can be 0.0 also... */
		}
		if (j != 3) {
			dum = 1.0f/a(j, j);
			for (i=j+1; i<4; i++) {
				a(i, j) *= dum;
			}
		}
	}
} // end ludcmp

rmatrix ZeroMatrix44(void)
{
    rmatrix ret;
	memset(&ret,0,sizeof(rmatrix));
    return ret;
} // end ZeroMatrix

rmatrix IdentityMatrix44()
{
	rmatrix r;
	r=ZeroMatrix44();
	r._11=1;r._22=1;r._33=1;r._44=1;
	return r;
}

rmatrix TranslateMatrix44(const float dx, const float dy, const float dz)
{
    rmatrix ret = IdentityMatrix44();
	ret._41 = dx;
	ret._42 = dy;
	ret._43 = dz;
	return ret;
} // end TranslateMatrix

rmatrix ViewMatrix44(const rvector& from, 
		   const rvector& viewdir, 
		   const rvector& world_up)
{
    rmatrix view;
    rvector up, right, dir;

	dir=Normalize(viewdir);
	right = Normalize(CrossProduct(world_up, dir));
	up = Normalize(CrossProduct(dir, right));

    view._11 = right.x;
    view._21 = right.y;
    view._31 = right.z;		view._14=0;
    view._12 = up.x;
    view._22 = up.y;
    view._32 = up.z;		view._24=0;
    view._13 = dir.x;
    view._23 = dir.y;
    view._33 = dir.z;		view._34=0;
	
    view._41 = -DotProduct(right, from);
    view._42 = -DotProduct(up, from);
    view._43 = -DotProduct(dir, from);		view._44=1;

    return view;
} // end ViewMatrix

rmatrix ScaleMatrix44(const float size)
{
    rmatrix ret = IdentityMatrix44();
	ret._11 = size;
	ret._22 = size;
	ret._33 = size;
	return ret;
} // end ScaleMatrix

rmatrix MatrixMult(rmatrix &v1,rmatrix &v2)
{
	rmatrix r;

	r._11=v1._11*v2._11+v1._12*v2._21+v1._13*v2._31+v1._14*v2._41;
	r._12=v1._11*v2._12+v1._12*v2._22+v1._13*v2._32+v1._14*v2._42;
	r._13=v1._11*v2._13+v1._12*v2._23+v1._13*v2._33+v1._14*v2._43;
	r._14=v1._11*v2._14+v1._12*v2._24+v1._13*v2._34+v1._14*v2._44;

	r._21=v1._21*v2._11+v1._22*v2._21+v1._23*v2._31+v1._24*v2._41;
	r._22=v1._21*v2._12+v1._22*v2._22+v1._23*v2._32+v1._24*v2._42;
	r._23=v1._21*v2._13+v1._22*v2._23+v1._23*v2._33+v1._24*v2._43;
	r._24=v1._21*v2._14+v1._22*v2._24+v1._23*v2._34+v1._24*v2._44;

	r._31=v1._31*v2._11+v1._32*v2._21+v1._33*v2._31+v1._34*v2._41;
	r._32=v1._31*v2._12+v1._32*v2._22+v1._33*v2._32+v1._34*v2._42;
	r._33=v1._31*v2._13+v1._32*v2._23+v1._33*v2._33+v1._34*v2._43;
	r._34=v1._31*v2._14+v1._32*v2._24+v1._33*v2._34+v1._34*v2._44;

	r._41=v1._41*v2._11+v1._42*v2._21+v1._43*v2._31+v1._44*v2._41;
	r._42=v1._41*v2._12+v1._42*v2._22+v1._43*v2._32+v1._44*v2._42;
	r._43=v1._41*v2._13+v1._42*v2._23+v1._43*v2._33+v1._44*v2._43;
	r._44=v1._41*v2._14+v1._42*v2._24+v1._43*v2._34+v1._44*v2._44;

	return r;
}

rvector TransformVector(rvector &v,rmatrix &m)
{
	static float sx,sy,sz;
	rvector r;

	sx=m._11*v.x+m._21*v.y+m._31*v.z+m._41;
	sy=m._12*v.x+m._22*v.y+m._32*v.z+m._42;
	sz=m._13*v.x+m._23*v.y+m._33*v.z+m._43;
	r.x=sx;
	r.y=sy;
	r.z=sz;
	return r;
}
