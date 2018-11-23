#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define WORD_SIZE 100
#define LINE_SIZE 1000
#define ROOTS_ARRAY_SIZE 1000
#define FALSE -1
#define TRUE 1

typedef struct  {
char rootName[WORD_SIZE];
int count;
int isRepresentative;
} root;

void choose_case(char *caseFileName);
void get_and_clean_reviews(FILE *caseFile, root roots[], int *sizeOfRootsArray);
int is_noun(char *word);
char *convert_to_singular(char *word);
void word_count(char *review, root roots[], int *numberOfRoots);
void sort_roots(root roots[], int numberOfRoots);
void group_synonyms(root roots[], int numberOfRoots);
void find_root(char *root, FILE *library);
void make_file(root roots[]);
int index_of_existing_word(char *word, root roots[]);
int compare(const void *p1, const void *p2);
void find_representatives(root roots[], int numberOfRoots, FILE *synLib);


int main(void){
	root roots[ROOTS_ARRAY_SIZE];
	int sizeOfRootsArray;
	char caseFileName[20];
	choose_case(caseFileName);
	FILE *caseFile, *synLib; 
	caseFile = fopen(caseFileName, "r"),
	synLib = fopen("syn_lib.dat", "r");

	if(caseFile != NULL){
		printf("%s\n", caseFileName);
	}
	else{
		printf("It didn't work\n");
	}

	get_and_clean_reviews(caseFile, roots, &sizeOfRootsArray);

	qsort(roots, sizeOfRootsArray, sizeof(root), compare);

    find_representatives(roots, sizeOfRootsArray, synLib);
	

	return 0;
}

// The user chooses a number, and then a specific string with the name of the file that is returned.
void choose_case(char *caseFileName){
	printf("Please write name of file, you motherfucker: ");
	scanf("%s", caseFileName);
}

//receives a FILE pointer. 
//Makes a clean string (with wordnet) with a review in it, and calls the other functions with each individual word.
void get_and_clean_reviews(FILE *caseFile, root roots[], int *sizeOfRootsArray){
	int i = 0, j = 0;
	//Somehow dynamically allocate enough space for reviews in the review char array.
	char review[1000];
	char word[WORD_SIZE];

	// maybe implement this later: clean_review()

	//At this point in the function, word is only lowercase and in base form in the review array.

	while (feof(caseFile)){
		fgets(review, LINE_SIZE, caseFile);
		//find the expression that will go inside the while, so it will go through each word.


		// int n = find antal ord i review array'en, på en eller anden smart måde. 
		int n = 10;
		while (i < n){

			// make some code that assigns a word to the word array.
			if (is_noun(word)){
				int indexExistingWord = index_of_existing_word(word, roots);
				if(indexExistingWord != FALSE){
					roots[indexExistingWord].count++;
				}
				else{
					strcpy(roots[j].rootName, word);
					roots[j].count = 1;
					roots[j].isRepresentative = TRUE;
					j++;
				}
			}
			i++;
		}
	}
	*sizeOfRootsArray = j;
}

//Checks whether the word is a noun
int is_noun(char *word){
	int is_noun;

	//return 1 if the word is a noun and 0 if it isn't
	return is_noun;
}

int index_of_existing_word(char *word, root roots[]){
	int index;

	//index is the index where the words exist, and should be -1 if it doesn't eixst.
	return index;
}



int compare(const void *p1, const void *p2){

    root *root1 = (root *) p1,         
           *root2 = (root *) p2;

    return strcmp(root1->rootName, root2->rootName);
}

void find_representatives(root roots[], int numberOfRoots, FILE *synLib){
    int i = 0;

    for (i = 0; i < numberOfRoots; i++) {
    	if (roots[i].isRepresentative) {
    		find_root(roots[i].rootName, synLib);
    	}

    }

}

/* Proceduren tager en rod samt en filpointer og efter kaldet
 * peger filpointeren på linjen lige under roden ("dens første synonymlinje"). */
void find_root(char *root, FILE *file) {
    
    char str[LINE_SIZE];
    
    /* Går til startposition i filen: */
    rewind(file);

    do {
        fgets(str, LINE_SIZE, file);
        sscanf(str, "%[^|]", str);
        
        /* Hvis det ikke er en præbetingelse, at ordet findes i library:
        if (feof(library))
            break;*/
    } while (strcmp(str, root) != 0);
}