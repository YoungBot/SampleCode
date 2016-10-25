//==========================================================
//NPCFactory.cpp
//==========================================================
#include "NPCFactory.hpp"
#include "Engine/Utility/Utility.hpp"
#include "SD4/Roar/Codes/StringTable.hpp"

std::map<std::string, NPCFactory*> NPCFactory::s_npcFactroies;

NPCFactory::NPCFactory(XMLNode& npcNode)
{
	if (npcNode.isEmpty())
		m_prototype = nullptr;
	else
		m_prototype = new NPC(npcNode);
}
//////////////////////////////////////////////////////////////////////////

NPCFactory::~NPCFactory()
{

}
///----------------------------------------------------------
///
///----------------------------------------------------------

bool NPCFactory::LoadAllNPCFactories()
{
	s_npcFactroies.clear();
	std::vector<std::string> allNPCFiles;
	Utility::FindAllFilesOfType("Data/NPCs/", "*.npc.xml", allNPCFiles);
	for (unsigned int i = 0; i < allNPCFiles.size(); i++)
	{
		std::string dir = "Data/NPCs/";
		dir += allNPCFiles[i];
		XMLNode NPCParser = XMLNode::parseFile(dir.c_str(), "NPC");	
		NPCFactory* currentNPC = new NPCFactory(NPCParser);
		s_npcFactroies.insert(std::pair<std::string,NPCFactory*>(currentNPC->m_prototype->m_name,currentNPC));
		StringTable::GetIDByName(currentNPC->m_prototype->m_name);
	}
	return true;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

NPCFactory* NPCFactory::FindFactoryByName(const std::string& name)
{
	std::map<std::string, NPCFactory*>::iterator npcFactoriesIter =  s_npcFactroies.find(name);
	if (npcFactoriesIter != s_npcFactroies.end())
	{
		return npcFactoriesIter->second;
	}
	else
		return nullptr;
}
///----------------------------------------------------------
///
///----------------------------------------------------------

NPC* NPCFactory::CreateNPCFromPrototype()
{
	NPC* newNPC = new NPC(*m_prototype);
	//NPC aaa(newNPC);
	return newNPC;
}