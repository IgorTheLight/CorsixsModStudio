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

/*
[23:44] <Kresjah> Yup, it has it's own almost full weapons profile... two secs for the variable name
[23:44] <Kresjah> GameData["special_attack_ext"]["special_attacks"]["special_attack_XX"]       everything after that is whatever you'd have in any other weapons LUA
[23:45] <Kresjah> Area affect, throw data, weapon damage (including ap, min-max damage), etc.
[23:45] <Kresjah> Extra special fields are:
[23:45] <dreddnott> Tomb Spyder definitely has one for close-combat, it murders people
[23:45] <Kresjah> GameData["special_attack_ext"]["special_attacks"]["special_attack_XX"]["chance"] = Chance in percentage if I'm not mistaken
[23:46] <Kresjah> GameData["special_attack_ext"]["special_attacks"]["special_attack_XX"]["duration"] = How long in seconds the attack lasts (for the sake of syncing with anim and knowing when to go back to "normal behaviour")
[23:46] <Kresjah> GameData["special_attack_ext"]["time_between_special_attacks"] = 5.00000
[23:46] <Kresjah> GameData["special_attack_ext"]["time_between_special_attacks_random"] = 1.00000
[23:47] <Kresjah> Guessing the first one is the default time between special attacks
[23:47] <Corsix> and the second?
[23:47] <Kresjah> The second one (random) telling how much longer (or even possibly earlier) it can start a special attack to make timing seem more random
[23:47] <Kresjah> Meaning it will launch one every 4-6 second
[23:47] <Corsix> ok
[23:48] <Kresjah> Although that random thingy... that's just an educated guess
[00:01] <Kresjah> Oh, one more thing
[00:01] <Kresjah> Special attacks are melee only
[00:08] <dreddnott> they interrupt the normal delivery of damage, correct?
[00:08] <Kresjah> Yup
*/

#include "Tool_AutoDPS.h"
#include <algorithm>
#include "Common.h"
using namespace AutoDPS_Internal;
using namespace AutoDPS;

AutoDPS_Internal::tAutoDPS_WeaponInfo::tAutoDPS_WeaponInfo()
	: sFileName(0), sUiName(0), iMinDamage(0.0), iMaxDamage(0.0), iAccuracy(0.0), iReloadTime(1.0), iMinDamageValue(0.0), iDefaultAP(0.0) {}

AutoDPS_Internal::tAutoDPS_WeaponInfo::~tAutoDPS_WeaponInfo()
{
	free(sFileName);
	//wchar_t* sUiName; ??

	for(std::map<char*, float>::iterator itr = mapAP.begin(); itr != mapAP.end(); ++itr)
	{
		free(itr->first);
	}
}

AutoDPS_Internal::tAutoDPS_Package::tAutoDPS_Package() : pDirectories(0) {}

AutoDPS_Internal::tAutoDPS_Package::~tAutoDPS_Package()
{
	for(std::vector<tAutoDPS_WeaponInfo*>::iterator itr = pWeapons.begin(); itr != pWeapons.end(); ++itr)
	{
		delete *itr;
	}

	for(std::vector<tAutoDPS_AP*>::iterator itr = pAPTypes.begin(); itr != pAPTypes.end(); ++itr)
	{
		delete *itr;
	}
}

AutoDPS_Internal::tAutoDPS_AP::tAutoDPS_AP() :sFilename(0),	sNicename(0) {}
AutoDPS_Internal::tAutoDPS_AP::~tAutoDPS_AP()
{
	free(sFilename);
	free(sNicename);

	for(std::vector<char*>::iterator itr = vUnitList.begin(); itr != vUnitList.end(); ++itr)
	{
		free(*itr);
	}
}

