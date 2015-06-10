#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define NAME_MAX	1024
#define NUM_MAX		21

#define DEBUG		1

#define _debug(FMT, ...)					\
	do {							\
		if (DEBUG)					\
			printf("Debug:  "FMT, ##__VA_ARGS__);	\
	} while (0)

#define _error(FMT, ...)						\
	do {								\
		fprintf(stderr, "\nERROR at line %d of %s() in \"%s\":\n\t" \
				FMT"\n", __LINE__, __func__, __FILE__,	\
							##__VA_ARGS__);	\
	} while (0)

/* output message with a leading tab */
#define emit(FMT, ...)				\
	printf("\t"FMT, ##__VA_ARGS__)		\

/* output message with a leading tab and a newline */
#define emit_n(FMT, ...)			\
	printf("\t"FMT"\n", ##__VA_ARGS__)

/* report what was expected */
#define expected(FMT, ...)						\
	do {								\
		fprintf(stderr, FMT" is expected at %d\n",		\
			##__VA_ARGS__, __LINE__);			\
		exit(1);						\
	} while (0)


extern char look;
extern char look_back;
extern char token[NAME_MAX];
extern char next_token[NAME_MAX];

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
static const char NAME_EXTRA_CHAR[] = { '_' };
static const char SEPARATORS[] = { '+', '-', '*', '/', '=', '(', ')', '{', '}', '!', '>', '<', '|', '&', ';' };
static const char APPEND_OPERATORS[] = { '+', '-', '=', '|', '&' };

/* report an error */
static inline void error(const char *str)
{
	fprintf(stderr, "ERROR: %s\n", str);
}

/* report error and halt */
static inline void fail(const char *str)
{
	error(str);
	exit(1);
}

static inline bool is_addop(char c)
{
	return c == '+' || c == '-';
}

static inline bool is_mulop(char c)
{
	return c == '*' || c == '/';
}

static inline bool is_orop(const char *op)
{
	return strcmp(op, "||") == 0;
}

static inline bool is_notop(const char *op)
{
	return strcmp(op, "!") == 0;
}

static inline bool is_andop(const char *op)
{
	return strcmp(op, "&&") == 0;
}

static inline bool is_relop(const char *op)
{
	return strcmp(op, "==") == 0 ||
		strcmp(op, "!=") == 0 ||
		strcmp(op, ">") == 0 ||
		strcmp(op, ">=") == 0 ||
		strcmp(op, "<") == 0 ||
		strcmp(op, "<=") == 0;
}

static inline bool is_name_char(char c)
{
	int i;
	
	if (isalnum(c))
		return true;

	for (i = 0; i < ARRAY_SIZE(NAME_EXTRA_CHAR); i++)
		if (c == NAME_EXTRA_CHAR[i])
			return true;
	return false;
}

/* read a char */
static inline void getchar_x()
{
	look = getchar();
	if (ferror(stdin))
		fail("stdin error");
	/*
	if (feof(stdin))
		fail("stdin reaches EOF");
	*/
}

static inline void skip_space()
{
	while (isspace(look))
		getchar_x();
}

static inline bool is_separator(char c)
{
	int i, len;

	if (isspace(c))
		return true;
	
	for (i = 0, len = ARRAY_SIZE(SEPARATORS); i < len; i++)
		if (c == SEPARATORS[i])
			return true;
	return false;
}

static inline bool is_append_operator(char c)
{
	int i, len;

	if (isspace(c) || isalnum(c))
		return false;

	for (i = 0, len = ARRAY_SIZE(APPEND_OPERATORS); i < len; i++)
		if (c == APPEND_OPERATORS[i])
			return true;
	return false;
}

/*
 * ++, --, +=, -=, *=, /=, ==, !=, <=, >=, &&, ||
 */
static inline int append_operator(int i, char *to)
{
	switch (to[0]) {
	case '+':
	case '-':
		if (to[0] == look || look == '=') {
			to[i++] = look;
			getchar_x();
		}
		break;

	case '*':
	case '/':
	case '=':
	case '!':
	case '>':
	case '<':
		if (look == '=') {
			to[i++] = look;
			getchar_x();
		}
		break;

	case '&':
	case '|':
		if (to[0] == look) {
			to[i++] = look;
			getchar_x();
		}
		break;

	default:
		break;
	}

	return i;
}

static inline char *get_token()
{
	int i = 0;

	if (next_token[0] != '\0') {
		strcpy(token, next_token);
		next_token[0] = '\0';
		look = look_back;
		return token;
	}

	token[i++] = look;
	getchar_x();
	if (is_separator(token[0]))
		goto out;

	while (i < NAME_MAX - 1) {
		if (is_separator(look))
			break;
		token[i++] = look;
		getchar_x();
	}

out:
	if (i == 1 && is_append_operator(look))
		i = append_operator(i, token);
	token[i] = '\0';
	skip_space();

	return token;
}

static inline char *look_forward()
{
	int i = 0;
	char c;

	if (next_token[0] != '\0')
		return next_token;

	c = look;
	next_token[i++] = look;
	getchar_x();
	if (is_separator(next_token[0]))
		goto out;

	while (i < NAME_MAX - 1) {
		if (is_separator(look))
			break;
		next_token[i++] = look;
		getchar_x();
	}

out:
	if (i == 1 && is_append_operator(look))
		i = append_operator(i, next_token);
	next_token[i] = '\0';
	skip_space();

	look_back = look;
	look = c; 

	return next_token;
}

/* match a specific input character */
/*
static inline void match(char c)
{
	if (look != c)
		expected("'%c'", c);

	getchar_x();
	skip_space();
}
*/

/* match a specific input token */
/*
*/

/* match a specific input token with a char */
static inline void match(char c)
{
	get_token();
	if (token[0] != c || token[1] != '\0')
		expected("'%c'", c);
}

static inline void match_token(const char *str)
{
	get_token();
	if (strcmp(token, str) != 0)
		expected("\"%s\"", str);
}

static inline bool is_name(char *name, char *buf)
{
	int i;

	if (!isalpha(name[0]))
		return false;

	for (i = 0; name[i] != '\0' && is_name_char(name[i]); i++)
		buf[i] = name[i];

	if (name[i] == '\0') {
		buf[i] = '\0';
		return true;
	}

	return false;
}

static inline bool is_num(char *num, char *buf)
{
	int i;

	if (!isdigit(num[0]))
		return false;

	for (i = 0; num[i] != '\0' && isdigit(num[i]); i++)
		buf[i] = num[i];

	if (num[i] == '\0') {
		buf[i] = '\0';
		return true;
	}

	return false;
}

/* read an identifier */
static inline char *get_name(char *name_buf)
{
	get_token();
	if (is_name(token, name_buf))
		return name_buf;
	else
		expected("name");

}

/* read a number */
static inline char *get_num(char *num_buf)
{
	get_token();
	if (is_num(token, num_buf))
		return num_buf;
	else
		expected("integer");
}

static inline bool is_const_boolean(const char *str)
{
	return strcmp(str, "true") == 0 ||
		strcmp(str, "false") == 0;
}

static inline bool get_boolean()
{
	get_token();
	if (strcmp(token, "true") == 0)
		return true;
	else if (strcmp(token, "false") == 0)
		return false;
	else
		expected("Boolean");
}

/* initialize */
static inline void init()
{
	getchar_x();
	skip_space();
}

#endif
