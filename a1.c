/*
 *			Unjumble.c
 *		CSSE2310 - Assignment One
 *		 Written by Riley Mosca	
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


//A safe assumption for the maximum word length is 50 letters
#define MAX_DICT_WORD_LENGTH 50

//A safe Buffer size of 42 accomodates for new line and terminating null
#define BUFFER_SIZE_WORD (MAX_DICT_WORD_LENGTH + 2)

//Our dictionary of words will be taken from the following path definition
#define DICT_PATH "/usr/share/dict/words"

//Types which represent the type of order the words are outputted as
typedef enum {
    DEFAULT, ALPHA, LEN, LONGEST
} OutputType;

// Structure to store the command-line parameters
typedef struct {
  OutputType outputType;   //The output type of words that are unjumbled
    int includeChar;	   //1 or 0 depending if parameter is present
    char letterToInclude;  //A letter that must be included in the output(s)
    char* lettersToSearch; //A char array of letters that will be used
    char* fileName;        //The dictionary filename to be used in unjumbling
} UnjumbleParameters;

//Structure storing words made and the number of words made from dictionary
typedef struct {
    char** words; 
    int numOfWords;
} WordsMade;

/* Function declarations - including parameters and return types */
/* Not including our main function*/
UnjumbleParameters unjumble_command_line(int argc, char* argv[]);
void invalid_args(void);
void invalid_file(char* fileName);
void invalid_letters_size(void);
void invalid_letters_alpha(void);
void sort_and_print(OutputType output, WordsMade wordsMade);
void included_letter_valid(char letter);
void letters_input_valid(char* letters);
void print_words(WordsMade wordsMade);
UnjumbleParameters check_output_arg(char* outputType, \
        UnjumbleParameters outputParam);
WordsMade add_words_to_list(WordsMade wordsMade, char* word);
char* to_lower_all(char* word);	
int match_made(char* a, char* b);
int check_duplicates(char* word, char letter);
int char_occurences(char* word, char letter);
int compare_alpha(const void* word1, const void* word2);
int compare_len(const void* word1, const void* word2);
/***********************************************************/

/* Below is the body of our code for all our functions */
int main(int argc, char** argv) {
	UnjumbleParameters unjumble;
	WordsMade wordsMade;
    FILE* dictionaryArg;
    char currentWord[BUFFER_SIZE_WORD];

    wordsMade.words = NULL;
    wordsMade.numOfWords = 0;
	
	//finds out struct paramters
    unjumble = unjumble_command_line(argc, argv);
	
	//checks if the included char is a letter
    if (unjumble.includeChar) {
        included_letter_valid(unjumble.letterToInclude);
    }
	
	//checks the validity of the letters input
    letters_input_valid(unjumble.lettersToSearch);
	
	//checks the validity of the file input
    dictionaryArg = fopen(unjumble.fileName, "r");
    if (!dictionaryArg) {
        invalid_file(unjumble.fileName);
    }
	
    while (fgets(currentWord, BUFFER_SIZE_WORD, dictionaryArg)) {
        if (match_made(unjumble.lettersToSearch, currentWord)) {
            if (unjumble.includeChar) { //checking for include
                for (int i = 0; i < strlen(currentWord); i++) {
                    if (unjumble.letterToInclude == tolower(currentWord[i]) 
                            || unjumble.letterToInclude
                            == toupper(currentWord[i])) {
                        wordsMade 
                                = add_words_to_list(wordsMade, currentWord);
                    }						
                }
            } else {
                wordsMade = add_words_to_list(wordsMade, currentWord);
            }
        }
    }
    sort_and_print(unjumble.outputType, wordsMade); //sort and print words
	
    if (wordsMade.numOfWords < 1) {
        exit(10); //no words have been added, exit with status 10
    }
	//close the dictionary, we have finished.
    fclose(dictionaryArg);	
	
    return 0; //Exiting with a return of 0 means no issues were encountered
}

/* A function which filters an input into it's struct counterpart and
 * returns the struct to be used in the program.
 * params:
 *	- (int) argc our "size" of our char**
 *  - (char**) the array of our strings taken in by the command line
 * returns:
 *  - returns a structure of our arguments to be used in the main()
 */
