/* this program was written by A413 */

/* ctype is used for the function isalpha */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#define WORD_SIZE 100
#define SYN_ARRAY_SIZE 32
#define LINE_SIZE 15000
#define ROOTS_ARRAY_SIZE 5000
#define CLUSTERS_SIZE 5000
#define FALSE -1
#define TRUE 1

typedef struct  {
	char rootName[WORD_SIZE];
	int count;
	int isRepresentative;
    int clusterCount;
} root;

void choose_case(char caseFileName[], int *linesToBeAnalyzed);
void clean_review_and_make_roots_array(char caseFileName[], root roots[], int *sizeOfRootsArray, int linesToBeAnalyzed);
void get_reviews_from_file(char caseFileName[], FILE *caseFileDirty, int linesToBeAnalyzed);
void clean_review(FILE *caseFileDirty, FILE *caseFileClean);
void scan_words_into_temp_array(FILE *caseFileClean, root rootsTemp[], int *sizeOfRootsArrayTemp);
void make_roots_array(root roots[], int *sizeOfRootsArray, root rootsTemp[], int sizeOfRootsArrayTemp, FILE *nounLib, FILE *nounExceptions);
int is_noun(FILE *nounLib, FILE *nounExceptions, char *word, fpos_t *posNoun, fpos_t *posExc);
int found_in_lib(char word[], FILE *lib, fpos_t *pos);
int found_in_lib_exc(char word[], FILE *lib, fpos_t *pos);
void convert_to_singular(char *word);
int index_of_existing_word(char *word, root roots[], int sizeOfRootsArray);
int compare(const void *p1, const void *p2);
void find_representatives(root roots[], int numberOfRoots, FILE *synLib);
int syn_in_array(char synonym[], root roots[], int numberOfRoots);
void go_to_first_syn_line(char *root, FILE *file);
int is_representative_and_not_end_of_line(root roots[], char synonymLine[], int lineIndex, int rootIndex);
void make_clusters(root *clusters[][SYN_ARRAY_SIZE], root *EndOfCluster, int *sizeOfClustersArray, root roots[], int sizeOfRootsArray, FILE *synLib);
int find_biggest_line(root roots[], int sizeOfRootsArray, FILE *synLib);
int compare_clusters(const void *p1, const void *p2);
int find_cluster_size (root *cluster[]);
void print_clusters(root *clusters[][SYN_ARRAY_SIZE], int sizeOfClustersArray);
void print_clusters2(root *clusters[][SYN_ARRAY_SIZE], int sizeOfClustersArray);
int find_synonym_in_roots(char synonymLine[], char synonym[], int *i, int *j, root roots[], int sizeOfRootsArray);

int main(void) {
	root roots[ROOTS_ARRAY_SIZE];
	root *clusters[CLUSTERS_SIZE][SYN_ARRAY_SIZE];
	root EndOfCluster = {"*EOC*", FALSE, FALSE, FALSE};
    int sizeOfRootsArray,
        sizeOfClustersArray;
    int linesToBeAnalyzed = 400;
    /* clock_t start,
            end;
    double cpu_time_used; */
	FILE *synLib = fopen("syn_lib.txt", "r");

	char caseFileName[WORD_SIZE];
	choose_case(caseFileName, &linesToBeAnalyzed);



    /*Tjekker om filen er åben*/
	if (synLib != NULL) {

        /* start = clock(); */

		clean_review_and_make_roots_array(caseFileName, roots, &sizeOfRootsArray, linesToBeAnalyzed);

        /* end = clock();
        cpu_time_used = ((double)(end - start) / CLOCKS_PER_SEC);
        printf("make_roots_arry uses %lf seconds\n", cpu_time_used);
        printf("sizeOfRootsArray = %d\n", sizeOfRootsArray);*/
        qsort(roots, sizeOfRootsArray, sizeof(root), compare);

        find_representatives(roots, sizeOfRootsArray, synLib);
        make_clusters(clusters, &EndOfCluster, &sizeOfClustersArray, roots, sizeOfRootsArray, synLib);

        qsort(clusters, sizeOfClustersArray, sizeof(clusters[0]), compare_clusters);

        print_clusters(clusters, sizeOfClustersArray);
	}
	else {
		printf("syn_lib.txt failed to load. Bye bye.\n");
        exit(EXIT_FAILURE);
	}

	return 0;
}
/* Brugeren vælger en case, som bruges i switchen til at vælge hvilken fil der skal åbnes */
void choose_case(char caseFileName[], int *linesToBeAnalyzed) {
    int caseNumber;

    printf("Please write the number of which case you want. \n 1: Reviews of musical intruments\n 2: Reviews of phones and accessories\n Choose a case:  ");
	scanf(" %d", &caseNumber);
    printf("Please write the number of reviews you want to analyze (maximum of 400 reviews)\n Choose amount of reviews: ");
    scanf(" %d", linesToBeAnalyzed);
	printf("\n");

    switch (caseNumber) {
        case 1:
			strcpy(caseFileName, "Musical_Instruments_5.txt");
			break;
        case 2:
			strcpy(caseFileName, "Cell_Phones_and_Accessories_5.txt");
			break;
		default:
			printf("Please try again...\n");
			exit(EXIT_FAILURE);
    }
}

