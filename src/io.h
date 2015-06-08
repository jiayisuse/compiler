#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define NAME_MAX	1024
#define NUM_MAX		21

#define _debug(FMT, ...)					\
		printf("Debug:  "FMT, ##__VA_ARGS__)

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
		fprintf(stderr, FMT" is expected\n", ##__VA_ARGS__);	\
		exit(1);						\
	} while (0)


extern char look;
extern char token[NAME_MAX];

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
static const char NAME_EXTRA_CHAR[] = { '_' };
static const char SEPARATORS[] = { '+', '-', '*', '/', '=', '(', ')' };

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
	if (feof(stdin))
		error("stdin reaches EOF");
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

static inline char *get_token()
{
	int i = 0;

	skip_space();
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
	token[i] = '\0';

	return token;
}

/* match a specific input character */
static inline void match(char c)
{
	if (look != c) {
		expected("'%c'", c);
	}

	getchar_x();
	skip_space();
}

/* match a specific input token */
static inline void token_match(const char *str)
{
	if (strcmp(token, str) != 0)
		expected("\"%s\"", str);
}

/* read an identifier */
static inline char *get_name(char *name_buf)
{
	int i;

	if (!isalpha(look))
		expected("name");

	for (i = 0; is_name_char(look); i++) {
		if (i == NAME_MAX - 1)
			fail("name is too long");
		name_buf[i] = look;
		getchar_x();
	}
	name_buf[i] = '\0';

	skip_space();

	return name_buf;
}

/* read a number */
static inline char *get_num(char *num_buf)
{
	int i;

	if (!isdigit(look))
		expected("integer");

	for (i = 0; isdigit(look); i++) {
		if (i == NUM_MAX - 1)
			fail("number is too long");
		num_buf[i] = look;
		getchar_x();
	}
	num_buf[i] = '\0';

	skip_space();

	return num_buf;
}

/* initialize */
static inline void init()
{
	getchar_x();
	skip_space();
}

#endif