UnjumbleParameters unjumble_command_line(int argc, char* argv[]) {
    UnjumbleParameters arguments;	 //our arguments struct
    int isOutputPresent = 0;		 //indicator if output is present
    int isIncludePresent = 0;		 //indicator if include is present
    char dash = '-';				 // delimeter
	
	//First char* in argv is our executable name, so we shall skip this
	//And proceed to check the arguments
    argv++;                          //updating position in array
    argc--;                          //decrementing array size
    arguments.outputType = DEFAULT;  //default output type
    arguments.includeChar = 0;		 //include char default option
    arguments.fileName = DICT_PATH;  // default file path
	
    while ((argc > 0) && (argv[0][0] == dash)) {
		
        if (strcmp(argv[0], "-include") == 0) {
            isIncludePresent = 1;
            arguments.includeChar = isIncludePresent;
            argv++;
            argc--;
			
            if (isIncludePresent == 1) {
                arguments.letterToInclude = argv[0][0];
					
                if (strlen(argv[0]) > 1 || !isalpha((int)argv[0][0])) {
                    invalid_args();
                }	
                argv++;
                argc--;
            }
            continue; // letter has been assigned, continue checking
			
        } else if (!(isOutputPresent) && strcmp(argv[0], "-alpha") == 0) {
            arguments.outputType = ALPHA;
            isOutputPresent = 1;
			
        } else if (!(isOutputPresent) && strcmp(argv[0], "-len") == 0) {
            arguments.outputType = LEN;
            isOutputPresent = 1;
				
        } else if (!(isOutputPresent) && strcmp(argv[0], "-longest") == 0) {
            arguments.outputType = LONGEST;
            isOutputPresent = 1;
			
        } else {
            invalid_args(); //output type is invalid or repeated
        }
		
        argv++;
        argc--;
    }
	
	//Arguments that remain, can only be "letters [filename]" or "letters"
	//anymore than this, an error has occurred (too many inputs)
    if (argc == 2) {
        arguments.lettersToSearch = argv[0]; //letters
        arguments.fileName = argv[1]; //filename
		
    } else if (argc == 1) {
        arguments.lettersToSearch = argv[0]; //letters
		
    } else {
        invalid_args(); //invalid arguments, error should occur
		
    }
    return arguments;
}

/* A function which prints an error message for invalid arguments
 * returns:
 *  - non-zero exit status and error message
 */
void invalid_args(void) {
    fprintf(stderr, "Usage: unjumble [-alpha|-len|-longest] [-include letter]"
            " letters [dictionary]\n");
    exit(1);
}

/* A function which prints an error message for an invalid file name
 * returns:
 *  - non-zero exit status and error message
 */
void invalid_file(char* fileName) {
    fprintf(stderr, "unjumble: file \"%s\" can not be opened\n", fileName);
    exit(2);
}

/* A function which prints an error message for invalid "letters" size
 * returns:
 *  - non-zero exit status and error message
 */
void invalid_letters_size(void) {
    fprintf(stderr, "unjumble: must supply at least three letters\n");
    exit(3);
}

/* A function which prints an error message for invalid "letters" type
 * returns:
 *  - non-zero exit status and error message
 */
void invalid_letters_alpha(void) {
    fprintf(stderr, "unjumble: can only unjumble alphabetic characters\n");
    exit(4);
}

/* A function which checks validty of the "letter" input
 * params:
 *	- (char) letter, the letter we are checking
 * returns:
 *  - returns 1 if fine, an error otherwise
 */
void included_letter_valid(char letter) {
    if (!isalpha(letter)) {
        invalid_letters_alpha();
    }
}

/* A function which checks validty of "letters" input
 * params:
 *	- (char*) letters, the letters we are checking for validity
 * returns:
 *  - returns 1 if fine, returns an error otherwise
 */
void letters_input_valid(char* letters) {
    int sizeString = strlen(letters);

    if ((sizeString) < 3) {
        invalid_letters_size();
    }

    for (int i = 0; i < sizeString; i++) {
        if (isalpha(letters[i])) {
            continue;
        } else {
            invalid_letters_alpha();
        }
    }
}

/* A function which checks for word matches in the dictionary
 * params:
 *	- (char*) letters, the letters we are using for comaprison
 *  - (char*) word, the word to compare to and possibly return
 * returns:
 *  - returns the word which matches
 */
int match_made(char* letters, char* word) {
    char* lettersLower = to_lower_all(strdup(letters));
    char* wordLower = to_lower_all(strdup(word));
    int sizeW = strlen(wordLower);
    if (sizeW < 4) {
        return 0;
    }
	
    for (int i = 0; i < sizeW - 1; i++) {
        if (char_occurences(wordLower, wordLower[i]) > 
                char_occurences(lettersLower, wordLower[i])) {
            return 0;
        } 
    }
    return 1;
}

