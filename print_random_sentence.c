/*  make_random_sentences.c
*   2015, Cody M Leff
*   for Argo coding challenge
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "model.h"



/*	Function: main
*	--------------
*	Invocation: make_random_sentences [filename] [n_words_in_sentence]
*	Creates the model, then requests a sentence of the specified length and prints it, if found.
*/
int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("Please invoke with 2 arguments: the source text filename and the number of words in sentence.\n");
		exit(1);
	}
	char* filename = argv[1];
	int n_words;
	if (!sscanf(argv[2], "%d", &n_words)) {
		printf("Word number could not be read.\n");
		exit(1);
	}
	FILE* text = fopen(filename, "r");
	if (!text) {
		printf("File could not be opened.");
		exit(1);
	}
	Model* model = create_model(text);
	fclose(text);
	char* sentence = generate_sentence(model, n_words);
	if (!sentence) printf("No sentences of selected length possible from this model.\n");
	else printf("Random sentence of %d words: \"%s\"\n", n_words, sentence);
}