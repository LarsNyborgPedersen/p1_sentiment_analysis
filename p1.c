//this program was written by A413

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

char* choose_case(char *caseFileName);
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
	char * caseFileName = choose_case(caseFileName);
	FILE *caseFile, *synLib;
	caseFile = fopen(caseFileName, "r"),
	synLib = fopen("syn_lib.dat", "r");



    // Checking if the files has been opened
	if(caseFile != NULL && synLib != NULL){
		printf("%s\n", caseFileName);
	}
	else{
		printf("One or both files failed to load. Bye bye.\n");
        exit(EXIT_FAILURE);
	}

	get_and_clean_reviews(caseFile, roots, &sizeOfRootsArray);

	qsort(roots, sizeOfRootsArray, sizeof(root), compare);

    find_representatives(roots, sizeOfRootsArray, synLib);


	return 0;
}
// The user chooses a number, and then a specific string with the name of the file that is returned.
char* choose_case(char *caseFileName){
    int caseNumber;


    printf("Please write the number of which case you want. \n Shirt: 1 \n Toothbrush: 2 \n Choose a case: ");
	scanf("%d", &caseNumber);

    switch (caseNumber) {
        case 1: caseFileName = "shirt.txt"; break;
        case 2: caseFileName = "tooth.txt"; break;
    }

    return caseFileName;
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


void find_representatives(root roots[], int numberOfRoots, FILE *synLib) {
    int i = 0, j = 0, k = 0, synIndex = 0;
    char synonym[WORD_SIZE], synonymLine[LINE_SIZE];
    
    for (i = 0; i < numberOfRoots; i++) {
        find_root(roots[i].rootName, synLib);      

        /* Hvis ordet ikke blev fundet i vores bibliotek: 
        if (feof(synLib)) {
            rewind(synLib); continue;
        }*/
        
        do {                        
            fgets(synonymLine, LINE_SIZE, synLib);
            
            for (j = 1; roots[i].isRepresentative == TRUE && synonymLine[j] != '\n' && synonymLine[j] != '*'; j++, k = 0) {
                
                while (synonymLine[j] != '|')
                    synonym[k++] = synonymLine[j++];                    
                
                synonym[k] = '\0';
                
                synIndex = syn_in_array(synonym, roots, numberOfRoots);
                
                if (synIndex != FALSE && roots[i].count < roots[synIndex].count)
                    roots[i].isRepresentative = 0;
            }           
            
        } while (roots[i].isRepresentative == TRUE && synonymLine[j] != '*');
    }
    
}


int syn_in_array(char synonym[], root roots[], int numberOfRoots) {
    
    int first = 0,
        last = numberOfRoots - 1,
        middle;

    while (first <= last) {
        
        middle = (first + last) / 2;
        
        if (strcmp(roots[middle].rootName, synonym) < 0)
            first = middle + 1;
        else if (strcmp(roots[middle].rootName, synonym) > 0)
            last = middle - 1;
        else
            return middle;            
    }    
    
    if (first > last)
        return FALSE;
}


/*NB: Den her funktion antager, at roden findes i vores bibliotek(!) 
 Ellers kan man lave en indledende øvelse, så ordet i biblioteket skal matche inkl. pipen. */
void find_root(char *root, FILE *file) {
    int rootLength = strlen(root);
    char line[LINE_SIZE];
    
    do {
        fgets(line, LINE_SIZE, file);
    } while (strncmp(line, root, rootLength) != 0 && !feof(file));
}    
