#pragma once

#include "RTypes.h"
#include "RMath.h"

class rline2d
{
private:
	float SignedDistance(const rvector2& point) const;
public:
	rvector2	start;
	rvector2	end;

	// creators
	rline2d() : start(0,0), end(1,1) {};
	rline2d(float xa, float ya, float xb, float yb) : start(xa, ya), end(xb, yb) {};
	rline2d(const rvector2& _start, const rvector2& _end) : start(_start), end(_end) {};
	rline2d(const rline2d& other) : start(other.start), end(other.end) {};

	// operators
	rline2d operator+(const rvector2& point) const { return rline2d(start + point, end + point); };
	rline2d& operator+=(const rvector2& point) { start += point; end += point; return *this; };
	rline2d operator-(const rvector2& point) const { return rline2d(start - point, end - point); };
	rline2d& operator-=(const rvector2& point) { start -= point; end -= point; return *this; };
	bool operator==(const rline2d& other) const { return (start==other.start && end==other.end) || (end==other.start && start==other.end);};
	bool operator!=(const rline2d& other) const { return !(start==other.start && end==other.end) || (end==other.start && start==other.end);};

	void SetLine(const float& xa, const float& ya, const float& xb, const float& yb) { start.x = xa; start.y = ya; end.x = xb; end.y = yb; }
	void SetLine(const rvector2& nstart, const rvector2& nend) {start = nstart; end = nend; }
	void SetLine(const rline2d& line) { start=line.start; end=line.end; }
	
	float GetLength() const
	{
		float xdist = end.x-start.x;
		float ydist = end.y-start.y;

		xdist *= xdist;
		ydist *= ydist;

		return static_cast<float>(sqrt(xdist + ydist));
	}
	rvector2 GetVector() const { return rvector2(start.x - end.x, start.y - end.y); }

public:
	enum POINT_CLASSIFICATION
	{
		ON_LINE,		// The point is on, or very near, the line
		LEFT_SIDE,		// looking from endpoint A to B, the test point is on the left
		RIGHT_SIDE		// looking from endpoint A to B, the test point is on the right
	};

	enum LINE_CLASSIFICATION
	{
		COLLINEAR,			// both lines are parallel and overlap each other
		LINES_INTERSECT,	// lines intersect, but their segments do not
		SEGMENTS_INTERSECT,	// both line segments bisect each other
		A_BISECTS_B,		// line segment B is crossed by line A
		B_BISECTS_A,		// line segment A is crossed by line B
		PARALELL			// the lines are paralell
	};

	POINT_CLASSIFICATION ClassifyPoint(const rvector2& point, float fEpsilon = 0.0f) const;
	LINE_CLASSIFICATION Intersection(const rline2d& line, rvector2* pIntersectPoint=NULL)const;
};

inline float rline2d::SignedDistance(const rvector2& point) const
{
	using namespace RealSpace2;
	auto TestVector = Normalized(point - start);
	auto normal = Normalized(end - start);

	float fOldYValue = normal.y;
	normal.y = -normal.x;
	normal.x = fOldYValue;

	return DotProduct(TestVector, normal);
}

inline rline2d::POINT_CLASSIFICATION rline2d::ClassifyPoint(const rvector2& point, float fEpsilon) const
{
	POINT_CLASSIFICATION Result = ON_LINE;
	float fDistance = SignedDistance(point);
    
    if (fDistance > fEpsilon)
    {
        Result = LEFT_SIDE;
    }
    else if (fDistance < -fEpsilon)
    {
        Result = RIGHT_SIDE;
    }

    return(Result);
}

inline rline2d::LINE_CLASSIFICATION rline2d::Intersection(const rline2d& line, rvector2* pIntersectPoint)const
{
	float Ay_minus_Cy = start.y - line.start.y;	
	float Dx_minus_Cx = line.end.x - line.start.x;	
	float Ax_minus_Cx = start.x - line.start.x;	
	float Dy_minus_Cy = line.end.y - line.start.y;	
	float Bx_minus_Ax = end.x - start.x;	
	float By_minus_Ay = end.y - start.y;	

	float Numerator = (Ay_minus_Cy * Dx_minus_Cx) - (Ax_minus_Cx * Dy_minus_Cy);
	float Denominator = (Bx_minus_Ax * Dy_minus_Cy) - (By_minus_Ay * Dx_minus_Cx);

	// if lines do not intersect, return now
	if (!Denominator)
	{
		if (!Numerator)
		{
			return COLLINEAR;
		}

		return PARALELL;
	}

	float FactorAB = Numerator / Denominator;
	float FactorCD = ((Ay_minus_Cy * Bx_minus_Ax) - (Ax_minus_Cx * By_minus_Ay)) / Denominator;

	// posting (hitting a vertex exactly) is not allowed, shift the results
	// if they are within a minute range of the end vertecies
/*	if (fabs(FactorCD) < 1.0e-6f)
	{
		FactorCD = 1.0e-6f;
	}
	if (fabs(FactorCD - 1.0f) < 1.0e-6f)
	{
		FactorCD = 1.0f - 1.0e-6f;
	}
*/

	// if an interection point was provided, fill it in now
	if (pIntersectPoint)
	{
		pIntersectPoint->x = (start.x + (FactorAB * Bx_minus_Ax));
		pIntersectPoint->y = (start.y + (FactorAB * By_minus_Ay));
	}

	// now determine the type of intersection
	if ((FactorAB >= 0.0f) && (FactorAB <= 1.0f) && (FactorCD >= 0.0f) && (FactorCD <= 1.0f))
	{
		return SEGMENTS_INTERSECT;
	}
	else if ((FactorCD >= 0.0f) && (FactorCD <= 1.0f))
	{
		return (A_BISECTS_B);
	}
	else if ((FactorAB >= 0.0f) && (FactorAB <= 1.0f))
	{
		return (B_BISECTS_A);
	}

	return LINES_INTERSECT;

}