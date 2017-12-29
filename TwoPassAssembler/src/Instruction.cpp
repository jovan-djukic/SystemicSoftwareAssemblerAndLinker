#include "Instruction.h"
#include"Tables.h"
#include "ContentTable.h"
#include "Utilities.h"
#include "ErrorException.h"
#include <string>
#include <cmath>

Instruction::Instruction(std::string& _instruction, int _line) : Token(_line), instruction(_instruction) { }

Instruction::~Instruction() { }

FirstPassInstruction::FirstPassInstruction(std::string& _instruction, int _line) : Instruction(_instruction, _line) { }

void FirstPassInstruction::resolve(Tables *tables) {
	int by = 4;
	if (instruction.substr(0, 3) == "ldc" && !(instruction.substr(0, 4) == "ldcl" || instruction.substr(0, 4) == "ldch")) {
		by = 8;
	}
	tables->incrementLocationCounter(by);
}

SecondPassInstruction::SecondPassInstruction(std::string& _instruction, int _line) : Instruction(_instruction, _line), instructionLength(0) { }

char SecondPassInstruction::isRegister(std::string& s) {
	std::string expression = utilities::removeBlankSpacesBeforeAndAfter(s);
	for (int i = 0; i < 20; i++) {
		if (expression == std::string("r").append(std::to_string(i))) {
			return i;
		}
	}
	if (expression == "pc") {
		return 16;
	}
	else if (expression == "lr") {
		return 17;
	}
	else if (expression == "sp") {
		return 18;
	}
	else if (expression == "psw") {
		return 19;
	}
	else if (expression == "bp") {
		return 15;
	}
	else {
		return 20;
	}
}

char SecondPassInstruction::getConditionCode() {
	char returnValue = 7;
	std::string condition = instruction.substr(instructionLength, 2);
	if (condition == "eq") {
		returnValue = 0;
	}
	else if (condition == "ne") {
		returnValue = 1;
	}
	else if (condition == "gt") {
		returnValue = 2;
	}
	else if (condition == "ge") {
		returnValue = 3;
	}
	else if (condition == "lt") {
		returnValue = 4;
	}
	else if (condition == "le") {
		returnValue = 5;
	}
	if (returnValue != 7) {
		instructionLength += 2;
	}
	return returnValue;
}

char SecondPassInstruction::getFlagsBit() {
	char returnValue = 0;
	if (instruction[instructionLength] == 's') {
		returnValue = 1;
		instructionLength++;
	}
	return returnValue;
}

void SecondPassInstruction::resolve(Tables *tables) {
	char buffer[4];
	char operationCode = getOperationCode();
	char condition = getConditionCode();
	char flagsBit = getFlagsBit();

	if (instruction[instructionLength] != ' ' && instruction[instructionLength] != '\t') {
		throw ErrorException(std::string("Syntax error, unknown instruction, line: ").append(std::to_string(line)));
	}

	buffer[3] = (condition << 5) | (flagsBit << 4) | operationCode;
	getParameters(tables, buffer);

	tables->getCurrentSectionTable()->addContent(buffer, 4);
}

Int::Int(std::string& _instruction, int _line) : SecondPassInstruction(_instruction, _line) { }

char Int::getOperationCode() {
	instructionLength = 3;
	return 0;
}

void Int::getParameters(Tables *tables, char *buffer) {
	std::string number = instruction.substr(instructionLength + 1);
	if (!utilities::isNumber(number)) {
		throw ErrorException(std::string("INT instruction, parameter not a number, line: ").append(std::to_string(line)));
	}
	int interruptNumber = strtol(number.c_str(), nullptr, 0);
	if (interruptNumber >= 16 || (interruptNumber <= 3 && interruptNumber > 0)) {
		throw ErrorException(std::string("INT instruction, interrupt number not allowed, line: ").append(std::to_string(line)));
	}

	buffer[2] = ((char)interruptNumber) << 4;
	buffer[1] = buffer[0] = 0;
}

