#include "Linker.h"
#include "ErrorException.h"
#include "Emulator.h"
#include "Memory.h"

int main(int argc, const char **argv) {
	char ****memory = nullptr;
	try {
		char ****memory = linker::link(argc, argv);
		emulate(memory);
	}
	catch (ErrorException& e) {
		std::cout << e << std::endl;
	}
	deleteMemory(memory);
	return 0;
}