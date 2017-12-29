#ifndef _DIRECTIVE_H_
#define _DIRECTIVE_H_

#include "Tables.h"
#include "Token.h"
#include <string>

class Directive : public Token {
public:
	Directive(int);
	virtual ~Directive();
};

class Section : public Directive {
protected:
	std::string section;
public:
	Section(std::string&, int);
};

class FirstPassSection : public Section {
public:
	FirstPassSection(std::string&, int);
	void resolve(Tables*);
};

class SecondPassSection : public Section {
public:
	SecondPassSection(std::string&, int);
	void resolve(Tables*);
};

class MemoryConsuming : public Directive {
private:
	std::string directive;
public:
	MemoryConsuming(std::string&, int);
	void resolve(Tables*);
};

class Skip : public Directive {
protected:
	std::string directive;
public:
	Skip(std::string&, int);
	virtual ~Skip();
};

class FirstPassSkip : public Skip {
public:
	FirstPassSkip(std::string&, int);
	void resolve(Tables*);
};

class SecondPassSkip : public Skip {
public:
	SecondPassSkip(std::string&, int);
	void resolve(Tables*);
};

class MemoryAllocation : public Directive {
private:
	int size;
	std::string initializators;
public:
	MemoryAllocation(std::string&, int);
	void resolve(Tables*);
};

class Align : public Directive {
protected:
	std::string directive;
public:
	Align(std::string&, int);
};

class FirstPassAlign : public Align {
public:
	FirstPassAlign(std::string&, int);
	void resolve(Tables*);
};

class SecondPassAlign : public Align {
public:
	SecondPassAlign(std::string&, int);
	void resolve(Tables*);
};

class ScopeChange : public Directive {
private:
	std::string directive;
protected:
	virtual void changeScope(Tables*, std::string&) = 0;
public:
	ScopeChange(std::string&, int);
	void resolve(Tables*);
};

class Public : public ScopeChange {
protected:
	void changeScope(Tables*, std::string&);
public:
	Public(std::string&, int);
};

class Extern : public ScopeChange {
protected:
	void changeScope(Tables*, std::string&);
public:
	Extern(std::string&, int);
};

class End : public Directive {
private:
	std::string directive;
public:
	End(std::string&, int);
	void resolve(Tables*);
};

class Asciiz : public Directive {
protected:
	std::string text;
public:
	Asciiz(std::string&, int);
};

class FirstPassAsciiz : public Asciiz {
public:
	FirstPassAsciiz(std::string&, int);
	void resolve(Tables*);
};

class SecondPassAsciiz : public Asciiz {
public:
	SecondPassAsciiz(std::string&, int);
	void resolve(Tables*);
};

#endif