IMetaNode* AutoDPS_Internal::AutoDPS_GetTableChild(IMetaNode::IMetaTable* pTable, const char* sChildName)
{
	// Some useful variables
	IMetaNode::IMetaTable* pTableToDelete = 0;

	// Identify the name part
	const char* sSeperator = strchr(sChildName, '\\');
	if(sSeperator == 0) sSeperator = sChildName + strlen(sChildName);

	size_t iNamePartLen = sSeperator - sChildName;
	const char* sNamePart = sChildName;
	unsigned long iNamePartHash = CRgdHashTable::ValueToHashStatic(sNamePart, iNamePartLen);

	// Get table stats
search_again:
	unsigned long iTableChildrenCount = pTable->VGetChildCount();
	IMetaNode* pTableChild = 0;

	// Begin looking
	for(unsigned long iTableChild = 0; iTableChild < iTableChildrenCount; ++iTableChild)
	{
		pTableChild = pTable->VGetChild(iTableChild);

		// Test if this child has right name
		if(pTableChild->VGetName())
		{
			if(strlen(pTableChild->VGetName()) == iNamePartLen && strncmp(pTableChild->VGetName(), sNamePart, iNamePartLen) == 0)
			{
				goto got_child;
			}
		}
		else
		{
			if(pTableChild->VGetNameHash() == iNamePartHash)
			{
				goto got_child;
			}
		}

		// Move on to next child
		delete pTableChild;
	}
	// Error: child not found

	if(pTableToDelete) delete pTableToDelete;
	return 0;

	// Found child
got_child:

	if(*sSeperator == 0) // If nothing more to get, then return
	{
		if(pTableToDelete) delete pTableToDelete;
		return pTableChild;
	}

	// Find next seperator part
	++sSeperator;
	const char* sNextSeperator = strchr(sSeperator, '\\');
	if(sNextSeperator == 0) sNextSeperator = sSeperator + strlen(sSeperator);

	iNamePartLen = sNextSeperator - sSeperator;
	sNamePart = sSeperator;
	sSeperator = sNextSeperator;
	iNamePartHash = CRgdHashTable::ValueToHashStatic(sNamePart, iNamePartLen);

	// Get table of this child
	if(pTableToDelete) delete pTableToDelete;
	pTable = pTableToDelete = pTableChild->VGetValueMetatable();
	delete pTableChild;

	// Go again
	goto search_again;
}

void AutoDPS_Internal::AutoDPS_FileForEach_Rgd(CRgdFile* pRgd, tAutoDPS_Package* pPackage, tAutoDPS_WeaponInfo* pWeapon)
{
	IMetaNode::IMetaTable* pGameDataTable, *pAPTable;
	pGameDataTable = pRgd->VGetValueMetatable();

	IMetaNode* pNode;

	pNode = AutoDPS_GetTableChild(pGameDataTable, "area_effect\\weapon_damage\\armour_damage\\min_damage");
	pWeapon->iMinDamage = pNode ? pNode->VGetValueFloat() : 0.0f;
	delete pNode;

	pNode = AutoDPS_GetTableChild(pGameDataTable, "area_effect\\weapon_damage\\armour_damage\\max_damage");
	pWeapon->iMaxDamage = pNode ? pNode->VGetValueFloat() : 0.0f;
	delete pNode;

	pNode = AutoDPS_GetTableChild(pGameDataTable, "area_effect\\weapon_damage\\armour_damage\\min_damage_value");
	pWeapon->iMinDamageValue = pNode ? pNode->VGetValueFloat() : 0.0f;
	delete pNode;

	pNode = AutoDPS_GetTableChild(pGameDataTable, "area_effect\\weapon_damage\\armour_damage\\armour_piercing");
	pWeapon->iDefaultAP = pNode ? pNode->VGetValueFloat() : 0.0f;
	if(pWeapon->iDefaultAP > 100.0f) pWeapon->iDefaultAP = 100.0f;
	delete pNode;

	pNode = AutoDPS_GetTableChild(pGameDataTable, "accuracy");
	pWeapon->iAccuracy = pNode ? pNode->VGetValueFloat() : 0.0f;
	delete pNode;

	pNode = AutoDPS_GetTableChild(pGameDataTable, "reload_time");
	pWeapon->iReloadTime = pNode ? pNode->VGetValueFloat() : 0.0f;
	delete pNode;

	pNode = AutoDPS_GetTableChild(pGameDataTable, "area_effect\\weapon_damage\\armour_damage\\armour_piercing_types");
	if(pNode)
	{
		pAPTable = pNode->VGetValueMetatable();
		delete pNode;

		unsigned long iAPCount = pAPTable->VGetChildCount();
		for(unsigned long i = 0; i < iAPCount; ++i)
		{
			IMetaNode* pChild = pAPTable->VGetChild(i);
			IMetaNode::IMetaTable* pAPEntryTable = pChild->VGetValueMetatable();

			float fValue;

			pNode = AutoDPS_GetTableChild(pAPEntryTable, "armour_piercing_value");
			fValue = pNode ? pNode->VGetValueFloat() : 0.0f;
			if(fValue > 100.0f) fValue = 100.0f;
			delete pNode;

			pNode = AutoDPS_GetTableChild(pAPEntryTable, "armour_type");
			if(pNode)
			{
				IMetaNode::IMetaTable* pArmourTypeTable = pNode->VGetValueMetatable();

				pWeapon->mapAP[strdup(pArmourTypeTable->VGetReferenceString())] = fValue;

				delete pArmourTypeTable;
				delete pNode;
			}

			delete pAPEntryTable;
			delete pChild;
		}
		delete pAPTable;
	}

	if(pWeapon->iReloadTime < 0.0001f) delete pWeapon;
	else pPackage->pWeapons.push_back(pWeapon);

	delete pGameDataTable;
}

