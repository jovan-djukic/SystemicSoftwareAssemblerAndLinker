#include "Linker.h"
#include "SectionList.h"
#include "SymbolTables.h"
#include "GlobalSymbolTable.h"
#include "ErrorException.h"
#include "Utilities.h"
#include "ScriptParser.h"
#include "Memory.h"
#include <fstream>
#include <cstring>
#include <cmath>

void linker::loadFromFile(const char *fileName, SectionList *sectionList, SectionList *relocationSectionList, SymbolTables *symbolTables) {
	FileHeader fileHeader;
	std::ifstream input(fileName, std::ios::binary | std::ios::in);

	if (!input.is_open()) {
		throw ErrorException(std::string("File ").append(fileName).append(" doesn't exist"));
	}

	input.read(reinterpret_cast<char*>(&fileHeader), sizeof(FileHeader));
	//READ SECTION TABLE
	SectioTableEntry *entries = new SectioTableEntry[fileHeader.numOfSections];
	input.seekg(fileHeader.sectionTableOffset);
	for (int i = 0; i < fileHeader.numOfSections; i++) {
		input.read(reinterpret_cast<char*>(entries + i), sizeof(SectioTableEntry));
	}
	//READ SECTION NAMES
	char *buffer = new char[entries[fileHeader.stringTableEntry].size];
	input.seekg(entries[fileHeader.stringTableEntry].offset);
	input.read(buffer, entries[fileHeader.stringTableEntry].size);
	utilities::CharInputStream sectionNamesStream(buffer, entries[fileHeader.stringTableEntry].size);
	//INITIATE STRUCTURES
	for (int i = 0; i < fileHeader.numOfSections; i++) {
		if (entries[i].type == TYPE::SYMTAB) {
			char *symbolTableBytes = new char[entries[i].size];
			char *symbolNamesBytes = new char[entries[i + 1].size];

			input.seekg(entries[i].offset);
			input.read(symbolTableBytes, entries[i].size);
			input.read(symbolNamesBytes, entries[i + 1].size);

			std::string file = std::string(fileName);
			symbolTables->addSymbolTable(file, symbolTableBytes, entries[i].size, symbolNamesBytes, entries[i + 1].size);

			delete[] symbolTableBytes;
			delete[] symbolNamesBytes;
		}
		else if (entries[i].type == TYPE::REL) {
			std::string file = std::string(fileName);
			std::string sectionName = sectionNamesStream.readString(entries[i].name);
			relocationSectionList->addSection(file, sectionName, entries[i].offset, entries[i].size);
		}
		else if (entries[i].type != TYPE::STRTAB) {
			std::string file = std::string(fileName);
			std::string sectionName = sectionNamesStream.readString(entries[i].name);
			sectionList->addSection(file, sectionName, entries[i].offset, entries[i].size);
		}
	}

	delete[] buffer;
	delete[] entries;
}

void linker::scriptResolver(const char *scriptName, SectionList *sectionList, SymbolTables *symbolTables, GlobalSymbolTable *globalSymbolTable, int& currentPosition) {
	ScriptParser scriptParser(scriptName);
	while (scriptParser.hasNextToken()) {
		Token *token = scriptParser.getNextToken();
		try {
			token->resolve(symbolTables, globalSymbolTable, sectionList, currentPosition);
		}
		catch (ErrorException& e) {
			delete token;
			throw e;
		}
		delete token;
	}
}

void linker::basicResolver(SectionList *sectionList, SymbolTables *symbolTables, GlobalSymbolTable *globalSymbolTable, int& currentPosition) {
	while (sectionList->hasNext()) {
		SectionList::Entry entry;
		entry = sectionList->removeNext();
		symbolTables->relocate(entry.file, entry.name, currentPosition, globalSymbolTable);
		currentPosition += entry.size;
	}
}

