#include "Emulator.h"

LazyFlags lazyFlags;

extern ProcessorContext cpu;

int getZF() {
	switch (lazyFlags.instruction) {
	case UNKNOWN:
		return cpu.reg[PSW] & Z_FLAG;
	default:
		return lazyFlags.result == 0;
	}
}

int getNF() {
	switch (lazyFlags.instruction) {
	case UNKNOWN:
		return cpu.reg[PSW] & N_FLAG;
	default:
		return lazyFlags.result & MOST_SIGNIFICANT_BIT_MASK;
	}
}

int getCF() {
	switch (lazyFlags.instruction)
	{
	case ADD:
		return lazyFlags.result < lazyFlags.firstOperand;
	case SUB:
	case CMP:
		return lazyFlags.firstOperand < lazyFlags.secondOperand;
	case MOV:
	case SHR:
		return (lazyFlags.firstOperand >> (lazyFlags.secondOperand - 1)) & LEAST_SIGNIFICANT_BIT_MASK;
	case SHL:
		return (lazyFlags.firstOperand >> (32 - lazyFlags.secondOperand - 1)) & LEAST_SIGNIFICANT_BIT_MASK;
	case UNKNOWN:
	case MUL:
	case DIV:
	case AND:
	case OR:
	case NOT:
	case TEST:
		return cpu.reg[PSW] & C_FLAG;
	}
}

int getOF() {
	switch (lazyFlags.instruction) {
	case ADD:
		return ((lazyFlags.firstOperand ^ lazyFlags.result) & MOST_SIGNIFICANT_BIT_MASK) & ((lazyFlags.secondOperand ^ lazyFlags.result) & MOST_SIGNIFICANT_BIT_MASK);
	case SUB:
	case CMP:
		((lazyFlags.firstOperand ^ lazyFlags.result) & MOST_SIGNIFICANT_BIT_MASK) & lazyFlags.result & lazyFlags.secondOperand & MOST_SIGNIFICANT_BIT_MASK;
	case UNKNOWN:
	case MUL:
	case DIV:
	case AND:
	case OR:
	case NOT:
	case TEST:
	case SHR:
	case SHL:
	case MOV:
		return cpu.reg[PSW] & O_FLAG;
	}
}


int eq() {
	return getZF() == 1;
}

int ne() {
	return getZF() == 0;
}

int gt() {
	return (getZF() == 0) && (getNF() == getOF());
}

int ge() {
	return getNF() == getOF();
}

int lt() {
	return getNF() != getOF();
}

int le() {
	return (getZF() == 1) || (getNF() != getOF());
}

int al() {
	return 1;
}

ConditionFunction conditionFunction[8] = { eq, ne, gt, ge, lt, le, al, al };

void fillFlags() {
	if (getZF()) {
		cpu.reg[PSW] |= Z_FLAG;
	}
	if (getNF()) {
		cpu.reg[PSW] |= N_FLAG;
	}
	if (getCF()) {
		cpu.reg[PSW] |= C_FLAG;
	}
	if (getOF()) {
		cpu.reg[PSW] |= O_FLAG;
	}
}