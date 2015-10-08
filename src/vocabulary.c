#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"
#include "vocabulary.h"

// Prototipi di funzioni usate solo internamente

int     ___vNode_new            (vNode ** instance);
int     ___vNode_setword        (vNode ** instance, char * word);
void    ___vNode_dispose        (vNode * instance);

inline int      ___vocb_importline      (char * strIn);
inline int      ___vocb_insert          (vocabulary * instance, char * word);
inline int      ___vNodeInsertChild     (vocabulary * instance, vNode * parent, char * word);
inline int      ___vNodeSplitAtIndex    (vocabulary * instance, vNode * node, int index, int isWord);
inline vNode *  ___vNode_searchRadix    (vNode * startingNode, char * startingChars);
inline void     ___vCache_push          (vocabulary * instance, vNode * node);

// Implementazioni

// Crea un nuovo vocabolario
int vocabulary_new(vocabulary ** instance)
{
    vNode * rootNode;
    (*instance) = (vocabulary *)malloc(sizeof(vocabulary));
    if ((*instance) == NULL) return 0;

    if (!___vNode_new(&rootNode)) return 0;
    if (!___vNode_setword(&rootNode, "ROOT")) return 0;

    (*instance) -> root = rootNode;
    (*instance) -> cacheIndex = 0;

    return 1;
}

// Importa il vocabolario da un file
int vocabulary_import(vocabulary * vocb, char * filePath)
{
    FILE * fIn;
    char inLine[MAX_LINE + 1];

    fIn = fopen(filePath, "r");
    if (fIn == NULL) return 0;

    // Leggo il numero di linee previsto
    if (fgets(inLine, MAX_LINE, fIn) == NULL) return 0;

    // Leggo tutte le altre linee
    while (fgets(inLine, MAX_LINE, fIn) != NULL)
    {
        if (___vocb_importline(inLine))
        {
            if (!___vocb_insert(vocb, inLine))
            {
                fclose(fIn);
                return 0;
            }
        }
    }

    fclose(fIn);
    return 1;
}

// Ricerca una parola nel vocabolario
vLookup vocabulary_lookup(vocabulary * instance, char * word)
{
    vNode * node, * commonRadixNode;
    int similCount;
    int rnodeLen, wordLen;
    int i;
    char c;

    node = instance -> root;

    // Esploro i nodi validi in cache per trovare radici
    for (i = 0; i < instance -> cacheIndex; i ++)
    {
        // Verifico se il nodo è parte della parola
        similCount = 0;

        while (1)
        {
            c = instance -> cache[i] -> word[similCount];
            if (c != word[similCount] || c == '\0') break;
            similCount ++;
        }

        // in "similCount" ho il numero di lettere coincidenti
        if (instance -> cache[i] -> word[similCount] == '\0')
        {
            // tutto il nodo è contenuto
            if (word[similCount] == '\0')
            {
                // corrispondenza esatta!
                return (instance -> cache[i] -> isWord ? vLookup_match : 0) |
                       (instance -> cache[i] -> child != NULL ? vLookup_radix : 0);
            }

            node = instance -> cache[i];
            word = word + similCount;
        }
        else
        {
            // Il nodo non è completamente contenuto (in parte, o niente)
            // contrassegna il nodo come non valido per la cache
            instance -> cacheIndex = i;
            break;
        }
    }

    while (1)
    {
        commonRadixNode = ___vNode_searchRadix(node -> child, word);
        if (commonRadixNode == NULL) return vLookup_noMatch;

        for (similCount = 0;
            commonRadixNode -> word[similCount] == word[similCount]
                && word[similCount] != '\0';
            similCount ++) ;

        rnodeLen = strlen(commonRadixNode -> word);
        wordLen = strlen(word);

        if (similCount == wordLen && similCount == rnodeLen)
        {
            ___vCache_push(instance, commonRadixNode);

            if (commonRadixNode -> isWord)
            {
                if (commonRadixNode -> child != NULL)
                    return vLookup_matchAndRadix;
                else
                    return vLookup_match;
            }
            else
                return vLookup_radix;
        }

        else if (similCount == wordLen)
            // Il nodo contiene tutta la parola E altre lettere
            return vLookup_radix;

        else if (similCount == rnodeLen)
        {
            ___vCache_push(instance, commonRadixNode);
            node = commonRadixNode;
            word = word + similCount;
        }

        else
            // il nodo contiene solo ALCUNE lettere radici, e altre non comuni
            return vLookup_noMatch;
    }
}

// Elimina un nodo
void ___vNode_dispose(vNode * instance)
{
    if (instance == NULL) return;

    ___vNode_dispose(instance -> next);
    ___vNode_dispose(instance -> child);
    free(instance -> word);
    free(instance);
}

// Elimina tutto il vocabolario e ne libera la memoria
void vocabulary_dispose(vocabulary * instance)
{
    ___vNode_dispose(instance -> root);
    free(instance);
}

// Crea un nuovo nodo
int ___vNode_new(vNode ** instance)
{
    (*instance) = (vNode *)malloc(sizeof(vNode));
    if ((*instance) == NULL) return 0;

    (*instance) -> word = NULL;
    (*instance) -> child = NULL;
    (*instance) -> next = NULL;
    (*instance) -> isWord = 1;

    return 1;
}

// Copia una stringa ad un nodo istanziando la memoria
int ___vNode_setword(vNode ** instance, char * word)
{
    (*instance) -> word = (char*)malloc( (strlen(word) + 1) * sizeof(char) );
    if ((*instance) -> word == NULL) return 0;
    strcpy((*instance) -> word, word);
    return 1;
}

