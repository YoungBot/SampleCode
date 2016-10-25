#pragma once
#ifndef _TILESHEET_HPP_
#define _TILESHEET_HPP_

#include "Engine/Renderer/Texture.hpp"
#include <string>
#include <vector>
#include <map>
struct XMLNode;
class TileButton;
class NamedProperties;
class GroupWidget;
class RectButton;
class Matrix4x4;
class TilePropertiesTag;
struct PropertiesStruct;

class TileSheet
{
public:
	TileSheet();
	~TileSheet();
	void Draw(const Matrix4x4& ortho, const Matrix4x4& I);
	void Update(float deltaSeconds, Vector2& cursorPos);
	void PreviousPage(NamedProperties& np);
	void NextPage(NamedProperties& np);
	void SetSelectedTileTexCO(Vector2 min, Vector2 max, int index);
	void SaveProperties(XMLNode& rootNode);
	void LoadProperties(XMLNode& propertiesNode);
	std::vector<PropertiesStruct>* GetSelectTileCharProperties();
	Vector2 GetOffset(int index);
	Vector2 GetTileOffset() { return Vector2(1.0f/m_tileWidthAndHeight.x,1.0f/m_tileWidthAndHeight.y); }
	Vector2 GetTexCo(int index);
	Vector2 GetTexCoByTileIndex(int tileIndex);
	TileButton* GetSelectTile();
	std::string GetSheetName();
	void AddProperty(int index, std::string key, std::string value);
	std::vector<PropertiesStruct>* CreateProperties(int index);
private:
	void UpdateTexCoForAllTiles();

	unsigned int m_currentPage;
	std::string m_tileSheetName;
	IntVector2 m_rowAndColumn;
	Vector2 m_position;
	Vector2 m_tileWidthAndHeight;
	float m_tileSize;

	TilePropertiesTag* m_propertiesTag;
	GroupWidget* m_tileButtons;
	RectButton* m_nextPageButton;
	RectButton* m_prevPageButton;
	TileButton* m_selectedTile;
	std::map<int, std::vector<PropertiesStruct>*> m_charPropertiesByIndex;
};

#endif