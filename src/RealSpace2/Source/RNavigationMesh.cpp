#include "stdafx.h"
#include "RNavigationMesh.h"
#include "RNavigationNode.h"
#include "MZFileSystem.h"
#include "RVersions.h"
#include "RMath.h"

RNavigationMesh::RNavigationMesh()
{
	m_nVertCount = 0;
	m_nFaceCount = 0;
	m_vertices = NULL;
	m_faces = NULL;
}

RNavigationMesh::~RNavigationMesh()
{
	Clear();
}

void RNavigationMesh::Clear()
{
	if (m_vertices != NULL) delete [] m_vertices;
	if (m_faces != NULL) delete [] m_faces;

	m_nVertCount = 0;
	m_nFaceCount = 0;
	m_vertices = NULL;
	m_faces = NULL;

	ClearNodes();
}

void RNavigationMesh::ClearNodes()
{
	for (RNodeArray::iterator itor = m_NodeArray.begin(); itor != m_NodeArray.end(); ++itor)
	{
		delete (*itor);
	}
	m_NodeArray.clear();
}

void RNavigationMesh::AddNode(int nID, const rvector& PointA, const rvector& PointB, const rvector& PointC)
{
	RNavigationNode* pNewNode = new RNavigationNode;

	pNewNode->Init(nID, PointA, PointB, PointC);
	m_NodeArray.push_back(pNewNode);
}

void RNavigationMesh::MakeNodes()
{
	if (!m_NodeArray.empty()) ClearNodes();

	for (int i = 0; i < m_nFaceCount; i++)
	{
		rvector* vp[3];
		vp[0] = &m_vertices[m_faces[i].v1];
		vp[1] = &m_vertices[m_faces[i].v2];
		vp[2] = &m_vertices[m_faces[i].v3];

		rvector temp = m_vertices[m_faces[i].v3];

		_ASSERT( (*vp[0] != *vp[1]) && (*vp[1] != *vp[2]) && (*vp[2] != *vp[1]) );
		AddNode(i, *vp[0], *vp[1], *vp[2]);		// 반시계방향
		//AddNode(i, *vp[0], *vp[2], *vp[1]);		// 시계방향
	}
}

void RNavigationMesh::LinkNodes()
{
	for (RNodeArray::iterator itorA = m_NodeArray.begin(); itorA != m_NodeArray.end(); ++itorA)
	{
		RNavigationNode* pNodeA = (*itorA);
		for (RNodeArray::iterator itorB = m_NodeArray.begin(); itorB != m_NodeArray.end(); ++itorB)
		{
			if (itorA == itorB) continue;

			RNavigationNode* pNodeB = (*itorB);

			if (!pNodeA->GetLink(RNavigationNode::SIDE_AB) && pNodeB->RequestLink(pNodeA->Vertex(0), pNodeA->Vertex(1), pNodeA))
			{
				pNodeA->SetLink(RNavigationNode::SIDE_AB, pNodeB);
			}
			else if (!pNodeA->GetLink(RNavigationNode::SIDE_BC) && pNodeB->RequestLink(pNodeA->Vertex(1), pNodeA->Vertex(2), pNodeA))
			{
				pNodeA->SetLink(RNavigationNode::SIDE_BC, pNodeB);
			}
			else if (!pNodeA->GetLink(RNavigationNode::SIDE_CA) && pNodeB->RequestLink(pNodeA->Vertex(2), pNodeA->Vertex(0), pNodeA))
			{
				pNodeA->SetLink(RNavigationNode::SIDE_CA, pNodeB);
			}
		}
	}
}

