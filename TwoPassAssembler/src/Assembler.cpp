#include "Assembler.h"
#include "ErrorException.h"
#include "TokenFactory.h"
#include <string>
#include <iostream>

int assembler::assemble(std::string& input, std::string& output) {
	int returnValue = 0;
	TokenFactory *factory = nullptr;
	Token *token = nullptr;
	Tables tables;
	try {
		for (int i = 0; i < 2; i++) {
			token = nullptr;
			if (i == 0) {
				factory = new FirstPassTokenFactory(input); 
			}
			else {
				factory = new SecondPassTokenFactory(input);
			}

			while ((token = factory->nextToken()) != nullptr) {
				token->resolve(&tables);
				delete token;
				token = nullptr;
			}
			delete factory;
		}
		
		tables.writeIntoFile(output);
	}
	catch (ErrorException& e) {
		if (token != nullptr) {
			delete token;
		}
		delete factory;
		std::cout << e << std::endl;
		returnValue = 1;
	}

	return returnValue;
}