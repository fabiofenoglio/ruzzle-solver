#ifndef _inclguard_ruzzle_h
#define _inclguard_ruzzle_h

#include "vocabulary.h"
#include "grid.h"

// Prototipi

void ruzzle_solve  (vocabulary * vocb,
                    topList * wordList,
                    cell grid[GS][GS]);

int ruzzle_outputToFile(topList * wordList,
                        cell grid[GS][GS],
                        char * filePath,
                        unsigned int maxSolutions);

#endif // _inclguard_ruzzle_h
