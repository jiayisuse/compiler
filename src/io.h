#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdlib.h>
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


extern char look;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
static const char NAME_EXTRA_CHAR[] = { '_' };

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

/* report what was expected */
static inline void expected(const char *str)
{
	fprintf(stderr, "%s is expected\n", str);
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
	while (isspace(look) && look != 10)
		getchar_x();
}

/* math a specific input character */
static inline void match(char c)
{
	if (look != c) {
		char str[4] = { '\'', '\0', '\'' };
		str[1] = c;
		expected(str);
	}

	getchar_x();
	skip_space();
}

/* read an identifier */
static inline void get_name(char *name_buf)
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
}

/* read a number */
static inline void get_num(char *num_buf)
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
}

/* initialize */
static inline void init()
{
	getchar_x();
	skip_space();
}

#endif