RNavigationNode* RNavigationMesh::FindClosestNode(const rvector& point) const
{
	float ClosestDistance = FLT_MAX;
	float ClosestHeight = FLT_MAX;
	bool bFoundHomeNode = false;
	float ThisDistance;
	RNavigationNode* pClosestNode=NULL;

		
	for (RNodeArray::const_iterator itorNode = m_NodeArray.begin(); itorNode != m_NodeArray.end(); ++itorNode)
	{
		RNavigationNode* pNode = (*itorNode);

		if (pNode->IsPointInNodeColumn(point))
		{
			rvector NewPosition(point);
			pNode->MapVectorHeightToNode(NewPosition);

			// 가장 가까운 높이의 노드를 찾는다.
			ThisDistance = fabs(NewPosition.z - point.z);

			if (bFoundHomeNode)
			{
				if (ThisDistance < ClosestHeight)
				{
					pClosestNode = pNode;
					ClosestHeight = ThisDistance;
				}
			}
			else
			{
				pClosestNode = pNode;
				ClosestHeight = ThisDistance;
				bFoundHomeNode = true;
			}
		}

		if (!bFoundHomeNode)
		{
			rvector2 Start(pNode->CenterVertex().x, pNode->CenterVertex().y);
			rvector2 End(point.x, point.y);
			rline2d MotionPath(Start, End);

			RNavigationNode* pNextNode;
			RNavigationNode::NODE_SIDE WallHit;
			rvector2 PointOfIntersection;

			RNavigationNode::PATH_RESULT Result = pNode->ClassifyPathToNode(MotionPath, &pNextNode, WallHit, &PointOfIntersection);

			if (Result == RNavigationNode::EXITING_NODE)
			{
				rvector ClosestPoint3D(PointOfIntersection.x, PointOfIntersection.y, 0.0f);
				pNode->MapVectorHeightToNode(ClosestPoint3D);

				ClosestPoint3D -= point;

				ThisDistance = Magnitude(ClosestPoint3D);

				if (ThisDistance<ClosestDistance)
				{
					ClosestDistance=ThisDistance;
					pClosestNode = pNode;
				}
			}
		}
	}
	


	return pClosestNode;



}

#ifdef _WIN32
void DrawNavFace(LPDIRECT3DDEVICE9 pd3dDevice, rvector* vertices, int v1, int v2, int v3)
{
	rvector v[3];
	v[0] = vertices[v1];
	v[1] = vertices[v2];
	v[2] = vertices[v3];

	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, v, sizeof(rvector));
}

void DrawNavFaceWireFrame(LPDIRECT3DDEVICE9 pd3dDevice, rvector* vertices, int v1, int v2, int v3)
{
	rvector v[4];
	v[0] = vertices[v1];
	v[1] = vertices[v2];
	v[2] = vertices[v3];
	v[3] = vertices[v1];

	pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP,3,&v,sizeof(rvector));
}

#include "RealSpace2.h"
using namespace RealSpace2;

void RNavigationMesh::Render()
{
	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();

	RSetTransform(D3DTS_WORLD, GetIdentityMatrix());

	pd3dDevice->SetFVF( D3DFVF_XYZ );
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW );
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0x40FF9100);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	RSetWBuffer(true);
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0x40ffffff);

	for (RNodeArray::iterator itor = m_NodeArray.begin(); itor != m_NodeArray.end(); ++itor)
	{
		RNavigationNode* pNode = *itor;

		if (pNode->bSelected) pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0x40FFFFFF);
		else pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0x40FF9100);

		rvector _vertices[3];
		_vertices[0] = pNode->Vertex(0);
		_vertices[1] = pNode->Vertex(1);
		_vertices[2] = pNode->Vertex(2);

		DrawNavFaceWireFrame(pd3dDevice, _vertices, 0, 1, 2);

	}


	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0x40ff00ff);

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

}


