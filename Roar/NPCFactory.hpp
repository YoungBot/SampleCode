//==========================================================
//NPCFactory.hpp
//==========================================================
#pragma once
#ifndef _NPCFACTORY_HPP_
#define _NPCFACTORY_HPP_
#include <map>
#include "Engine/Utility/xmlParser.h"
#include "Faction.hpp"
#include "NPC.hpp"
class NPCFactory
{
public:
	NPCFactory(XMLNode& npcNode);
	~NPCFactory();
	static bool LoadAllNPCFactories();
	static NPCFactory* FindFactoryByName(const std::string& name);
	static std::map<std::string, NPCFactory*>& GetAllFactories(){ return s_npcFactroies; }
	NPC* CreateNPCFromPrototype();
protected:
	static std::map<std::string, NPCFactory*> s_npcFactroies;
	NPC* m_prototype;
};

#endif
