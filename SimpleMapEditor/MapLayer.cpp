//==========================================================
//MapLayer.cpp
//==========================================================
#include "MapLayer.hpp"
#include "LayerDropdownMenu.hpp"
#include "TileSheet.hpp"
#include "TilePropertiesTag.hpp"
#include "MapSlider.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/Utility/Utility.hpp"
#include "Engine/UI/GroupWidget.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/Renderer/Fonts.hpp"


void LayerDropDownEvent(NamedProperties& np)
{
	np;
	LayerDropdownMenu* currentWidget = nullptr;
	np.Get("widget", currentWidget);
	unsigned int index;
	np.Get("index", index);
	MapLayer* currentMapLayer = nullptr;
	np.Get("mapLayer", currentMapLayer);

	std::string text;
	currentWidget->m_options[index]->m_currentStateProperty.fromCode.Get("text", text);
	for (int i = 0; i < NUM_WIDGET_STATE_; i++)
	{
		currentWidget->m_options[0]->m_forStates[i].fromWidget.Set("text", text);
	}
	currentMapLayer->SelectLayer(index-1);
	currentMapLayer->RefreshAllMaps();
}

MapLayer::MapLayer(TileSheet* sheet)
	:m_currentLayerIndex(-1)
	,m_sheet(sheet)
{
	m_layers.clear();
	m_propertiesTag = new TilePropertiesTag(this,Vector2(1.0f, 500.0f));
	DropdownWidget* button = (DropdownWidget*)s_theUISystem->GetSpecificWidgetByName("LayerDropdown");
	m_dropDown = new LayerDropdownMenu(button, this);
	m_dropDown->m_currentStateProperty = m_dropDown->m_forStates[m_dropDown->m_currentState];
	delete button;
	Utility::RegisterEventCallback("LayerDropdown", LayerDropDownEvent);
	Utility::RegisterEventSubscribe("savelayer", *this, &MapLayer::SaveLayerFromConsole);
	Utility::RegisterEventSubscribe("loadlayer", *this, &MapLayer::LoadLayerFromConsole);
	Utility::RegisterEventSubscribe("addmap", *this, &MapLayer::AddMapFromConsole);
	Utility::RegisterEventSubscribe("removemap", *this, &MapLayer::RemoveMapFromConsole);
	Utility::RegisterEventSubscribe("addmapfromfile", *this, &MapLayer::AddMapFromFile);
	Utility::RegisterEventSubscribe("savemaptofile", *this, &MapLayer::SaveMapToFile);
	m_botSliderWidget = new MapSlider((SliderWidget*)s_theUISystem->GetBaseWidget2ByName("SliderWidget"), this);
	m_botSliderWidget->SetMinsAndMaxs(Vector2(200.0f, 80.0f), Vector2(1040.0f, 110.0f));
	m_rightSliderWidget = new MapSlider((SliderWidget*)s_theUISystem->GetBaseWidget2ByName("SliderWidget"), this);
	m_rightSliderWidget->SetMinsAndMaxs(Vector2(1050.0f, 120.0f), Vector2(1080.0f, 840.0f));
	m_rightSliderWidget->m_isHorizon = false;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

MapLayer::~MapLayer()
{

}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::Startup()
{

}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::Update(float deltaSeconds, Vector2& cursorPos)
{
	m_botSliderWidget->Update(deltaSeconds, cursorPos);
	m_rightSliderWidget->Update(deltaSeconds, cursorPos);
	
	m_dropDown->Update(deltaSeconds, cursorPos);
	if (m_layers.size()==0)
	{
		return;
	}
	if (m_currentLayerIndex!=-1)
	{
		m_layers[m_currentLayerIndex]->Update(deltaSeconds, cursorPos);
	}
	m_propertiesTag->Update(deltaSeconds, cursorPos);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::Draw(const Matrix4x4& ortho, const Matrix4x4& I)
{
	for (int i = m_layers.size()-1; i >=0; i--)
	{
		if (m_currentLayerIndex<=i)
		{
			m_layers[i]->Draw(ortho, I);
		}
	}
	m_dropDown->Draw(ortho, I);
	m_botSliderWidget->Draw(ortho, I);
	m_rightSliderWidget->Draw(ortho, I);
	m_propertiesTag->Draw(ortho, I);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::AddMap(EditableMap* newLayer)
{
	m_layers.push_back(newLayer);

	if (m_currentLayerIndex==-1)
	{
		m_currentLayerIndex = 0;
	}
	std::string mapName = newLayer->GetName();

	if (mapName=="")
	{
		mapName = "Layer";
		std::string index = std::to_string(m_dropDown->m_options.size());
		mapName += index;
	}
	if (m_dropDown->m_options.size()==0)
	{
		m_dropDown->AddNewOption(mapName);
		m_dropDown->AddNewOption(mapName);
		return;
	}
	if (m_dropDown->m_options.size() == 1)
	{
		m_dropDown->m_options[0]->SetText(mapName);
	}
	
	m_dropDown->AddNewOption(mapName);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::SelectLayer(int selectedLayerIndex)
{
	m_currentLayerIndex = selectedLayerIndex;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::SaveLayer(const std::string mapName)
{
	XMLNode rootNode = XMLNode::createXMLTopNode("Layer");
	for each (EditableMap* map in m_layers)
	{
		XMLNode data = XMLNode::createXMLTopNode("Data");
		map->SaveMapToNode(data);
		rootNode.addChild(data);
	}
	rootNode.writeToFile(mapName.c_str());
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::SaveLayerFromConsole(NamedProperties& np)
{
	std::string fileName = "";
	np.Get("fileName", fileName);
	SaveLayer(fileName);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::SaveMapToFile(NamedProperties& np)
{
	std::string mapName = "";
	np.Get("mapName", mapName);
	std::string fileName = "";
	np.Get("fileName", fileName);
	XMLNode rootNode = XMLNode::createXMLTopNode("Data");
	for each (EditableMap* map in m_layers)
	{
		if (map->GetName()== mapName)
		{
			map->SaveMapToNode(rootNode);
			rootNode.writeToFile(fileName.c_str());
			return;
		}		
	}	
}

///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::LoadLayer(const std::string mapName)
{
	XMLNode saveFile = XMLNode::parseFile(mapName.c_str());
	if (saveFile.isEmpty())
	{
		return;
	}
	int i = 0;
	XMLNode layerData = saveFile.getChildNode(0);
	XMLNode mapData = layerData.getChildNode(i);
	while (!mapData.isEmpty())
	{
		if (i<m_layers.size())
		{
			m_layers[i]->LoadMapFromNode(mapData);
		}
		else
		{
			TileSheet* newSheet = new TileSheet();
			EditableMap* newMap = new EditableMap(newSheet,this);
			newMap->LoadMapFromNode(mapData);
			newMap->Init();
			AddMap(newMap);
		}
		i++;
		mapData = layerData.getChildNode(i);
	}
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::LoadLayerFromConsole(NamedProperties& np)
{
	std::string fileName = "";
	np.Get("fileName", fileName);
	LoadLayer(fileName);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::AddMapFromFile(NamedProperties& np)
{
	std::string fileName = "";
	np.Get("fileName", fileName);
	XMLNode saveFile = XMLNode::parseFile(fileName.c_str());
	if (saveFile.isEmpty())
	{
		return;
	}
	TileSheet* newSheet = new TileSheet();
	EditableMap* newMap = new EditableMap(newSheet,this);
	newMap->LoadMapFromNode(saveFile);
	newMap->Init();
	AddMap(newMap);
}

///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::RefreshSelectedMap()
{
	m_layers[m_currentLayerIndex]->CreateVertsForMap();
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::RefreshAllMaps()
{
	for each (EditableMap* map in m_layers)
	{
		map->CreateVertsForMap();
	}
}

///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::AddMapFromConsole(NamedProperties& np)
{
	std::string mapName = "";
	np.Get("mapName", mapName);
	EditableMap* newMap = new EditableMap(m_sheet,this);
	newMap->SetName(mapName);
	newMap->Init();
	AddMap(newMap);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void MapLayer::RemoveMapFromConsole(NamedProperties& np)
{
	std::string mapName = "";
	np.Get("mapName", mapName);
	for (std::vector<EditableMap*>::iterator iter = m_layers.begin() ; iter != m_layers.end(); ++iter)
	{
		std::string name = (*iter)->GetName();
		if (name == mapName)
		{
			m_layers.erase(iter);
			m_dropDown->RemoveOption(mapName);
			return;
		}
	}
}
///----------------------------------------------------------
///
///----------------------------------------------------------

EditableMap* MapLayer::GetCurrentMap()
{
	if (m_currentLayerIndex==-1)
	{
		return nullptr;
	}
	return m_layers[m_currentLayerIndex];
}

CONSOLE_COMMAND(savelayer)
{
	NamedProperties* np = new NamedProperties;
	if (args->m_args == nullptr)
	{
		return;
	}
	std::string fileName = args->m_args[0];
	np->Set("fileName", fileName);
	Utility::FireEvent("savelayer", np);
	delete np;
}

CONSOLE_COMMAND(loadlayer)
{
	NamedProperties* np = new NamedProperties;
	if (args->m_args == nullptr)
	{
		return;
	}
	std::string fileName = args->m_args[0];
	np->Set("fileName", fileName);
	Utility::FireEvent("loadlayer", np);
	delete np;
}

CONSOLE_COMMAND(addmap)
{
	NamedProperties* np = new NamedProperties;
	if (args->m_args == nullptr)
	{
		return;
	}
	std::string mapName = args->m_args[0];
	np->Set("mapName", mapName);
	Utility::FireEvent("addmap", np);
	delete np;
}

CONSOLE_COMMAND(removemap)
{
	NamedProperties* np = new NamedProperties;
	if (args->m_args == nullptr)
	{
		return;
	}
	std::string mapName = args->m_args[0];
	np->Set("mapName", mapName);
	Utility::FireEvent("removemap", np);
	delete np;
}

CONSOLE_COMMAND(addmapfromfile)
{
	NamedProperties* np = new NamedProperties;
	if (args->m_args == nullptr)
	{
		return;
	}
	std::string fileName = args->m_args[0];
	np->Set("fileName", fileName);
	Utility::FireEvent("addmapfromfile", np);
	delete np;
}

CONSOLE_COMMAND(savemaptofile)
{
	NamedProperties* np = new NamedProperties;
	if (args->m_args == nullptr)
	{
		return;
	}
	std::string fileName = args->m_args[0];
	np->Set("fileName", fileName);
	std::string mapName = args->m_args[1];
	np->Set("mapName", mapName);	
	Utility::FireEvent("savemaptofile", np);
	delete np;
}
