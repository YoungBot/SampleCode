#include "TileSheet.hpp"
#include "Engine/UI/RectButton.hpp"
#include "Engine/UI/GroupWidget.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/Math/Math2D.hpp"
#include "Engine/Utility/Utility.hpp"
#include "TileButton.hpp"
#include "TilePropertiesTag.hpp"
#include "Engine/Input/InputSystem.hpp"

int g_selectTileIndex = 0;
TileSheet::TileSheet()
	:m_tileWidthAndHeight(32.0f,32.0f)
	,m_currentPage(0)
	,m_rowAndColumn(4,4)
	,m_tileButtons(new GroupWidget())
	,m_position(1200.0f,700.0f)
	,m_tileSheetName("Data/Images/SimpleMinerAtlas.png")
	, m_tileSize(60.0f)
	,m_nextPageButton(nullptr)
	,m_prevPageButton(nullptr)
	,m_selectedTile(nullptr)
{
	m_charPropertiesByIndex.clear();
	m_propertiesTag = new TilePropertiesTag(this, Vector2(1360.0f, 730.0f));
	
	int totalShowTiles = m_rowAndColumn.x * m_rowAndColumn.y;
	for (int i = 0; i < totalShowTiles; i++)
	{
		RectButton* button = (RectButton*)s_theUISystem->GetSpecificWidgetByName("TileButton");
		TileButton* currentButton = new TileButton(button, this);
		delete button;
		Vector2 offset = GetOffset(i);
		offset -= Vector2(100.0f, 0.0f);
		currentButton->SetMinsAndMaxs(m_position + offset, m_position + offset + Vector2(m_tileSize-1.0f, m_tileSize-1.0f));
		Vector2 texCo = GetTexCo(i);
		Vector2 tileTexSize(1.0f / 32.0f, 1.0f / 32.0f);
		currentButton->SetTexCo(texCo, texCo + tileTexSize);
		char indexChar[10];
		_itoa_s(i, indexChar,10);
		std::string eventName = "selectTile";
		eventName += indexChar;
		Utility::RegisterEventSubscribe(eventName, *currentButton, &TileButton::SetSelectButtonTexco);
		currentButton->SetEvent(eventName);
		currentButton->m_currentTileIndex = i;
		m_tileButtons->AddWidget(currentButton);
	}
	//////////////////////////////////////////////////////////////////////////
	RectButton* button = (RectButton*)s_theUISystem->GetSpecificWidgetByName("TileButton");
	m_selectedTile = new TileButton(button, this);
	Vector2 selectTileOffset = Vector2(2.0f*m_tileSize,2.0f*m_tileSize);
	Vector2 selectTilePosition = m_position + Vector2(m_tileSize, -m_tileSize*(m_rowAndColumn.x + 2));
	m_selectedTile->SetMinsAndMaxs(selectTilePosition, selectTilePosition + selectTileOffset);
	m_tileButtons->AddWidget(m_selectedTile);
	//////////////////////////////////////////////////////////////////////////
	Vector2 nextButtonOffset = Vector2(80.0f, 30.0f);
	Vector2 prevPageButtonPosition = m_position + Vector2(0.0f, -m_tileSize*(m_rowAndColumn.x + 4));
	m_prevPageButton = (RectButton*)s_theUISystem->GetSpecificWidgetByName("NextButton");
	m_prevPageButton->SetMinsAndMaxs(prevPageButtonPosition, prevPageButtonPosition + nextButtonOffset);
	m_prevPageButton->SetText("Prev");
	m_prevPageButton->SetEvent("previousPage");
	Utility::RegisterEventSubscribe("previousPage", *this, &TileSheet::PreviousPage);
	//Utility::RegisterEventCallback("previousPage", (this->PreviousPage));
	m_tileButtons->AddWidget(m_prevPageButton);
	//////////////////////////////////////////////////////////////////////////
	Vector2 nextPageButtonPosition = prevPageButtonPosition + Vector2(160.0f,0.0f);
	m_nextPageButton = (RectButton*)s_theUISystem->GetSpecificWidgetByName("NextButton");
	m_nextPageButton->SetMinsAndMaxs(nextPageButtonPosition, nextPageButtonPosition + nextButtonOffset);
	//Utility::RegisterEventCallback("nextPage", (this->NextPage));
	m_nextPageButton->SetEvent("nextPage");
	Utility::RegisterEventSubscribe("nextPage", *this, &TileSheet::NextPage);
	m_tileButtons->AddWidget(m_nextPageButton);
	
	std::vector<PropertiesStruct>* test = new std::vector<PropertiesStruct>;
	PropertiesStruct aa;
	aa.key = "testkey";
	aa.value = "testValue";
	test->push_back(aa);
	//test->insert(std::pair<std::string, std::string>("testKey", "testValue"));
	m_charPropertiesByIndex.insert(std::pair<int, std::vector<PropertiesStruct>*>(1, test));

}
///----------------------------------------------------------
///
///----------------------------------------------------------

TileSheet::~TileSheet()
{

}
///----------------------------------------------------------
///
///----------------------------------------------------------

