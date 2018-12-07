/* this program was written by A413 */

/* ctype is used for the function isalpha */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define WORD_SIZE 100
#define SYN_ARRAY_SIZE 32
#define LINE_SIZE 1000
#define ROOTS_ARRAY_SIZE 1000
#define CLUSTERS_SIZE 400
#define FALSE -1
#define TRUE 1

typedef struct  {
	char rootName[WORD_SIZE];
	int count;
	int isRepresentative;
    int clusterCount;
} root;

void choose_case(char caseFileName[]); 
void make_roots_array(char caseFileName[], root roots[], root roots2[], int *sizeOfRootsArray, int *sizeOfRootsArray2);
void cleanReview(FILE *caseFile, FILE *caseFileClean);
int is_noun(FILE *nounLib, FILE *nounExceptions, char *word, fpos_t *posNoun, fpos_t *posExc);
/* char *convert_to_singular(char *word); */
void find_root(char *root, FILE *library);
int index_of_existing_word(char *word, root roots[], int sizeOfRootsArray);
int compareLALA(const void *p1, const void *p2);
int compare(const void *p1, const void *p2);
void find_representatives(root roots[], int numberOfRoots, FILE *synLib);
int syn_in_array(char synonym[], root roots[], int numberOfRoots);
void find_root(char *root, FILE *file);
void make_clusters(root *clusters[][SYN_ARRAY_SIZE], root *EndOfCluster, int *sizeOfClustersArray, root roots[], int sizeOfRootsArray, FILE *synLib);
int find_biggest_line(char *rootName, root roots[], int sizeOfRootsArray, FILE *synLib);
int compare_clusters(const void *p1, const void *p2);
int find_cluster_size (root *cluster[]);
void print_clusters(root *clusters[][SYN_ARRAY_SIZE], int sizeOfClustersArray);



int main(void) {
	root roots[ROOTS_ARRAY_SIZE];
	root roots2[ROOTS_ARRAY_SIZE];
	root *clusters[CLUSTERS_SIZE][SYN_ARRAY_SIZE];
	root EndOfCluster = { "*EOC*", FALSE, FALSE, FALSE};
    int sizeOfRootsArray,
    	sizeOfRootsArray2,
        sizeOfClustersArray;
	FILE *synLib = fopen("syn_lib.dat", "r");

	char caseFileName[WORD_SIZE];
	choose_case(caseFileName);

    
	
    /*  Checking if the files has been opened */
	if (synLib != NULL) {

		make_roots_array(caseFileName, roots, roots2, &sizeOfRootsArray, &sizeOfRootsArray2);

        qsort(roots, sizeOfRootsArray, sizeof(root), compare);

        find_representatives(roots, sizeOfRootsArray, synLib);

        make_clusters(clusters, &EndOfCluster, &sizeOfClustersArray, roots, sizeOfRootsArray, synLib);

        print_clusters(clusters, sizeOfClustersArray);
	}
	else {
		printf("File failed to load. Bye bye.\n");
        exit(EXIT_FAILURE);
	}

	return 0;
}
/*  The user chooses a number, and then a specific string with the name of the file is returned with output parameter */
void choose_case(char caseFileName[]) {
    int caseNumber;

    printf("Please write the number of which case you want. \n Shirt: 1 \n Toothbrush: 2 \n Choose a case: ");
	scanf("%d", &caseNumber);

    switch (caseNumber) {
        case 1: 
        	strcpy(caseFileName, "shirt.txt");
        	break;
        case 2: 
        	strcpy(caseFileName, "tooth.txt");
        	break;
    }
}

