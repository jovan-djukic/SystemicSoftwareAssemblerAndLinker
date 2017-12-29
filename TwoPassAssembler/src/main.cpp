#include "Assembler.h"
#include <string>
#include <iostream>

int main(int argc, const char **argv) {
	int retValue = 0;
	if (argc != 3) {
		std::cout << "Wrong number of parameters" << std::endl;
		retValue = 1;
	}
	else {
		std::string input = std::string(argv[1]);
		std::string output = std::string(argv[2]);
		retValue = assembler::assemble(input, output);
	}
	return retValue;
}