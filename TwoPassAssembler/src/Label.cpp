#include "Label.h"
#include "ErrorException.h"
#include "Utilities.h"
#include <string>

Label::Label(std::string& _name, int _line) : Token(_line), name(_name) { }

void Label::resolve(Tables *tables) {
	std::string beginning = name.substr(0, 1);
	if (utilities::isNumber(beginning)) {
		throw ErrorException(std::string("Label cant begin with a nubmer, line: ").append(std::to_string(line)));
	}
	if (name.find_first_of(INVALID_CHARACTERS) != std::string::npos) {
		throw ErrorException(std::string("Label contains invalid characters, line: ").append(std::to_string(line)));
	}
	if (!tables->getSymbolTable()->doesExist(name)) {
		std::string section = tables->getCurrentSection();
		tables->getSymbolTable()->addSymbol(name, section, tables->getLocationCounter());
	}
	else {
		throw ErrorException(std::string("Label already exists, line: ").append(std::to_string(line)));
	}
}