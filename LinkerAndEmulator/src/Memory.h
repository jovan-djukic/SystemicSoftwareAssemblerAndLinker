#ifndef _MEMORY_H_
#define _MEMORY_H_

#define DIMENSION 0x100
#define MASK 0xFF

unsigned int read(char****&, unsigned int);
unsigned int read(char**&, unsigned int);
void write(char****&, unsigned int, unsigned int);
void write(char**&, unsigned int, unsigned int);
void write(char****&, unsigned int, char*, int);
void deleteMemory(char****);
void deleteMemory(char**);

#endif