/*  model.c
*   2015, Cody M Leff
*   for Argo coding challenge - Semantic Analysis
*   -------------------------
*   Summary: creates a model of a piece of text by recording every word that is
*   found to follow a given word from the text, then creates sentences from
*   the model by randomly selecing a word and then randomly selecting consecutive
*   words from the list of words that have followed the previous word in the text.
*   -------------------------
*   Design choices & notes:
*    - The only patterns recorded are two-word pairs.  This algorithm could
*      be made more accurate in its creation of valid sentences by recording
*      patterns of three or more words, but it would then create sentences
*      of lesser variety.
*    - A significant amount of the algorithm is dedicated to reproducing when
*      words are found to start and end sentences.  I focused on this because
*      it was an easy-to-implement way to make sentences appear more correct
*      without having to delve very far into semantics.  A more involved
*      algorithm tailored to the English langugae would produce more correct
*      sentences, but I did not feel comfortable with my background in English
*      language rules to implement anything more complicated, plus I was worried
*      such a project would prove unruly when written in C.
*    - Though the algorithm recognizes phrases ending in periods, exclamation
*      points, question marks and semicolons as independent clauses, it only
*      produces sentences with periods.  A next step might be to randomize the
*      phrase-ending punctuation to provide more variety.
*    - The algorithm leaves commas attached to their respective words.  This
*      means that matching will not occur between comma'd words and their non-
*      comma'd equivalents, but it adds variety to the sentences.
*    - To create sentences, the algorithm uses recursive backtracking to find
*      sentences that are of the requested length but also end with a sentence
*      ending word.  It's expensive, but it creates sentences that are more
*      realistic.
*   -------------------------
*   Other Notes:
*    - n_occurrences is only used for printing model
*    - forgot to add the new sentence to new_sentences
*    - forgot to implement freeing!!!  would free: sentences, then word strings for each Word, then model
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include "model.h"

#define MAX_WORD_LENGTH 50  // hard-coded into fscanf format string
#define MAX_WORDS_IN_MODEL 10000
#define MAX_STARTING_WORDS 1000
#define MAX_FOLLOWING_WORDS 100
#define MAX_SENTENCES 100

/*  Struct: Word
*   ------------
*   Data structure storing the information for each word in the model.  Note that if
*   the first
*/
typedef struct Word {
    char* string;   // pointer to the string
    int n_occurrences; // number of times this word appeared in the text
    bool is_sentence_ender;  // if the word has been found to end a sentence
    int n_nw;  // size of next_words and counts
    void* next_words[MAX_FOLLOWING_WORDS]; // array of pointers to any words that followed
    bool tested[MAX_FOLLOWING_WORDS];    // used in marking words for the recursive sentence generation
} Word;

/*  Struct: ModelImplementation
*   ---------------------------
*   Data structure that stores the model.  Contains an array of Word structs, along
*   with an array of pointers to all words that start sentences.
*/
struct ModelImplementation {
    int n_w; // size of words
    Word words[MAX_WORDS_IN_MODEL]; // array of every word, stored as Word structs
    int n_ssw;  // size of sentence_starting_words
    Word* sentence_starting_words[MAX_STARTING_WORDS];  // array of pointers to all words that start sentences
    int n_s;  // size of sentences
    char* sentences[MAX_SENTENCES];
};


//  -------Function prototypes-------
Model* initialize_model();
int scan_next_word(FILE* text, char* next_word_buf);
bool check_if_ends_sentence(char* next_word_buf);
Word* add_next_word_to_model(Model* model, char* next_word_buf, bool ends_sentence, bool new_sentence);
Word* search_words(Model* model, char* next_word_buf);
Word* create_word(Model* model, char* next_word_buf, bool ends_sentence);
void link_words(Word* this_word, Word* next_word);
void print_model(Model* model);
bool find_words(Model* model, int length, Word* sentence[]);
bool find_words_recursive(int length, Word* sentence[], int cur_index);
bool not_all_checked(bool array[], int n_elems);
char* combine_words(Word* sentence[], int length);
int random_int(int lower_bound, int upper_bound);
//  ---------------------------------