char ****linker::loader(SectionList *sectionList, SymbolTables *symbolTables, const char** fileNames, int numOfFiles) {
	std::map<std::string, std::ifstream> files;
	for (int i = 0; i < numOfFiles; i++) {
		files.insert(std::pair<std::string, std::ifstream>(std::string(fileNames[i]), std::ifstream(fileNames[i], std::ios::binary | std::ios::in)));
	}

	char ****memory = nullptr;

	while (sectionList->hasNext()) {
		SectionList::Entry entry;
		entry = sectionList->removeNext();
		char *buffer = new char[entry.size];
		//CHECK IF BSS
		if (entry.name.substr(1, 3) == "bss") {
			memset(buffer, 0, entry.size);
		}
		else {
			//std::ifstream file(entry.file, std::ios::in | std::ios::binary);
			files[entry.file].seekg(entry.offset);
			files[entry.file].read(buffer, entry.size);
			//file.seekg(entry.offset);
			//file.read(buffer, entry.size);
		}
		//GET ADRESS
		int address = symbolTables->getSymbolValue(entry.file, entry.name);
		//WRITE TO MEMORY
		write(memory, address, buffer, entry.size);
		//DELETE BUFFER
		delete[] buffer;
	}

	return memory;
}

void linker::relocator(SectionList *relocationSectionList, SymbolTables *symbolTables, char ****memory, const char **fileNames, int numOfFiles) {
	std::map<std::string, std::ifstream> files;
	for (int i = 0; i < numOfFiles; i++) {
		std::string file = std::string(fileNames[i]);
		files.insert(std::pair<std::string, std::ifstream>(file, std::ifstream(file, std::ios::binary | std::ios::in)));
	}

	while (relocationSectionList->hasNext()) {
		SectionList::Entry entry;
		entry = relocationSectionList->removeNext();
		char *buffer = new char[entry.size];

		files[entry.file].seekg(entry.offset);
		files[entry.file].read(buffer, entry.size);
		std::string section = entry.name.substr(4);
		int value = symbolTables->getSymbolValue(entry.file, section);

		for (int i = 0; i < entry.size; i += 8) {
			int address = utilities::unpack(buffer + i, sizeof(int)) + value;
			int index = utilities::unpack(buffer + i + sizeof(int), 3);
			int operation = utilities::unpack(buffer + i + sizeof(int) + 3, 1);
			int symbolValue = symbolTables->getSymbolValue(entry.file, index);
			//READ DATA FROM MEMORY
			unsigned long long data = read(memory, address);
			//APPLY RELOCATION
			if (operation == 0) {
				data += symbolValue;
			}
			else {
				data -= symbolValue;
			}
			//CHECK FOR OVERFLOW
			if (data >= pow(2, 32)) {
				deleteMemory(memory);
				throw ErrorException(std::string("Overflow on address: ").append(std::to_string(address)));
			}
			//WRITE DATA BACK
			char buffer[sizeof(int)];
			utilities::pack(buffer, data, sizeof(int));
			write(memory, address, buffer, sizeof(int));
		}

		delete[] buffer;
	}
}

char ****linker::link(int numOfParameters, const char **parameters) {
	int index = 1, fileStart = 1;
	bool hasScriptFile = false;
	int numOfFiles = numOfParameters - 1;
	if (strcmp(parameters[1], "-T") == 0 || strcmp(parameters[1], "-t") == 0) {
		hasScriptFile = true;
		index += 2;
		fileStart += 2;
		numOfFiles -= 2;
	}
	//ALLOCATE NEEDed STRUCTURES
	SectionList sectionList, backUpSectionList, relocationSectionList;
	GlobalSymbolTable globalSymbolTable;
	SymbolTables symbolTables;
	int currentPosition = 0;
	//LOAD NEEDED DATA
	for (int i = 0; i < numOfFiles; i++, index++) {
		linker::loadFromFile(parameters[index], &sectionList, &relocationSectionList, &symbolTables);
	}
	backUpSectionList = sectionList;
	//PROCESS SCRIPT FILE
	if (hasScriptFile) {
		linker::scriptResolver(parameters[2], &sectionList, &symbolTables, &globalSymbolTable, currentPosition);
	}
	//PROCESS THE REST
	linker::basicResolver(&sectionList, &symbolTables, &globalSymbolTable, currentPosition);
	//CHEACK FOR UNDEFINED SYMBOLS
	symbolTables.checkForUndefinedSymbols(&globalSymbolTable);
	//LOAD DATA
	char ****memory = loader(&backUpSectionList, &symbolTables, parameters + fileStart, numOfFiles);
	//APPLY RELOCATION ENTRIES
	relocator(&relocationSectionList, &symbolTables, memory, parameters + fileStart, numOfFiles);
	
	return memory;
}