// Inserisce una parola nel vocabolario
inline int ___vocb_insert(vocabulary * instance, char * oWord)
{
    int i, com;
    char c;
    vNode * baseNode, * commonRadixNode;
    char * word = oWord;
    int rnodeLen, wordLen;

    baseNode = instance -> root;

    // Esploro i nodi validi in cache per trovare radici
    for (i = 0; i < instance -> cacheIndex; i ++)
    {
        // Verifico se il nodo è parte della parola
        com = 0;

        while (1)
        {
            c = instance -> cache[i] -> word[com];
            if (c != word[com] || c == '\0') break;
            com ++;
        }

        // in "com" ho il numero di lettere coincidenti
        if (instance -> cache[i] -> word[com] == '\0')
        {
            // tutto il nodo è contenuto. verifico se c'è piena
            // corrispondenza anche con la parola, in questo
            // caso è la stessa parola.
            if (word[com] == '\0')
            {
                // è la stessa parola, già contenuta in cache.
                instance -> cache[i] -> isWord = 1;
                return 1;
            }

            baseNode = instance -> cache[i];
            word = word + com;
        }
        else
        {
            // Il nodo non è completamente contenuto (in parte, o niente)
            // contrassegna il nodo come non valido per la cache
            instance -> cacheIndex = i;
            break;
        }
    }

    // Proseguo con la ricerca dall'ultimo nodo cache individuato
    while (1)
    {
        commonRadixNode = ___vNode_searchRadix(baseNode -> child, word);

        if (commonRadixNode == NULL)
        {
            // non ho trovato nessun nodo con radice comune
            return ___vNodeInsertChild(instance, baseNode, word);
        }
        else
        {
            for (com = 0;
                 commonRadixNode -> word[com] == word[com]
                    && word[com] != '\0';
                 com ++) ;

            rnodeLen = strlen(commonRadixNode -> word);
            wordLen = strlen(word);

            if (com == wordLen && com == rnodeLen)
            {
                ___vCache_push(instance, commonRadixNode);
                commonRadixNode -> isWord = 1;
                return 1;
            }
            else if (com == wordLen)
            {
                return ___vNodeSplitAtIndex(instance, commonRadixNode, com, 1);
            }
            else if (com == rnodeLen)
            {
                ___vCache_push(instance, commonRadixNode);
                baseNode = commonRadixNode;
                word = word + com;
            }
            else
            {
                if (! ___vNodeSplitAtIndex(instance, commonRadixNode, com, 0))
                    return 0;

                return ___vNodeInsertChild(instance, commonRadixNode, word + com);
            }
        }
    }

    return 1;
}

// Aggiunge un nodo in coda alla cache
inline void ___vCache_push(vocabulary * instance, vNode * node)
{
    instance -> cache[(instance -> cacheIndex) ++] = node;
}

// Esegue il parsing di una linea dal file del dizionario
inline int ___vocb_importline(char * strIn)
{
    int ind;
    char c;

    // Tronca la stringa per eliminare '\n' e commenti
    for (ind = 0;
         strIn[ind] != '\0' && strIn[ind] != '\n' && strIn[ind] != '/';
         ind ++) ;

    strIn[ind] = '\0';

    // Controlla la lunghezza della parola
    if (ind > MAX_WORD_LEN || ind < MIN_WORD_LEN) return 0;

    // Converti i caratteri secondo le specifiche
    for(ind = ind - 1; ind >= 0; ind--)
    {
        c = strIn[ind];

        if (c < 'a' || c > 'z')
        {
            if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
            else
            {
                switch(c)
                {
                  case 'à': case 'á':
                      c = 'a'; break;
                  case 'è': case 'é':
                      c = 'e'; break;
                  case 'ì': case 'í':
                      c = 'i'; break;
                  case 'ò': case 'ó':
                      c = 'o'; break;
                  case 'ù': case 'ú':
                      c = 'u'; break;
                  default: return 0;
                };
            }

            strIn[ind] = c;
        }
    }

    return 1;
}

// Ricerca un nodo con radice comune alla stringa passata
inline vNode * ___vNode_searchRadix(vNode * startingNode, char * startingChars)
{
    while (startingNode != NULL)
    {
        if (startingNode -> word[0] == startingChars[0]) return startingNode;
        startingNode = startingNode -> next;
    }

    return NULL;
}

// Inserisce un nodo figlio con la stringa passata al nodo passato
inline int ___vNodeInsertChild(vocabulary * instance, vNode * parent, char * word)
{
    vNode * newNode;

    if (!___vNode_new(&newNode)) return 0;
    if (!___vNode_setword(&newNode, word))
    {
        ___vNode_dispose(newNode);
        return 0;
    }
    newNode -> next = parent -> child;
    parent -> child = newNode;

    ___vCache_push(instance, newNode);
    return 1;
}

// Divide il nodo passato ad una certa posizione della sua stringa creando
// un nodo figlio
inline int ___vNodeSplitAtIndex(vocabulary * instance, vNode * node, int index, int isWord)
{
    vNode * splitNode;

    if (!___vNode_new(&splitNode)) return 0;
    if (!___vNode_setword(&splitNode, node -> word + index))
    {
        ___vNode_dispose(splitNode);
        return 0;
    }

    splitNode -> child = node -> child;
    splitNode -> isWord = node -> isWord;

    node -> child = splitNode;

    node -> word[index] = '\0';
    node -> isWord = isWord;

    ___vCache_push(instance, node);
    return 1;
}