/*  Function: create_model
*   ----------------------
*   Generates the model of words.  Maintains a focus window of two words (this_word and next_word),
*   represented by pointers to Word structs in the model's main array.  On each interation, the
*   function first scans a new word, notes whether that word ends a sentence and removes that
*   punctuation if it does, then adds it to the model.  With this_word and next_word populated,
*   the function links this_word to next_word in the model if this_word did not end a sentence;
*   if this_word ended a sentence, the function de-capitalizes this_word and adds next_word to the
*   model's list of words that can begin sentences.  The function then returns a pointer to the model.
*/
Model* create_model(FILE* text) {
    if (!text) {
        printf("Could not create model, no file provided.\n");  // checks for NULL file pointer
        exit(1);
    }
    Model* model = initialize_model();
    Word* this_word = NULL;
    bool new_sentence = true;
    while (true) {
        char next_word_buf[MAX_WORD_LENGTH + 1];
        if (scan_next_word(text, next_word_buf)) break;
        bool ends_sentence = check_if_ends_sentence(next_word_buf);
        Word* next_word = add_next_word_to_model(model, next_word_buf, ends_sentence, new_sentence);
        if (new_sentence) {  // if this_word ended a sentence and next_word begins a sentence
            // LIMITATION: if sentence starts with prop. noun, will be un-capitalized in model
            (model->sentence_starting_words)[model->n_ssw++] = next_word;
            new_sentence = false;
        } else link_words(this_word, next_word);
        if (ends_sentence) {
            next_word->is_sentence_ender = true;
            new_sentence = true;
        }
        this_word = next_word;
    }
    if (!model->n_w) {
        printf("could not create model, no words found.\n");
        exit(1);
    }
    return model;
}

/*  Function: initialize_model
*   --------------------------
*   Creates a new heap-allocated model and sets its fields to empty.
*/
Model* initialize_model() {
    Model* model = malloc(sizeof(Model));    // declares the model struct
    model->n_w = 0;
    model->n_ssw = 0;
    srand(time(NULL));
    return model;
}

/*  Function: scan_next_word
*   ------------------------
*   Scans the next word from the text, populating the next_word_buf character array with the
*   string, including punctuation and capitalization.  Returns 0 on success, -1 on failure.
*/
int scan_next_word(FILE* text, char* next_word_buf) {
    int n_scanned = fscanf(text, " %50[a-zA-Z!?,.;:'] %*50[^a-zA-Z!?,.;:']", next_word_buf);
    if (n_scanned != 1) return -1;
    return 0;
}

/*  Function: check_if_ends_sentence
*   --------------------------------
*   Checks if the string contained in next_word_buf ends with independent-clause-forming
*   punctuation, and if so, removes that punctuation and returns true.  If not, returns
*   false.
*/
bool check_if_ends_sentence(char* next_word_buf) {
    char* final_char = next_word_buf + strlen(next_word_buf) - 1;
    if (*final_char == '.' || *final_char == '?' || *final_char == '!' || *final_char == ';') {
        *final_char = '\0';
        return true;
    }
    return false;
}

/*  Function: add_next_word_to_model
*   --------------------------------
*   Decapitalizes next_word_buf if new_sentence is true, then searches the model's Word array
*   for a match.  If a match is found, the function updates that Word entry and returns a pointer
*   to the found entry.  If no match is found, the function creates a new Word struct in the
*   array and returns a pointer to it.
*/
Word* add_next_word_to_model(Model* model, char* next_word_buf, bool ends_sentence, bool new_sentence) {
    if (new_sentence) *next_word_buf = tolower(*next_word_buf);  // decapitalizes word
    Word* match = search_words(model, next_word_buf);
    if (match) {
        if (ends_sentence) match->is_sentence_ender = true;
        match->n_occurrences++;
        return match;
    } else return create_word(model, next_word_buf, ends_sentence);
}

/*  Function: search_words
*   ----------------------
*   Iterates through the model's array of Word structs, comparing each struct's string field
*   to next_word_buf and returning a pointer to the matching struct, if it exists, or NULL if
*   not found.
*/
Word* search_words(Model* model, char* next_word_buf) {
    for (int i = 0; i < model->n_w; i++) {
        char* word_to_cmp = (model->words[i]).string;
        if (!strcmp(word_to_cmp, next_word_buf)) return model->words + i;
    }
    return NULL;
}

/*  Function: create_word
*   ---------------------
*   Creates a new Word struct for the found word, appending it to the end of the array of Word
*   structs in the model.
*/
Word* create_word(Model* model, char* next_word_buf, bool ends_sentence) {
    (model->words)[model->n_w].string = strdup(next_word_buf);
    (model->words)[model->n_w].n_occurrences = 1;
    (model->words)[model->n_w].is_sentence_ender = ends_sentence;
    (model->words)[model->n_w].n_nw = 0;
    model->n_w++;
    return model->words + model->n_w - 1;
}

/*  Function: link_words
*   --------------------
*   Adds next_word to the next_words array of this_word.
*/
void link_words(Word* this_word, Word* next_word) {
    (this_word->next_words)[this_word->n_nw++] = next_word;
}

/*  Function: print_model
*   ---------------------
*   Prints all elements in the model.
*/
void print_model(Model* model) {
    printf("----------MODEL----------\n");
    printf("---Model size: %d words\n", model->n_w);
    printf("---Words:\n");
    for (int i = 0; i < model->n_w; i++) {
        Word word = (model->words)[i];
        printf("%s (%d)", word.string, word.n_occurrences);
        if (word.is_sentence_ender) printf(" (se)");
        printf(": ");
        for (int j = 0; j < word.n_nw; j++) {
            printf("%s ", ((Word*)(word.next_words)[j])->string);
        }
        printf("\n");
    }
    printf("---Sentence-starting words (%d):\n", model->n_ssw);
    for (int i = 0; i < model->n_ssw; i++) {
        Word* word = (model->sentence_starting_words)[i];
        printf("%s\n", word->string);
    }
    printf("---------------------------\n");
}

