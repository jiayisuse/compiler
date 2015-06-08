#include <string.h>

#include "process.h"
#include "io.h"
#include "operation.h"


#define LABEL_LEN	12

extern char token[NAME_MAX];

static int label_count = 0;

static void ident()
{
	char name[NAME_MAX];
	get_name(name);
	if (look == '(') {
		match('(');
		match(')');
		emit_n("CALL\t%s", name);
	} else {
		emit_n("MOV\t%s, %%edx", name);
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
	} else {
		char num_str[NUM_MAX];
		get_num(num_str);
		emit_n("MOV\t$%s, %%eax", num_str);
	}
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
	char name[NAME_MAX];
	get_name(name);
	match('=');
	expression();
	emit_n("MOV\t%s, %%edx", name);
	emit_n("MOV\t%%eax, (%%edx)");
}


char *new_label(char *label)
{
	sprintf(label, "L%03d", label_count++);
	return label;
}

void post_label(const char *label)
{
	printf("%s:\n", label);
}

void condition()
{
	emit_n("<condition>");
}

void do_if()
{
	char label_false[LABEL_LEN], label_end[LABEL_LEN];

	token_match("if");
	new_label(label_false);
	condition();
	emit_n("JEQ\t%s", label_false);
	block();
	if (strcmp(token, "else") == 0) {
		new_label(label_end);
		emit_n("JMP\t%s", label_end);
		post_label(label_false);
		block();
		strcpy(label_false, label_end);
	}
	token_match("endif");
	post_label(label_false);
}

void do_while()
{
	char label_loop[LABEL_LEN], label_end[LABEL_LEN];

	token_match("while");
	new_label(label_loop);
	new_label(label_end);
	post_label(label_loop);
	condition();
	emit_n("JEQ\t%s", label_end);
	block();
	token_match("endwhile");
	emit_n("JMP\t%s", label_loop);
	post_label(label_end);
}

void other()
{
	emit_n("%s", token);
}

void block()
{
	for (get_token();
			memcmp(token, "else", 4) &&
			memcmp(token, "endif", 6) &&
			memcmp(token, "endwhile", 6) &&
			memcmp(token, "end", 4);
			get_token()) {
		if (strcmp(token, "if") == 0)
			do_if();
		else if (strcmp(token, "while") == 0)
			do_while();
		else
			other();
	}
}

void do_program()
{
	block();
	token_match("end");
	emit_n("end");

}
