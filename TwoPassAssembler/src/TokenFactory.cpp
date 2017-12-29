#include "TokenFactory.h"
#include "ErrorException.h"
#include "Token.h"
#include "Label.h"
#include "Instruction.h"
#include "Directive.h"
#include "Utilities.h"
#include <string>
#include <fstream>

TokenFactory::TokenFactory(std::string& fname) : line(""), position(0), lineNumber(0), end(false), input(fname) {
	if (!input.is_open()) {
		throw ErrorException(std::string("File: ").append(fname).append(" not found"));
	}
}

TokenFactory::~TokenFactory() {
	input.close();
}

Token* TokenFactory::nextToken() {
	if (end) {
		return nullptr;
	}
	while (line == "" || line[position] == '\0') {
		if (input.eof() && !end) {
			throw ErrorException(std::string("End of file, no end directive, line: ").append(std::to_string(lineNumber)));
		}

		lineNumber++;
		getline(input, line, '\n');
		position = 0;

		line = utilities::removeBlankSpacesBeforeAndAfter(line);
	}

	while ((line[position] == ' ' || line[position] == '\t') && line[position] != '\0') {
		position++;
	}

	if (line.find_first_of(':', position) != std::string::npos) {
		std::string lname = line.substr(position, line.find_first_of(":"));
		position = line.find_first_of(":") + 1;
		return newLabel(lname);
	}
	else if (line[position] == '.') {
		std::string directive = line.substr(position + 1, line.length());
		line = "";
		return newDirective(directive);
	}
	else {
		std::string instruction = line.substr(position, line.length());
		line = "";
		return newInstruction(instruction);
	}
}

FirstPassTokenFactory::FirstPassTokenFactory(std::string& fname) : TokenFactory(fname), section("") { }

Token* FirstPassTokenFactory::newLabel(std::string& name) {
	return new Label(name, lineNumber);
}

Token* FirstPassTokenFactory::newInstruction(std::string& line) {
	if (section != "text") {
		throw ErrorException(std::string("Instruction not in text section, line: ").append(std::to_string(lineNumber)));
	}
	return new FirstPassInstruction(line, lineNumber);
}

Token* FirstPassTokenFactory::newDirective(std::string& line) {
	if (line.substr(0, 3) == "bss") {
		section = "bss";
		return new FirstPassSection(line, lineNumber);
	}
	else if (line.substr(0, 4) == "text" || line.substr(0, 4) == "data") {
		section = line.substr(0, 4);
		return new FirstPassSection(line, lineNumber);
	}
	else if (line.substr(0, 4) == "char" || line.substr(0, 4) == "word" || line.substr(0, 4) == "long") {
		return new MemoryConsuming(line, lineNumber);
	}
	else if (line.substr(0, 4) == "skip") {
		return new FirstPassSkip(line, lineNumber);
	}
	else if (line.substr(0, 5) == "align") {
		return new FirstPassAlign(line, lineNumber);
	} 
	else if(line.substr(0, 6) == "asciiz") {
		return new FirstPassAsciiz(line, lineNumber);
	}
	else if (line.substr(0, 3) == "end") {
		end = true;
		return new End(line, lineNumber);
	}
	else{
		return new Token(lineNumber);
	}
}

SecondPassTokenFactory::SecondPassTokenFactory(std::string& fname) : TokenFactory(fname) { }

Token* SecondPassTokenFactory::newLabel(std::string& name) {
	return new Token(lineNumber);
}

Token* SecondPassTokenFactory::newDirective(std::string& line) {
	if (line.substr(0, 3) == "bss" || line.substr(0, 4) == "text" || line.substr(0, 4) == "data") {
		return new SecondPassSection(line, lineNumber);
	}
	else if (line.substr(0, 4) == "skip") {
		return new SecondPassSkip(line, lineNumber);
	}
	else if (line.substr(0, 5) == "align" && line[5] == ' ') {
		return new SecondPassAlign(line, lineNumber);
	}
	else if (line.substr(0, 4) == "char" || line.substr(0, 4) == "word" || line.substr(0, 4) == "long") {
		return new MemoryAllocation(line, lineNumber);
	}
	else if (line.substr(0, 6) == "public" || line.substr(0,6) == "global") {
		return new Public(line, lineNumber);
	}
	else if (line.substr(0, 6) == "extern") {
		return new Extern(line, lineNumber);
	} 
	else if (line.substr(0, 6) == "asciiz") {
		return new SecondPassAsciiz(line, lineNumber);
	}
	else if (line.substr(0, 3) == "end") {
		return nullptr;
	}
	else {
		throw ErrorException(std::string("Unkown directive, line: ").append(std::to_string(lineNumber)));
	}
}

Token* SecondPassTokenFactory::newInstruction(std::string& line) {
	if (line.substr(0, 3) == "int") {
		return new Int(line, lineNumber);
	}
	else if (line.substr(0, 3) == "add" || line.substr(0, 3) == "sub" || line.substr(0, 3) == "mul" || line.substr(0, 3) == "div" || line.substr(0, 3) == "cmp") {
		return new ArithmeticAndCmp(line, lineNumber);
	}
	else if (line.substr(0, 3) == "and" || line.substr(0, 2) == "or" || line.substr(0, 3) == "not" || line.substr(0, 4) == "test") {
		return new Logical(line, lineNumber);
	}
	else if (line.substr(0, 3) == "ldr" || line.substr(0, 3) == "str") {
		return new LdrStr(line, lineNumber);
	}
	else if (line.substr(0, 4) == "push") {
		return new Push(line, lineNumber);
	}
	else if (line.substr(0, 3) == "pop") {
		return new Pop(line, lineNumber);
	}
	else if (line.substr(0, 4) == "call") {
		return new Call(line, lineNumber);
	}
	else if (line.substr(0, 3) == "out" || line.substr(0, 2) == "in") {
		return new InOut(line, lineNumber);
	}
	else if (line.substr(0, 3) == "mov" || line.substr(0, 3) == "shr" || line.substr(0, 3) == "shl") {
		return new MovShrShl(line, lineNumber);
	}
	else if (line.substr(0, 3) == "ret") {
		return new Ret(line, lineNumber);
	}
	else if (line.substr(0, 4) == "iret") {
		return new Iret(line, lineNumber);
	}
	else if (line.substr(0, 3) == "ldc") {
		return new Ldc(line, lineNumber);
	}
	else {
		throw ErrorException(std::string("Unkown instruction, line: ").append(std::to_string(lineNumber)));
	}
}