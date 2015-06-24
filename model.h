/*  model.h
*   2015, Cody M Leff
*   for Argo coding challenge
*/


/*  Struct: Model
*   -------------
*   Reference to stored model created from text document.
*/
typedef struct ModelImplementation Model;

/*  Function: create_model
*   ----------------------
*   Analyzes the source text, passed as a file pointer, and creates a language
*   model from the patterns found.  Returns a Model* pointer for use in client
*   functions.
*/
Model* create_model(FILE* text);

/*  Function: print_model
*   ---------------------
*   Prints all elements in the model.
*/
void print_model(Model* model);

/*  Function: generate_sentence
*   ---------------------------
*   Creates a randomly-generated sentence of the specified word-length based on
*   the language model referenced by the Model* pointer.  Returns a char* pointer
*   to the sentence.
*/
char* generate_sentence(Model* model, int length);

/*  Function: free_allocated
*   ------------------------
*   Frees heap-allocated memory associated with the model and all generated
*   sentences.  Warning: will free memory holding generated sentences, so be
*   sure to save them elsewhere before calling and do not attempt to access
*   them afterwards.
*/
void free_allocated(Model* model);