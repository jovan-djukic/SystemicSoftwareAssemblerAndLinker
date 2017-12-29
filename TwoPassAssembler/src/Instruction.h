#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

#include "Token.h"
#include <string>

class Instruction : public Token {
protected:
	std::string instruction;
public:
	Instruction(std::string&, int);
	virtual ~Instruction() = 0;
};

class FirstPassInstruction : public Instruction {
public:
	FirstPassInstruction(std::string&, int);
	void resolve(Tables*);
};

class SecondPassInstruction : public Instruction {
protected:
	int instructionLength;
	char isRegister(std::string&);
	virtual char getOperationCode() = 0;
	char getConditionCode();
	char getFlagsBit();
	virtual void getParameters(Tables*, char*) = 0;
public:
	SecondPassInstruction(std::string&, int);
	void resolve(Tables*);
};

class Int : public SecondPassInstruction {
protected:
	char getOperationCode();
	void getParameters(Tables*, char*);
public:
	Int(std::string&, int);
};

class ArithmeticAndCmp : public SecondPassInstruction {
private:
	std::string operation;
protected:
	char getOperationCode();
	void getParameters(Tables*, char*);
public:
	ArithmeticAndCmp(std::string&, int);
};

class Logical : public SecondPassInstruction {
protected:
	char getOperationCode();
	void getParameters(Tables*, char*);
public:
	Logical(std::string&, int);
};

class LdrStr : public SecondPassInstruction {
private:
	int isLoad, mode;
protected:
	virtual char getOperationCode();
	void getParameters(Tables*, char*);
public:
	LdrStr(std::string&, int);
};

class Push : public LdrStr {
protected:
	char getOperationCode();
public:
	Push(std::string&, int);
};

class Pop : public LdrStr {
protected:
	char getOperationCode();
public:
	Pop(std::string&, int);
};

class Call : public SecondPassInstruction {
protected:
	char getOperationCode();
	void getParameters(Tables*, char*);
public:
	Call(std::string&, int);
};

class InOut : public SecondPassInstruction {
private:
	int isIn;
protected:
	char getOperationCode();
	void getParameters(Tables*, char*);
public:
	InOut(std::string&, int);
};

class MovShrShl : public SecondPassInstruction {
private:
	int code, isL;
protected:
	virtual char getOperationCode();
	void getParameters(Tables*, char*);
public:
	MovShrShl(std::string&, int);
};

class Ret : public MovShrShl {
protected:
	char getOperationCode();
public:
	Ret(std::string&, int);
};

class Iret : public MovShrShl {
protected:
	char getOperationCode();
public:
	Iret(std::string&, int);
};

class Ldc : public SecondPassInstruction {
private:
	int isHigh;
	bool pseudo;
protected:
	char getOperationCode();
	void getParameters(Tables*, char*);
public:
	Ldc(std::string&, int);
};
#endif