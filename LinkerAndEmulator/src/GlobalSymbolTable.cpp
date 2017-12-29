#include "GlobalSymbolTable.h"

GlobalSymbolTable::GlobalSymbolTable() { }

void GlobalSymbolTable::addSymbol(std::string& name, int value) {
	globalSymbolTable.insert(std::pair<std::string, int>(name, value));
}

bool GlobalSymbolTable::doesExist(std::string& name) {
	return globalSymbolTable.find(name) != globalSymbolTable.end();
} 

int GlobalSymbolTable::getValue(std::string& name) {
	return globalSymbolTable[name];
}