/* Vi rengører vores streng, sorterer navneord og opretter dem i vores array*/
void clean_review_and_make_roots_array(char caseFileName[], root roots[], int *sizeOfRootsArray, int linesToBeAnalyzed) {
    FILE *nounLib = fopen("noun_lib.txt", "r"),
         *nounExceptions = fopen("noun_exc.txt", "r"),
         *caseFileDirty = fopen("caseFileDirty.txt", "w"),
         *caseFileClean = fopen("clean_review.txt", "w");
    root rootsTemp[ROOTS_ARRAY_SIZE];
    int sizeOfRootsArrayTemp = 0;
        *sizeOfRootsArray = 0;



	/*Tjekker om filerne er åben og kører de funktioner som skal bruges*/
    if (caseFileClean != NULL && caseFileDirty != NULL && nounLib != NULL && nounExceptions != NULL) {
		printf("Retrieving reviews...");
        get_reviews_from_file(caseFileName, caseFileDirty, linesToBeAnalyzed);
		printf(" Done!\n");

		printf("Cleaning reviews...  ");
        clean_review(caseFileDirty, caseFileClean);
		printf(" Done!\n");

		printf("Sorting reviews...   ");
        scan_words_into_temp_array(caseFileClean, rootsTemp, &sizeOfRootsArrayTemp);
        qsort(rootsTemp, sizeOfRootsArrayTemp, sizeof(root), compare);
        printf(" Done!\n");

		printf("Checking for nouns...");
        make_roots_array(roots, sizeOfRootsArray, rootsTemp, sizeOfRootsArrayTemp, nounLib, nounExceptions);
        printf(" Done!\n");


        fclose(nounLib);
        fclose(caseFileClean);

    }
    else {
        printf("clean_review.txt, caseFileDirty.txt, noun_lib.txt or noun_exc.txt failed to load. Bye bye.\n");
        exit(EXIT_FAILURE);
    }
}