/*  Function: generate_sentence
*   ---------------------------
*   Finds a set of pattern-matched Word structs, then combines them into one
*   heap-allocated sentence string and returns a pointer to it.
*/
char* generate_sentence(Model* model, int length) {
    Word* sentence[length];
    if (!find_words(model, length, sentence)) {
        return NULL;
    }
    return combine_words(sentence, length);
}

/*  Function: find_words
*   --------------------
*   For each word in sentence_starting_words (selected in random order), the function calls
*   find_words_recursive to attempt to create a sentence from that initial word.  If an attempt
*   succeeds, the function returns 'true' with a populated sentence array; if none succeed the
*   function returns false.
*/
bool find_words(Model* model, int length, Word* sentence[]) {
    bool ssw_checked[model->n_ssw];
    for (int i = 0; i < model->n_ssw; i++) ssw_checked[i] = false;
    while(not_all_checked(ssw_checked, model->n_ssw)) {
        int ssw_index = random_int(0, model->n_ssw - 1);
        if (!ssw_checked[ssw_index]) {
            ssw_checked[ssw_index] = true;
            sentence[0] = model->sentence_starting_words[ssw_index];
            if (find_words_recursive(length, sentence, 0)) return true;
        }
    }
    return false;
}

/*  Function: find_words_recursive
*   ------------------------------
*   First checks the base case where the recursion has found enough words to create a sentence;
*   the function returns true all the way down the stack if the final word is a sentence ender
*   and returns false to the previous stack frame if not.  For non-base-cases, the function
*   zeros out the array that keeps track of which next_words elements it has already tested,
*   then randomly selects next_words elements that have not been previously tried to append
*   to the sentence and recursively test until all elements have been tested.  A success at
*   sentence end propagates a 'true' value back to the calling function; 'false' is returned
*   when none work.  LIMITATION: because of how the next words are stored, the function may
*   unnecessarily test duplicate entries of the same next_word.
*/
bool find_words_recursive(int length, Word* sentence[], int cur_index) {
    if (length == cur_index + 1) {
        if (sentence[length - 1]->is_sentence_ender) return true;
        else return false;
    }
    Word* this_word = sentence[cur_index];
    for (int i = 0; i < this_word->n_nw; i++) this_word->tested[i] = false;
    while(not_all_checked(this_word->tested, this_word->n_nw)) {
        int nw_index = random_int(0, this_word->n_nw - 1);
        if (!this_word->tested[nw_index]) {
            this_word->tested[nw_index] = true;
            Word* next_word = this_word->next_words[nw_index];
            sentence[cur_index + 1] = next_word;
            if (find_words_recursive(length, sentence, cur_index + 1)) return true;
        }
    }
    return false;
}

/*  Function: not_all_checked
*   -------------------------
*   Returns true if any of the boolean values in the passed_array are 'false.'
*/
bool not_all_checked(bool array[], int n_elems) {
    for (int i = 0; i < n_elems; i++) {
        if (!array[i]) return true;
    }
    return false;
}

/*  Function: combine_words
*   -----------------------
*   Transforms the array of Word structs into a string containing all the words,
*   making sure to capitalize the initial word and add a period to the final
*   word.  Records the location of the heap-allocated string in the model's
*   sentences array and returns a pointer to the heap-allocated string.
*/
char* combine_words(Word* sentence[], int length) {
    char* sentence_string = calloc(1, 1);
    for (int i = 0; i < length; i++) {
        char* next_word = sentence[i]->string;  // retrieves the next word
        char buf[strlen(sentence_string) + strlen(next_word) + 1 + 1];  // creates a temporary merge location
        // + 1 + 1 to make space for the space or final period
        strcpy(buf, sentence_string);  // moves the string-so-far from the heap into buf
        strcat(buf, next_word);  // concatenates the next word onto the end of buf
        if (i == length - 1) strcat(buf, ".");
        else strcat(buf, " ");
        free(sentence_string);
        sentence_string = strdup(buf);  // moves the new string-so-far to the heap
    }
    *sentence_string = toupper(*sentence_string);
    return sentence_string;
}

/*  Function: random_int
*   --------------------
*   Returns a random integer in the range of the two passed bounds, inclusive.
*/
int random_int(int lower_bound, int upper_bound) {
    int range = upper_bound - lower_bound + 1;
    int random_int = (rand() % range) + lower_bound;
    assert(random_int >= lower_bound && random_int <= upper_bound);
    return random_int;
}
