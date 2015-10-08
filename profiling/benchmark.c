#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "header.h"
#include "toplist.h"
#include "vocabulary.h"
#include "ruzzle.h"
#include "grid.h"

#include "benchmark.h"

#define DEFAULT_BOARD_OUT_FILE DEFAULT_BOARD_FILE ".auto.txt"
#define DEFAULT_LOG_FILE DEFAULT_BOARD_FILE ".log"

#define NUM_LETTERE 26

void benchmark_runFullSolutionTest(unsigned long iterNum);
void benchmark_runVocabularyFileReadTest(unsigned long iterNum);
void benchmark_runVocabularyImportTest(unsigned long iterNum);
void benchmark_runGridImportTest(unsigned long iterNum);
void benchmark_runSolutionTest(unsigned long iterNum);
void benchmark_runOutputWriteTest(unsigned long iterNum);
void benchmark_runVocabularyPopulationTest();

void benchmark_runMemAllocTest();

void ___result(char * descr, double tDiff, unsigned long itern);

void ___benchmark_runSingleFullSolutionTest();
void ___benchmark_getWordsFromVocabulary(vNode * instance,
                                         char * wordPath,
                                         unsigned int level,
                                         topList * checkList[NUM_LETTERE]);

void ___benchmark_runSolutionTest(vocabulary * vocb, cell grid [GS][GS]);

unsigned long errNum = 0;
unsigned long gCnt = 0;
FILE * fLog;

void benchmark_runTests()
{
    errNum = 0;
    fLog = fopen(DEFAULT_LOG_FILE, "w+");
    assert(fLog != NULL);

    #if 1
        // Serie completa di test su tutti i moduli
        fprintf(fLog, "Running extended tests ...\n\n");
        benchmark_runVocabularyPopulationTest();
        benchmark_runFullSolutionTest(405);
        benchmark_runVocabularyImportTest(405);
        benchmark_runGridImportTest(303612);
        benchmark_runSolutionTest(20373);
        benchmark_runOutputWriteTest(30843);
        benchmark_runVocabularyFileReadTest(1283);
    #elif 1
        // Attivare il profiler prima di eseguire questo test
        fprintf(fLog, "Running profiling test ...\n\n");
        benchmark_runFullSolutionTest(500);
    #elif 1
        // Test per la ricerca di memory leaks
        benchmark_runMemAllocTest();
    #endif

    if (errNum)
        fprintf(fLog, "\nTEST FAILED (%lu errors)\n", errNum);
    else
        fprintf(fLog, "\nAll tests OK!\n");

    fclose(fLog);

    // Apro il file di log
    // (dipendente dal sistema operativo)

    system("start notepad \"" DEFAULT_LOG_FILE "\"");
    // system("nano \"" DEFAULT_LOG_FILE "\"" );
}

void ___result(char * descr, double tDiff, unsigned long itern)
{
    if (tDiff >= 2.0)
        fprintf(fLog, "\t%-20s: %*.0f us/call (%6lu calls in %3.0f s)\n", descr, 6, 1000000.0 * tDiff / (float)itern, itern, tDiff);
    else
        fprintf(fLog, "\t%-20s: not enough data.\n", descr);
}

void ___benchmark_runSingleFullSolutionTest()
{
    char            vocbFilePath    [MAX_FILEPATH_LEN];
    char            inFilePath      [MAX_FILEPATH_LEN];
    char            outFilePath     [MAX_FILEPATH_LEN];;
    unsigned int    numberToFind;
    vocabulary *    vocb;
    topList *       wordList;
    cell            grid            [GS][GS];

    strcpy(inFilePath, DEFAULT_BOARD_FILE);
    strcpy(vocbFilePath, DEFAULT_VOCABULARY_FILE);
    strcpy(outFilePath, DEFAULT_BOARD_OUT_FILE);
    numberToFind = DEFAULT_WORDS_NUMBER;

    assert( topList_new(&wordList, numberToFind));
    assert( grid_import(grid, inFilePath));
    assert( vocabulary_new(&vocb));
    assert( vocabulary_import(vocb, vocbFilePath));
    ruzzle_solve(vocb, wordList, grid);
    assert( ruzzle_outputToFile(wordList, grid, outFilePath, numberToFind));

    topList_dispose(wordList);
    vocabulary_dispose(vocb);
}

