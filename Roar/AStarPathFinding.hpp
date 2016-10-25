//==========================================================
//AStarPathFinding.hpp
//==========================================================

#pragma once
#ifndef _ASTAR_PATHFINDING_
#define _ASTAR_PATHFINDING_
#include <list>
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Renderer/OpenGLRenderer.hpp"
class Map;
struct PathNode
{
	PathNode(float avoidance_cost_g, float distance_cost_g, float parent_g, float h, IntVector2 position, PathNode* parentNode)
	{
		m_avoidance_cost_g = avoidance_cost_g;
		m_distance_cost_g = distance_cost_g;
		m_parent_g = parent_g;
		m_heuristic = h;
		m_position = position;
		m_parent = parentNode;
	}
	PathNode(){}
	float m_avoidance_cost_g;
	float m_distance_cost_g;
	float m_parent_g;
	float m_heuristic;
	float CaculateFullCost()
	{
		float fullCost;
		fullCost = m_heuristic + m_avoidance_cost_g + m_distance_cost_g + m_parent_g;
		return fullCost;
	}
	float CaculateG()
	{
		float fullCostG;
		fullCostG = m_avoidance_cost_g + m_distance_cost_g;
		return fullCostG;
	}
	IntVector2 m_position;
	PathNode* m_parent;
};



class AStarPathFinding
{
public:
	AStarPathFinding(IntVector2 startPosition, IntVector2 goalPosition,Map* map);
	~AStarPathFinding();
	void PathFinder();
	void ResetPathFinding(IntVector2 startPosition, IntVector2 goalPosition, Map* map);
	float CalculateHeuristic(IntVector2 currentPosition, IntVector2 goalPosition);
	float CalculateHeuristic(PathNode currentPathNode, IntVector2 goalPosition);
	float GetDistanceG(IntVector2 currentPosition);
	PathNode PopOutLowestFullCostOnOpenList();
	PathNode& GetPathNodeFromOpenlistByPosition(IntVector2 position);
	bool CheckOnClosedListAndResetParent(IntVector2 currentPosition);
	PathNode* CheckOnOpenList(IntVector2 currentPosition);
	IntVector2 StepByStepPathFinder();
	NTBVertex3D_PCT* CreatePathVerts(int& numberOfVerts);
	std::list<PathNode> m_openList;
	std::list<PathNode> m_closedList;
	IntVector2 m_startPosition;
	IntVector2 m_goalPosition;
	PathNode m_activeNode;
	Map* m_map;
};


#endif