void TileSheet::Draw(const Matrix4x4& ortho, const Matrix4x4& I)
{
	m_tileButtons->Draw(ortho,I);
	m_propertiesTag->Draw(ortho, I);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void TileSheet::Update(float deltaSeconds, Vector2& cursorPos)
{
	m_tileButtons->Update(deltaSeconds, cursorPos);
	m_propertiesTag->Update(deltaSeconds, cursorPos);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void TileSheet::PreviousPage(NamedProperties& np)
{
	np;
	if (m_currentPage == 0)
		m_currentPage = 63;
	else
		m_currentPage--;

	UpdateTexCoForAllTiles();
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void TileSheet::NextPage(NamedProperties& np)
{
	np;
	if (m_currentPage == 64)
		m_currentPage = 0;
	else
		m_currentPage++;

	UpdateTexCoForAllTiles();
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void TileSheet::SetSelectedTileTexCO(Vector2 min, Vector2 max, int index)
{
	m_selectedTile->SetTexCo(min, max);
	m_selectedTile->m_currentTileIndex = index;
	m_propertiesTag->UpdateLabels(GetSelectTileCharProperties(),index);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void TileSheet::SaveProperties(XMLNode& rootNode)
{
	XMLNode propertiesNode = rootNode.addChild("Properties");
	for (std::map<int, std::vector<PropertiesStruct>*>::iterator iter = m_charPropertiesByIndex.begin(); iter != m_charPropertiesByIndex.end(); ++iter)
	{
		XMLNode node = XMLNode::createXMLTopNode("property");
		int index = iter->first;
		std::string indexString = std::to_string(index);
		node.addAttribute("index",indexString.c_str());
		std::vector<PropertiesStruct>* currentVector = iter->second;
		if (currentVector->size()==0)
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

void TileSheet::LoadProperties(XMLNode& propertiesNode)
{
	int i = 0;
	XMLNode currentProperty = propertiesNode.getChildNode(i);
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
		currentProperty = propertiesNode.getChildNode(i);
	}
	
}
///----------------------------------------------------------
///
///----------------------------------------------------------

std::vector<PropertiesStruct>* TileSheet::GetSelectTileCharProperties()
{
	if (m_selectedTile==nullptr)
	{
		return nullptr;
	}
	std::map<int, std::vector<PropertiesStruct>*>::iterator iter = m_charPropertiesByIndex.find(m_selectedTile->m_currentTileIndex);
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

Vector2 TileSheet::GetOffset(int index)
{
	float xOffset =  m_tileSize * (index % m_rowAndColumn.y) ;
	float yOffset = -m_tileSize * (index / m_rowAndColumn.y);
	return Vector2(xOffset, yOffset);
}
///----------------------------------------------------------
///
///----------------------------------------------------------

Vector2 TileSheet::GetTexCo(int index)
{
	Vector2 pageOffset;
	Vector2 tileOffset;
	//hard code page num:  8*8 = 64
	float pageSize = 1.0f / 8.0f;
	float tileSize = 1.0f / 32.0f;
	int pageRow = m_currentPage % 8;
	int pageColumn = m_currentPage / 8;
	pageOffset.x = pageSize * pageRow;
	pageOffset.y = pageSize * pageColumn;

	float tileRow = index % m_rowAndColumn.y;
	float tileColumn = index / m_rowAndColumn.y;
	tileOffset.x = tileSize * tileRow;
	tileOffset.y = tileSize * tileColumn;

	return pageOffset + tileOffset;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

Vector2 TileSheet::GetTexCoByTileIndex(int tileIndex)
{
	Vector2 pageOffset;
	Vector2 tileOffset;
	float pageSize = 1.0f / 8.0f;
	float tileSize = 1.0f / 32.0f;
	int pageNumber = tileIndex / (m_rowAndColumn.x * m_rowAndColumn.y);
	int pageRow = pageNumber % 8;
	int pageColumn = pageNumber / 8;
	pageOffset.x = pageSize * pageRow;
	pageOffset.y = pageSize * pageColumn;
	int index = tileIndex % (m_rowAndColumn.x * m_rowAndColumn.y);

	float tileRow = index % m_rowAndColumn.y;
	float tileColumn = index / m_rowAndColumn.y;
	tileOffset.x = tileSize * tileRow;
	tileOffset.y = tileSize * tileColumn;

	return pageOffset + tileOffset;
}

///----------------------------------------------------------
///
///----------------------------------------------------------

TileButton* TileSheet::GetSelectTile()
{
	return m_selectedTile;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

std::string TileSheet::GetSheetName()
{
	return m_tileSheetName;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void TileSheet::AddProperty(int index, std::string key, std::string value)
{
	std::map<int, std::vector<PropertiesStruct>*>::iterator iter = m_charPropertiesByIndex.find(index);
	PropertiesStruct currentStruct(key,value);
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

std::vector<PropertiesStruct>* TileSheet::CreateProperties(int index)
{
	std::vector<PropertiesStruct>* propertiesVec = new std::vector<PropertiesStruct>;
	//test->insert(std::pair<std::string, std::string>("testKey", "testValue"));
	//PropertiesStruct empty;
	//empty.key = "";
	//empty.value = "";
	//propertiesVec->push_back(empty);
	m_charPropertiesByIndex.insert(std::pair<int, std::vector<PropertiesStruct>*>(index, propertiesVec));
	return propertiesVec;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

void TileSheet::UpdateTexCoForAllTiles()
{
	int totalShowTiles = m_rowAndColumn.x * m_rowAndColumn.y;
	for (int i = 0; i < totalShowTiles; i++)
	{
		TileButton* currentButton = (TileButton*)m_tileButtons->m_widgets[i];
		Vector2 texCo = GetTexCo(i);
		Vector2 tileTexSize(1.0f / 32.0f, 1.0f / 32.0f);
		currentButton->SetTexCo(texCo, texCo + tileTexSize);
		currentButton->m_currentTileIndex = i + m_currentPage * (m_rowAndColumn.x *m_rowAndColumn.y);
	}
}

