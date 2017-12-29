#include "Emulator.h"
#include "Memory.h"
#include <cstdio>
#include <ctime>

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

struct ThreadParameters {
	sem_t ioMutex, startSem;
	bool keyboardInterrupt;
	char **io;
};

void *ioDevice(void *parameters) {
	ThreadParameters *threadParameters = (ThreadParameters*) parameters;

	while(true) {
		sem_wait(&threadParameters->startSem);

		char c = 0;
		//std::cin >> c;
		c = getchar();

		sem_wait(&threadParameters->ioMutex);		
		write(threadParameters->io, 0x1000, c);
		unsigned int value = read(threadParameters->io, 0x1010);
		write(threadParameters->io, 0x1010, value | 0x200);
		sem_post(&threadParameters->ioMutex);

		threadParameters->keyboardInterrupt = true;
	}

	return nullptr;
}


ProcessorContext cpu;
extern ConditionFunction conditionFunction[8];
extern LazyFlags lazyFlags;

void emulate(char ****memory) {
	//BASIC STUFF
	bool exitRequested = false;
	cpu.reg[PC] = read(memory, 0);
	//IO SPACE
	char **io = nullptr;
	//INITIALIZE IO DEVICE STATUS REGISTER
	write(io, 0x1010, 0);
	//IO DEVICE(THREAD) INITIALIZATION
	ThreadParameters threadParameters;
	threadParameters.keyboardInterrupt = false;
	threadParameters.io = io;
	if(sem_init(&threadParameters.startSem, NULL, 1) != 0 && sem_init(&threadParameters.ioMutex, NULL, 1) != 0) {
		std::cout << "Could not intialize io device" << std::endl;
		return;
	}
	pthread_t device;
	if(pthread_create(&device, NULL, ioDevice, (void*)&threadParameters)) {
		std::cout << "Could not start device" << std::endl;
	}

	//TIMER
	double timeLeft = 1;
	//INVALID INSTRUCTION
	bool invalidInstruction = false;
	
	while (!exitRequested) {

		clock_t start = clock();

		unsigned int instruction = read(memory, cpu.reg[PC]);
		cpu.reg[PC] += 4;

		if (conditionFunction[CONDITION(instruction)]()) {
			
			switch (unsigned int operationCode = OPERATION_CODE(instruction)) {

			case INT: {
				int interruptNumber = INTERRUPT_NUMBER(instruction);
				
				if(interruptNumber < 0 || interruptNumber > 15) {
					//INVALID INSTRUCTION INTER
					invalidInstruction = true;
				} else  if (interruptNumber != 0) {
					//FILL FLAGS REGISTER
					fillFlags();
					//PUSH FLAGS REGISTER
					cpu.reg[SP] -= 4;
					write(memory, cpu.reg[SP] - 4, cpu.reg[PSW]);
					//PUSH LR
					cpu.reg[SP] -= 4;
					write(memory, cpu.reg[SP], cpu.reg[LR]);
					//SAVE PC IN LR AND WRITE INTERRUPT ROUTINE ADRESS IN PC
					cpu.reg[LR] = cpu.reg[PC];
					cpu.reg[PC] = read(memory, interruptNumber * 4);
					cpu.reg[PSW] &= ~I_FLAG;
				}
				else {
					exitRequested = true;
				}
				break;
			}

			case ADD: {
				int dst = ARITHMETIC_DESTINATION(instruction);
				if(dst > 18) {
					invalidInstruction = true;
					break;
				}

				int value = 0;
				if(ARITHMETIC_HAS_IMMEDIATE(instruction)) {
					value = ARITHMETIC_IMMEDIATE(instruction);
					value = SIGN_EXTENSION(value, 18);
				} else {
					int src = ARITHMETIC_SOURCE(instruction);
					if(src > 18) {
						invalidInstruction = true;
						break;
					}
					value = cpu.reg[src];
				}

				if (FLAGS_CHANGE(instruction)) {
					lazyFlags.firstOperand = cpu.reg[dst];
					lazyFlags.secondOperand = value;
					lazyFlags.result = (cpu.reg[dst] += lazyFlags.secondOperand);
					lazyFlags.instruction = ADD;
				}
				else {
					cpu.reg[dst] += value;
				}
				break;
			}

			case SUB: {
				int dst = ARITHMETIC_DESTINATION(instruction);
				if(dst > 18) {
					invalidInstruction = true;
					break;
				}
				int value = 0;
				if(ARITHMETIC_HAS_IMMEDIATE(instruction)) {
					value = ARITHMETIC_IMMEDIATE(instruction);
					value = SIGN_EXTENSION(value, 18);
				} else {
					int src = ARITHMETIC_SOURCE(instruction);
					if(src > 18) {
						invalidInstruction = true;
						break;
					}
					value = cpu.reg[src];
				}
				if (FLAGS_CHANGE(instruction)) {
					lazyFlags.firstOperand = cpu.reg[dst];
					lazyFlags.secondOperand = value;
					lazyFlags.result = (cpu.reg[dst] -= lazyFlags.secondOperand);
					lazyFlags.instruction = SUB;
				}
				else {
					cpu.reg[dst] -= value;
				}
				break;
			}

			case MUL: {
				int dst = ARITHMETIC_DESTINATION(instruction);
				if(dst > 16) {
					invalidInstruction = true;
					break;
				}

				int value = 0;
				if(ARITHMETIC_HAS_IMMEDIATE(instruction)) {
					value = ARITHMETIC_IMMEDIATE(instruction);
					value = SIGN_EXTENSION(value, 18);
				} else {
					int src = ARITHMETIC_SOURCE(instruction);
					if(src > 16) {
						invalidInstruction = true;
						break;
					}
					value = cpu.reg[src];
				}

				if (FLAGS_CHANGE(instruction)) {
					//SAVE C AND O FLAGS
					if (getCF()) {
						cpu.reg[PSW] |= C_FLAG;
					}
					else {
						cpu.reg[PSW] &= ~C_FLAG;
					}
					if (getOF()) {
						cpu.reg[PSW] |= O_FLAG;
					}
					else {
						cpu.reg[PSW] &= ~O_FLAG;
					}
					lazyFlags.result = (cpu.reg[dst] *= value);
					lazyFlags.instruction = MUL;
				}
				else {
					cpu.reg[dst] *= value;
				}
				break;
			}

			case DIV: {
				int dst = ARITHMETIC_DESTINATION(instruction);
				if(dst > 16) {
					invalidInstruction = true;
					break;
				}

				int value = 0;
				if(ARITHMETIC_HAS_IMMEDIATE(instruction)) {
					value = ARITHMETIC_IMMEDIATE(instruction);
					value = SIGN_EXTENSION(value, 18);
				} else {
					int src = ARITHMETIC_SOURCE(instruction);
					if(src > 16) {
						invalidInstruction = true;
						break;
					}
					value = cpu.reg[src];
				}

				if (FLAGS_CHANGE(instruction)) {
					//SAVE C AND O FLAGS
					if (getCF()) {
						cpu.reg[PSW] |= C_FLAG;
					}
					else {
						cpu.reg[PSW] &= ~C_FLAG;
					}
					if (getOF()) {
						cpu.reg[PSW] |= O_FLAG;
					}
					else {
						cpu.reg[PSW] &= ~O_FLAG;
					}
					lazyFlags.result = (cpu.reg[dst] /= value);
					lazyFlags.instruction = DIV;
				}
				else {
					cpu.reg[dst] /= value;
				}
				break;
			}

			case CMP: {
				int dst = ARITHMETIC_DESTINATION(instruction);
				if(dst > 16) {
					invalidInstruction = true;
					break;
				}

				int value = 0;
				if(ARITHMETIC_HAS_IMMEDIATE(instruction)) {
					value = ARITHMETIC_IMMEDIATE(instruction);
					value = SIGN_EXTENSION(value, 18);

				} else {
					int src = ARITHMETIC_SOURCE(instruction);
					if(src > 16) {
						invalidInstruction = true;
						break;
					}
					value = cpu.reg[src];
				}
				lazyFlags.firstOperand = cpu.reg[dst];
				lazyFlags.secondOperand = value;
				lazyFlags.result = lazyFlags.firstOperand - lazyFlags.secondOperand;
				lazyFlags.instruction = CMP;
				break;
			}

			case AND: {
				int dst = LOGICAL_DESTINATION(instruction);
				if(dst > 16 && dst != 18) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}
				int src = LOGICAL_SOURCE(instruction);
				if(src > 16 && src != 18) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}

				if (FLAGS_CHANGE(instruction)) {
					//SAVE C AND O FLAGS
					if (getCF()) {
						cpu.reg[PSW] |= C_FLAG;
					}
					else {
						cpu.reg[PSW] &= ~C_FLAG;
					}
					if (getOF()) {
						cpu.reg[PSW] |= O_FLAG;
					}
					else {
						cpu.reg[PSW] &= ~O_FLAG;
					}
					lazyFlags.result = (cpu.reg[dst] &= cpu.reg[src]);

					lazyFlags.instruction = AND;
				}
				else {
					cpu.reg[dst] &= cpu.reg[src];
				}
				break;
			}

			case OR: {
				int dst = LOGICAL_DESTINATION(instruction);
				if(dst > 16 && dst != 18) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}
				int src = LOGICAL_SOURCE(instruction);
				if(src > 16 && src != 18) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}

				if (FLAGS_CHANGE(instruction)) {
					//SAVE C AND O FLAGS
					if (getCF()) {
						cpu.reg[PSW] |= C_FLAG;
					}
					else {
						cpu.reg[PSW] &= ~C_FLAG;
					}
					if (getOF()) {
						cpu.reg[PSW] |= O_FLAG;
					}
					else {
						cpu.reg[PSW] &= ~O_FLAG;
					}
					lazyFlags.result = (cpu.reg[dst] |= cpu.reg[src]);

					lazyFlags.instruction = OR;
				}
				else {
					cpu.reg[dst] |= cpu.reg[src];
				}
				break;
			}

			case NOT: {
				int dst = LOGICAL_DESTINATION(instruction);
				if(dst > 16 && dst != 18) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}
				int src = LOGICAL_SOURCE(instruction);
				if(src > 16 && src != 18) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}

				if (FLAGS_CHANGE(instruction)) {
					//SAVE C AND O FLAGS
					if (getCF()) {
						cpu.reg[PSW] |= C_FLAG;
					}
					else {
						cpu.reg[PSW] &= ~C_FLAG;
					}
					if (getOF()) {
						cpu.reg[PSW] |= O_FLAG;
					}
					else {
						cpu.reg[PSW] &= ~O_FLAG;
					}
					lazyFlags.result = (cpu.reg[dst] = ~cpu.reg[src]);

					lazyFlags.instruction = NOT;
				}
				else {
					cpu.reg[dst] = ~cpu.reg[src];
				}
				break;
			}

			case TEST: {
				int dst = LOGICAL_DESTINATION(instruction);
				if(dst > 16 && dst != 18) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}
				int src = LOGICAL_SOURCE(instruction);
				if(src > 16 && src != 18) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}
				//SAVE C AND O FLAGS
				if (getCF()) {
					cpu.reg[PSW] |= C_FLAG;
				}
				else {
					cpu.reg[PSW] &= ~C_FLAG;
				}
				if (getOF()) {
					cpu.reg[PSW] |= O_FLAG;
				}
				else {
					cpu.reg[PSW] &= ~O_FLAG;
				}

				lazyFlags.result = cpu.reg[dst] & cpu.reg[src];

				lazyFlags.instruction = TEST;
				break;
			}

			case LDRSTR: {
				int addressMode = MEMORY_ACCESS_ADDRESS_MODE(instruction);
				int base = MEMORY_ACCESS_BASE(instruction);
				if((addressMode != 0 && base == PC) || base > 18) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}
				int dst = MEMORY_ACCESS_DESTINATION(instruction);
				if(dst > 19) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}
				int displacement = MEMORY_ACCESS_DISPLACEMENT(instruction);
				displacement = SIGN_EXTENSION(displacement, 10);
				//CHECK ADDRESS MODE
				if (addressMode == 4) {
					cpu.reg[base] += 4;
				}
				else if (addressMode == 5) {
					cpu.reg[base] -= 4;
				}
				else if (addressMode == 0) {
					displacement += 4;
				}
				//MEMORY ACCESS
				if (MEMORY_ACCESS_IS_LOAD(instruction)) {
					cpu.reg[dst] = read(memory, cpu.reg[base] + SIGN_EXTENSION(displacement, 10));
				}
				else {
					write(memory, cpu.reg[base] + SIGN_EXTENSION(displacement, 10), cpu.reg[dst]);
				}
				//CHECK ADDRESS MODE
				if(addressMode == 2) {
					cpu.reg[base] += 4;
				}
				else if (addressMode == 3) {
					cpu.reg[base] -= 4;
				}

				break;
			}

			case CALL: {
				cpu.reg[LR] = cpu.reg[PC];
				int base = CALL_BASE(instruction);
				if(base > 19) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}
				int displacement = CALL_DISPLACEMENT(instruction);
				displacement = SIGN_EXTENSION(displacement, 19);
				int address = cpu.reg[base] + SIGN_EXTENSION(displacement, 19);
				if (base == PC) {
					address += 4;
				}

				cpu.reg[PC] = address;

				break;
			}

			case INOUT: {
				unsigned int address = cpu.reg[IO_SOURCE(instruction)];
				if (IO_IS_IN(instruction)) {
					if(address == 0x1000 || address == 0x1010) {
						sem_wait(&threadParameters.ioMutex);
					}

					cpu.reg[IO_DESTINATION(instruction)] = read(io, address);
					
					if(address == 0x1000 || address == 0x1010) {
						sem_post(&threadParameters.ioMutex);
					}
				}
				else {
					if (address == 0x2000) {
						putchar(cpu.reg[IO_DESTINATION(instruction)] & BYTE_MASK);
					}
					else {
						if(address == 0x1000 || address == 0x1010) {
							sem_wait(&threadParameters.ioMutex);
						}

						write(io, address, cpu.reg[IO_DESTINATION(instruction)]);
						//IF WE WRITE 0 IN STATUS REGISTER
						if(address == 0x1010 && (cpu.reg[IO_DESTINATION(instruction)] & 0x200) == 0) {
							sem_post(&threadParameters.startSem);
						}

						if(address == 0x1000 || address == 0x1010) {
							sem_post(&threadParameters.ioMutex);
						}
					}
				}
				break;
			}

			case MOV: {
				int dst = MOV_DESTINATION(instruction);
				if(dst > 19) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}
				int src = MOV_SOURCE(instruction);
				if(src > 19) {
					//INVALID INSTRUCTION
					invalidInstruction = true;
					break;
				}
				if (FLAGS_CHANGE(instruction)) {
					int immediate = MOV_IMMEDIATE(instruction);
					//CHECK IF IRET
					if (immediate == 0 && dst == PC) {
						cpu.reg[PC] = cpu.reg[src];
						cpu.reg[LR] = read(memory, cpu.reg[SP]);
						cpu.reg[SP] += 4;
						cpu.reg[PSW] = read(memory, cpu.reg[SP]);
						cpu.reg[SP] += 4;
						lazyFlags.instruction = UNKNOWN;
					}
					else {
						if (getOF()) {
							cpu.reg[PSW] |= O_FLAG;
						}
						else {
							cpu.reg[PSW] &= ~O_FLAG;
						}
						lazyFlags.firstOperand = cpu.reg[src];
						lazyFlags.secondOperand = immediate;
						if (MOV_IS_LEFT(instruction)) {
							lazyFlags.result = (cpu.reg[dst] = lazyFlags.firstOperand << lazyFlags.secondOperand);
							lazyFlags.instruction = SHL;
						}
						else {
							lazyFlags.result = (cpu.reg[dst] = lazyFlags.firstOperand >> lazyFlags.secondOperand);
							lazyFlags.instruction = SHR;
						}
					}
				}
				else {
					cpu.reg[dst] = MOV_IS_LEFT(instruction) ? cpu.reg[src] << MOV_IMMEDIATE(instruction) : cpu.reg[src] >> MOV_IMMEDIATE(instruction);
				}
				break;
			}
			
			case LDC: {
				int dst = LDC_DESTINATION(instruction);
				if (LDC_IS_HIGH(instruction)) {
					cpu.reg[dst] &= 0x0000FFFF;
					cpu.reg[dst] |= LDC_C(instruction) << 16;
				}
				else {
					cpu.reg[dst] &= 0xFFFF0000;
					cpu.reg[dst] |= LDC_C(instruction);
				}
				break;
			}
					  
			default: {
				invalidInstruction = true;
				break;
			}

			}

			clock_t end = clock();
			timeLeft -= (end - start) / (double)CLOCKS_PER_SEC;

			if (invalidInstruction) {
				invalidInstruction = false;
				//FILL FLAGS REGISTER
				fillFlags();
				//PUSH FLAGS REGISTER
				cpu.reg[SP] -= 4;
				write(memory, cpu.reg[SP], cpu.reg[PSW]);
				//PUSH LR
				cpu.reg[SP] -= 4;
				write(memory, cpu.reg[SP], cpu.reg[LR]);
				//SAVE PC IN LR AND WRITE INTERRUPT ROUTINE ADRESS IN PC
				cpu.reg[LR] = cpu.reg[PC];
				cpu.reg[PC] = read(memory, 8);
				cpu.reg[PSW] &= ~I_FLAG;
			}
			else if (timeLeft <= 0) {
				timeLeft += 1;
				if (cpu.reg[PSW] & T_FLAG) {
					//FILL FLAGS REGISTER
					fillFlags();
					//PUSH FLAGS REGISTER
					cpu.reg[SP] -= 4;
					write(memory, cpu.reg[SP], cpu.reg[PSW]);
					//PUSH LR
					cpu.reg[SP] -= 4;
					write(memory, cpu.reg[SP], cpu.reg[LR]);
					//SAVE PC IN LR AND WRITE INTERRUPT ROUTINE ADRESS IN PC
					cpu.reg[LR] = cpu.reg[PC];
					cpu.reg[PC] = read(memory, 4);
					cpu.reg[PSW] &= ~I_FLAG;
				}
			} else if(threadParameters.keyboardInterrupt && (cpu.reg[PSW] & I_FLAG)) {
					threadParameters.keyboardInterrupt = false;
					//FILL FLAGS REGISTER
					fillFlags();
					//PUSH FLAGS REGISTER
					cpu.reg[SP] -= 4;
					write(memory, cpu.reg[SP], cpu.reg[PSW]);
					//PUSH LR
					cpu.reg[SP] -= 4;
					write(memory, cpu.reg[SP], cpu.reg[LR]);
					//SAVE PC IN LR AND WRITE INTERRUPT ROUTINE ADRESS IN PC
					cpu.reg[LR] = cpu.reg[PC];
					cpu.reg[PC] = read(memory, 12);
					cpu.reg[PSW] &= ~I_FLAG;
			}
		}
	}

	pthread_cancel(device);
	pthread_join(device, NULL);
	sem_destroy(&threadParameters.startSem);
	sem_destroy(&threadParameters.ioMutex);

	deleteMemory(io);
}