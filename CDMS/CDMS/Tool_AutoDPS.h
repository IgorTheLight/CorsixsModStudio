/*
    This file is part of Corsix's Mod Studio.

    Corsix's Mod Studio is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Corsix's Mod Studio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Corsix's Mod Studio; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef _TOOL_AUTO_DPS_H_
#define _TOOL_AUTO_DPS_H_

#include <Rainman.h>
#include <map>
#include <vector>

namespace AutoDPS_Internal
{

struct tAutoDPS_WeaponInfo
{
	tAutoDPS_WeaponInfo();
	~tAutoDPS_WeaponInfo();

	char* sFileName;
	wchar_t* sUiName;

	float iMinDamage; // GameData\area_effect\weapon_damage\armour_damage\min_damage
	float iMaxDamage; // GameData\area_effect\weapon_damage\armour_damage\max_damage
	float iAccuracy; // GameData\accuracy
	float iReloadTime; // GameData\reload_time

	float iMinDamageValue; // GameData\area_effect\weapon_damage\armour_damage\min_damage_value
	float iDefaultAP; // GameData\area_effect\weapon_damage\armour_damage\armour_piercing
	// mapped from GameData\area_effect\weapon_damage\armour_damage\armour_piercing_types\entry_01 through 15
	// key: entry_nn\armour_type reference, value: entry_nn\armour_piercing_value
	std::map<char*, float> mapAP;

	// Results of analysis
	std::vector< std::pair<bool, float> > vAnalysisOutput;
};

struct tAutoDPS_AP
{
	tAutoDPS_AP();
	~tAutoDPS_AP();

	char* sFilename;
	char* sNicename;

	std::vector<char*> vUnitList;
};

struct tAutoDPS_Package
{
	tAutoDPS_Package();
	~tAutoDPS_Package();

	std::vector<tAutoDPS_WeaponInfo*> pWeapons;

	std::vector<tAutoDPS_AP*> pAPTypes;

	IDirectoryTraverser* pDirectories;
};


IMetaNode* AutoDPS_GetTableChild(IMetaNode::IMetaTable* pTable, const char* sChildName);

void AutoDPS_FileForEach(IDirectoryTraverser::IIterator* pFileItr, void* pTag);
void AutoDPS_FileForEach_Rgd(CRgdFile* pRgd, tAutoDPS_Package* pPackage, tAutoDPS_WeaponInfo* pWeapon);

void AutoDPS_RacebpsForEach(IDirectoryTraverser::IIterator* pFileItr, void* pTag);
void AutoDPS_RacebpsForEach_Rgd(CRgdFile* pRgd, tAutoDPS_Package* pPackage);

void AutoDPS_EbpsForEach(IDirectoryTraverser::IIterator* pFileItr, void* pTag);
void AutoDPS_EbpsForEach_Rgd(CRgdFile* pRgd, tAutoDPS_Package* pPackage, const char* sFile);

};

namespace AutoDPS
{

AutoDPS_Internal::tAutoDPS_Package* AutoDPS_Scan(IDirectoryTraverser::IIterator* pDirectory);
void AutoDPS_Analyse(AutoDPS_Internal::tAutoDPS_Package* pPackage);
void AutoDPS_AddUnitList(AutoDPS_Internal::tAutoDPS_Package* pPackage, IDirectoryTraverser* pDirectories);
void AutoDPS_OutputHTML(AutoDPS_Internal::tAutoDPS_Package* pPackage, const char* sOutFile);

};

#endif