ArithmeticAndCmp::ArithmeticAndCmp(std::string& _instruction, int _line) : SecondPassInstruction(_instruction, _line) { }

char ArithmeticAndCmp::getOperationCode() {
	operation = instruction.substr(0, 3);
	instructionLength += 3;
	if (operation == "add") {
		return 1;
	}
	else if (operation == "sub") {
		return 2;
	}
	else if (operation == "mul") {
		return 3;
	}
	else if (operation == "div") {
		return 4;
	}
	else {
		return 5;
	}
}

void ArithmeticAndCmp::getParameters(Tables *tables, char *buffer) {
	utilities::StringTokenizer tokenizer(instruction.substr(instructionLength + 1), ",");
	char firstOperand = 20;
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Arithmetic instruction, no parameters, line: ").append(std::to_string(line)));
	}
	std::string token = tokenizer.nextToken();
	if ((firstOperand = SecondPassInstruction::isRegister(token)) >= 19 || (operation != "add" && operation != "sub" && firstOperand >= 16)) {
		throw ErrorException(std::string("Arithmetic instruction, invalid first instruction parameter, line: ").append(std::to_string(line)));
	}
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Arithmetic instruction, invalid number of parameters, line: ").append(std::to_string(line)));
	}
	
	token = tokenizer.nextToken();
	if (tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Arithmetic instruction, invalid number of parameters, line: ").append(std::to_string(line)));
	}
	if (SecondPassInstruction::isRegister(token) != 20) {
		char secondOperand = SecondPassInstruction::isRegister(token);
		if (operation != "add" && operation != "sub" && secondOperand >= 16) {
			throw ErrorException(std::string("Arithmetic instruction, invalid second register, line: ").append(std::to_string(line)));
		}

		buffer[2] = (firstOperand << 3) | (secondOperand >> 3);
		buffer[1] = secondOperand << 5;
		buffer[0] = 0;
	}
	else {
		if (Token::getClassificationIndex(tables, token) != 0) {
			throw ErrorException(std::string("Arithmetic instruction, second parameter not a constant, line: ").append(std::to_string(line)));
		}
		int secondOperand = Token::getExpressionValue(tables, token);
		if (secondOperand > (pow(2, 17) - 1) || secondOperand < (-1 * pow(2, 17))) {
			throw ErrorException(std::string("Arithmetic instruction, second parameter out of bound, line: ").append(std::to_string(line)));
		}

		buffer[2] = (firstOperand << 3) | (1 << 2) | ((secondOperand & 0x00030000) >> 16);
		buffer[1] = (secondOperand & 0x0000ff00) >> 8;
		buffer[0] = secondOperand & 0x000000ff;
	}
}

Logical::Logical(std::string& _instruction, int _line) : SecondPassInstruction(_instruction, _line) { }

char Logical::getOperationCode() {
	instructionLength += 3;
	if (instruction.substr(0, 2) == "or") {
		instructionLength--;
		return 7;
	}
	else if (instruction.substr(0, 4) == "test") {
		instructionLength++;
		return 9;
	}
	else if (instruction.substr(0, 3) == "and") {
		return 6;
	}
	else {
		return 8;
	}
}

void Logical::getParameters(Tables *tables, char *buffer) {
	utilities::StringTokenizer tokenizer(instruction.substr(instructionLength + 1), ",");
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Logical instruction, no instruction parameters, line: ").append(std::to_string(line)));
	}
	std::string token = tokenizer.nextToken();
	char firstOperand = SecondPassInstruction::isRegister(token);
	if (firstOperand >= 16 && firstOperand != 18) {
		throw ErrorException(std::string("Logical instruction, invalid first instruction parameter, line: ").append(std::to_string(line)));
	}
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Logical instruction, no second parameter, line: ").append(std::to_string(line)));
	}
	token = tokenizer.nextToken();
	char secondOperand = SecondPassInstruction::isRegister(token);
	if (tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Logical instruction, invalid number of parameters, line: ").append(std::to_string(line)));
	}
	if (secondOperand == 16 || secondOperand >= 18) {
		throw ErrorException(std::string("Logical instruction, invalid second instruction parameter, line: ").append(std::to_string(line)));
	}

	buffer[2] = (firstOperand << 3) | (secondOperand >> 2);
	buffer[1] = secondOperand << 6;
	buffer[0] = 0;
}

