#include <unistd.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char ** argv) {
	helloworld();
	printf("This is an int: ");
	printint(6);
	char myChar[] = "This is a char String!\n";
	printstring(myChar, sizeof(myChar));
	printf("This is a regular string!\n");
	return 0;
	_exit(0);
}
