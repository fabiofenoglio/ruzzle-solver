#ifndef _inclguard_header_h
#define _inclguard_header_h

// Prompt per la richiesta di dati all'utente
#define BOARD_FILE_PROMPT       "Board input file: "
#define VOCABULARY_FILE_PROMPT  "Dictionary input file: "
#define WORDS_NUMBER_PROMPT     "Number of words to find: "

// Messaggi di errore
#define _EMSG_BOARDIMPORT       "error importing board"
#define _EMSG_VOCBIMPORT        "error importing dictionary"
#define _EMSG_OUTWRITE          "error writing output to file"
#define _EMSG_MEMALLOC          "error allocating memory"

// Opzioni di default (file di input, dizionario, numero di parole)
// impostare NULL forza l'utente ad inserire dei dati
#define DEFAULT_BOARD_FILE      NULL
#define DEFAULT_VOCABULARY_FILE "it_IT.dic"
#define DEFAULT_WORDS_NUMBER    40u

// Stringa da concatenare al file di input per ottenere il file di output
#define RESULT_APPEND_STR       "_results"

// Massima dimensione per una linea di input dall'utente
#define MAX_LINE                1024

// Massima dimensione per un percorso di file (dipendente dall'OS)
#define MAX_FILEPATH_LEN        256

// Dimensioni della griglia
#define GS                      4
#define GRID_LEN                (GS * GS)

// Intervallo di lunghezza delle parole da considerare
#define MIN_WORD_LEN            2
#define MAX_WORD_LEN            GRID_LEN

// Macro per verificare che un'operazione critica abbia successo
#define attemptOrDieWithMessage(action, emsg) \
    {if (!(action)) \
    { \
        fprintf(stderr, "\n\tFatal error: %s\n\n\texecuting " #action \
                "\n\tin file \"" __FILE__ "\"\n\tat line %i\n", emsg, __LINE__); \
        exit(EXIT_FAILURE);\
    }}

// Macro utilizzata per validazioni nei dati (modulo di benchmarking)
#define assert(sentence) attemptOrDieWithMessage(sentence, "ASSERTION FAILED")

#endif // _inclguard_header_h