LdrStr::LdrStr(std::string& _instruction, int _line) : SecondPassInstruction(_instruction, _line), isLoad(1), mode(1) { }

char LdrStr::getOperationCode() {
	instructionLength += 3;
	if (instruction.substr(0, 3) == "str") {
		isLoad = 0;
	}
	if (instruction.substr(3, 2) == "ib") {
		mode = 4;
		instructionLength += 2;
	}
	else if (instruction.substr(3, 2) == "db") {
		mode = 5;
		instructionLength += 2;
	}
	else if (instruction.substr(3, 2) == "ia") {
		mode = 2;
		instructionLength += 2;
	}
	else if (instruction.substr(3, 2) == "da") {
		mode = 3;
		instructionLength += 2;
	}
	return 10;
}

void LdrStr::getParameters(Tables *tables, char *buffer) {
	utilities::StringTokenizer tokenizer(instruction.substr(instructionLength + 1), ",");
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Memory access instruction, no instruction parameters, line: ").append(std::to_string(line)));
	}
	std::string token = tokenizer.nextToken();
	char firstOperand = SecondPassInstruction::isRegister(token);
	if (firstOperand == 20) {
		throw ErrorException(std::string("Memory access instruction, invalid first instruction parameter, line: ").append(std::to_string(line)));
	}
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("Memory access instruction, invalid number of parameters, line: ").append(std::to_string(line)));
	}

	token = tokenizer.getRestOfTheLine();
	token = utilities::removeBlankSpacesBeforeAndAfter(token);
	char secondOperand = 20;
	int offset = 0;

	if (token[0] == '[' && token[token.length() - 1] == ']') {
		tokenizer = utilities::StringTokenizer(token.substr(1, token.length() - 2), ",");
		if (!tokenizer.hasNextToken()) {
			throw ErrorException(std::string("Memory access instruction, syntax error, address expression, base register not specified, line:").append(std::to_string(line)));
		}
		token = tokenizer.nextToken();
		secondOperand = SecondPassInstruction::isRegister(token);
		if (secondOperand >= 19) {
			throw ErrorException(std::string("Memory access instruction, invalid base register in address field, line: ").append(std::to_string(line)));
		}
		if (secondOperand == 16) {
			if (mode != 1) {
				throw ErrorException(std::string("Memory access instruction, invalid adress mode, line: ").append(std::to_string(line)));
			}
			mode = 0;
		}
		if (tokenizer.hasNextToken()) {
			token = tokenizer.nextToken();
			token = utilities::removeBlankSpacesBeforeAndAfter(token);

			if (!utilities::isNumber(token) && Token::getClassificationIndex(tables, token) != 0) {
				throw ErrorException(std::string("Memory access instruction, invalid expression in adress field, line: ").append(std::to_string(line)));
			}
			offset = Token::getExpressionValue(tables, token);
			if (offset > (pow(2, 9) - 1) || offset < (-1 * pow(2, 9))) {
				throw ErrorException(std::string("Memory access instruction, invalid offset in adress field, line: ").append(std::to_string(line)));
			}
			if (tokenizer.hasNextToken()) {
				throw ErrorException(std::string("Memory access instruction, invalid number of parameters in address field, line: ").append(std::to_string(line)));
			}
		}
	}
	else {
		if (mode != 1) {
			throw ErrorException(std::string("Memory access instruction, invalid adress mode, line: ").append(std::to_string(line)));
		}
		std::string expression = token.append("-8-*");
		if (Token::getClassificationIndex(tables, expression) != 0) {
			throw ErrorException(std::string("Memory access instruction, invalid adress field, line: ").append(std::to_string(line)));
		}
		offset = Token::getExpressionValue(tables, expression);
		if (offset > (pow(2, 9) - 1) || offset < (-1 * pow(2, 9))) {
			throw ErrorException(std::string("Memory access instruction, label in address field too far, line: ").append(std::to_string(line)));
		}
		if (tokenizer.hasNextToken()) {
			throw ErrorException(std::string("Memory access instruction, invalid number of parameters, line: ").append(std::to_string(line)));
		}

		mode = 0;
		secondOperand = 16;
	}

	buffer[2] = (secondOperand << 3) | (firstOperand >> 2);
	buffer[1] = (firstOperand << 6) | (mode << 3) | (isLoad << 2) | ((offset & 0x300) >> 8);
	buffer[0] = (offset & 0xff);
}

