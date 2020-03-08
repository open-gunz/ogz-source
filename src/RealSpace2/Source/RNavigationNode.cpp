#include "stdafx.h"
#include "RNavigationNode.h"
#include "RMath.h"

RNavigationNode::RNavigationNode() : RAStarNode(), bSelected(false), m_nArrivalLink(0), m_nID(-1)
{
}

RNavigationNode::~RNavigationNode()
{

}

void RNavigationNode::Init(int nID, const rvector& v1, const rvector& v2, const rvector& v3)
{
	m_nID = nID;

	m_Vertex[0] = v1;
	m_Vertex[1] = v2;
	m_Vertex[2] = v3;

	m_Links[0] = 0;
	m_Links[1] = 0;
	m_Links[2] = 0;

	ComputeNodeData();
}

void RNavigationNode::ComputeNodeData()
{
	// create 2D versions of our verticies
	rvector2 Point1(m_Vertex[VERT_A].x,m_Vertex[VERT_A].y);
	rvector2 Point2(m_Vertex[VERT_B].x,m_Vertex[VERT_B].y);
	rvector2 Point3(m_Vertex[VERT_C].x,m_Vertex[VERT_C].y);

	// innitialize our sides
	m_Side[SIDE_AB].SetLine(Point1,Point2);	// line AB
	m_Side[SIDE_BC].SetLine(Point2,Point3);	// line BC
	m_Side[SIDE_CA].SetLine(Point3,Point1);	// line CA

	m_Plane = PlaneFromPoints(m_Vertex[VERT_A], m_Vertex[VERT_B], m_Vertex[VERT_C]);

	// compute midpoint as centroid of polygon
	m_CenterPos.x=((m_Vertex[VERT_A].x + m_Vertex[VERT_B].x + m_Vertex[VERT_C].x)/3);
	m_CenterPos.y=((m_Vertex[VERT_A].y + m_Vertex[VERT_B].y + m_Vertex[VERT_C].y)/3);
	m_CenterPos.z=((m_Vertex[VERT_A].z + m_Vertex[VERT_B].z + m_Vertex[VERT_C].z)/3);

	// compute the midpoint of each cell wall
	m_WallMidpoint[0].x = (m_Vertex[VERT_A].x + m_Vertex[VERT_B].x)/2.0f;
	m_WallMidpoint[0].y = (m_Vertex[VERT_A].y + m_Vertex[VERT_B].y)/2.0f;
	m_WallMidpoint[0].z = (m_Vertex[VERT_A].z + m_Vertex[VERT_B].z)/2.0f;

	m_WallMidpoint[1].x = (m_Vertex[VERT_B].x + m_Vertex[VERT_C].x)/2.0f;
	m_WallMidpoint[1].y = (m_Vertex[VERT_B].y + m_Vertex[VERT_C].y)/2.0f;
	m_WallMidpoint[1].z = (m_Vertex[VERT_B].z + m_Vertex[VERT_C].z)/2.0f;

	m_WallMidpoint[2].x = (m_Vertex[VERT_C].x + m_Vertex[VERT_A].x)/2.0f;
	m_WallMidpoint[2].y = (m_Vertex[VERT_C].y + m_Vertex[VERT_A].y)/2.0f;
	m_WallMidpoint[2].z = (m_Vertex[VERT_C].z + m_Vertex[VERT_A].z)/2.0f;


	// compute the distances between the wall midpoints
	m_WallDistance[0] = _REALSPACE2::Magnitude(m_WallMidpoint[0] - m_WallMidpoint[1]);
	m_WallDistance[1] = _REALSPACE2::Magnitude(m_WallMidpoint[1] - m_WallMidpoint[2]);
	m_WallDistance[2] = _REALSPACE2::Magnitude(m_WallMidpoint[2] - m_WallMidpoint[0]);
}

bool RNavigationNode::RequestLink(const rvector& PointA, const rvector& PointB, RNavigationNode* pCaller)
{
	if (m_Vertex[VERT_A] == PointA)
	{
		if (m_Vertex[VERT_B] == PointB)
		{
			m_Links[SIDE_AB] = pCaller;
			return true;
		}
		else if (m_Vertex[VERT_C] == PointB)
		{
			m_Links[SIDE_CA] = pCaller;
			return true;
		}
	}
	else if (m_Vertex[VERT_B] == PointA)
	{
		if (m_Vertex[VERT_A] == PointB)
		{
			m_Links[SIDE_AB] = pCaller;
			return true;
		}
		else if (m_Vertex[VERT_C] == PointB)
		{
			m_Links[SIDE_BC] = pCaller;
			return true;
		}
	}
	else if (m_Vertex[VERT_C] == PointA)
	{
		if (m_Vertex[VERT_A] == PointB)
		{
			m_Links[SIDE_CA] = pCaller;
			return true;
		}
		else if (m_Vertex[VERT_B] == PointB)
		{
			m_Links[SIDE_BC] = pCaller;
			return true;
		}
	}

	return false;
}