/* Vi rengører vores streng, sorterer navneord og opretter dem i vores array*/
void get_reviews_from_file (char caseFileName[], FILE *caseFileDirty, int linesToBeAnalyzed) {
    char reviewLineTemp[LINE_SIZE];
    char reviewLine[LINE_SIZE];
    FILE *caseFile = fopen(caseFileName, "r");
    int i;
    if (caseFile != NULL) {
    	/*Vi scanner linjerne fra vores review kilde fil, og sætter ordene ind i et temp array*/
        for (i = 0; i < linesToBeAnalyzed; ++i) {
            fgets(reviewLineTemp, LINE_SIZE, caseFile);

            /*if (strlen(reviewLineTemp) > j) {
            j = strlen(reviewLineTemp);
            }*/

            /*fscanf(caseFile, "%[^\[]")*/
            sscanf(reviewLineTemp, "%*s %*s %*s %*s %*s %*[^,] %*[^0-9] %*d, %*d], %*s \"%[^\"]", reviewLine);
            /*sscanf(reviewLineTemp, "%*s %*s %*s %*s %*s %*[^0-9] %*d, %*d], %*s \"%[^\"]", reviewLine);*/

            /*sscanf(reviewLineTemp, "%*s %*s %*s %*s %*s %*[^0-9] %*d, %*d], %*s \"%[^\"]\" %*[^\n]\n", reviewLine);*/

            /*sscanf(reviewLineTemp, "%*[^[] %*[^:] \"%[^\"] %*[^\n]\n", reviewLine);*/
            fprintf(caseFileDirty, "%s\n", reviewLine);
        }
    }
    else {
        printf("%s failed to load. Bye Bye.\n", caseFileName);
        exit(EXIT_FAILURE);
    }
    fclose(caseFileDirty);
    fclose(caseFile);
        caseFileDirty = fopen("caseFileDirty.txt", "r");
}
/*Vi renser alle vores ord fra tegn og numre*/
void clean_review(FILE *caseFileDirty, FILE *caseFileClean) {
    int currentChar;
    if (caseFileDirty != NULL) {
        /*Vi indlæser hver eneste bogstav, indtil vores review fil er udtømt*/
        while (!feof(caseFileDirty)) {
    		/*Vi tjekker tegn for tegn om det er et stort bogstav eller et tegn, så bliver de lavet om eller fjernet*/
            currentChar = fgetc(caseFileDirty);
            if (isupper(currentChar)) {
                currentChar = tolower(currentChar);
            }
            if (isalpha(currentChar) || currentChar == ' ' || currentChar == '\n') {
                fputc(currentChar, caseFileClean);
            }
        }
    }
    else {
        printf("caseFileDirty.txt filed to load. Bye Bye.\n");
        exit(EXIT_FAILURE);
    }
    fclose(caseFileClean);
    caseFileClean = fopen("clean_review.txt", "r");
    fclose(caseFileDirty);
}
/*Vi scanner vores ord over i et array af structs*/
void scan_words_into_temp_array(FILE *caseFileClean, root rootsTemp[], int *sizeOfRootsArrayTemp) {
    int indexExistingWord = -1;
    int scanRes;
    char word[WORD_SIZE];

    while (!feof(caseFileClean)) {
        scanRes = fscanf(caseFileClean, " %s ", word);
		/*Hvis vi har scannet array succesfuldt*/
        if (scanRes == 1) {
			/*Vi finder indexet i vores array*/
            indexExistingWord = index_of_existing_word(word, rootsTemp, *sizeOfRootsArrayTemp);

            if (indexExistingWord == FALSE) {
				/*Hvis ordet ikke står i vores array, så oprettet vi ordet som et nyt element*/
                /* printf("%s FALSE!!!\n", word); */
                strcpy(rootsTemp[*sizeOfRootsArrayTemp].rootName, word);
                rootsTemp[*sizeOfRootsArrayTemp].count = 1;
                (*sizeOfRootsArrayTemp)++;
            }
            else {
				/*Ellers tæller vi hyppigheden op*/
                rootsTemp[indexExistingWord].count++;
            }
        }
    }
}

void make_roots_array(root roots[], int *sizeOfRootsArray, root rootsTemp[], int sizeOfRootsArrayTemp, FILE *nounLib, FILE *nounExceptions) {
    int i;
    int indexExistingWord = -1;
    fpos_t posNoun, posExc;

    /* rewind(nounLib);
        rewind(nounExceptions); */
    fgetpos(nounLib, &posNoun);
    fgetpos(nounExceptions, &posExc);


    for (i = 0; i < sizeOfRootsArrayTemp; i++) {
        if (is_noun(nounLib, nounExceptions, rootsTemp[i].rootName, &posNoun, &posExc)) {
            indexExistingWord = index_of_existing_word(rootsTemp[i].rootName, roots, *sizeOfRootsArray);

            if (indexExistingWord == FALSE) {
                strcpy(roots[*sizeOfRootsArray].rootName, rootsTemp[i].rootName);
                roots[*sizeOfRootsArray].count = rootsTemp[i].count;
                roots[*sizeOfRootsArray].isRepresentative = TRUE;
                (*sizeOfRootsArray)++;
            }
            else {
                roots[indexExistingWord].count += rootsTemp[i].count;
            }
        }
    }
}

