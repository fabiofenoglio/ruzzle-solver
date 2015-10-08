#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "toplist.h"

// Prototipi di funzioni usate solo internamente

int  ___topListElement_new    (topListElement ** instance);
void ___topListElement_dispose(topListElement * el);

// Implementazioni

// Libera la memoria di un elemento
void ___topListElement_dispose(topListElement * el)
{
    if (el -> str != NULL) free(el -> str);
    if (el != NULL) free(el);
}

// Crea un nuovo elemento
int ___topListElement_new(topListElement ** instance)
{
    (*instance) = (topListElement *) malloc (sizeof(topListElement));
    if ((*instance) == NULL) return 0;
    (*instance) -> str = NULL;
    return 1;
}

// Ricerca un elemento all'interno della lista (per stringa)
topListElement * topList_lookupByStr(topList * instance, char * str)
{
    topListElement * el = instance -> first;
    while (el != NULL)
    {
        if (strcmp(str, el -> str) == 0) return el;
        el = el -> next;
    }
    return NULL;
}

// Crea una nuova lista
int topList_new(topList ** instance, unsigned int maxSize)
{
    (*instance) = (topList *) malloc (sizeof(topList));
    if ((*instance) == NULL) return 0;
    (*instance) -> first = NULL;
    (*instance) -> last = NULL;
    (*instance) -> maxElements = maxSize;
    (*instance) -> numElements = 0;
    return 1;
}

// Elimina una lista e ne libera la memoria
void topList_dispose(topList * instance)
{
    topListElement * el, * nextEl;

    if (instance == NULL) return;
    el = instance -> first;

    while (el != NULL)
    {
        nextEl = el -> next;
        ___topListElement_dispose(el);
        el = nextEl;
    }

    free(instance);
}

// Inserisce un nuovo elemento nella lista
int topList_insert(topList * instance, unsigned int key, char * str, int percorso[GRID_LEN + 1])
{
    topListElement * newEl, * cfrEl, * prevEl;
    unsigned int i;

    // Se la lista è piena e l'elemento non ha punteggio maggiore dell'ultimo, non lo inserisco
    if (instance -> numElements >= instance -> maxElements)
    {
        if (key <= instance -> last -> key) return 1;
    }

    // Creo un nuovo elemento
    if (! ___topListElement_new(&newEl)) return 0;

    // Importo i dati passati
    newEl -> key = key;
    newEl -> str = (char *) malloc ( (1 + strlen(str)) * sizeof(char) );
    if (newEl -> str == NULL)
    {
        ___topListElement_dispose(newEl);
        return 0;
    }
    strcpy(newEl -> str, str);

    for (i = 0; i < GRID_LEN; i ++)
        newEl -> percorso[i] = percorso[i];

    // Cerca il posto dove inserire l'elemento
    cfrEl = instance -> first;
    prevEl = NULL;

    while (cfrEl != NULL && key < cfrEl -> key)
    {
        prevEl = cfrEl;
        cfrEl = cfrEl -> next;
    }

    // Inserisco l'elemento prima dell'ultimo trovato
    // (tra prevEl e cfrEl)
    if (prevEl == NULL)
    {
        // L'elemento è il primo della lista
        instance -> first = newEl;
        newEl -> previous = NULL;
    }
    else
    {
        prevEl -> next = newEl;
        newEl -> previous = prevEl;
    }

    newEl -> next = cfrEl;

    if (cfrEl != NULL)
        cfrEl -> previous = newEl;
    else
        instance -> last = newEl;

    if (instance -> numElements >= instance -> maxElements)
    {
        // Troppi elementi, elimino l'ultimo della lista
        prevEl = instance -> last -> previous;
        ___topListElement_dispose(instance -> last);

        instance -> last = prevEl;
        if (prevEl != NULL)
            prevEl -> next = NULL;
    }
    else
    {
        // Incremento il numero degli elementi nella lista
        instance -> numElements ++;
    }

    return 1;
}

// Elimina un elemento dalla lista e ne libera la memoria
void topList_delete(topList * instance, topListElement * elInstance)
{
    topListElement * prevEl;
    topListElement * nextEl;

    prevEl = elInstance -> previous;
    nextEl = elInstance -> next;

    if (prevEl != NULL)
    {
        prevEl -> next = nextEl;
        if (nextEl != NULL)
            nextEl -> previous = prevEl;
        else
            instance -> last = prevEl;
    }
    else
    {
        instance -> first = elInstance -> next;
        if (nextEl != NULL)
            nextEl -> previous = NULL;
    }
    ___topListElement_dispose(elInstance);
    instance -> numElements --;
}
