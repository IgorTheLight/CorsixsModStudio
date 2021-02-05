#include "CInheritTable.h"
#include "..\zLib/zlib.h"
#include <algorithm>

extern "C" {
typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;
ub4 hash(ub1 * k,ub4 length,ub4 initval);
ub4 hash3(ub1 * k,ub4 length,ub4 initval);
}

CInheritTable::CNode::CNode()
{
	m_pParent = 0;
	m_pHashMapNext = 0;
	m_sFullName = 0;
	m_sMiniName = 0;
	m_bIsNil = false;
}

CInheritTable::CNode::~CNode()
{
	delete m_pHashMapNext;
	if(m_sFullName) free(m_sFullName);
	if(m_sMiniName) free(m_sMiniName);
	for(std::vector<CNode*>::iterator itr = m_vChildren.begin(); itr != m_vChildren.end(); ++itr)
	{
		delete *itr;
	}
}

void CInheritTable::CNode::setParent(CInheritTable::CNode* pParent)
{
	// Remove from old parent's child list
	if(m_pParent)
	{
		std::vector<CNode*>::iterator itr = std::find(m_pParent->m_vChildren.begin(), m_pParent->m_vChildren.end(), (CNode*)this);
		if(itr != m_pParent->m_vChildren.end())
		{
			m_pParent->m_vChildren.erase(itr);
		}
	}

	// Update parent
	m_pParent = pParent;

	// Add to new parent's child list
	if(m_pParent)
	{
		m_pParent->m_vChildren.push_back((CNode*)this);
	}
}

void CInheritTable::CNode::print(FILE* f, int iL)
{
	for(int i = 0; i < iL; ++i) fputc(' ', f);
	fprintf(f, "%s (%s)\n", m_sMiniName, m_sFullName);
	for(std::vector<CNode*>::iterator itr = m_vChildren.begin(); itr != m_vChildren.end(); ++itr)
	{
		(**itr).print(f, iL + 1);
	}
}

void CInheritTable::CNode::setName(const char* sPath)
{
	// Update full name
	if(m_sFullName) free(m_sFullName);
	m_sFullName = strdup(sPath);

	// Update mini name
	char* sTmp = strrchr(m_sFullName, '\\');
	if(m_sMiniName) free(m_sMiniName);
	m_sMiniName = strdup(sTmp ? (sTmp+1) : "");

	// Update nil flag
	sTmp = strrchr(m_sMiniName, '.');
	m_bIsNil = false;
	if(sTmp)
	{
		m_bIsNil = (stricmp(sTmp, ".nil") == 0);
		*sTmp = 0;
	}
}

CInheritTable::CNode* CInheritTable::CNode::getParent() const
{
	return m_pParent;
}

const char* CInheritTable::CNode::getFullName() const
{
	return m_sFullName;
}

const char* CInheritTable::CNode::getMiniName() const
{
	return m_sMiniName;
}

size_t CInheritTable::CNode::getChildCount() const
{
	return m_vChildren.size();
}

CInheritTable::CNode* CInheritTable::CNode::getChild(size_t i) const
{
	return m_vChildren[i];
}

bool CInheritTable::CNode::getIsNil() const
{
	return m_bIsNil;
}

CInheritTable::CInheritTable()
{
	m_pRootNode = new CNode;
	m_pRootNode->setName("");
	m_mapNodes[crc32_case_idt(0, (const Bytef*)"", 0)] = m_pRootNode;
}

CInheritTable::~CInheritTable()
{
	delete m_pRootNode;
}

CInheritTable::CNode* CInheritTable::getRoot()
{
	return m_pRootNode;
}

CInheritTable::CNode* CInheritTable::findOrMake(const char* sPath)
{
	unsigned long iHash = crc32_case_idt(0, (const Bytef *)sPath, (uInt)strlen(sPath));
	CNode* pNode = m_mapNodes[iHash], *pNode2;
	pNode2 = pNode;

	// Attempt find
	while(pNode2)
	{
		if(stricmp(sPath, pNode2->m_sFullName) == 0) return pNode2;
		pNode2 = pNode2->m_pHashMapNext;
	}

	// Make
	pNode2 = new CNode;
	pNode2->setName(sPath);
	//pNode2->setParent(m_pRootNode);
	pNode2->m_pHashMapNext = pNode;
	m_mapNodes[iHash] = pNode2;
	return pNode2;
}

void CInheritTable::assignOrphansTo(CNode* pNode)
{
	CNode* pNode2;
	for(std::map<unsigned long, CNode*>::iterator itr = m_mapNodes.begin(); itr != m_mapNodes.end(); ++itr)
	{
		if(itr->first != 0)
		{
			pNode2 = itr->second;
			while(pNode2)
			{
				if(pNode2->getParent() == 0) pNode2->setParent(pNode);
				pNode2 = pNode2->m_pHashMapNext;
			}
		}
	}
}