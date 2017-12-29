#ifndef _TABLES_H_
#define _TABLES_H_

#include "SymbolTable.h"
#include "ContentTable.h"
#include <string>
#include <map>

class Tables {
private:
	int locationCounter;
	std::string currentSection;
	SymbolTable *symbolTable;
	std::map<std::string, ContentTable*> *sectionTables;
public:
	Tables();
	void setCurrentSection(std::string&);
	std::string getCurrentSection() const;
	void addSectionTable();
	ContentTable* getCurrentSectionTable();
	SymbolTable* getSymbolTable();
	int getLocationCounter() const;
	void incrementLocationCounter(int);
	ContentTable *getRelocationTable(std::string&);
	void writeIntoFile(std::string&);
	~Tables();
};


#endif