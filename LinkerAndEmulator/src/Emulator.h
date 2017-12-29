#ifndef _EMULATOR_H_
#define _EMULATOR_H_

enum INSTRUCTION {INT, ADD, SUB, MUL, DIV, CMP, AND, OR, NOT, TEST, LDRSTR, CALL = 12, INOUT, MOV, LDC, SHR, SHL, UNKNOWN};

#define NUMBER_OF_REGISTERS 20
#define PC 16
#define LR 17
#define SP 18
#define PSW 19

struct ProcessorContext {
	unsigned int reg[NUMBER_OF_REGISTERS];
};

#define Z_FLAG 0x00000001
#define N_FLAG 0x00000002
#define C_FLAG 0x00000004
#define O_FLAG 0x00000008
#define T_FLAG 0x40000000
#define I_FLAG 0x80000000
#define MOST_SIGNIFICANT_BIT_MASK 0x80000000
#define LEAST_SIGNIFICANT_BIT_MASK 0x1
#define BYTE_MASK 0xFF


struct LazyFlags {
	unsigned int firstOperand, secondOperand, result;
	INSTRUCTION instruction;
};

#define DIMENSION 0x100

typedef int (*ConditionFunction)();
int getCF();
int getOF();
void fillFlags();

void emulate(char****);

#define BITS1 0x1
#define BITS3 0x7
#define BITS4 0xF
#define BITS5 0x1F
#define BITS10 0x3FF
#define BITS16 0xFFFF
#define BITS18 0x3FFFF
#define BITS19 0x7FFFF

#define EXTRACT(instruction, position, numOfBits) ((instruction >> position) & BITS##numOfBits)

#define CONDITION(instruction) EXTRACT(instruction, 29, 3)
#define OPERATION_CODE(instruction) EXTRACT(instruction, 24, 4)
#define FLAGS_CHANGE(instruction) EXTRACT(instruction, 28, 1)

#define INTERRUPT_NUMBER(instruction) EXTRACT(instruction, 20, 4)

#define ARITHMETIC_HAS_IMMEDIATE(instruction) EXTRACT(instruction, 18, 1)
#define ARITHMETIC_DESTINATION(instruction) EXTRACT(instruction, 19, 5)
#define ARITHMETIC_SOURCE(instruction) EXTRACT(instruction, 13, 5)
#define ARITHMETIC_IMMEDIATE(instruction) EXTRACT(instruction, 0, 18)

#define LOGICAL_DESTINATION(instruction) EXTRACT(instruction, 19, 5)
#define LOGICAL_SOURCE(instruction) EXTRACT(instruction, 14, 5)

#define MEMORY_ACCESS_BASE(instruction) EXTRACT(instruction, 19, 5)
#define MEMORY_ACCESS_DESTINATION(instruction) EXTRACT(instruction, 14, 5)
#define MEMORY_ACCESS_ADDRESS_MODE(instruction) EXTRACT(instruction, 11, 3)
#define MEMORY_ACCESS_IS_LOAD(instruction) EXTRACT(instruction, 10, 1)
#define MEMORY_ACCESS_DISPLACEMENT(instruction) EXTRACT(instruction, 0, 10)

#define CALL_BASE(instruction) EXTRACT(instruction, 19, 5)
#define CALL_DISPLACEMENT(instruction) EXTRACT(instruction, 0, 19)

#define IO_DESTINATION(instruction) EXTRACT(instruction, 20, 4)
#define IO_SOURCE(instruction) EXTRACT(instruction, 16, 4)
#define IO_IS_IN(instruction) EXTRACT(instruction, 15, 1)

#define MOV_DESTINATION(instruction) EXTRACT(instruction, 19, 5)
#define MOV_SOURCE(instruction) EXTRACT(instruction, 14, 5)
#define MOV_IMMEDIATE(instruction) EXTRACT(instruction, 9, 5)
#define MOV_IS_LEFT(instruction) EXTRACT(instruction, 8, 1)

#define LDC_DESTINATION(instruction) EXTRACT(instruction, 20, 4)
#define LDC_IS_HIGH(instruction) EXTRACT(instruction, 19, 1)
#define LDC_C(instruction) EXTRACT(instruction, 0, 16)

#define SIGN10 0x200
#define SIGN18 0x20000
#define SIGN19 0x40000

#define NEG10 0xFFFFFC00
#define NEG18 0xFFFC0000
#define NEG19 0xFFF80000

#define SIGN_EXTENSION(value, signBit) ((value & SIGN##signBit) ? (value | NEG##signBit) : value)
#endif