#include "Directive.h"
#include "Tables.h"
#include "ErrorException.h"
#include "Utilities.h"
#include <string>
#include <cstring>
#include <map>
#include <cmath>

Directive::Directive(int _line) : Token(_line) { }

Directive::~Directive() {

}

Section::Section(std::string& _section, int _line) : Directive(_line), section(_section) { }

FirstPassSection::FirstPassSection(std::string& _section, int _line) : Section(_section, _line) { }

void FirstPassSection::resolve(Tables *tables) {
	int pos = 4;
	if (section.substr(0, 3) == "bss") {
		pos = 3;
	}
	if (section[pos] != '.' || section.find_first_of(INVALID_CHARACTERS, pos + 1) != std::string::npos) {
		throw ErrorException(std::string("Invalid section name, line: ").append(std::to_string(line)));
	}
	if (tables->getSymbolTable()->doesExist(section)) {
		throw ErrorException(std::string("Section ").append(section).append(" already exists, line: ").append(std::to_string(line)));
	}
	tables->addSectionTable();
	tables->getSymbolTable()->addSymbol(std::string(".").append(section), std::string(".").append(section), 0);
	tables->setCurrentSection(std::string(".").append(section));
}

SecondPassSection::SecondPassSection(std::string& _section, int _line) : Section(_section, _line) { }

void SecondPassSection::resolve(Tables *tables) {
	tables->setCurrentSection(std::string(".").append(section));
}

MemoryConsuming::MemoryConsuming(std::string& _directive, int _line) : Directive(_line), directive(_directive) { }

void MemoryConsuming::resolve(Tables *tables) {
	if (directive[4] != ' ') {
		throw ErrorException(std::string("Unknown directive, line: ").append(std::to_string(line)));
	}
	int size = 0;
	if (directive.substr(0, 4) == "char") {
		size = 1;
	}
	else if (directive.substr(0, 4) == "word") {
		size = 2;
	}
	else {
		size = 4;
	}
	std::string intitializers = directive.substr(5);
	utilities::StringTokenizer tokenizer(intitializers, ",");
	int numOfInitializers = 0;
	while (tokenizer.hasNextToken()) {
		numOfInitializers++;
		tokenizer.nextToken();
	}

	tables->incrementLocationCounter(numOfInitializers * size);
}

Skip::Skip(std::string& _directive, int _line) : Directive(_line), directive(_directive) { }

Skip::~Skip() { }

FirstPassSkip::FirstPassSkip(std::string& _directive, int _line) : Skip(_directive, _line) { }

void FirstPassSkip::resolve(Tables *tables) {
	if (directive[4] != ' ') {
		throw ErrorException(std::string("Unknown directive, line: ").append(std::to_string(line)));
	}
	std::string number = directive.substr(5);
	if (!utilities::isNumber(number)) {
		throw ErrorException(std::string("SKIP directive, parameter not a number, line: ").append(std::to_string(line)));
	}
	long long offset = std::stoll(number.c_str(), nullptr, 0);
	if (offset <= 0 || offset >= pow(2, 32)) {
		throw ErrorException(std::string("SKIP directive, invalid offset, line: ").append(std::to_string(line)));
	}
	tables->incrementLocationCounter((int)offset);
}

SecondPassSkip::SecondPassSkip(std::string& _directive, int _line) : Skip(_directive, _line) { }

void SecondPassSkip::resolve(Tables *tables) {
	int size = strtol(directive.substr(5).c_str(), nullptr, 0);
	char *buffer = new char[size];
	memset(buffer, 0, size);
	tables->getCurrentSectionTable()->addContent(buffer, size);
	delete[] buffer;
}

MemoryAllocation::MemoryAllocation(std::string& _directive, int _line) : Directive(_line) {
	if (_directive.substr(0, 4) == "char") {
		size = 1;
	}
	else if (_directive.substr(0, 4) == "word") {
		size = 2;
	}
	else {
		size = 4;
	}
	initializators = _directive.substr(5);
}

