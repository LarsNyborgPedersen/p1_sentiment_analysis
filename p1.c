/* this program was written by A413 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#define LINES_TO_BE_ANALYSED 700
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

void choose_case(char caseFileName[]);
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
int word_in_roots_array(char synonym[], root roots[], int numberOfRoots);
void go_to_first_syn_line(char *root, FILE *file);
int is_representative_and_not_end_of_line(root roots[], char synonymLine[], int lineIndex, int rootIndex);
void make_clusters(root *clusters[][SYN_ARRAY_SIZE], root *EndOfCluster, int *sizeOfClustersArray, root roots[], int sizeOfRootsArray, FILE *synLib);
int find_biggest_line(root roots[], int sizeOfRootsArray, FILE *synLib);
int compare_clusters(const void *p1, const void *p2);
int find_cluster_size (root *cluster[]);
void print_clusters(root *clusters[][SYN_ARRAY_SIZE], int sizeOfClustersArray);
int find_synonym_in_roots(char synonymLine[], char synonym[], int *i, int *j, root roots[], int sizeOfRootsArray);

int main(void) {
	root roots[ROOTS_ARRAY_SIZE];
	root *clusters[CLUSTERS_SIZE][SYN_ARRAY_SIZE];
	root EndOfCluster = {"*EOC*", FALSE, FALSE, FALSE};
    int sizeOfRootsArray,
        sizeOfClustersArray;
    int linesToBeAnalyzed = LINES_TO_BE_ANALYSED;
	FILE *synLib = fopen("syn_lib.txt", "r");
    char caseFileName[WORD_SIZE];

    /* choose which file to analyze and the number of lines */
	choose_case(caseFileName, &linesToBeAnalyzed);

	if (synLib != NULL) {
        clean_review_and_make_roots_array(caseFileName, roots, &sizeOfRootsArray, linesToBeAnalyzed);
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

/* Brugeren vælger en case, som bruges i switchen til at vælge hvilken fil der skal åbnes, og derudover vælges antal linjer */
void choose_case(char caseFileName[]) {
    int caseNumber;

    printf("Please write the number of which case you want. \n 1: Reviews of musical intruments\n 2: Reviews of phones and accessories\n Choose a case:  ");
	scanf(" %d", &caseNumber);


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

	/*Tjekker om filerne er åbne og kører de funktioner som skal bruges*/
    if (caseFileClean != NULL && caseFileDirty != NULL && nounLib != NULL && nounExceptions != NULL) {
		printf("Retrieving reviews...");
        get_reviews_from_file(caseFileName, caseFileDirty, linesToBeAnalyzed);
		printf(" Done!\n");

		printf("Cleaning reviews...  ");
        clean_review(caseFileDirty, caseFileClean);
		printf(" Done!\n");
        /* vi putter først ordene i et midlertidigt array, så vi kan sortere ordene i alfabetisk rækkefølge i is_noun senere. */
		printf("Sorting reviews...   ");
        scan_words_into_temp_array(caseFileClean, rootsTemp, &sizeOfRootsArrayTemp);
        qsort(rootsTemp, sizeOfRootsArrayTemp, sizeof(root), compare);
        printf(" Done!\n");
        /* Her laves det endelige array root roots[] */
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

/* Vi tager anmeldelsesfilerne og renser dem, så der kun er anmeldelsestekst tilbage*/
void get_reviews_from_file (char caseFileName[], FILE *caseFileDirty, int linesToBeAnalyzed) {
    char reviewLineTemp[LINE_SIZE];
    char reviewLine[LINE_SIZE];
    FILE *caseFile = fopen(caseFileName, "r");
    int i;
    if (caseFile != NULL) {
    	/*Vi scanner linjerne fra vores review kilde fil, og sætter ordene ind i et temp array*/
        for (i = 0; i < linesToBeAnalyzed; ++i) {
            fgets(reviewLineTemp, LINE_SIZE, caseFile);
            sscanf(reviewLineTemp, "%*s %*s %*s %*s %*s %*[^,] %*[^0-9] %*d, %*d], %*s \"%[^\"]", reviewLine);
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

/*Vi renser alle vores ord fra tegn og tal*/
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

/*Vi scanner vores ord over i et midlertidigt array af structs*/
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

/* Her scanner vi vores ord ind i det endelige array */
void make_roots_array(root roots[], int *sizeOfRootsArray, root rootsTemp[], int sizeOfRootsArrayTemp, FILE *nounLib, FILE *nounExceptions) {
    int i;
    int indexExistingWord = -1;
    fpos_t posNoun, posExc;

    /* Vi får startspositionen af filerne, som skal bruges i is_noun */
    fgetpos(nounLib, &posNoun);
    fgetpos(nounExceptions, &posExc);

    /* For alle ord i vores midlertidige array, så puttes de i det nye, hvis de er navneord. */
    for (i = 0; i < sizeOfRootsArrayTemp; i++) {
        if (is_noun(nounLib, nounExceptions, rootsTemp[i].rootName, &posNoun, &posExc)) {
            /* indexExistingWord = index_of_existing_word(rootsTemp[i].rootName, roots, *sizeOfRootsArray); */
            indexExistingWord = word_in_roots_array(rootsTemp[i].rootName, roots, *sizeOfRootsArray);


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

    /* Først ses om ordet bliver fundet i nounLib */
    if (found_in_lib(word, nounLib, posNoun)) {
        isNoun = 1;
    }
    /* Ellers, så ser vi om vi kan finde det i vores exceptionsliste */
    else if (found_in_lib_exc(word, nounExceptions, posExc)) {
        isNoun = 1;
    }
    /* hvis ingen af disse to ting før lykkes, så laver vi det til entalsform og tjekker nounLib igen */
    else {
        convert_to_singular(word);
        if (found_in_lib(word, nounLib, posNoun)) {
            isNoun = 1;
        }
    }
    return isNoun;
}

/* Leder efter ordet i nounLib */
int found_in_lib(char word[], FILE *nounLib, fpos_t *pos) {
    char tempNoun[100];
	char firstCharOfTempNoun,
        firstCharOfWord;
    int isNoun = 0;
	firstCharOfWord = word[0];

    /* Sætter positionen til det sted vi sidst fandt et ord i nounLib, eller starten, hvis intet ord er fundet endnu*/
    fsetpos(nounLib, pos);

    do {
        fgets(tempNoun, 100, nounLib);
        tempNoun[strlen(tempNoun) - 1] = '\0';
		firstCharOfTempNoun = tempNoun[0];

        if (strcmp(word, tempNoun) == 0) {
            /* Gå tilbage til starten af det ord i Nounlib som matchede, i tilfælde af samme ord kommer igen. */
            fseek(nounLib, -(strlen(tempNoun) + 2), SEEK_CUR);
            /* Sæt positionen ved det fundne ord. */
            fgetpos(nounLib, pos);
            isNoun = 1;
            return isNoun;
        }
        /* Gør dette så længe vi ikke er ved enden af filen OG vi ikke er kørt forbi startbogstavet på det ord vi leder efter */
    } while (!feof(nounLib) && firstCharOfTempNoun <= firstCharOfWord);

    /* Returnere 0, hvis ordet ikke blev fundet i nounLib */
    return isNoun;
}

/* Leder efter ordene i nounExceptions */
int found_in_lib_exc(char word[], FILE *nounExceptions, fpos_t *pos) {
    int isNoun = 0;
    char tempNouns[100];
    char tempSingular[100];
    char tempPlural[100];
	char firstCharOfTempNoun,
        firstCharOfWord;
	firstCharOfWord = word[0];

    /* Sætter positionen til det sted vi sidst fandt et ord i nounExceptions, eller starten, hvis intet ord er fundet endnu*/
    fsetpos(nounExceptions, pos);
    do {
        fgets(tempNouns, 100, nounExceptions);
		firstCharOfTempNoun = tempNouns[0];
        sscanf(tempNouns, "%s %s", tempPlural, tempSingular);
        if (strcmp(word, tempPlural) == 0) {
            /* Gå tilbage til starten af det ord i NounExceptions som matchede, i tilfælde af samme ord kommer igen. */
            fseek(nounExceptions, -(strlen(tempNouns) + 2), SEEK_CUR);
            /* Sæt positionen ved det fundne ord. */
            fgetpos(nounExceptions, pos);
            /* Kopier entalsversionen af ordet ind i word, så det er den der gemmes i vores root array. */
            strcpy(word, tempSingular);
            isNoun = 1;
            return isNoun;
        }
        /* Gør dette så længe vi ikke er ved enden af filen OG vi ikke er kørt forbi startbogstavet på det ord vi leder efter */
    } while (!feof(nounExceptions) && (firstCharOfTempNoun == firstCharOfWord));

    /* Returnere 0, hvis ordet ikke blev fundet i nounLib */
    return isNoun;
}

/* Laver vores ord om til ental */
void convert_to_singular(char *word) {

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
}

/* Returner det index hvor word eksistere i vores array af roots. */
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

/* Compare funktion til Qsort, så ordene sorteres alfabetisk */
int compare(const void *p1, const void *p2) {
    root *root1 = (root *) p1,
         *root2 = (root *) p2;

    return strcmp(root1->rootName, root2->rootName);
}

/* Binær søgning i vores array af roots, for at returnere index på det sted den findes. */
int word_in_roots_array(char synonym[], root roots[], int numberOfRoots) {
    int result = -1;
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

/* Denne funktion går til første synonymlinje for et bestemt ord, og ellers helt ned til EOF, hvis ordet ikke findes i biblioteket */
void go_to_first_syn_line(char *root, FILE *file) {
    int rootLength = strlen(root);
    char line[LINE_SIZE];

    do {
        fgets(line, LINE_SIZE, file);
    } while ((line[rootLength] != '|' || strncmp(line, root, rootLength)) != 0 && !feof(file));
}

/* Denne funktion tager vores navneord en efter en, og hvis der er et ord i dets synonymlinje som er nævnt oftere, så bliver vores navneord's isRepresentative sat til 0 */
void find_representatives(root roots[], int sizeOfRootsArray, FILE *synLib) {
    int rootIndex = 0, lineIndex = 0, wordIndex = 0, synIndex = 0;
    char synonym[WORD_SIZE], synonymLine[LINE_SIZE];

    for (rootIndex = 0; rootIndex < sizeOfRootsArray; rootIndex++) {
        go_to_first_syn_line(roots[rootIndex].rootName, synLib);

        /* Hvis ordet ikke blev fundet i vores bibliotek, så betragter vi ikke ordet som representant */
        if (feof(synLib)) {

            roots[rootIndex].isRepresentative = 0;
            rewind(synLib); continue;
        }

        do {
            fgets(synonymLine, LINE_SIZE, synLib);
            /* scan each word and see if it's count is bigger than the current root. */
            for (lineIndex = 1, wordIndex = 0; roots[rootIndex].isRepresentative == TRUE && synonymLine[lineIndex] != '\n' && synonymLine[lineIndex] != '*'; lineIndex++, wordIndex = 0) {
                /* Finder index hvor vores synonym findex i vores root roots[] array. */
                synIndex = find_synonym_in_roots(synonymLine, synonym, &lineIndex, &wordIndex, roots, sizeOfRootsArray);

                if (synIndex != FALSE && roots[rootIndex].count < roots[synIndex].count) {
                    roots[rootIndex].isRepresentative = 0;
                }
            }
          /* Gør dette så længe at vores nuværende navneord er sat til representativ og der stadig er flere ord på synonymlinjerne. (Vi afslutter synonymlinjer med "*") */
        } while (roots[rootIndex].isRepresentative == TRUE && synonymLine[lineIndex] != '*');
    }

}

/* Vi samler største synonymklynge for hvert mulig repræsentant, derefter kopierer vi den ind i vores clusters array */
void make_clusters(root *clusters[][SYN_ARRAY_SIZE], root *EndOfCluster, int *sizeOfClustersArray, root roots[], int sizeOfRootsArray, FILE *synLib) {

    int i = 0, rootIndex = 0, wordIndex = 0, lineIndex = 0, membersIndex = 0, clusterIndex = 0, synIndex = 0, biggestLineN = 0;
    char synonymLine[LINE_SIZE], synonym[WORD_SIZE];
    /*Vi spoler tilbage i vores fil efter find_representatives*/
    rewind(synLib);

	printf("Finding synonyms...  ");

    /*Vi tjekker vores array for repræsentanter*/
    for (rootIndex = 0, clusterIndex = 0, membersIndex = 0; rootIndex < sizeOfRootsArray; rootIndex++, membersIndex = 0) {
       /* Vi gør dette for alle roots der er representanter */
        if (roots[rootIndex].isRepresentative) {
            /*Hvis det er en repræsentant, så tilføjer vi det til en cluster*/
            clusters[clusterIndex][membersIndex++] = &roots[rootIndex];
            /* Vi går til første synonymlinje for ordet */
            go_to_first_syn_line(roots[rootIndex].rootName, synLib);

            /* Returnerer linjetal af største synonymlinje. */
            biggestLineN = find_biggest_line(roots, sizeOfRootsArray, synLib);

            /*Hvis biggest line er lig 0, så er der ingen synonymlinjer*/
            if (biggestLineN != 0) {
                /*Vi henter vores linje*/
                for (i = 0; i < biggestLineN; i++) {
                    fgets(synonymLine, LINE_SIZE, synLib);
                }

                /* Køre loopet indtil vi har nået enden af linjen, og tilføjer alle ord til vores cluster array. */
                for (lineIndex = 1, wordIndex = 0; synonymLine[lineIndex] != '\n' && synonymLine[lineIndex] != '*'; lineIndex++, wordIndex = 0) {

                    synIndex = find_synonym_in_roots(synonymLine, synonym, &lineIndex, &wordIndex, roots, sizeOfRootsArray);

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

/* Finder største synonymlinje for et navneord i vores synLib bibliotek */
int find_biggest_line(root roots[], int sizeOfRootsArray, FILE *synLib) {

    int currentBiggestSum = 0, tempSum = 0, biggestLineNr = 0, lineIndex = 0, wordIndex = 0, synIndex = 0, currentLineNr = 0;
    char synonymLine[LINE_SIZE], synonym[WORD_SIZE];

    /*Vi gemmer position inden vi henter linjer*/
    fpos_t synPosition;
    fgetpos(synLib, &synPosition);

    do {
        fgets(synonymLine, LINE_SIZE, synLib);
        currentLineNr++;
        /*Vi kører indtil linjen er slut*/
        tempSum = 0;
        /* Looper igennem hele linjen */
        for (lineIndex = 1, wordIndex = 0; synonymLine[lineIndex] != '\n' && synonymLine[lineIndex] != '*'; lineIndex++, wordIndex = 0) {
            /* Ser hvor synonymet er i vores root roots array. */
            synIndex = find_synonym_in_roots(synonymLine, synonym, &lineIndex, &wordIndex, roots, sizeOfRootsArray);

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
    /* Alt dette bliver gjort så længe der stadig er flere synonymlinjer tilbage */
    } while (synonymLine[lineIndex] != '*');

    /*Vi går tilbage til vores gemte position*/
    fsetpos(synLib, &synPosition);

    return biggestLineNr;
}

/* Compare funktion til clusters, som enten sortere dem efter hyppighed, eller navnet på representanten */
int compare_clusters(const void *p1, const void *p2) {

    root  **cluster1 = (root **) p1,
          **cluster2 = (root **) p2;

    /*Hvis de ikke er lig hinanden, så sorterer vi efter hyppighed, ellers alfabetisk*/
    if (cluster1[0]->clusterCount != cluster2[0]->clusterCount)
        return (cluster1[0]->clusterCount > cluster2[0]->clusterCount) ? -1 : 1;
    else
        return strcmp(cluster1[0]->rootName, cluster2[0]->rootName);
}

/* Finder størrelsen af et cluster */
int find_cluster_size (root *cluster[]) {
    int i = 0, sum = 0;

    /*Vi kører loopet indtil arrayet er slut, og adderer pointene sammen*/
    for (i = 0; cluster[i]->count != FALSE; i++) {
        sum += cluster[i]->count;
    }

    return sum;
}

/* Printer de 10 største clusters og putter dem i en CSV fil. */
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

/* Finder et ord af gangen på vores synonymlinje og returnere index på dets index i root roots[], ved hjælp af funktionen word_in_roots_array */
int find_synonym_in_roots(char synonymLine[], char synonym[], int *i, int *j, root roots[], int sizeOfRootsArray) {
    /*Vi tjekker hvert ord, og tæller hvor langt det er*/
    while (synonymLine[*i] != '|') {
        synonym[(*j)++] = synonymLine[(*i)++];
    }
    /*Vi nul afslutter ordet*/
    synonym[*j] = '\0';

    /*Vi tjekker om det er i vores synonym aray*/
    return word_in_roots_array(synonym, roots, sizeOfRootsArray);
}