/* Checks whether the word is a noun */
int is_noun(FILE *nounLib, FILE *nounExceptions, char word[], fpos_t *posNoun, fpos_t *posExc) {
    int isNoun = 0;

    if (found_in_lib(word, nounLib, posNoun)) {
        isNoun = 1;
    }
    else if (found_in_lib_exc(word, nounExceptions, posExc)) {
        isNoun = 1;
    }
    else {
        convert_to_singular(word);
        if (found_in_lib(word, nounLib, posNoun)) {
            isNoun = 1;
        }
    }
    return isNoun;
}

int found_in_lib(char word[], FILE *lib, fpos_t *pos) {
    char tempNoun[100];
	char firstCharOfTempNoun,
        firstCharOfWord;
    int isNoun = 0;
	firstCharOfWord = word[0];

    fsetpos(lib, pos);

    do {
        fgets(tempNoun, 100, lib);
        tempNoun[strlen(tempNoun) - 1] = '\0';
		firstCharOfTempNoun = tempNoun[0];

        if (strcmp(word, tempNoun) == 0) {
            fseek(lib, -(strlen(tempNoun) + 2), SEEK_CUR);
            fgetpos(lib, pos);
            isNoun = 1;
            return isNoun;
        }
    } while (!feof(lib) && firstCharOfTempNoun <= firstCharOfWord);
    return isNoun;
}

int found_in_lib_exc(char word[], FILE *lib, fpos_t *pos) {
    int isNoun = 0;
    char tempNouns[100];  /* longest line in noun_exc.txt is 46 characters + '\n' (line 1003)...... no the longest line is 69 in lenght */
    char tempSingular[100]; /* longest word is 23 letters (line 1003) */
    char tempPlural[100];
	char firstCharOfTempNoun,
        firstCharOfWord;
	firstCharOfWord = word[0];

    rewind(lib);
    do {
        fgets(tempNouns, 100, lib);
		firstCharOfTempNoun = tempNouns[0];
        sscanf(tempNouns, "%s %s", tempPlural, tempSingular);
        if (strcmp(word, tempPlural) == 0) {
            /* fseek(lib, -(strlen(tempNouns) + 1), SEEK_CUR); */
            strcpy(word, tempSingular);
            isNoun = 1;
            return isNoun;
        }
    } while (!feof(lib) && (firstCharOfTempNoun == firstCharOfWord));
    return isNoun;
}

void convert_to_singular(char *word) {
    /* Make word singular (if it wasn't an exceptions to normal noun rules)
     * if end of word = "ches" make it = "ch" */

    /* printf("word before: %s\n", word); */


    if (strcmp(&word[strlen(word) - 4], "ches") == 0) {
        strcpy(&word[strlen(word) - 4], "ch");
    }
    /* if end of word = "shes" make it = "sh" */
    else if (strcmp(&word[strlen(word) - 4], "shes") == 0) {
        strcpy(&word[strlen(word) - 4], "sh");
    }
    /* if end of word = "men" make it = "man" */
    else if (strcmp(&word[strlen(word) - 3], "men") == 0) {
        strcpy(&word[strlen(word) - 3], "man");
    }
    /* if end of word = "ses" make it = "s" */
    else if (strcmp(&word[strlen(word) - 3], "ses") == 0) {
        strcpy(&word[strlen(word) - 3], "s");
    }
    /* if end of word = "xes" make it = "x" */
    else if (strcmp(&word[strlen(word) - 3], "xes") == 0) {
        strcpy(&word[strlen(word) - 3], "x");
    }
    /* if end of word = "zes" make it = "z" */
    else if (strcmp(&word[strlen(word) - 3], "zes") == 0) {
        strcpy(&word[strlen(word) - 3], "z");
    }
    /* if end of word = "ies" make it = "y" */
    else if (strcmp(&word[strlen(word) - 3], "ies") == 0) {
        strcpy(&word[strlen(word) - 3], "y");
    }
    /* if end of word = "s" make it = "" */
    else if (strcmp(&word[strlen(word) - 1], "s") == 0) {
        word[strlen(word) - 1] = '\0';
    }
    /* printf("word after: %s\n", word); */
}

