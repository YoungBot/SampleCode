//==========================================================
//EditableMap.cpp
//==========================================================
#include "EditableMap.hpp"

#include "Engine/UI/GroupWidget.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/Renderer/GLMeshRenderer.hpp"
#include "Engine/EventSystem/NamedProperties.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/UI/SliderWidget.hpp"
#include "Engine/Renderer/Fonts.hpp"

#include "TilePropertiesTag.hpp"
#include "MapSlider.hpp"
#include "TileSheet.hpp"
#include "MapTile.hpp"
#include "MapTileButton.hpp"
#include "EditableMap.hpp"
#include "TileButton.hpp"
#include "MapLayer.hpp"
EditableMap::EditableMap()
	:m_mapSize(20, 20)
	, m_tileSize(40.0f)
	, m_mapTiles(new GroupWidget())
	, m_position(Vector2(230.0f, 800.0f))
	, m_sheet(nullptr)
	, m_renderer(nullptr)
	, m_offset(IntVector2(0, 0))
	, m_mouseOverIndex(-1)
	, m_windowPositionMin(Vector2(220.0f, 130.0f))
	, m_windowSize(Vector2(880.0f, 770.0f))
	, m_frameSize(IntVector2(21, 18))
	, m_isShowing(true)
	, m_name("")
{
	m_tiles.clear();
	int totalTiles = m_mapSize.x * m_mapSize.y;
	for (unsigned int i = 0; i < totalTiles; i++)
	{
		MapTile* newTile = new MapTile();
		Vector2 offset = GetOffset(i);
		Vector2 tileSize(m_tileSize,m_tileSize);
		Vector2 texCoTopLeft(31.0f / 32.0f,31.0f/32.0f);
		Vector2 texCoBotRight(1.0f,1.0f);

		newTile->m_properties->Set("mins", m_position +offset );
		newTile->m_properties->Set("maxs", m_position +offset+ tileSize);
		newTile->m_properties->Set("texCoTopLeft", aa);
		newTile->m_properties->Set("texCoBotRight", aa+aa);
		m_tiles.push_back(newTile);
	}
	m_renderer = new GLMeshRenderer(VAO_PCT);
	m_renderer->m_material.m_sampler.gTexDiffuse = Texture::CreateOrGetTexture("Data/Images/SimpleMinerAtlas.png");
	m_tileInfoRenderer = new GLMeshRenderer(VAO_PCT);
	m_tileInfoRenderer->m_material.m_sampler.gTexDiffuse = Texture::CreateOrGetTexture(s_font->m_fontTextureFileName);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

EditableMap::EditableMap(TileSheet* sheet, MapLayer* layer)
	:m_mapSize(30, 30)
	, m_tileSize(40.0f)
	, m_mapTiles(new GroupWidget())
	, m_position(Vector2(202.0f, 800.0f))
	, m_sheet(nullptr)
	, m_renderer(nullptr)
	, m_offset(IntVector2(0, 0))
	, m_mouseOverIndex(-1)
	, m_windowPositionMin(Vector2(211.0f, 130.0f))
	, m_windowSize(Vector2(889.0f, 770.0f))
	, m_botSliderWidget(nullptr)
	, m_frameSize(IntVector2(21, 18))
	, m_selectedTile(nullptr)
	, m_isShowing(true)
	, m_name("")
	, m_layer(layer)
{
	m_tiles.clear();
	int totalTiles = m_mapSize.x * m_mapSize.y;
	for (unsigned int i = 0; i < totalTiles; i++)
	{
		MapTile* newTile = new MapTile();
		Vector2 offset = GetOffset(i);
		Vector2 tileSize(m_tileSize, m_tileSize);
		float tileOffset = 0.03125f;
		Vector2 texCoTopLeft(31.0f / 32.0f, 31.0f / 32.0f);
		Vector2 texCoBotRight(1.0f, 1.0f);

		newTile->m_properties->Set("mins", m_position + offset);
		newTile->m_properties->Set("maxs", m_position + offset + tileSize);
		newTile->m_properties->Set("texCoTopLeft", Vector2(6* tileOffset,0.0f));
		newTile->m_properties->Set("texCoBotRight", Vector2(7 * tileOffset, tileOffset));
		newTile->m_tileIndex = 18;
		newTile->m_mapIndes = i;
		m_tiles.push_back(newTile);
	}
	m_renderer = new GLMeshRenderer(VAO_PCT);
	m_renderer->m_material.m_sampler.gTexDiffuse = Texture::CreateOrGetTexture("Data/Images/SimpleMinerAtlas.png");
	SetSheet(sheet);
	m_botSliderWidget = m_layer->GetBotSlider();
	m_rightSliderWidget = m_layer->GetRightSlider();

	Utility::RegisterEventSubscribe("savemap", *this, &EditableMap::SaveMapFromConsole);
	Utility::RegisterEventSubscribe("loadmap", *this, &EditableMap::LoadMapFromConsole);
	Vertex3D_PCT* verts = CreateVertsForMap();
}
///----------------------------------------------------------
///
///----------------------------------------------------------

EditableMap::~EditableMap()
{

}
///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::Update(float deltaSeconds, Vector2& cursorPos)
{
	m_offset.x = m_botSliderWidget->m_percentage * (m_mapSize.x - m_frameSize.x); //30-21
	m_offset.y = m_rightSliderWidget->m_percentage * (m_mapSize.y - m_frameSize.y); //30-18


	Vector2 mins = m_position - Vector2(0.0f, m_tileSize * 17);
	Vector2 maxs = m_position + Vector2(m_tileSize * 21, m_tileSize);
	Vector2 percentOffset(1.0f / (m_mapSize.x - m_frameSize.x), 1.0f / (m_mapSize.y - m_frameSize.y));

	if (cursorPos.x<mins.x || cursorPos.x > maxs.x || cursorPos.y < mins.y || cursorPos.y > maxs.y)
	{
		if (m_mouseOverIndex >= 0)
		{
			m_tiles[m_mouseOverIndex]->m_isMouseOver = false;
		}
		return;
	}
	CheckSelectTile(cursorPos);
	if (s_theInputSystem->IsMouseButtonDown(VK_LBUTTON))
	{
		TileButton* selectTile = nullptr;
		if (m_sheet)
		{
			selectTile = m_sheet->GetSelectTile();
			Vector2 topLeft;
			Vector2 botRight;
			topLeft = selectTile->GetTexCoTopLeft();
			botRight = selectTile->GetTexCoBotRight();
			m_tiles[m_mouseOverIndex]->m_properties->Set("texCoTopLeft", topLeft);
			m_tiles[m_mouseOverIndex]->m_properties->Set("texCoBotRight", botRight);
			m_tiles[m_mouseOverIndex]->m_tileIndex = selectTile->m_currentTileIndex;

		}
	}
	if (s_theInputSystem->WasMouseButtonJustDown(VK_RBUTTON))
	{
		if (m_selectedTile != nullptr)
			m_selectedTile->m_isSelected = false;

		m_selectedTile = m_tiles[m_mouseOverIndex];
		m_selectedTile->m_isSelected = true;
		CreateVertsForMap();
		m_layer->GetPropertiesTag()->UpdateLabels(GetSelectTileCharProperties(), m_mouseOverIndex);
	}
}
	

///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::Draw(const Matrix4x4& ortho, const Matrix4x4& I)
{
	Vector2 mins = m_position - Vector2(0.0f, m_tileSize*(m_mapSize.y - 1));
	//int size = 0;
	//CreateVertsForMap(size);
	
	glScissor(m_windowPositionMin.x, m_windowPositionMin.y, m_windowSize.x, m_windowSize.y);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_SCISSOR_TEST);
	m_renderer->DrawShader(0, 0, ortho, I);
	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5F);
	
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::SetSheet(TileSheet* sheet)
{
	m_sheet = sheet;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

Vertex3D_PCT* EditableMap::CreateVertsForMap()
{
	int size;
	m_offset.x = m_botSliderWidget->m_percentage * (m_mapSize.x - m_frameSize.x); //30-21
	m_offset.y = m_rightSliderWidget->m_percentage * (m_mapSize.y - m_frameSize.y); //30-18
	unsigned int total = m_mapSize.x * m_mapSize.y;
	std::vector<Vertex3D_PCT> verts;
	verts.clear();
	for (unsigned int i = 0; i < total; ++i)
	{
		CreateVertsForTileByIndex(i,verts);
	}
	size = verts.size();

	Vertex3D_PCT* passOutVerts = new Vertex3D_PCT[size];
	memcpy(passOutVerts, verts.data(), verts.size()*sizeof(Vertex3D_PCT));
	m_renderer->m_mesh.UpdateVBO(passOutVerts, size*sizeof(Vertex3D_PCT));
	return passOutVerts;
}

///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::CreateVertsForTileByIndex(unsigned int i, std::vector<Vertex3D_PCT>& verts)
{
	if (m_tiles[i]->m_isShow==false)
		return;
	if (!IsTileInWindow(i))
		return;
	Vector2 offset(m_offset.x * m_tileSize, -m_offset.y * m_tileSize);
	Vector2 mins;
	m_tiles[i]->m_properties->Get("mins", mins);
	Vector2 maxs;
	m_tiles[i]->m_properties->Get("maxs", maxs);
	mins -= offset;
	maxs -= offset;
	Vector2 texCoTopLeft;
	m_tiles[i]->m_properties->Get("texCoTopLeft", texCoTopLeft);
	Vector2 texCoBotRight;
	m_tiles[i]->m_properties->Get("texCoBotRight", texCoBotRight);
	RGBA BGColor;
	if (m_tiles[i]->m_isSelected)
		BGColor = RGBA(1.0f, 0.0f, 1.0f,1.0f);
	else if (m_tiles[i]->m_isMouseOver)
		BGColor = RGBA(0.0f, 0.0f, 1.0f, 1.0f);
	else
		BGColor = RGBA(1.0f, 1.0f, 1.0f, 1.0f);
	
	verts.push_back(Vertex3D_PCT(Vector3(mins.x, maxs.y, 0.0f), Vector2(texCoTopLeft.x, texCoTopLeft.y), BGColor));
	verts.push_back(Vertex3D_PCT(Vector3(mins.x, mins.y, 0.0f), Vector2(texCoTopLeft.x, texCoBotRight.y), BGColor));
	verts.push_back(Vertex3D_PCT(Vector3(maxs.x, mins.y, 0.0f), Vector2(texCoBotRight.x, texCoBotRight.y), BGColor));

	verts.push_back(Vertex3D_PCT(Vector3(mins.x, maxs.y, 0.0f), Vector2(texCoTopLeft.x, texCoTopLeft.y), BGColor));
	verts.push_back(Vertex3D_PCT(Vector3(maxs.x, mins.y, 0.0f), Vector2(texCoBotRight.x, texCoBotRight.y), BGColor));
	verts.push_back(Vertex3D_PCT(Vector3(maxs.x, maxs.y, 0.0f), Vector2(texCoBotRight.x, texCoTopLeft.y), BGColor));
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::CheckSelectTile(Vector2& cursorPos)
{
	//Vector2 mins = m_position - Vector2(0.0f, m_tileSize*(m_mapSize.y-1));
	//Vector2 maxs = m_position + Vector2(m_tileSize*m_mapSize.x, m_tileSize);
	Vector2 mins = m_position - Vector2(0.0f, m_tileSize * 17);
	Vector2 maxs = m_position + Vector2(m_tileSize * 21, m_tileSize);

	Vector2 pos(cursorPos.x - mins.x, cursorPos.y - mins.y);
	int row = 17 - MathUtils::RoundUpToInt(pos.y / m_tileSize);
	int col = MathUtils::RoundUpToInt(pos.x / m_tileSize);
	row += m_offset.y;
	col += m_offset.x;
	int index = row * m_mapSize.x  + (col);
	if (index<=0)
	{
		index = 0;
		return;
	}
	if (index>=899)
	{
		index = 899;
		return;
	}
	if (index == m_mouseOverIndex)
	{
		return;
	}
	if (m_mouseOverIndex >= 0)
	{
		m_tiles[m_mouseOverIndex]->m_isMouseOver = false;
	}	
	m_tiles[index]->m_isMouseOver = true;
	m_mouseOverIndex = index;
	CreateVertsForMap();
}
///----------------------------------------------------------
///
///----------------------------------------------------------

bool EditableMap::IsTileInWindow(int index)
{
	int row = index / m_mapSize.x;
	int col = index % m_mapSize.y;
	row -= m_offset.y;
	col -= m_offset.x;

	if (row >= 0 && row <= 17 && col >= 0 && col <= 20)
	{
		return true;
	}
	else
		return false;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::SaveMap(const std::string mapName)
{
	//textureFileName,tile texco,
	XMLNode rootNode = XMLNode::createXMLTopNode("Data");
	SaveMapToNode(rootNode);
	rootNode.writeToFile(mapName.c_str());
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::SaveMapToNode(XMLNode& parentNode)
{
	parentNode.addAttribute("name", m_name.c_str());
	XMLNode mapNode = XMLNode::createXMLTopNode("MapData");
	std::string mapdata = m_sheet->GetSheetName();
	mapNode.addText(mapdata.c_str());
	for (int i = 0; i < m_tiles.size(); i++)
	{
		XMLNode tileNode = XMLNode::createXMLTopNode("Tile");
		std::string tileData = "";
		char indexChar[10];
		_itoa_s(i, indexChar, 10);
		tileData += indexChar;
		tileData += ",";

		Vector2 texCoTopLeft;
		Vector2 texCoBotRight;
		int index = m_tiles[i]->m_tileIndex;
		tileData += std::to_string(index);
		tileNode.addText(tileData.c_str());
		mapNode.addChild(tileNode);
	}
	parentNode.addChild(mapNode);
	SaveMapTileProperties(mapNode);
	m_sheet->SaveProperties(parentNode);
}

///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::SaveMapFromConsole(NamedProperties& np)
{
	std::string fileName = "";
	np.Get("fileName",fileName);
	SaveMap(fileName);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::SaveMapTileProperties(XMLNode& parentNode)
{
	XMLNode propertiesNode = parentNode.addChild("MapTileProperties");
	for (std::map<int, std::vector<PropertiesStruct>*>::iterator iter = m_charPropertiesByIndex.begin(); iter != m_charPropertiesByIndex.end(); ++iter)
	{
		XMLNode node = XMLNode::createXMLTopNode("property");
		int index = iter->first;
		std::string indexString = std::to_string(index);
		node.addAttribute("index", indexString.c_str());
		std::vector<PropertiesStruct>* currentVector = iter->second;
		if (currentVector->size() == 0)
		{
			continue;
		}
		propertiesNode.addChild(node);
		for (unsigned int i = 0; i < currentVector->size(); i++)
		{
			XMLNode propertyStruct = node.addChild("propertyStruct");
			std::string key = (*currentVector)[i].key;
			std::string value = (*currentVector)[i].value;
			key += " ";
			key += value;
			propertyStruct.addText(key.c_str());
		}
	}
}

///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::LoadMap(const std::string mapName)
{
	XMLNode saveFile = XMLNode::parseFile(mapName.c_str());
	if (saveFile.isEmpty())
		return;
	LoadMapFromNode(saveFile);
	CreateVertsForMap();
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::LoadMapFromNode(XMLNode& parentNode)
{
	if (parentNode.isEmpty())
		return;
	std::string name = parentNode.getName();
	XMLNode data;
	if (name == "Data")
	{
		data = parentNode;
	}
	else
	{
		data = parentNode.getChildNode("Data");
	}
	std::string mapName = data.getAttribute("name");
	m_name = mapName;
	XMLNode mapNode = data.getChildNode("MapData");
	std::string sheetName = mapNode.getText();
	int i = 0;
	XMLNode tileNode = mapNode.getChildNode(i);
	std::string mapTileName = "MapTileProperties";
	
	while (!tileNode.isEmpty() && strcmp(tileNode.getName(), mapTileName.c_str()))
	{
		int tileNumber;
		int tileIndex;
		Vector2 topLeft(0.0f, 0.0f);
		Vector2 botRight(0.0f, 0.0f);
		std::string data = tileNode.getText();
		std::string pieceOfData = Utility::CutString(data, ',');
		tileNumber = atoi(pieceOfData.c_str());
		pieceOfData = Utility::CutString(data, ',');
		tileIndex = atof(pieceOfData.c_str());
		topLeft = Vector2(m_sheet->GetTexCoByTileIndex(tileIndex));
		Vector2 offset(m_sheet->GetTileOffset());
		botRight = topLeft + offset;
		m_tiles[tileNumber]->m_properties->Set("texCoTopLeft", topLeft);
		m_tiles[tileNumber]->m_properties->Set("texCoBotRight", botRight);
		m_tiles[tileNumber]->m_tileIndex = tileIndex;
		i++;
		tileNode = mapNode.getChildNode(i);
	}
	LoadMapTileProperties(tileNode);
	m_sheet->LoadProperties(data.getChildNode("Properties"));
}

///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::LoadMapFromConsole(NamedProperties& np)
{
	std::string fileName = "";
	np.Get("fileName", fileName);
	LoadMap(fileName);
	
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::LoadMapTileProperties(XMLNode& parentNode)
{
	int i = 0;
	XMLNode currentProperty = parentNode.getChildNode(i);
	while (!currentProperty.isEmpty())
	{
		int index = 0;
		const char* indexNum = currentProperty.getAttribute("index");
		index = atoi(indexNum);
		int j = 0;
		XMLNode currentPropertyStruct = currentProperty.getChildNode(j);
		while (!currentPropertyStruct.isEmpty())
		{
			std::string propertyStruct = currentPropertyStruct.getText();

			std::string key = Utility::CutString(propertyStruct, ' ');
			std::string value = Utility::CutString(propertyStruct, ' ');
			AddProperty(index, key, value);
			j++;
			currentPropertyStruct = currentProperty.getChildNode(j);
		}
		i++;
		currentProperty = parentNode.getChildNode(i);
	}
}

///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::DrawTileInfo(const Matrix4x4& ortho, const Matrix4x4& I)
{
	
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::AddProperty(int index, std::string key, std::string value)
{
	std::map<int, std::vector<PropertiesStruct>*>::iterator iter = m_charPropertiesByIndex.find(index);
	PropertiesStruct currentStruct(key, value);
	if (iter == m_charPropertiesByIndex.end())
	{
		std::vector<PropertiesStruct>* newVector = new std::vector<PropertiesStruct>;
		newVector->push_back(currentStruct);
		m_charPropertiesByIndex.insert(std::pair<int, std::vector<PropertiesStruct>*>(index, newVector));
	}
	else
	{
		iter->second->push_back(currentStruct);
	}
}

///----------------------------------------------------------
///
///----------------------------------------------------------

std::vector<PropertiesStruct>* EditableMap::CreateProperties(int index)
{
	std::vector<PropertiesStruct>* propertiesVec = new std::vector<PropertiesStruct>;
	m_charPropertiesByIndex.insert(std::pair<int, std::vector<PropertiesStruct>*>(index, propertiesVec));
	return propertiesVec;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

std::vector<PropertiesStruct>* EditableMap::GetSelectTileCharProperties()
{
	if (m_selectedTile == nullptr)
	{
		return nullptr;
	}
	std::map<int, std::vector<PropertiesStruct>*>::iterator iter = m_charPropertiesByIndex.find(m_selectedTile->m_mapIndes);
	if (iter == m_charPropertiesByIndex.end())
	{
		return nullptr;
	}
	else
		return iter->second;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

///----------------------------------------------------------
///
///----------------------------------------------------------

void EditableMap::Init()
{
	m_botSliderWidget->m_currentStateProperty = m_botSliderWidget->m_forStates[m_botSliderWidget->m_currentState];
	m_rightSliderWidget->m_currentStateProperty = m_rightSliderWidget->m_forStates[m_rightSliderWidget->m_currentState];
}

///----------------------------------------------------------
///
///----------------------------------------------------------

Vector2 EditableMap::GetOffset(int index)
{
	float xOffset = m_tileSize * (index % m_mapSize.y);
	float yOffset = -m_tileSize * (index / m_mapSize.y);
	return Vector2(xOffset, yOffset);
}

CONSOLE_COMMAND(savemap)
{
	NamedProperties* np = new NamedProperties;
	if (args->m_args == nullptr)
	{
		return;
	}
	std::string fileName = args->m_args[0];
	np->Set("fileName", fileName);
	Utility::FireEvent("savemap",np);
	delete np;
}

CONSOLE_COMMAND(loadmap)
{
	NamedProperties* np = new NamedProperties;
	if (args->m_args == nullptr)
	{
		return;
	}
	std::string fileName = args->m_args[0];
	np->Set("fileName", fileName);
	Utility::FireEvent("loadmap", np);
	delete np;
}