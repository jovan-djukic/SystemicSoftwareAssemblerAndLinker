#include "Memory.h"
#include "ErrorException.h"

void checkIfAllocated(char ****&memory, int first, int second, int third) {
	if (memory == nullptr) {
		memory = new char***[DIMENSION];
		for (int i = 0; i < DIMENSION; i++) {
			memory[i] = nullptr;
		}
	}
	if (memory[first] == nullptr) {
		memory[first] = new char**[DIMENSION];
		for (int i = 0; i < DIMENSION; i++) {
			memory[first][i] = nullptr;
		}
	}
	if (memory[first][second] == nullptr) {
		memory[first][second] = new char*[DIMENSION];
		for (int i = 0; i < DIMENSION; i++) {
			memory[first][second][i] = nullptr;
		}
	}
	if (memory[first][second][third] == nullptr) {
		memory[first][second][third] = new char[DIMENSION];
	}
}

void checkIfAllocated(char **&io, int first) {
	if (io == nullptr) {
		io = new char*[DIMENSION];
		for (int i = 0; i < DIMENSION; i++) {
			io[i] = nullptr;
		}
	}
	if (io[first] == nullptr) {
		io[first] = new char[DIMENSION];
	}
}

void incrementAddress(int& first, int& second, int& third, int& fourth, int numOfBytes) {
	fourth += numOfBytes;
	if (fourth == DIMENSION) {
		fourth = 0;
		third++;
		if (third == DIMENSION) {
			third = 0;
			second++;
			if (second == DIMENSION) {
				second = 0;
				first++;
				if (first == DIMENSION) {
					std::string errorMessage = std::string("Program is too big, can't load");
					throw ErrorException(errorMessage);
				}
			}
		}
	}
}

void incrementAddress(int& first, int& second, int numBytes) {
	second += numBytes;
	if (second == DIMENSION) {
		second = 0;
		first++;
		if (first == DIMENSION) {
			std::string errorMessage = std::string("Program is too big, can't load");
			throw ErrorException(errorMessage);
		}
	}
}

void write(char ****&memory, unsigned int address, char *buffer, int length) {
	//int first = (address >> 24) & MASK;
	//int second = (address >> 16) & MASK;
	//int third = (address >> 8) & MASK;
	//int fourth = address & MASK;

	//int position = 0;

	//while (length > 0) {
	//	//ALLOCATE IF NEEDED
	//	checkIfAllocated(memory, first, second, third, fourth);
	//	//COPY BYTES
	//	int numOfBytes = length > DIMENSION ? DIMENSION - fourth : length;
	//	memcpy(memory[first][second][third] + fourth, buffer + position, numOfBytes);
	//	//INCREMENT CURRENT POSITION
	//	position += numOfBytes;
	//	length -= numOfBytes;
	//	//CALCULATE NEW ADRESS
	//	incrementAddress(first, second, third, fourth, numOfBytes);
	//}

	for (int i = 0; i < length; i++) {
		int first = (address >> 24) & MASK;
		int second = (address >> 16) & MASK;
		int third = (address >> 8) & MASK;
		int fourth = address & MASK;

		checkIfAllocated(memory, first, second, third);
		memory[first][second][third][fourth] = buffer[i];

		address++;
	}
}

void write(char ****&memory, unsigned int address, unsigned int value) {
	int first = (address >> 24) & MASK;
	int second = (address >> 16) & MASK;
	int third = (address >> 8) & MASK;
	int fourth = address & MASK;

	checkIfAllocated(memory, first, second, third);

	for (int i = 0; i < 4; i++) {
		int newFourth = address & MASK;

		if (newFourth < fourth) {
			fourth = newFourth;
			incrementAddress(first, second, third, fourth, 0);
			checkIfAllocated(memory, first, second, third);
		}

		memory[first][second][third][newFourth] = (value >> (i * 8)) & MASK;
		
		address++;
	}

}

void write(char **&io, unsigned int address, unsigned int value) {
	int first = (address >> 8) & MASK;
	int second = address & MASK;

	checkIfAllocated(io, first);

	for (int i = 0; i < 4; i++) {
		int newSecond = address & MASK;

		if (newSecond < second) {
			second = newSecond;
			incrementAddress(first, second, 0);
			checkIfAllocated(io, first);
		}

		io[first][newSecond] = (value >> (i * 8)) & MASK;

		address++;
	}
}

unsigned int read(char ****&memory, unsigned int address) {
	int first = (address >> 24) & MASK;
	int second = (address >> 16) & MASK;
	int third = (address >> 8) & MASK;
	int fourth = address & MASK;

	unsigned int value = 0;

	checkIfAllocated(memory, first, second, third);

	for (int i = 0; i < 4; i++) {
		int newFourth = address & MASK;

		if (newFourth < fourth) {
			fourth = newFourth;
			incrementAddress(first, second, third, fourth, 0);
			checkIfAllocated(memory, first, second, third);
		}

		value |= (memory[first][second][third][newFourth] << (i * 8)) & (MASK << (i * 8));

		address++;
	}

	return value;
}

unsigned int read(char **&io, unsigned int address) {
	int first = (address >> 8) & MASK;
	int second = address & MASK;

	unsigned int value = 0;

	checkIfAllocated(io, first);

	for (int i = 0; i < 4; i++) {
		int newSecond = address & MASK;

		if (newSecond < second) {
			second = newSecond;
			incrementAddress(first, second, 0);
			checkIfAllocated(io, first);
		}

		value |= (io[first][newSecond] << (i * 8)) & (MASK << (i * 8));

		address++;
	}

	return value;
}

void deleteMemory(char ****memory) {
	if (memory != nullptr) {
		for (int first = 0; first < DIMENSION; first++) {
			for (int second = 0; second < DIMENSION; second++) {
				for (int third = 0; third < DIMENSION; third++) {
					if (memory[first][second][third] != nullptr) {
						delete[] memory[first][second][third];
					}
				}
				delete[] memory[first][second];
			}
			delete[] memory[first];
		}
		delete[] memory;
	}
}

void deleteMemory(char **io) {
	if (io != nullptr) {
		for (int first = 0; first < DIMENSION; first++) {
			if (io[first] != nullptr) {
				delete[] io[first];
			}
		}
		delete[] io;
	}
}