void benchmark_runMemAllocTest()
{
    char vocbFilePath[MAX_FILEPATH_LEN];
    vocabulary * vocb;
    double iterCnt = 0;

    strcpy(vocbFilePath, DEFAULT_VOCABULARY_FILE);

    while (1)
    {
        if (!vocabulary_new(&vocb)) break;

        if (!vocabulary_import(vocb, vocbFilePath))
        {
            vocabulary_dispose(vocb);
            break;
        }
        vocabulary_dispose(vocb);

        iterCnt += 1;
        if (iterCnt >= 10000) return;
    }

    fprintf(fLog, "\tMemAllocTest error: stopped after #%f\n", iterCnt);
    errNum += 1;
}

void benchmark_runFullSolutionTest(unsigned long iterNum)
{
    unsigned long cnt;
    time_t tStart, tEnd;
    time(&tStart);

    for (cnt = 0; cnt < iterNum; cnt ++)
        ___benchmark_runSingleFullSolutionTest();

    time(&tEnd);

    ___result("Full solution", difftime(tEnd, tStart), iterNum);
}

void ___benchmark_getWordsFromVocabulary(vNode * instance,
                                         char * wordPath,
                                         unsigned int level,
                                         topList * checkList[NUM_LETTERE])
{
    topListElement * el;
    if (instance == NULL) return;

    // Prima "next", poi "instance", poi "child"
    ___benchmark_getWordsFromVocabulary(instance -> next,
                                        wordPath,
                                        level,
                                        checkList);

    strcpy(wordPath + level, instance -> word);

    if (instance -> isWord)
    {
        // ho trovato una parola dall'albero. controllo che esista
        // anche nella lista delle parole importate
        gCnt ++;

        el = topList_lookupByStr(checkList[wordPath[0] - 'a'], wordPath);

        if (el == NULL)
        {
            fprintf(fLog, "\n\t[ERROR] word in dictionary but NOT in file: \"%s\"\n", wordPath);
            errNum ++;
        }
    }

    ___benchmark_getWordsFromVocabulary(instance -> child,
                                        wordPath,
                                        strlen(wordPath),
                                        checkList);
}

void benchmark_runVocabularyPopulationTest()
{
    topList * fromFileList[NUM_LETTERE];
    vocabulary * vocb;
    FILE * fIn;
    unsigned int i;
    unsigned long wCnt, eCnt;
    char inLine[MAX_LINE + 1];
    int path[GRID_LEN + 1] = {-1};
    topListElement * el;
    vLookup lc;

    char buildPath[GRID_LEN + 1];

    for (i = 0; i < NUM_LETTERE; i ++)
        assert( topList_new(&(fromFileList[i]), 1000000));
    assert( vocabulary_new(&vocb));

    fIn = fopen(DEFAULT_VOCABULARY_FILE, "r");
    assert(fIn != NULL);
    assert(fgets(inLine, MAX_LINE, fIn) != NULL);
    gCnt = 0;

    while (fgets(inLine, MAX_LINE, fIn) != NULL)
    {
        if (___vocb_importline(inLine))
        {
            assert(___vocb_insert(vocb, inLine));
            if (topList_lookupByStr(fromFileList[inLine[0] - 'a'], inLine) == NULL)
            {
                gCnt ++;
                assert( topList_insert(fromFileList[inLine[0] - 'a'], 0, inLine, path));
            }
        }
    }

    fclose(fIn);
    wCnt = gCnt;
    fprintf(fLog, "\tIMPORT test passed (%lu words)\n", gCnt);

    gCnt = 0;
    eCnt = errNum;
    if (vocb -> root != NULL)
        ___benchmark_getWordsFromVocabulary(vocb -> root -> child, buildPath, 0, fromFileList);
    else
        ___benchmark_getWordsFromVocabulary(NULL, buildPath, 0, fromFileList);

    if (errNum == eCnt)
        fprintf(fLog, "\tREADING test passed\n");

    if (gCnt != wCnt)
    {
        errNum ++;
        fprintf(fLog, "\t[ERROR] words number mismatch in comparison test (%lu / %lu)\n\n", gCnt, wCnt);
    }

    eCnt = errNum;
    gCnt = 0;
    for (i = 0; i < NUM_LETTERE; i ++)
    {
        el = fromFileList[i] -> first;
        while (el != NULL)
        {
            gCnt ++;

            lc = vocabulary_lookup(vocb, el -> str);

            if (! (lc & vLookup_match) )
            {
                fprintf(fLog, "\n\t[ERROR] word in file but NOT in TREE: \"%s\"\n\t", el -> str);
                errNum ++;
            }

            el = el -> next;
        }
    }

    if (errNum == eCnt)
        fprintf(fLog, "\tLOOKUP test passed\n");

    if (gCnt != wCnt)
    {
        errNum ++;
        fprintf(fLog, "\t[ERROR] words number mismatch in comparison test (%lu / %lu)\n\n", gCnt, wCnt);
    }

    fprintf(fLog, "\n");
}