void RNavigationMesh::RenderLinks()
{
	RSetTransform(D3DTS_WORLD, GetIdentityMatrix());

	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0xFF0033FF);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	DWORD color = 0xFFFF0000;

	for (RNavigationMesh::RNodeArray::iterator itor = GetNodes()->begin();
		itor != GetNodes()->end(); ++itor)
	{
		RNavigationNode* pNodeA = (*itor);

		if (pNodeA->GetLink(RNavigationNode::SIDE_AB))
		{
			RNavigationNode* pSideNode = pNodeA->GetLink(RNavigationNode::SIDE_AB);

			rvector p1, p2;
			p1 = pNodeA->CenterVertex();
			p2 = pSideNode->CenterVertex();

			RDrawLine(p1, p2, color);
		}


		if (pNodeA->GetLink(RNavigationNode::SIDE_BC))
		{
			RNavigationNode* pSideNode = pNodeA->GetLink(RNavigationNode::SIDE_BC);

			rvector p1, p2;
			p1 = pNodeA->CenterVertex();
			p2 = pSideNode->CenterVertex();

			RDrawLine(p1, p2, color);
		}

		if (pNodeA->GetLink(RNavigationNode::SIDE_CA))
		{
			RNavigationNode* pSideNode = pNodeA->GetLink(RNavigationNode::SIDE_CA);
			rvector p1, p2;
			p1 = pNodeA->CenterVertex();
			p2 = pSideNode->CenterVertex();

			RDrawLine(p1, p2, color);
		}

	}

}
#endif

rvector RNavigationMesh::SnapPointToNode(RNavigationNode* pNode, const rvector& Point)
{
	rvector PointOut = Point;

	//if (!pNode->IsPointInNodeColumn(PointOut))
	{
		pNode->ForcePointToNodeColumn(PointOut);
	}

	pNode->MapVectorHeightToNode(PointOut);
	return (PointOut);
}

rvector RNavigationMesh::SnapPointToMesh(RNavigationNode** NodeOut, const rvector& Point)
{
	rvector PointOut = Point;
	*NodeOut = FindClosestNode(PointOut);
	return (SnapPointToNode(*NodeOut, PointOut));
}

bool RNavigationMesh::BuildNavigationPath(RNavigationNode* pStartNode, 
							const rvector& StartPos, RNavigationNode* pEndNode, const rvector& EndPos)
{
	m_pStartNode = pStartNode;
	m_pGoalNode = pEndNode;

	bool ret = m_AStar.Search(pStartNode, pEndNode);
	if (ret == false) return false;

	m_WaypointList.clear();
	m_WaypointList.push_back(EndPos);


	RNavigationNode* pVantageNode = NULL;
	rvector vantagePos;

	pVantageNode = pEndNode;
	vantagePos = EndPos;

	RNavigationNode* pLastNode = NULL;
	rvector lastPos;

	list<RAStarNode*>* pPath = &m_AStar.m_ShortestPath;

	bool bPushed = true;
	for (list<RAStarNode*>::iterator itor = pPath->begin(); itor != pPath->end(); itor++)
	{
		RNavigationNode* pTestNode = (RNavigationNode*)(*itor);

		rvector testPos = pTestNode->GetWallMidPoint(pTestNode->GetArrivalLink());
		testPos = SnapPointToNode(pTestNode, testPos);

		if (LineOfSightTest(pVantageNode, vantagePos, pTestNode, testPos))
		{
			pLastNode = pTestNode;
			lastPos = testPos;
			bPushed = false;
		}
		else
		{
			_ASSERT(pLastNode != NULL);
			m_WaypointList.push_back(lastPos);
			pVantageNode = pLastNode;
			vantagePos = lastPos;
			bPushed = true;
		}
	}
	
	if (!bPushed) 
	{
		if (!LineOfSightTest(pVantageNode, vantagePos, pStartNode, StartPos))
		{
			m_WaypointList.push_back(lastPos);
		}
	}

	//m_WaypointList.push_back(StartPos);


	// 순서를 뒤바꾼다.
	m_WaypointList.reverse();

	return ret;
}

bool RNavigationMesh::BuildNavigationPath(const rvector& vStartPos, const rvector& vGoalPos)
{
	RNavigationNode* pStartNode = FindClosestNode(vStartPos);
	if (pStartNode == NULL) return false;

	RNavigationNode* pGoalNode = FindClosestNode(vGoalPos);
	if (pGoalNode==NULL) return false;

	return BuildNavigationPath(pStartNode, vStartPos, pGoalNode, vGoalPos);
}


