#ifndef _incl_list_h
#define _incl_list_h

#include "header.h"

// Elemento della lista
typedef struct _topListElement
{
    unsigned int key;
    char * str;
    int percorso[GRID_LEN + 1];

    struct _topListElement * next;
    struct _topListElement * previous;

} topListElement;

// Lista auto ordinante a numero chiuso
typedef struct _topList
{
    topListElement * first;
    topListElement * last;

    unsigned int maxElements;
    unsigned int numElements;

} topList;

// Prototipi

int                 topList_new           (topList ** instance, unsigned int maxSize);
void                topList_dispose       (topList * instance);
int                 topList_insert        (topList * instance, unsigned int key, char * str, int percorso[GRID_LEN + 1]);
void                topList_delete        (topList * instance, topListElement * elInstance);
topListElement *    topList_lookupByStr   (topList * instance, char * str);

#endif // _incl_list_h
