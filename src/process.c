#include <string.h>

#include "process.h"
#include "io.h"
#include "operation.h"


#define LABEL_LEN	12
#define LINE_ENDING	';'

#define TMP_FILE	".tmp.parser"

extern char token[NAME_MAX];
extern char next_token[NAME_MAX];

static int label_count = 0;
static int for_file_count = 0;

static void ident()
{
	char name[NAME_MAX];
	get_name(name);
	if (look == '(') {
		match('(');
		match(')');
		emit_n("CALL\t%s", name);
	} else {
		emit_n("LEAL\t%s, %%edx", name);
		emit_n("MOVL\t(%%edx), %%eax");
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
		emit_n("MOVL\t$%s, %%eax", num_str);
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
	emit_n("LEAL\t%s, %%edx", name);
	emit_n("MOVL\t%%eax, (%%edx)");
}


/*
 * boolean processing
 */

static void is_equal()
{
	expression();
	emit_n("POP\t%%ebx");
	emit_n("CMP\t%%eax, %%ebx");
	emit_n("SETE\t%%al");
	emit_n("MOVSX\t%%al, %%eax");
}

static void is_notequal()
{
	expression();
	emit_n("POP\t%%ebx");
	emit_n("CMP\t%%eax, %%ebx");
	emit_n("SETNE\t%%al");
	emit_n("MOVSX\t%%al, %%eax");
}

static void is_greater()
{
	expression();
	emit_n("POP\t%%ebx");
	emit_n("CMP\t%%eax, %%ebx");
	emit_n("SETG\t%%al");
	emit_n("MOVSX\t%%al, %%eax");
}

static void is_greater_or_equal()
{
	expression();
	emit_n("POP\t%%ebx");
	emit_n("CMP\t%%eax, %%ebx");
	emit_n("SETGE\t%%al");
	emit_n("MOVSX\t%%al, %%eax");
}

static void is_less()
{
	expression();
	emit_n("POP\t%%ebx");
	emit_n("CMP\t%%eax, %%ebx");
	emit_n("SETL\t%%al");
	emit_n("MOVSX\t%%al, %%eax");
}

static void is_less_or_equal()
{
	expression();
	emit_n("POP\t%%ebx");
	emit_n("CMP\t%%eax, %%ebx");
	emit_n("SETLE\t%%al");
	emit_n("MOVSX\t%%al, %%eax");
}

static void bool_relation()
{
	char *token_forward;
	expression();

	token_forward = look_forward();
	if (is_relop(token_forward)) {
		get_token();
		emit_n("PUSH\t%%eax");
		if (strcmp(token, "==") == 0)
			is_equal();
		else if (strcmp(token, "!=") == 0)
			is_notequal();
		else if (strcmp(token, ">") == 0)
			is_greater();
		else if (strcmp(token, ">=") == 0)
			is_greater_or_equal();
		else if (strcmp(token, "<") == 0)
			is_less();
		else if (strcmp(token, "<=") == 0)
			is_less_or_equal();
		emit_n("TEST\t%%eax, %%eax");
	}
}

static void bool_factor()
{
	char *token_forward;

	token_forward = look_forward();
	if (is_const_boolean(token_forward)) {
		get_token();
		if (get_boolean())
			emit_n("MOVL\t$-1, %%eax");
		else
			emit_n("XOR\t%%eax, %%eax");
	} else
		bool_relation();
}

static void not_factor()
{
	char *token_forward;

	token_forward = look_forward();
	if (is_notop(token_forward)) {
		get_token();
		bool_factor();
		emit_n("NOTL\t%%eax");
	} else
		bool_factor();
}

static void bool_term()
{
	char *token_forward;

	not_factor();

	for (token_forward = look_forward();
			is_andop(token_forward);
			token_forward = look_forward()) {
		get_token();
		emit_n("PUSH\t%%eax");
		not_factor();
		emit_n("POP\t%%ebx");
		emit_n("ANDL\t%%eax, %%ebx");
	}
}

static void bool_or()
{
	bool_term();
	emit_n("POP\t%%ebx");
	emit_n("ORL\t%%ebx, %%eax");
}

static void bool_expression(char ending)
{
	char *token_forward;

	bool_term();

	for (token_forward = look_forward();
			is_orop(token_forward);
			token_forward = look_forward()) {
		get_token();
		emit_n("PUSH\t%%eax");
		bool_or();
	}
	match(ending);
}


static inline char *new_label(char *label)
{
	sprintf(label, "L%03d", label_count++);
	return label;
}

static inline void post_label(const char *label)
{
	printf("%s:\n", label);
}

static inline char *new_for_file(char *file_name)
{
	sprintf(file_name, "%s.%d", TMP_FILE, for_file_count++);
	return file_name;
}

void do_if(const char *label_continue, const char *label_break)
{
	char *token_forward;
	char label_false[LABEL_LEN], label_end[LABEL_LEN];
	char block_ending = LINE_ENDING;

	new_label(label_false);
	match('(');
	bool_expression(')');

	if (look == '{') {
		get_token();
		block_ending = '}';
	}

	emit_n("JEQ\t%s", label_false);
	block(block_ending, label_continue, label_break);

	token_forward = look_forward();
	if (strcmp(token_forward, "else") == 0) {
		get_token();
		if (look == '{') {
			get_token();
			block_ending = '}';
		} else
			block_ending = LINE_ENDING;
		new_label(label_end);
		emit_n("JMP\t%s", label_end);
		post_label(label_false);
		block(block_ending, label_continue, label_break);
		strcpy(label_false, label_end);
	}
	post_label(label_false);
}

void do_while()
{
	char label_loop[LABEL_LEN], label_end[LABEL_LEN];
	char block_ending = LINE_ENDING;

	new_label(label_loop);
	new_label(label_end);
	post_label(label_loop);
	match('(');
	bool_expression(')');

	if (look == '{') {
		get_token();
		block_ending = '}';
	}

	emit_n("JEQ\t%s", label_end);
	block(block_ending, label_loop, label_end);
	emit_n("JMP\t%s", label_loop);
	post_label(label_end);
}

void do_dowhile()
{
	char label_loop[LABEL_LEN], label_end[LABEL_LEN];
	char block_ending = LINE_ENDING;

	post_label(new_label(label_loop));
	new_label(label_end);

	if (look == '{') {
		get_token();
		block_ending = '}';
	}

	block(block_ending, label_loop, label_end);

	match_token("while");
	match('(');
	bool_expression(')');
	_debug("%s %c\n", token, look);
	emit_n("JNE\t%s", label_loop);
	post_label(label_end);
	match(LINE_ENDING);
	_debug("%s %c\n", token, look);
}

static void parse_for_init()
{
	char name[NAME_MAX];

	match('(');

	if (look == ';') {
		match(';');
		return;
	}

	get_name(name);
	if (look == '=') {
		match('=');
		expression();
		emit_n("LEAL\t%s, %%edx", name);
		emit_n("MOVL\t%%eax, (%%edx)");
	} else if (look == '(') {
		match('(');
		match(')');
		emit_n("CALL\t%s", name);
	}
	match(';');
}

static void parse_for_condition(const char *label_loop, const char *label_end)
{
	if (look == ';') {
		match(';');
		return;
	}

	post_label(label_loop);

	emit_n("; for <condition> ---");
	bool_expression(LINE_ENDING);
	emit_n("; END <condition>");

	emit_n("JEQ\t%s", label_end);
}

static FILE *file_for_repeat()
{
	char file_name[NAME_MAX];
	FILE *fp = fopen(new_for_file(file_name), "w+");

	while (look != ')') {
		fputc(look, fp);
		getchar_x();
	}
	fputc(LINE_ENDING, fp);
	match(')');
	return fp;
}

static void parse_for_repeat(FILE *fp,
		const char *label_loop,
		const char *label_end)
{
	char name[NAME_MAX];
	FILE *stdin_back = stdin;
	char c = look;

	rewind(fp);
	stdin = fp;
	init();
	get_name(name);
	if (name[0] == '\0')
		goto out;

	emit_n("; for repeate part");

	if (look == '=') {
		match('=');
		expression();
		emit_n("LEAL\t%s, %%edx", name);
		emit_n("MOVL\t%%eax, (%%edx)");
	} else if (look == '(') {
		match('(');
		match(')');
		emit_n("CALL\t%s", name);
	}
out:
	stdin = stdin_back;
	look = c;
	emit_n("JMP\t%s", label_loop);
	post_label(label_end);
	emit_n();
}

void do_for()
{
	char label_loop[LABEL_LEN], label_end[LABEL_LEN];
	char block_ending = LINE_ENDING;
	FILE *fp;

	new_label(label_loop);
	new_label(label_end);

	parse_for_init();
	parse_for_condition(label_loop, label_end);
	fp = file_for_repeat();

	if (look == '{') {
		match('{');
		block_ending = '}';
	}

	block(block_ending, label_loop, label_end);

	parse_for_repeat(fp, label_loop, label_end);
}

static inline void do_continue(const char *label_continue)
{
	if (label_continue != NULL)
		emit_n("JMP\t%s", label_continue);
	else
		fail("NO loop to continue");
}

static inline void do_break(const char *label_break)
{
	if (label_break != NULL)
		emit_n("JMP\t%s", label_break);
	else
		fail("NO loop to break from");
}

void other()
{
	emit_n("%s", token);
}

void block(char ending, const char *label_continue, const char *label_break)
{
	for (get_token(); token[0] != ending && look != EOF; get_token()) {
		if (strcmp(token, "if") == 0)
			do_if(label_continue, label_break);
		else if (strcmp(token, "while") == 0)
			do_while();
		else if (strcmp(token, "do") == 0)
			do_dowhile();
		else if (strcmp(token, "for") == 0)
			do_for();
		else if (strcmp(token, "continue") == 0)
			do_continue(label_continue);
		else if (strcmp(token, "break") == 0)
			do_break(label_break);
		else
			other();
	}
}

void do_program()
{
	while (look != EOF)
		block(LINE_ENDING, NULL, NULL);
}