void AutoDPS_Internal::AutoDPS_FileForEach(IDirectoryTraverser::IIterator* pFileItr, void* pTag)
{
	const char* sFileName = pFileItr->VGetName();
	const char* sExtension = strrchr(sFileName, '.');
	if(sExtension && stricmp(sExtension, ".rgd") == 0)
	{
		IFileStore::IStream* pIn = pFileItr->VOpenFile();

		CRgdFile oRgd;
		oRgd.Load(pIn);

		tAutoDPS_WeaponInfo* pWeapon = new tAutoDPS_WeaponInfo;
		pWeapon->sFileName = strdup(sFileName);

		AutoDPS_FileForEach_Rgd(&oRgd, (tAutoDPS_Package*)pTag, pWeapon);

		delete pIn;
	}
}

void AutoDPS_Internal::AutoDPS_RacebpsForEach(IDirectoryTraverser::IIterator* pFileItr, void* pTag)
{
	const char* sFileName = pFileItr->VGetName();
	const char* sExtension = strrchr(sFileName, '.');
	if(sExtension && stricmp(sExtension, ".rgd") == 0)
	{
		IFileStore::IStream* pIn = pFileItr->VOpenFile();

		CRgdFile oRgd;
		oRgd.Load(pIn);

		AutoDPS_RacebpsForEach_Rgd(&oRgd, (tAutoDPS_Package*)pTag);

		delete pIn;
	}
}

void AutoDPS_Internal::AutoDPS_RacebpsForEach_Rgd(CRgdFile* pRgd, tAutoDPS_Package* pPackage)
{
	IMetaNode::IMetaTable* pGameDataTable;
	pGameDataTable = pRgd->VGetValueMetatable();

	IMetaNode* pNode;

	pNode = AutoDPS_GetTableChild(pGameDataTable, "race_details\\playable");
	if(pNode->VGetValueBool())
	{
		delete pNode;

		pNode = AutoDPS_GetTableChild(pGameDataTable, "race_path\\unit_path");
		const char* sFolder = pNode->VGetValueString();

		char* sFull = new char[32 + strlen(sFolder)];
		sprintf(sFull, "data\\attrib\\ebps\\%s", sFolder);
		IDirectoryTraverser::IIterator* pItr = pPackage->pDirectories->VIterate(sFull);
		Rainman_ForEach(pItr, AutoDPS_EbpsForEach, (void*)pPackage, true);
		delete pItr;
		delete pNode;

		pNode = AutoDPS_GetTableChild(pGameDataTable, "race_path\\building_path");
		sFolder = pNode->VGetValueString();

		sprintf(sFull, "data\\attrib\\ebps\\%s", sFolder);
		pItr = pPackage->pDirectories->VIterate(sFull);
		Rainman_ForEach(pItr, AutoDPS_EbpsForEach, (void*)pPackage, true);
		delete pItr;
		delete []sFull;
		delete pNode;
	}
	else
	{
		delete pNode;
	}

	delete pGameDataTable;
}

void AutoDPS_Internal::AutoDPS_EbpsForEach(IDirectoryTraverser::IIterator* pFileItr, void* pTag)
{
	const char* sFileName = pFileItr->VGetName();
	const char* sExtension = strrchr(sFileName, '.');
	if(sExtension && stricmp(sExtension, ".rgd") == 0)
	{
		IFileStore::IStream* pIn = pFileItr->VOpenFile();

		CRgdFile oRgd;
		oRgd.Load(pIn);

		AutoDPS_EbpsForEach_Rgd(&oRgd, (tAutoDPS_Package*)pTag, sFileName);

		delete pIn;
	}
}

