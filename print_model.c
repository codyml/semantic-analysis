/*  print_model.c
*   2015, Cody M Leff
*   for Argo coding challenge
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "model.h"

/*	Function: main
*	--------------
*	Invocation: print_model [filename]
*	Creates the model, then prints it.
*/
int main(int argc, char* argv[]) {
	FILE* text = fopen(argv[1], "r");
	assert(text);
	Model* model = create_model(text);
	print_model(model);
	fclose(text);
	return 0;
}