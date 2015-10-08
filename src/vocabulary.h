#ifndef _inclguard_vocabulary_h
#define _inclguard_vocabulary_h

#include "header.h"

// Nodo del vocabolario
typedef struct _vNode
{
    char * word;
    int isWord;

    struct _vNode * child;
    struct _vNode * next;

} vNode;

// Vocabolario (radice e cache)
typedef struct
{
    vNode * root;

    int cacheIndex;
    vNode * cache[MAX_WORD_LEN + 1];

} vocabulary;

// Risultato di una ricerca nel vocabolario
typedef enum
{
    vLookup_noMatch = 0,

    // nessuna corrispondenza ma la parola è radice di altre
    vLookup_radix = 1,

    // corrispondenza esatta
    vLookup_match = 2,

    vLookup_matchAndRadix = vLookup_radix + vLookup_match

} vLookup;

// Prototipi

int     vocabulary_new      (vocabulary ** instance);
int     vocabulary_import   (vocabulary *  instance, char * filePath);
vLookup vocabulary_lookup   (vocabulary *  instance, char * word);
void    vocabulary_dispose  (vocabulary *  instance);

#endif