int index_of_existing_word(char *word, root roots[], int sizeOfRootsArray) {
    int index = -1;
    int i;

    for (i = 0; i < sizeOfRootsArray; i++) {
        if (strcmp(roots[i].rootName, word) == 0) {
            return i;
        }
    }

    /* index is the index where the words exist, and should be -1 if it doesn't eixst. */
    return index;
}

int compare(const void *p1, const void *p2) {
    root *root1 = (root *) p1,
         *root2 = (root *) p2;

    return strcmp(root1->rootName, root2->rootName);
}

int syn_in_array(char synonym[], root roots[], int numberOfRoots) {
    int result = 0;
    int first = 0,
        last = numberOfRoots - 1,
        middle;

    while (first <= last) {

        middle = (first + last) / 2;

        if (strcmp(roots[middle].rootName, synonym) < 0){
            first = middle + 1;
        }
        else if (strcmp(roots[middle].rootName, synonym) > 0){
            last = middle - 1;
        }
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
void go_to_first_syn_line(char *root, FILE *file) {
    int rootLength = strlen(root);
    char line[LINE_SIZE];

    do {
        fgets(line, LINE_SIZE, file);
    } while ((line[rootLength] != '|' || strncmp(line, root, rootLength)) != 0 && !feof(file));
}

int is_representative_and_not_end_of_line(root roots[], char synonymLine[], int lineIndex, int rootIndex) {
	return roots[rootIndex].isRepresentative == TRUE && synonymLine[lineIndex] != '\n' && synonymLine[lineIndex] != '*';
}

void find_representatives(root roots[], int sizeOfRootsArray, FILE *synLib) {
    int rootIndex = 0, j = 0, wordIndex = 0, synIndex = 0;
    char synonym[WORD_SIZE], synonymLine[LINE_SIZE];

    for (rootIndex = 0; rootIndex < sizeOfRootsArray; rootIndex++) {
        go_to_first_syn_line(roots[rootIndex].rootName, synLib);

        /* Hvis ordet ikke blev fundet i vores bibliotek: */
        if (feof(synLib)) {

            roots[rootIndex].isRepresentative = 0;
            rewind(synLib); continue;
        }

        do {
            fgets(synonymLine, LINE_SIZE, synLib);

            for (j = 1, wordIndex = 0; roots[rootIndex].isRepresentative == TRUE && synonymLine[j] != '\n' && synonymLine[j] != '*'; j++, wordIndex = 0) {

                synIndex = find_synonym_in_roots(synonymLine, synonym, &j, &wordIndex, roots, sizeOfRootsArray);

                if (synIndex != FALSE && roots[rootIndex].count < roots[synIndex].count) {
                    roots[rootIndex].isRepresentative = 0;
                }
            }

        } while (roots[rootIndex].isRepresentative == TRUE && synonymLine[j] != '*');
    }

}

/* Vi samler største synonymklynge for hvert mulig repræsentant, derefter kopierer vi den ind i vores clusters array */
void make_clusters(root *clusters[][SYN_ARRAY_SIZE], root *EndOfCluster, int *sizeOfClustersArray, root roots[], int sizeOfRootsArray, FILE *synLib) {

    int i = 0, j = 0, k = 0, membersIndex = 0, clusterIndex = 0, synIndex = 0, biggestLineN = 0;
    char synonymLine[LINE_SIZE], synonym[WORD_SIZE];
    /*Vi spoler tilbage i vores fil efter find_representatives*/
    rewind(synLib);

	printf("Finding synonyms...  ");

    /*Vi tjekker vores array for repræsentanter*/
    for (i = 0, clusterIndex = 0, membersIndex = 0; i < sizeOfRootsArray; i++, membersIndex = 0) {
        if (roots[i].isRepresentative) {
            /*Hvis det er en repræsentant, så tilføjer vi det til en cluster*/
            clusters[clusterIndex][membersIndex++] = &roots[i];
            /*Vi finder vores rod i synonym biblioteket*/
            go_to_first_syn_line(roots[i].rootName, synLib);

            /* Returnerer linjetal af største synonymlinje. */
            biggestLineN = find_biggest_line(roots, sizeOfRootsArray, synLib);

            /*Hvis biggest line er lig 0, så er der ingen synonymlinjer*/
            if (biggestLineN != 0) {
                /*Vi henter vores linje*/
                for (j = 0; j < biggestLineN; j++) {
                    fgets(synonymLine, LINE_SIZE, synLib);
                }

                /*Kører loopet indtil linejn er slut med newline eller "*" */
                for (j = 1, k = 0; synonymLine[j] != '\n' && synonymLine[j] != '*'; j++, k = 0) {

                    synIndex = find_synonym_in_roots(synonymLine, synonym, &j, &k, roots, sizeOfRootsArray);

                    /*Hvis ordet er i vores array, så tilføjer vi det til vores cluster*/
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

	printf(" Done!\n");
}

int find_biggest_line(root roots[], int sizeOfRootsArray, FILE *synLib) {

    int currentBiggestSum = 0, tempSum = 0, biggestLineNr = 0, i = 0, j = 0, synIndex = 0, currentLineNr = 0;
    char synonymLine[LINE_SIZE], synonym[WORD_SIZE];

    /*Vi gemmer position inden vi henter linjer*/
    fpos_t synPosition;
    fgetpos(synLib, &synPosition);

    do {
        fgets(synonymLine, LINE_SIZE, synLib);
        currentLineNr++;
        /*Vi kører indtil linjen er slut*/
        tempSum = 0;
        for (i = 1, j = 0; synonymLine[i] != '\n' && synonymLine[i] != '*'; i++, j = 0) {
            synIndex = find_synonym_in_roots(synonymLine, synonym, &i, &j, roots, sizeOfRootsArray);

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
    FILE *fp = fopen ("aspects.csv", "w");

    printf(" All steps are done! Displaying results...\n\n\n Printing top 10 aspects:\n");


    /*Vi printer indtil array er udtømt eller de første 10 elementer*/
    for (i = 0; (i < 10) && (i < sizeOfClustersArray); i++) {
        fprintf(fp,"%s;%d\n", clusters[i][0]->rootName, clusters[i][0]->clusterCount);
        printf(" Aspect: %-10s Mentioned %3d times\n", clusters[i][0]->rootName, clusters[i][0]->clusterCount);
    }
	printf("\n There has been made a CSV file in the program's folder. \n You can open it in Excel and make a graph easily.\n");
}
void print_clusters2(root *clusters[][SYN_ARRAY_SIZE], int sizeOfClustersArray) {
    int i = 0;
    FILE *fp = fopen ("aspects.csv", "w");

    printf("100%%: Done! Displaying results...\n");


    /*Vi printer indtil array er udtømt eller de første 10 elementer*/
    for (i = 0; (i < sizeOfClustersArray); i++) {
        fprintf(fp,"%s;%d\n", clusters[i][0]->rootName, clusters[i][0]->clusterCount);
        printf("navn: %s. hyppighed: %d\n", clusters[i][0]->rootName, clusters[i][0]->clusterCount);
    }
    printf("sizeOfClustersArray %d\n", sizeOfClustersArray);
}

int find_synonym_in_roots(char synonymLine[], char synonym[], int *i, int *j, root roots[], int sizeOfRootsArray) {
    /*Vi tjekker hvert ord, og tæller hvor langt det er*/
    while (synonymLine[*i] != '|') {
        synonym[(*j)++] = synonymLine[(*i)++];
    }
    /*Vi nul afslutter ordet*/
    synonym[*j] = '\0';

    /*Vi tjekker om det er i vores synonym aray*/
    return syn_in_array(synonym, roots, sizeOfRootsArray);
}