/* receives a FILE pointer. */
/* Makes a clean string (with wordnet) with a review in it, and calls the other functions with each individual word. */
void make_roots_array(char caseFileName[], root roots[], root roots2[], int *sizeOfRootsArray, int *sizeOfRootsArray2) {
    FILE *nounLib = fopen("noun_lib.dat", "r"),
         *nounExceptions = fopen("noun_exc.txt", "r"),
         *caseFile = fopen(caseFileName, "r"),
         *caseFileClean = fopen("clean_review.txt", "w+");
    fpos_t posNoun, posExc;
    int i;
    
    int scanRes;
    char word[WORD_SIZE];
    *sizeOfRootsArray = 0;
    *sizeOfRootsArray2 = 0;


    
    fgetpos(nounLib, &posNoun);
    fgetpos(nounExceptions, &posExc);
    /* caseFile = fopen("test.txt", "r");           til debugging*/

    if (caseFileClean != NULL) {



        cleanReview(caseFile, caseFileClean);

        do {
            scanRes = fscanf(caseFileClean, "%s", word);
            //printf("%s %d\n", word, is_noun(nounLib, nounExceptions, word, &posNoun, &posExc));
            if (scanRes == 1) {
                int indexExistingWord = index_of_existing_word(word, roots, *sizeOfRootsArray);

                if (indexExistingWord == FALSE) {
                	//printf("%s FALSE!!!\n", word);
                    strcpy(roots[*sizeOfRootsArray].rootName, word);
                    roots[*sizeOfRootsArray].count = 1;
                    roots[*sizeOfRootsArray].isRepresentative = TRUE;
                    (*sizeOfRootsArray)++;
                }
                else {
                	//printf("%s TRUE!!!\n", word);
                    roots[indexExistingWord].count++;
                }
            }
        } while (scanRes == 1);

        qsort(roots, *sizeOfRootsArray, sizeof(root), compareLALA);

        for (i = 0; i < *sizeOfRootsArray; ++i){
        	printf("%d %s\n", i, roots[i].rootName);
        }

        for (i = 0; i < *sizeOfRootsArray; i++) {
        	if (is_noun(nounLib, nounExceptions, roots[i].rootName, &posNoun, &posExc)) {
                int indexExistingWord = index_of_existing_word(roots[i].rootName, roots2, *sizeOfRootsArray2);


                if (indexExistingWord == FALSE) {
                	printf("%s FALSE!!!\n", roots[i].rootName);
                    strcpy(roots2[*sizeOfRootsArray2].rootName, roots[i].rootName);
                    roots2[*sizeOfRootsArray2].count = 1;
                    roots2[*sizeOfRootsArray2].isRepresentative = TRUE;
                    (*sizeOfRootsArray2)++;
                }
                else {
                	printf("%s TRUE!!!\n", roots[i].rootName);
                    roots[indexExistingWord].count++;
                }
            }
        }
/*
        do {
            scanRes = fscanf(caseFileClean, "%s", word);
            printf("%s %d\n", word, is_noun(nounLib, nounExceptions, word, &posNoun, &posExc));
            if (scanRes == 1 && is_noun(nounLib, nounExceptions, word, &posNoun, &posExc)) {
                int indexExistingWord = index_of_existing_word(word, roots, *sizeOfRootsArray);

                if (indexExistingWord == FALSE) {
                	printf("%s FALSE!!!\n", word);
                    strcpy(roots[*sizeOfRootsArray].rootName, word);
                    roots[*sizeOfRootsArray].count = 1;
                    roots[*sizeOfRootsArray].isRepresentative = TRUE;
                    (*sizeOfRootsArray)++;
                }
                else {
                	printf("%s TRUE!!!\n", word);
                    roots[indexExistingWord].count++;
                }
            }
        } while (scanRes == 1);

*/

        for (i = 0; i < *sizeOfRootsArray2; ++i){
        	printf("%d %s\n", i, roots2[i].rootName);
        }
        

        fclose(nounLib);
        fclose(caseFileClean);
    }
    else {
        printf("One or both files failed to load. Bye bye.\n");
        exit(EXIT_FAILURE);
    }
}

void cleanReview(FILE *caseFile, FILE *caseFileClean) {
    int currentChar;

    while (!feof(caseFile)) {
        currentChar = fgetc(caseFile);

        if (isalpha(currentChar) || currentChar == ' ') {
            fputc(currentChar, caseFileClean);
        }
    }
    fclose(caseFile);
    rewind(caseFileClean);
}