void benchmark_runOutputWriteTest(unsigned long iterNum)
{
    vocabulary *    vocb;
    cell            grid [GS][GS];
    topList *       wordList;
    unsigned long   cnt;
    time_t tStart, tEnd;

    assert( grid_import(grid, DEFAULT_BOARD_FILE));
    assert( vocabulary_new(&vocb));
    assert( vocabulary_import(vocb, DEFAULT_VOCABULARY_FILE));
    assert( topList_new(&wordList, DEFAULT_WORDS_NUMBER));
    ruzzle_solve(vocb, wordList, grid);

    time(&tStart);

    for (cnt = 0; cnt < iterNum; cnt ++)
    {
        assert
        (
            ruzzle_outputToFile(wordList, grid, DEFAULT_BOARD_OUT_FILE, DEFAULT_WORDS_NUMBER)
        );
    }

    time(&tEnd);

    topList_dispose(wordList);
    vocabulary_dispose(vocb);
    ___result("Output write", difftime(tEnd, tStart), iterNum);
}

void ___benchmark_runSolutionTest(vocabulary * vocb, cell grid [GS][GS])
{
    topList *    wordList;

    assert(topList_new(&wordList, DEFAULT_WORDS_NUMBER));
    ruzzle_solve(vocb, wordList, grid);
    topList_dispose(wordList);
}

void benchmark_runSolutionTest(unsigned long iterNum)
{
    vocabulary *    vocb;
    cell            grid [GS][GS];
    unsigned long cnt;
    time_t tStart, tEnd;

    assert( grid_import(grid, DEFAULT_BOARD_FILE) );
    assert( vocabulary_new(&vocb) );
    assert( vocabulary_import(vocb, DEFAULT_VOCABULARY_FILE) );

    time(&tStart);

    for (cnt = 0; cnt < iterNum; cnt ++)
        ___benchmark_runSolutionTest(vocb, grid);

    time(&tEnd);

    vocabulary_dispose(vocb);
    ___result("Search algorithm", difftime(tEnd, tStart), iterNum);
}

void benchmark_runGridImportTest(unsigned long iterNum)
{
    cell grid [GS][GS];
    unsigned long cnt;
    time_t tStart, tEnd;
    time(&tStart);

    for (cnt = 0; cnt < iterNum; cnt ++)
    {
        assert( grid_import(grid, DEFAULT_BOARD_FILE) );
    }

    time(&tEnd);
    ___result("Grid import", difftime(tEnd, tStart), iterNum);
}

void benchmark_runVocabularyFileReadTest(unsigned long iterNum)
{
    unsigned long cnt;
    time_t tStart, tEnd;
    FILE * fIn;
    char inLine[MAX_LINE + 1];

    time(&tStart);

    for (cnt = 0; cnt < iterNum; cnt ++)
    {
        fIn = fopen(DEFAULT_VOCABULARY_FILE, "r");
        assert(fIn != NULL);
        fgets(inLine, MAX_LINE, fIn);

        while (fgets(inLine, MAX_LINE, fIn) != NULL)
        {
            ___vocb_importline(inLine);
        }
        fclose(fIn);
    }

    time(&tEnd);
    ___result("Dictionary read", difftime(tEnd, tStart), iterNum);
}

void benchmark_runVocabularyImportTest(unsigned long iterNum)
{
    vocabulary * vocb;
    unsigned long cnt;
    time_t tStart, tEnd;
    time(&tStart);

    for (cnt = 0; cnt < iterNum; cnt ++)
    {
        assert( vocabulary_new(&vocb) );
        assert( vocabulary_import(vocb, DEFAULT_VOCABULARY_FILE) );
        vocabulary_dispose(vocb);
    }

    time(&tEnd);
    ___result("Dictionary import", difftime(tEnd, tStart), iterNum);
}