void AutoDPS_Internal::AutoDPS_EbpsForEach_Rgd(CRgdFile* pRgd, tAutoDPS_Package* pPackage, const char* sFile)
{
	IMetaNode::IMetaTable* pGameDataTable;
	pGameDataTable = pRgd->VGetValueMetatable();

	IMetaNode* pNode;

	pNode = AutoDPS_GetTableChild(pGameDataTable, "type_ext\\type_armour");
	IMetaNode::IMetaTable* pTab = pNode->VGetValueMetatable();
	const char* sType = pTab->VGetReferenceString();

	for(std::vector<tAutoDPS_AP*>::iterator itr = pPackage->pAPTypes.begin(); itr != pPackage->pAPTypes.end(); ++itr)
	{
		if( strcmp((**itr).sFilename, sType) == 0 )
		{
			char *s = strdup(sFile);
			char* sDot = strrchr(s, '.');
			*sDot = 0;
			sDot = s;
			while(*sDot)
			{
				if(*sDot == '_') *sDot = ' ';
				++sDot;
			}
			(**itr).vUnitList.push_back(s);
			break;
		}
	}

	delete pTab;
	delete pNode;
	delete pGameDataTable;
}

tAutoDPS_Package* AutoDPS::AutoDPS_Scan(IDirectoryTraverser::IIterator* pDirectory)
{
	tAutoDPS_Package* pPackage = new tAutoDPS_Package;

	Rainman_ForEach(pDirectory, AutoDPS_FileForEach, (void*)pPackage, true);

	return pPackage;
}

void AutoDPS::AutoDPS_AddUnitList(tAutoDPS_Package* pPackage, IDirectoryTraverser* pDirectories)
{
	pPackage->pDirectories = pDirectories;

	IDirectoryTraverser::IIterator* pItr = pDirectories->VIterate("data\\attrib\\racebps");

	Rainman_ForEach(pItr, AutoDPS_RacebpsForEach, (void*)pPackage, true);

	delete pItr;

	pPackage->pDirectories = 0;
}

static bool SortAPList(tAutoDPS_AP* p1, tAutoDPS_AP* p2)
{
	return (strcmp(p1->sNicename, p2->sNicename) < 0);
}

void AutoDPS::AutoDPS_Analyse(tAutoDPS_Package* pPackage)
{
	// Find all of the different armour types
	for(std::vector<tAutoDPS_WeaponInfo*>::iterator itr = pPackage->pWeapons.begin(); itr != pPackage->pWeapons.end(); ++itr)
	{
		for(std::map<char*, float>::iterator itr2 = (**itr).mapAP.begin(); itr2 != (**itr).mapAP.end(); ++itr2)
		{
			char* sAPFile = itr2->first;

			for(std::vector<tAutoDPS_AP*>::iterator itr3 = pPackage->pAPTypes.begin(); itr3 != pPackage->pAPTypes.end(); ++itr3)
			{
				if(strcmp(sAPFile, (**itr3).sFilename) == 0) goto found_ap_in_ap_list;
			}

			tAutoDPS_AP* pNewAP = new tAutoDPS_AP;
			pNewAP->sFilename = strdup(sAPFile);

			pNewAP->sNicename = strrchr(sAPFile, '\\') + 4;
			pNewAP->sNicename = strdup(pNewAP->sNicename);
			char* sD = strrchr(pNewAP->sNicename, '.');
			*sD = 0;
			sD = pNewAP->sNicename;
			while(*sD)
			{
				if(*sD == '_') *sD = ' ';
				++sD;
			}
			pPackage->pAPTypes.push_back(pNewAP);

			found_ap_in_ap_list:;
		}
	}
	std::sort(pPackage->pAPTypes.begin(), pPackage->pAPTypes.end(), SortAPList);

	// Filter out unwanted armour types
rekill:
	for(std::vector<tAutoDPS_AP*>::iterator itr = pPackage->pAPTypes.begin(); itr != pPackage->pAPTypes.end(); ++itr)
	{
		if(strcmp(((**itr).sNicename), "armour") == 0 || strcmp(((**itr).sNicename), "builder") == 0)
		{
			pPackage->pAPTypes.erase(itr);
			goto rekill;
		}
	}

	// For each AP type / weapon, calculate DPS
	for(std::vector<tAutoDPS_AP*>::iterator itr = pPackage->pAPTypes.begin(); itr != pPackage->pAPTypes.end(); ++itr)
	{
		for(std::vector<tAutoDPS_WeaponInfo*>::iterator itr2 = pPackage->pWeapons.begin(); itr2 != pPackage->pWeapons.end(); ++itr2)
		{
			float fDPS = (((**itr2).iMinDamage + (**itr2).iMaxDamage) / 2.0f) * (**itr2).iAccuracy;

			float fDPSMinimum = ((**itr2).iMinDamageValue * (**itr2).iAccuracy) / (**itr2).iReloadTime;

			for(std::map<char*, float>::iterator itr3 = (**itr2).mapAP.begin(); itr3 != (**itr2).mapAP.end(); ++itr3)
			{
				if( strcmp(itr3->first, (**itr).sFilename) == 0 )
				{
					fDPS = (fDPS * (itr3->second / 100.0f)) / (**itr2).iReloadTime;

					//(**itr2).vAnalysisOutput.push_back( std::pair<bool, float>(true, fDPS) );
					goto got_dps_from_weap;
				}
			}
			fDPS = (fDPS * ((**itr2).iDefaultAP / 100.0f)) / (**itr2).iReloadTime;

			got_dps_from_weap:

			if(fDPS > fDPSMinimum) (**itr2).vAnalysisOutput.push_back( std::pair<bool, float>(true, fDPS) );
			else (**itr2).vAnalysisOutput.push_back( std::pair<bool, float>(true, fDPSMinimum) );
		}
	}
}

