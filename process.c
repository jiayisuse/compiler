#include "process.h"
#include "cradle.h"
#include "operation.h"

/* parse and translate a math factor */
void factor()
{
	emit_n("MOV\t$%c, %%eax", get_num());
}

/* parse and translate a math expression */
void term()
{
	factor();
	while (is_mulop(look)) {
		emit_n("PUSH\t%%eax");
		switch (look) {
		case '*':
			multiply();
			break;
		case '/':
			divide();
			break;
		default:
			expected("mul/div operation");
		}
	}
}


/* parse and translate an expression */
void expression()
{
	term();
	while (is_addop(look)) {
		emit_n("PUSH\t%%eax");
		switch (look) {
		case '+':
			add();
			break;
		case '-':
			sub();
			break;
		default:
			expected("add/sub operation");
		}
	}
}