/* Checks whether the word is a noun */
int is_noun(FILE *nounLib, FILE *nounExceptions, char *word, fpos_t *posNoun, fpos_t *posExc) {
    int is_noun = 0;
    char tempNoun[25];
    char tempAll[48]; //longest line in noun_exc.dat is 46 characters + '\n' (line 1003)
    char tempSingular[25]; //longest word is 23 letters (line 1003)
    char tempPlural[25];
    //fgetpos(nounLib, &posNoun);
    //Check if word is in nounLib (singular nouns)
    //rewindLine(nounLib);
    

    fsetpos(nounLib, posNoun);
    while (!feof(nounLib)) {
        fgets(tempNoun, 30, nounLib);
        //printf("%s", tempNoun);
        tempNoun[strlen(tempNoun) - 1] = 0;
        if (strcmp(word, tempNoun) == 0) {
            fseek(nounLib, -(strlen(tempNoun) + 2), SEEK_CUR);
            fgetpos(nounLib, posNoun);
            printf("Word was found in nounLib first time\n");
            is_noun = 1;
            //printf("1 %d\n", is_noun);
            return is_noun;
        }
    }
    //printf("HEY HEY HEY HYE\n");
    //fgetpos(nounExceptions, &posExc);
    //Check if word is in nounExceptions, and if it is, make it singular
    //rewindLine(nounExceptions);
    fsetpos(nounExceptions, posExc);
    while (!feof(nounExceptions)) {
        fgets(tempAll, 48, nounExceptions);
        sscanf(tempAll, "%s %s", tempPlural, tempSingular);
        //printf("%s %s %s\n", tempAll, tempSingular, tempPlural);
        if (strcmp(word, tempPlural) == 0) {
            fgetpos(nounExceptions, posExc);
            fseek(nounExceptions, -(strlen(tempAll) + 1), SEEK_CUR);
            printf("Word was found in exceptions list!\n");
            strcpy(word, tempSingular);
            is_noun = 1;
            //printf("2 %d\n", is_noun);
            return is_noun;
        }
    }
    //Make word singular (if it wasn't an exceptions to normal noun rules)
    //if end of word = "ches" make it = "ch"
    if (strcmp(&word[strlen(word) - 4], "ches") == 0) {
        strcpy(&word[strlen(word) - 4], "ch");
    }
    //if end of word = "shes" make it = "sh"
    else if (strcmp(&word[strlen(word) - 4], "shes") == 0) {
        strcpy(&word[strlen(word) - 4], "sh");
    }
    //if end of word = "men" make it = "man"
    else if (strcmp(&word[strlen(word) - 3], "men") == 0) {
        strcpy(&word[strlen(word) - 3], "man");
    }
    //if end of word = "ses" make it = "s"
    else if (strcmp(&word[strlen(word) - 3], "ses") == 0) {
        strcpy(&word[strlen(word) - 3], "s");
    }
    //if end of word = "xes" make it = "x"
    else if (strcmp(&word[strlen(word) - 3], "xes") == 0) {
        strcpy(&word[strlen(word) - 3], "x");
    }
    //if end of word = "zes" make it = "z"
    else if (strcmp(&word[strlen(word) - 3], "zes") == 0) {
        strcpy(&word[strlen(word) - 3], "z");
    }
    //if end of word = "ies" make it = "y"
    else if (strcmp(&word[strlen(word) - 3], "ies") == 0) {
        strcpy(&word[strlen(word) - 3], "y");
    }
    //if end of word = "s" make it = ""
    else if (strcmp(&word[strlen(word) - 1], "s") == 0) {
        word[strlen(word) - 1] = 0;
    }
    
    //Check if word is in nounLib (singular nouns)
    fsetpos(nounLib, posNoun);
    //rewindLine(nounLib);
    while (!feof(nounLib)) {
        fgets(tempNoun, 30, nounLib);
        tempNoun[strlen(tempNoun) - 1] = 0;
        if (strcmp(word, tempNoun) == 0) {
            fseek(nounLib, -(strlen(tempNoun) + 2), SEEK_CUR);
            fgetpos(nounLib, posNoun);
            printf("Word was found in nounLib second time\n");
            is_noun = 1;
            //printf("3 %d\n", is_noun);
            return is_noun;
        }
    }
    strcpy(&word[strlen(word)], "e");
    fsetpos(nounLib, posNoun);
    //rewindLine(nounLib);
    while (!feof(nounLib)) {
        fgets(tempNoun, 30, nounLib);
        tempNoun[strlen(tempNoun) - 1] = 0;
        if (strcmp(word, tempNoun) == 0) {
            fseek(nounLib, -(strlen(tempNoun) + 2), SEEK_CUR);
            fgetpos(nounLib, posNoun);
            printf("Word was found in nounLib third time\n");
            is_noun = 1;
            //printf("4 %d\n", is_noun);
            return is_noun;
        }
    }

    printf("Word was not found in any file\n");
    is_noun = 0;
    //printf("5 %d\n", is_noun);
    return is_noun;
}

