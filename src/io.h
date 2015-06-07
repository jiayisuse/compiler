#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define _debug(FMT, ...)						\
	do {								\
		if (debug)						\
			printf("Debug:  "FMT, ##__VA_ARGS__);		\
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


extern char look;

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

/* read a char */
static inline void getchar_x()
{
	look = getchar();
	if (ferror(stdin))
		fail("stdin error");
	if (feof(stdin))
		error("stdin reaches EOF");
}

/* math a specific input character */
static inline void match(char c)
{
	char str[4] = { '\'', '\0', '\'' };
	if (look == c)
		getchar_x();
	else {
		str[1] = c;
		expected(str);
	}
}

static inline bool is_addop(char c)
{
	return c == '+' || c == '-';
}

static inline bool is_mulop(char c)
{
	return c == '*' || c == '/';
}

/* read an identifier */
static inline char get_name()
{
	char c = look;
	if (!isalpha(look))
		expected("name");
	else
		getchar_x();
	return c;
}

/* read a number */
static inline char get_num()
{
	char c = look;
	if (!isdigit(look))
		expected("integer");
	else
		getchar_x();
	return c;
}

/* initialize */
static inline void init()
{
	getchar_x();
}

#endif
