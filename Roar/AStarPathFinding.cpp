//==========================================================
//AStarPathFinding.cpp
//==========================================================
#pragma once
#include "AStarPathFinding.hpp"
#include "Map.hpp"
#include "Engine/Input/InputSystem.hpp"
const int TILE_SCALE = 20;
const Vector2 OFFSET = Vector2(200.0f, 200.0f);
AStarPathFinding::AStarPathFinding(IntVector2 startPosition, IntVector2 goalPosition,Map* map)
{
	m_openList.clear();
	m_closedList.clear();
	PathNode firstNode(1.0f, 0.0f, 0.0f, CalculateHeuristic(startPosition, goalPosition), startPosition,nullptr);
	m_goalPosition = goalPosition;
	m_openList.push_back(firstNode);
	m_map = map;
}

AStarPathFinding::~AStarPathFinding()
{
}

float AStarPathFinding::CalculateHeuristic(IntVector2 currentPosition, IntVector2 goalPosition)
{
	Vector2 heuristicXY;
	heuristicXY.x = abs((float)(currentPosition.x - goalPosition.x));
	heuristicXY.y = abs((float)(currentPosition.y - goalPosition.y));
	return heuristicXY.x + heuristicXY.y;
}

float AStarPathFinding::CalculateHeuristic(PathNode currentPathNode, IntVector2 goalPosition)
{
	IntVector2 currentPosition = currentPathNode.m_position;
	Vector2 heuristicXY;
	heuristicXY.x = abs((float)(currentPosition.x - goalPosition.x));
	heuristicXY.y = abs((float)(currentPosition.y - goalPosition.y));
	return heuristicXY.x + heuristicXY.y;
}

void AStarPathFinding::PathFinder()
{
	while (!m_openList.empty())
	{
		PathNode theLowestFullCostNode = PopOutLowestFullCostOnOpenList();
		m_closedList.push_back(theLowestFullCostNode);
		m_activeNode = theLowestFullCostNode;
		if (m_activeNode.m_position.x == m_goalPosition.x && m_activeNode.m_position.y == m_goalPosition.y)
		{
			break;
		}
		std::vector<IntVector2> adjacentPositions;
		m_map->PushBackAdjacentPosition(adjacentPositions, m_activeNode.m_position);
		for (std::vector<IntVector2>::iterator adjacentPositionIter = adjacentPositions.begin(); adjacentPositionIter != adjacentPositions.end();++adjacentPositionIter)
		{
			IntVector2 currentPosition = (*adjacentPositionIter);
			if (CheckOnClosedListAndResetParent(currentPosition))
			{
				continue;
			}
			PathNode* nodeFromOpenList = CheckOnOpenList(currentPosition);
			if (nodeFromOpenList==nullptr)
			{
				//PathNode(float avoidance_cost_g, float distance_cost_g, float parent_g, float h, IntVector2 position, PathNode* parentNode)
				PathNode newNode(1.0f, GetDistanceG(currentPosition), m_activeNode.CaculateG(), CalculateHeuristic(currentPosition,m_goalPosition), currentPosition, &m_activeNode);
				m_openList.push_back(newNode);
				continue;
			}
			else
			{
				float localG = 1.0f + GetDistanceG(currentPosition);
				float parentG = m_activeNode.CaculateG();
				if (localG + parentG < nodeFromOpenList->CaculateG())
				{
					nodeFromOpenList->m_distance_cost_g = GetDistanceG(currentPosition);
					nodeFromOpenList->m_parent_g = parentG;
					nodeFromOpenList->m_parent = &m_closedList.back();
				}
			}			
		}
	}
}

PathNode AStarPathFinding::PopOutLowestFullCostOnOpenList()
{
	std::list<PathNode>::iterator theLowestFullCostNodeIter = m_openList.begin();
	for (std::list<PathNode>::iterator openListIter = m_openList.begin(); openListIter != m_openList.end(); ++openListIter)
	{
		if (theLowestFullCostNodeIter == openListIter)
		{
			continue;
		}
		else if (openListIter->CaculateFullCost() < theLowestFullCostNodeIter->CaculateFullCost())
		{
			theLowestFullCostNodeIter = openListIter;
		}
		else
			continue;
	}
	PathNode theLowestFullCostNode = (*theLowestFullCostNodeIter);
	m_openList.erase(theLowestFullCostNodeIter);
	return theLowestFullCostNode;
}

bool AStarPathFinding::CheckOnClosedListAndResetParent(IntVector2 currentPosition)
{
	for (std::list<PathNode>::iterator closedListIter = m_closedList.begin(); closedListIter != m_closedList.end(); ++closedListIter)
	{
		PathNode currentNode = (*closedListIter);
		if (currentNode.m_position.x == currentPosition.x && currentNode.m_position.y == currentPosition.y)
		{
			closedListIter->m_parent = &m_activeNode;
			return true;
		}
	}
	return false;
}