bool RNavigationMesh::LineOfSightTest(RNavigationNode* pStartNode, const rvector& StartPos, RNavigationNode* pGoalNode, const rvector& EndPos)
{
	if ((pStartNode == NULL) || (pGoalNode == NULL)) return false;
	if (pStartNode == pGoalNode) return true;

	rline2d MotionPath(rvector2(StartPos.x,StartPos.y), rvector2(EndPos.x,EndPos.y));

	RNavigationNode* pNextNode = pStartNode;
	RNavigationNode::NODE_SIDE WallNumber;
	RNavigationNode::PATH_RESULT Result;

	while((Result = pNextNode->ClassifyPathToNode(MotionPath, &pNextNode, WallNumber, 0)) == RNavigationNode::EXITING_NODE)
	{
		if (!pNextNode) return(false);
	}


	return (Result == RNavigationNode::ENDING_NODE);
}

bool RNavigationMesh::Open(const char* szFileName, MZFileSystem* pZFileSystem)
{
	MZFile file;
	if(!file.Open(szFileName, pZFileSystem)) return false;

	// header -------------
	RHEADER header;
	file.Read(&header,sizeof(RHEADER));
	if(header.dwID!=R_NAV_ID || header.dwVersion!=R_NAV_VERSION)
	{
		file.Close();
		return false;
	}

	int nVertCount,nFaceCount;

	// vertex -------------
	file.Read(&nVertCount,sizeof(int));
	InitVertices(nVertCount);
	for (int i = 0; i < nVertCount; i++)
	{
		file.Read(&m_vertices[i], sizeof(rvector));
	}

	// face ---------------
	file.Read(&nFaceCount,sizeof(int));
	InitFaces(nFaceCount);
	for (int i = 0; i < nFaceCount; i++)
	{
		file.Read(&m_faces[i], sizeof(RNavFace));
	}

	MakeNodes();

	// link ---------------
	for (RNodeArray::iterator itor = m_NodeArray.begin(); itor != m_NodeArray.end(); ++itor)
	{
		RNavigationNode* pNode = (*itor);

		for (int i = 0; i < 3; i++)
		{
			int nSideIndex = -1;

			if (file.Read(&nSideIndex, sizeof(int)))
			{
				if (nSideIndex >= 0)
				{
					pNode->SetLink(RNavigationNode::NODE_SIDE(i), m_NodeArray[nSideIndex]);
				}
			}
			else
			{
				_ASSERT(0);
			}
		}
	}

	file.Close();

	return true;
}

bool RNavigationMesh::Save(const char* szFileName)
{
	if (m_nVertCount <= 0) return false;

	FILE* file = fopen(szFileName, "wb");
	if (!file) return false;

	// header -------------
	RHEADER header{ R_NAV_ID, R_NAV_VERSION };
	fwrite(&header, sizeof(RHEADER), 1, file);

	// vertex -------------
	fwrite(&m_nVertCount, sizeof(int), 1, file);
	for (int i = 0; i < m_nVertCount; i++)
	{
		fwrite(&m_vertices[i], sizeof(rvector), 1, file);
	}

	// face ---------------
	fwrite(&m_nFaceCount, sizeof(int), 1, file);
	for (int i = 0; i < m_nFaceCount; i++)
	{
		fwrite(&m_faces[i], sizeof(RNavFace), 1, file);
	}

	MakeNodes();
	LinkNodes();

	// link ---------------
	for (RNodeArray::iterator itor = m_NodeArray.begin(); itor != m_NodeArray.end(); ++itor)
	{
		RNavigationNode* pNode = (*itor);

		for (int i = 0; i < 3; i++)
		{
			int nSideIndex = -1;
			if (pNode->GetLink(i))
			{
				nSideIndex = pNode->GetLink(i)->GetID();
			}
			fwrite(&nSideIndex, sizeof(int), 1, file);
		}
	}



	fclose(file);

	return true;
}


void RNavigationMesh::ClearAllNodeWeight()
{
	for (RNodeArray::iterator itor = m_NodeArray.begin(); itor != m_NodeArray.end(); ++itor)
	{
		RNavigationNode* pNode = (*itor);
		pNode->SetWeight(1.0f);
	}
}