#include "process.h"
#include "io.h"
#include "operation.h"

static void ident()
{
	char name = get_name();
	if (look == '(') {
		match('(');
		match(')');
		emit_n("CALL\t%c", name);
	} else {
		emit_n("MOV\t%c, %%edx", name);
		emit_n("MOV\t(%%edx), %%eax");
	}
}

/* parse and translate a math factor */
void factor()
{
	if (look == '(') {
		match('(');
		expression();
		match(')');
	} else if (isalpha(look)) {
		ident();
	} else
		emit_n("MOV\t$%c, %%eax", get_num());
}

/* parse and translate a mul/div expression */
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

/* parse and translate an math expression */
void expression()
{
	if (is_addop(look))
		emit_n("XOR\t%%eax, %%eax");
	else
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

/* parse and translate an assignment experssion */
void assignment()
{
	char name = get_name();
	match('=');
	expression();
	emit_n("MOV\t%c, %%edx", name);
	emit_n("MOV\t%%eax, (%%edx)");
}
