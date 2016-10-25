//==========================================================
//EditableMap.hpp
//==========================================================
#pragma once
#ifndef _EDITABLEMAP_HPP_
#define _EDITABLEMAP_HPP_
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "TilePropertiesTag.hpp"
#include <vector>
#include <map>
#include <string>

struct XMLNode;
class Matrix4x4;
class GroupWidget;
class TileSheet;
class GLMeshRenderer;
class MapTile;
class Vertex3D_PCT;
class MapSlider;
class TilePropertiesTag;
class NamedProperties;
class MapLayer;
class EditableMap
{
public:
	EditableMap();
	EditableMap(TileSheet* sheet, MapLayer* layer = nullptr);
	~EditableMap();
	void Update(float deltaSeconds, Vector2& cursorPos);
	void Draw(const Matrix4x4& ortho, const Matrix4x4& I);
	void SetSheet(TileSheet* sheet);
	Vertex3D_PCT* CreateVertsForMap();
	void CreateVertsForTileByIndex(unsigned int i, std::vector<Vertex3D_PCT>& verts);
	void CheckSelectTile(Vector2& cursorPos);
	bool IsTileInWindow(int index);
	void SaveMap(const std::string mapName);
	void SaveMapToNode(XMLNode& parentNode);
	void SaveMapFromConsole(NamedProperties& np);
	void SaveMapTileProperties(XMLNode& parentNode);
	void SetName(std::string name) { m_name = name; }
	void LoadMap(const std::string mapName);
	void LoadMapFromNode(XMLNode& parentNode);
	void LoadMapFromConsole(NamedProperties& np);
	void LoadMapTileProperties(XMLNode& parentNode);
	void DrawTileInfo(const Matrix4x4& ortho, const Matrix4x4& I);
	bool GetIsShowing() { return m_isShowing; }
	void AddProperty(int index, std::string key, std::string value);
	std::vector<PropertiesStruct>* CreateProperties(int index);
	std::vector<PropertiesStruct>* GetSelectTileCharProperties();
	std::string GetName() { return m_name; }
	void Init();
private:
	Vector2 GetOffset(int index);
	
	IntVector2 m_mapSize;
	IntVector2 m_frameSize;
	float m_tileSize;
	Vector2 m_position;
	IntVector2 m_offset;
	Vector2 m_windowPositionMin;
	Vector2 m_windowSize;
	std::string m_name;
	int m_mouseOverIndex;
	bool m_isShowing;
	
	GroupWidget* m_mapTiles;
	TileSheet* m_sheet;
	GLMeshRenderer* m_renderer;
	GLMeshRenderer* m_tileInfoRenderer;
	MapSlider* m_botSliderWidget;
	MapSlider* m_rightSliderWidget;
	MapTile* m_selectedTile;
	MapLayer* m_layer;

	std::vector<MapTile*> m_tiles;
	std::map<int, std::vector<PropertiesStruct>*> m_charPropertiesByIndex;
};

#endif