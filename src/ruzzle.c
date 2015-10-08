#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"
#include "grid.h"
#include "toplist.h"
#include "vocabulary.h"
#include "ruzzle.h"

// Direzioni in cui è possibile spostarsi partendo da una casella
typedef enum
{
    ___none = 0,
    ___n = 1,
    ___ne = 2,
    ___e = 4,
    ___se = 8,
    ___s = 16,
    ___sw = 32,
    ___w = 64,
    ___nw = 128,

    ___all = 255

} ___drc;

// Variazioni degli indici per spostarsi in una data direzione
___drc  ___staticDir[] =       {___n, ___ne, ___e, ___se, ___s, ___sw, ___w, ___nw};
int     ___staticDirRMove[] =  {  -1,    -1,    0,    +1,   +1,    +1,    0,    -1};
int     ___staticDirCMove[] =  {   0,    +1,   +1,    +1,    0,    -1,   -1,    -1};

// Prototipi di funzioni usate solo internamente

void    ___ruzzle_solveFromPt (vocabulary * vocb,
                               topList * wordList,
                               cell board[GS][GS],
                               int r_now,
                               int c_now,
                               unsigned int level,
                               char * out,
                               int * outHistory);

___drc  ___findPath (cell grid[GS][GS],
                     int r,
                     int c);

void    ___logSolution (topList * wordList,
                        cell board[GS][GS],
                        char * parola,
                        int * outHistory);

// Implementazioni

// Restituisce un'OR con i percorsi in cui è possibile spostarsi
// partendo da una certa casella
___drc ___findPath(cell gr[GS][GS], int r, int c)
{
    ___drc res = ___none;

    if (r > 0 && !gr[r - 1][c].visited)                       res += ___n;
    if (r > 0 && c < (GS - 1) && !gr[r - 1][c + 1].visited)   res += ___ne;
    if (c < (GS - 1) && !gr[r][c + 1].visited)                res += ___e;
    if (c < (GS - 1) && r < (GS - 1) && !gr[r + 1][c + 1].visited)
                                                              res += ___se;
    if (r < (GS - 1) && !gr[r + 1][c].visited)                res += ___s;
    if (r < (GS - 1) && c > 0 && !gr[r + 1][c - 1].visited)   res += ___sw;
    if (c > 0 && !gr[r][c - 1].visited)                       res += ___w;
    if (c > 0 && r > 0 && !gr[r - 1][c - 1].visited)          res += ___nw;

    return res;
}

// Calcola i punti ed inserisce una soluzione nella lista, se necessario
void ___logSolution(topList * wordList, cell board[GS][GS], char * parola, int * outHistory)
{
    int i, ir, ic;
    unsigned int tempPts, tempBns;
    unsigned int wMul = 1;
    unsigned int wPts = 0;
    topListElement * listLookupRes;

    for (i = 0; outHistory[i] != -1; i ++)
    {
        ir = outHistory[i] / GS;
        ic = outHistory[i] % GS;

        tempBns = board[ir][ic].bonus;
        tempPts = board[ir][ic].points;

        switch (tempBns)
        {
            case doubleW:   wPts += tempPts;     wMul *= 2; break;
            case tripleW:   wPts += tempPts;     wMul *= 3; break;
            case doubleL:   wPts += 2 * tempPts;            break;
            case tripleL:   wPts += 3 * tempPts;            break;
            default:        wPts += tempPts;
        }
    }

    wPts *= wMul;

    while (i-- > 4)
        wPts += 5;

    listLookupRes = topList_lookupByStr(wordList, parola);
    if (listLookupRes == NULL)
    {
        // Nella lista non esiste ancora questa parola, la aggiungo
        topList_insert(wordList, wPts, parola, outHistory);
    }
    else if (wPts > listLookupRes -> key)
    {
        // Nella lista esiste già questa parola ma ho trovato una
        // occorrenza con un punteggio maggiore, quindi rimpiazzo

        topList_delete(wordList, listLookupRes);
        topList_insert(wordList, wPts, parola, outHistory);
    }
    // altrimenti, nella lista esiste già la parola ed il punteggio
    // è maggiore o uguale alla parola trovata, quindi non devo fare nulla.
}