bool RNavigationNode::IsPointInNodeColumn(const rvector& TestPoint) const
{
	rvector2 pos = rvector2(TestPoint.x,TestPoint.y);

	return (IsPointInNodeColumn(pos));
}

bool RNavigationNode::IsPointInNodeColumn(const rvector2& TestPoint) const
{
	int InteriorCount = 0;

	for (int i=0;i<3;i++)
	{
		rline2d::POINT_CLASSIFICATION SideResult = m_Side[i].ClassifyPoint(TestPoint);

		if (SideResult != rline2d::RIGHT_SIDE)
		{
			InteriorCount++;
		}
	}

	return (InteriorCount == 3);



	return false;
}

bool RNavigationNode::ForcePointToNodeColumn(rvector2& TestPoint) const
{
	bool PointAltered = false;

	// create a motion path from the center of the cell to our point
	rline2d TestPath(rvector2(m_CenterPos.x, m_CenterPos.y), TestPoint);
	rvector2 PointOfIntersection;
	NODE_SIDE Side;
	RNavigationNode* pNextNode;

	PATH_RESULT result = ClassifyPathToNode(TestPath, &pNextNode, Side, &PointOfIntersection);
	// compare this path to the cell.

	if (result == EXITING_NODE)
	{
		rvector2 PathDirection(PointOfIntersection.x - m_CenterPos.x, PointOfIntersection.y - m_CenterPos.y);

		PathDirection *= 0.9f;

		TestPoint.x = m_CenterPos.x + PathDirection.x;
		TestPoint.y = m_CenterPos.y + PathDirection.y;
		return (true);
	}
	else if (result == NO_RELATIONSHIP)
	{
		TestPoint.x = m_CenterPos.x;
		TestPoint.y = m_CenterPos.y;
		return (true);
	}

	return (false);
}

bool RNavigationNode::ForcePointToNodeColumn(rvector& TestPoint)const
{
	rvector2 TestPoint2D(TestPoint.x,TestPoint.y);
	bool PointAltered = ForcePointToNodeColumn(TestPoint2D);

	if (PointAltered)
	{
		TestPoint.x=TestPoint2D.x;
		TestPoint.y=TestPoint2D.y;
	}
	return (PointAltered);
}

void RNavigationNode::MapVectorHeightToNode(rvector& MotionPoint) const
{
	// ax + by + cz + d = 0
	// cz = -(ax+by+d)
	// z = -(ax+by+d)/b
	if (m_Plane.c)
	{
		MotionPoint.z = ( -(m_Plane.a*MotionPoint.x + m_Plane.b*MotionPoint.y+m_Plane.d)/m_Plane.c);
	}
	else
	{
		MotionPoint.z = 0.0f;
	}
}

RNavigationNode::PATH_RESULT RNavigationNode::ClassifyPathToNode(const rline2d& MotionPath, RNavigationNode** ppNextNode, 
								NODE_SIDE& side, rvector2* pPointOfIntersection) const
{
	int nInteriorCount = 0;

	for (int i=0; i<3; ++i)
	{
		if (m_Side[i].ClassifyPoint(MotionPath.end) != rline2d::LEFT_SIDE)
		{
			if (m_Side[i].ClassifyPoint(MotionPath.start) != rline2d::RIGHT_SIDE)
			{
				rline2d::LINE_CLASSIFICATION IntersectResult = MotionPath.Intersection(m_Side[i], pPointOfIntersection);
				
				if (IntersectResult == rline2d::SEGMENTS_INTERSECT || IntersectResult == rline2d::A_BISECTS_B)
				{
					*ppNextNode = m_Links[i];
					side = (NODE_SIDE)i;
					return (EXITING_NODE);
				}
			}
		}
		else
		{
			nInteriorCount++;
		}

	}

	if (nInteriorCount == 3)
	{
		return (ENDING_NODE);
	}


	return (NO_RELATIONSHIP);
}

float RNavigationNode::GetSuccessorCost(RAStarNode* pSuccessor)
{
	_ASSERT(pSuccessor != NULL);
	
	float fWeight = (m_fWeight + pSuccessor->GetWeight()) / 2.0f;
	rvector d = m_CenterPos - ((RNavigationNode*)pSuccessor)->m_CenterPos;
	return MagnitudeSq(d) * fWeight;
}

float RNavigationNode::GetHeuristicCost(RAStarNode* pGoal)
{
	_ASSERT(pGoal != NULL);

	rvector d = m_CenterPos - ((RNavigationNode*)pGoal)->m_CenterPos;
	return MagnitudeSq(d);
}