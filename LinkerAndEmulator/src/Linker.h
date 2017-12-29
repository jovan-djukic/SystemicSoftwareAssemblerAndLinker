#ifndef _LINKER_H_
#define _LINKER_H_

#include "SectionList.h"
#include "GlobalSymbolTable.h"
#include "SymbolTables.h"
#include "Memory.h"
#include <string>

namespace linker {
	struct FileHeader {
		int numOfSections;
		int sectionTableOffset;
		int stringTableEntry;
	};

	struct SectioTableEntry {
		int name;
		int type;
		int offset;
		int size;
		int info;
		int link;
	};

	enum TYPE { TEXT, DATA, BSS, REL, SYMTAB, STRTAB };

	void loadFromFile(const char*, SectionList*, SectionList*, SymbolTables*);
	void scriptResolver(const char*, SectionList*, SymbolTables*, GlobalSymbolTable*, int&);
	void basicResolver(SectionList*, SymbolTables*, GlobalSymbolTable*, int&);
	char ****loader(SectionList*, SymbolTables*, const char**, int);
	void relocator(SectionList*, SymbolTables*, char****, const char**, int);
	char ****link(int, const char**);
}

#endif