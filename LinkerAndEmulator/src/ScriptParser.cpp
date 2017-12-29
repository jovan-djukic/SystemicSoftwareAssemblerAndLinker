#include "ScriptParser.h"
#include "Utilities.h"
#include "ErrorException.h"
#include <string>

#define INVALIDE_CHARACTERS "() \t,"

Token::Token(std::string _line, int _lineNumber) : line(_line), lineNumber(_lineNumber) { }

void Token::resolve(SymbolTables *symbolTables, GlobalSymbolTable *globalSymbolTable, SectionList *sectionList, int& currentPosition) {

}

int Token::exppressionEvaluation(std::string& expression, GlobalSymbolTable *globalSymbolTable, int& currentPosition) {
	expression = utilities::removeBlankSpacesBeforeAndAfter(expression);
	if (expression[0] == '-') {
		expression = std::string("0").append(expression);
	}

	int value = 0;
	utilities::StringTokenizer tokenizer(expression, "+-");
	while (tokenizer.hasNextToken()) {
		int increment = 0;
		std::string symbol = tokenizer.nextToken();
		symbol = utilities::removeBlankSpacesBeforeAndAfter(symbol);
		if (symbol.find_first_of(INVALIDE_CHARACTERS) != std::string::npos && symbol.substr(0, 5) != "align") {
			throw ErrorException(std::string("Inadequate symbol in expression, line: ").append(std::to_string(lineNumber)));
		}
		if (utilities::isNumber(symbol)) {
			increment = strtol(symbol.c_str(), nullptr, 0);
		} 
		else if (symbol.substr(0, 5) == "align") {
			increment = Token::align(symbol, currentPosition);
		}
		else {
			if (!globalSymbolTable->doesExist(symbol) && symbol != ".") {
				throw ErrorException(std::string("Undefined symbol ").append(symbol).append(", line: ").append(std::to_string(lineNumber)));
			}
			if (symbol != ".") {
				increment = globalSymbolTable->getValue(symbol);
			}
			else {
				increment = currentPosition;
			}
		}
		if (tokenizer.getLastDelimiter() == '-') {
			increment = -1 * increment;
		}
		value += increment;
	}

	return value;
}

int Token::align(std::string& alignExpression, int& currentPosition) {
	int start = alignExpression.find_first_of('(') + 1, end = alignExpression.find_last_of(')');
	std::string parameters = alignExpression.substr(start, end - start);
	parameters = utilities::removeBlankSpacesBeforeAndAfter(parameters);
	utilities::StringTokenizer tokenizer(parameters, ",");
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Align operation, not enough parameters, line: ").append(std::to_string(lineNumber)));
	}
	std::string firstParameter = tokenizer.nextToken();
	firstParameter = utilities::removeBlankSpacesBeforeAndAfter(firstParameter);
	if (!utilities::isNumber(firstParameter) && firstParameter != ".") {
		throw ErrorException(std::string("Align operation, first parameter not a number, line: ").append(std::to_string(lineNumber)));
	}
	long address = firstParameter == "." ? currentPosition : std::strtol(firstParameter.c_str(), nullptr, 0);
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Align operation, not enough parameters, line: ").append(std::to_string(lineNumber)));
	}
	std::string secondParameter = tokenizer.nextToken();
	secondParameter = utilities::removeBlankSpacesBeforeAndAfter(secondParameter);
	if (!utilities::isNumber(secondParameter)) {
		throw ErrorException(std::string("Align operation, second parameter not a number, line: ").append(std::to_string(lineNumber)));
	}
	long delimiter = std::strtol(secondParameter.c_str(), nullptr, 0);

	if (address % delimiter != 0) {
		return address / delimiter * delimiter + delimiter;
	}
	else {
		return address;
	}
}

EqualityToken::EqualityToken(std::string line, int lineNumber) : Token(line, lineNumber) { }

