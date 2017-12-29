#ifndef _SCRIPT_PARSER_H_
#define _SCRIPT_PARSER_H_

#include <string>
#include <fstream>
#include "GlobalSymbolTable.h"
#include "SymbolTables.h"
#include "SectionList.h"
#include "Linker.h"

class Token {
protected:
	std::string line;
	int lineNumber;

	int exppressionEvaluation(std::string&, GlobalSymbolTable*, int&);
	int align(std::string&, int&);

public:
	Token(std::string, int);
	virtual void resolve(SymbolTables*, GlobalSymbolTable*, SectionList*, int&);
};

class EqualityToken : public Token {
public:
	EqualityToken(std::string, int);
	void resolve(SymbolTables*, GlobalSymbolTable*, SectionList*, int&);
};

class PlacementToken : public Token {
public:
	PlacementToken(std::string, int);
	void resolve(SymbolTables*, GlobalSymbolTable*, SectionList*, int&);
};

class ScriptParser {
private:
	std::ifstream script;
	int lineNumber;
public:
	ScriptParser(std::string);
	bool hasNextToken();
	Token* getNextToken();
	~ScriptParser();
};

#endif