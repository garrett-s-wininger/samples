#include <stdio.h>

extern int add(int, int);

int main(int argc, char** argv)
{
	int result = add(2, 3);
	printf("Result of calculation (2 + 3): %d\n", result); 

	return 0;
}