void EqualityToken::resolve(SymbolTables *symbolTables, GlobalSymbolTable *globalSymbolTable, SectionList *sectionList, int &currentPosition) {
	utilities::StringTokenizer tokenizer(line, "=");
	std::string symbol = tokenizer.nextToken();
	symbol = utilities::removeBlankSpacesBeforeAndAfter(symbol);
	if (symbol == "" || symbol.find_first_of(INVALIDE_CHARACTERS) != std::string::npos) {
		throw ErrorException(std::string("Inadequate symbol on left side of equality, line: ").append(std::to_string(lineNumber)));
	}

	if (symbol != "." && globalSymbolTable->doesExist(symbol)) {
		throw ErrorException(std::string("Symbol redefiniton, line: ").append(std::to_string(lineNumber)));
	}

	std::string token = tokenizer.nextToken();
	token = utilities::removeBlankSpacesBeforeAndAfter(token);
	int value = Token::exppressionEvaluation(token, globalSymbolTable, currentPosition);

	if (tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Invalid number of parameters, line: ").append(std::to_string(lineNumber)));
	}
	if (symbol == "." ) {
		if (currentPosition >= value) {
			throw ErrorException(std::string("Current position redefinition invalid, value is smaller then previous, line: ").append(std::to_string(lineNumber)));
		}
		else {
			currentPosition = value;
		}
	}
	else {
		globalSymbolTable->addSymbol(symbol, value);
	}

}

PlacementToken::PlacementToken(std::string line, int lineNumber) : Token(line, lineNumber) { }

void PlacementToken::resolve(SymbolTables *symbolTables, GlobalSymbolTable *globalSymbolTable, SectionList *sectionList, int& currentPosition) {
	utilities::StringTokenizer tokenizer(line, "()");
	std::string file = tokenizer.nextToken();
	file = utilities::removeBlankSpacesBeforeAndAfter(file);
	if (file.find_first_of(INVALIDE_CHARACTERS) != std::string::npos) {
		throw ErrorException(std::string("Invalide file name, line: ").append(std::to_string(lineNumber)));
	}
	if (tokenizer.getNextDelimiter() != '(' && tokenizer.getNextDelimiter() != '\0') {
		throw ErrorException(std::string("Syntax error, invalid placement directive, line: ").append(std::to_string(lineNumber)));
	}
	if (!symbolTables->doesExist(file)) {
		throw ErrorException(std::string("File doesn't exist, line: ").append(std::to_string(lineNumber)));
	}

	std::string section;

	if (tokenizer.hasNextToken()) {
		section = tokenizer.nextToken();
		section = utilities::removeBlankSpacesBeforeAndAfter(section);
		if (tokenizer.getNextDelimiter() != ')') {
			throw ErrorException(std::string("Syntax error, invalid placement directive, line: ").append(std::to_string(lineNumber)));
		}
		if (section.find_first_of(INVALIDE_CHARACTERS) != std::string::npos) {
			throw ErrorException(std::string("Invalide section name, line: ").append(std::to_string(lineNumber)));
		}
		if (tokenizer.hasNextToken() && tokenizer.nextToken() != "") {
			throw ErrorException(std::string("Syntax error, invalid placement directive, too many paramenters, line: ").append(std::to_string(lineNumber)));
		}
		if (!symbolTables->doesExist(file, section)) {
			throw ErrorException(std::string("Section doesn't exist, line: ").append(std::to_string(lineNumber)));
		}
	}
	else {
		section = SectionList::all;
	}

	while (sectionList->doesExist(file, section)) {
		SectionList::Entry entry;
		entry = sectionList->remove(file, section);
		symbolTables->relocate(entry.file, entry.name, currentPosition, globalSymbolTable);
		currentPosition += entry.size;
	}
}

ScriptParser::ScriptParser(std::string scriptName) : script(scriptName), lineNumber(0) {
	if (!script.is_open()) {
		throw ErrorException(std::string("Cannot open script file").append(scriptName));
	}
}

bool ScriptParser::hasNextToken() {
	return !script.eof();
}

Token* ScriptParser::getNextToken() {
	std::string line = "";
	while (!script.eof() && line == "") {
		std::getline(script, line);
		lineNumber++;
		line = utilities::removeBlankSpacesBeforeAndAfter(line);
	}
	if (line == "") {
		return new Token(line, lineNumber);
	}
	else if (line.find_first_of('=') != std::string::npos) {
		return new EqualityToken(line, lineNumber);
	}
	else {
		return new PlacementToken(line, lineNumber);
	}
}

ScriptParser::~ScriptParser() {
	script.close();
}