PathNode* AStarPathFinding::CheckOnOpenList(IntVector2 currentPosition)
{
	for (std::list<PathNode>::iterator openListIter = m_openList.begin(); openListIter != m_openList.end(); ++openListIter)
	{
		PathNode currentNode = (*openListIter);
		if (currentNode.m_position.x == currentPosition.x && currentNode.m_position.y == currentPosition.y)
		{
			return &(*openListIter);
		}
	}
	return nullptr;
}

float AStarPathFinding::GetDistanceG(IntVector2 currentPosition)
{
	float distanceG = (float)sqrt((currentPosition.x - m_activeNode.m_position.x)*(currentPosition.x - m_activeNode.m_position.x) + (currentPosition.y - m_activeNode.m_position.y)*(currentPosition.y - m_activeNode.m_position.y));
	return distanceG;
}

void AStarPathFinding::ResetPathFinding(IntVector2 startPosition, IntVector2 goalPosition, Map* map)
{
	m_openList.clear();
	m_closedList.clear();
	PathNode firstNode(1.0f, 0.0f, 0.0f, CalculateHeuristic(startPosition, goalPosition), startPosition, nullptr);
	m_startPosition = startPosition;
	m_goalPosition = goalPosition;
	m_openList.push_back(firstNode);
	m_map = map;
}

IntVector2 AStarPathFinding::StepByStepPathFinder()
{

	PathNode theLowestFullCostNode = PopOutLowestFullCostOnOpenList();
	m_closedList.push_back(theLowestFullCostNode);
	m_activeNode = theLowestFullCostNode;
	if (m_activeNode.m_position.x == m_goalPosition.x && m_activeNode.m_position.y == m_goalPosition.y)
	{
		
		IntVector2 randomStartPosition(MathUtils::GetRandomIntInRange(2, m_map->m_size.x - 2), MathUtils::GetRandomIntInRange(2, m_map->m_size.y - 2));
		IntVector2 randomEndPosition(MathUtils::GetRandomIntInRange(2, m_map->m_size.x - 2), MathUtils::GetRandomIntInRange(2, m_map->m_size.y - 2));
		while (m_map->GetTileByTilePosition(randomStartPosition)->m_tileType!=Tile::TILE_TYPE_AIR)
		{
			randomStartPosition = IntVector2(MathUtils::GetRandomIntInRange(2, m_map->m_size.x - 2), MathUtils::GetRandomIntInRange(2, m_map->m_size.y - 2));
		}
		while (m_map->GetTileByTilePosition(randomEndPosition)->m_tileType != Tile::TILE_TYPE_AIR)
		{
			randomEndPosition = IntVector2(MathUtils::GetRandomIntInRange(2, m_map->m_size.x - 2), MathUtils::GetRandomIntInRange(2, m_map->m_size.y - 2));
		}
		ResetPathFinding(randomStartPosition, randomEndPosition, m_map);		
		return m_startPosition;
	}
	std::vector<IntVector2> adjacentPositions;
	m_map->PushBackAdjacentPosition(adjacentPositions, m_activeNode.m_position);
	for (std::vector<IntVector2>::iterator adjacentPositionIter = adjacentPositions.begin(); adjacentPositionIter != adjacentPositions.end(); ++adjacentPositionIter)
	{
		IntVector2 currentPosition = (*adjacentPositionIter);
		if (CheckOnClosedListAndResetParent(currentPosition))
		{
			continue;
		}
		PathNode* nodeFromOpenList = CheckOnOpenList(currentPosition);
		if (nodeFromOpenList == nullptr)
		{
			//PathNode(float avoidance_cost_g, float distance_cost_g, float parent_g, float h, IntVector2 position, PathNode* parentNode)
			PathNode newNode(1.0f, GetDistanceG(currentPosition), m_activeNode.CaculateG(), CalculateHeuristic(currentPosition, m_goalPosition), currentPosition, &m_activeNode);
			m_openList.push_back(newNode);
			continue;
		}
		else
		{
			float localG = 1.0f + GetDistanceG(currentPosition);
			float parentG = m_activeNode.CaculateG();
			if (localG + parentG < nodeFromOpenList->CaculateG())
			{
				nodeFromOpenList->m_distance_cost_g = GetDistanceG(currentPosition);
				nodeFromOpenList->m_parent_g = parentG;
				nodeFromOpenList->m_parent = &m_closedList.back();
			}
		}
	}
	return m_activeNode.m_position;
		
	
}
//////////////////////////////////////////////////////////////////////////

