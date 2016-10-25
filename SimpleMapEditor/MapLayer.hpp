#pragma once
#ifndef _MAPLAYER_HPP_
#define _MAPLAYER_HPP_
#include "EditableMap.hpp"
#include <vector>
class TileSheet;
class Vector2;
class Matrix4x4;
class LayerDropdownMenu;
class MapSlider;
class MapLayer
{
public:
	MapLayer(TileSheet* sheet);
	~MapLayer();
	void Startup();
	void Update(float deltaSeconds, Vector2& cursorPos);
	void Draw(const Matrix4x4& ortho, const Matrix4x4& I);
	void AddMap(EditableMap* newLayer);
	void SelectLayer(int selectedLayerIndex);
	void SaveLayer(const std::string mapName);
	void SaveLayerFromConsole(NamedProperties& np);
	void SaveMapToFile(NamedProperties& np);
	void LoadLayer(const std::string mapName);
	void LoadLayerFromConsole(NamedProperties& np);
	void AddMapFromFile(NamedProperties& np);
	void RefreshSelectedMap();
	void RefreshAllMaps();
	void AddMapFromConsole(NamedProperties& np);
	void RemoveMapFromConsole(NamedProperties& np);
	unsigned int GetNumerOfLayers() { return m_layers.size(); }
	MapSlider* GetBotSlider() { return m_botSliderWidget; }
	MapSlider* GetRightSlider() { return m_rightSliderWidget; }
	EditableMap* GetCurrentMap();
	TilePropertiesTag* GetPropertiesTag() { return m_propertiesTag; }
private:
	int m_currentLayerIndex;
	LayerDropdownMenu* m_dropDown;
	TileSheet* m_sheet;
	MapSlider* m_botSliderWidget;
	MapSlider* m_rightSliderWidget;
	TilePropertiesTag* m_propertiesTag;
	std::vector<EditableMap*> m_layers;
};
#endif