void MemoryAllocation::resolve(Tables *tables) {
	utilities::StringTokenizer tokenizer(initializators, ",");
	int numOfInitializers = 0;
	while (tokenizer.hasNextToken()) {
		std::string token = tokenizer.nextToken();
		token = utilities::removeBlankSpacesBeforeAndAfter(token);
		if (token == "") {
			throw ErrorException(std::string("Memory allocation directive, invalid initializer expression number ").append(std::to_string(numOfInitializers + 1)).append(" , line:").append(std::to_string(line)));
		}
		int classificationIndex = Token::getClassificationIndex(tables, token);
		if (!utilities::isNumber(token) && size == 4 && classificationIndex != 0 && classificationIndex != 1) {
			throw ErrorException(std::string("Memory allocation directive, invalid initializer expression number ").append(std::to_string(numOfInitializers + 1)).append(" , line:").append(std::to_string(line)));
		}
		numOfInitializers++;
	}

	tokenizer = utilities::StringTokenizer(initializators, ",");
	int num = 0;

	while (tokenizer.hasNextToken()) {
		std::string token = tokenizer.nextToken();
		long long value = 0;
		if (utilities::isNumber(token)) {
			value = std::stoll(token.c_str(), nullptr, 0);
		}
		else {
			value = Token::getExpressionValue(tables, token);
		}

		if (value >= pow(2, size * 8)) {
			throw ErrorException(std::string("Syntax error, value out of bound, line: ").append(std::to_string(line)));
		}

		if (value != 0 && tables->getCurrentSection().substr(0, 4) == ".bss") {
			throw ErrorException(std::string("Syntax error, can't assemble non zero values in bss section, line: ").append(std::to_string(line)));
		}

		char *buffer = new char[size];
		utilities::pack(buffer , value, size);
		tables->getCurrentSectionTable()->addContent(buffer, size);
		delete[] buffer;
	}
}

Align::Align(std::string& _directive, int _line) : Directive(_line), directive(_directive) { }

FirstPassAlign::FirstPassAlign(std::string& _parameters, int _line) : Align(_parameters, _line) { }

void FirstPassAlign::resolve(Tables *tables) {
	if (directive[5] != ' ') {
		throw ErrorException(std::string("Unkown directive, line: ").append(std::to_string(line)));
	}
	//check format
	utilities::StringTokenizer tokenizer(directive.substr(6), ",");
	//check exponent
	unsigned int alignBy = 0;
	std::string number;
	if (!tokenizer.hasNextToken() || !utilities::isNumber(number = tokenizer.nextToken())) {
		throw ErrorException(std::string("ALIGN directive, exponent field missing or not a number, line: ").append(std::to_string(line))); 
	}
	else {
		alignBy = pow(2, strtol(number.c_str(), nullptr, 0));
	}
	//check fill
	if (tokenizer.hasNextToken()) {
		std::string token = tokenizer.nextToken();
		if (!utilities::isNumber(token)) {
			throw ErrorException(std::string("ALIGN directive, fill field not a number, line: ").append(std::to_string(line)));
		}
	}
	//check max field
	unsigned max = ~0;
	if (tokenizer.hasNextToken()) {
		if (!utilities::isNumber(number = tokenizer.nextToken())) {
			throw ErrorException(std::string("ALIGN directive, max field, not a number, line: ").append(std::to_string(line)));
		}
		else {
			max = strtol(number.c_str(), nullptr, 0);
		}
	}
	if (tokenizer.hasNextToken()) {
		throw ErrorException(std::string("ALIGN directive, too many parameters, line: ").append(std::to_string(line)));
	}
	if (alignBy > max) {
		alignBy = max;
	}
	alignBy = alignBy - (tables->getLocationCounter() % alignBy);
	tables->incrementLocationCounter(alignBy);
}

SecondPassAlign::SecondPassAlign(std::string& _directive, int _line) : Align(_directive, _line) { }

