#include <string.h>

#include "process.h"
#include "io.h"
#include "operation.h"


#define LABEL_LEN	12
#define LINE_ENDING	';'

extern char token[NAME_MAX];
extern char pre_token[NAME_MAX];

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
	get_token();
	token_match("(");
	for (get_token(); memcmp(token, ")", 2) != 0; get_token()) {
		emit_n("%s", token);
	}
	emit_n("<condition>");
}

void do_if()
{
	char *token_forward;
	char label_false[LABEL_LEN], label_end[LABEL_LEN];
	char block_ending = LINE_ENDING;

	token_match("if");

	new_label(label_false);
	condition();

	if (look == '{') {
		get_token();
		block_ending = '}';
	}

	emit_n("JEQ\t%s", label_false);
	block(block_ending);

	token_forward = look_forward();
	if (strcmp(token_forward, "else") == 0) {
		step_forward();
		if (look == '{') {
			get_token();
			block_ending = '}';
		} else
			block_ending = LINE_ENDING;
		new_label(label_end);
		emit_n("JMP\t%s", label_end);
		post_label(label_false);
		block(block_ending);
		strcpy(label_false, label_end);
	}
	token_match_char(block_ending);
	post_label(label_false);
}

void do_while()
{
	char label_loop[LABEL_LEN], label_end[LABEL_LEN];
	char block_ending = LINE_ENDING;

	token_match("while");
	new_label(label_loop);
	new_label(label_end);
	post_label(label_loop);
	condition();

	if (look == '{') {
		get_token();
		block_ending = '}';
	}

	emit_n("JEQ\t%s", label_end);
	block(block_ending);
	token_match_char(block_ending);
	emit_n("JMP\t%s", label_loop);
	post_label(label_end);
}

void other()
{
	emit_n("%s", token);
}

void block(char ending)
{
	for (get_token(); token[0] != ending && look != EOF; get_token()) {
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
	while (look != EOF)
		block(LINE_ENDING);
}