int index_of_existing_word(char *word, root roots[], int sizeOfRootsArray) {
    int index = -1;
    int i;

    for (i = 0; i < sizeOfRootsArray; i++) {
        if (strcmp(roots[i].rootName, word) == 0) {
            index = i;
            
        }
    }

    /* index is the index where the words exist, and should be -1 if it doesn't eixst. */
    return index;
}

int compareLALA(const void *p1, const void *p2) {

    root *root1 = (root *) p1,
           *root2 = (root *) p2;

    return strcmp(root1->rootName, root2->rootName);
}



int compare(const void *p1, const void *p2) {

    root *root1 = (root *) p1,
           *root2 = (root *) p2;

    return strcmp(root1->rootName, root2->rootName);
}


void find_representatives(root roots[], int numberOfRoots, FILE *synLib) {
    int i = 0, j = 0, k = 0, synIndex = 0;
    char synonym[WORD_SIZE], synonymLine[LINE_SIZE];

    for (i = 0; i < numberOfRoots; i++) {
        find_root(roots[i].rootName, synLib);

        /* Hvis ordet ikke blev fundet i vores bibliotek: */
        if (feof(synLib)) {
            roots[i].isRepresentative = 0;
            rewind(synLib); continue;
        }

        do {
            fgets(synonymLine, LINE_SIZE, synLib);

            for (j = 1, k = 0; roots[i].isRepresentative == TRUE && synonymLine[j] != '\n' && synonymLine[j] != '*'; j++, k = 0) {

                while (synonymLine[j] != '|') {
                    synonym[k++] = synonymLine[j++];
                }

                synonym[k] = '\0';

                synIndex = syn_in_array(synonym, roots, numberOfRoots);

                if (synIndex != FALSE && roots[i].count < roots[synIndex].count) {
                    roots[i].isRepresentative = 0;
                }
            }

        } while (roots[i].isRepresentative == TRUE && synonymLine[j] != '*');
    }

}

int syn_in_array(char synonym[], root roots[], int numberOfRoots) {
    int result = 0;
    int first = 0,
        last = numberOfRoots - 1,
        middle;

    while (first <= last) {

        middle = (first + last) / 2;

        if (strcmp(roots[middle].rootName, synonym) < 0)
            first = middle + 1;
        else if (strcmp(roots[middle].rootName, synonym) > 0)
            last = middle - 1;
        else{
            result = middle;
            break;
        }
    }

    if (first > last)
        result = FALSE;

    return result;
}

/*NB: Den her funktion antager, at roden findes i vores bibliotek(!)
 Ellers kan man lave en indledende øvelse, så ordet i biblioteket skal matche inkl. pipen. */
void find_root(char *root, FILE *file) {
    int rootLength = strlen(root);
    char line[LINE_SIZE];

    do {
        fgets(line, LINE_SIZE, file);
    } while ((line[rootLength] != '|' || strncmp(line, root, rootLength)) != 0 && !feof(file));
}

/* Vi samler største synonymklynge for hvert mulig repræsentant, derefter kopierer vi den ind i vores clusters array */
void make_clusters(root *clusters[][SYN_ARRAY_SIZE], root *EndOfCluster, int *sizeOfClustersArray, root roots[], int sizeOfRootsArray, FILE *synLib) {

    int i = 0, j = 0, k = 0, membersIndex = 0, clusterIndex = 0, synIndex = 0, biggestLineN = 0;
    char synonymLine[LINE_SIZE], synonym[WORD_SIZE];
    /*Vi spoler tilbage i vores fil efter find_representatives*/
    rewind(synLib);

    /*Vi tjekker vores array for repræsentanter*/
    for (i = 0, clusterIndex = 0, membersIndex = 0; i < sizeOfRootsArray; i++, membersIndex = 0) {
        if (roots[i].isRepresentative) {
            /*Hvis det er en repræsentant, så tilføjer vi det til en cluster*/
            clusters[clusterIndex][membersIndex++] = &roots[i];
            /*Vi finder vores rod i synonym biblioteket*/
            find_root(roots[i].rootName, synLib);

            /* Returnerer linjetal af største synonymlinje. */
            biggestLineN = find_biggest_line(roots[i].rootName, roots, sizeOfRootsArray, synLib);

            /*Hvis biggest line er lig 0, så er der ingen synonymlinjer*/
            if (biggestLineN != 0) {
                /*Vi henter vores linje*/
                for (j = 0; j < biggestLineN; j++) {
                    fgets(synonymLine, LINE_SIZE, synLib);
                }

                /*Kører loopet indtil linejn er slut med newline eller "*" */
                for (j = 1, k = 0; synonymLine[j] != '\n' && synonymLine[j] != '*'; j++, k = 0) {
                    /*Vi tjekker hvert ord*/
                    while (synonymLine[j] != '|') {
                        synonym[k++] = synonymLine[j++];
                    }
                    /*Vi nul slutter ordet*/
                    synonym[k] = '\0';

                    /*Tjekker om ordet er i vores synonym array*/
                    synIndex = syn_in_array(synonym, roots, sizeOfRootsArray);

                    /*Hvis det er, så tilføjer vi det til vores cluster*/
                    if (synIndex != FALSE) {
                        clusters[clusterIndex][membersIndex++] = &roots[synIndex];
                    }
                }
            }
            /* Vi tildeler EndOfCluster som sidste elemtent og tildeler klyngestrørrelsen på repræsentantens felt*/
            clusters[clusterIndex][membersIndex] = EndOfCluster;
            clusters[clusterIndex][0]->clusterCount = find_cluster_size(clusters[clusterIndex]);
            clusterIndex++;
        }
    }
    /*Vi tæller størrelsen af vores cluster array op*/
    *sizeOfClustersArray = clusterIndex;
}