Push::Push(std::string& _instruction, int _line) : LdrStr(_instruction, _line) { }

char Push::getOperationCode() {
	if (instruction[4] != ' ') {
		throw ErrorException(std::string("PUSH instruction, syntax error, line: ").append(std::to_string(line)));
	}
	std::string parameter = instruction.substr(5);
	parameter = utilities::removeBlankSpacesBeforeAndAfter(parameter);
	instruction = std::string("strdb ").append(parameter).append(", [sp]");
	return LdrStr::getOperationCode();
}

Pop::Pop(std::string& _instruction, int _line) : LdrStr(_instruction, _line) { }

char Pop::getOperationCode() {
	if (instruction[3] != ' ') {
		throw ErrorException(std::string("POP instruction, syntax error, line: ").append(std::to_string(line)));
	}
	std::string parameter = instruction.substr(4);
	parameter = utilities::removeBlankSpacesBeforeAndAfter(parameter);
	instruction = std::string("ldria ").append(parameter).append(", [sp]");
	return LdrStr::getOperationCode();
}

Call::Call(std::string& _instruction, int _line) : SecondPassInstruction(_instruction, _line) { }

char Call::getOperationCode() {
	instructionLength += 4;
	return 12;
}

void Call::getParameters(Tables *tables, char *buffer) {
	char firstOperand = 20;
	int offset = 0;
	std::string token = instruction.substr(instructionLength + 1);
	token = utilities::removeBlankSpacesBeforeAndAfter(token);
	
	if (token[0] == '[' && token[token.length() - 1] == ']') {
		utilities::StringTokenizer tokenizer(token.substr(1, token.length() - 2), ",");
		token = tokenizer.nextToken();
		if ((firstOperand = SecondPassInstruction::isRegister(token)) == 20) {
			throw ErrorException(std::string("CALL instruction, invalid base register, line: ").append(std::to_string(line)));
		}
		if (tokenizer.hasNextToken()) {
			token = tokenizer.nextToken();
			if (Token::getClassificationIndex(tables, token) != 0 || (offset = Token::getExpressionValue(tables, token)) > (pow(2, 18) - 1) || offset < (-1 * pow(2, 18))) {
				throw ErrorException(std::string("CALL instruction, invalid offset, line: ").append(std::to_string(line)));
			}
			if (tokenizer.hasNextToken()) {
				throw ErrorException(std::string("CALL instruction, invalid number of parameters in address field, line: ").append(std::to_string(line)));
			}
		}
	}
	else {
		std::string expression = token.append("-8-*");
	
		if (Token::getClassificationIndex(tables, expression) != 0) {
			throw ErrorException(std::string("CALL instruction, invalid parameter, line: ").append(std::to_string(line)));
		}
		firstOperand = 16;
		offset = Token::getExpressionValue(tables, expression);
		if (offset > (pow(2, 18) - 1) || offset < (-1 * pow(2, 18))) {
			throw ErrorException(std::string("CALL instruction, label too far, line: ").append(std::to_string(line)));
		}
	}

	buffer[2] = (firstOperand << 3) | ((offset & 0x70000) >> 16);
	buffer[1] = (offset & 0x0000ff00) >> 8;
	buffer[0] = offset & 0x000000ff;
}	

