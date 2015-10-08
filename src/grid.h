
#ifndef _inclguard_grid_h
#define _inclguard_grid_h

#include "header.h"

// Possibili bonus di una cella
typedef enum _bonusEnum
{
    none    = 0,
    doubleL = 1,
    tripleL = 2,
    doubleW = 3,
    tripleW = 4

} bonusEnum;

// Cella della griglia
typedef struct _cell
{
    unsigned char       character;
    unsigned int        points;
    bonusEnum           bonus;
    unsigned int        visited;

} cell;

// Prototipi

int grid_import (cell instance[GS][GS], char * filePath);

#endif
