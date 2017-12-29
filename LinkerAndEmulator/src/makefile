#makefile for LinkerAndEmulator
objs = Emulator.o ErrorException.o FlagsAndConditions.o GlobalSymbolTable.o Linker.o main.o Memory.o ScriptParser.o SectionList.o SymbolTables.o Utilities.o

LinkerAndEmulator: $(objs)
	g++-5 -std=c++11 -o LinkerAndEmulator $(objs) -lpthread

Emulator.o: Emulator.cpp Emulator.h
	g++-5 -std=c++11 -c Emulator.cpp 

ErrorException.o: ErrorException.cpp ErrorException.h
	g++-5 -std=c++11 -c ErrorException.cpp

FlagsAndConditions.o: FlagsAndConditions.cpp
	g++-5 -std=c++11 -c FlagsAndConditions.cpp

GlobalSymbolTable.o: GlobalSymbolTable.cpp GlobalSymbolTable.h
	g++-5 -std=c++11 -c GlobalSymbolTable.cpp

Linker.o: Linker.cpp Linker.h
	g++-5 -std=c++11 -c Linker.cpp

main.o: main.cpp
	g++-5 -std=c++11 -c main.cpp

Memory.o: Memory.cpp Memory.h
	g++-5 -std=c++11 -c Memory.cpp

ScriptParser.o: ScriptParser.cpp ScriptParser.h
	g++-5 -std=c++11 -c ScriptParser.cpp

SectionList.o: SectionList.cpp SectionList.h
	g++-5 -std=c++11 -c SectionList.cpp

SymbolTables.o: SymbolTables.cpp SymbolTables.h
	g++-5 -std=c++11 -c SymbolTables.cpp

Utilities.o: Utilities.cpp Utilities.h
	g++-5 -std=c++11 -c Utilities.cpp

.PHONY: clean

clean: 
	rm *.o