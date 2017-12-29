#ifndef _GLOBAL_SYMBOL_TABLE_H_
#define _GLOBAL_SYMBOL_TABLE_H_

#include <map>
#include <string>

class GlobalSymbolTable {
private:
	std::map<std::string, int> globalSymbolTable;
public:
	GlobalSymbolTable();
	void addSymbol(std::string&, int);
	bool doesExist(std::string&);
	int getValue(std::string&);
};


#endif