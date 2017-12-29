#include "SymbolTable.h"
#include "ErrorException.h"
#include "Utilities.h"
#include <map>
#include <cstring>

//TEST
//#include <iostream>

SymbolTable::Entry::Entry() { }

SymbolTable::Entry::Entry(std::string& _name, std::string& _section, int _value, int _index, bool _isGlobal) : name(_name), section(_section), value(_value), index(_index), isGlobal(_isGlobal) {}

SymbolTable::SymbolTable() : count(1) { 
	symbolTable = new std::map<std::string, Entry>();
	std::string und = std::string("UND");
	symbolTable->insert(std::pair<std::string, Entry>(und, Entry(und, und, 0, 0, false)));
}

void SymbolTable::addSymbol(std::string& name, std::string& section, int offset) {
	symbolTable->insert(std::pair<std::string, Entry>(name, Entry(name, section, offset, count++, false)));
}

void SymbolTable::setScope(std::string& name, bool isGlobal) {
	(*symbolTable)[name].isGlobal = isGlobal;
}

int SymbolTable::getIndex(std::string& name) {
	return (*symbolTable)[name].index;
}

int SymbolTable::getValue(std::string& name) {
	return (*symbolTable)[name].value;
}

bool SymbolTable::doesExist(std::string& name) {
	return symbolTable->find(name) != symbolTable->end();
}

bool SymbolTable::isGlobal(std::string& name) {
	return (*symbolTable)[name].isGlobal;
}

std::string SymbolTable::getSection(std::string& name) {
	if ((*symbolTable)[name].section == "UND") {
		return name;
	}
	else {
		return (*symbolTable)[name].section;
	}
}

int SymbolTable::getSymbolTableSize() {
	return symbolTable->size() * sizeof(FileEntry);
}

char* SymbolTable::getSymbolTableContent() {
	char *buffer = new char[this->getSymbolTableSize()];
	
	int stringOffset = 0;
	
	for (std::map<std::string, Entry>::iterator i = symbolTable->begin(); i != symbolTable->end(); ++i) {
		char *position = buffer + i->second.index * sizeof(FileEntry);
		utilities::pack(position, stringOffset, sizeof(int));
		stringOffset += i->second.name.size() + 1;
		utilities::pack(position + sizeof(int), i->second.value, sizeof(int));
		utilities::pack(position + 2 * sizeof(int), this->getIndex(i->second.section), sizeof(int));
		utilities::pack(position + 3 * sizeof(int), i->second.isGlobal ? 1 : 0, sizeof(int));
	}

	return buffer;
}

int SymbolTable::getStringTableSize() {
	int size = 0;
	for (std::map<std::string, Entry>::iterator i = symbolTable->begin(); i != symbolTable->end(); ++i) {
		size += i->second.name.size() + 1;
	}
	return size;
}

char* SymbolTable::getStringTableContent() {
	char *buffer = new char[this->getStringTableSize()];
	char *position = buffer;
	
	for (std::map<std::string, Entry>::iterator i = symbolTable->begin(); i != symbolTable->end(); ++i) {
		strcpy(position, i->second.name.c_str());
		position += i->second.name.size() + 1;
	}

	return buffer;
}

SymbolTable::~SymbolTable() {
	/*for (std::map<std::string, Entry>::iterator i = symbolTable->begin(); i != symbolTable->end(); ++i) {
		std::cout << i->second.name << '\t' << i->second.section << '\t' << i->second.value << '\t' << i->second.isGlobal << '\t' << i->second.index << std::endl;
	}*/

	delete symbolTable;
}