#include "operation.h"
#include "io.h"
#include "process.h"

/* recognize and translate ADD operation */
void add()
{
	match('+');
	term();
	emit_n("POP\t%%ebx");
	emit_n("ADD\t%%ebx, %%eax");
}

/* recognize and translate SUB operation */
void sub()
{
	match('-');
	term();
	emit_n("POP\t%%ebx");
	emit_n("SUB\t%%eax, %%ebx");
	emit_n("MOV\t%%ebx, %%eax");
	/*
	emit_n("SUB\t%%ebx, %%eax");
	emit_n("NEG\t%%eax");	// use NEG instruction to inverse sign
	*/
}

void multiply()
{
	match('*');
	factor();
	emit_n("POP\t%%ebx");
	emit_n("MUL\t%%ebx");
}

void divide()
{
	match('/');
	factor();
	emit_n("MOV\t%%eax, %%ebx");
	emit_n("POP\t%%eax");
	emit_n("XOR\t%%edx, %%edx");
	emit_n("DIV\t%%ebx");
}
