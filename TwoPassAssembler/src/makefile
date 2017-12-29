#make file for TwoPassAssembler
objs = Assembler.o ContentTable.o Directive.o ErrorException.o Instruction.o Label.o main.o SymbolTable.o Tables.o Token.o TokenFactory.o Utilities.o

TwoPassAssembler: $(objs)
	g++-5 -std=c++11 -o TwoPassAssembler $(objs)

Assembler.o: Assembler.cpp Assembler.h
	g++-5 -std=c++11 -c Assembler.cpp

ContentTable.o: ContentTable.cpp ContentTable.h
	g++-5 -std=c++11 -c ContentTable.cpp

Directive.o: Directive.cpp Directive.h
	g++-5 -std=c++11 -c Directive.cpp

ErrorException.o: ErrorException.cpp ErrorException.h
	g++-5 -std=c++11 -c ErrorException.cpp

Instruction.o: Instruction.cpp Instruction.h
	g++-5 -std=c++11 -c Instruction.cpp

Label.o: Label.cpp Label.h
	g++-5 -std=c++11 -c Label.cpp

main.o: main.cpp
	g++-5 -std=c++11 -c main.cpp

SymbolTable.o: SymbolTable.cpp SymbolTable.h
	g++-5 -std=c++11 -c SymbolTable.cpp

Tables.o: Tables.cpp Tables.h
	g++-5 -std=c++11 -c Tables.cpp

Token.o: Token.cpp Token.h
	 g++-5 -std=c++11 -c Token.cpp

TokenFactory.o: TokenFactory.cpp TokenFactory.h
	g++-5 -std=c++11 -c TokenFactory.cpp

Utilities.o: Utilities.cpp Utilities.h
	g++-5 -std=c++11 -c Utilities.cpp

clean: TwoPassAssembler
	rm *.o

.PHONY: clean