/* A function which converts all characters to lowercase
 *  - (char*) word, the word we are converting
 * returns:
 *  - returns a word all in lowercase
 */
char* to_lower_all(char* word) {
    for (int i = 0; i < strlen(word); i++) {
        word[i] = tolower((int) word[i]);
    }
    return word;
}

/* A function which counts the occurences of a 
 * given char in a string
 * params:
 *  - (char) letter, the letter we want to count
    - (char*) word, the word we cant to check.
 * returns:
 *  - returns an integer representing the occurence
 *    of that char in the string
 */
int char_occurences(char* word, char letter) {
    int count = 0;
    int sizeW = strlen(word);
	
    for (int i = 0; i < sizeW; i++) {
        if (word[i] == letter) {
            count++;
        }
    }
    return count;
}

/* A function which adds words to a list for sorting
 * params:
 *	- (WordsMade) wordsMade, the struct we are updating
 *  - (char*) word, the word we are adding
 * returns:
 *  - returns a structure of our WordsMade to be used in sorting
 */
WordsMade add_words_to_list(WordsMade wordsMade, char* word) {
    char* tempWord; //Initialising a temporarily word char array
	
	//allocating memory for word length and room for '/0';
    tempWord = malloc(sizeof(char) * (strlen(word) + 1));
	
    strcpy(tempWord, word); //Copying the string pointer word to tempword
	
	//And storing this to the struct wordsMade.words variable
    wordsMade.words = realloc(wordsMade.words, sizeof(char*) 
            * (wordsMade.numOfWords + 1));
		
    wordsMade.words[wordsMade.numOfWords] = tempWord;
    wordsMade.numOfWords++; //increment #of words 
	
    return wordsMade;
}

/* A function which checks the output sorting type, and sorts the words
 * params:
 * - (OutputType) output, the output sort type as per the arguments
 */
void sort_and_print(OutputType output, WordsMade wordsMade) {
    switch (output) {
        case ALPHA:
			//return ALPHA
            qsort(wordsMade.words, wordsMade.numOfWords,
                    sizeof(char*), compare_alpha);
				
            print_words(wordsMade);
            break;
			
        case LEN:
            //returns len words, i.e words from longest to shortest
            qsort(wordsMade.words, wordsMade.numOfWords,
                    sizeof(char*), compare_alpha);
				
            qsort(wordsMade.words, wordsMade.numOfWords,
                    sizeof(char*), compare_len);
				
            print_words(wordsMade);
            break;
			
        case LONGEST:
			//return LONGEST, i.e. longest words which match
            qsort(wordsMade.words, wordsMade.numOfWords,
                    sizeof(char*), compare_alpha);
            
            qsort(wordsMade.words, wordsMade.numOfWords,
                    sizeof(char*), compare_len);
			
            for (int i = 0; i < wordsMade.numOfWords; i++) {
                if (strlen(wordsMade.words[i])
                        == strlen(wordsMade.words[0])) {
						
                    printf("%s", wordsMade.words[i]);
                }
            }
            break;
			
        default:
			//return in default order as 
			//they were found i.e. do nothing
            print_words(wordsMade);
            break;
    }
}
 
/* A comparison function for order of two words by strcmp()
 * params:
 *  - (const void) *word1, the first word we cant to compare
 *  - (const void) *word2, the second word we cant to compare
 * returns:
 *  - returns an integer which tells qsort how to order the words
 *    in the struct given
 */
int compare_alpha(const void* word1, const void* word2) {
    const char* wordOne = *((char**)word1);
    const char* wordTwo = *((char**)word2);
	
    return strcasecmp(wordOne, wordTwo);
}

/* A comparison function for the length of two words
 * params:
 *  - (const void) *word1, the first word we cant to compare
 *  - (const void) *word2, the second word we cant to compare
 * returns:
 *  - returns an integer which tells qsort how to order the words
 *    in the struct given
 */
int compare_len(const void* word1, const void* word2) {
    const char* wordOne = *((char**)word1);
    const char* wordTwo = *((char**)word2);
    int sizeWordOne = strlen(wordOne);
    int sizeWordTwo = strlen(wordTwo);
	
    if (sizeWordTwo == sizeWordOne) {
        return strcasecmp(wordOne, wordTwo);
    }
	
    return strlen(wordTwo) - strlen(wordOne);
}

/* A function which prints the words to terminal from the struct
 * (WordsMade) the struct we want to print from
 */
void print_words(WordsMade wordsMade) {
    for (int i = 0; i < wordsMade.numOfWords; i++) {
        printf("%s", wordsMade.words[i]); //print the words from struct
    }
}

//END