InOut::InOut(std::string& _instruction, int _line) : SecondPassInstruction(_instruction, _line), isIn(1) { }

char InOut::getOperationCode() {
	instructionLength += 2;
	if (instruction.substr(0, 3) == "out") {
		isIn = 0;
		instructionLength++;
	}
	return 13;
}

void InOut::getParameters(Tables *tables, char *buffer) {
	utilities::StringTokenizer tokenizer(instruction.substr(instructionLength + 1), ",");
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("IO memory access instruction, no instruction parameters, line: ").append(std::to_string(line)));
	}
	std::string token = tokenizer.nextToken();
	char firstOperand = SecondPassInstruction::isRegister(token);
	if (firstOperand >= 16) {
		throw ErrorException(std::string("IO memory access instruction, invalid register, line: ").append(std::to_string(line)));
	}
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("IO memory access instruction, no second instruction parameter, line: ").append(std::to_string(line)));
	}
	token = tokenizer.nextToken();
	token = utilities::removeBlankSpacesBeforeAndAfter(token);
	if (token[0] != '[' || token[token.length() - 1] != ']') {
		throw ErrorException(std::string("IO memory access instruction, syntax error, address field, line: ").append(std::to_string(line)));
	}
	token = token.substr(1, token.length() - 2);
	char secondOperand = SecondPassInstruction::isRegister(token);
	if (secondOperand >= 16) {
		throw ErrorException(std::string("IO memory access instruction, invalide adress register, line: ").append(std::to_string(line)));
	}
	if (tokenizer.hasNextToken()) {
		throw ErrorException(std::string("IO memory access instruction, invalid number of parameters, line: ").append(std::to_string(line)));
	}

	buffer[2] = (firstOperand << 4) | secondOperand;
	buffer[1] = isIn == 1 ? 0x80 : 0x00; 
 	buffer[0] = 0;
}

MovShrShl::MovShrShl(std::string& _instruction, int _line) : SecondPassInstruction(_instruction, _line), code(0), isL(1) { }

char MovShrShl::getOperationCode() {
	instructionLength += 3;
	if (instruction.substr(0, 3) == "mov") {
		code = 1; 
	}
	else if (instruction.substr(0, 3) == "shr") {
		isL = 0;
	}
	return 14;
}

void MovShrShl::getParameters(Tables *tables, char *buffer) {
	utilities::StringTokenizer tokenizer(instruction.substr(instructionLength + 1), ",");
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("MOV, SHR or SHL instruction, no instruction parameters, line: ").append(std::to_string(line)));
	}
	char firstOperand = 0, secondOperand = 0;
	int offset = 0;
	std::string token = tokenizer.nextToken();
	if ((firstOperand = SecondPassInstruction::isRegister(token)) == 20) {
		throw ErrorException(std::string("MOV, SHR or SHL instruction, invalid first instruction parameters, line: ").append(std::to_string(line)));
	}
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("MOV, SHR or SHL instruction, invalid numbe of instruction parameters, line: ").append(std::to_string(line)));
	}
	token = tokenizer.nextToken();
	if ((secondOperand = SecondPassInstruction::isRegister(token)) == 20) {
		throw ErrorException(std::string("MOV, SHR or SHL instruction, invalid first instruction parameters, line: ").append(std::to_string(line)));
	}
	if (code != 1) {
		if (!tokenizer.hasNextToken()) {
			throw ErrorException(std::string("SHR or SHL instruction, offset not specified, line: ").append(std::to_string(line)));
		}
		token = tokenizer.nextToken();
		token = utilities::removeBlankSpacesBeforeAndAfter(token);
		if (!utilities::isNumber(token)) {
			throw ErrorException(std::string("SHR or SHL instruction, offset invalid, line: ").append(std::to_string(line)));
		}
		offset = strtol(token.c_str(), nullptr, 0);
		if (offset > 31 || offset < 0) {
			throw ErrorException(std::string("SHR or SHL instruction, offset invalid, line: ").append(std::to_string(line)));
		}
	}
	if (tokenizer.hasNextToken()) {
		throw ErrorException(std::string("MOV, SHR or SHL instruction, invalid number of parameters, line: ").append(std::to_string(line)));
	}

	buffer[2] = (firstOperand << 3) | (secondOperand >> 2);
	buffer[1] = (secondOperand << 6) | (offset << 1) | isL;
	buffer[0] = 0;
}