static size_t fwrites(FILE* f, const char* s)
{
	return fwrite(s, 1, strlen(s), f);
}

void AutoDPS::AutoDPS_OutputHTML(AutoDPS_Internal::tAutoDPS_Package* pPackage, const char* sOutFile)
{
	FILE* f = fopen(sOutFile, "w");
	
	if(f == 0) throw new CModStudioException(0, __FILE__, __LINE__, "Unable to open file \'%s\' for writing", sOutFile);

	fwrites(f, "<html><head><title>DoW DPS</title></head><body><table border=1>\n");

	for(std::vector<tAutoDPS_AP*>::iterator itr = pPackage->pAPTypes.begin(); itr != pPackage->pAPTypes.end(); ++itr)
	{
		fprintf(f, "\t<tr>\n\t<th>%s</th> <td>", (**itr).sNicename);

		for(std::vector<char*>::iterator itr2 = (**itr).vUnitList.begin(); itr2 != (**itr).vUnitList.end(); ++itr2)
		{
			fprintf(f, "%s, ", *itr2);
		}

		fprintf(f, "</td>\n\t</tr>\n");
	}

	fwrites(f, "</table><br/><table border=1>\n");

	int n = 23;
	for(std::vector<tAutoDPS_WeaponInfo*>::iterator itr = pPackage->pWeapons.begin(); itr != pPackage->pWeapons.end(); ++itr)
	{
		if(++n == 24)
		{
			fwrites(f, "\t<tr>\n\t<th>File</th> ");
			for(std::vector<tAutoDPS_AP*>::iterator itr = pPackage->pAPTypes.begin(); itr != pPackage->pAPTypes.end(); ++itr)
			{
				fprintf(f, "<th>%s</th> ", (**itr).sNicename);
			}
			fwrites(f, "\n\t</tr>\n");
			n = 0;
		}

		const char* sBG = (n % 2 ? "bgcolor=\"#e0e0e0\"" : "bgcolor=\"#d0d0d0\"");
		fprintf(f, "\t<tr>\n\t<td %s>%s</td> ", sBG, (**itr).sFileName);

		for(std::vector< std::pair<bool, float> >::iterator itr2 = (**itr).vAnalysisOutput.begin(); itr2 != (**itr).vAnalysisOutput.end(); ++itr2)
		{
			if(itr2->first)
				fprintf(f, "<td %s>%.3f</td> ", sBG, itr2->second);
			else
				fprintf(f, "<td %s>?</td> ", sBG);
		}

		fwrites(f, "\n\t</tr>\n");
	}

	fwrites(f, "</table></body></html>");
	fclose(f);
}