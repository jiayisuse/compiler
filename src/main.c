#include <string.h>
#include "io.h"
#include "process.h"

char look;
char token[NAME_MAX];

int main()
{
	init();
	/*
	assignment();
	if (look == '\n')
		expected("New Line");
	*/
	do_program();
	/*
	while (get_token()) {
		if (strcmp(token, "end") == 0)
			break;
		printf("--- %s\n", token);
	}
	*/
	return 0;
}