NTBVertex3D_PCT* AStarPathFinding::CreatePathVerts(int& numberOfVerts)
{
	RGBA red = RGBA(1.0f, 0.0f, 0.0f, 1.0f);
	RGBA blue = RGBA(0.0f, 0.0f, 1.0f, 1.0f);
	RGBA green = RGBA(0.0f, 1.0f, 0.0f, 1.0f);
	int closedListSize = m_closedList.size();
	int openListSize = m_openList.size();
	NTBVertex3D_PCT* verts = new NTBVertex3D_PCT[6 * (closedListSize + openListSize + 1)]; // 1 is end position
	int counter = 0;
	
	float startPosX = m_goalPosition.x * TILE_SCALE + OFFSET.x;
	float startPosY = m_goalPosition.y * TILE_SCALE + OFFSET.y;

	Vector2 mins(startPosX, startPosY - TILE_SCALE);
	Vector2 maxs(startPosX + TILE_SCALE, startPosY);

	verts[counter] = NTBVertex3D_PCT(Vector3(mins.x, maxs.y, 0.0f), Vector2(0.0, 0.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), green);
	verts[counter + 1] = NTBVertex3D_PCT(Vector3(mins.x, mins.y, 0.0f), Vector2(0.0, 1.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), green);
	verts[counter + 2] = NTBVertex3D_PCT(Vector3(maxs.x, mins.y, 0.0f), Vector2(1.0, 1.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), green);
	verts[counter + 3] = NTBVertex3D_PCT(Vector3(mins.x, maxs.y, 0.0f), Vector2(0.0, 0.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), green);
	verts[counter + 4] = NTBVertex3D_PCT(Vector3(maxs.x, mins.y, 0.0f), Vector2(1.0, 1.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), green);
	verts[counter + 5] = NTBVertex3D_PCT(Vector3(maxs.x, maxs.y, 0.0f), Vector2(1.0, 0.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), green);
	counter += 6;

	for (std::list<PathNode>::iterator openListIter = m_openList.begin(); openListIter != m_openList.end(); ++openListIter)
	{
		PathNode currentNode = (*openListIter);
		float startPosX = currentNode.m_position.x * TILE_SCALE + OFFSET.x;
		float startPosY = currentNode.m_position.y * TILE_SCALE + OFFSET.y;

		Vector2 mins(startPosX , startPosY - TILE_SCALE);
		Vector2 maxs(startPosX + TILE_SCALE, startPosY);

		verts[counter] = NTBVertex3D_PCT(Vector3(mins.x, maxs.y, 0.0f), Vector2(0.0, 0.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), blue);
		verts[counter+1] = NTBVertex3D_PCT(Vector3(mins.x, mins.y, 0.0f), Vector2(0.0, 1.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), blue);
		verts[counter+2] = NTBVertex3D_PCT(Vector3(maxs.x, mins.y, 0.0f), Vector2(1.0, 1.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), blue);
		verts[counter+3] = NTBVertex3D_PCT(Vector3(mins.x, maxs.y, 0.0f), Vector2(0.0, 0.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), blue);
		verts[counter+4] = NTBVertex3D_PCT(Vector3(maxs.x, mins.y, 0.0f), Vector2(1.0, 1.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), blue);
		verts[counter+5] = NTBVertex3D_PCT(Vector3(maxs.x, maxs.y, 0.0f), Vector2(1.0, 0.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), blue);
		counter += 6;
	}
	for (std::list<PathNode>::iterator closedListIter = m_closedList.begin(); closedListIter != m_closedList.end(); ++closedListIter)
	{
		PathNode currentNode = (*closedListIter);
		float startPosX = currentNode.m_position.x * TILE_SCALE + OFFSET.x;
		float startPosY = currentNode.m_position.y * TILE_SCALE + OFFSET.y;

		Vector2 mins(startPosX, startPosY - TILE_SCALE);
		Vector2 maxs(startPosX + TILE_SCALE, startPosY);

		verts[counter] = NTBVertex3D_PCT(Vector3(mins.x, maxs.y, 0.0f), Vector2(0.0, 0.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), red);
		verts[counter + 1] = NTBVertex3D_PCT(Vector3(mins.x, mins.y, 0.0f), Vector2(0.0, 1.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), red);
		verts[counter + 2] = NTBVertex3D_PCT(Vector3(maxs.x, mins.y, 0.0f), Vector2(1.0, 1.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), red);
		verts[counter + 3] = NTBVertex3D_PCT(Vector3(mins.x, maxs.y, 0.0f), Vector2(0.0, 0.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), red);
		verts[counter + 4] = NTBVertex3D_PCT(Vector3(maxs.x, mins.y, 0.0f), Vector2(1.0, 1.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), red);
		verts[counter + 5] = NTBVertex3D_PCT(Vector3(maxs.x, maxs.y, 0.0f), Vector2(1.0, 0.0), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), red);
		counter += 6;
	}
	numberOfVerts = counter;
	return verts;
}