Ret::Ret(std::string& _instruction, int _line) : MovShrShl(_instruction, _line) { }

char Ret::getOperationCode() {
	if (instruction.length() != 3 && instruction.find_first_not_of(' ', 3) != std::string::npos) {
		throw ErrorException(std::string("RET instruction, syntax error, line: ").append(std::to_string(line)));
	}
	instruction = std::string("mov pc, lr");
	return MovShrShl::getOperationCode();
}

Iret::Iret(std::string& _instruction, int _line) : MovShrShl(_instruction, _line) { }

char Iret::getOperationCode() {
	if (instruction.length() != 4 && instruction.find_first_not_of(' ', 4) != std::string::npos) {
		throw ErrorException(std::string("IRET instruction, syntax error, line: ").append(std::to_string(line)));
	}
	instruction = std::string("movs pc, lr");
	return MovShrShl::getOperationCode();
}

Ldc::Ldc(std::string& _instruction, int _line) : SecondPassInstruction(_instruction, _line), pseudo(false), isHigh(1) { }

char Ldc::getOperationCode() {
	instructionLength += 4;
	if(instruction.substr(0, 4) == "ldcl") {
		isHigh = 0;
	}
	else if (instruction.substr(0, 4) != "ldch") {
		pseudo = true;
		instructionLength -= 1;
	}
	return 15;
}
 
void Ldc::getParameters(Tables *tables, char *buffer) {
	utilities::StringTokenizer tokenizer(instruction.substr(instructionLength + 1), ",");
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("LDC instruction, no instruction parameters, line: ").append(std::to_string(line)));
	}
	char firstOperand = 0;
	std::string token = tokenizer.nextToken();
	if ((firstOperand = SecondPassInstruction::isRegister(token)) >=16) {
		throw ErrorException(std::string("LDC instruction, invalid register, line: ").append(std::to_string(line)));
	}
	int c = 0;
	if (!tokenizer.hasNextToken()) {
		throw ErrorException(std::string("LDC instruction, invalid number of parameters, line: ").append(std::to_string(line)));
	}
	token = tokenizer.nextToken();
	token = utilities::removeBlankSpacesBeforeAndAfter(token);
	if (!utilities::isNumber(token) && Token::getClassificationIndex(tables, token) != 0) {
		throw ErrorException(std::string("LDC instruction, second parameter not a constant, line: ").append(std::to_string(line)));
	}
	if (tokenizer.hasNextToken()) {
		throw ErrorException(std::string("LDC instruction, invalid number of parameters, line: ").append(std::to_string(line)));
	}
	c = Token::getExpressionValue(tables, token);
	if (pseudo == 1) {
		Ldc(std::string("ldcl r").append(std::to_string((int)firstOperand)).append(", ").append(std::to_string(c & 0xffff)), line).resolve(tables);
		c >>= 16;
	}

	buffer[2] = (firstOperand << 4) | (isHigh << 3);
	buffer[1] = (c & 0xff00) >> 8;
	buffer[0] = c & 0xff;
}