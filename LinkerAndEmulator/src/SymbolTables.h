#ifndef _SYMBOL_TABLES_H_
#define _SYMBOL_TABLES_H_

#include "GlobalSymbolTable.h"
#include <string>
#include <map>


class SymbolTables {
public:
	struct Symbol {
		std::string name;
		int value, section, index;
		bool isGlobal;
		Symbol() { }
		Symbol(std::string& _name, int _value, int _section, int _index,  bool _isGlobal) : name(_name), value(_value), section(_section), index(_index), isGlobal(_isGlobal) { }
	};
	typedef std::map<std::string, Symbol> SymbolTable;
private:
	std::map<std::string, std::map<std::string, Symbol>> symbolTables;
public:
	SymbolTables();
	bool doesExist(std::string&);
	bool doesExist(std::string&, std::string&);
	void addSymbolTable(std::string&, char*, int, char*, int);
	void relocate(std::string&, std::string&, int, GlobalSymbolTable*);
	void checkForUndefinedSymbols(GlobalSymbolTable*);
	int getSymbolValue(std::string&, std::string&);
	int getSymbolValue(std::string&, int);
};

#endif