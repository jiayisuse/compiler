#include "io.h"
#include "process.h"

char look;

int main()
{
	init();
	assignment();
	if (look == '\n')
		expected("New Line");
	return 0;
}
