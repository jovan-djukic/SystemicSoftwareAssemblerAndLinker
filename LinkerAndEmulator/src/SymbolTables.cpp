#include "SymbolTables.h"
#include "Utilities.h"
#include "GlobalSymbolTable.h"
#include "ErrorException.h"
#include <iostream>

SymbolTables::SymbolTables() { }

bool SymbolTables::doesExist(std::string& file) {
	return symbolTables.find(file) != symbolTables.end();
}

bool SymbolTables::doesExist(std::string& file, std::string& symbol) {
	return symbolTables.find(file) != symbolTables.end() && symbolTables[file].find(symbol) != symbolTables[file].end();
}

void SymbolTables::addSymbolTable(std::string& file, char *symbolTableBytes, int numOfSymbolTableBytes, char *symbolNamesBytes, int numOfSymbolNameBytes) {
	
	struct FileEntry {
		int name, value, section, info;
	};

	FileEntry entry;
	utilities::CharInputStream symbolTableStream(symbolTableBytes, numOfSymbolTableBytes);
	utilities::CharInputStream symbolNamesStream(symbolNamesBytes, numOfSymbolNameBytes);
	std::map<std::string, Symbol> symbolTable;

	int index = 0;

	while (!symbolTableStream.end()) {
		symbolTableStream.read(reinterpret_cast<char*>(&entry), sizeof(FileEntry));
		std::string name = symbolNamesStream.readString(entry.name);
		Symbol symbol(name, entry.value, entry.section, index++, entry.info);
		symbolTable.insert(std::pair<std::string, Symbol>(name, symbol));
	}

	symbolTables.insert(std::pair<std::string, SymbolTable>(file, symbolTable));
}

void SymbolTables::relocate(std::string& file, std::string& section, int value, GlobalSymbolTable *globalSymbolTable) {
	for (std::map<std::string, Symbol>::iterator symbol = symbolTables[file].begin(); symbol != symbolTables[file].end(); ++symbol) {
		if (symbol->second.section == symbolTables[file][section].section) {
			symbol->second.value += value;
			if (symbol->second.isGlobal) {
				if (globalSymbolTable->doesExist(symbol->second.name)) {
					throw ErrorException(std::string("Global symbol redifinition, symbol: ").append(symbol->second.name));
				}
				else {
					globalSymbolTable->addSymbol(symbol->second.name, symbol->second.value);
				}
			}
		}
	}
}

void SymbolTables::checkForUndefinedSymbols(GlobalSymbolTable *globalSymbolTable) {
	for (std::map<std::string, SymbolTable>::iterator table = symbolTables.begin(); table != symbolTables.end(); ++table) {
		for (std::map<std::string, Symbol>::iterator symbol = table->second.begin(); symbol != table->second.end(); ++symbol) {
			if (symbol->second.section == 0 && symbol->second.name != "UND") {
				if (!globalSymbolTable->doesExist(symbol->second.name)) {
					throw ErrorException(std::string("Undefined symbol ").append(symbol->second.name));
				}
				else {
					symbol->second.value = globalSymbolTable->getValue(symbol->second.name);
				}
			}
		}
	}
}

int SymbolTables::getSymbolValue(std::string& file, std::string& symbol) {
	return symbolTables[file][symbol].value;
}

int SymbolTables::getSymbolValue(std::string& file, int index) {
	for (std::map<std::string, Symbol>::iterator symbol = symbolTables[file].begin(); symbol != symbolTables[file].end(); ++symbol) {
		if (symbol->second.index == index) {
			return symbol->second.value;
		}
	}
	return 0;
}