// Risolve i possibili percorsi a partire da una certa casella
void ___ruzzle_solveFromPt(vocabulary * vocb,
                           topList * wordList,
                           cell board[GS][GS],
                           int r_now, int c_now, unsigned int level,
                           char * out,
                           int * outHistory)
{
	int r, c, i;
    ___drc allowed;
    vLookup lRes;

    // Marca la casella come visitata
	board[r_now][c_now].visited = 1;

    // Aggiungi la lettera della casella
    out[level] = board[r_now][c_now].character;
    out[level + 1] = '\0';
    outHistory[level] = r_now * GS + c_now;
    outHistory[level + 1] = -1;

    // Interroga il dizionario
    lRes = vocabulary_lookup(vocb, out);

    if (lRes & vLookup_match)
        ___logSolution(wordList, board, out, outHistory);

    if (! (lRes & vLookup_radix))
        return;

    allowed = ___findPath(board, r_now, c_now);
    if (!allowed) return;

    // per ogni possibile direzione
	for (i = 0; i < 8; i++)
	{
	    // se la direzione non è consentita, ignoro la casella
		if (!(allowed & ___staticDir[i])) continue;

		// sposto r e c nella nuova casella
		r = r_now + ___staticDirRMove[i];
		c = c_now + ___staticDirCMove[i];

        ___ruzzle_solveFromPt(vocb, wordList, board, r, c, level + 1, out, outHistory);
        board[r][c].visited = 0;
	}
}

// Algoritmo di risoluzione principale
void ruzzle_solve(vocabulary * vocb, topList * wordList, cell grid[GS][GS])
{
    int ir, ic;
    int history[GRID_LEN + 1];
    char path[GRID_LEN + 1];

    for (ir = 0; ir < GS; ir ++)
    {
        for (ic = 0; ic < GS; ic ++)
        {
            ___ruzzle_solveFromPt(vocb, wordList, grid, ir, ic, 0, path, history);
            grid[ir][ic].visited = 0;
        }
    }
}

// Scrive l'output trovato dall'algoritmo su file
int ruzzle_outputToFile(topList * wordList, cell grid[GS][GS], char * filePath, unsigned int maxSolutions)
{
    topListElement * el;
    size_t l;
    int condensedIndex, ir, ic, pathIndex;
    FILE * fHandle;
    unsigned int totalScore = 0;
    size_t partOfThePath;

    fHandle = fopen(filePath, "w+");
    if (fHandle == NULL) return 0;

    el = wordList -> first;

    while (el != NULL)
    {
        for (l = 0; l < strlen(el -> str); l ++)
            fprintf(fHandle, "%c", toupper(el -> str[l]));

        fprintf(fHandle, " %u\n", el -> key);
        totalScore += el -> key;

        // Per ogni casella della griglia
        for (ir = 0; ir < GS; ir ++)
        {
            for (ic = 0; ic < GS; ic ++)
            {
                // Cerca se la casella è parte del percorso della parola
                condensedIndex = ir * GS + ic;

                partOfThePath = 0;
                for (pathIndex = 0;
                     !partOfThePath && el -> percorso[pathIndex] != -1;
                     pathIndex ++)
                {
                    if (el -> percorso[pathIndex] == condensedIndex) partOfThePath = 1;
                }

                // In tal caso, scrivi la lettera in maiuscolo
                if (partOfThePath)
                    fprintf(fHandle, "%c", toupper(grid[ir][ic].character));
                else
                    fprintf(fHandle, "%c", grid[ir][ic].character);
            }
            fprintf(fHandle, "\n");
        }

        fprintf(fHandle, "\n");

        el = el -> next;
    }

    if (wordList -> numElements != maxSolutions)
        fprintf(stderr, "[Warning] Not enough words found (%u requested, %u found)\n", maxSolutions, wordList -> numElements);

    fprintf(fHandle, "TOTAL SCORE: %u\n", totalScore);
    fclose(fHandle);
    return 1;
}
