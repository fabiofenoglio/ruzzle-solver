#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"
#include "toplist.h"
#include "vocabulary.h"
#include "ruzzle.h"
#include "grid.h"

// Richiede all'utente il percorso / nome di un file
void ask_path (char * whereToSave,
               const char * prompt,
               const char * defaultValue);

// Richiede all'utente un valore unsigned int
void ask_uint (unsigned int * whereToSave,
               const char * prompt,
               const unsigned int defaultValue);

// Ottiene il percorso di output dal percorso di input
int fileNameAppend (char * inputFileName,
                    char ** outputFileName);

int main()
{
    char         vocbFilePath [MAX_FILEPATH_LEN];
    char         inFilePath   [MAX_FILEPATH_LEN];
    char *       outFilePath;
    unsigned int numberToFind;

    cell         board [GS][GS];
    vocabulary * vocb;
    topList *    wordList;

    // Richiedo all'utente i parametri di input

    ask_path(inFilePath,    BOARD_FILE_PROMPT,      DEFAULT_BOARD_FILE);
    ask_path(vocbFilePath,  VOCABULARY_FILE_PROMPT, DEFAULT_VOCABULARY_FILE);
    ask_uint(&numberToFind, WORDS_NUMBER_PROMPT,    DEFAULT_WORDS_NUMBER);
    if (numberToFind == 0) return EXIT_SUCCESS;

    // Costruisco e carico le strutture dati per la risoluzione

    attemptOrDieWithMessage(
        fileNameAppend(inFilePath, &outFilePath),
        _EMSG_MEMALLOC );

    attemptOrDieWithMessage(
        topList_new(&wordList, numberToFind),
        _EMSG_MEMALLOC );

    attemptOrDieWithMessage(
        vocabulary_new(&vocb),
        _EMSG_MEMALLOC );

    attemptOrDieWithMessage(
        grid_import(board, inFilePath),
        _EMSG_BOARDIMPORT );

    attemptOrDieWithMessage(
        vocabulary_import(vocb, vocbFilePath),
        _EMSG_VOCBIMPORT );

    // Lancio l'algoritmo di risoluzione

    ruzzle_solve(vocb, wordList, board);

    // Scrivo l'output su file

    attemptOrDieWithMessage(
        ruzzle_outputToFile(wordList, board, outFilePath, numberToFind),
        _EMSG_OUTWRITE
    );

    // Le risorse sono liberate automaticamente alla fine
    return EXIT_SUCCESS;
}

// Richiede all'utente il percorso / nome di un file
void ask_path(char * whereToSave, const char * prompt, const char * defaultValue)
{
    char inLine[MAX_LINE + 1];
    char * trm;

    while (1)
    {
        printf(prompt);
        fgets(inLine, MAX_LINE, stdin);

        if (inLine[0] != '\n')
        {
            // Non posso usare sscanf perchè termina sugli spazi
            strcpy(whereToSave, inLine);
            trm = strchr(whereToSave, '\n');
            if (trm != NULL) *trm = '\0';
            break;
        }
        else if (defaultValue != NULL)
        {
            strcpy(whereToSave, defaultValue);
            break;
        }
    }

}

// Richiede all'utente un valore unsigned int
void ask_uint(unsigned int * whereToSave, const char * prompt, const unsigned int defaultValue)
{
    char inLine[MAX_LINE + 1];

    printf(prompt);
    fgets(inLine, MAX_LINE, stdin);

    if (inLine[0] != '\n')
        sscanf(inLine, "%u\n", whereToSave);
    else
        *whereToSave = defaultValue;
}

// Ottiene il percorso di output dal percorso di input
int fileNameAppend (char * inputFileName, char ** outputFileName)
{
    const char * toAppend = RESULT_APPEND_STR;
    int ind, c_ind;

    (*outputFileName) = (char*)
        malloc( (strlen(inputFileName) + strlen(toAppend) + 1) * sizeof(char) );

    if ((*outputFileName) == NULL) return 0;

    // Trova l'ultimo carattere '.'
    c_ind = -1;

    for (ind = 0; inputFileName[ind] != '\0'; ind ++)
    {
        if (inputFileName[ind] == '.') c_ind = ind;
    }

    if (c_ind == -1)
    {
        strcpy(*outputFileName, inputFileName);
        strcat(*outputFileName, toAppend);
    }
    else
    {
        for (ind = 0; ind < c_ind; ind ++)
            (*outputFileName)[ind] = inputFileName[ind];

        (*outputFileName)[ind] = '\0';

        strcat(*outputFileName, toAppend);
        strcat(*outputFileName, inputFileName + c_ind);
    }

    return 1;
}
