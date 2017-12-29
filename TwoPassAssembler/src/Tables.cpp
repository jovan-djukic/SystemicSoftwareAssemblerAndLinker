#include "Tables.h"
#include "ContentTable.h"
#include "ErrorException.h"
#include <string>
#include <cstring>
#include <map>
#include <iterator>
#include <fstream>

Tables::Tables() : locationCounter(0), currentSection("") {
	symbolTable = new SymbolTable();
	sectionTables = new std::map<std::string, ContentTable*>();
}

SymbolTable* Tables::getSymbolTable() {
	return symbolTable;
}

void Tables::setCurrentSection(std::string& section) {
	currentSection = section;
	locationCounter = 0;
}

void Tables::addSectionTable() {
	if (currentSection != "") {
		if (currentSection.substr(0, 3) != "bss") {
			sectionTables->insert(std::pair<std::string, ContentTable*>(currentSection, new StaticContentTable(currentSection, locationCounter)));
		}
		else {
			sectionTables->insert(std::pair<std::string, ContentTable*>(currentSection, new ContentTable(currentSection, locationCounter)));
		}
	}
}

std::string Tables::getCurrentSection() const {
	return currentSection;
}

ContentTable* Tables::getCurrentSectionTable() {
	return (*sectionTables)[currentSection];
}

int Tables::getLocationCounter() const {
	return locationCounter;
}

void Tables::incrementLocationCounter(int size) {
	locationCounter += size;
}

ContentTable* Tables::getRelocationTable(std::string& name) {
	if (sectionTables->find(name) == sectionTables->end()) {
		sectionTables->insert(std::pair<std::string, ContentTable*>(name, new DynamicContentTable(name, 0)));
	}
	return (*sectionTables)[name];
}

void Tables::writeIntoFile(std::string& fileName) {
	enum TYPE {TEXT, DATA, BSS, REL, SYMTAB, STRTAB};
	struct FileHeader {
		int numOfSections;
		int sectionTableOffset;
		int stringTableEntry;
	};
	FileHeader fileHeader = { 0, 0, 0};
	struct SectioTableEntry {
		int name;
		int type;
		int offset;
		int size;
		int info;
		int link;
	};

	std::ofstream output(fileName, std::ios::out | std::ios::binary);
	if (!output.is_open()) {
		std::string errorMessage = std::string("Output file couldn't be opened");
		throw ErrorException(errorMessage);
	}
	output.seekp(sizeof(FileHeader));
	char *content = nullptr;
	//GET STARTING INFO
	fileHeader.numOfSections = sectionTables->size() + 3;//symbolTable, symbolNameTable, sectionNametable
	fileHeader.stringTableEntry = fileHeader.numOfSections - 1;
	fileHeader.sectionTableOffset = 0;
	SectioTableEntry *entries = new SectioTableEntry[fileHeader.numOfSections];
	int offset = sizeof(FileHeader);
	std::string shstrtabName = std::string(".shstrtab");
	DynamicContentTable shstrtab(shstrtabName, 0);

	//FIRST GOES SYMBOL TABLE
	entries[0].name = shstrtab.getSize();
	shstrtab.addContent(".symtab", strlen(".symtab") + 1);
	entries[0].type = SYMTAB;
	entries[0].offset = offset;
	entries[0].size = symbolTable->getSymbolTableSize();
	offset += entries[0].size;
	entries[0].info = entries[0].link = 0;
	//WRITE SYMBOL TABLE
	content = symbolTable->getSymbolTableContent();
	output.write(content, entries[0].size);
	delete[] content;

	//SECOND GOES SYMBOL TABLE STRING TABLE
	entries[1].name = shstrtab.getSize();
	shstrtab.addContent(".strtab", strlen(".strtab") + 1);
	entries[1].type = STRTAB;
	entries[1].offset = offset;
	entries[1].size = symbolTable->getStringTableSize();
	offset += entries[1].size;
	entries[1].info = entries[1].link = 0;
	//WRITE SYMBOL STRING TALE
	content = symbolTable->getStringTableContent();
	output.write(content, entries[1].size);
	delete[] content;

	//NOW GO THE SECTIONS
	int currentEntrie = 2;
	for (std::map<std::string, ContentTable*>::iterator i = sectionTables->begin(); i != sectionTables->end(); ++i) {
		entries[currentEntrie].name = shstrtab.getSize();
		shstrtab.addContent(i->first.data(), strlen(i->first.data()) + 1);
		
		if (i->first.substr(0, 5) == ".text") {
			entries[currentEntrie].type = TEXT;
		}
		else if (i->first.substr(0, 5) == ".data") {
			entries[currentEntrie].type = DATA;
		}
		else if (i->first.substr(0, 4) == ".bss") {
			entries[currentEntrie].type = BSS;
		}
		else {
			entries[currentEntrie].type = REL;
		}
		
		entries[currentEntrie].offset = offset;
		entries[currentEntrie].size = i->second->getSize();
		//IF BSS NOT IN FILE SO NO INCREMENTING OFFSET
		if (entries[currentEntrie].type != BSS) {
			offset += entries[currentEntrie].size;
		}

		entries[currentEntrie].info = entries[currentEntrie].link = 0;
		//IF REL TYPE THEN NEED TO WRITE ADITIONAL INFO
		if (entries[currentEntrie].type == REL) {
			entries[currentEntrie].link = 0;
			std::string section = i->first.substr(4);
			entries[currentEntrie].info = symbolTable->getIndex(section);
		}
		//IF BSS NO WRITINIG IN OUTPUT FILE
		if (entries[currentEntrie].type != BSS) {
			output.write(i->second->getContent(), entries[currentEntrie].size);
		}

		//INCREMENT CURRENT ENTRIE
		currentEntrie++;
	}

	//
	//STRTAB LAST
	entries[currentEntrie].name = shstrtab.getSize();
	shstrtab.addContent(".shstrtab", strlen(".shstrtab") + 1);
	entries[currentEntrie].type = STRTAB;
	entries[currentEntrie].offset = offset;
	entries[currentEntrie].size = shstrtab.getSize();
	offset += entries[currentEntrie].size;
	entries[currentEntrie].info = entries[currentEntrie].link = 0;
	//WRITE SHSTRTAB
	output.write(shstrtab.getContent(), entries[currentEntrie].size);

	//WRITE OFFSET IN HEADER
	fileHeader.sectionTableOffset = offset;

	//WRITE SECTION TABLE
	for (int i = 0; i < fileHeader.numOfSections; i++) {
		output.write(reinterpret_cast<char*>(entries + i), sizeof(SectioTableEntry));
	}

	//WRITE FILE HEADER
	output.seekp(0);
	output.write(reinterpret_cast<char*>(&fileHeader), sizeof(FileHeader));

	delete[] entries;
	output.close();
}

Tables::~Tables() {
	delete symbolTable;
	for (std::map<std::string, ContentTable*>::iterator i = sectionTables->begin(); i != sectionTables->end(); ++i) {
		delete i->second;
	}
	delete sectionTables;
}


