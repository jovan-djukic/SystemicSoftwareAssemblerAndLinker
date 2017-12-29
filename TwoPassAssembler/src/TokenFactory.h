#ifndef _TOKEN_FACTORY_H_
#define _TOKEN_FACTORY_H_

#include "Token.h"
#include <fstream>
#include <string>

class TokenFactory {
protected:
	std::ifstream input;
	std::string line;
	int position;
protected:
	int lineNumber;
	bool end;

	virtual Token* newLabel(std::string&) = 0;
	virtual Token* newInstruction(std::string&) = 0;
	virtual Token* newDirective(std::string&) = 0;
public:
	TokenFactory(std::string&);
	Token* nextToken();
	virtual ~TokenFactory();
};
 
class FirstPassTokenFactory : public TokenFactory {
private:
	std::string section;
protected:
	Token* newLabel(std::string&);
	Token* newInstruction(std::string&);
	Token* newDirective(std::string&);
public:
	FirstPassTokenFactory(std::string&);
};

class SecondPassTokenFactory : public TokenFactory {
protected:
	Token* newLabel(std::string&);
	Token* newInstruction(std::string&);
	Token* newDirective(std::string&);
public:
	SecondPassTokenFactory(std::string&);
};

#endif