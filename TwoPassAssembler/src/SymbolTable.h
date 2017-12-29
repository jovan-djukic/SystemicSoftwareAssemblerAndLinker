#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <string>
#include <map>

class SymbolTable {
private:
	struct Entry {
		std::string name, section;
		int value, index;
		bool isGlobal;

		Entry(std::string&, std::string&, int, int, bool);
		Entry();
	};
	struct FileEntry {
		int name, value, index, info;
	};
	std::map<std::string, Entry> *symbolTable;
	int count;
public:
	SymbolTable();
	void addSymbol(std::string&, std::string&, int);
	void setScope(std::string&, bool);
	int getValue(std::string&);
	bool doesExist(std::string&);
	bool isGlobal(std::string&);
	int getIndex(std::string&);
	std::string getSection(std::string&);
	int getSymbolTableSize();
	char* getSymbolTableContent();
	int getStringTableSize();
	char* getStringTableContent();
	~SymbolTable();
};

#endif