#include "Token.h"
#include "Tables.h"
#include "Utilities.h"
#include "ErrorException.h"
#include <stack>
#include <cmath>

Token::Token(int _line) : line(_line) { }

void Token::resolve(Tables *tables) {

}

int Token::getClassificationIndex(Tables *tables, std::string expression) {
	expression = utilities::removeBlankSpacesBeforeAndAfter(expression);
	if (expression[0] == '-') {
		expression = std::string("0").append(expression);
	}
	utilities::StringTokenizer tokenizer(expression, "+-");
	int returnValue = 0;
	//parse expression
	std::map<std::string, int> table;

	while (tokenizer.hasNextToken()) {
		std::string token = tokenizer.nextToken();
		token = utilities::removeBlankSpacesBeforeAndAfter(token);

		if (!utilities::isNumber(token) && token != "*") {
			if (token.find_first_of(INVALID_CHARACTERS) != std::string::npos) {
				throw ErrorException(std::string("Syntax error, incorect symbol name: ").append(token).append(", line: ").append(std::to_string(line)));
			}
			if (!tables->getSymbolTable()->doesExist(token)) {
				std::string section = std::string("UND");
				tables->getSymbolTable()->addSymbol(token, section, 0);
				tables->getSymbolTable()->setScope(token, true);
			}
		}

		if (!utilities::isNumber(token)) {
			std::string section;
			if (token == "*") {
				section = tables->getCurrentSection();
			}
			else {
				section = tables->getSymbolTable()->getSection(token);
			}
			if (table.find(section) == table.end()) {
				table.insert(std::pair<std::string, int>(section, 0));
			}
			if (tokenizer.getLastDelimiter() == '-') {
				table[section]--;
			}
			else {
				table[section]++;
			}
		}
	}

	int classificationIndex = 0;
	int count = 0;
	for (std::map<std::string, int>::iterator i = table.begin(); i != table.end(); ++i) {
		if (i->second != 0) {
			classificationIndex = 1;
			count += i->second;
		}
	}
	if (count != 0 && count != 1) {
		classificationIndex = 2;
	}
	return classificationIndex;
}

int Token::getExpressionValue(Tables *tables, std::string expression) {
	expression = utilities::removeBlankSpacesBeforeAndAfter(expression);
	if (expression[0] == '-') {
		expression = std::string("0").append(expression);
	}

	long long value = 0;
	struct Entry {
		std::string symbol;
		char operation;
		Entry() { }
		Entry(std::string _symbol, char _operation) : symbol(_symbol), operation(_operation) { }
	};
	std::map<std::string, std::stack<Entry>> table;
	utilities::StringTokenizer tokenizer(expression, "+-");
	while (tokenizer.hasNextToken()) {
		long long increment = 0;
		std::string symbol = tokenizer.nextToken();
		symbol = utilities::removeBlankSpacesBeforeAndAfter(symbol);
		if (utilities::isNumber(symbol)) {
			increment = strtoll(symbol.c_str(), nullptr, 0);
		}
		else {
			std::string section;
			if (symbol == "*") {
				section = tables->getCurrentSection();
				increment = tables->getCurrentSectionTable()->getCurrentPosition();
			}
			else {
				section = tables->getSymbolTable()->getSection(symbol);
				increment = tables->getSymbolTable()->getValue(symbol);
			}
			char operation = '+';
			if (tokenizer.getLastDelimiter() == '-') {
				operation = '-';
			}
			if (table.find(section) == table.end()) {
				table.insert(std::pair<std::string, std::stack<Entry>>(section, std::stack<Entry>()));
			}
			if (!table[section].empty() && table[section].top().operation != operation) {
				table[section].pop();
			}
			else {
				table[section].push(Entry(symbol, operation));
			}
		}
		if (tokenizer.getLastDelimiter() == '-') {
			increment = -1 * increment;
		}
		value += increment;
	}

	if (value > (pow(2, 32) - 1)) {
		throw ErrorException(std::string("Expression value out of bound, line: ").append(std::to_string(line)));
	}

	for (std::map<std::string, std::stack<Entry>>::iterator i = table.begin(); i != table.end(); ++i) {
		while (!i->second.empty()) {
			Entry entry = i->second.top();
			i->second.pop();
			std::string symbol = i->first;
			if (entry.symbol != "*" && tables->getSymbolTable()->isGlobal(entry.symbol)) {
				value -= tables->getSymbolTable()->getValue(entry.symbol);
				symbol = entry.symbol;
			}

			int address = tables->getCurrentSectionTable()->getCurrentPosition();
			int index = tables->getSymbolTable()->getIndex(symbol);
			char buffer[8];
			utilities::pack(buffer, address, 4);
			utilities::pack(buffer + 4, index, 3);
			buffer[7] = entry.operation == '+' ? 0 : 1;
			
			tables->getRelocationTable(std::string(".rel").append(tables->getCurrentSection()))->addContent(buffer, 8);
		}
	}

	return value;
}

Token::~Token() {

}