int find_biggest_line(char *rootName, root roots[], int sizeOfRootsArray, FILE *synLib) {

    int currentBiggestSum = 0, tempSum = 0, biggestLineNr = 0, i = 0, j = 0, synIndex = 0, currentLineNr = 0;
    char synonymLine[LINE_SIZE], synonym[WORD_SIZE];

    /*Vi gemmer position inden vi henter linjer*/
    fpos_t synPosition;
    fgetpos(synLib, &synPosition);

    do {
        fgets(synonymLine, LINE_SIZE, synLib);
        currentLineNr++;
        /*Vi kører indtil linjen er slut*/
        for (i = 1, j = 0; synonymLine[i] != '\n' && synonymLine[i] != '*'; i++, j = 0) {
            /*Vi tjekker hvert ord, og tæller hvor langt det er*/
            while (synonymLine[i] != '|') {
                synonym[j++] = synonymLine[i++];
            }

            synonym[j] = '\0';
            /*Vi tjekker om det er i vores synonym aray*/
            synIndex = syn_in_array(synonym, roots, sizeOfRootsArray);
            /*Vi tæller summen for hele linjen op*/
            if (synIndex != FALSE) {
                tempSum += roots[synIndex].count;
            }
        }
        /*Vi tjekker om denne linje er større end den tidligere linje*/
        if (tempSum > currentBiggestSum) {
            currentBiggestSum = tempSum;
            biggestLineNr = currentLineNr;
        }

    } while (synonymLine[i] != '*');

    /*Vi går tilbage til vores gemte position*/
    fsetpos(synLib, &synPosition);

    return biggestLineNr;
}

int compare_clusters(const void *p1, const void *p2) {
  
    root  **cluster1 = (root **) p1,
          **cluster2 = (root **) p2;

    /*Hvis de ikke er lig hinanden, så sorterer vi efter hyppighed, ellers alfabetisk*/
    if (cluster1[0]->clusterCount != cluster2[0]->clusterCount)
        return (cluster1[0]->clusterCount > cluster2[0]->clusterCount) ? -1 : 1;
    else
        return strcmp(cluster1[0]->rootName, cluster2[0]->rootName);
}

int find_cluster_size (root *cluster[]) {
    int i = 0, sum = 0;

    /*Vi kører loopet indtil arrayet er slut, og adderer pointene sammen*/
    for (i = 0; cluster[i]->count != FALSE; i++) {
        sum += cluster[i]->count;
    }

    return sum;
}

void print_clusters(root *clusters[][SYN_ARRAY_SIZE], int sizeOfClustersArray) {
    int i = 0;
    FILE *fp = fopen ("aspekt.csv", "w");

    /*Vi printer indtil array er udtømt eller de første 10 elementer*/
    for (i = 0; (i < 10) && (i < sizeOfClustersArray); i++) {
        fprintf(fp,"%s;%d\n", clusters[i][0]->rootName, clusters[i][0]->clusterCount);
        printf("navn: %s. hyppighed: %d\n", clusters[i][0]->rootName, clusters[i][0]->clusterCount);
    }
}
