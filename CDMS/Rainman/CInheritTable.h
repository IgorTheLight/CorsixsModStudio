#ifndef _CINHERITANCE_TABLE_H_
#define _CINHERITANCE_TABLE_H_

#include <vector>
#include <map>
#include "IFileStore.h"

class RAINMAN_API CInheritTable
{
public:
	class RAINMAN_API CNode
	{
	protected:
		friend class CInheritTable;
		CNode();
		~CNode();

		CNode* m_pParent;
		CNode* m_pHashMapNext;
		char* m_sFullName;
		char* m_sMiniName;
		bool m_bIsNil;
		std::vector<CNode*> m_vChildren;

		void setName(const char* sPath);
	public:
		void print(FILE* f, int iL = 0);
		void setParent(CNode* pParent);

		CNode* getParent() const;
		const char* getFullName() const;
		const char* getMiniName() const;
		size_t getChildCount() const;
		CNode* getChild(size_t i) const;
		bool getIsNil() const;
	};
protected:
	std::map<unsigned long, CNode*> m_mapNodes;
	CNode* m_pRootNode;

public:
	CInheritTable();
	~CInheritTable();
	CNode* findOrMake(const char* sPath);
	CNode* getRoot();
	void assignOrphansTo(CNode* pNode);

	/*
	void save(IFileStore::IOutputStream* pStream);
	void load(IFileStore::IStream* pStream);
	*/
};

#endif