void SecondPassAlign::resolve(Tables *tables) {
	utilities::StringTokenizer tokenizer(directive.substr(6), ",");
	//get exponent field
	unsigned int alignBy = std::strtol(tokenizer.nextToken().c_str(), nullptr, 0);
	unsigned int fill = 0;
	if (tokenizer.hasNextToken()) {
		fill = strtol(tokenizer.nextToken().c_str(), nullptr, 0);
		if(fill > 0xFF) {
			throw ErrorException(std::string("ALIGN directive, fille field to big").append(std::to_string(line)));
		} 
	}
	else if (tables->getCurrentSection().substr(0, 4) == "text") {
		fill = 16;
	}
	unsigned int max = ~0;
	if (tokenizer.hasNextToken()) {
		max = strtol(tokenizer.nextToken().c_str(), nullptr, 0);
	}
	if (alignBy > max) {
		alignBy = max;
	}
	alignBy = alignBy - (tables->getLocationCounter() % alignBy) - 1;
	char *buffer = new char[alignBy];
	memset(buffer, fill, alignBy);
	tables->getCurrentSectionTable()->addContent(buffer, alignBy);
	delete[] buffer;
}

ScopeChange::ScopeChange(std::string& _directive, int _line) : Directive(_line), directive(_directive) { }

void ScopeChange::resolve(Tables *tables) {
	if (directive[6] != ' ') {
		throw ErrorException(std::string("Unkown directive, line: ").append(std::to_string(line)));
	}
	utilities::StringTokenizer tokenizer(directive.substr(7), ",");
	while (tokenizer.hasNextToken()) {
		std::string token = tokenizer.nextToken();
		token = utilities::removeBlankSpacesBeforeAndAfter(token);
		changeScope(tables, token);
	}
}

Public::Public(std::string& _symbols, int _line) : ScopeChange(_symbols, _line) { }

void Public::changeScope(Tables *tables, std::string& symbol) {
	if (tables->getSymbolTable()->doesExist(symbol)) {
		tables->getSymbolTable()->setScope(symbol, true);
	}
	else {
		throw ErrorException(std::string("PUBLIC directive, symbol ").append(symbol).append(" undefined, line: ").append(std::to_string(line)));
	}
}

Extern::Extern(std::string& _symbols, int _line) : ScopeChange(_symbols, _line) { }

void Extern::changeScope(Tables *tables, std::string& symbol) {
	//if symbols exist in this module conflict
	if (!tables->getSymbolTable()->doesExist(symbol)) {
		std::string section = std::string("UND");
		tables->getSymbolTable()->addSymbol(symbol, section, 0);
		tables->getSymbolTable()->setScope(symbol, true);
	}
	else {
		throw ErrorException(std::string("EXTERN directive, symbol ").append(symbol).append(" already exists, line: ").append(std::to_string(line)));
	}
}

End::End(std::string& _directive, int _line) : Directive(_line), directive(_directive) { }

void End::resolve(Tables *tables) {
	if (directive.length() != 3 && directive.find_first_not_of(' ', 3) != std::string::npos && directive.find_first_not_of('\t', 3) != std::string::npos) {
		throw ErrorException(std::string("END directive, syntax error, line: ").append(std::to_string(line)));
	}
	tables->addSectionTable();
}

Asciiz::Asciiz(std::string& _directive, int _line) : Directive(_line) {
	text = _directive.substr(7);
	text = utilities::removeBlankSpacesBeforeAndAfter(text);
	if(text[0] != '\"' || text[text.length() - 1] != '\"') {
		throw ErrorException(std::string("ASCIIZ directive, syntax error, line: ").append(std::to_string(line)));
	}
	text = text.substr(1, text.length() - 2);
}

FirstPassAsciiz::FirstPassAsciiz(std::string& _directive, int _line) : Asciiz(_directive, _line) { }

void FirstPassAsciiz::resolve(Tables *tables) {
	tables->incrementLocationCounter(text.length() + 1);
}

SecondPassAsciiz::SecondPassAsciiz(std::string& _directive, int _line) : Asciiz(_directive, _line) { }

void SecondPassAsciiz::resolve(Tables *tables) {
	tables->getCurrentSectionTable()->addContent(text.c_str(), text.length() + 1);
}