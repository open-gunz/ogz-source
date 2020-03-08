#include "stdafx.h"
#include "RAStar.h"

RAStarNode::RAStarNode()
{
	m_fCostFromStart = 0.0f;
	m_fCostToGoal = 0.0f;
	m_pParent = NULL;
//	m_nMapID = 0;
//	m_ppSuccessors = NULL;
//	m_bClosed = false;
//	m_nValue = 0;
//	m_nVisitID = 0;
	m_nSessionID = 0;
	m_pData = NULL;

//	m_nSuccessorCount = 0;
	m_fWeight = 1.0f;
}

RAStarNode::~RAStarNode()
{

}

/////////////////////////////////////////////////////////////////////////////////////
RAStar::RAStar() : m_nPathSession(0)
{
//	m_pNodes = NULL;
}

RAStar::~RAStar()
{

}

void RAStar::PushOnOpenList(RAStarNode* pStartNode)
{
	m_OpenList.push_back(pStartNode);
}

bool RAStar::IsOpenListEmpty()
{
	return m_OpenList.empty();
}

RAStarNode* RAStar::PopLowestCostFromOpenList()
{
	RAStarNode* pOutNode = NULL;
	list<RAStarNode*>::iterator itorErase = m_OpenList.end();

	for (list<RAStarNode*>::iterator itor = m_OpenList.begin(); itor != m_OpenList.end(); ++itor)
	{
		RAStarNode* pNode = (*itor);
		if (pOutNode==NULL)
		{
			pOutNode = pNode;
			itorErase = itor;
		}
		else
		{
			if (pOutNode->GetTotalCost() > pNode->GetTotalCost())
			{
				pOutNode = pNode;
				itorErase = itor;
			}
		}
	}

	m_OpenList.erase(itorErase);
	return pOutNode;
}

#include "RNavigationNode.h"

void RAStar::PushToShortestPath(RAStarNode* pNode)
{
	RNavigationNode* pNNode = (RNavigationNode*)pNode;
	m_ShortestPath.push_back(pNode);
}


bool RAStar::Search(RAStarNode* pStartNode, RAStarNode* pGoalNode)
{
	// clear Open and Closed
	m_OpenList.clear();
	m_nPathSession++;

	// 시작 노드를 초기화한다.
	// 시작지점을 노드 P로 둔다.
	// P에 f, g, h값들을 배정한다.
	pStartNode->m_fCostFromStart = 0.0f;
	pStartNode->m_fCostToGoal = pStartNode->GetHeuristicCost(pGoalNode);
	pStartNode->m_pParent = NULL;

	// push StartNode on Open
	PushOnOpenList(pStartNode);

	// 성공 또는 실패에 이를 때까지 목록을 처리한다.
	while (!IsOpenListEmpty())		// 열린 목록이 비었다면 경로를 찾을 수 없음
	{
		// pop Node from Open
		RAStarNode* pNode = PopLowestCostFromOpenList();

		// 목표에 도달했으면 성공으로 끝낸다.
		if (pNode == pGoalNode)
		{
			// construct a path backward from Node to StartLoc
			m_ShortestPath.clear();
			RAStarNode* pShortestNode = pNode;
			while (pShortestNode != pStartNode)
			{
				PushToShortestPath(pShortestNode);
				pShortestNode = pShortestNode->m_pParent;
			}

			// 마지막에 시작 노드를 넣어준다.
//			PushToShortestPath(pStartNode);
			return true;
		}

		// for each successor NewNode of Node
		for(int i=0; i < pNode->GetSuccessorCount(); i++)
		{
			RAStarNode* pSuccessor = pNode->GetSuccessor(i);
			if(pSuccessor==NULL) continue;	// NULL이면 없는 노드로 간주한다.
			if(pSuccessor==pStartNode) continue;					// 시작 노드로 다시 돌아올 필요가 없으므로 제외한다.
			if(pNode->m_pParent==pSuccessor) continue;

			float fNewCostFromStart = pNode->GetSuccessorCostFromStart(pSuccessor);

			// 이 노드가 존재하며 더 나은 결과가 아니라면 무시한다.
			// 이 노드가 열린 목록이나 닫힌 목록에 들어있는지 점검한다.
			if(pSuccessor->GetSessionID() == m_nPathSession)
			{
				if( pSuccessor->m_fCostFromStart <= fNewCostFromStart ) continue;	// Skip
			}

			// 새로운, 즉 더 나은 정보를 저장한다.
			pSuccessor->m_pParent = pNode;
			pSuccessor->m_fCostFromStart = fNewCostFromStart;
			pSuccessor->m_fCostToGoal = pSuccessor->GetHeuristicCost(pGoalNode);
			pSuccessor->OnSetData(i, pNode);
			

			if(pSuccessor->GetSessionID() != m_nPathSession)
			{
				pSuccessor->m_nSessionID = m_nPathSession;
				PushOnOpenList(pSuccessor);
			}

		}

	}	// while

	return false;
}


/*
-------------------------------------------------------------------------------------
Open: priorityqueue of searchnode
Closed: list of searchnode

AStarSearch(location StartLoc, location GoalLoc, agenttype Agent) {
	clear Open and Closed

	// 시작 노드를 초기화한다.
	StartNode.Loc = StartLoc
	StartNode.CostFromStart = 0
	StartNode.CostToGoal = PathCostEstimate(StartLoc, GoalLoc, Agent)
	StartNode.Parent = null
	push StartNode on Open

	// 성공 또는 실패에 이를 때까지 목록을 처리한다.
	while Open is not empty {
		pop Node from Open // 노드가 최저의 TotalCost를 가지는 경우

		// 목표에 도달했으면 성공으로 끝낸다.
		if (Node is a goal node) {
			construct a path backward from Node to StartLoc
			return success
		} else {
			for each successor NewNode of Node {
				NewCost = Node.CostFromStart + TraverseCost(Node, NewNode, Agent)
				// 이 노드가 존재하며 더 나은 결과가 아니라면 무시한다.
				if (NewNode is in Open or Closed) and
				(NewNode.CostFromStart <= NewCost) {
					continue
				} else {	// 새로운, 즉 더 나은 정보를 저장한다.
					NewNode.Parent = Node
					NewNode.CostFromStart = NewCost
					NewNode.CostToGoal = PathCostEstimate(NewNode.Loc, GoalLoc, Agent)
					NewNode.TotalCost = NewNode.CostFromStart + NewNode.CostToGoal
					if (NewNode is in Closed) {
						remove NewNode from Closed
					}
					if (NewNode is in Open) {
						adjust NewNode's location in Open
					} else {
						push NewNode onto Open
					}
				} // if (NewNode is ... <= NewNode)에 대한 else 블록의 끝
			} // for
		} // if (Node is a gole node)
		push Node onto Closed
	} // while Open is not empty
	return false
}
*/
